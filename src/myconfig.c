/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：myconfig.c
* 摘	要：初始化认证参数
* 作	者：HustMoon@BYHH
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#else
static const char *VERSION = "0.2.2";
static const char *PACKAGE_BUGREPORT = "http://code.google.com/p/mentohust/issues/list";
#endif

#include "myconfig.h"
#include "myini.h"
#include "myfunc.h"
#include <pcap.h>
#include <string.h>
#include <arpa/inet.h>

#define ACCOUNT_SIZE		65	/* 用户名密码长度*/
#define NIC_SIZE			60	/* 网卡名最大长度 */
#define MAX_PATH			255	/* FILENAME_MAX */
#define D_TIMEOUT			8	/* 默认超时间隔 */
#define D_ECHOINTERVAL		30	/* 默认心跳间隔 */
#define D_RESTARTWAIT		15	/* 默认重连间隔 */
#define D_STARTMODE			0	/* 默认组播模式 */
#define D_DHCPMODE			0	/* 默认DHCP模式 */

static const char *D_DHCPSCRIPT = "dhclient";	/* 默认DHCP脚本 */
static const char *CFG_FILE = "/etc/mentohust.conf";	/* 配置文件 */
/* const char *LOCK_FILE = "/var/run/mentohust.pid"; */

char userName[ACCOUNT_SIZE] = "";	/* 用户名 */
char password[ACCOUNT_SIZE] = "";	/* 密码 */
char nic[NIC_SIZE] = "";	/* 网卡名 */
char dataFile[MAX_PATH] = "";	/* 数据文件 */
char dhcpScript[MAX_PATH] = "";	/* DHCP脚本 */
u_int32_t ip = 0;	/* 本机IP */
u_int32_t mask = 0;	/* 子网掩码 */
u_int32_t gateway = 0;	/* 网关 */
u_int32_t dns = 0;	/* DNS */
u_char localMAC[6];	/* 本机MAC */
u_char destMAC[6];	/* 服务器MAC */
unsigned timeout = D_TIMEOUT;	/* 超时间隔 */
unsigned echoInterval = D_ECHOINTERVAL;	/* 心跳间隔 */
unsigned restartWait = D_RESTARTWAIT;	/* 失败等待 */
unsigned startMode = D_STARTMODE;	/* 组播模式 */
unsigned dhcpMode = D_DHCPMODE;	/* DHCP模式 */
pcap_t *hPcap = NULL;	/* Pcap句柄 */

static int readFile();	/* 读取配置文件来初始化 */
static void readArg(char argc, char **argv, int *saveFlag);	/* 读取命令行参数来初始化 */
static void showHelp(const char *fileName);	/* 显示帮助信息 */
static int getAdapter();	/* 查找网卡名 */
static void printConfig();	/* 显示初始化后的认证参数 */
static int openPcap();	/* 初始化pcap、设置过滤器 */
static void saveConfig();	/* 保存参数 */

void initConfig(int argc, char **argv)
{
	int saveFlag = 0;	/* 是否需要保存参数 */

	printf("\n欢迎使用MentoHUST\t版本: %s\n"
			"Copyright (C) 2009 HustMoon Studio\n"
			"人到华中大，有甜亦有辣。明德厚学地，求是创新家。\n"
			"Bug report to %s\n\n", VERSION, PACKAGE_BUGREPORT);
	saveFlag = (readFile()==0 ? 0 : 1);
	readArg(argc, argv, &saveFlag);
	if (nic[0] == '\0')
	{
		saveFlag = 1;
		if (getAdapter() == -1)	/* 找不到（第一块）网卡？ */
			exit(-1);
	}
	if (dhcpScript[0] == '\0')	/* 未填写DHCP脚本？ */
		strcpy(dhcpScript, D_DHCPSCRIPT);
	if (userName[0]=='\0' || password[0]=='\0')	/* 未写用户名密码？ */
	{
		printf("?? 请输入用户名: ");
		scanf("%s", userName);
		printf("?? 请输入密码: ");
		scanf("%s", password);
		saveFlag = 1;
	}
	newBuffer();
	printConfig();
	if (fillHeader()==-1 || openPcap()==-1)	/* 获取IP、MAC，打开网卡 */
		exit(-1);
	if (saveFlag)
		saveConfig();
}

