#include<stdio.h>
#include<stdlib.h>
#include"callback.h"

void mycallocke(char *ch)
{
	printf("%s  \n",ch);
}

void pinarryget(char *out)
{
	out[0] = 0;
	out[1] = 1;
}


int main()
{ 
	//char namefiel[20] = "helloworld";
	//MYcalloback(mycallocke, namefiel);

	/*char buffer[256] = "23jhgh56567rf";
	char *ch = &buffer[6];
	int number = atoi(ch);
	printf("the number is %d \n", number);*/


	char pinarry[10] = { 0 };
	pinarryget(pinarry);

	printf("pinarry =  %d , %c \n", pinarry[0], pinarry[0]);
	printf("pinarry =  %d , %c \n", pinarry[1], pinarry[1]);

	/*char ch = 0;
	if (ch == 0)
	{
		printf("ch is 0 \n");
	}

	ch = 1;
	if (ch == 1)
	{
		printf("ch is 1 \n");
	}*/

	getchar();
	return 0;
}
