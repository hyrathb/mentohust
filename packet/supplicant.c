/*
 * supplicant.c : the ruijie supplicant auth engine
 *
 *  Created on: 2009-12-11
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdint.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include "packetsender.h"

#include "supplicant.h"

#define EAP_START       1

#include "Tables.h"

static uint8_t   ruijie_privatedata[]={
    // OEM Extra
    // 0 --> 22
      0xff,0xff,0x37,0x77, // Encode( 0x00,0x00,0x13,0x11 )
                           // 求反并头尾颠倒.add by lsyer
      0xff,                // Encode( 0x01/00  EnableDHCP flag )
                           // 0xff:Static IP  0x3f:DHCP
      0x00,0x00,0x00,0x00, // Encode( IP )
      0x00,0x00,0x00,0x00, // Encode( SubNetMask )
      0x00,0x00,0x00,0x00, // Encode( NetGate )
      0x00,0x00,0x00,0x00, // Encode( DNS )
      0x00,0x00,           // Checksum( )
    // 23 --> 58
    // ASCII 8021x.exe
      0x00,0x00,0x13,0x11,0x38,0x30,0x32,0x31,0x78,0x2E,0x65,0x78,0x65,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,
    // 59 --> 62
      0x00,0x00,0x00,0x02, // 8021x.exe File Version (2.56.00)
      //63                 // base16 code.add by lsyer
      0x00,                // unknow flag
      //64 --> 77     // 01 0x33
      0x00,0x00,0x13,0x11,0x00,0x28,0x1A,0x28,0x00,0x00,0x13,0x11,0x17,0x22,// Const strings
       // 78 --> 109
      // 32bits spc. Random strings
      0x46,0x38,0x42,0x35,0x32,0x42,0x42,0x37,0x44,0x38,0x32,0x36,0x41,0x34,0x33,0x32,
    //    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      // 32bits spc. Random strings
      0x34,0x43,0x30,0x30,0x30,0x33,0x38,0x38,0x34,0x39,0x38,0x36,0x33,0x39,0x66,0x34,
    //  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,

      //110 --> 120
      0x1a,0x0c, // unknow flag
      0x00,0x00,0x13,0x11,0x18,0x06,0x00,0x00,0x00,// Const strings
      // 121
      0x00,               // DHCP and first time flag

      // V2.56 (and upper?) added
      // 122 --> 129
      0x1A,0x0E,0x00,0x00,0x13,0x11,0x2D,0x08,  // Const strings

      // 130 --> 143
      0x00,0x00,0x00,0x00,0x00,0x00,            // True NIC MAC
      0x1A,0x08,0x00,0x00,0x13,0x11,0x2F,0x02,  // Const strings

      //u_int8_t ruijie_new_extra[]=
      //144 --> 151
      0x1A,0x09,0x00,0x00,0x13,0x11,0x35,0x03,  // Const strings
      //152
      0x00, //unknow
      //153 --> 160
      0x1a,0x18,0x00,0x00,0x13,0x11,0x36,0x12,   // Const strings
      //161 --> 176
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      //177 --> 183
      0x1A,0x18,0x00,0x00,0x13,0x11,0x38,0x12,  // Const strings
      //184 --> 199
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      //200 --> 207
      0x1A,0x18,0x00,0x00,0x13,0x11,0x4d,0x82,  // Const strings


    /************************************************************************************
     WE DON'T KNOW IT
     ************************************************************************************/

      //208 --> 239
      // 32bits spc. Random strings
      0x30,0x66,0x38,0x62,0x35,0x32,0x62,0x62,0x30,0x37,0x64,0x38,0x30,0x32,0x36,0x61,
    //0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      // 32bits spc. Random strings
      0x34,0x33,0x32,0x34,0x33,0x35,0x30,0x66,0x33,0x32,0x62,0x37,0x33,0x30,0x37,0x34,
    //  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      //240
      0x34,0x30,0x33,0x61,0x36,0x65,0x35,0x39,0x32,0x32,0x37,0x62,0x30,0x36,0x38,0x61,
    //0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      // 32bits spc. Random strings
      0x36,0x35,0x32,0x64,0x34,0x34,0x34,0x32,0x32,0x33,0x62,0x34,0x30,0x36,0x33,0x63,
    //  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      //272
      0x33,0x37,0x33,0x63,0x34,0x38,0x65,0x31,0x35,0x61,0x62,0x32,0x37,0x32,0x65,0x66,
    //    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      // 32bits spc. Random strings
    //  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      //304
      0x31,0x62,0x35,0x36,0x30,0x30,0x33,0x61,0x36,0x32,0x66,0x38,0x34,0x33,0x36,0x64,
    //    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      // 32bits spc. Random strings
      0x31,0x31,0x32,0x63,0x37,0x66,0x30,0x34,0x30,0x32,0x65,0x65,0x35,0x36,0x63,0x64,
    //  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      0x32,0x34,0x38,0x62,0x36,0x36,0x62,0x30,0x34,0x37,0x64,0x36,0x37,0x30,0x39,0x37,
      //336
      0x1A,0x28,0x00,0x00,0x13,0x11,0x39,0x22   // Const strings

      //352
  };

