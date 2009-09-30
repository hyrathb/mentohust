/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：myconfig.c
* 摘	要：初始化认证参数
* 作	者：HustMoon@BYHH
*/
#include "myconfig.h"
#include "inireader.h"
#include <pcap.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#define NIC_SIZE		51	/* 网卡名最大长度 */
#define D_TIMEOUT		10	/* 默认超时间隔 */
#define D_ECHOINTERVAL	30	/* 默认心跳间隔 */

char userName[32] = "";		/* 用户名 */
char password[16] = "";		/* 密码 */
u_char localMAC[6];		/* 本机MAC */
u_int32_t ip = 0;		/* 本机IP */
u_int32_t mask = 0;		/* 子网掩码 */
u_int32_t gateway = 0;		/* 网关 */
u_int32_t dns = 0xFFFFFFFF;	/* DNS */
unsigned timeout = D_TIMEOUT;	/* 超时间隔 */
unsigned echoInterval = D_ECHOINTERVAL;	/* 响应间隔 */
pcap_t *pcapHandle;		/* Pcap句柄 */

static const char *APP_VERSION = "0.1.0";	/* 版本 */
static const char *CFG_FILE = "/etc/mentohust/mentohust.conf";	/* 配置文件 */

static void readFile(char *nic);	/* 读取配置文件来初始化 */
static void readArg(char argc, char **argv, char *nic);	/* 读取命令行参数来初始化 */
static void showHelp(const char *fileName);	/* 显示帮助信息 */
static int getAdapter(char *nic);	/* 查找网卡名 */
static int getAddress(const char *nic);	/* 获取地址 */
static int openPcap(const char *nic);	/* 打开网卡、设置过滤器 */
static void printConfig(const char *nic);	/* 显示初始化后的认证参数 */

void initConfig(int argc, char **argv)
{
	char nic[NIC_SIZE] = "";	/* 网卡名称 */
	printf("\n欢迎使用MentoHUST赛尔版\t版本: %s\n"
			"Copyright (C) 2009 HustMoon Studio\n"
			"人到华中大，有甜亦有辣。明德厚学地，求是创新家。\n\n", APP_VERSION);
	readFile(nic);
	readArg(argc, argv, nic);
	if (userName[0]=='\0' || password[0]=='\0')
	{
		printf("!! 用户名、密码不得为空! 使用参数-?可查看详细参数规则。\n");
		exit(-1);
	}
	if (nic[0]=='\0' && getAdapter(nic)==-1)	/* 找不到（第一块）网卡？ */
		exit(-1);
	if (getAddress(nic)==-1 || openPcap(nic)==-1)	/* 获取IP、MAC，打开网卡失败？ */
		exit(-1);
	printConfig(nic);
}

static void readFile(char *nic)
{
	char tmp[16];
	char *buf = loadFile(CFG_FILE);
	if (buf == NULL)
		return;
	getString(buf, "MentoHUST", "username", "", userName, sizeof(userName));
	getString(buf, "MentoHUST", "password", "", password, sizeof(password));
	getString(buf, "MentoHUST", "nic", "", nic, NIC_SIZE);
	getString(buf, "MentoHUST", "ip", "0.0.0.0", tmp, sizeof(tmp));
	ip = inet_addr(tmp);	/* 掩码需要用来设置过滤器，就不给自定义了，反正没啥用 */
	getString(buf, "MentoHUST", "gateway", "0.0.0.0", tmp, sizeof(tmp));
	gateway = inet_addr(tmp);
	getString(buf, "MentoHUST", "dns", "255.255.255.255", tmp, sizeof(tmp));
	dns = inet_addr(tmp);
	timeout = getInt(buf, "MentoHUST", "timeout", D_TIMEOUT) % 100;
	echoInterval = getInt(buf, "MentoHUST", "echointerval", D_ECHOINTERVAL) % 1000;
	free(buf);
	return;
}

static void readArg(char argc, char **argv, char *nic)
{
	char *str;
	int i;
	for (i=1; i<argc; i++)
	{
		str = argv[i];
		if (str[0]!='-' && str[0]!='/')
			continue;
		if (str[1] == 'h' || str[1] == 'H' || str[1] == '?')
		{
			showHelp(argv[0]);
			exit(0);
		}
		if (strlen(str) > 2)
		{
			switch (str[1])
			{
			case 'U':
			case 'u':
				strncpy(userName, str+2, sizeof(userName)-1);
				break;
			case 'P':
			case 'p':
				strncpy(password, str+2, sizeof(password)-1);
				break;
			case 'N':
			case 'n':
				strncpy(nic, str+2, NIC_SIZE-1);
				break;
			case 'I':
			case 'i':
				ip = inet_addr(str+2);
				break;
			case 'G':
			case 'g':
				gateway = inet_addr(str+2);
				break;
			case 'S':
			case 's':
				dns = inet_addr(str+2);
				break;
			case 'T':
			case 't':
				timeout = atoi(str+2) % 100;
				break;
			case 'E':
			case 'e':
				echoInterval = atoi(str+2) % 1000;
				break;
			default:
				break;
			}
		}
	}
}

