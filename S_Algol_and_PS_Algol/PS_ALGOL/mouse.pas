Unit Mouse;

{$R+}

Interface

Uses
  Crt,
  DOS,
  graph;

type cursor_shape = record
                      mask: array [1..32] of word;
                      hot_x,hot_y: word;
                    end;
     mouse_mode_type = (idle,pressed,held,start_drag,dragging,end_drag,clicked);

function reset_mouse(var num_buttons: word): boolean; {}
function mouse_exists: boolean; {}
procedure show_mouse; {}
procedure hide_mouse; {}
procedure show_mouse_moved;
procedure mouse_shape(var shape: cursor_shape); {}
procedure get_mouse_position(var x,y,button: word); {}
procedure set_mouse_position(x,y: word); {}
procedure last_mouse_press(var x,y,count: word; button: word); {}
procedure last_mouse_release(var x,y,count: word; button: word); {}
procedure mouse_limits(xmin,ymin,xmax,ymax: word); {}
function mouse_x_factor: word; {}
function screen_mouse(var num_buttons: word): boolean; {}
procedure test_mouse; {}
procedure test_text_mouse; {}
function mouse_mode(var x1,y1,x2,y2: word): mouse_mode_type; {}

Implementation

var x_factor: word;
    shown,textmouse: boolean;
    mode: mouse_mode_type;
    mouse_ok: boolean;




{--------------------------------------------------------------------------

PROCEDURE    call_mouse

PARAMETERS   service: word; service required
             var ax: word;  register input/output
             var bx: word;  register input/output
             var cx: word;  register input/output
             var dx: word;  register input/output
             var es: word;  register input/output

DESCRIPTION  calls the microsoft mouse interface via interrupt $33

             sets up the registers:
               AX := service
               BX := bx
               CX := cx
               DX := dx
               ES := es

             calls interrupt $33

             returns the register values
               ax := AX
               bx := BX
               cx := CX
               dx := DX
               es := ES

             the calling conventions and services are described in the microsoft
             mouse manual

--------------------------------------------------------------------------}
procedure call_mouse(service: word; var ax,bx,cx,dx,es: word);
  var r: registers;
begin
  r.ax:=service;
  r.bx:=bx;
  r.cx:=cx;
  r.dx:=dx;
  r.es:=es;
  intr($33,r);
  ax:=r.ax;
  bx:=r.bx;
  cx:=r.cx;
  dx:=r.dx;
  es:=r.es;
end;




{--------------------------------------------------------------------------

PROCEDURE    show_mouse;

DESCRIPTION  makes the mouse cursor visible

             calls mouse service 1

             the microsoft service decrements an internal flag and shows the
             mouse if the flag is 0 otherwise it's hidden

             this procedure protects the user from that piece of silly design:
             if you call show_mouse, the mouse is shown full stop!

GLOBALS      shown

--------------------------------------------------------------------------}
procedure show_mouse;
  var a: word;
begin
  if not shown then
    call_mouse(1,a,a,a,a,a);
  shown:=true;
end;




{--------------------------------------------------------------------------

PROCEDURE    show_mouse_moved;

DESCRIPTION  makes the mouse cursor visible if the mouse has moved since this
             was last called

--------------------------------------------------------------------------}
procedure show_mouse_moved;
  const px: word = 10000;
        py: word = 10000;
  var x,y,b: word;
begin
  get_mouse_position(x,y,b);
  if (x <> px) or (y <> py) then
  begin
    show_mouse;
    px:=x; py:=y;
  end;
end;




{--------------------------------------------------------------------------

PROCEDURE    hide_mouse;

DESCRIPTION  hides the mouse cursor

             calls mouse service 2

             the microsoft service decrements an internal flag and shows the
             mouse if the flag is 0 otherwise it's hidden

             this procedure protects the user from that piece of silly design:
             if you call hide_mouse, the mouse is hidden full stop!

GLOBALS      shown

--------------------------------------------------------------------------}
procedure hide_mouse;
  var a: word;
begin
  if shown then
    call_mouse(2,a,a,a,a,a);
  shown:=false;
end;




