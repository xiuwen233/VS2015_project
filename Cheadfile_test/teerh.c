#include<errno.h>
#include<stdio.h>
#include<math.h>
#include<assert.h>

int mainerror()
{
	assert(errno == 0);
	perror("NO ERROR reported as");
	errno = EDOM;
	assert(errno == EDOM);
	perror("EDOM error reported as");
	errno = ERANGE;
	assert(errno == ERANGE);
	perror("ERANGE  error reported as");
	errno = 0;
	assert(errno == 0);
	sqrt(-1.0);
	assert(errno == EDOM);
	perror("Domin error reported as");
	puts("SUCCESS testing error.h");
	getchar();
	return 0;
}