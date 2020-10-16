# include"HDEXtoDC.h"
#include "stdafx.h"


char _t_HexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - 0x30;
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	else
		return 0x10;
}


int _t_Str2Hex(CString str, char* data)
{
	int t, t1;
	int rlen = 0, len = str.GetLength();
	//data.SetSize(len/2);
	for (int i = 0; i<len;)
	{
		char l, h = str[i];
		if (h == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len)
			break;
		l = str[i];
		t = _t_HexChar(h);
		t1 = _t_HexChar(l);
		if ((t == 16) || (t1 == 16))
			break;
		else
			t = t * 16 + t1;
		i++;
		data[rlen] = (char)t;
		rlen++;
	}
	return rlen;
}


//自己实现一个转换成16进制数
char  getcharhes(  int num)
{  
	char ch;
	switch (num)
	{
	case 0:
		ch = '0';
		break;
	case 1:
		ch = '1';
		break;
	case 2:
		ch = '2';
		break;
	case 3:
		ch = '3';
		break;
	case 4:
		ch = '4';
		break;
	case 5:
		ch = '5';
		break;
	case 6:
		ch = '6';
		break;
	case 7:
		ch = '7';
		break;
	case 8:
		ch = '8';
		break;
	case 9:
		ch = '9';
		break;
	case 10:
		ch = 'A';
		break;
	case 11:
		ch = 'B';
		break;
	case 12:
		ch = 'C';
		break;
	case 13 :
		ch = 'D';
		break;
	case 14:
		ch = 'E';
		break;
	case 15:
		ch = 'F';
		break;
	default:
		break;
	}
	return ch;
}

void datatoHEx(int len, char* data, unsigned char * src)
{
	int num = 0;
	int high_set = 0;
	int low_set = 0;
	char media_str = '0';
	for (int i = 0,m = 1; i < len; i++)
	{
		num = src[i];
		m = 2*i;
		high_set = num / 16;
		data[m] =  getcharhes(high_set);
		
		low_set = num % 16;
		data[m+1] = getcharhes(low_set);
			
	}

}