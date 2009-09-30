/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mentohust.c
* 摘	要：MentoHUST主函数
* 作	者：HustMoon@BYHH
*/
#include <iconv.h>
#include <signal.h>
#include "myconfig.h"
#include "mystate.h"

static void sig_handle(int sig);	/* 信号回调 */
static void pcap_handle(u_char *user, const struct pcap_pkthdr *h, const u_char *p);	/* pcap_loop回调 */
static void printMsg(const u_char *buf, unsigned bufLen);	/* 显示认证成功或失败后返回的消息 */
static int gbk2utf(char **dst, char *src, size_t srclen);	/* GBK转UTF8 */

int main(int argc, char **argv)
{
	initConfig(argc, argv);	/* 初始化参数 */
	signal(SIGALRM, sig_handle);	/* 定时器 */
	signal(SIGHUP, sig_handle);	 /* 注销时 */
	signal(SIGINT, sig_handle);	 /* Ctrl+C */
	signal(SIGQUIT, sig_handle);	/* Ctrl+\ */
	signal(SIGTSTP, sig_handle);	/* Ctrl+Z */
	signal(SIGTERM, sig_handle);	/* 被结束时 */
	if (dhcpMode == 3)	  /* 认证前DHCP */
		switchState(ID_DHCP);
	else
		switchState(ID_START);
	if (-1 == pcap_loop(pcapHandle, -1, pcap_handle, NULL)) /* 开始捕获数据包 */
		logEvent("!! 捕获数据包失败，请检查网络连接!\n");
	doExit(-1);
}

static void sig_handle(int sig)
{
	if (sig == SIGALRM)	 /* 定时器 */
	{
		if (-1 == switchState(state))
		{
			pcap_breakloop(pcapHandle);
			logEvent("!! 发送数据包失败, 请检查网络连接!\n");
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
	switch (buf[0x12])
	{
	case 0x01:				  /* Request，验证用户名或者密码时收到 */
		memcpy(serverKey, buf+0x10, 0x20);
		logDebug("## [10+20]: %s\n", printHex(serverKey, 0x20));
		switch (buf[0x16])
		{
		case 0x01:			  /* Request Identity */
			if (startMode < 2)
			{
				memcpy(addrs.dmac, buf+6, 6);
				logEvent("** 认证MAC:\t%s\n", printHex(addrs.dmac, 6));
				startMode += 2;
			}
			switchState(ID_IDENTITY);
			break;
		case 0x04:			  /* Request MD5-Challenge */
			switchState(ID_CHALLENGE);
			break;
		}
		break;

	case 0x03:		  		/* Success */
		logEvent(">> 认证成功!\n");
		getEchoKey(buf);
		printMsg(buf, h->len);
		switchState(ID_ECHO);
		break;

	case 0x04:	  /* 认证失败或被踢下线，只要不是服务器无提示掉线，就可以实现重连 */
		if (state == ID_ECHO)
		{
			logEvent(">> 认证掉线，开始重连!\n");
			switchState(ID_START);
		}
		else if (buf[0x1b] > 0)
		{
			logEvent(">> 认证失败!\n");
			printMsg(buf, h->len);
		}
		break;
	}/* end switch */
}

static void printMsg(const u_char *buf, unsigned bufLen)
{
	char *serverMsg;
	unsigned length = buf[0x1b];
	if (length > 0)
	{
		for (serverMsg=(char *)(buf+0x1c); *serverMsg=='\r'||*serverMsg=='\n'; serverMsg++,length--);	/* 跳过开头的换行符 */
		if (strlen(serverMsg) < length)
			length = strlen(serverMsg);
		if (gbk2utf(&serverMsg, serverMsg, length) == 0)
		{
			logEvent("$$ 系统提示:\t%s\n", serverMsg);
			free(serverMsg);
		}
	}
	if ((length=0x1c+buf[0x1b]+0x69+39) < bufLen)
	{
		logDebug("## 2nd message offset=%u length=%u", length, buf[length-1]-2);
		serverMsg=(char *)(buf+length);
		if (buf[length-1]-2 > bufLen-length)
			length = bufLen - length;
		else
			length = buf[length-1]-2;
		for (; *serverMsg=='\r'||*serverMsg=='\n'; serverMsg++,length--);
		if (gbk2utf(&serverMsg, serverMsg, length) == 0)
		{
			logEvent("$$ 计费信息:\t%s\n", serverMsg);
			free(serverMsg);
		}
	}
}
static int gbk2utf(char **dst, char *src, size_t srclen)
{
	/* GBK一汉字俩字节，UTF-8一汉字3字节，二者ASCII字符均一字节 */
	size_t dstlen = srclen * 3 / 2 + 1;	/* 所以这样申请是足够的了，要记得释放 */
	size_t left = dstlen;
	int res = 0;
	char *pdst = NULL;
	iconv_t cd  = iconv_open("utf-8", "gbk");
	if (cd == (iconv_t)-1)
		return -1;
	*dst = (char *)malloc(dstlen);
	pdst = *dst;
	res = iconv(cd, &src, &srclen, &pdst, &left);
	iconv_close(cd);
	if (res == -1)
	{
		free(*dst);
		return -1;
	}
	(*dst)[dstlen-left] = '\0';
	return 0;
}
