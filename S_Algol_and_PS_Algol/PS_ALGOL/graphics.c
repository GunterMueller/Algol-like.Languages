/*  graphics run time library for salgol */

#define GRAPH 0
#define HERC 0
#define VGA 1
#define TEXT 2
#define LASTVIDEO TEXT
#define ALPHALINES 25
static int dimensions[LASTVIDEO+1][2]={720,348,640,350,80,24};
static int mode=TEXT;
static int grx=0;
static int gry=0;
static int adaptor = VGA;
SETPIXELHGC(int x,int y, int n);
SETPIXELVGA(int x, int y,int n);
SETPIXELTEXT(int x, int y, int n);
static void (*SETPIXEL[LASTVIDEO+1])(int x,int y, int n) =
     {SETPIXELHGC,
	 SETPIXELVGA,SETPIXELTEXT};
clearscreen()
{
  if (mode == TEXT){ clrscr();}
  else{
    gclear();
  }
}

gputc(c)char c;
/* put a character to the graphics screen */
{
  char * table = (char *)(0x0f000fa6eL);
  int i,j;
  if( (mode == TEXT)|| (adaptor == VGA))  {putchar(c);}else
  if (adaptor==HERC){
     j=c *8;
	 if(c=='\n'){gnewline();}else{
		if(c>=32)
	    for (i=0;i<8;i++)gputbyte(grx,gry-i,table[i+j]);
	    incpos();
	 }
  }
}
gputbyte(x,y,b)int x,y;char b;
{
   int i;
   for(i=8;i;){
      SetPixel(x+(--i),y,b & 1);
	  b = b>>1;
   }
}
incpos()
{
	if((grx+=8)>dimensions[adaptor][0]){
	   gnewline();
	}
}
gnewline()
{
    if((gry-=8)<0)gry=dimensions[adaptor][1];
    grx =0;
}
gclear()
{
 long * screen;
 long screenaddr = 0xb0000000L;
 int i;
 if (adaptor !=HERC){

   }{
     screen = (long*) screenaddr;
     for (i=0;i<0x2000; i++)*screen++ = 0L;
     grx=0; gry=dimensions[adaptor][1];
   }
}

Line(x1,y1,x2,y2,n)
int x1,y1,x2,y2;
int n ; /* colour */
{
	int dx,dy,x,y,aincr,bincr,yincr,steep;
	long d;
 	if(x2 == x1)
	{
		/* vertical line  */
		if(y1 >y2){
		   Swap(&y1,&y2);
	       for(y=y1;y<=y2;y++)
		      SetPixel(x1,y,n);
	       return;}
	}

	if (x1 >x2)
	{
		Swap(&x1,&x2);
		Swap(&y1,&y2);
    }
	if (y2>y1)
   	 yincr =1;
	else if (y2<y1)
     yincr = -1;
	else {
	 /* horizontal line */
	 for (x=x1;x<=x2;x++)
	 SetPixel(x,y1,n);
	 return;}

    dx = x2 -x1;
	dy = abs(y2-y1);
	if(steep = (dy>dx)){
	   Swap(&dx,&dy);
	   Swap(&x1,&y1);
	   Swap(&x2,&y2);
	};

	d = 2* dy -dx;
	aincr = 2 * (dy -dx);
	bincr = 2 * dy;
	if (x1 >x2)
	{
		Swap(&x1,&x2);
		Swap(&y1,&y2);
    }
	x = x1; y= y1;

    if (steep) {SetPixel(y,x,n); }
    else SetPixel(x,y,n);
	for (x=x1+1;x <= x2; x++)
	{
		if (d >=0){
		   y += yincr;
		   d += aincr;
		}else
		   d += bincr;
		if (steep){ SetPixel(y,x,n);}
		else SetPixel(x,y,n);
	}
 }
 Swap(x,y)int *x,*y;
 {
	int temp;
	temp= *x;
	*x= *y;
	*y = temp;
 }
 SetPixel(x,y,n)int x,y,n;
 {  if(mode == VGA){SETPIXELVGA(x,y,n);}
	else
 if (mode == TEXT) {SETPIXELTEXT(x,y,n);}
	else
    {(*SETPIXEL[mode])(x,y,n);return;}
 }
 SETPIXELTEXT(x,y,n)int x,y,n;
 {
	VGOTOXY(x,y);
	putch('+');

}
SETPIXELVGA(x,y,n)int x,y,n;
{
union REGS reg;
reg.h.ah = 0x0c; /* write dot */
reg.h.bh = 0;
reg.x.dx = y;
reg.x.cx = x;
reg.h.al = n;
int86(16,&reg,&reg);
}
VSETPAL(r,g,b,pal)int r,g,b,pal;
{
union REGS reg;
reg.h.ah = 0x10; /* set palette */
reg.x.bx = pal;
reg.h.ch = g;
reg.h.dh = r;
reg.h.cl = b;
reg.h.al = 0x10; /* write DAC register */
int86(16,&reg,&reg);
}
VSETCUR(top,bot)int top,bot;
{
union REGS reg;
reg.h.ah = 1; /* set cursor */
reg.h.ch = top;
reg.h.cl = bot;
int86(16,&reg,&reg);
}
VSETCH(c,rep)int c,rep;
{
union REGS reg;
reg.h.ah = 9; /* set char */
reg.x.cx = rep;
reg.h.bh = 0;
reg.h.bl = c >>8;
reg.h.al = c & 255;
int86(16,&reg,&reg);
}
VGOTOXY(x,y)int x,y;
{
union REGS reg;
reg.h.ah = 2; /* set char pos */
reg.h.bh = 0;
reg.h.dh = y;
reg.h.dl = x;
int86(16,&reg,&reg);
}

ginit(struct saframe pad, int m)
{
  union REGS reg;
  if(adaptor==HERC){ GMODE();} else
  {
	reg.h.ah = 0x0; /* set mode */
	reg.h.al = m;
	int86(16,&reg,&reg);
	}
mode=adaptor;
}
textmode()
{
   struct saframe pad;
   if(adaptor==HERC){TMODE();}
   else{  ginit(pad,3); };

mode=TEXT;
}
