/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：inireader.h
* 摘	要：读取ini文件
* 作	者：HustMoon@BYHH
*/
#ifndef HUSTMOON_INIREADER_H
#define HUSTMOON_INIREADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define NOT_COMMENT(c)	(c!=';' && c!='#')	/* 不是注释行 */

char *loadFile(const char *fileName);	/* 读取文件 */
int getString(const char *buf, const char *section, const char *key,
	const char *defaultValue, char *value, unsigned long size);	/* 读取字符串 */
int getInt(const char *buf, const char *section, const char *key, int defaultValue);	/* 读取整数 */

#endif

