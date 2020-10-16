#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include <windows.h>


void test_array_index()
{
	int a[7] = { 1,2,3,4,5,6,7 };
	int *p = a;
	printf("%d  %#x \n",&a[0], &a[0]);
	printf("%d  %#x\n", &a[0] + sizeof(int), &a[0] + sizeof(int));
	//p = &a[0] + sizeof(int);
	printf("%d \n",*(&a[0] + sizeof(int)));
	//p = &a + sizeof(int) * 2;
	//printf("%d \n",*p);
}


int  main3443()
{
	/*clock_t start = clock();
	Sleep(1000);
	clock_t stop = clock();

	printf("time is %f \n", (double)(stop - start)/CLOCKS_PER_SEC);*/

	//time  t2 = time();
	test_array_index();
	getchar();
	return 0;
}