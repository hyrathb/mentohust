/*
 * packetsender.c : wrapper of libpcap , send and receive packages *
 *
 *  Created on: 2009-12-9
 *      Author: microcai <microcai AT sina.com > from ZSTU
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <pcap/pcap.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef HAVE_NET_IF_DL_H
#include <sys/param.h>
#include <net/if_dl.h>
#define _OS_BSD_ BSD
#else

#endif

#include "packetsender.h"

/**************************************
 * Define pcap_* macro , to use or no to use static link
 **************************************/
pcap_t * (*ppcap_open_live)(const char *, int, int, int, char *);
int (*ppcap_fileno)(pcap_t *);
char * (*ppcap_geterr)(pcap_t *);

//The BPF
int (*ppcap_compile)(pcap_t *, struct bpf_program *, const char *, int,
		bpf_u_int32);
int (*ppcap_setfilter)(pcap_t *, struct bpf_program *);
void (*ppcap_freecode)(struct bpf_program *);

int (*ppcap_sendpacket)(pcap_t *, const u_char *, int);
void (*ppcap_close)(pcap_t *);
int (*ppcap_next_ex)(pcap_t *, struct pcap_pkthdr **, const u_char **);

/**************************************
 * static variable here
 **************************************/
static const char FILTER_STR[] =
		"ether[12:2]=0x888e and ether dst %02x:%02x:%02x:%02x:%02x:%02x";
static pcap_t * pcap_handle;
static char pcap_errbuf[PCAP_ERRBUF_SIZE];
static in_addr_t nic_ip, nic_mask;
static char nic_name[PCAP_ERRBUF_SIZE];
static char nic_hwaddr[6];
static char pkt_buffer[ETH_MTU];
static int pkt_length;
static char pkt_errbuff[PCAP_ERRBUF_SIZE];

/*
 * open libpcap.so.*
 */
#ifdef USE_DYLIBPCAP
#include <dlfcn.h>
#endif

int open_lib()
{
#ifndef USE_DYLIBPCAP
	return 0;
#else
	char libpcap[4][32]=
	{
		{	"libpcap.so.1"},
		{	"libpcap.so.0.9"},
		{	"libpcap.so.0.8"},
		{	"libpcap.so.0.7"},
	};
	void *plibpcap;
	int index = 0;

	for(index = 0; index < 4;index ++ )
	{
		plibpcap = dlopen(libpcap[index],RTLD_NOW);
		if(plibpcap)
		break;
	}

	if(!plibpcap)
	{
		fprintf(stderr,"Err loading libpcap.so. Please install libpcap\n");
		return -1;
	}
	// Now bind the fucntion

	ppcap_open_live = dlsym(plibpcap,"pcap_open_live");
#define pcap_open_live(z,x,c,v,b) ppcap_open_live(z,x,c,v,b)

	ppcap_fileno = dlsym(plibpcap,"pcap_fileno");
#define pcap_fileno(x) ppcap_fileno(x)

	ppcap_geterr = dlsym(plibpcap,"pcap_geterr");
#define pcap_geterr(x) ppcap_geterr(x)

	ppcap_compile = dlsym(plibpcap,"pcap_compile");
#define pcap_compile(z,x,c,v,b) ppcap_compile(z,x,c,v,b)

	ppcap_setfilter = dlsym(plibpcap,"pcap_setfilter");
#define pcap_setfilter(x,c) ppcap_setfilter(x,c)

	ppcap_freecode = dlsym(plibpcap,"pcap_freecode");
#define pcap_freecode(x) ppcap_freecode(x)

	ppcap_next_ex = dlsym(plibpcap,"pcap_next_ex");
#define pcap_next_ex(z,x,c) ppcap_next_ex(z,x,c)

	ppcap_sendpacket = dlsym(plibpcap,"pcap_sendpacket");
#define pcap_sendpacket(z,x,c) ppcap_sendpacket(z,x,c)

	ppcap_close = dlsym(plibpcap,"pcap_close");
#define pcap_close(x) ppcap_close(x)

#endif
	return 0;
}

char* pkt_lasterr()
{
	return pkt_errbuff;
}

