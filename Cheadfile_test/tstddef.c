#include<assert.h>
#include<limits.h>
#include<stdio.h>
#include<stddef.h>


typedef struct {
	char f1;
	struct {
		float  f1t;
	}f2;
	int f3;
}Str;

static char *pc = NULL;
static double *pd = NULL;
static size_t offs[] ={
	 offsetof(Str, f1),
	 offsetof(Str,f2),
	 offsetof(Str, f3)
};

int maineffee()
{
	ptrdiff_t pd = &pc[INT_MAX] - &pc[0];
	wchar_t wc = L'Z';
	Str x = { 1,2,3 };
	char *ps = (char *)&x;

	assert(sizeof(ptrdiff_t) == sizeof(size_t));
	assert(sizeof(size_t) == sizeof(sizeof(char)));
	assert(pd == &pc[INT_MAX] - &pc[0]);
	assert(wc ==L'Z');
	assert(offs[0] < offs[1]);
	assert(offs[1] < offs[2]);
	assert(*(char *)(ps+offs[0]) == 1);
	assert(*(float *)(ps+offs[1]) == 2);
	assert(*(int *)(ps+offs[2]) == 3);
	printf("sizeof (size_t) = %u\n",sizeof(size_t));
	printf("sizeof(wchar_t) = %u\n",sizeof(wchar_t));
	puts("SUCCESS testing <stddef.h>");
	getchar();
	return 0;
}