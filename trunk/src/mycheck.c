/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：mycheck.c
* 摘	要：客户端校验算法
* 作	者：kkHAIKE & HustMoon
*/
#include "mycheck.h"
#include "md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BYTE *bin_8021x = NULL;
static DWORD size_8021x;
static BYTE hex[][17]={"0123456789ABCDEF",
						"0123456789abcdef"};

void hex_to_str(const BYTE *a, char *b, int hexsize, int upper) {
	BYTE *q = (BYTE *)b;
	int i;
	for (i=0; i<hexsize; i++) {
		*q = hex[upper][a[i]>>4]; q++;
		*q = hex[upper][a[i]&0xf]; q++;
	}
	*q = 0;
}

BYTE *ReadCode(const char *file, DWORD *size) {
	BYTE *data = NULL;
	int i;
	FILE *fp;
	PPE_HEADER_MAP hpe;
	
	if ((fp=fopen(file, "rb")) == NULL)
		goto fileError;
	data = (BYTE *)malloc(0x1000);
	if (fread(data, 0x1000, 1, fp) < 1)
		goto fileError;
	
	hpe = (PPE_HEADER_MAP)(((PIMAGE_DOS_HEADER)data)->e_lfanew + data);
	for (i=0; i<hpe->_head.NumberOfSections; i++) {
		if (hpe->section_header[i].Characteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE)) {
			fseek(fp, hpe->section_header[i].PointerToRawData, SEEK_SET);
			*size = hpe->section_header[i].SizeOfRawData;
			free(data);
			data = (BYTE *)malloc(*size);
			if (fread(data, *size, 1, fp) < 1)
				goto fileError;
			fclose(fp);
			return data;
		}
	}

fileError:
	if (fp != NULL)
		fclose(fp);
	if (data != NULL)
		free(data);
	return NULL;
}

BYTE *ReadCode2(const char *dataFile, DWORD *size) {
	BYTE Buf[16], *buf=Buf;
	FILE *fp = NULL;
	if ((fp=fopen(dataFile, "rb")) == NULL
		|| fread(buf, 16, 1, fp ) < 1)
		goto fileError;
	*size = (*(UINT4 *)buf ^ *(UINT4 *)(buf + 4));
	if ((int)*size <= 0)
		goto fileError;
	buf = (BYTE *)malloc(*size+0x100);
	if (fread(buf, *size, 1, fp) < 1) {
		free(buf);
		goto fileError;
	}
	fclose(fp);
	return buf;
	
fileError:
	if (fp != NULL)
		fclose(fp);
	return NULL;
}

void check_free() {
	if (bin_8021x) {
		free(bin_8021x);
		bin_8021x = NULL;
	}
}

int check_init(const char *dataFile) {
	char name[0x100];
	char *p;
	check_free();
	strcpy(name, dataFile);
	if ((p=strrchr(name, '/')+1) == (void *)1)
		p = name;
	strcpy(p, "8021x.exe");
	if ((bin_8021x=ReadCode(name, &size_8021x)) == NULL
		&& (bin_8021x=ReadCode2(dataFile, &size_8021x)) == NULL)
		return -1;
	return 0;
}

void V2_check(const BYTE *seed, char *final_str) {
	int i, size = size_8021x / 8;
	BYTE table[144], *md5Dig, *b8021x = (BYTE *)malloc(size+16);
	memcpy(b8021x, seed, 16);
	for (i=0; i<8; i++) {
		memcpy(b8021x+16, bin_8021x+size*i, size);
		md5Dig = ComputeHash(b8021x, size+16);
		table[18*i] = seed[2*i];
		memcpy(table+18*i+1, md5Dig, 16);
		table[18*i+17] = seed[2*i+1];
	}
	free(b8021x);
	md5Dig = ComputeHash(table, 144);
	hex_to_str(md5Dig, final_str, 16, 1);
}
DWORD getVer(const char *file) {
	FILE *fp;
	BYTE *data = NULL;
	int i, j;
	DWORD size, VirtualAddress;
	PPE_HEADER_MAP hpe;
	PIMAGE_RESOURCE_DIRECTORY prd;
	PIMAGE_RESOURCE_DATA_ENTRY prde;
	PVS_VERSIONINFO pvs;
	
	if ((fp=fopen(file, "rb")) == NULL)
		goto fileError;
	data = (BYTE *)malloc(0x1000);
	if (fread(data, 0x1000, 1, fp) < 1)
		goto fileError;

	hpe = (PPE_HEADER_MAP)(((PIMAGE_DOS_HEADER)data)->e_lfanew + data);
	for (i=hpe->_head.NumberOfSections-1; i>=0; i--) {
		if (strcmp(hpe->section_header[i].Name, ".rsrc") == 0) {
			fseek(fp, hpe->section_header[i].PointerToRawData, SEEK_SET);
			size = hpe->section_header[i].SizeOfRawData;
			VirtualAddress = hpe->section_header[i].VirtualAddress;
			free(data);
			data = (BYTE *)malloc(size);
			if (fread(data, size, 1, fp) < 1)
				goto fileError;
			prd = (PIMAGE_RESOURCE_DIRECTORY)data;
			for (j=0; j<prd->NumberOfIdEntries; j++) {
				if (prd->DirectoryEntries[j].Id==16 && prd->DirectoryEntries[j].NameIsString==0) {
					prd = (PIMAGE_RESOURCE_DIRECTORY)(data+prd->DirectoryEntries[j].OffsetToDirectory);
					prd = (PIMAGE_RESOURCE_DIRECTORY)(data+prd->DirectoryEntries[0].OffsetToDirectory);
					prde = (PIMAGE_RESOURCE_DATA_ENTRY)(data+prd->DirectoryEntries[0].OffsetToData);
					pvs = (PVS_VERSIONINFO)(data+(prde->OffsetToData-VirtualAddress));
					size = pvs->Value.dwFileVersionMS;
					fclose(fp);
					free(data);
					return size;
				}
			}
			goto fileError;
		}
	}

fileError:
	if (fp != NULL)
		fclose(fp);
	if (data != NULL)
		free(data);
	return -1;
}

