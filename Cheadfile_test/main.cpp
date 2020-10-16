#include<stdio.h>

void test_array(int a[20])
{
	printf("%d \n",sizeof(a));
}

int main()
{

	int b[20] = {2,4,5,2};
	int mm[40] = {9,3,3,4,2};
	printf("%d \n",sizeof(b));
	test_array(b);
	test_array(mm);
	getchar();
	return 0;
}