static u_char           circleCheck[2];
static u_char           ruijie_dest[6];
static uint32_t         ruijie_Echo_Key;
static uint32_t         ruijie_Echo_Diff;
static const u_char* 	ruijie_recv;

static int gen_ruijie_private_packet(int mode,char*version)
{
  int iCircle = 0x15;
  int i, ax = 0, bx = 0, dx = 0;

  unsigned int c_ver1, c_ver2;
  struct sockaddr so_addr;
  u_char sCircleBase[0x15] =
    {
        0x00, 0x00, 0x13, 0x11, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

  ruijie_privatedata[0x3B] = 0x01;
  ruijie_privatedata[0x3C] = 0x33;

  if (version && sscanf(version, "%u.%u", &c_ver1, &c_ver2))
    {
      ruijie_privatedata[0x3B] = c_ver1;
      ruijie_privatedata[0x3C] = c_ver2;
    }
  if( c_ver1 >=3 && c_ver2 >= 50)
    ruijie_Echo_Key = htonl(0x0000102b);

  pkt_get_param(PKT_PG_HWADDR, &so_addr);
  memcpy(ruijie_privatedata + 130, so_addr.sa_data, 6);

  uint8_t * ForFill = ruijie_privatedata + 0x05;

  pkt_get_param(PKT_PG_IPADDR, &so_addr);

  ForFill[0] = Alog(so_addr.sa_data[2]);
  ForFill[1] = Alog(so_addr.sa_data[3]);
  ForFill[2] = Alog(so_addr.sa_data[4]);
  ForFill[3] = Alog(so_addr.sa_data[5]);

  if (mode & RUIJIE_AUTHMODE_DHCP)//Dhcp Enabled
    {
      sCircleBase[0x04] = 0x01;
      ruijie_privatedata[0x04] = 0x7f;
      if(mode & RUIJIE_AUTHMODE_NOIP)
    	ruijie_privatedata[0x79] = 1;
      else
    	ruijie_privatedata[0x79]= 0 ;
      //dhcpstate & 1; // 1 if first auth in dhcp mode
    }
  else
    {
      sCircleBase[0x04] = 0x00;
      memcpy(sCircleBase + 5, so_addr.sa_data + 2, 4);
    }

  pkt_get_param(PKT_PG_IPMASK, &so_addr);

  ForFill[4] = Alog(so_addr.sa_data[2]);
  ForFill[5] = Alog(so_addr.sa_data[3]);
  ForFill[6] = Alog(so_addr.sa_data[4]);
  ForFill[7] = Alog(so_addr.sa_data[5]);

  memcpy(sCircleBase + 9, so_addr.sa_data + 2, 4);

  pkt_get_param(PKT_PG_DEFAULTROUTE, &so_addr);

  ForFill[8] = Alog(so_addr.sa_data[2]);
  ForFill[9] = Alog(so_addr.sa_data[3]);
  ForFill[10] = Alog(so_addr.sa_data[4]);
  ForFill[11] = Alog(so_addr.sa_data[5]);

  memcpy(sCircleBase + 13, so_addr.sa_data + 2, 4);

  pkt_get_param(PKT_PG_DNS, &so_addr);

  ForFill[12] = Alog(ForFill[12]);
  ForFill[13] = Alog(ForFill[13]);
  ForFill[14] = Alog(ForFill[14]);
  ForFill[15] = Alog(ForFill[15]);

  memcpy(sCircleBase + 17, so_addr.sa_data + 2, 4);

  for (i = 0; i < iCircle; i++)
    {
      dx = ax;
      bx = 0;
      bx = (bx & 0xff00) | sCircleBase[i]; // add "( )" by cdx
      dx &= 0xffff;
      dx >>= 8;
      dx ^= bx;
      bx = 0;
      bx &= 0x00ff;
      bx |= (ax & 0xff) << 8;

      ax = Table[dx * 2] | Table[dx * 2 + 1] << 8;
      ax ^= bx;
    }
  circleCheck[0] = (unsigned char) ((ax & 0xff00) >> 8);
  circleCheck[1] = (unsigned char) (ax & 0x00ff);

  ForFill[16] = Alog(circleCheck[0]);
  ForFill[17] = Alog(circleCheck[1]);

  return 0;
}

static int ruijie_start(int broadcastmethod)
{
  u_char broadcast[2][6]=
    {
        { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x03 }, // standard broadcast addr
        { 0x01, 0xD0, 0xF8, 0x00, 0x00, 0x03 } // ruijie private broadcast addr
    };
  const  u_char *       packet;

  pkt_build_start();
  pkt_build_ruijie(sizeof(ruijie_privatedata),ruijie_privatedata);
  pkt_build_8021x(1,EAP_START,4,0,0);
  pkt_build_ethernet(broadcast[broadcastmethod?1:0],0,ETH_PROTO_8021X);
  return pkt_write_link();
}

static int ruijie_ack_name(int id,char*name)
{
  char          payload[128];
  int           payload_len;
  payload[127] = 0;
  payload[0] = 1; // EAP type
  payload_len = strlen(name)+1;

  pkt_build_start();

  pkt_build_ruijie(sizeof(ruijie_privatedata),ruijie_privatedata);
  strncpy(payload + 1, name, 128);
  pkt_build_8021x_ext(EAP_RESPONSE,id,payload_len,payload);
  pkt_build_8021x(1,1,payload_len+4,0,0);
  pkt_build_ethernet(ruijie_dest,0,ETH_PROTO_8021X);
  return pkt_write_link();
}

static int ruijie_ack_password(int id,char*name,char*passwd,const u_char* MD5value, int MD5value_len)
{
  char          EAP_EXTRA[128];
  unsigned char md5Data[256]=""; // password,md5 buffer
  unsigned char md5Dig[32]; // result of md5 sum

  int md5Len = 0;

  int passwordLen = strlen(passwd);

  md5Data[md5Len++] = id;//ID
  memcpy(md5Data + md5Len, passwd, passwordLen);
  md5Len += passwordLen; // password

  memcpy(md5Data + md5Len, MD5value, MD5value_len);
  md5Len += MD5value_len; // private key

  Computehash(md5Data, md5Len,md5Dig);

  pkt_build_start();

  pkt_build_ruijie(sizeof(ruijie_privatedata),ruijie_privatedata);
  EAP_EXTRA[0] = 4 ;// Type: MD5-Challenge [RFC3748] (4)
  EAP_EXTRA[1] = 16 ; //Value-Size: 16
  memcpy(EAP_EXTRA+2,md5Dig,16); // md5 encrypt passwd
  strcpy(EAP_EXTRA+18,name);//user name
  pkt_build_8021x_ext(EAP_RESPONSE,id,22+strlen(name),EAP_EXTRA);
  pkt_build_8021x(1,0,22+strlen(name),0,0);
  pkt_build_ethernet(ruijie_dest,0,ETH_PROTO_8021X);
  return pkt_write_link();
}

static int ruijie_ripe_success_info()
{
  //get 心跳信息初始码
  //uTemp.ulValue = *(((u_long *)(pkt_data+0x9d)));
  u_int16_t offset;

  offset = MAKEWORD(ruijie_recv[0x10],ruijie_recv[0x11]);

  union
  {
    u_int32_t l;
    u_char s[4];
  } tmp;

  tmp.s[0] = Alog(ruijie_recv[offset + 0x9]);//0xff
  tmp.s[1] = Alog(ruijie_recv[offset + 0xa]);//0xff
  tmp.s[2] = Alog(ruijie_recv[offset + 0xb]);//0x19
  tmp.s[3] = Alog(ruijie_recv[offset + 0xc]);//0x09

  ruijie_Echo_Diff = ntohl(tmp.l);

  return 0;
}

int ruijie_echo()
{

  union
  {
    u_int32_t l;
    u_char s[4];

  } tmp;

  char  EchoData[30] = {
      0xFF,0xFF,0x37,0x77,0x7F,0x9F,0xF7,0xFF,0x00,0x00,0xFF,0xFF,0x37,0x77,0x7F,
      0x9F,0xF7,0xFF,0x00,0x00,0xFF,0xFF,0x37,0x77,0x7F,0x3F,0xFF,0x00,0x00,0x00
  };

  tmp.l = ruijie_Echo_Diff;

  EchoData[0x10] = Alog(tmp.s[0]);
  EchoData[0x11] = Alog(tmp.s[1]);
  EchoData[0x12] = Alog(tmp.s[2]);
  EchoData[0x13] = Alog(tmp.s[3]);

  tmp.l = htonl(ntohl(ruijie_Echo_Key) + ruijie_Echo_Diff);

  EchoData[6] = Alog(tmp.s[0]);
  EchoData[7] = Alog(tmp.s[1]);
  EchoData[8] = Alog(tmp.s[2]);
  EchoData[9] = Alog(tmp.s[3]);
  pkt_build_start();
  pkt_build_8021x(1, 191, 30,EchoData,30);
  pkt_build_ethernet(ruijie_dest,0,ETH_PROTO_8021X);
  ruijie_Echo_Key = htonl(ntohl(ruijie_Echo_Key) + 1);
  return pkt_write_link();
}

static int ruije_logoff()
{
  pkt_build_start();
  pkt_build_8021x(1,2,0,0,0);
  pkt_build_ethernet(ruijie_dest,0,ETH_PROTO_8021X);
  pkt_write_link();
  pkt_close();
  return 0;
}

int ruijie_start_auth(char * name, char*passwd, char* nic_name, int authmode,
	int (*authprogress)(int reason, const char * current_packet, void*userptr),
	    void * userptr)
{
  if(open_lib())
	return -1;

  if(!(authmode & RUIJIE_AUTHMODE_NOECHOKEY))
	ruijie_Echo_Key = htonl(0x1b8b4563);

  char * msg, *utf8_msg;
  int msg_len;

  int success=1,tryed=0;

  if(pkt_open_link(nic_name))
	{
	  fprintf(stderr,"%s",pkt_lasterr());
	  return-1;
	}
  gen_ruijie_private_packet(authmode, "3.33");
  ruijie_start(authmode & 0x1F);

  if (authprogress(RUIJIE_AUTH_FINDSERVER, 0, userptr)) return -1;

  do
    {
      while (!pkt_read_link(&ruijie_recv) && ruijie_recv )
        {
          switch ( ruijie_recv[0x12])
            {
          case EAP_FAILED:
        	authprogress(RUIJIE_AUTH_FAILED,ruijie_recv,userptr);
        	break;
          case EAP_RESPONSE:
            switch (ruijie_recv[0x16])
              {
            case 1: //Type: Identity [RFC3748] (1)
              if (authprogress(RUIJIE_AUTH_NEEDNAME, ruijie_recv, userptr)) return -1;
              ruijie_ack_name(ruijie_recv[0x13], name);
              break;
            case 4://Type: MD5-Challenge [RFC3748] (4)
            default:
              if(authprogress(RUIJIE_AUTH_NEEDPASSWD,ruijie_recv,userptr)) return -1;
              ruijie_ack_password(ruijie_recv[0x13], name, passwd, ruijie_recv + 0x18, ruijie_recv[0x17]);
              break;
              }
            break;
          case EAP_SUCCESS:
            ruijie_ripe_success_info();
            if (authprogress(RUIJIE_AUTH_SUCCESS, ruijie_recv, userptr)) return -1;
            success = 0;
            break;
            }
        }
      tryed += success;
      ruijie_start(authmode & 0x1F);
    }
  while (success && tryed < 5);
  return success;
}

int ruijie_stop_auth()
{
  return ruije_logoff();
}

/*
 * raw GBK message, return message length
 */
int ruijie_get_server_msg( char * raw_encode_message_out,int length)
{
  size_t len = ntohs(*((u_int16_t*) (ruijie_recv + 0x10))) - 10;

  if (length < len)
    // space allocation of buffer exceeded
    return -1;

  if (len < 0)
    // did not retrieve any messages from sever.
    return 0;
  char *msgBuf = (typeof(msgBuf)) (ruijie_recv + 0x1c);

  //remove the leading "\r\n" which seems always exist!
  if (len > 3 && (msgBuf[0] == 0xd) && (msgBuf[1] == 0xa))
    {
      msgBuf += 2;
    }
  strcpy(raw_encode_message_out,msgBuf);
  return strlen(raw_encode_message_out);
}

