#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>



void function_newfile()
{
	FILE *fp;
	char  *filename = "D://workspaceVS15/CHeaderFileTest/x64/Debug/test.txt";
	fp = fopen(filename, "w+");
	if (fp == NULL)
	{
		printf("File open eeror \n");
		getchar();
		return 0;
	}
	char filetext[200] = "dagydggg\nhsdyqh\tncjasdnjad\ndhj  hhhj__+==hdg|\nkj<>,.;';;'&%$#@\n";
	fputs(filetext, fp);
	fclose(fp);
}





int mainFILE()
{   
	char  *filename = "D://workspaceVS15/CHeaderFileTest/x64/Debug/test.txt";
	char *newfilename = "D://workspaceVS15/CHeaderFileTest/x64/Debug/YDHU.txt";
	FILE *fp;
	fpos_t postion;
	unsigned   char filetext[200];
	memset(filetext,0,200);
	fp = fopen(newfilename,"rb+");
	/*char filetext[200] = "dagydggg\nhsdyqh\tncjasdnjad\ndhj  hhhj__+==hdg|\nkj<>,.;';;'&%$#@\nÄãºÃ\n";
	fputs(filetext, fp);
	fclose(fp);*/
	printf(" CHAR_MAX = %d \n",CHAR_MAX);
	fgetpos(fp,&postion);
	printf("offset = %d\n",postion);
	int num=0;
	int n = 0;
	unsigned char *buf = (unsigned char*)malloc(sizeof(unsigned char)*3);
	fread(buf, sizeof(unsigned char), 3, fp);
	for (; n < 3;n++)
	{
		printf(" %#x\t",buf[n]);
	}
	printf("\n");
	//fseek(fp,10L, SEEK_SET); 
	free(buf);
	rewind(fp);
	int ret =ferror(fp);
	clearerr(fp);
	fclose(fp);
	/*while (fgets(filetext,200,fp) != NULL)
	{
		printf("%s",filetext);
		printf("%d\n",num=strlen(filetext));

		for (n = 0; n < num;n++)
		{
			if (filetext[n] > 127)
			{
				printf("%#x",filetext[n]);
			}
		}
		printf("\n");
		fgetpos(fp, &postion);
		printf("offset = %d\n", postion);
	}
	fclose(fp);*/

	//rename(filename, newfilename);
	//char *pch = (char*)malloc(200);
	//memset(pch,0,200);
	//printf("sizeof pch is %d \n  strlen pch is %d \n", sizeof(pch),strlen(pch));
	//printf("pch is %s\n",pch);
	//free(pch);
	getchar();
	return 0;
}