static int readFile()
{
	char tmp[16];
	char *buf = loadFile(CFG_FILE);
	if (buf == NULL)
		return -1;
	getString(buf, "MentoHUST", "Username", "", userName, sizeof(userName));
	getString(buf, "MentoHUST", "Password", "", password, sizeof(password));
	getString(buf, "MentoHUST", "Nic", "", nic, sizeof(nic));
	getString(buf, "MentoHUST", "Datafile", "", dataFile, sizeof(dataFile));
	getString(buf, "MentoHUST", "DhcpScript", "", dhcpScript, sizeof(dhcpScript));
	getString(buf, "MentoHUST", "IP", "0.0.0.0", tmp, sizeof(tmp));
	ip = inet_addr(tmp);
	getString(buf, "MentoHUST", "Gateway", "0.0.0.0", tmp, sizeof(tmp));
	gateway = inet_addr(tmp);
	getString(buf, "MentoHUST", "DNS", "0.0.0.0", tmp, sizeof(tmp));
	dns = inet_addr(tmp);
	timeout = getInt(buf, "MentoHUST", "Timeout", D_TIMEOUT) % 100;
	echoInterval = getInt(buf, "MentoHUST", "EchoInterval", D_ECHOINTERVAL) % 1000;
	restartWait = getInt(buf, "MentoHUST", "RestartWait", D_RESTARTWAIT) % 100;
	startMode = getInt(buf, "MentoHUST", "StartMode", D_STARTMODE) % 3;
	dhcpMode = getInt(buf, "MentoHUST", "DhcpMode", D_DHCPMODE) % 4;
	free(buf);
	return 0;
}

static void readArg(char argc, char **argv, int *saveFlag)
{
	char *str, c;
	int i;
	for (i=1; i<argc; i++)
	{
		str = argv[i];
		if (str[0]!='-' && str[0]!='/')
			continue;
		c = str[1];
		if (c=='H' || c=='h' || c=='?')
			showHelp(argv[0]);
		else if (c=='W' || c=='w')
			*saveFlag = 1;
		else if (strlen(str) > 2)
		{
			if (c=='U' || c=='u')
				strncpy(userName, str+2, sizeof(userName)-1);
			else if (c=='P' || c=='p')
				strncpy(password, str+2, sizeof(password)-1);
			else if (c=='N' || c=='n')
				strncpy(nic, str+2, sizeof(nic)-1);
			else if (c=='F' || c=='f')
				strncpy(dataFile, str+2, sizeof(dataFile)-1);
			else if (c=='C' || c=='c')
				strncpy(dhcpScript, str+2, sizeof(dhcpScript)-1);
			else if (c=='I' || c=='i')
				ip = inet_addr(str+2);
			else if (c=='G' || c=='g')
				gateway = inet_addr(str+2);
			else if (c=='S' || c=='s')
				dns = inet_addr(str+2);
			else if (c=='T' || c=='t')
				timeout = atoi(str+2) % 100;
			else if (c=='E' || c=='e')
				echoInterval = atoi(str+2) % 1000;
			else if (c=='R' || c=='r')
				restartWait = atoi(str+2) % 100;
			else if (c=='A' || c=='a')
				startMode = atoi(str+2) % 3;
			else if (c=='D' || c=='d')
				dhcpMode = atoi(str+2) % 4;
		}
	}
}

static void showHelp(const char *fileName)
{
	char *helpString =
		"用法:\t%s [-选项][参数]\n"
		"选项:\t-H 显示本帮助信息\n"
		/* "\t-K 退出程序\n" */
		"\t-W 保存参数到配置文件\n"
		"\t-U 用户名\n"
		"\t-P 密码\n"
		"\t-N 网卡名\n"
		"\t-I IP[默认本机IP]\n"
		"\t-G 网关[默认0.0.0.0]\n"
		"\t-S DNS[默认0.0.0.0]\n"
		"\t-T 认证超时(秒)[默认8]\n"
		"\t-E 响应间隔(秒)[默认30]\n"
		"\t-R 失败等待(秒)[默认15]\n";
	printf(helpString, fileName);
	helpString =
		"\t-A 组播地址: 0(标准) 1(私有) 2(赛尔) [默认0]\n"
		"\t-D DHCP方式: 0(不使用) 1(二次认证) 2(认证后) 3(认证前) [默认0]\n"
		"\t-F 自定义数据文件[默认不使用]\n"
		"\t-C DHCP脚本[默认dhclient]\n"
		"例如:\t%s -Uusername -Ppassword -Neth0 -I192.168.0.1 -G0.0.0.0 -S0.0.0.0 -T8 -E30 -R15 -A0 -D1 -Fdefault.mpf -Cdhclient\n"
		"使用时请确保是以root权限运行！\n\n";
	printf(helpString, fileName);
	exit(0);
}

