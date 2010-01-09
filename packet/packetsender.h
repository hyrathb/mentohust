/*
 * packetsender.h
 *
 *  Created on: 2009-12-9
 *      Author: <microcai AT sina.com > from ZSTU *
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
 */

#ifndef PACKETSENDER_H_
#define PACKETSENDER_H_

#define ETH_MTU 1500
#define ETH_PROTO_8021X 0x888E

#define PKT_PG_HWADDR 1
#define PKT_PG_IPADDR 2
#define PKT_PG_IPMASK 3
#define PKT_PG_DEFAULTROUTE     4
#define PKT_PG_DNS              5


#define HIBYTE(word) (( ((word) & 0xFF00 ) >>8) & 0xFF)
#define LOBYTE(word) ( word & 0xFF)

int open_lib();

int pkt_first_link(char *nic);
int pkt_open_link(const char * _nic_name);
int pkt_build_ruijie(int lengh,const char* ruijiedata);
int pkt_build_8021x_ext(u_char code, u_char id, uint16_t length,const char* extra);
int pkt_build_8021x(u_char version, u_char type, uint16_t length,void*paylod,int payloadlen);
int pkt_build_ethernet(u_char*dest,u_char*src,uint16_t protocol);
int pkt_write_link();
int pkt_read_link(const u_char**packet);
int pkt_close();


#endif /* PACKETSENDER_H_ */
