segment "menuseg"
! segment to maintain simple menues on the text screen
procedure make.text.menu(*string menu; int attribs-> * *int)
begin
    let menuseg= "menuseg"
    let dummy =1
    let lo=lwb(menu)
    let hi=upb(menu)
    let size=lo-hi+1
    let max:=0
    for i=lo to hi do
    begin
         let l= length(menu(i))
         if l>max do max:=l
    end
    let a= attribs * 256
    let v:=vector lo-1::hi+1 ,0::max+1 of 32 +a
    let decode =proc(string s->int)
             alien "_DECODE"

   for i=hi to lo by -1 do
    begin
        let v2 := v(i)
        v2(0):=179+a
        v2(max+1):=179+a
        let m = menu(i)
        let l= length(m)
        for j=1 to l by 1 do
        begin
            v2(j):=decode(m(j|1))+a
        end
    end
    let vb :=v(hi+1)
    let vt :=v(lo-1)
    vt(0):= a + 218
    vt(max+1):=a+ 191
    vb(0):= a+192
    vb(max+1):=a+217
    let line = a+196
    for i= 1 to max do {
       vt(i):= line
       vb(i):=line
    }
   v
end


procedure set.palette(int red,green,blue,palette) ; alien "_setpal"
! set the intensity of the colours on a VGA display
! palette : selects the DAC table entry
! red,green,blue : select intensity
! all numbers in range 0..255





procedure swap.block(* * int block; int x,y-> * *int)
! exchange a 2 d array of chars+ attributes with the
! screen returning old contents of screen
begin
   let menuseg="menuseg"
   procedure set.cur(int top,bottom); alien "_setcur"
   ! set the top and bottom lines of the cursor within a
   ! character cell
   procedure cursor.x(->int);alien "_wherex"
   ! returns the x pos of the video cursor

   procedure cursor.y (->int);alien "_wherey"
   ! returns y pos of video cursor

   procedure move.cursor (int x,y); alien "_gotoxy"
   ! move the video cursor to this place

   procedure getch(->int) ; alien "_getch"
   ! return the character and attribute beneath cursor
   ! the attribute is in the top 8 bits the char in the lower 8

   procedure set.ch(int c,replication) ; alien "_setch"
   ! put char and attribute at cursor
   ! replicates it as specified
   ! does not move cursor




   ! create outer dimension of returned vector
   let l = lwb(block)
   let u = upb(block)
   let v := vector l::u ,1::2 of 0
   let oldx = cursor.x()
   let oldy = cursor.y()
   set.cur(0,0)
   for i= l to u do
   begin
       ! create inner dimension of returned vector
       let b2 = block(i)
       let l2 = lwb(b2)
       let u2 = upb(b2)
       let v2 := vector l2::u2 of 0
       v(i):=v2
       for j= l2 to u2 do
       begin
           move.cursor(x+j-l2,y+i-l)
           v2(j):=getch()
           set.ch(b2(j),1)

       end
   end
   move.cursor(oldx,oldy)
   set.cur(4,6)
   v
end

procedure menu.pop.up(* *int menu;int x,y ->int)
begin
! MICROSOFT MOUSE BIOS
  procedure mouse.x(->int); alien "_mousex"
  ! x position of the mouse

  procedure mouse.y(->int);alien "_mousey"
  ! y position of the mouse

  procedure mouse.buttons (->int);alien "_mousebut"
  ! returns bit to indicate if mouse buttons down

  procedure mouse.present (->bool);alien "_mousepres"
  ! true if mouse dirver installed

  procedure mouse.show ;alien "_mouseshow"
  ! make mouse cursor appear

  procedure mouse.hide ;alien "_mousehide"
  !make mouse cursor vanish
  let men = "menuseg"
  let swap.block =
         import("swap.block")proc(* *int, int ,int -> * *int) segment men
    mouse.show()
    mouse.hide()
    let o=swap.block(menu,x,y)
    mouse.show()
    let y1:=mouse.y()
    while mouse.buttons() = 0 do {
       y1:=mouse.y()
    }
    let x1:=mouse.x() div 8
    y1:=y1 div 8
    mouse.hide()
    let new=swap.block(o,x,y)
    mouse.show()
    let u= upb(menu)
    let l= lwb(menu)
    let w=upb(menu(l))
    if ~(x1>x and x1<(x+w)) then  l-1 else
    if y1>y and y1< (y+u-l) then y1  - y +l
    else l-1
end


?
