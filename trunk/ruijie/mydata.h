/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mydata.h
* 摘	要：认证需要用到的全局量
* 作	者：HustMoon@BYHH
*/
#ifndef HUSTMOON_MYDATA_H
#define HUSTMOON_MYDATA_H

#include <pcap.h>
#include "function.h"

#define ACCOUNT_SIZE		65	/* 用户名密码长度*/
#define NIC_SIZE			60	/* 网卡名最大长度 */
#define MAX_SEND_COUNT		3	/* 最大超时次数 */
#define ID_DISCONNECT		0	/* 断开状态 */
#define ID_START			1	/* 寻找服务器 */
#define ID_IDENTITY			2	/* 发送用户名 */
#define ID_CHALLENGE		3	/* 发送密码 */
#define ID_ECHO				4	/* 发送定时响应 */
#define ID_DHCP				5	/* 更新IP */

extern const char *APP_VERSION;		/* 程序版本 */
extern const char *CFG_FILE/*, *LOCK_FILE*/;	/* 配置文件名、加锁文件名 */

extern char userName[ACCOUNT_SIZE], password[ACCOUNT_SIZE];	/* 用户名、密码 */
extern char nic[NIC_SIZE];	/* 网卡名 */
extern char dhcpScript[MAX_PATH];	/* DHCP脚本文件 */
extern NET_ADDRS addrs;	/* 各种网络地址 */
extern unsigned timeout, echoInterval, autoReconnect;	/* 认证超时、响应间隔、自动重连间隔 */
extern unsigned startMode, dhcpMode;	/* 组播方式、DHCP方式 */
extern pcap_t *pcapHandle;	/* pcap句柄 */
extern UC_BUF dataPacket;	/* 用来填充的数据包 */
extern u_char serverKey[0x20];	/* 保存MD5种子及服务器发来的其他有用信息 */
extern volatile int state;	/* 认证状态 */

void doExit(int exitCode);	/* 退出 */
#endif
