/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：function.h
* 摘	要：认证相关算法及方法
* 作	者：HustMoon@BYHH
*/
#ifndef HUSTMOON_FUNCTION_H
#define HUSTMOON_FUNCTION_H

#include <stdlib.h>
#include <string.h>

#define MAX_PATH			260	/* 最大路径长度 */

typedef struct uc_buf	/* 这些结构体就是为了传参方便，原打算把该文件写成一个库的，结果就写成了这样 */
{
	u_char *buf;
	unsigned size;
}UC_BUF;

typedef struct net_addrs
{
	u_int32_t ip;
	u_int32_t mask;
	u_int32_t gateway;
	u_int32_t dns;
	u_char lmac[6];
	u_char dmac[6];
}NET_ADDRS;

typedef struct file_names
{
	char data[MAX_PATH];
	char log[MAX_PATH];
	int mode;
}FILE_NAMES;

extern const u_char STANDARD_ADDR[], RUIJIE_ADDR[];	/* 标准、私有组播地址 */

void logEvent(const char *fmt, ...);	/* 输出事件信息 */
void logDebug(const char *fmt, ...);	/* 输出高级信息，这两个函数写得太烂了，貌似可以用宏实现的 */
char *printHex(const u_char *buf, int length);	/* 转换成十六进制形式 */
void setData(FILE_NAMES *files, UC_BUF *data);	/* 设置文件名，开辟内存 */
int fillHeader(UC_BUF *data, const char *nic, unsigned startMode, unsigned dhcpMode, NET_ADDRS *addrs);	/* 填充网络地址及校验值部分 */
void fillStartPacket(UC_BUF *data, unsigned dhcpMode, const u_char *localMAC);	/* 填充Start包 */
void fillMd5Packet(UC_BUF *data, unsigned dhcpMode, const u_char *localMAC, const u_char *md5Seed);	/* 填充Md5包 */
void fillEchoPacket(u_char *sendPacket);	/* 填充Echo包 */
void getEchoKey(const u_char *pcapBuf);	/* 获取EchoKey */
u_char *checkPass(u_char id, const u_char *md5Seed, const char *password);	/* 计算密码的md5 */

#endif

