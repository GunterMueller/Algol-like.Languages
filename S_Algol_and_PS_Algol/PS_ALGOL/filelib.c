/*   file io and string library for salgol system */
#include <dos.h>
#include <fcntl.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "\7layer\layer3\layer3.h"
#include "\7layer\layer4.h"
#include "salib.h"
#define read myread
#define write mywrite
#define open myopen
#define close myclose
#define creat mycreat
#define digits "0123456789-"
#define realdigits "eE0123456789-."
#define ctrlz 26

int endofchan0 = 0;

typedef     union{PID ref;
     struct{ char pp[sizeof(PID)-sizeof(int)];int res;}x ;
     } pidparam;
typedef union{double realval;
     struct{ char pp[sizeof(double)-sizeof(int)];int res;}x ;
     } realparam;
/*--------------------initfileio----------------------------*/
/* called at start of process 								*/
initfileio()
{
	endofchan0 =0;
}
/*--------------------OPEN----------------------------------*/
OPEN(struct saframe pad,int mode,
pidparam u)
{
	char p[maxstring];
	int i;
	scopy(u.ref,p);
/*    u.x.res=bdosptr(61,p,mode);*/
	u.x.res=myopen(p,mode);
}


/*--------------------CREATE----------------------------------*/
CREATE(struct saframe pad,
	      pidparam u)
{
	char p[maxstring];
	int i;
	scopy(u.ref,p);
    u.x.res= mycreat(p,0);

}
MAKEROOT (struct saframe pad,PID pntr)
{
	SetRoot(pntr);
}
/*------------------- READBYTE ------------------------------*/
static readchar(int f)
{
	char c;

	if (f) {
	   if(-1 ==( myread(f,&c,1))) { return -1; } else {
	      return c;
	   }
   } else{
     c= getchar();
	 if ( c == EOF ) endofchan0 = 1;
     if ( c == ctrlz ) endofchan0 = 1;
   }
   return c;
}
READ_BYTE(int bx,int f)
{

	int * res;
	res = &f;
	*res = 127 & readchar(f);
}

/*------------------- READ ----------------------------------*/
READ(int bx,
     union{PID ref;
     struct{ char pp[sizeof(PID)-sizeof(int)];int res;}x ;
     }u)
{
	char p[maxstring];
	int i;
	char onechrstr();
	onechrstr(readchar(u.x.res) & 127,&u.ref);
}
/*------------------- READR ----------------------------------*/
READR(int bx,
	    realparam u)
{
	char p[maxstring];
	int i;float r;double *rp =(double *)&u.realval;
	token(u.x.res,p,realdigits);
	sscanf(p,"%f",&r);
	*rp = r;
}
/*------------------- PEEK ----------------------------------*/
PEEK(int bx,
	pidparam u)
{
	char p[maxstring],onechrstr();
	int i=u.x.res;
	long lseek();
	onechrstr(readchar(u.x.res) & 127,&u.ref);
    lseek (i, -1L,1);
}
/*------------------- READS ----------------------------------*/
READS(int bx,pidparam u)
{
	char p[maxstring];
	int i,f,j;
	f=u.x.res;
	while ((i=readchar(f))>=0 && i!='"'){};
    for(i= 0 ;i<(maxstring-1) &&p[i-1]!='"' ;i++)
		{j=readchar(f); if (j== -1 )break;p[i]=j;}
    p[i-1]='\0';


	mkstr(p,&u.ref,strlen(p));
}
/*------------------- READP ----------------------------------*/
READP(int bx,
      pidparam u)
{
	myread(u.x.res,&u.ref,sizeof(PID));
}
/*------------------- READ_A_LINE ----------------------------------*/
READ_A_LINE(int bx,pidparam u)
{
	char p[maxstring];
	long i;
	long initial ;
	for (i=0;(i<maxstring && p[i-1]!='\n')&& !EOI(1,u.x.res); i++)
	{p[i]=readchar(u.x.res);};
	p[i-1]='\0';
	mkstr(p,&u.ref,strlen(p));
}

/*---------------------------READI--------------------------*/
READI(int bx,int f)
{   char c[maxstring];
    token(f,c,digits);
	sscanf(c,"%d",&f);

}
EOI(int bx,int f)
{
    long lseek();int *res = &f;
	/* handle chan 0 as special case */
	if(f == 0) {f=endofchan0;return endofchan0;};
    if (-1 == readchar(f)){*res= 1;}
	else{
		lseek(f,-1L,1);
		*res = 0;
	}
	return(*res);

}

/*-------------------------- READB --------------------------*/

READB(int bx,int f)
{   char c[maxstring];
    token(f,c,"truefals");
	f =0;
	if(0==strcmp(c,"true")) f=1;
}

/*--------------------CLOSE----------------------------------*/
CLOSE(struct saframe pad,int f)
{
	myclose(f);
}


/*--------------------SEEK----------------------------------*/
SEEK(struct saframe pad,int key,int offset,int f)
{   long lseek(),off = offset;
	f=lseek(f,off,key);
}
/*-------------------UNLINK---------------------------------*/

UNLINK(struct saframe pad,
	      pidparam u)
{
	char p[maxstring];
	int i;
	scopy(u.ref,p);
    u.x.res= unlink(p);

}
isin(char c,char *ok)
{
	for(;*ok;ok++)if(c== *ok)return 1;
	return 0;
}
static token(int f,char *c,char *ok)
{

union REGS reg;struct SREGS sreg;

    int i,j;
	for(j=readchar(f);(j>= 0 && (isin(j,ok)==0));j=readchar(f))
	{};
    *c = j;
    for(i= 1 ;i<(maxstring-1) &&  isin(c[i-1],ok);i++)
		{j=readchar(f); if (j<0 )break; c[i] = j;}
	c[i]  ='\0';
	if (f){  /* seek back 1 char */
    reg.h.ah = 0x42; /* seek */
    reg.x.bx = f;
	reg.x.cx = -1; /* offset */
	reg.x.dx = -1;
	reg.h.al = 1 ; /* seek relative */
	intdosx(&reg,&reg,&sreg);


/*	lseek(f,-1L,SEEK_CUR);  */
	}else ungetch(j);


}
/*************************** CODE ****************************/
CODE(struct saframe pad,
     union{PID ref;
     struct{ char pp[sizeof(PID)-sizeof(int)];int res;}x ;
     }u)
{
	char onechrstr();
	onechrstr((u.x.res) & 127,&u.ref);
}
/************************ DECODE ***************************/
DECODE(struct saframe pad,
     union{PID ref;
     struct{ char pp[sizeof(PID)-sizeof(int)];int res;}x ;
     }u)
{
	char c;
	GetBytes(1,4,&u.ref,&c);
	u.x.res = c;
}
/*********************** ARGC **************************/
ARGC(struct saframe pad,int r)
{ extern int Argc; r=Argc; }
/********************** ARGV **************************/
ARGV(struct saframe pad,
     union{PID ref;
     struct{ char pp[sizeof(PID)-sizeof(int)];int res;}x ;
     }u)
{   extern char ** Argv;
    mkstr(Argv[u.x.res-1],&u.ref,strlen(Argv[u.x.res-1]));
}