static void showHelp(const char *fileName)
{
	char *helpString =
		"用法:\t%s [-选项][参数]\n"
		"选项:\t-H 显示本帮助信息\n"
		"\t-U 用户名[必需]\n"
		"\t-P 密码[必需]\n"
		"\t-N 网卡名[可选]\n"
		"\t-I IP[可选，默认本机IP]\n"
		"\t-G 网关[可选，默认X.X.X.2]\n"
		"\t-S DNS[可选，默认255.255.255.255]\n"
		"\t-T 认证超时(秒)[可选，默认10]\n"
		"\t-E 响应超时(秒)[可选，默认30]\n"
		"例如:\t%s -Uusername -Ppassword -Neth0 -I192.168.0.1 -G192.168.0.2 -S192.168.0.3 -T3 -E30\n"
		"使用时请确保是以root权限运行！\n\n";
	printf(helpString, fileName, fileName);
}

static int getAdapter(char *nic)
{
	pcap_if_t *alldevs, *d;
	int num = 0, avail = 0, i;
	char errbuf[PCAP_ERRBUF_SIZE];
	if (pcap_findalldevs(&alldevs, errbuf)==-1 || alldevs==NULL)
	{
		printf("!! 查找网卡失败: %s\n", errbuf);
		return -1;
	}
	for (d=alldevs; d!=NULL; d=d->next)
	{
		num++;
		if (!(d->flags & PCAP_IF_LOOPBACK) && strcmp(d->name, "any")!=0)
		{
			printf("** 网卡[%d]:\t%s\n", num, d->name);
			avail++;
			i = num;
		}
	}
	if (avail == 0)
	{
		pcap_freealldevs(alldevs);
		printf("!! 找不到网卡！\n");
		return -1;
	}
	if (avail > 1)
	{
		printf("?? 请选择网卡[1-%d]: ", num);
		scanf("%d", &i);
		if (i < 1)
			i = 1;
		else if (i > num)
			i = num;
	}
	printf("** 您选择了第[%d]块网卡。\n", i);
	for (d=alldevs; i>1; d=d->next, i--);
	strncpy(nic, d->name, NIC_SIZE-1);
	pcap_freealldevs(alldevs);
	return 0;
}

static int getAddress(const char *nic)
{
	struct ifreq ifr;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		printf("!! 创建套接字失败!\n");
		return -1;
	}
	strcpy(ifr.ifr_name, nic);
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		close(sock);
		printf("!! 在网卡%s上获取MAC失败!\n", nic);
		return -1;
	}
	memcpy(localMAC, ifr.ifr_hwaddr.sa_data, 6);
	if (ip == 0)
	{
		if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
			printf("!! 在网卡%s上获取IP失败!\n", nic);
		else
			ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
	}
	if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0)
		printf("!! 在网卡%s上获取子网掩码失败!\n", nic);
	else
		mask = ((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr.s_addr;
	if (gateway == 0)
		gateway = ip & 0x00FFFFFF | 0x02000000;	/* 据说赛尔的网关是ip前三字节，后一字节是2 */
	close(sock);
	return 0;
}

static int openPcap(const char *nic)
{
	char buf[PCAP_ERRBUF_SIZE], *fmt;
	struct bpf_program fcode;
	if ((pcapHandle = pcap_open_live(nic, 65535, 1, 500, buf)) == NULL)	/* 赛尔的必须以混杂模式打开网卡 */
	{
		printf("!! 打开网卡%s失败: %s\n", nic, buf);
		return -1;
	}
	fmt = formatHex(localMAC, 6);
	sprintf(buf, "ether proto 0x888e and not ether src %s and "
		"(ether dst %s or ether dst 01:80:c2:00:00:03)", fmt, fmt);
	if (pcap_compile(pcapHandle, &fcode, buf, 0, mask) == -1
			|| pcap_setfilter(pcapHandle, &fcode) == -1)
	{
		printf("!! 设置pcap过滤器失败: %s\n", pcap_geterr(pcapHandle));
		pcap_close(pcapHandle);
		pcapHandle = NULL;
		return -1;
	}
	pcap_freecode(&fcode);
	return 0;
}

static void printConfig(const char *nic)
{
	u_char *p = NULL;
	printf("** 用户名:\t%s\n", userName);
	/* printf("## 密码:\t%s\n", password); */
	printf("** 网卡:\t%s\n", nic);
	printf("** 本机MAC:\t%s\n", formatHex(localMAC, 6));
	p = (u_char *)&ip;
	printf("** 使用IP:\t%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	p = (u_char *)&mask;
	printf("** 子网掩码:\t%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	p = (u_char *)&gateway;
	printf("** 网关地址:\t%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	p = (u_char *)&dns;
	printf("** DNS地址:\t%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	printf("** 认证超时:\t%d秒\n", timeout);
	printf("** 响应超时:\t%d秒\n", echoInterval);
}

char *formatHex(const u_char *buf, int length)
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
