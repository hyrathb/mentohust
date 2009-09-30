/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mystate.c
* 摘	要：改变认证状态
* 作	者：HustMoon@BYHH
*/
#include "mystate.h"
#include <pcap.h>
#include <string.h>
#include "md5.h"

#define MAX_SEND_COUNT		2	/* 最大超时次数 */

extern char userName[32];
extern char password[16];
extern u_char localMAC[6];
extern u_int32_t ip;
extern u_int32_t mask;
extern u_int32_t gateway;
extern u_int32_t dns;
extern unsigned timeout;
extern unsigned echoInterval;
extern pcap_t *pcapHandle;

const u_char *pcapBuf = NULL;	/* 抓到的包 */
volatile int state = ID_DISCONNECT;	/* 认证状态 */
static const u_char STANDARD_ADDR[] = {0x01,0x80,0xC2,0x00,0x00,0x03};	/* 认证MAC */
static u_char sendPacket[0x50] = {0};	/* 用来发送的包，最长是48+31=0x4F字节 */
static unsigned sendCount = 0;	/* 同一阶段发包计数 */

static void setTimer(unsigned interval);	/* 设置定时器 */
static int sendStartPacket();	 /* 发送Start包 */
static int sendIdentityPacket();	/* 发送Identity包 */
static int sendChallengePacket();   /* 发送Md5 Challenge包 */
static int waitEchoPacket();	/* 等候响应包 */
static int sendEchoPacket();	/* 发送响应包 */
static int sendLogoffPacket();  /* 发送退出包 */

static void setTimer(unsigned interval) /* 设置定时器 */
{
	struct itimerval timer;
	timer.it_value.tv_sec = interval;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = interval;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
}

int switchState(int type)
{
	if (state == type) /* 跟上次是同一状态？ */
		sendCount++;
	else
	{
		state = type;
		sendCount = 0;
	}
	if (sendCount >= MAX_SEND_COUNT)  /* 超时太多次？ */
	{
		switch (type)
		{
		case ID_START:
			printf(">> 找不到服务器，重启认证！\n");
			break;
		case ID_IDENTITY:
			printf(">> 发送用户名超时，重启认证！\n");
			break;
		case ID_CHALLENGE:
			printf(">> 发送密码超时，重启认证！\n");
			break;
		case ID_WAITECHO:
			printf(">> 等候响应包超时，重启认证！");
			break;
		}
		state = ID_DISCONNECT;	/* switchState(ID_DISCONNECT); */
		return switchState(ID_START);
	}
	switch (type)
	{
	case ID_START:
		return sendStartPacket();
	case ID_IDENTITY:
		return sendIdentityPacket();
	case ID_CHALLENGE:
		return sendChallengePacket();
	case ID_WAITECHO:
		return waitEchoPacket();
	case ID_ECHO:
		return sendEchoPacket();
	case ID_DISCONNECT:
		return sendLogoffPacket();
	}
	return 0;
}

static int sendStartPacket()
{
	if (sendCount == 0)
	{
		memcpy(sendPacket, STANDARD_ADDR, 6);
		memcpy(sendPacket+0x06, localMAC, 6);
		*(u_int16_t *)(sendPacket+0x0C) = htons(0x888E);
		*(u_int16_t *)(sendPacket+0x0E) = htons(0x0101);
		*(u_int16_t *)(sendPacket+0x10) = 0;
		memset(sendPacket+0x12, 0xa5, 42);
		setTimer(timeout);
		printf(">> 寻找服务器...\n");
	}
	return pcap_sendpacket(pcapHandle, sendPacket, 60);
}

static int sendIdentityPacket()
{
	int nameLen = strlen(userName);
	if (sendCount == 0)
	{
		*(u_int16_t *)(sendPacket+0x0E) = htons(0x0100);
		*(u_int16_t *)(sendPacket+0x10) = *(u_int16_t *)(sendPacket+0x14) = htons(nameLen+30);
		sendPacket[0x12] = 0x02;
		sendPacket[0x16] = 0x01;
		sendPacket[0x17] = 0x01;
		memcpy(sendPacket+0x18, &ip, 4);
		memcpy(sendPacket+0x1C, &mask, 4);
		memcpy(sendPacket+0x20, &gateway, 4);
		memcpy(sendPacket+0x24, &dns, 4);
		memcpy(sendPacket+0x28, "03.02.05", 8);
		memcpy(sendPacket+0x30, userName, nameLen);
		setTimer(timeout);
		printf(">> 发送用户名...\n");
	}
	sendPacket[0x13] = pcapBuf[0x13];
	return pcap_sendpacket(pcapHandle, sendPacket, nameLen+48);
}

static int sendChallengePacket()
{
	int nameLen = strlen(userName);
	if (sendCount == 0)
	{
		int md5Len = 0;
		u_char md5Src[50];
		*(u_int16_t *)(sendPacket+0x0E) = htons(0x0100);
		*(u_int16_t *)(sendPacket+0x10) = *(u_int16_t *)(sendPacket+0x14) = htons(nameLen+22);
		sendPacket[0x12] = 0x02;
		sendPacket[0x13] = pcapBuf[0x13];
		sendPacket[0x16] = 0x04;
		sendPacket[0x17] = 16;
		md5Src[md5Len++] = pcapBuf[0x13];
		memcpy(md5Src+md5Len, password, strlen(password));
		md5Len += strlen(password);
		memcpy(md5Src+md5Len, "xxghlmxhzb", 10);
		md5Len += 10;
		memcpy(md5Src+md5Len, pcapBuf+0x18, pcapBuf[0x17]);
		md5Len += pcapBuf[0x17];
		memcpy(sendPacket+0x18, ComputeHash(md5Src, md5Len), 16);
		memcpy(sendPacket+0x28, userName, nameLen);
		setTimer(timeout);
		printf(">> 发送密码...\n");
	}
	return pcap_sendpacket(pcapHandle, sendPacket, nameLen+40);
}

static int waitEchoPacket()
{
	if (sendCount == 0)
		setTimer(echoInterval);
	return 0;
}

static int sendEchoPacket()
{
	*(u_int16_t *)(sendPacket+0x0E) = htons(0x0106);
	*(u_int16_t *)(sendPacket+0x10) = 0;
	memset(sendPacket+0x12, 0xa5, 42);
	printf(">> 发送了一次响应包。\n");
	switchState(ID_WAITECHO);	/* 继续等待 */
	return pcap_sendpacket(pcapHandle, sendPacket, 60);
}

static int sendLogoffPacket()
{
	setTimer(0);	/* 取消定时器 */
	*(u_int16_t *)(sendPacket+0x0E) = htons(0x0102);
	*(u_int16_t *)(sendPacket+0x10) = 0;
	memset(sendPacket+0x12, 0xa5, 42);
	return pcap_sendpacket(pcapHandle, sendPacket, 60);
}
