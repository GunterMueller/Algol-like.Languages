#include <math.h>
#include "\7layer\layer3\layer3.h"
#include "\7layer\layer4.h"
#include "salib.h"
SIN(struct saframe pad,double res)
{
	res = sin(res);
}
SQRT(struct saframe pad, double res)
{   	res = sqrt(res);
}
COS(struct saframe pad, double res)
{
	res = cos(res);
}
EXP(struct saframe pad, double res)
{
	res = exp(res);
}
ATAN(struct saframe pad, double res)
{
	res = atan(res);
}



void TAN(struct saframe pad, double res)
{
	res = tan(res);
}

void LN(struct saframe pad, double res)
{
	res = log(res);
}
TRUNCATE(struct saframe pad,
         union {
				double x;
				int t[4];
				}p)
{
	int i;
	i= p.x;p.t[3]=i;
}

long Hash(union{PID ref;
                              struct{ char pp[sizeof(PID)-sizeof(int)];
									  int res;}x ;
                              }u)
{
	char * p;
	int i,j;long h;
	p = whereis(&u.ref);
	i = ObjSize(&u.ref);
	h=0;
	for(i = MIN( i,48);i;i--)h = h*3 + *p++;
	if (h<0) h= -h;
	return( h);
}