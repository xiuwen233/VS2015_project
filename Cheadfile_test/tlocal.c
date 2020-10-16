#include<locale.h>
#include<stdio.h>

int mainlocale()
{  
	printf("LC_ALL %d \n", LC_ALL);
	printf("LC_COLLATE %d \n", LC_COLLATE);
	printf("LC_CTYPE %d \n", LC_CTYPE);
	printf("LC_MAX %d \n", LC_MAX);
	printf("LC_MIN %d \n", LC_MIN);
	printf("LC_MONETARY %d \n", LC_MONETARY);
	printf("LC_TIME %d \n", LC_TIME);
	printf("LC_NUMERIC%d \n", LC_NUMERIC);
	struct  lconv * p = localeconv();
	printf("localeconv is \n");
	printf(" %s \t %s \t %s \t %s\n",p->decimal_point,p->thousands_sep,p->grouping,p->int_curr_symbol);
	printf(" %s \t %s \t %s \t %s\n", p->currency_symbol, p->mon_decimal_point, p->mon_thousands_sep, p->mon_grouping);
	printf(" %s \t %s \t %c \t %c\n",p->positive_sign,p->negative_sign,p->int_frac_digits,p->frac_digits);
	printf("%c \t %c \t %c \t %c \n",p->p_cs_precedes,p->p_sep_by_space,p->n_cs_precedes,p->n_sep_by_space);
	printf("%c \t %c \t  %d \t %d \n",p->p_sign_posn,p->n_sign_posn,p->_W_decimal_point,p->_W_thousands_sep);
	printf("%d \t %d \t %d \t  %d \n", p->_W_int_curr_symbol,p->_W_currency_symbol,p->_W_mon_decimal_point,p->_W_mon_thousands_sep);
	printf("%d \t %d \n",p->_W_positive_sign,p->_W_negative_sign);
	getchar();
	return 0;
}