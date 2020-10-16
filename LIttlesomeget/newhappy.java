#include <iostream>

using namespace std;

int cout_one_bits(unsigned int value)
{
	int cout = 0;
	while (value > 0)
	{
		if (value % 2 == 1)
		{
			cout++;
		}
		value = value / 2;
	}
	return cout;
}


int getnum(int num , int index)
{
	return (num & (0x1 << index)) >> index;
}

int main()
{
	int num = 0;
	printf(" int 的位数%d  \n", sizeof(num)*8);
	for (int i=31; i >=0; i--)
	{
		printf("%d", getnum(0b11101010, i));
	}
	printf("\n");

	getchar();
	return 0;
}
