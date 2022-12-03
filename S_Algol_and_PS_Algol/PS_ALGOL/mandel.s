
! Plots the mandelbrot set in a fractal fashion
! uses menuseg
! needs a VGA
LISTON


! z = z*z +c
! (zr ,zi)=(zr*zr -zi*zi +cr,+2*zi*zr+ci )
! crt control procedures
! these give access to the ibm video bios
! Copyright Paul Cockshott
let cursor.x = proc(->int);alien "_wherex"
! returns the x pos of the video cursor

procedure cursor.y (->int);alien "_wherey"
! returns y pos of video cursor

procedure move.cursor (int x,y); alien "_gotoxy"
! move the video cursor to this place

procedure set.cur(int top,bottom); alien "_setcur"
! set the top and bottom lines of the cursor within a
! character cell

procedure getch(->int) ; alien "_getch"
! return the character and attribute beneath cursor
! the attribute is in the top 8 bits the char in the lower 8

procedure set.palette(int red,green,blue,palette) ; alien "_setpal"
! set the intensity of the colours on a VGA display
! palette : selects the DAC table entry
! red,green,blue : select intensity
! all numbers in range 0..255

procedure insert.line ;alien "_insline"
! insert a blank line at the cursor

procedure delete.line ;alien "_delline"
! delete the current line

let text.colour = proc(int c);alien "_textcol"
! set the colour of the text

procedure clear.screen; alien "_clearscreen"
! clears screen in either text or graphics mode

let background.colour = proc(int c);alien "_textback"
! set the colour of the background
! colours are encoded as below

let bright=8
! add this to a coulour to brighten it i.e. text.colour(bright+red)
let black =0; let blue =1; let green=2 ; let cyan = 3
let red =4; let magenta = 5; let brown =6; let white=7

! MICROSOFT MOUSE BIOS
let mouse.x = proc(->int); alien "_mousex"
! x position of the mouse

let mouse.y = proc(->int);alien "_mousey"
! y position of the mouse

let mouse.buttons = proc(->int);alien "_mousebut"
! returns bit to indicate if mouse buttons down

let mouse.present = proc(->bool);alien "_mousepres"
! true if mouse dirver installed

procedure mouse.show ;alien "_mouseshow"
! make mouse cursor appear

procedure mouse.hide ;alien "_mousehide"
!make mouse cursor vanish
! menues
! Menulib.s depends upon file menuseg.out in PSDIR
! Works in IBM text mode with microsoft mouse
! Copyright Paul Cockshott
let menuseg="menuseg"
let make.text.menu = import("make.text.menu")proc(*string,int -> * *int)
                segment menuseg
! first parameter is a vector of strings
! second parameter is the attributes to be used
! low order 4 bits specify foreground
! bits 4 to 6 specify background colour
! returns a 2 d aray of integers made up of the
! each integer contains a char in lower 8 bits and
! attributes in the top 8


let swap.block = import("swap.block")proc(* *int , int,int->* *int)
                segment menuseg
! swap a 2d array with an area of screen ram
! the old contents of the screen ram are returned
! first param is a 2 d array of integers
! next 2 give x,y position of top left corner

let menu.pop.up = import("menu.pop.up")proc(* *int,int,int->int)
                  segment menuseg
! put up a menu return the entry selected
! -1 if no entry selected
! graphics-------------------------------------------
procedure draw.line(int x1,y1,x2,y2,n); alien "_line"
! draw line in colour n from x1,y1 to x2,y2

procedure text.mode ; alien "_textmode"
! put into text mode