{--------------------------------------------------------------------------

FUNCTION     reset_mouse

PARAMETERS   var num_buttons: word; returns the number of buttons on the mouse

RESULT       boolean; if a mouse is intalled then true else false

DESCRIPTION  the mouse drivers are reset

             calls mouse service 0

             the x_factor is set to the number of mickeys per pixel

             the cursor is hidden

             textmouse := false; i.e. the cursor is a graphics cursor

             if the reset succeeded then

               return true

             else

               return false

GLOBALS      x_factor
             textmouse
             shown

--------------------------------------------------------------------------}
function reset_mouse(var num_buttons: word): boolean;
  var ax,a: word;
begin
  call_mouse(0,ax,num_buttons,a,a,a);
  mouse_ok:=ax=$FFFF;
  reset_mouse:=mouse_ok;
  if GetMaxX <> 0 then x_factor:=640 div GetMaxX;
  mode:=idle;
  textmouse:=false;
  shown:=false;
end;




{--------------------------------------------------------------------------

PROCEDURE    mouse_shape

PARAMETERS   var shape: cursor_shape;

DESCRIPTION  define the mouse cursor shape

             calls mouse service 9

             the cursor is described in the microsoft mouse manual

             the 'mask' defines the shape of the cursor
             hot_x and hot_y define the cursor hotspot

--------------------------------------------------------------------------}
procedure mouse_shape(var shape: cursor_shape);
  var dx,es: word;
begin
  with shape do
  begin
    es:=seg(mask);
    dx:=ofs(mask);
    call_mouse(9,dx,hot_x,hot_y,dx,es);
  end;
end;




{--------------------------------------------------------------------------

PROCEDURE    get_mouse_position

PARAMETERS   var x: word;      returns the mouse position in pixels or chars
             var y: word;      returns the mouse position in pixels or chars
             var button: word; returns the button states

DESCRIPTION  returns the current mouse position and button states

             is its a graphics cursor then
                returns the position in pixels
              else
                returns the position in screen characters

             calls mouse service 3

             each button is represented by one bit:

               button 1 down:    00000001
               button 2 down:    00000010
               button 3 down:    00000100

             button is the sum of these bits

--------------------------------------------------------------------------}
procedure get_mouse_position(var x,y,button: word);
  var a: word;
begin
  if not mouse_ok then
    begin x:=1; y:=1; button:=0; exit end;

  call_mouse(3,a,button,x,y,a);
  if textmouse then
  begin
    x:=1+x div 8;
    y:=1+y div 8;
  end else
    x:=x div x_factor;
end;




{--------------------------------------------------------------------------

PROCEDURE    set_mouse_position

PARAMETERS   x: word; the mouse position in pixels
             y: word; the mouse position in pixels

DESCRIPTION  sets the current mouse position

             calls mouse service 4

             is its a graphics cursor then
                the position is in pixels
              else
                the position is in screen characters

--------------------------------------------------------------------------}
procedure set_mouse_position(x,y: word);
  var a: word;
begin
  if textmouse then
  begin
    x:=x*8-1;
    y:=y*8-1;
  end else
    x:=x * x_factor;
  call_mouse(4,a,a,x,y,a);
end;




{--------------------------------------------------------------------------

PROCEDURE    last_mouse_press

PARAMETERS   var x: word;      returns the mouse position in pixels or chars
             var y: word;      returns the mouse position in pixels or chars
             var count: word;  returns the number of presses since last called
             button: word;     the button of interest

DESCRIPTION  returns the mouse position when the specified button was last
             pressed

             is its a graphics cursor then
                returns the position in pixels
              else
                returns the position in screen characters

             calls mouse service 5

             the button of interest is 1, 2, 3, etc.

--------------------------------------------------------------------------}
procedure last_mouse_press(var x,y,count: word; button: word);
  var a: word;
begin
  if not mouse_ok then
    begin x:=1; y:=1; count:=0; button:=0; exit end;

  call_mouse(5,button,count,x,y,a);
  if textmouse then
  begin
    x:=1+x div 8;
    y:=1+y div 8;
  end else
    x:=x div x_factor;
end;




