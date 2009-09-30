/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：function.c
* 摘	要：认证相关算法及方法
* 作	者：HustMoon@BYHH
*/

#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include "md5.h"
#include "function.h"

const u_char STANDARD_ADDR[] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};
const u_char RUIJIE_ADDR[] = {0x01, 0xD0, 0xF8, 0x00, 0x00, 0x03};

static char dataFile[MAX_PATH] = "";	/* 数据文件名 */
static char logFile[MAX_PATH] = "";	/* 日志文件名 */	
static int logMode = 0;	/* 输出方式：0.stdout 1.文件 2.文件(Debug) */
static u_int32_t readSize = 0, checkSize = 0;	/* 读取大小、校验大小 */
static u_int32_t echoKey = 0, echoNo = 0;	/* Echo阶段所需 */

static char *getTime();	/* 获取格式化的当前时间 */
static int getAddress(const char *nic, unsigned startMode, unsigned dhcpMode, NET_ADDRS *addrs);	/* 获取网络地址 */
static u_char encode(u_char base);	/* 锐捷算法，颠倒一个字节的8位 */
static void checkSum(u_char *buf);	/* 锐捷算法，计算两个字节的检验值 */
static int setVendor(UC_BUF *data, u_char type, const u_char *value, int length);	/* 设置指定属性 */
static int readPacket(UC_BUF *data, unsigned dhcpMode, const u_char *localMAC, int type);	/* 读取数据 */
static int checkRuijie(UC_BUF *data, const u_char *md5Seed);	/* V2校验算法 */

static char *getTime()
{
	static char buf[24];
	time_t now = time(NULL);
	struct tm *pt = localtime(&now);
	/* strftime(buf, sizeof(buf), "[%F %T] ", pt); 某C编译器不支持*/
	strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S] ", pt);
	return buf;
}

void logEvent(const char *fmt, ...)
{
	FILE *fp = NULL;
	va_list va;
	int len;
	char str[2048] = "";
	if (logMode == 2)
		strcpy(str, getTime());
	len = strlen(str);
	va_start(va, fmt);
	vsprintf(str+strlen(str), fmt, va);
	va_end(va);
	printf("%s", str+len);
	if (logMode == 0)
		return;
	if ((fp = fopen(logFile, "a")) == NULL)
	{
		printf("!! 打开日志文件%s失败，改用默认输出方式。\n", logFile);
		logMode = 0;
		return;
	}
	fprintf(fp, "%s", str);
	fclose(fp);
}

void logDebug(const char *fmt, ...)
{
	FILE *fp = NULL;
	va_list va;
	char str[2048] = "";
	if (logMode != 2)
		return;
	strcpy(str, getTime());
	va_start(va, fmt);
	vsprintf(str+strlen(str), fmt, va);
	va_end(va);
	if ((fp = fopen(logFile, "a")) == NULL)
	{
		printf("!! 打开日志文件%s失败，改用默认输出方式。\n", logFile);
		logMode = 0;
		return;
	}
	fprintf(fp, "%s", str);
	fclose(fp);
}

char *printHex(const u_char *buf, int length)
{
	static char hex[385];
	int i;
	if (length > 128)
		length = 128;
	for (i=0; i<length; i++)
		sprintf(hex+3*i,"%02x:", buf[i]);
	hex[3*length-1] = '\0';
	return hex;
}

void setData(FILE_NAMES *files, UC_BUF *data)
{
	u_char buf[16];
	FILE *fp = NULL;
	if (files->log[0] != '\0')
	{
		if ((fp=fopen(files->log, "w")) == NULL)
		{
			printf("!! 打开日志文件%s失败，改用默认输出方式。\n", files->log);
			files->log[0] = '\0';
		}
		else
		{
			fclose(fp);
			strcpy(logFile, files->log);
			logMode = files->mode;
		}
	}
	
	if ((fp=fopen(files->data, "rb")) == NULL)
		goto fileError;
	if (fread(buf, 16, 1, fp) < 1)
	{
		fclose(fp);
		goto fileError;
	}
	readSize = (*(u_int32_t *)buf ^ *(u_int32_t *)(buf + 8)) + 16;
	checkSize = (*(u_int32_t *)buf ^ *(u_int32_t *)(buf + 12)) + 16;
	fseek(fp, 0, SEEK_END);
	data->size = ftell(fp);
	fclose(fp);
	logDebug("## FileName:%s FileSize:%d\n", files->data, data->size);
	if (data->size < readSize)
		goto fileError;
	data->size = (data->size - readSize) / 2 + 0x17;
	if (checkSize < readSize || data->size < 0x80)
		goto fileError;
	strcpy(dataFile, files->data);
	data->buf = (u_char *)malloc(data->size);
	logDebug("## ReadSize=%u CheckSize=%u PacketSize=%u\n", readSize, checkSize, data->size);
	return;

fileError:
	if (files->data[0] != '\0')
	{
		logEvent("!! 所选数据包%s无效，改用内置数据认证。\n", files->data);
		files->data[0] = '\0';
	}
	data->size = 0x80;
	data->buf = (u_char *)malloc(data->size);
}

