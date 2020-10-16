#include<stdio.h>
#include<string.h>

int mainstring()
{
	char dest[300] = "daiudaygiyu";
	char ugdg[200] = "sdjaiuhs";
//	char *op = memchr(dest,'y',sizeof(dest));
	char *src = dest;
	char *op = strstr(dest,"day");
	printf("%d\n", op-src);
	
	//strcpy(dest,ugdg);
	//memcpy(dest,ugdg,sizeof(ugdg));
	//strcat(dest,ugdg);
	/*int num = strcmp(dest, ugdg);
	printf("%d\n", num);
	num = strcmp(ugdg, dest);
	printf("%d\n", num);*/
	//printf("%s\n",dest);
	getchar();
	return 0;
}