{--------------------------------------------------------------------------

PROCEDURE    last_mouse_release

PARAMETERS   var x: word;      returns the mouse position in pixels or chars
             var y: word;      returns the mouse position in pixels or chars
             var count: word;  returns the number of releases since last called
             button: word;     the button of interest

DESCRIPTION  returns the mouse position when the specified button was last
             released

             is its a graphics cursor then
                returns the position in pixels
              else
                returns the position in screen characters

             calls mouse service 6

             the button of interest is 1, 2, 3, etc.

--------------------------------------------------------------------------}
procedure last_mouse_release(var x,y,count: word; button: word);
  var a: word;
begin
  if not mouse_ok then
    begin x:=1; y:=1; count:=0; button:=0; exit end;

  call_mouse(6,button,count,x,y,a);
  if textmouse then
  begin
    x:=1+x div 8;
    y:=1+y div 8;
  end else
    x:=x div x_factor;
end;




{--------------------------------------------------------------------------

PROCEDURE    mouse_limits

PARAMETERS   xmin: word; limit of mouse cursor in pixels or chars
             ymin: word; limit of mouse cursor in pixels or chars
             xmax: word; limit of mouse cursor in pixels or chars
             ymax: word; limit of mouse cursor in pixels or chars

DESCRIPTION  sets the mouse limits

             calls mouse service 7 and 8

             is its a graphics cursor then
                the position is in pixels
              else
                the position is in screen characters

--------------------------------------------------------------------------}
procedure mouse_limits(xmin,ymin,xmax,ymax: word);
  var a: word;
begin
  if textmouse then
  begin
    xmin:=xmin*8-1;
    xmax:=xmax*8-1;
    ymin:=ymin*8-1;
    ymax:=ymax*8-1;
  end else
  begin
    xmin:=xmin * x_factor;
    xmax:=xmax * x_factor;
  end;
  call_mouse(7,a,a,xmin,xmax,a);
  call_mouse(8,a,a,ymin,ymax,a);
end;




{--------------------------------------------------------------------------

FUNCTION     mouse_x_factor

RESULT       word; the x_factor

DESCRIPTION  returns the mouse xfactor in mickeys per pixel

--------------------------------------------------------------------------}
function mouse_x_factor: word;
begin
  mouse_x_factor:=x_factor;
end;




{--------------------------------------------------------------------------

FUNCTION     mouse_x_factor

RESULT       true if the mouse has been reset ok else false;

DESCRIPTION  returns whetheer the mouse drivers exist and have
             been reset

--------------------------------------------------------------------------}
function mouse_exists: boolean; {}
begin
  mouse_exists:=mouse_ok;
end;



{--------------------------------------------------------------------------

FUNCTION     screen_mouse

PARAMETERS   var num_buttons: word; returns the number of buttons on the mouse

RESULT       boolean; if a mouse is intalled then true else false

DESCRIPTION  the mouse drivers are reset

             calls mouse service 0 and 10

             the x_factor is set to 1

             the text cursor is set to invert the colour of the character

             the cursor is shown

             textmouse := true; i.e. the cursor is a text cursor

             if the reset succeeded then

               return true

             else

               return false

GLOBALS      x_factor
             textmouse
             shown

--------------------------------------------------------------------------}
function screen_mouse(var num_buttons: word): boolean;
  var ax,bx,and_mask,xor_mask,es: word;
begin
  call_mouse(0,ax,num_buttons,bx,bx,bx);
  mouse_ok:=ax=$FFFF;
  screen_mouse:=mouse_ok;
  bx:=0;
  and_mask:=$FFFF;
  xor_mask:=$7700;
  call_mouse(10,ax,bx,and_mask,xor_mask,es);

  mode:=idle;
  x_factor:=1;
  shown:=false;
  show_mouse;

  textmouse:=true;
end;