int getAddress(const char *nic, unsigned startMode, unsigned dhcpMode, NET_ADDRS *addrs)
{
	u_char *p = NULL;
	struct ifreq ifr;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		logEvent("!! 创建套接字失败!\n");
		return -1;
	}
	strcpy(ifr.ifr_name, nic);
	
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		close(sock);
		logEvent("!! 在网卡%s上获取MAC失败!\n", nic);
		return -1;
	}
	memcpy(addrs->lmac, ifr.ifr_hwaddr.sa_data, 6);
	
	if (startMode == 0)
		memcpy(addrs->dmac, STANDARD_ADDR, 6);
	else if (startMode == 1)
		memcpy(addrs->dmac, RUIJIE_ADDR, 6);
		
	if (dhcpMode!=0 || addrs->ip==0)
	{
		if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
			logEvent("!! 在网卡%s上获取IP失败!\n", nic);
		else
			addrs->ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
	}
	
	if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0)
		logEvent("!! 在网卡%s上获取子网掩码失败!\n", nic);
	else
		addrs->mask = ((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr.s_addr;

	close(sock);
	logEvent("** 本机MAC:\t%s\n", printHex(addrs->lmac, 6));
	p = (u_char *)(&addrs->ip);
	logEvent("** 使用IP:\t%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	p = (u_char *)(&addrs->mask);
	logEvent("** 子网掩码:\t%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	return 0;
}

static u_char encode(u_char base)	/* 算法，将一个字节的8位颠倒并取反 */
{
	u_char result = 0;
	int i;
	for (i=0; i<8; i++)
	{
		result <<= 1;
		result |= base&0x01;
		base >>= 1;
	}
	return ~result;
}

static void checkSum(u_char *buf)	/* 算法，计算两个字节的checksum */
{
	u_char table[] =
	{
		0x00,0x00,0x21,0x10,0x42,0x20,0x63,0x30,0x84,0x40,0xA5,0x50,0xC6,0x60,0xE7,0x70,
		0x08,0x81,0x29,0x91,0x4A,0xA1,0x6B,0xB1,0x8C,0xC1,0xAD,0xD1,0xCE,0xE1,0xEF,0xF1,
		0x31,0x12,0x10,0x02,0x73,0x32,0x52,0x22,0xB5,0x52,0x94,0x42,0xF7,0x72,0xD6,0x62,
		0x39,0x93,0x18,0x83,0x7B,0xB3,0x5A,0xA3,0xBD,0xD3,0x9C,0xC3,0xFF,0xF3,0xDE,0xE3,
		0x62,0x24,0x43,0x34,0x20,0x04,0x01,0x14,0xE6,0x64,0xC7,0x74,0xA4,0x44,0x85,0x54,
		0x6A,0xA5,0x4B,0xB5,0x28,0x85,0x09,0x95,0xEE,0xE5,0xCF,0xF5,0xAC,0xC5,0x8D,0xD5,
		0x53,0x36,0x72,0x26,0x11,0x16,0x30,0x06,0xD7,0x76,0xF6,0x66,0x95,0x56,0xB4,0x46,
		0x5B,0xB7,0x7A,0xA7,0x19,0x97,0x38,0x87,0xDF,0xF7,0xFE,0xE7,0x9D,0xD7,0xBC,0xC7,
		0xC4,0x48,0xE5,0x58,0x86,0x68,0xA7,0x78,0x40,0x08,0x61,0x18,0x02,0x28,0x23,0x38,
		0xCC,0xC9,0xED,0xD9,0x8E,0xE9,0xAF,0xF9,0x48,0x89,0x69,0x99,0x0A,0xA9,0x2B,0xB9,
		0xF5,0x5A,0xD4,0x4A,0xB7,0x7A,0x96,0x6A,0x71,0x1A,0x50,0x0A,0x33,0x3A,0x12,0x2A,
		0xFD,0xDB,0xDC,0xCB,0xBF,0xFB,0x9E,0xEB,0x79,0x9B,0x58,0x8B,0x3B,0xBB,0x1A,0xAB,
		0xA6,0x6C,0x87,0x7C,0xE4,0x4C,0xC5,0x5C,0x22,0x2C,0x03,0x3C,0x60,0x0C,0x41,0x1C,
		0xAE,0xED,0x8F,0xFD,0xEC,0xCD,0xCD,0xDD,0x2A,0xAD,0x0B,0xBD,0x68,0x8D,0x49,0x9D,
		0x97,0x7E,0xB6,0x6E,0xD5,0x5E,0xF4,0x4E,0x13,0x3E,0x32,0x2E,0x51,0x1E,0x70,0x0E,
		0x9F,0xFF,0xBE,0xEF,0xDD,0xDF,0xFC,0xCF,0x1B,0xBF,0x3A,0xAF,0x59,0x9F,0x78,0x8F,
		0x88,0x91,0xA9,0x81,0xCA,0xB1,0xEB,0xA1,0x0C,0xD1,0x2D,0xC1,0x4E,0xF1,0x6F,0xE1,
		0x80,0x10,0xA1,0x00,0xC2,0x30,0xE3,0x20,0x04,0x50,0x25,0x40,0x46,0x70,0x67,0x60,
		0xB9,0x83,0x98,0x93,0xFB,0xA3,0xDA,0xB3,0x3D,0xC3,0x1C,0xD3,0x7F,0xE3,0x5E,0xF3,
		0xB1,0x02,0x90,0x12,0xF3,0x22,0xD2,0x32,0x35,0x42,0x14,0x52,0x77,0x62,0x56,0x72,
		0xEA,0xB5,0xCB,0xA5,0xA8,0x95,0x89,0x85,0x6E,0xF5,0x4F,0xE5,0x2C,0xD5,0x0D,0xC5,
		0xE2,0x34,0xC3,0x24,0xA0,0x14,0x81,0x04,0x66,0x74,0x47,0x64,0x24,0x54,0x05,0x44,
		0xDB,0xA7,0xFA,0xB7,0x99,0x87,0xB8,0x97,0x5F,0xE7,0x7E,0xF7,0x1D,0xC7,0x3C,0xD7,
		0xD3,0x26,0xF2,0x36,0x91,0x06,0xB0,0x16,0x57,0x66,0x76,0x76,0x15,0x46,0x34,0x56,
		0x4C,0xD9,0x6D,0xC9,0x0E,0xF9,0x2F,0xE9,0xC8,0x99,0xE9,0x89,0x8A,0xB9,0xAB,0xA9,
		0x44,0x58,0x65,0x48,0x06,0x78,0x27,0x68,0xC0,0x18,0xE1,0x08,0x82,0x38,0xA3,0x28,
		0x7D,0xCB,0x5C,0xDB,0x3F,0xEB,0x1E,0xFB,0xF9,0x8B,0xD8,0x9B,0xBB,0xAB,0x9A,0xBB,
		0x75,0x4A,0x54,0x5A,0x37,0x6A,0x16,0x7A,0xF1,0x0A,0xD0,0x1A,0xB3,0x2A,0x92,0x3A,
		0x2E,0xFD,0x0F,0xED,0x6C,0xDD,0x4D,0xCD,0xAA,0xBD,0x8B,0xAD,0xE8,0x9D,0xC9,0x8D,
		0x26,0x7C,0x07,0x6C,0x64,0x5C,0x45,0x4C,0xA2,0x3C,0x83,0x2C,0xE0,0x1C,0xC1,0x0C,
		0x1F,0xEF,0x3E,0xFF,0x5D,0xCF,0x7C,0xDF,0x9B,0xAF,0xBA,0xBF,0xD9,0x8F,0xF8,0x9F,
		0x17,0x6E,0x36,0x7E,0x55,0x4E,0x74,0x5E,0x93,0x2E,0xB2,0x3E,0xD1,0x0E,0xF0,0x1E
	};
	u_char *checkSum = buf + 0x15;
	int i, index;
	for (i=0; i<0x15; i++)
	{
		index = checkSum[0] ^ buf[i];
		checkSum[0] = checkSum[1] ^ table[index*2+1];
		checkSum[1] = table[index*2];
	}
	for (i=0; i<0x17; i++)
		buf[i] = encode(buf[i]);
}

int fillHeader(UC_BUF *data, const char *nic, unsigned startMode, unsigned dhcpMode, NET_ADDRS *addrs)
{
	u_char *buf = data->buf;
	if (getAddress(nic, startMode, dhcpMode, addrs) == -1)
		return -1;
	memset(buf, 0, data->size);
	buf[0x02] = 0x13;
	buf[0x03] = 0x11;
	if (dhcpMode != 0)
		buf[0x04] = 0x01;		/* DHCP位，使用1、不使用0 */
	memcpy(buf+0x05, &addrs->ip, 4);
	memcpy(buf+0x09, &addrs->mask, 4);
	memcpy(buf+0x0D, &addrs->gateway, 4);
	memcpy(buf+0x11, &addrs->dns, 4);
	checkSum(buf);
	logDebug("## Header: %s\n", printHex(buf, 0x17));
	return 0;
}

static int setVendor(UC_BUF *data, u_char type, const u_char *value, int length)
{
	u_char *p = data->buf + 0x46, *end = data->buf + data->size - length - 8;	/* 形如1a 28 00 00 13 11 17 22，至少8个字节 */
	logDebug("## Vendor type=0x%02x value=%s length=%d\n", type, printHex(value, length), length);
	while (p <= end)	/* 有些老版本没有前两个字节，包括xrgsu */
	{
		if (*p == 0x1a)
			p += 2;
		logDebug("## %s\n", printHex(p, 6+length));
		if (p[4] == type)
		{
			memcpy(p+4+p[5]-length, value, length);
			logDebug("## Set type 0x%02x success: %s\n", type, printHex(p, 4+p[5]));
			return 0;
		}
		p += p[5] + 4;
	}
	logDebug("## Vendor type=0x%02x not found!\n", type);
	return -1;
}

static int readPacket(UC_BUF *data, unsigned dhcpMode, const u_char *localMAC, int type)
{
	u_char dhcp[] = {0x00};
	FILE *fp = fopen(dataFile, "rb");
	if (fp == NULL)
		goto fileError;
	type %= 2;	/* 奇数读Start包，偶数读Md5包 */
	fseek(fp, readSize+(data->size-0x17)*type, SEEK_SET);
	if (fread(data->buf+0x17, data->size-0x17, 1, fp) < 1)	/* 前0x17字节是地址及校验值 */
	{
		fclose(fp);
		goto fileError;
	}
	fclose(fp);
	if (dhcpMode == 1)	/* 认证前第一次 */
		dhcp[0] = 0x01;
	setVendor(data, 0x18, dhcp, 1);
	setVendor(data, 0x2D, localMAC, 6);
	logDebug("## Read %s packet success.\n", type?"MD5":"START");
	return 0;

fileError:
	logEvent("!! 所选数据包%s无效，改用内置数据认证。\n", dataFile);
	dataFile[0] = '\0';
	return -1;
}

void fillStartPacket(UC_BUF *data, unsigned dhcpMode, const u_char *localMAC)
{
	if (dataFile[0] == '\0')	/* 不使用数据包？ */
	{
		u_char packet[] =
		{
			0x00,0x00,0x13,0x11,0x38,0x30,0x32,0x31,0x78,0x2e,0x65,0x78,0x65,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x13,0x11,0x00,0x28,0x1a,
			0x28,0x00,0x00,0x13,0x11,0x17,0x22,0x91,0x66,0x64,0x93,0x67,0x60,0x65,0x62,0x62,
			0x94,0x61,0x69,0x67,0x63,0x91,0x93,0x92,0x68,0x66,0x93,0x91,0x66,0x95,0x65,0xaa,
			0xdc,0x64,0x98,0x96,0x6a,0x9d,0x66,0x00,0x00,0x13,0x11,0x18,0x06,0x02,0x00,0x00
		};
		memcpy(data->buf+0x17, packet, sizeof(packet));
		if (dhcpMode == 1)
			data->buf[0x77] = 0x01;
		else
			data->buf[0x77] = 0x00;
	}
	else if (readPacket(data, dhcpMode, localMAC, 0) == -1)	/* 读取数据失败就用默认的填充 */
		fillStartPacket(data, dhcpMode, localMAC);
}

void fillMd5Packet(UC_BUF *data, unsigned dhcpMode, const u_char *localMAC, const u_char *md5Seed)
{
	if (dataFile[0] == '\0')	/* 不使用数据包？ */
	{
		/* xrgsu的Md5包与Start包只有一个字节的差异，若以其他版本为基础，可进一步区别对待 */
		fillStartPacket(data, dhcpMode, localMAC);
		data->buf[0x3F] = 0x04;
	}
	else if (readPacket(data, dhcpMode, localMAC, 1) == -1
			|| checkRuijie(data, md5Seed) == -1)
		fillMd5Packet(data, dhcpMode, localMAC, md5Seed);
	echoNo = 0x0000102B;	/* 初始化echoNo */
}

static int checkRuijie(UC_BUF *data, const u_char *md5Seed)	/* V2算法 */
{
	FILE *fp = NULL;
	u_char *ruijie = NULL;
	u_char *buf = data->buf;
	int i, j;
	u_char table[144], *md5Dig;
	char md5[33];
	logEvent("** 客户端版本:%d.%d 适用:%s 类型:%d\n", buf[0x3B], buf[0x3C], buf[0x3D]?"Linux":"Windows", buf[0x3E]);
	if (readSize<0x410 || ntohs(*(u_int16_t *)(buf+0x3B))<0x0238)		/* 2.56之前 */
	{
		logEvent("!! 该版本没有客户端校验。\n");	/* 目前V3算法没有攻破，V2算法几乎废掉，所以我们可能仅仅只需要抓包部分，把readSize设小就行了 */
		return 0;
	}
	if ((fp = fopen(dataFile, "rb")) == NULL)
		goto fileError;
	ruijie = (u_char *)malloc(checkSize);
	if (fread(ruijie, readSize, 1, fp) < 1)
	{
		fclose(fp);
		free(ruijie);
		goto fileError;
	}
	fclose(fp);
	for (i=16; i<readSize; i+=16)	/* 还原数据，这是从Windows版的MentoHUST遗留下来的（mpf格式就定义为了这样），如果V3被攻破，倒是可以重新定义一个格式 */
	{
		for (j=0; j<16; j++)
			ruijie[i+j] ^= ruijie[j];
	}
	memset(ruijie+readSize, 0, checkSize-readSize);
	j = (checkSize - 16) / 8;	/* 数据量现在清楚了，下面用Soar的算法开始计算相应MD5值 */
	for (i=0; i<8; i++)	
	{
		memcpy(ruijie + j * i, md5Seed, 16);
		md5Dig = ComputeHash(ruijie + j * i, j + 16);
		table[18*i] = md5Seed[2*i];
		memcpy(table+18*i+1, md5Dig, 16);
		table[18*i+17] = md5Seed[2*i+1];
	}
	free(ruijie);
	md5Dig = ComputeHash(table, 144);
	for (i=0; i<16; i++)
		sprintf(md5+2*i,"%02x", md5Dig[i]);
	logEvent("** MD5校验值:\t%s\n", md5);
	setVendor(data, 0x17, (u_char *)md5, 32);
	return 0;

fileError:
	logEvent("!! 所选数据包%s无效，改用内置数据认证。\n", dataFile);
	dataFile[0] = '\0';
	return -1;
}

void fillEchoPacket(u_char *sendPacket)
{
	int i;
	u_char bt1[4], bt2[4];
	*(u_int32_t *)bt1 = htonl(echoKey + echoNo);
	*(u_int32_t *)bt2 = htonl(echoNo);
	for (i=0; i<4; i++)
	{
		sendPacket[0x18+i] = encode(bt1[i]);
		sendPacket[0x22+i] = encode(bt2[i]);
	}
	logDebug("## Echo: echoNo=%u part1=%s\n", echoNo, printHex(sendPacket+0x18, 4)); 
	echoNo++;
}

void getEchoKey(const u_char *pcapBuf)
{
	int i, offset = 0x1c+pcapBuf[0x1b]+0x69+24;	/* 通过比较了大量抓包，通用的提取点就是这样的 */
	u_char *base;
	echoKey = ntohl(*(u_int32_t *)(pcapBuf+offset));
	base = (u_char *)(&echoKey);
	for (i=0; i<4; i++)
		base[i] = encode(base[i]);
	logDebug("## Original Echo: %s\n", printHex(pcapBuf+offset-6, 15));
	logDebug("## EchoKey=%d [%s]\n", echoKey, printHex(pcapBuf+offset, 4));
}

u_char *checkPass(u_char id, const u_char *md5Seed, const char *password)
{
	u_char md5Src[80], *md5Dig;
	int passLen = strlen(password);
	md5Src[0] = id;
	memcpy(md5Src+1, password, passLen);
	memcpy(md5Src+1+passLen, md5Seed, 16);
	md5Dig = ComputeHash(md5Src, passLen+17);
	logDebug("## ID=0x%02x MD5Seed=%s\n", id, printHex(md5Seed, 16));
	logDebug("## Password MD5=%s\n", printHex(md5Dig, 16));
	return md5Dig;
}
