/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：myconfig.c
* 摘	要：初始化认证参数
* 作	者：HustMoon@BYHH
*/
#include "inireader.h"
#include "myconfig.h"

static void readFile(FILE_NAMES *files);	/* 读取配置文件来初始化 */
static void readArg(char argc, char **argv, FILE_NAMES *files);	/* 读取命令行参数来初始化 */
static void showHelp(const char *fileName);	/* 显示帮助信息 */
static int getAdapter();	/* 查找第一块网卡名 */
static void printConfig(FILE_NAMES *files);	/* 显示初始化后的认证参数 */
static int openPcap();	/* 初始化pcap、设置过滤器 */

void initConfig(int argc, char **argv)
{
	FILE_NAMES files = {"", "", 0};
	
	printf("\n欢迎使用MentoHUST\t版本: %s\n"
			"Copyright (C) 2009 HustMoon Studio\n"
			"人到华中大，有甜亦有辣。明德厚学地，求是创新家。\n\n", APP_VERSION);
	readFile(&files);
	readArg(argc, argv, &files);
	setData(&files, &dataPacket);
	if (userName[0]=='\0' || password[0]=='\0')
	{
		logEvent("!! 用户名、密码不得为空! 使用参数-?可查看详细参数规则。\n");
		doExit(-1);
	}
	if (nic[0]=='\0' && getAdapter()==-1)	/* 找不到（第一块）网卡？ */
		doExit(-1);
	printConfig(&files);
	if (fillHeader(&dataPacket, nic, startMode, dhcpMode, &addrs)==-1 || openPcap()==-1)	/* 获取IP、MAC，打开网卡 */
		doExit(-1);
}

static void readFile(FILE_NAMES *files)
{
	char ip[16];
	char *buf = loadFile(CFG_FILE);
	if (buf == NULL)
		return;
	getString(buf, "MentoHUST", "username", "", userName, sizeof(userName));
	getString(buf, "MentoHUST", "password", "", password, sizeof(password));
	getString(buf, "MentoHUST", "nic", "", nic, sizeof(nic));
	getString(buf, "MentoHUST", "package", "", files->data, sizeof(files->data));
	getString(buf, "MentoHUST", "logfile", "", files->log, sizeof(files->log));
	if (files->log[0] != '\0')
		files->mode = 1;
	getString(buf, "MentoHUST", "dhcpscript", "", dhcpScript, sizeof(dhcpScript));
	getString(buf, "MentoHUST", "ip", "0.0.0.0", ip, sizeof(ip));
	addrs.ip = inet_addr(ip);
	getString(buf, "MentoHUST", "gateway", "0.0.0.0", ip, sizeof(ip));
	addrs.gateway = inet_addr(ip);
	getString(buf, "MentoHUST", "dns", "0.0.0.0", ip, sizeof(ip));
	addrs.dns = inet_addr(ip);
	timeout = getInt(buf, "MentoHUST", "timeout", 3) % 100;
	echoInterval = getInt(buf, "MentoHUST", "echointerval", 30) % 1000;
	autoReconnect = getInt(buf, "MentoHUST", "autoreconnect", 0) % 1440;
	startMode = getInt(buf, "MentoHUST", "startaddress", 0) % 2;
	dhcpMode = getInt(buf, "MentoHUST", "dhcpmode", 0) % 4;
	free(buf);
	return;
}

