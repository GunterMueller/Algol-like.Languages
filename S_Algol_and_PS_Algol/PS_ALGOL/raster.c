/* raster .c
   library of raster op routines
   for psalgol and the IBM EGA or VGA
   */
#include <dos.h>
#define RBUF 1000
#include "\7layer\layer3\layer3.h"
#define VIDEOBUFFER MK_FP(0xa000,0)
#define SIOP 0x3c4  /* sequencer io port */
#define GCA 0x3ce   /* graphics controler address */
#include "\7layer\layer4.h"
#include "salib.h"
#ifdef NOOPT
static getline(char *src,char *dest,INTEGER x,INTEGER dx)
 /* src start address of a raster line */
 /* dest start address of an aligned buffer */
 /* x starting bit to copy */
 /* dx how many bits to copy */
/* get the bits and align them */
{
    int first;
	int bytes;
	register int i;
	char mask;
	union {
		   char pair[2];
		   int acc;
		   } a;
    int shift;
	shift = 8- (x & 7) ;

	first = x>>3;
	bytes = dx>>3;
	if (dx & 7 ) bytes++;
	src = & src[first];
    for(i=0;i<bytes;i++)
	{
        a.pair[0] = src[1];
		a.pair[1] = *src++;
		* dest ++ = a.acc >>(shift);

	}


}

putloop(char * src,char * dest,int bytes,int shift,char mask,int error)
{ int i;
    char junk;
	union {
		   char pair[2];
		   int acc;
		   } a,b,c,d;

		b.acc=0;
		b.pair[0]= 255;
		b.acc = (b.acc << (shift))^ -1;

    for(i=bytes; i; i--)
	{
        a.acc=0;
		a.pair[0] = *src ++;
		a.acc = a.acc << shift;
		c.pair[0] = dest[1];
		c.pair[1] = *dest;
		c.acc = (c.acc & (b.acc))| a.acc;
		*dest++ =c.pair[1];
		junk=*dest;     /* this is needed to load the EGA latches */
		*dest=c.pair[0];


	}
	if(error==0) return;

    /* mask off the final part byte */
        a.acc=0;d.acc=0;
    	a.pair[0] = *src;
		d.pair[0] =  mask;

		a.acc = a.acc << shift;
		d.acc = (d.acc << shift);

		c.pair[0] = dest[1];
		c.pair[1] = *dest;
		c.acc = (c.acc & ~(d.acc))| (a.acc&(d.acc));
		*dest=c.pair[1];
		junk=dest[1];     /* this is needed to load the EGA latches */

		dest[1]=c.pair[0];

}

#endif


static putline(src,dest,x,dx)
char *src; /* start address of a buffer */
char *dest; /* start address of a raster line */
INTEGER x;	   /* starting bit to copy */
INTEGER dx;  /*  how many bits to copy */
/* get the bits and align them */
{
    int first;
	int bytes;
    register	int i;
	char m1,m2,hi,lo;
	static char mask[8]={128,192,224,240,248,252,254,255};

    int shift;
	char junk;
	shift = 8-(x & 7) ;

	first = x>>3;
	bytes = dx>>3;

    dest = & dest [first];
    PLOOP(src,dest,bytes,shift,mask[dx & 7],dx & 7);
}
struct rast{ struct sinfo tm;
			 PID  pntr;
	         INTEGER screen;
             INTEGER width;     /* in bytes of a line */
			 INTEGER x,y,dx,dy,rplane;
			 };

rasterop(struct saframe pad,PID p2,PID p1,int op)
/* this is called from PS-algol */
{   struct rast r1,r2;
    GetBytes(sizeof(r1),0,&p1,(char*)&r1);
    GetBytes(sizeof(r1),0,&p2,(char *)&r2);
	if(r1.dx <r2.dx){ r2.dx= r1.dx;} else r1.dx=r2.dx;
	if(r1.dy <r2.dy){ r2.dy= r1.dy;}else r1.dy = r2.dy;
	rastcpy(&r1,&r2,op);
}
static rastcpy(r1,r2,op)struct rast *r1,*r2;int op;
{
	struct rast R1,R2;
	  R2 = *r2; R1= *r1;
	if(r1->dx <r2->dx) R2.dx= r1->dx;
	if(r1->dy <r2->dy) R2.dy= r1->dy;

    if(rspace(&R2) > RBUF) {
	  /* do it by halves */

	  R1.dy /=2;

	  rastcpy(&R1,&R2,op);
	  R1.y += (R1.dy);
      R2.y += R1.dy;
	  R2.dy= r2->dy-R1.dy;
	  R1.dy= r1->dy-R1.dy;
	  rastcpy(&R1,&R2,op);
	} else primcpy(&R1,&R2,op);

}
static int rspace(r)struct rast *r;
{
   int bpl;
   bpl = r->dx / 8;
   if(r->dx & 7 ) bpl ++;
   return (bpl * r->dy);

}
static char * lstart(r,row)struct rast *r; int row;
{   char * p;
    struct saframe pad;
    if((r->screen)!=0L){p = VIDEOBUFFER; plane(pad,r->rplane);}
	else p = &((char*) whereis(&(r->pntr)))[4];

	return & ( p[row * r->width]);

}
static primcpy(r1,r2,op)struct rast *r1,*r2;int op;
{    char b1[RBUF],b2[RBUF],*B1; register char  *B2;
     int i,row;
	 int bytespline, l1,n;
	 l1 = 0;
	 n= rspace(r2);
	 bytespline =(r2->dy? n / r2->dy:0);
	 for(row=0;row<r2->dy;row++){
		if((op==3) || (op==6)) { /* this is a copy operation */}
		else
		GETLINE(lstart(r2,(int)(row+r2->y)),&b2[l1],r2->x,r2->dx);
		GETLINE(lstart(r1,(int)(row+r1->y)),&b1[l1],r1->x,r2->dx);
		l1 += bytespline;
	 }

     B1 = b1; B2= b2;
	 switch(op){
	 case 0:
	   for(i=n;i;i--) *B2++ |= *B1++;
	   break;
     case 1:
	   for(i=0;i<n;i++) *B2++ &= *B1++;
	   break;
     case 2:
	   for(i=0;i<n;i++) *B2++ ^= *B1++;
	   break;

     case 3:
	   for(i=0;i<n;i++) *B2++ = *B1++;
	   break;

     case 4:
	   for(i=0;i<n;i++,B2++) *B2 = ~(*B2 &= *B1++);
	   break;

     case 5:
	   for(i=0;i<n;i++) b2[i]= ~(b2[i] |= b1[i]);
	   break;
     case 6:
	   for(i=0;i<n;i++,B1++) *B2++ = ~(*B1);
	   break;


     case 7:
	   for(i=0;i<n;i++) b2[i]= ~(b2[i] ^= b1[i]);

	   break;
	}
	l1 =0;
    for(row=0;row<r2->dy;row++){
		putline(&b2[l1],lstart(r2,(int)(row+r2->y)),r2->x,r2->dx);
		l1 += bytespline;
	 }


}

static prselect(int plane)
{
  outport(GCA,5);outport(GCA,256*(3&plane) +4);

}
static pwselect(int plane)
{
   outport(SIOP,(1<<(8+(3&plane)))+2);

}
plane(struct saframe pad,int p)
{ prselect(p); pwselect(p);}

