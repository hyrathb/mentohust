#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, const char *argv[])
{
	char path[100]={0};
	strncpy(path,argv[0],strlen(argv[0])-4);
	char mento[200]="chown root ";
	char chmod[200]="chmod 4577 ";
	char dylib[200]="chmod 777 ";
	strcat(mento,path);
	strcat(dylib,path);
	strcat(chmod,path);

	printf("%s\n",chmod);
	strcat(mento,"mentohust");
	strcat(chmod,"mentohust");

	printf("%s\n",dylib);
	strcat(dylib,"../Frameworks/mypcap.dylib");
	if(system(mento)==0 && system(chmod)==0 &&system(dylib)==0 )return 0;
	return 1;
}