{--------------------------------------------------------------------------

FUNCTION     mouse_mode

PARAMETERS   var x1,y1: word; start drag position
             var x2,y2: word; end drag position

RESULT       mouse_mode_type;

DESCRIPTION  returns current mouse status, e.g.

                   mouse action      mouse_mode    x1,y1        x2,y2
                   ------------      ----------    -----        -----
                                     idle          current pos  current pos
                                     idle          current pos  current pos
                                     idle          current pos  current pos
                   move
                                     idle          current pos  current pos
                   move
                                     idle          current pos  current pos
                                     idle          current pos  current pos
                   press
                                     pressed       press pos    current pos
                                     held          press pos    current pos
                                     held          press pos    current pos
                                     held          press pos    current pos
                   release
                                     clicked       press pos    release pos
                                     idle          current pos  current pos
                                     idle          current pos  current pos
                                     idle          current pos  current pos
                   press
                                     pressed       press pos    current pos
                                     held          press pos    current pos
                                     held          press pos    current pos
                                     held          press pos    current pos
                   move
                                     start_drag    press pos    current pos
                                     dragging      press pos    current pos
                                     dragging      press pos    current pos
                   move
                                     dragging      press pos    current pos
                                     dragging      press pos    current pos
                   release
                                     end_drag      press pos    release pos
                                     idle          current pos  current pos
                                     idle          current pos  current pos

             x1,y1 return the current mouse position or the position it was
             in when the button was last pressed

             x2,y2 return the current mouse position or the position it was
             in when the button was last released

             the routine polls the mouse rather than using the
             last_mouse_press and last_mouse_release procedures

--------------------------------------------------------------------------}
function mouse_mode(var x1,y1,x2,y2: word): mouse_mode_type;
  var x,y,b,c: word;
  const moved: boolean = false;
        prev_b:    boolean = false;
        last_press_x:   word = 0;
        last_press_y:   word = 0;
        last_release_x: word = 0;
        last_release_y: word = 0;
begin
  Get_Mouse_position(x,y,B);

  if (b and 1) = 0 then
  begin
    if prev_b then
    begin
      last_release_x:=x;
      last_release_y:=y;
    end;
    prev_b:=false;

    case mode of
      pressed,
      held,
      start_drag,
      dragging: begin
              x1:=last_press_x;
              y1:=last_press_y;
              x2:=last_release_x;
              y2:=last_release_y;
              if (x1 <> x2) or (y1 <> y2) then moved:=true;
              if moved then
                mode:=end_drag else
                mode:=clicked;
            end;
      else  begin
              x1:=x;
              y1:=y;
              x2:=x;
              y2:=y;
              mode:=idle;
            end;
    end;
  end else
  begin
    if not prev_b then
    begin
      last_press_x:=x;
      last_press_y:=y;
    end;
    prev_b:=true;

    case mode of
      idle: begin
              x1:=last_press_x;
              y1:=last_press_y;
              x2:=x;
              y2:=y;
              mode:=pressed;
              moved:=false;
            end;
      else  begin
              x1:=last_press_x;
              y1:=last_press_y;
              x2:=x;
              y2:=y;
              if (x1 <> x2) or (y1 <> y2) then moved:=true;
              if not moved then
                mode:=held else
              if mode = held then
                mode:=start_drag else
                mode:=dragging;
            end;
    end;
  end;
  mouse_mode:=mode;
end;

{--------------------------------------------------------------------------

PROCEDURE    test_mouse;

DESCRIPTION  a short test program

             resets the mouse to a graphics cursor

--------------------------------------------------------------------------}
procedure test_mouse;
  var x,y,b,ax,num_buttons,a,x1,y1,x2,y2: word;
begin
  if not reset_mouse(num_buttons) then
    begin writeln('Error on Reset'); halt; end;
  x_factor:=1;
  clrscr;
  repeat
    Get_Mouse_position(x,y,B);
    gotoxy(1,1);
    writeln(x:10,y:10,b:10);
    case mouse_mode(x1,y1,x2,y2) of
      idle:     writeln('idle     ');
      pressed:  begin
                  writeln('pressed    ');
                  writeln(x1:10,y1:10);
                  writeln(x2:10,y2:10);
                  delay(500);
                end;
      held:     begin
                  writeln('held       ');
                  writeln(x1:10,y1:10);
                  writeln(x2:10,y2:10);
                end;
      start_drag: begin
                  writeln('start_drag ');
                  writeln(x1:10,y1:10);
                  writeln(x2:10,y2:10);
                  delay(500);
                end;
      dragging: begin
                  writeln('dragging   ');
                  writeln(x1:10,y1:10);
                  writeln(x2:10,y2:10);
                end;
      end_drag:  begin
                  writeln('end_drag    ');
                  writeln(x1:10,y1:10);
                  writeln(x2:10,y2:10);
                  delay(500);
                end;
      clicked:  begin
                  writeln('clicked    ');
                  writeln(x1:10,y1:10);
                  writeln(x2:10,y2:10);
                  delay(500);
                end;
    end;
  until keypressed;