static int getAdapter()
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
	strncpy(nic, d->name, sizeof(nic)-1);
	pcap_freealldevs(alldevs);
	return 0;
}

static void printConfig()
{
	char *addr[] = {"标准", "私有", "赛尔"};
	char *dhcp[] = {"不使用", "二次认证", "认证后", "认证前"};
	printf("** 用户名:\t%s\n", userName);
	/* printf("## 密码:\t%s\n", password); */
	printf("** 网卡:\t%s\n", nic);
	printf("** 网关地址:\t%s\n", formatIP(gateway));
	printf("** DNS地址:\t%s\n", formatIP(dns));
	printf("** 认证超时:\t%d秒\n", timeout);
	printf("** 响应间隔:\t%d秒\n", echoInterval);
	printf("** 失败等待:\t%d秒\n", restartWait);
	printf("** 组播地址:\t%s\n", addr[startMode]);
	printf("** DHCP方式:\t%s\n", dhcp[dhcpMode]);
	if (dataFile[0] != '\0')
		printf("** 数据文件:\t%s\n", dataFile);
	if (dhcpMode != 0)
		printf("** DHCP脚本:\t%s\n", dhcpScript);
}

static int openPcap()
{
	char buf[PCAP_ERRBUF_SIZE], *fmt;
	struct bpf_program fcode;
	if ((hPcap = pcap_open_live(nic, 65535, 1, 500, buf)) == NULL)
	{
		printf("!! 打开网卡%s失败: %s\n", nic, buf);
		return -1;
	}
	fmt = formatHex(localMAC, 6);
	sprintf(buf, "ether proto 0x888e and not ether src %s and "
		"(ether dst %s or ether dst 01:80:c2:00:00:03)", fmt, fmt);
	if (pcap_compile(hPcap, &fcode, buf, 0, mask) == -1
			|| pcap_setfilter(hPcap, &fcode) == -1)
	{
		printf("!! 设置pcap过滤器失败: %s\n", pcap_geterr(hPcap));
		return -1;
	}
	pcap_freecode(&fcode);
	return 0;
}

static void saveConfig()
{
	char *buf = loadFile(CFG_FILE);
	if (buf == NULL)
	{
		buf = (char *)malloc(1);
		buf[0] = '\0';
	}
	setString(&buf, "MentoHUST", "DhcpScript", dhcpScript);
	setString(&buf, "MentoHUST", "DataFile", dataFile);
	setInt(&buf, "MentoHUST", "DhcpMode", dhcpMode);
	setInt(&buf, "MentoHUST", "StartMode", startMode);
	setInt(&buf, "MentoHUST", "RestartWait", restartWait);
	setInt(&buf, "MentoHUST", "EchoInterval", echoInterval);
	setInt(&buf, "MentoHUST", "Timeout", timeout);
	setString(&buf, "MentoHUST", "DNS", formatIP(dns));
	setString(&buf, "MentoHUST", "Gateway", formatIP(gateway));
	setString(&buf, "MentoHUST", "IP", formatIP(ip));
	setString(&buf, "MentoHUST", "Nic", nic);
	setString(&buf, "MentoHUST", "Password", password);
	setString(&buf, "MentoHUST", "Username", userName);
	if (saveFile(buf, CFG_FILE) != 0)
		printf("!! 保存认证参数到%s失败！\n", CFG_FILE);
	else
		printf("** 认证参数已成功保存到%s.\n", CFG_FILE);
	free(buf);
}
