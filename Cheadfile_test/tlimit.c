#include<limits.h>
#include<stdio.h>

int tlimit()
{
	printf("CHAR_BIT %d \n", CHAR_BIT);
	printf("CHAR_MAX %d \n", CHAR_MAX);
	printf("CHAR_MIN %d \n", CHAR_MIN);
	printf("SCHAR_MAX %d \n", SCHAR_MAX);
	printf("SCHAR_MIN %d \n", SCHAR_MIN);
	printf("UCHAR_MAX %d \n", UCHAR_MAX);
	printf("SHRT_MAX %d \n", SHRT_MAX);
	printf("SHRT_MIN %d \n", SHRT_MIN);
	printf("USHRT_MAX %d \n", USHRT_MAX);
	printf("INT_MAX %d \n", INT_MAX);
	printf("INT_MIN %d \n", INT_MIN);
	printf("UINT_MAX %d \n", UINT_MAX);
	getchar();
	return 0;

}