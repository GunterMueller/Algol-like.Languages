/*  VIDEO BIOS INTERFACE */
#include <stdio.h>
#include <dos.h>
#include "\7layer\layer3\layer3.h"
#include "\7layer\layer4.h"
#include "salib.h"
#include "graphics.c"

SGOTOXY(struct saframe pad,int y,int x)
{ if (mode == TEXT)VGOTOXY(x, y);
  else {
  grx=x;gry=y;
  }
 }
STEXTBACK(struct saframe pad,short col)
{ textbackground(col);}
STEXTCOL(struct saframe pad,short col)
{ textcolor(col);}
SWHEREX(struct saframe pad,int x)
{ x = wherex();if (mode!=TEXT) x = grx;}
SWHEREY(struct saframe pad,int y)
{ y = wherey()-1;if (mode !=TEXT)y= gry;}
SGETCH(struct saframe pad, int ch )
{ ch = VGETCH(); }
SSETCH(struct saframe pat,int replication, int ch)
{ VSETCH(ch,replication) ;}
SSETCUR(struct saframe pad, int b, int t)
{ VSETCUR(t,b); }
SSETPAL(struct saframe pad, int pal, int b, int g , int r)
{ VSETPAL(r,g,b,pal); }
SLINE(struct saframe pad,int n,int y2,int x2,int y1,int x1)
{

 Line(x1,y1,x2,y2,n);
}
Smousex(struct saframe pad,int x)
{ x = MOUSEX();}

Smousey(struct saframe pad,int x)
{ x = MOUSEY();}

Smousebut(struct saframe pad,int x)
{ x = MOUSEBUT();}

Smousepres(struct saframe pad,int x)
{ x = (MOUSEPRES()?1:0);}