int pkt_first_link(char nic[])
{

	struct ifaddrs *ifaddr, *ifaddr_org;
	getifaddrs(&ifaddr);
	ifaddr_org = ifaddr;
	while (ifaddr->ifa_flags & IFF_LOOPBACK)
	{
		ifaddr = ifaddr->ifa_next;
	}
	//  printf("using nic %s\n", ifaddr->ifa_name);//,ifaddr->ifa_next);
	strcpy(nic, ifaddr->ifa_name);
	freeifaddrs(ifaddr_org);
	return 0;
}

int pkt_open_link(const char * _nic_name)
{
	struct bpf_program filter_code;
	char filter_buf[256];

	strncpy(nic_name, _nic_name, PCAP_ERRBUF_SIZE);

	if (pcap_handle)
		pcap_close(pcap_handle);

	if (!(pcap_handle = pcap_open_live(nic_name, 65536, 0, 2000, pcap_errbuf)))
	{
		snprintf(pkt_errbuff, sizeof(pkt_errbuff), "Cannot open nic %s :%s",
				nic_name, pcap_errbuf);
		return -1;
	}

	snprintf(filter_buf, sizeof(filter_buf), FILTER_STR, nic_hwaddr[0],
			nic_hwaddr[0], nic_hwaddr[0], nic_hwaddr[0], nic_hwaddr[0],
			nic_hwaddr[0]);

	if (pcap_compile(pcap_handle, &filter_code, filter_buf, 0, nic_mask) == -1)
	{
		fprintf(stderr, "pcap_compile(): %s", pcap_geterr(pcap_handle));
		pcap_close(pcap_handle);
		return 1;
	}
	if (pcap_setfilter(pcap_handle, &filter_code) == -1)
	{
		fprintf(stderr, "pcap_setfilter(): %s", pcap_geterr(pcap_handle));
		pcap_close(pcap_handle);
		return 1;
	}
	pcap_freecode(&filter_code); // avoid  memory-leak

	return (0);
}

int pkt_get_param(int what, struct sockaddr * sa_data)
{
	static int initialized = 0;
#ifdef HAVE_NET_IF_DL_H
	struct ifaddrs * pifaddrs, *pifaddr;
#else
	struct ifreq rif;
#endif

	if (!initialized)
	{
#ifndef SIOCGIFHWADDR
		if (!getifaddrs(&pifaddrs))
		{
			for (pifaddr = pifaddrs; pifaddr; pifaddr = pifaddr->ifa_next)
			{
				if (pifaddr->ifa_name && pifaddr->ifa_name[0] && !strcmp(
								(const char*) pifaddr->ifa_name, nic_name))
				{
					nic_ip = ((struct sockaddr_in*) pifaddr->ifa_addr)->sin_addr.s_addr;
					nic_mask = ((struct sockaddr_in*) pifaddr->ifa_netmask)->sin_addr.s_addr;

					const struct sockaddr_dl * sdl = (struct sockaddr_dl*) pifaddr->ifa_addr;
					memcpy(nic_hwaddr, sdl->sdl_data + sdl->sdl_nlen, 6);
					break;
				}
			}
			freeifaddrs(pifaddrs);
		}
		else
		{
			snprintf(pkt_errbuff,sizeof(pkt_errbuff),"cannot get nic:%s paramters",nic_name);
			return -1;
		}
#else
		int sock = socket(AF_INET, SOCK_DGRAM, 0);

		if (!ioctl(sock, SIOCGIFHWADDR, &rif))
		{
			memcpy(nic_hwaddr, rif.ifr_hwaddr.sa_data, 6);
		}
		else
		{
			snprintf(pkt_errbuff, sizeof(pkt_errbuff),
					"Err getting %s address\n", nic_name);
			close(sock);
			return -1;
		}

		if (!ioctl(sock, SIOCGIFADDR, &rif))
			memcpy(&nic_ip, rif.ifr_addr.sa_data + 2, 4);

		if (!ioctl(sock, SIOCGIFNETMASK, &rif))
			memcpy(&nic_mask, rif.ifr_addr.sa_data + 2, 4);
		else
			nic_mask = inet_addr("255.255.255.0");
		close(sock);
#endif
	}

	((struct sockaddr_in*) sa_data)->sin_family = PF_INET;
	switch (what)
	{
	case PKT_PG_HWADDR:
		sa_data->sa_family = 0;
		memcpy(sa_data->sa_data, nic_hwaddr, 6);
		break;
	case PKT_PG_IPADDR:
		((struct sockaddr_in*) sa_data)->sin_family = AF_INET;
		((struct sockaddr_in*) sa_data)->sin_addr.s_addr = nic_ip;
		break;
	case PKT_PG_IPMASK:
		((struct sockaddr_in*) sa_data)->sin_family = PF_INET;
		((struct sockaddr_in*) sa_data)->sin_addr.s_addr = nic_mask;
		break;
	case PKT_PG_DEFAULTROUTE:
		((struct sockaddr_in*) sa_data)->sin_family = PF_INET;
		((struct sockaddr_in*) sa_data)->sin_addr.s_addr = nic_mask;
		sa_data->sa_data[3] = 1;
		break;
	case PKT_PG_DNS:
	{
		FILE * fresolv;
		char line[128];
		int nameserver_ip[4] =
		{ 0 };

		((struct sockaddr_in*) sa_data)->sin_family = PF_INET;

		if ((fresolv = fopen("/etc/resolv.conf", "r")))
		{
			while (fgets(line, 80, fresolv) > 0)
				if (sscanf(line, "nameserver%d.%d.%d.%d", nameserver_ip,
						nameserver_ip + 1, nameserver_ip + 2, nameserver_ip + 3))
					break;

			sa_data->sa_data[2] = nameserver_ip[0];
			sa_data->sa_data[3] = nameserver_ip[1];
			sa_data->sa_data[4] = nameserver_ip[2];
			sa_data->sa_data[5] = nameserver_ip[3];

			fclose(fresolv);
		}
		else
		{
			((struct sockaddr_in*) sa_data)->sin_addr.s_addr = 0;
		}
	}
		break;
	default:
		return -1;
	}
	return 0;
}