static void readArg(char argc, char **argv, FILE_NAMES *files)
{
	char *str = NULL;
	int i;
	for (i=1; i<argc; i++)
	{
		str = argv[i];
		if (str[0]!='-' && str[0]!='/')
			continue;
		if (str[1] == 'h' || str[1] == 'H' || str[1] == '?')
		{
			showHelp(argv[0]);
			doExit(0);
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
				strncpy(nic, str+2, sizeof(nic)-1);
				break;
			case 'F':
			case 'f':
				strncpy(files->data, str+2, sizeof(files->data)-1);
				break;
			case 'L':
			case 'l':
				strncpy(files->log, str+2, sizeof(files->log)-1);
				if (str[1]=='l')
					files->mode = 1;
				else
					files->mode = 2;
				break;
			case 'C':
			case 'c':
				strncpy(dhcpScript, str+2, sizeof(dhcpScript)-1);
				break;
			case 'I':
			case 'i':
				addrs.ip = inet_addr(str+2);
				break;
			case 'G':
			case 'g':
				addrs.gateway = inet_addr(str+2);
				break;
			case 'S':
			case 's':
				addrs.dns = inet_addr(str+2);
				break;
			case 'T':
			case 't':
				timeout = atoi(str+2) % 100;
				break;
			case 'E':
			case 'e':
				echoInterval = atoi(str+2) % 1000;
				break;
			case 'R':
			case 'r':
				autoReconnect = atoi(str+2) % 1440;
				break;
			case 'A':
			case 'a':
				startMode = atoi(str+2) % 2;
				break;
			case 'D':
			case 'd':
				dhcpMode = atoi(str+2) % 4;
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
		/* "\t-K 退出程序\n" */
		"\t-U 用户名[必需]\n"
		"\t-P 密码[必需]\n"
		"\t-N 网卡名[可选，默认第一块]\n"
		"\t-I IP[可选，默认本机IP]\n"
		"\t-G 网关[可选，默认0.0.0.0]\n"
		"\t-S DNS[可选，默认0.0.0.0]\n"
		"\t-F 自定义数据包[可选，默认不使用]\n"
		/* "\t-L 日志文件[可选，默认不使用]\n" */
		"\t-T 认证超时(秒)[可选，默认3]\n"
		"\t-E 响应间隔(秒)[可选，默认30]\n"
		"\t-R 自动重连(分)[可选，默认关闭]\n"
		"\t-A 组播地址: 0(标准) 1(私有) [可选，默认0]\n";
	printf(helpString, fileName);
	helpString =
		"\t-D DHCP方式: 0(不使用) 1(二次认证) 2(认证后) 3(认证前) [可选，默认0]\n"
		"\t-C DHCP脚本[可选，建议为dhclient]\n"
		"例如:\t%s -Uusername -Ppassword -Neth0 -I192.168.0.1 -G0.0.0.0 -S0.0.0.0 -F/etc/mentohust/default.mpf "/*-L/etc/mentohust/mentohust.log */"-T3 -E30 -R240 -A0 -D1 -Cdhclient\n"
		"使用时请确保是以root权限运行！\n\n";
	printf(helpString, fileName);
}

static int getAdapter()
{
	pcap_if_t *alldevs, *d;
	int num = 0, avail = 0, i;
	char errbuf[PCAP_ERRBUF_SIZE];
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		logEvent("!! 查找网卡失败: %s\n", errbuf);
		return -1;
	}
	for (d=alldevs; d!=NULL; d=d->next)
	{
		num++;
		if (!(d->flags & PCAP_IF_LOOPBACK) && strcmp(d->name, "any")!=0)
		{
			logEvent("** 网卡[%d]:\t%s\n", num, d->name);
			avail++;
			i = num;
		}
		else
			logDebug("## 网卡[%d]:\t%s\n", num, d->name);
	}
	if (avail == 0)
	{
		pcap_freealldevs(alldevs);
		logEvent("!! 找不到网卡！\n");
		return -1;
	}
	if (avail > 1)
	{
		logEvent("?? 请选择网卡[1-%d]: ", num);
		scanf("%d", &i);
		if (i < 1)
			i = 1;
		else if (i > num)
			i = num;
	}
	logEvent("** 您选择了第[%d]块网卡。\n", i);
	for (d=alldevs; i>1; d=d->next, i--);
	strncpy(nic, d->name, sizeof(nic)-1);
	pcap_freealldevs(alldevs);
	return 0;
}

static void printConfig(FILE_NAMES *files)
{
	u_char *p = NULL;
	char *dhcp[] = {"不使用", "二次认证", "认证后", "认证前"};
	logEvent("** 用户名:\t%s\n", userName);
	logDebug("## 密码:\t%s\n", password);
	logEvent("** 网卡:\t%s\n", nic);
	p = (u_char *)(&addrs.gateway);
	logEvent("** 网关地址:\t%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	p = (u_char *)(&addrs.dns);
	logEvent("** DNS地址:\t%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
	if (files->data[0] != '\0')
		logEvent("** 数据包:\t%s\n", files->data);
	if (files->log[0] != '\0')
		logEvent("** 日志:\t%s\n", files->log);
	logEvent("** 认证超时:\t%d秒\n", timeout);
	logEvent("** 响应间隔:\t%d秒\n", echoInterval);
	if (autoReconnect != 0)
	{
		logEvent("** 自动重连:\t%d分钟\n", autoReconnect);
		autoReconnect *= 60;
	}
	logEvent("** 组播地址:\t%s\n", startMode?"私有":"标准");
	logEvent("** DHCP方式:\t%s\n", dhcp[dhcpMode]);
	if (dhcpMode != 0)
		logEvent("** DHCP脚本:\t%s\n", dhcpScript);
}

static int openPcap()
{
	char buf[PCAP_ERRBUF_SIZE];
	struct bpf_program fcode;
	if ((pcapHandle = pcap_open_live(nic, 65535, 0, 500, buf)) == NULL)
	{
		logEvent("!! 打开网卡%s失败: %s\n", nic, buf);
		return -1;
	}
	sprintf(buf, "ether[12:2]=0x888e and ether dst %s", printHex(addrs.lmac, 6));
	logDebug("## Filter: %s\n", buf);
	if (pcap_compile(pcapHandle, &fcode, buf, 0, addrs.mask) == -1
			|| pcap_setfilter(pcapHandle, &fcode) == -1)
	{
		logEvent("!! 设置pcap过滤器失败: %s\n", pcap_geterr(pcapHandle));
		return -1;
	}
	pcap_freecode(&fcode);
	return 0;
}

