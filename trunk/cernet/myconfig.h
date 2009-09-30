/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：myconfig.h
* 摘	要：初始化认证参数
* 作	者：HustMoon@BYHH
*/
#ifndef HUSTMOON_MYCONFIG_H
#define HUSTMOON_MYCONFIG_H

#include <stdlib.h>

void initConfig(int argc, char **argv);	/* 初始化配置 */
char *formatHex(const u_char *buf, int length);	/* 按十六进制格式化，最多128个字符 */

#endif