int pkt_build_start()
{
	pkt_length = 0;
	return 0;
}

int pkt_build_ruijie(int lengh, const char* ruijiedata)
{
	memmove(pkt_buffer + lengh, pkt_buffer, pkt_length);
	memcpy(pkt_buffer, ruijiedata, lengh);
	pkt_length += lengh;
	return 0;
}

int pkt_build_8021x_ext(u_char code, u_char id, uint16_t length,
		const char* extra)
{
	memmove(pkt_buffer + length, pkt_buffer, pkt_length);
	pkt_buffer[0] = code;
	pkt_buffer[1] = id;
	pkt_buffer[2] = HIBYTE(length);
	pkt_buffer[3] = LOBYTE(length);
	memcpy(pkt_buffer + 4, extra, length);
	pkt_length += length;
	return 0;
}

int pkt_build_8021x(u_char version, u_char type, uint16_t length, void*paylod,
		int payloadlen)
{
	memmove(pkt_buffer + 4 + payloadlen, pkt_buffer, pkt_length);
	pkt_buffer[0] = version;
	pkt_buffer[1] = type;
	pkt_buffer[2] = HIBYTE(length);
	pkt_buffer[3] = LOBYTE(length);
	memcpy(pkt_buffer + 4, paylod, payloadlen);
	pkt_length += 4 + payloadlen;
	return 0;
}

int pkt_build_ethernet(u_char*dest, u_char*src, uint16_t protocol)
{
	//we make sure that
	memmove(pkt_buffer + 14, pkt_buffer, pkt_length);
	memcpy(pkt_buffer, dest, 6);
	if (!src)
		src = (typeof(src)) nic_hwaddr;
	memcpy(pkt_buffer + 6, src, 6);
	pkt_buffer[12] = HIBYTE(protocol);
	pkt_buffer[13] = LOBYTE(protocol);
	pkt_length += 14;
	return 0;
}

int pkt_write_link()
{
	return pcap_sendpacket(pcap_handle, (const u_char*) pkt_buffer, pkt_length);
}

int pkt_read_link(const u_char**packet)
{
	struct pcap_pkthdr * pkt_hdr;
	int ret;
	ret = pcap_next_ex(pcap_handle, &pkt_hdr, packet);
	return ret;
}

int pkt_close()
{
	pcap_close(pcap_handle);
	return 0;
}