procedure graphics.mode(int mode) ; alien "_ginit"
! put into graphics mode
!  mode = iBM bios video mode

 ! rasterop.s  file to interface to C rasterop prims
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
procedure mkimage( int dx,dy; *int init-> *pntr)
{ write "mkimage "
  let depth = upb(init)
  let v:=vector 1::(depth ) of nil
  for i=1 to depth  do
      v(i):=mkplane( dx,dy,init(i))
  v
}
procedure mkscreen( int dx,dy,d -> *pntr)
{
  let r:=vector 1:: d of nil
  for i=1 to d do r(i):=
     rast(@ 1 of int[0], true,dx div 8,0,0,dx,dy,i-1)
  r
}
let EGASCREEN = mkscreen( 640,350,4)
let VGA18 = mkscreen(640,480,4)
procedure the.screen(->*pntr);VGA18
procedure limitplane(pntr r; int X,Y,Dx,Dy ->pntr)
{  procedure bounds
   begin text.mode()
        write"'nRaster bounds error'n",X,Y,Dx,Dy; abort; end;
   if (X+Dy)>r(rdx) then bounds()
   else if (Y+Dy)>r(rdy) do bounds()
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
procedure ivconcat(*int v1, v2 ->*int)
{            v1++v2
}
procedure getpixel( *pntr image;int x,y->*int)
{
  let p:=1
  procedure nth(int layer->*int)
    @0 of int[getbit(image(layer),x,y) ]
  let result:=nth(0)
  let u = upb(image)
  while p<=u do begin
       result:=ivconcat(result,nth(p))
       p:=p+1
  end
  result
}
! generate pixel literals
let vo.n = @1 of int[-1]
let vo.ff = @1 of int[0]
procedure mkon(->*int)
      vo.n
procedure mkoff(->*int)
      vo.ff
procedure ror(*pntr r1,r2);rasterop(0,r1,r2)
procedure rand(*pntr r1,r2);rasterop(1,r1,r2)
procedure xor(*pntr r1,r2);rasterop(2,r1,r2)
procedure copy(*pntr r1,r2);{ rasterop(3,r1,r2)}
procedure nand(*pntr r1,r2);rasterop(4,r1,r2)
procedure nor(*pntr r1,r2);rasterop(4,r1,r2)
procedure not(*pntr r1,r2);rasterop(5,r1,r2)
procedure xnor(*pntr r1,r2);  rasterop(6,r1,r2)

class pixel is *int and
{   <pixel-exp4> is { "on"} = mkon or
    <pixel-exp4> is { "off"} = mkoff or
    <pixel-expression> is {<pixel-exp1> "&" <pixel-expression>}= ivconcat
}

class #pixel is *pntr and
{
  <void-clause>is{"rand" <#pixel-clause>"onto" <#pixel-clause>}=rand
  or
  <void-clause>is{"nand" <#pixel-clause>"onto" <#pixel-clause>}=nand
  or
  <void-clause>is{"copy" <#pixel-clause>"onto" <#pixel-clause>}=copy
  or
  <void-clause>is{"xnor" <#pixel-clause>"onto" <#pixel-clause>}=xnor
  or
  <void-clause>is{"not" <#pixel-clause>"onto" <#pixel-clause>}=not
  or
  <#pixel-clause> is {"screen"}=the.screen
  or
  <#pixel-exp1> is {"image"<int-clause>"by"<int-clause>"of"<*int-clause>}
  = mkimage
  or
  <#pixel-expression> is {
     "limit"<#pixel-clause>"to"<int-clause>"by"<int-clause>
     "at"<int-clause> "," <int-clause>
     } =limitop
  or
  <void-clause>is{"ror" <#pixel-clause>"onto" <#pixel-clause>}=ror
  or
  <void-clause>is{"xor" <#pixel-clause>"onto" <#pixel-clause>}=xor
  or
  <void-clause>is{"nor" <#pixel-clause>"onto" <#pixel-clause>}=nor

}
!----------------------------------------------------------------
let accuracy =14
let gm=18
let sz=256
let nl ="'n"
procedure escape(real cr,ci ->int)
begin
   procedure absc(real r,i->real)
   begin
      (r*r + i*i)
   end
   let zi:=0.0
   let zr:=0.0
   let i:=0
   while i<accuracy and absc(zr,zi)<4 do {
      zr:=zr*zr-zi*zi +cr
      zi:=2*zi*zr +ci
      i:=i+1
      }
   if i>= accuracy do i:=0
   i

end
procedure int.to.pixel(int i->pixel)
begin
   procedure pixof(int i->pixel)
   if (i rem 2)=1 then on else off

   procedure rec.i.p(int i,l->pixel)
   if l=1 then pixof(i)
   else rec.i.p(i div 2, l-1 ) & pixof(i)

  rec.i.p(i,4)
end
graphics.mode(gm)
write "'n'nCreating rasters, please wait"

let one=1
let colours :=vector 0::15 of int.to.pixel(one)

for i= 1 to 15 do begin
    colours(i):=int.to.pixel(i)
    write "colour",i
end;

let block =32
let im1:=image block by block of colours(7)
write "im1 created"
let ims := vector 0::15 of im1
for i=0 to 15 do begin
      write i
      ims(i):=image block by block of colours(i)
      copy ims(i) onto screen

end;
let main = image sz by sz of colours(0)
text.mode()
structure mand(#pixel lastone;real mx1,my1,mdx,mdy; int mlim)
let context := mand(main,0.0, 0.0, 1.0, 1.0, 8)

procedure mandel(real x1,y1,x2,y2; int xpnts,ypnts,lim)
begin
   context:=mand(main,x1,y1,x2,y2,lim)

   let xstep = (x2-x1) / (1.0 *xpnts)
   let ystep = (y2 -y1)/ (1.0 *ypnts)
   for i= 4 to accuracy do {set.palette(i,(i rem 40)*4,70+lim,50)}
   procedure mandelrec(int x,y,dx,dy;real x1,y1)
   if (dx+dy >lim)and (mouse.buttons()=0) do
   begin

         dy:= dy div 2
         dx:=dx div 2
         let xinc= dx*xstep
         let yinc = dy*ystep

         if dx <= lim  do{
          let esc =(escape(x1,y1)  )
          let hue = ims(if esc=0 then 0 else (1+(esc rem 15)))
          let target2 = limit screen to dx by dy at x,y
          copy hue onto target2
         }
         mandelrec(x,y+dy, dx , dy,x1,y1+yinc )
         mandelrec(x,y,dx,dy,x1,y1)
         mandelrec(x+dx,y,dx,dy,x1+xinc,y1)
         mandelrec(x+dx,y+dy,dx,dy,x1+xinc,y1+yinc)
   end
   mandelrec(1,1,xpnts,ypnts,x1,y1)
   if (lim>1) and (mouse.buttons()=0) do
   mandel(x1,y1,x2,y2,xpnts,ypnts, lim div 3)
 end

procedure wait
begin
  copy ( screen ) onto main
  while {mouse.buttons()}=0 do {}
  not main onto screen
  let jjj= read.a.line()
  text.mode()
end

procedure new.run
begin
  write "terminate by double clicking mouse'n"
  write "type in a pair of x,y coordinates'nx1 "


  let x1=readr()
  write "y1 "
  let y1 = readr()
  write "x2 "
  let x2 = readr()
  write "y2 "
  let y2=readr()
  graphics.mode(gm)
  write x1,y1,x2,y2
  mouse.show()
  copy main onto screen
  mandel(x1,y1,x2,y2,sz,sz,sz)

  wait()
end

procedure rerun
begin
   graphics.mode(gm)
   copy main onto screen
   mandel(context(mx1),context(my1),context(mdx),context(mdy),sz,sz,
          context(mlim))
   wait()
end


  new.run()

?
