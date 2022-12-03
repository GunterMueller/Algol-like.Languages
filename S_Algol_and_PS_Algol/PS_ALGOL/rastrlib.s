! rastrlib.s  file to interface to C rasterop prims
! ---- pixels
let vo.n = @1 of int[-1]
let vo.ff = @1 of int[0]
procedure mkon(->*int);  vo.n
procedure mkoff(->*int);  vo.ff
procedure PixelRep(*int v->*int);v
procedure ivconcat(*int v1, v2 ->*int); v1++v2
class pixel is *int and
{   <pixel-exp1> is { "on."} = mkon or
    <pixel-exp1> is { "off."} = mkoff or
    <pixel-exp4> is {<pixel-exp5> "++" <pixel-expression>}= ivconcat or
    <*int-clause> is {"PixelRep." <pixel-expression>}=PixelRep
}
let on =on.
let off=off.
! ------ Rasters
structure rast( *int bitmap; bool is.screen
                 int width,rx, ry,rdx,rdy,rplane)
procedure mkplane( int dx,dy,init ->pntr)
{ let w:= dx div 8
  if (dx rem 8) >0 do w:=w+2
  rast(vector 1::(w*dy) div 2 of init,false,w,0,0,dx,dy,0)
}
procedure getpel(pntr raster;int x,y->int);alien "_getpel"
procedure getbit(pntr raster;int x1,y1->int)
{ if x1<=raster(rdx) and y1 <=raster(rdy) then begin
     let x2=x1+raster(rx)
     let y2=y1+raster(ry)
     if raster(is.screen) then getpel(raster,x2,y2)
     else begin
          let w = raster(width)
          let byte.offset = y2*w+(x2 div 8)
          let integer.offset = byte.offset div 2
          let bit.offset = 8*(1-(byte.offset rem 2))+(7-(x2 rem 8 ))
          let mask = shift.l(1,bit.offset)
          let bm = raster(bitmap)
          let word = bm(integer.offset)
          let masked = b.and(word, mask)
          if masked = mask then -1 else 0
     end
  end
  else 0
}
procedure crasterop(int op; pntr r1,r2);alien "_rasterop"
procedure mkimage( int dx,dy; pixel ini-> *pntr)
{ let init = PixelRep.(ini)
  let depth = upb(init)
  let v:=vector 1::(depth ) of nil
  for i=1 to depth  do
      v(i):=mkplane( dx,dy,init(i))
  v
}
procedure mkscreen( int dx,dy,d -> *pntr)
{
  let r:=vector 1:: d of nil
  for i=1 to d do{ r(i):=
     rast(@ 1 of int[0], true,dx div 8,0,0,dx,dy,i-1)
   }
  r
}
let EGASCREEN = mkscreen( 640,350,4)
let VGA18 = mkscreen(640,480,4)
procedure the.screen(->*pntr);EGASCREEN
procedure limitplane(pntr r; int X,Y,Dx,Dy ->pntr)
{  procedure bounds
   begin
        write"'nRaster bounds error'n",X,Y,Dx,Dy; abort; end;
   let rrdx=r(rdx)
   let rrdy=r(rdy)
   if (X)>rrdx then bounds()
   else if (Y)>rrdy do bounds()
   if (X+Dx)>rrdx do Dx:=rrdx-X
   if (Y+Dy)>rrdy do Dy:=rrdy-Y
   X:= X+r(rx)
   Y:= Y+r(ry)

   rast(r(bitmap),r(is.screen),r(width),X,Y,Dx,Dy,r(rplane))
}
procedure rasterop(int op;*pntr r1,r2 )
{
  let depth = upb(r1)
  for i=lwb(r1) to depth do {
    crasterop(op,r1(i),r2(i))
  }
}

procedure limitop(*pntr r ; cint dx,dy,x,y -> *pntr)
{  let depth = upb (r)
   let v:=vector 1::depth of limitplane(r(1),x,y,dx,dy)
   for i=1 to depth do v(i):=limitplane(r(i),x,y,dx,dy)
   v
}
procedure getpixel( *pntr image;int x,y->*int)
{
  let p:=1
  procedure nth(int layer->*int)
    @0 of int[getbit(image(layer),x,y) ]
  let result:=nth(0)
  let u = upb(image)
  while p<=u do begin
       result:=result++(nth(p))
       p:=p+1
  end
  result
}
procedure ror(*pntr r1,r2);rasterop(0,r1,r2)
procedure rand(*pntr r1,r2);rasterop(1,r1,r2)
procedure xor(*pntr r1,r2);rasterop(2,r1,r2)
procedure copy(*pntr r1,r2);{ rasterop(3,r1,r2)}
procedure nand(*pntr r1,r2);rasterop(4,r1,r2)
procedure nor(*pntr r1,r2);rasterop(4,r1,r2)
procedure not(*pntr r1,r2);rasterop(5,r1,r2)
procedure xnor(*pntr r1,r2);  rasterop(6,r1,r2)


class image is *pntr and
{
  <void-clause>is{"rand" <image-clause>"onto" <image-clause>}=rand
  or
  <void-clause>is{"nand" <image-clause>"onto" <image-clause>}=nand
  or
  <void-clause>is{"copy" <image-clause>"onto" <image-clause>}=copy
  or
  <void-clause>is{"xnor" <image-clause>"onto" <image-clause>}=xnor
  or
  <void-clause>is{"not" <image-clause>"onto" <image-clause>}=not
  or
  <image-clause> is {"screen"}=the.screen
  or
  <image-clause> is {"image"<int-clause>"by"<int-clause>"of"<pixel-clause>}
  = mkimage
  or
  <image-clause> is {
     "limit"<image-clause>"to"<int-clause>"by"<int-clause>
     "at"<int-clause> "," <int-clause>
     } =limitop
  or
  <void-clause>is{"ror" <image-clause>"onto" <image-clause>}=ror
  or
  <void-clause>is{"xor" <image-clause>"onto" <image-clause>}=xor
  or
  <void-clause>is{"nor" <image-clause>"onto" <image-clause>}=nor

}