end;





{--------------------------------------------------------------------------

PROCEDURE    test_text_mouse;

DESCRIPTION  a short test program

             resets the mouse to a text cursor

             shows the text cursor

             moves the current writing position whenever the left button is
             pressed

             repeats until Ctrl-C is pressed

             writes the mouse position and button state until a key is pressed

             drags a single line box

             on release: draws a double line box

--------------------------------------------------------------------------}
procedure test_text_mouse;
  procedure erase_box(x1,y1,x2,y2: integer);
    var x,y: integer;
  begin
    if (x1 = x2) or (y1 = y2) then exit;
    if x1 > x2 then begin x:=x1; x1:=x2; x2:=x; end;
    if y1 > y2 then begin y:=y1; y1:=y2; y2:=y; end;
    gotoxy(x1,y1);
    for x:=x1 to x2 do write(' ');
    for y:=y1+1 to y2-1 do
    begin
      gotoxy(x1,y); write(' ');
      gotoxy(x2,y); write(' ');
    end;
    gotoxy(x1,y2);
    for x:=x1 to x2 do write(' ');
  end;

  procedure draw_box_2(x1,y1,x2,y2: integer);
    var x,y: integer;
  begin
    if (x1 = x2) or (y1 = y2) then exit;
    if x1 > x2 then begin x:=x1; x1:=x2; x2:=x; end;
    if y1 > y2 then begin y:=y1; y1:=y2; y2:=y; end;
    gotoxy(x1,y1);
    write('É');
    for x:=x1+1 to x2-1 do write('Í');
    write('»');
    for y:=y1+1 to y2-1 do
    begin
      gotoxy(x1,y); write('º');
      gotoxy(x2,y); write('º');
    end;
    gotoxy(x1,y2);
    write('È');
    for x:=x1+1 to x2-1 do write('Í');
    write('¼');
  end;

  procedure draw_box_1(x1,y1,x2,y2: integer);
    var x,y: integer;
  begin
    if (x1 = x2) or (y1 = y2) then exit;
    if x1 > x2 then begin x:=x1; x1:=x2; x2:=x; end;
    if y1 > y2 then begin y:=y1; y1:=y2; y2:=y; end;
    gotoxy(x1,y1);
    write('Ú');
    for x:=x1+1 to x2-1 do write('Ä');
    write('¿');
    for y:=y1+1 to y2-1 do
    begin
      gotoxy(x1,y); write('³');
      gotoxy(x2,y); write('³');
    end;
    gotoxy(x1,y2);
    write('À');
    for x:=x1+1 to x2-1 do write('Ä');
    write('Ù');
  end;

  var
    px,py,x,y,b,x1,y1,x2,y2,px1,py1,px2,py2: word;
    c: char;
begin
  clrscr;
  if not screen_mouse(b) then write('mouse not found');
  get_mouse_position(px,py,b);

  repeat
    hide_mouse;
    repeat
      get_mouse_position(x,y,b);
      if (x <> px) or (y <> py) then show_mouse;
      case mouse_mode(x1,y1,x2,y2) of
        start_drag,
        dragging: begin
                    erase_box(px1,py1,px2,py2);
                    draw_box_1(x1,y1,x2,y2);
                    hide_mouse;
                    gotoxy(x,y);
                  end;
        end_drag:  begin
                    erase_box(px1,py1,px2,py2);
                    draw_box_2(x1,y1,x2,y2);
                    hide_mouse;
                    gotoxy(x,y);
                  end;
        clicked:  begin
                    hide_mouse;
                    x:=x1;
                    y:=y1;
                    gotoxy(x,y);
                  end;
      end;

{
gotoxy(1,1);
writeln(x1:10,y1:10);
writeln(x2:10,y2:10);
{}
      px:=x; py:=y;
      px1:=x1; py1:=y1;
      px2:=x2; py2:=y2;
    until keypressed;
    c:=readkey;
    write(c);
  until c = #3;
end;

begin
  mouse_ok:=false;
end.

/////////////////////////////////////////////////////


