/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：inireader.c
* 摘	要：读取ini文件
* 作	者：HustMoon@BYHH
*/
#include "inireader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define NOT_COMMENT(c)	(c!=';' && c!='#')	/* 不是注释行 */

#ifndef strnicmp
#define strnicmp strncasecmp
#endif
/*int strnicmp(const char *str1, const char *str2, size_t count);*/
static void getLine(const char *buf, int inStart, int *lineStart, int *lineEnd);
static int findKey(const char *buf, const char *section, const char *key,
	int *sectionEnd, int *valueStart, unsigned long *valueSize);

char *loadFile(const char *fileName)
{
	FILE *fp = NULL;
	long size = 0;
	char *buf = NULL;
	assert(fileName != NULL);
	if ((fp=fopen(fileName, "rb")) == NULL)
		return NULL;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	buf = (char *)malloc(size+1);
	buf[size] = '\0';
	if (fread(buf, size, 1, fp) < 1)
	{
		free(buf);
		buf = NULL;
	}
	fclose(fp);
	return buf;
}

/*int strnicmp(const char *str1, const char *str2, size_t count)
{
	int a1, a2;
	size_t i;
	assert(str1 != NULL);
	assert(str2 != NULL);
	assert(count > 0);
	for (i=0; i<count && str1[i]!='\0' && str2[i]!='\0'; i++)
	{
		a1 = toupper(str1[i]);
		a2 = toupper(str2[i]);
		if (a1 != a2)
			return (a1 < a2) ? -1 : 1;
	}
	a1 = strlen(str1);
	a2 = strlen(str2);
	return (i==count || a1==a2) ? 0 : ((a1 < a2) ? -1 : 1);
}*/

static void getLine(const char *buf, int inStart, int *lineStart, int *lineEnd)
{
	int start, end;
	for (start=inStart; buf[start]==' ' || buf[start]=='\t' || buf[start]=='\r' || buf[start]=='\n'; start++);
	for (end=start; buf[end]!='\r' && buf[end]!='\n' && buf[end]!='\0'; end++);
	*lineStart = start;
	*lineEnd = end;
}

static int findKey(const char *buf, const char *section, const char *key,
	int *sectionEnd, int *valueStart, unsigned long *valueSize)
{
	int lineStart, lineEnd, i;
	for (*sectionEnd=-1, lineEnd=0; buf[lineEnd]!='\0';)
	{
		getLine(buf, lineEnd, &lineStart, &lineEnd);
		if (buf[lineStart] == '[')
		{
			for (i=++lineStart; i<lineEnd && buf[i]!=']'; i++);
			if (i<lineEnd && strnicmp(buf+lineStart, section, i-lineStart)==0)	/* 找到Section？ */
				*sectionEnd = lineEnd;
			else if (*sectionEnd != -1)	/* 找到Section但未找到Key */
				return -1;
		}
		else if (*sectionEnd!=-1 && NOT_COMMENT(buf[lineStart]))	/* 找到Section且该行不是注释 */
		{
			for (i=lineStart+1; i<lineEnd && buf[i]!='='; i++);
			if (i<lineEnd && strnicmp(buf+lineStart, key, i-lineStart)==0)	/* 找到Key？ */
			{
				*valueStart = i + 1;
				*valueSize = lineEnd - *valueStart;
				return 0;
			}
		}
	}
	return -1;
}

int getString(const char *buf, const char *section, const char *key,
	const char *defaultValue, char *value, unsigned long size)
{
	int sectionEnd, valueStart;
	unsigned long valueSize;

	assert(section != NULL && strlen(section) > 0);
	assert(key != NULL && strlen(key) > 0);
	assert(defaultValue != NULL);
	assert(value != NULL);
	assert(size > 0);
	assert(buf != NULL);

	if (findKey(buf, section, key, &sectionEnd, &valueStart, &valueSize)!=0 || valueSize==0)	/* 未找到？ */
	{
		strncpy(value, defaultValue, size);
		return -1;
	}
	if (size-1 < valueSize)		/* 找到但太长？ */
		valueSize = size - 1;
	memset(value, 0, size);
	strncpy(value, buf+valueStart, valueSize);
	return 0;
}

int getInt(const char *buf, const char *section, const char *key, int defaultValue)
{
	char value[21] = {0};
	getString(buf, section, key, "", value, sizeof(value));
	if (value[0] == '\0')	/* 找不到或找到但为空？ */
		return defaultValue;
	return atoi(value);
}
