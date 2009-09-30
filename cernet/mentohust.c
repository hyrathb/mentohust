/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mentohust.c
* 摘	要：MentoHUST主函数
* 作	者：HustMoon@BYHH
*/
#include "myconfig.h"
#include "mystate.h"
#include <pcap.h>
#include <signal.h>
#include <string.h>
#include <iconv.h>

extern pcap_t *pcapHandle;
extern volatile int state;
extern const u_char *pcapBuf;

static int doExit(int exitCode);	/* 退出 */
static void sig_handle(int sig);	/* 信号回调 */
static void pcap_handle(u_char *user, const struct pcap_pkthdr *h, const u_char *buf);	/* pcap_loop回调 */
static char *gbk2utf(char *src, size_t srclen);	/* GBK转UTF－8 */
static void showMsg(const u_char *buf);	/* 显示服务器提示信息 */

int main(int argc, char **argv)
{
	initConfig(argc, argv);
	signal(SIGALRM, sig_handle);	/* 定时器 */
	signal(SIGHUP, sig_handle);	 /* 注销时 */
	signal(SIGINT, sig_handle);	 /* Ctrl+C */
	signal(SIGQUIT, sig_handle);	/* Ctrl+\ */
	signal(SIGTSTP, sig_handle);	/* Ctrl+Z */
	signal(SIGTERM, sig_handle);	/* 被结束时 */
	switchState(ID_START);	/* 开始认证 */
	if (-1 == pcap_loop(pcapHandle, -1, pcap_handle, NULL)) /* 开始捕获数据包 */
		printf("!! 捕获数据包失败，请检查网络连接!\n");
	doExit(-1);
}

static int doExit(int exitCode)
{
	if (state != ID_DISCONNECT)
		switchState(ID_DISCONNECT);
	if (pcapHandle != NULL)
		pcap_close(pcapHandle);
	printf(">> 认证已退出。\n");
	exit(exitCode);
}

static void sig_handle(int sig)
{
	if (sig == SIGALRM)	 /* 定时器 */
	{
		if (-1 == switchState(state))
		{
			pcap_breakloop(pcapHandle);
			printf("!! 发送数据包失败, 请检查网络连接!\n");
			doExit(-1);
		}
	}
	else	/* 退出 */
	{
		pcap_breakloop(pcapHandle);
		doExit(0);
	}
}

static void pcap_handle(u_char *user, const struct pcap_pkthdr *h, const u_char *buf)
{
	static int isFirst = 1;	/* 是否已得到服务器MAC */
	static u_char servMAC[6] = {0};	/* 服务器MAC */

	if (!isFirst && memcmp(servMAC, buf+6, 6)!=0)	/* 服务器MAC地址不符 */
	{
		printf("** 舍弃MAC:\t%s\n", formatHex(buf+6, 6));
		return;
	}
	pcapBuf = buf;
	if (buf[0x0F]==0x00 && buf[0x12]==0x01 && buf[0x16]==0x01)	/* 验证用户名 */
	{
		if (isFirst)	/* 记录服务器MAC */
		{
			memcpy(servMAC, buf+6, 6);
			printf("** 认证MAC:\t%s\n", formatHex(servMAC, 6));
			isFirst = 0;
		}
		switchState(ID_IDENTITY);
	}
	else if (buf[0x0F]==0x00 && buf[0x12]==0x01 && buf[0x16]==0x04)	/* 验证密码 */
		switchState(ID_CHALLENGE);
	else if (buf[0x0F]==0x00 && buf[0x12]==0x03)	/* 认证成功 */
	{
		printf(">> 认证成功!\n");
		switchState(ID_WAITECHO);
	}
	else if (buf[0x0F]==0x00 && buf[0x12]==0x01 && buf[0x16]==0x02)	/* 显示提示信息 */
		showMsg(buf);
	else if (buf[0x0F] == 0x05)	/* 响应在线 */
		switchState(ID_ECHO);
	else if (buf[0x0F]==0x00 && buf[0x12]==0x04)  /* 认证失败或被踢下线 */
	{
		if (state == ID_WAITECHO)
			printf(">> 认证掉线，开始重连!\n");
		else
			printf(">> 认证失败!\n");
		switchState(ID_START);
	}
}

static char *gbk2utf(char *src, size_t srclen)
{
	/* GBK一汉字俩字节，UTF-8一汉字3字节，二者ASCII字符均一字节
		 所以这样申请是足够的了，要记得释放 */
	size_t dstlen = srclen * 3 / 2 + 1;
	size_t left = dstlen;
	char *dst, *pdst;
	int res;
	iconv_t cd  = iconv_open("utf-8", "gbk");
	if (cd == (iconv_t)-1)
		return NULL;
	dst = (char *)malloc(dstlen);
	pdst = dst;
	res = iconv(cd, &src, &srclen, &pdst, &left);
	iconv_close(cd);
	if (res == -1)
	{
		free(dst);
		return NULL;
	}
	dst[dstlen-left] = '\0';
	return dst;
}

static void showMsg(const u_char *buf)
{
	char *servMsg;
	int length = ntohs(*(u_int16_t *)(buf+0x14)) - 5;
	if (length<=0 || (servMsg=gbk2utf((char *)(buf+0x17), length))==NULL)
		return;
	printf("$$ 系统提示: %s\n", servMsg);
	free(servMsg);
}

