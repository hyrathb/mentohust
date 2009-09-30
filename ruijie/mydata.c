/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mydata.c
* 摘	要：认证需要用到的数据
* 作	者：HustMoon@BYHH
*/
#include "mydata.h"
#include "mystate.h"

const char *APP_VERSION = "0.2.1";
const char *CFG_FILE = "/etc/mentohust/mentohust.conf";
/* const char *LOCK_FILE = "/var/run/mentohust.pid"; */

char userName[ACCOUNT_SIZE] = "", password[ACCOUNT_SIZE] = "";
char nic[NIC_SIZE] = "";
char dhcpScript[MAX_PATH] = "";
NET_ADDRS addrs = {0, 0, 0, 0, {0}, {0}};
unsigned timeout = 3, echoInterval = 30, autoReconnect = 0;
unsigned startMode = 0, dhcpMode = 0;
pcap_t *pcapHandle = NULL;
UC_BUF dataPacket = {NULL, 0};
u_char serverKey[0x20] = {0};
volatile int state = ID_DISCONNECT;

void doExit(int exitCode)	/* 其实貌似是可以用atexit()注册一个函数的，不过也差不多啦 */
{
	if (state != ID_DISCONNECT)
		switchState(ID_DISCONNECT);
	if (pcapHandle != NULL)
		pcap_close(pcapHandle);
	if (dataPacket.buf != NULL)
		free(dataPacket.buf);
	logEvent(">> 认证已退出。\n");
	exit(exitCode);
}
