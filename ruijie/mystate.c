/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mystate.c
* 摘	要：改变认证状态
* 作	者：HustMoon@BYHH
*/
#include "mystate.h"

static struct itimerval timer;	/* 用于定时器 */
static u_char sendPacket[0x3E8] = {0};	/* 用来发送的包 */
static unsigned sendCount = 0;	/* 同一阶段发包机数 */

static void setTimer(unsigned interval);	/* 设置定时器 */
static void fillEtherAddr(u_int32_t protocol);  /* 填充MAC地址和协议 */
static int sendTestPacket();	/* 发送试图重连的包 */
static int sendStartPacket();	 /* 发送Start包 */
static int sendIdentityPacket();	/* 发送Identity包 */
static int sendChallengePacket();   /* 发送Md5 Challenge包 */
static int sendEchoPacket();		/* 发送定时响应包 */
static int sendLogoffPacket();  /* 发送退出包 */

static void setTimer(unsigned interval) /* 设置定时器 */
{
	timer.it_value.tv_sec = interval;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = interval;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
	logDebug("## Interval = %us\n", interval);
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
	if (sendCount>=MAX_SEND_COUNT && type!=ID_ECHO)  /* 超时太多次？ */
	{
		switch (type)
		{
		case ID_START:
			logEvent(">> 找不到服务器，重启认证!\n");
			break;
		case ID_IDENTITY:
			logEvent(">> 发送用户名超时，重启认证!\n");
			break;
		case ID_CHALLENGE:
			logEvent(">> 发送密码超时，重启认证!\n");
			break;
		}
		switchState(ID_DISCONNECT);
		return switchState(ID_START);
	}
	switch (type)
	{
	case ID_DHCP:
		setTimer(0);
		logEvent(">> 正在获取IP...\n");
		system(dhcpScript);
		logEvent(">> 操作结束。\n");
		dhcpMode += 3; /* 标记为已获取，123变为456，5不需再认证*/
		if (fillHeader(&dataPacket, nic, startMode, dhcpMode, &addrs) == -1)
			doExit(-1);
		if (dhcpMode == 5)
			return switchState(ID_ECHO);
		return switchState(ID_START);
	case ID_START:
		return sendStartPacket();
	case ID_IDENTITY:
		return sendIdentityPacket();
	case ID_CHALLENGE:
		return sendChallengePacket();
	case ID_ECHO:
		if (dhcpMode==1 || dhcpMode==2)   /* 认证后或二次认证？ */
			return switchState(ID_DHCP);
		if (autoReconnect!=0 && autoReconnect<=echoInterval*sendCount)	/* 需要自动重连？ */
			sendTestPacket();
		return sendEchoPacket();
	case ID_DISCONNECT:
		return sendLogoffPacket();
	}
	return 0;
}

static void fillEtherAddr(u_int32_t protocol)
{
	memset(sendPacket, 0, 0x3E8);
	memcpy(sendPacket, addrs.dmac, 6);
	memcpy(sendPacket+0x06, addrs.lmac, 6);
	*(u_int32_t *)(sendPacket+0x0C) = htonl(protocol);
}

static int sendTestPacket()
{
	logEvent(">> 自动重连间隔已到，尝试重连，如无后续输出说明无需重连。\n");	/* 建议支持私有认证的才开启这个功能 */
	fillStartPacket(&dataPacket, dhcpMode, addrs.lmac);
	fillEtherAddr(0x888E0101);
	if (startMode % 2 == 0)
		memcpy(sendPacket, STANDARD_ADDR, 6);
	else
		memcpy(sendPacket, RUIJIE_ADDR, 6);
	memcpy(sendPacket+0x12, dataPacket.buf, dataPacket.size);
	sendCount = 0;
	return pcap_sendpacket(pcapHandle, sendPacket, 0x3E8);
}

static int sendStartPacket(int isTest)
{
	if (sendCount == 0)
	{
		logEvent(">> 寻找服务器...\n");
		fillStartPacket(&dataPacket, dhcpMode, addrs.lmac);
		fillEtherAddr(0x888E0101);
		memcpy(sendPacket+0x12, dataPacket.buf, dataPacket.size);
		setTimer(timeout);
	}
	return pcap_sendpacket(pcapHandle, sendPacket, 0x3E8);
}

static int sendIdentityPacket()
{
	int nameLen;
	if (sendCount == 0)
	{
		logEvent(">> 发送用户名...\n");
		fillEtherAddr(0x888E0100);
		nameLen = strlen(userName);
		*(u_int16_t *)(sendPacket+0x14) = *(u_int16_t *)(sendPacket+0x10) = htons(nameLen+5);
		sendPacket[0x12] = 2;
		sendPacket[0x13] = serverKey[3];
		sendPacket[0x16] = 1;
		memcpy(sendPacket+0x17, userName, nameLen);
		memcpy(sendPacket+0x17+nameLen, dataPacket.buf, dataPacket.size);
		setTimer(timeout);
	}
	return pcap_sendpacket(pcapHandle, sendPacket, 0x3E8);
}

static int sendChallengePacket()
{
	int nameLen;
	if (sendCount == 0)
	{
		logEvent(">> 发送密码...\n");
		fillMd5Packet(&dataPacket, dhcpMode, addrs.lmac, serverKey+0x08);
		fillEtherAddr(0x888E0100);
		nameLen = strlen(userName);
		*(u_int16_t *)(sendPacket+0x14) = *(u_int16_t *)(sendPacket+0x10) = htons(nameLen+22);
		sendPacket[0x12] = 2;
		sendPacket[0x13] = serverKey[3];
		sendPacket[0x16] = 4;
		sendPacket[0x17] = 16;
		memcpy(sendPacket+0x18, checkPass(serverKey[3], serverKey+0x08, password), 16);
		memcpy(sendPacket+0x28, userName, nameLen);
		memcpy(sendPacket+0x28+nameLen, dataPacket.buf, dataPacket.size);
		setTimer(timeout);
	}
	return pcap_sendpacket(pcapHandle, sendPacket, 0x3E8);
}

static int sendEchoPacket()
{
	if (sendCount == 0)
	{
		u_char echo[] =
		{
			0x00,0x1E,0xFF,0xFF,0x37,0x77,0x7F,0x9F,0xFF,0xFF,0xD9,0x13,0xFF,0xFF,0x37,0x77,
			0x7F,0x9F,0xFF,0xFF,0xF7,0x2B,0xFF,0xFF,0x37,0x77,0x7F,0x3F,0xFF
		};
		logEvent(">> 发送心跳包以保持在线...\n");
		fillEtherAddr(0x888E01BF);
		memcpy(sendPacket+0x10, echo, sizeof(echo));
		setTimer(echoInterval);
	}
	fillEchoPacket(sendPacket);
	return pcap_sendpacket(pcapHandle, sendPacket, 0x2D);
}

static int sendLogoffPacket()
{
	setTimer(0);
	fillStartPacket(&dataPacket, dhcpMode, addrs.lmac);
	fillEtherAddr(0x888E0102);
	memcpy(sendPacket+0x12, dataPacket.buf, dataPacket.size);
	return pcap_sendpacket(pcapHandle, sendPacket, 0x3E8);
}
