#include"callback.h"

void MYcalloback(void(*callocback)(char *), char *s)
{
	callocback(s);
}