{$B-,F-,I-,R+,S+,V+}

unit menu;

interface

uses
  crt,
  graph,
  screen,
  mouse;

type
  intptr = ^integer;
  realptr = ^real;
  buf_array = array[0..3000] of word;
  buf_array_ptr = ^buf_array;
  choicerecptr = ^choicerec;
  choicerec = record
      name: string[30];
      ans: integer;
      int_loc: intptr;
      real_loc: realptr;
      real_interval: integer;
      gauge_min,gauge_max: real;
      next: choicerecptr;
    end;
  menu_header = record
      menu_kind: (normal_menu,bar_menu,big_menu);
      num_items,page_start,x1,y1,x2,y2: integer;
      title: string[50];
      ScreenBuf: buf_array_ptr;
      choicelist: choicerecptr;
    end;

var
  entry,frame,select,in_box,normal: byte;
  AutoHelp: boolean;    { automatic selection of help screens }

const
  HelpFile: string = 'HELP.MSG';     { help message file }

procedure
 start_menu(x,y: integer; var menu: menu_header);
procedure start_menu_bar(y: integer; var menu: menu_header);
procedure menu_item(s: string; answer: integer; var menu: menu_header);
procedure int_gauge_item(s: string; answer: integer; var gauge: integer;
  min,max: integer; var menu: menu_header);
procedure real_gauge_item(s: string; answer: integer; var gauge: real;
  min,max: real; interval: integer; var menu: menu_header);
procedure change_menu_item(s: string; answer: integer; var menu: menu_header);
procedure change_gauge_item(answer: integer; var menu: menu_header);
procedure OnOff_menu_item(b: boolean; answer: integer; var menu: menu_header);
procedure menu_title(s: string; var menu: menu_header);
procedure hide_menu(var menu: menu_header);
procedure exec_menu(var cx,cy: integer; var answer: integer;
  var mmenu: menu_header);
procedure show_menu(var cx,cy: integer; c: integer; var menu: menu_header);
function string_box(cx,cy,len: integer; var ans: string; title: string):
  boolean;
function integer_box(cx,cy,len: integer; var ans: longint; title: string):
  boolean;
function real_box(cx,cy,len: integer; var ans: real; title: string):
  boolean;
function yes_or_no(cx,cy: integer; title,msg: string): char;
procedure message(cx,cy: integer; msg,title: string);
procedure draw_box(var cx,cy: integer; len,hgt: integer; title: string);
procedure make_box(var cx,cy: integer; len,hgt: integer; title: string;
  var buf: buf_array_ptr);
procedure delete_box(var buf: buf_array_ptr);
procedure Help_screen;
procedure push_Help_Page(h: integer);
procedure pop_Help_Page;

implementation

{======================================================================}
{ menu procedures
{======================================================================}

const
  alt_chars: array ['0'..'Z'] of char =
    (Alt_0,   {0}
     Alt_1,   {1}
     Alt_2,   {2}
     Alt_3,   {3}
     Alt_4,   {4}
     Alt_5,   {5}
     Alt_6,   {6}
     Alt_7,   {7}
     Alt_8,   {8}
     Alt_9,   {9}
     #255,
     #255,
     #255,
     #255,
     #255,
     #255,
     #255,
     Alt_A,    {a}
     Alt_B,    {b}
     Alt_C,    {c}
     Alt_D,    {d}
     Alt_E,    {e}
     Alt_F,    {f}
     Alt_G,    {g}
     Alt_H,    {h}
     Alt_I,    {i}
     Alt_J,    {j}
     Alt_K,    {k}
     Alt_L,    {l}
     Alt_M,    {m}
     Alt_N,    {n}
     Alt_O,    {o}
     Alt_P,    {p}
     Alt_Q,    {q}
     Alt_R,    {r}
     Alt_S,    {s}
     Alt_T,    {t}
     Alt_U,    {u}
     Alt_V,    {v}
     Alt_W,    {w}
     Alt_X,    {x}
     Alt_Y,    {y}
     Alt_Z);   {z}
  box_max_y = 15;
  big_width = 60;
  big_height = 15;
  mouse_select = #28;                { used by string_box routines }

var Help_Page: array[1..10] of integer;
    mouse_button: mouse_mode_type;
    mouse_x,mouse_y: word;

{--------------------------------------------------------------------------
{
{ procedure start_menu
{
{ Parameters x: integer; coord of top left of menu
{            y: integer; coord of top left of menu
{            var menu: menu_header;
{
{ Globals   Help_Page
{
{ Start a new box menu
{ if it is off the screen the cx,cy will be adjusted accordingly
{ menu.ScreenBuf:=nil
{
{--------------------------------------------------------------------------}
procedure start_menu(x,y: integer; var menu: menu_header);
begin
  with menu do
  begin
    menu_kind:=normal_menu;
    x1:=x; y1:=y;
    x2:=x; y2:=y+1;
    num_items:=0;
    title:='';
    ScreenBuf:=nil;
    choicelist:=nil;
  end;
  push_Help_Page(Help_Page[1]);
end;

{--------------------------------------------------------------------------
{
{ procedure start_menu_bar
{
{ Parameters y: integer; not used - retained for compatibility
{            var menu: menu_header;
{
{ Globals   Help_Page
{
{ Start a new bar menu
{ it will appear on the top line
{ menu.ScreenBuf:=nil
{
{--------------------------------------------------------------------------}
procedure start_menu_bar(y: integer; var menu: menu_header);
begin
  with menu do
  begin
    menu_kind:=bar_menu;
    y1:=y;
    num_items:=0;
    title:='';
    ScreenBuf:=nil;
    choicelist:=nil;
  end;
  push_Help_Page(Help_Page[1]);
end;

{--------------------------------------------------------------------------
{
{ function gauge_str
{
{ Parameters min: integer; min value of guage
{            max: integer; max value of guage
{            gauge: integer; gauge value
{
{ Return     string;
{
{ Globals    calculates a string representing a gauge value
{
{--------------------------------------------------------------------------}
function gauge_str(min,max,gauge: integer): string;
  var s: string;
begin
  s:='';
  while length(s) <= max-min do s:=s+'ฤ';
  if min = 0 then                 s[1-min]:='ฺ' else
  if max = 0 then                 s[1-min]:='ฟ' else
  if (min < 0) and (max > 0) then s[1-min]:='ย';
  if gauge < min then
    s[1]:='<' else
  if gauge > max then
    s[max+1-min]:='>' else
  if (gauge >= min) and (gauge <= max) then
    s[gauge+1-min]:='ล';
  gauge_str:=s;
end;

{--------------------------------------------------------------------------
{
{ procedure make_menu_item
{
{ Parameters s: string;           item name
{            answer: integer;     answer value
{            int_gauge: intptr;   pointer to integer dependent gauge
{            real_gauge: realptr; pointer to real dependent gauge
{            min: real            min gauge value
{            max: real;           max gauge value
{            interval: integer;   gauge interval
{            var menu: menu_header;
{
{ makes a new menu item of any type
{
{--------------------------------------------------------------------------}
procedure make_menu_item(s: string; answer: integer; int_gauge: intptr;
    real_gauge: realptr; min,max: real; interval: integer;
    var menu: menu_header);
  var choice: choicerecptr;
      x: integer;
  procedure add(var p: choicerecptr);
  begin
    if p = nil then p:=choice else add(p^.next);
  end;
begin
  new(choice);
  with menu do
  with choice^ do
  begin
    if screenbuf <> nil then halt; {runerror(0); {}
    inc(num_items);
    if int_gauge <> nil then s:=s+gauge_str(trunc(min),trunc(max),0);
    if real_gauge <> nil then s:=s+gauge_str(0,interval-1,0);
    if menu_kind <> bar_menu then
    begin
      inc(y2);
      if y2 > y1+box_max_y+1 then y2:=y1+box_max_y+1;
      x:=x1+length(s)+1; if x > x2 then x2:=x;
      if y2 > 25 then begin y1:=y1+25-y2; y2:=25; end;
      if x2 > 80 then begin x1:=x1+80-x2; x2:=80; end;
      if num_items > box_max_y then menu_kind:=big_menu;
    end;
    name:=s;
    ans:=answer;
    gauge_min:=min;
    gauge_max:=max;
    int_loc:=int_gauge;
    real_loc:=real_gauge;
    real_interval:=interval;
    add(choicelist);
    next:=nil;
  end;
end;

{--------------------------------------------------------------------------
{
{ procedure menu_item
{
{ Parameters s: string;
{            answer: integer;
{            var menu: menu_header;
{
{ makes a new simple menu item
{
{--------------------------------------------------------------------------}
procedure menu_item(s: string; answer: integer; var menu: menu_header);
begin
  make_menu_item(s,answer,nil,nil,0,-1,0,menu);
end;

{--------------------------------------------------------------------------
{
{ procedure int_gauge_item
{
{ Parameters s: string;           item name
{            answer: integer;     answer value
{            var gauge: integer;  dependent gauge variable
{            min: real            min gauge value
{            max: real;           max gauge value
{            var menu: menu_header;
{
{ makes a new menu item of integer gauge type
{
{--------------------------------------------------------------------------}
procedure int_gauge_item(s: string; answer: integer; var gauge: integer;
  min,max: integer; var menu: menu_header);
begin
  make_menu_item(s,answer,addr(gauge),nil,min,max,0,menu);
end;

{--------------------------------------------------------------------------
{
{ procedure real_gauge_item
{
{ Parameters s: string;           item name
{            answer: integer;     answer value
{            var gauge: real;     dependent gauge variable
{            min: real            min gauge value
{            max: real;           max gauge value
{            interval: integer;   interval of gauge value
{            var menu: menu_header;
{
{ makes a new menu item of real gauge type
{
{--------------------------------------------------------------------------}
procedure real_gauge_item(s: string; answer: integer; var gauge: real;
  min,max: real; interval: integer; var menu: menu_header);
begin
  make_menu_item(s,answer,nil,addr(gauge),min,max,interval,menu);
end;

{--------------------------------------------------------------------------
{
{ procedure change_menu_item
{
{ Parameters s: string;           new item name
{            answer: integer;     answer value
{            var menu: menu_header;
{
{ change an existing simple gauge item
{ the new name will be truncated or padded to the length of the
{ original name
{
{--------------------------------------------------------------------------}
procedure change_menu_item(s: string; answer: integer; var menu: menu_header);
  var choice: choicerecptr;
begin
  choice:=menu.choicelist;
  while choice <> nil do
  with menu,choice^ do
  begin
    if ans = answer then name:=copy(s,1,x2-x1);
    choice:=next;
  end;
end;

{--------------------------------------------------------------------------
{
{ procedure calc_real_gauge
{
{ Parameters choice: choicerecptr;
{            var g_v: integer;
{            var g_i_max: integer;
{            var g_i_min: integer;
{            var g_a: real;
{            var g_b: real;
{
{ used internally
{ calculates the gauge coefficients ready for calling gauge_str:
{
{      s:=gauge_str(trunc(g_i_min),trunc(g_i_max),g_v;
{
{ or for calculating the real value given an integer gauge value:
{
{      real_loc^:=g_a+(g_v+0.5)*g_b;
{
{--------------------------------------------------------------------------}
procedure calc_real_gauge(choice: choicerecptr;
      var g_v,g_i_max,g_i_min: integer;
      var g_a,g_b: real);
begin
  with choice^ do
  begin
    g_i_max:=trunc(gauge_max*(real_interval-1)/(gauge_max-gauge_min)+0.5);
    g_i_min:=g_i_max-real_interval+1;
    g_b:=(gauge_max-gauge_min)/(g_i_max-g_i_min);
    g_a:=gauge_min-(g_i_min+0.5)*g_b;
    if odd(g_i_max-g_i_min) then
      g_v:=trunc((abs(real_loc^)-g_a)/g_b-g_i_min -0.5)+g_i_min else
      g_v:=trunc((abs(real_loc^)-g_a)/g_b-g_i_min)+g_i_min;
    if real_loc^ < 0 then g_v:=-g_v;
  end;
end;

{--------------------------------------------------------------------------
{
{ procedure change_gauge_item
{
{ Parameters answer: integer;
{            var menu: menu_header;
{
{ changes the gauge string part of an existing gauge item
{
{--------------------------------------------------------------------------}
procedure change_gauge_item(answer: integer; var menu: menu_header);
  var choice: choicerecptr;
      s: string;
      g_v,g_i_max,g_i_min: integer;
      g_a,g_b: real;
begin
  choice:=menu.choicelist;
  while choice <> nil do
  with menu,choice^ do
  begin
    if (ans = answer) and (int_loc <> nil) then
    begin
      s:=gauge_str(trunc(gauge_min),trunc(gauge_max),int_loc^);
      name:=copy(name,1,length(name)-length(s))+s;
    end;
    if (ans = answer) and (real_loc <> nil) then
    begin
      calc_real_gauge(choice,g_v,g_i_max,g_i_min,g_a,g_b);
      s:=gauge_str(trunc(g_i_min),trunc(g_i_max),g_v);
      name:=copy(name,1,length(name)-length(s))+s;
    end;
    choice:=next;
  end;
end;

{--------------------------------------------------------------------------
{
{ procedure change_all_gauges
{
{ Parameters var menu: menu_header;
{
{ changes the gauge string part of all existing gauge items
{
{--------------------------------------------------------------------------}
procedure change_all_gauges(var menu: menu_header);
  var choice: choicerecptr;
begin
  choice:=menu.choicelist;
  while choice <> nil do
  with menu,choice^ do
  begin
    if (int_loc <> nil) or (real_loc <> nil) then
      change_gauge_item(ans,menu);
    choice:=next;
  end;
end;

{--------------------------------------------------------------------------
{
{ procedure OnOff_menu_item
{
{ Parameters b: boolean;
{            answer: integer;
{            var menu: menu_header;
{
{  changes an existing item
{
{ if b is true then
{    replaces the last three characters of the name with ' On'
{ else
{    replaces the last three characters of the name with 'Off'
{
{--------------------------------------------------------------------------}
procedure OnOff_menu_item(b: boolean; answer: integer; var menu: menu_header);
  var choice: choicerecptr;
begin
  choice:=menu.choicelist;
  while choice <> nil do
  with menu,choice^ do
  begin
    if ans = answer then
    begin
      while length(name) < x2-x1-4 do name:=name+' ';
      name:=copy(name,1,x2-x1-4);
      if b then name:=name+' On' else name:=name+'Off';
    end;
    choice:=next;
  end;
end;

{--------------------------------------------------------------------------
{
{ procedure menu_title
{
{ Parameters s: string;             the title
{            var menu: menu_header; the menu
{
{ sets the menu title
{
{ the menu title is only shown on big_box menus
{
{--------------------------------------------------------------------------}
procedure menu_title(s: string; var menu: menu_header);
begin
  menu.title:=s;
end;

{--------------------------------------------------------------------------
{
{ procedure get_box(
{
{ Parameters cx: integer;            topleft corner of box
{            cy: integer;            topleft corner of box
{            len: integer;           width of box
{            hgt: integer;           height of box
{            var buf: buf_array_ptr; buffer for saved area
{
{ saves an area of the screen into a buffer
{ the buffer remembers:
{    the size and location of the area
{    the old cursor position
{    the old window setting
{ space for the buffer is created on the heap
{
{--------------------------------------------------------------------------}
procedure get_box(cx,cy: integer; len,hgt: integer; var buf: buf_array_ptr);
  var x,y,i,s: integer;
      x1,y1,x2,y2: byte;
begin
  s:=(hgt*len+14)*2;
  getmem(buf,s);
  for i:=0 to hgt-1 do
    moveFromScreen(ScreenPtr^[cy+i,cx],Buf^[i*len+11],len*2);
  buf^[0]:=wherex;
  buf^[1]:=wherey;
  buf^[2]:=cx;
  buf^[3]:=cy;
  buf^[4]:=len;
  buf^[5]:=hgt;
  buf^[6]:=s;
  get_window(x1,y1,x2,y2);
  buf^[7] :=x1;
  buf^[8] :=y1;
  buf^[9] :=x2;
  buf^[10]:=y2;
end;

{--------------------------------------------------------------------------
{
{ procedure make_box
{
{ Parameters var cx: integer;
{            var cy: integer;
{            len: integer;
{            hgt: integer;
{            title: string;
{            var buf: buf_array_ptr;
{
{ stores the contents of the screen in buf
{ draws a double-walled box with a title
{ (the inside of the box measures len x height not the outside)
{ cx,cy is the topleft corner of the wall
{
{ sets the current window to the inside of the box
{
{--------------------------------------------------------------------------}
procedure make_box(var cx,cy: integer; len,hgt: integer; title: string;
    var buf: buf_array_ptr);
begin
  if cx > 78-len then cx:=78-len;
  if cy > 23-hgt then cy:=23-hgt;
  get_box(cx,cy,len+2,hgt+2,buf);
  draw_box(cx,cy,len,hgt,title);
  window(cx+1,cy+1,cx+len,cy+hgt);
end;

{--------------------------------------------------------------------------
{
{ procedure delete_box
{
{ Parameters var buf: buf_array_ptr; buffer for saved area
{
{ restores an area of the screen from a buffer
{ buffer must have been set up by get_box or make_box
{ the buffer has remembered:
{    the size and location of the area
{    the old cursor position
{    the old window setting
{ the space for the buffer on the heap is disposed
{
{--------------------------------------------------------------------------}
procedure delete_box(var buf: buf_array_ptr);
  var x,y,i,s: integer;
      cx,cy,len,hgt: integer;
begin
  if buf = nil then exit;
  window(buf^[7],buf^[8],buf^[9],buf^[10]);
  gotoxy(
       buf^[0],
       buf^[1]);
  cx:= buf^[2];
  cy:= buf^[3];
  len:=buf^[4];
  hgt:=buf^[5];
  s  :=buf^[6];
  for i:=0 to hgt-1 do
    moveToScreen(Buf^[i*len+11],ScreenPtr^[cy+i,cx],len*2);
  freemem(buf,s);
  buf:=nil;
end;

{--------------------------------------------------------------------------
{
{ procedure hide_menu
{
{ Parameters var menu: menu_header;
{
{ deletes the menu from the screen and restores what was underneath
{ disposes all the data structures of the menu
{ menu.ScreenBuf:=nil
{ can be called any number of times: ignores subsequent calls
{
{--------------------------------------------------------------------------}
procedure hide_menu(var menu: menu_header);
  var i: integer;
      choice,temp: choicerecptr;
begin
  with menu do
  begin
    if ScreenBuf <> nil then
    begin
      if mouse_exists then hide_mouse; {}
      delete_box(ScreenBuf);
      choice:=choicelist;
      while choice <> nil do
      begin
        temp:=choice^.next;
        dispose(choice);
        choice:=temp;
      end;
      pop_Help_Page;
    end;
    ScreenBuf:=nil
  end;
end;

{--------------------------------------------------------------------------
{
{ function mouseat
{
{ Parameters x: integer; coord of string
{            y: integer; coord of string
{            c: integer; colour of string
{            s: string;  string to be printed
{
{ Return     boolean;
{
{ Globals    mouse_button
{            mouse_x
{            mouse_y
{
{ same as writeat
{ writes a string on the screen
{ returns true if the mouse cursor is in the string
{
{--------------------------------------------------------------------------}
function mouseat(x,y,c: integer; s: string): boolean;
var  mmouseat: boolean;
begin
  writeat(x,y,c,s);
  if mouse_exists then
  begin
    mmouseat:=(mouse_button = clicked) and
      (y = mouse_y) and (mouse_x >= x) and (mouse_x < x+length(s));
  end else
    mmouseat:=false;
mouseat:=mmouseat;
if mmouseat then
begin
  mmouseat:=true;
end;
end;

{--------------------------------------------------------------------------
{
{ function show_bar
{
{ Parameters c: integer;      answer which should be illuminted
{            var cx: integer; coord of result
{            var cy: integer; coord of result
{            var menu: menu_header;
{
{ Return    boolean;
{
{ Globals   Help_Page
{           autohelp
{
{ draws a menu bar at the top of the screen
{ the answer = c item is shown selected
{ cx,cy returns the coords of the first char of the selected item
{
{ if the mouse has been clicked in one of the items then
{    c returns answer selected item
{    function returns true
{ else
{    function returns false
{
{--------------------------------------------------------------------------}
function show_bar(var c: integer; var cx,cy: integer; var menu: menu_header):
    boolean;
  var s: string[80];
      i,j,l: integer;
      choice,t: choicerecptr;
begin
  with menu do
  begin
    show_bar:=false;
    if ScreenBuf = nil then
      get_box(1,1,80,1,screenbuf);

    l:=screen_width;
    choice:=choicelist;
    while choice <> nil do
    with choice^ do
    begin
      l:=l-length(name);
      choice:=next;
    end;

    s:='';
    for i:=1 to l div (num_items+1) do begin s:=s+' '; dec(l); end;

    j:=num_items;
    choice:=choicelist;
    while choice <> nil do
    with choice^ do
    begin
      if mouse_exists and (mouse_button = clicked) and
          (mouse_y = 1) and (mouse_x > length(s)) and
          (mouse_x <= length(s)+length(name)) then
      begin
        show_bar:=true;
        c:=num_items-j+1;
      end;

      if c = num_items-j+1 then
      begin
        t:=choice;
        cx:=length(s)+1; cy:=y1;
        if AutoHelp then Help_Page[1]:=ans;
      end;
      s:=s+name;
      for i:=1 to l div j do begin s:=s+' '; dec(l); end;
      dec(j);
      choice:=next;
    end;

    writeat(1,1,in_box,s);
    if c <> 0 then
      with t^ do writeat(cx,cy,select,name);
  end;
end;

{--------------------------------------------------------------------------
{
{ function page_size
{
{ Parameters var mmenu: menu_header;
{
{ Return     integer;
{
{ used for big-box menus only
{ returns the number of items which will fit on one page
{
{--------------------------------------------------------------------------}
function page_size(var mmenu: menu_header): integer;
begin
  with mmenu do
  begin
    page_size:=big_height*(big_width div (x2-x1+1));
  end;
end;

{--------------------------------------------------------------------------
{
{ function show_big_box
{
{ Parameters var c: integer;  selected item
{            var cx: integer; coord of selected item
{            var cy: integer; coord of selected item
{            var mmenu: menu_header;
{
{ Return    boolean;
{
{ Globals   Help_Page
{
{ show a box menu as a big-box
{ prints the title of the menu
{ the answer = c item will be shown selected
{ cx,cy returns the coords of the first char of the selected item
{
{ if the mouse has been clicked in one of the items then
{    c returns answer selected item
{    function returns true
{ else
{    function returns false
{
{--------------------------------------------------------------------------}
function show_big_box(var c,cx,cy: integer; var mmenu: menu_header):
    boolean;
  var s: string[80];
      bx,by,xs,ys,i,j,l,n,k,cnt: integer;
      choice,t: choicerecptr;
      fin,shown: boolean;
      old_win_xmin,old_win_ymin,old_win_xmax,old_win_ymax: byte;
begin
  with mmenu do
  begin
    show_big_box:=false;
    get_window(old_win_xmin,old_win_ymin,old_win_xmax,old_win_ymax);
    bx:=(screen_width-big_width) div 2;
    by:=(screen_height-big_height) div 2;
    if ScreenBuf = nil then
      make_box(bx,by,big_width,big_height,title,ScreenBuf);

    repeat
      choice:=choicelist;
      for k:=2 to page_start do
        if choice <> nil then
          choice:=choice^.next;
      k:=page_start;
      xs:=1;
      ys:=1;
      shown:=false;

      repeat
        fin:=false;
        if choice = nil then
        begin
          s:='';
          while length(s) < x2-x1-1 do s:=s+' ';
          writeat(xs+bx,ys+by,in_box,s);
        end else
        with choice^ do
        begin
          s:=name;
          while length(s) < x2-x1-1 do s:=s+' ';
          if c = k then
          begin
            if mouseat(xs+bx,ys+by,select,s) then
              show_big_box:=true;
            if AutoHelp then Help_Page[1]:=ans;
            cx:=xs+bx; cy:=ys+by;
            shown:=true;
          end else
          if mouseat(xs+bx,ys+by,in_box,s) then
          begin
            show_big_box:=true;
            if AutoHelp then Help_Page[1]:=ans;
            cx:=xs+bx; cy:=ys+by;
            c:=k;
          end;
          inc(k); if k > num_items then k:=1;
          choice:=next;
        end;
        inc(ys);
        if ys > big_height then
        begin
          ys:=1;
          xs:=xs+x2-x1;
          if xs+x2-x1+1 > big_width then fin:=true;
        end;
      until fin;
      if not shown then
        if c < page_start then
        begin
          if (menu_kind = big_menu) and (page_start > page_size(mmenu)) then
          page_start:=page_start-page_size(mmenu);
        end else
        begin
          if (menu_kind = big_menu) and (page_start <= num_items-page_size(mmenu)) then
          page_start:=page_start+page_size(mmenu);
        end;
    until shown;
    window(old_win_xmin,old_win_ymin,old_win_xmax,old_win_ymax);
  end;
end;

{--------------------------------------------------------------------------
{
{ function show_small_box
{
{ Parameters var c: integer;  selected item
{            var cx: integer; coord of selected item
{            var cy: integer; coord of selected item
{            var mmenu: menu_header;
{
{ Return    boolean;
{
{ Globals   Help_Page
{
{ show a box menu as a small-box
{ the answer = c item will be shown selected
{ cx,cy returns the coords of the first char of the selected item
{
{ if the mouse has been clicked in one of the items then
{    c returns answer selected item
{    function returns true
{ else
{    function returns false
{
{--------------------------------------------------------------------------}
function show_small_box(var c,cx,cy: integer; var menu: menu_header):
    boolean;
  var s: string[80];
      i,j,l,k,cnt: integer;
      choice: choicerecptr;
begin
  with menu do
  begin
    show_small_box:=false;
    if ScreenBuf = nil then
      get_box(x1,y1,x2-x1+1,y2-y1+1,ScreenBuf);

    if cy > 22-num_items then cy:=22-num_items;
    writeat(x1,y1,frame,'ฺ');
    for i:=x1+1 to x2-1 do writeat(i,y1,frame,'ฤ');
    writeat(x2,y1,frame,'ฟ');
    choice:=choicelist;
    k:=1;
    for i:=1 to num_items do
    with choice^ do
    begin
      writeat(x1,y1+i,frame,'ณ');
      s:=name;
      while length(s) < x2-x1-1 do s:=s+' ';
      if c = k then
      begin
        if mouseat(x1+1,y1+i,select,s) then
          show_small_box:=true;
        if AutoHelp then Help_Page[1]:=ans;
        cx:=x1+1; cy:=y1+i;
      end else
      if mouseat(x1+1,y1+i,in_box,s) then
      begin
        show_small_box:=true;
        if AutoHelp then Help_Page[1]:=ans;
        cx:=x1+1; cy:=y1+i;
        c:=k;
      end;
      writeat(x2,y1+i,frame,'ณ');
      inc(k); if k > num_items then k:=1;
      choice:=next;
      if choice = nil then choice:=choicelist;
    end;
    writeat(x1,y2,frame,'ภ');
    for i:=x1+1 to x2-1 do writeat(i,y2,frame,'ฤ');
    writeat(x2,y2,frame,'ู');
  end;
end;

{--------------------------------------------------------------------------
{
{ function func_show_menu
{
{ Parameters var c: integer;  selected item
{            var cx: integer; coord of selected item
{            var cy: integer; coord of selected item
{            var mmenu: menu_header;
{
{ Return    boolean;
{
{ Globals   Help_Page
{
{ show a menu as a big-box, small-box or bar
{ the answer = c item will be shown selected
{ cx,cy returns the coords of the first char of the selected item
{
{ if the mouse has been clicked in one of the items then
{    c returns answer selected item
{    function returns true
{ else
{    function returns false
{
{--------------------------------------------------------------------------}
function func_show_menu(var cx,cy,c: integer; var menu: menu_header):
    boolean;
begin
  hide_mouse; {}
  func_show_menu:=false;
  change_all_gauges(menu);
  with menu do
  case menu_kind of
    bar_menu:    func_show_menu:=show_bar(c,cx,cy,menu);
    big_menu:    func_show_menu:=show_big_box(c,cx,cy,menu);
    normal_menu: func_show_menu:=show_small_box(c,cx,cy,menu);
  end;
end;

{--------------------------------------------------------------------------
{
{ procedure show_menu
{
{ Parameters var cx: integer; coord of selected item
{            var cy: integer; coord of selected item
{            c: integer;      selected item
{            var mmenu: menu_header;
{
{ Globals   Help_Page
{
{ show a menu as a big-box, small-box or bar
{ the answer = c item will be shown selected
{ cx,cy returns the coords of the first char of the selected item
{
{--------------------------------------------------------------------------}
procedure show_menu(var cx,cy: integer; c: integer; var menu: menu_header);
begin
  if func_show_menu(cx,cy,c,menu) then ;
end;

{--------------------------------------------------------------------------
{
{ procedure exec_menu
{
{ Parameters var cx: integer; coord of selected item
{            var cy: integer; coord of selected item
{            answer: integer; selected item
{
{ Globals    mouse_button
{            mouse_x
{            mouse_y
{
{ shows a menu in the appropriate format
{ if return pressed or mouse clicked on an item or Alt-item-initial pressed
{   then
{     answer = ans of selected item
{     cx,cy returns coord of first char if selected item
{ if escape pressed or mouse clicked off the menu then
{     answer = 0
{ else
{     move selection to a different item or adjust gauge item
{
{ can be called any number of times for the same menu; knows not to save
{  the screen again because menu.ScreenBuf <> nil
{
{--------------------------------------------------------------------------}
procedure exec_menu(var cx,cy: integer; var answer: integer;
    var mmenu: menu_header);

  var choice,old_choice: choicerecptr;

  function find_letter(cmd: char; var c: integer; advance: boolean): boolean;
    var found: boolean;
        old_c,i: integer;
        ch: char;
  begin
    if cmd > #127 then
      for ch:='0' to 'Z' do
        if cmd = alt_chars[ch] then cmd:=ch;

    with mmenu do
    begin
      old_choice:=choice;
      old_c:=c;
      i:=1;
      found:=(upcase(cmd) = upcase(choice^.name[1])) and not advance;
      while (i <= num_items) and not found do
      begin
        choice:=choice^.next;
        inc(c);
        if choice = nil then
        begin
          choice:=choicelist;
          c:=1;
        end;
        found:=upcase(cmd) = upcase(choice^.name[1]);
        inc(i);
      end;
      if not found then
      begin
        choice:=old_choice;
        c:=old_c;
      end;
      find_letter:=found;
    end;
  end;

  var c,i: integer;
      l: longint;
      r: real;
      buf: array [1..80] of word;
      cmd,ch: char;
      g_v,g_i_max,g_i_min: integer;
      g_a,g_b: real;
      mx,my: word;

  function show: boolean;
    var i: integer;
  begin
    with mmenu do
    begin
      show:=func_show_menu(cx,cy,c,mmenu);

      choice:=choicelist;
      for i:=2 to c do choice:=choice^.next;
      answer:=choice^.ans;
    end;
  end;

begin
  with mmenu do
  begin
    if num_items = 0 then begin answer:=0; exit; end;
    page_start:=1;
    c:=1;
    choice:=choicelist;
    for i:=1 to num_items do
    with choice^ do
    begin
      if ans=answer then c:=i;
      choice:=next;
    end;

    if mouse_exists then
    begin
      mouse_button:=mouse_mode(mouse_x,mouse_y,mx,my);
      if show then ;
    end;

    repeat
      c:=(c+num_items-1) mod num_items +1;

      if show then ;

      repeat
        if mouse_exists then
        begin
          show_mouse_moved;
          mouse_button:=mouse_mode(mouse_x,mouse_y,mx,my);
        end;
      until keyavail or (mouse_button = clicked);

      if keyavail then
        cmd:=getbigkey else
      if show then
        cmd:=CR else
        cmd:=Esc;

      case cmd of
        CR:  with choice^ do
             if int_loc <> nil then
             begin
{               repeat {}
                 l:=int_loc^;
                 if integer_box(cx,cy+1,12,l,'') then;
{               until (l >= gauge_min) and (l <= gauge_max); {}
               int_loc^:=l;
               change_gauge_item(answer,mmenu);
             end else
             if real_loc <> nil then
             begin
{               repeat {}
                 r:=real_loc^;
                 if real_box(cx,cy+1,13,r,'') then;
{               until (r >= gauge_min) and (r <= gauge_max); {}
               real_loc^:=r;
               change_gauge_item(answer,mmenu);
             end else
             begin if show then ; exit; end;
        Esc: begin
               c:=0;
               if menu_kind = bar_menu then if show then;
               answer:=0;
               exit;
             end;
        Up: if menu_kind <> bar_menu then c:=(c+num_items-2) mod num_items +1;
        Left:  if menu_kind = bar_menu then
                 c:=(c+num_items-2) mod num_items +1 else
               with choice^ do
               if int_loc <> nil then
               begin
                 if int_loc^ > gauge_min then
                 begin
                   dec(int_loc^);
                   change_gauge_item(answer,mmenu);
                 end;
               end else
               if real_loc <> nil then
               begin
                 calc_real_gauge(choice,g_v,g_i_max,g_i_min,g_a,g_b);
                 if g_v > g_i_min then
                 begin
                   dec(g_v);
                   real_loc^:=g_a+(g_v+0.5)*g_b;
                   if abs(real_loc^/(gauge_max-gauge_min)) < 1E-4 then
                     real_loc^:=0;
                   change_gauge_item(answer,mmenu);
                 end;
               end else
               if menu_kind = big_menu then
               begin
                 if c > big_height then c:=c-big_height;
               end;
        Right: if menu_kind = bar_menu then
                 c:=(c mod num_items +1) else
               with choice^ do
               if int_loc <> nil then
               begin
                 if int_loc^ < gauge_max then
                 begin
                   inc(int_loc^);
                   change_gauge_item(answer,mmenu);
                 end;
               end else
               if real_loc <> nil then
               begin
                 calc_real_gauge(choice,g_v,g_i_max,g_i_min,g_a,g_b);
                 if g_v < g_i_max then
                 begin
                   inc(g_v);
                   real_loc^:=g_a+(g_v+0.5)*g_b;
                   if abs(real_loc^/(gauge_max-gauge_min)) < 1E-4 then
                     real_loc^:=0;
                   change_gauge_item(answer,mmenu);
                 end;
               end else
               if menu_kind = big_menu then
               begin
                 if c <= num_items-big_height then c:=c+big_height;
               end;
        Down: if menu_kind = bar_menu then exit else c:=c mod num_items +1;
        Page_up: if (menu_kind = big_menu) and (c > page_size(mmenu)) then
                   c:=c-page_size(mmenu);
        Page_dn: if (menu_kind = big_menu) and (c <= num_items-page_size(mmenu)) then
                   c:=c+page_size(mmenu);

        F1: Help_screen;
        #128..#255: if find_letter(cmd,c,false) then stuffkey(CR);
        else        if find_letter(cmd,c,true) then;
      end;
    until false;
  end;
end;

{--------------------------------------------------------------------------
{
{ procedure draw_box
{
{ Parameters var cx: integer;        topleft corner of box
{            var cy: integer;        topleft corner of box
{            len: integer;           width of box
{            hgt: integer;           height of box
{            title: string;          title of box
{
{ draws a double-walled box with a title
{ (the inside of the box measures len x height not the outside)
{ cx,cy is the topleft corner of the wall
{
{--------------------------------------------------------------------------}
procedure draw_box(var cx,cy: integer; len,hgt: integer; title: string);
  var x,y,i: integer;
      s: string;
begin
  writeat(cx,cy,in_box,'ษอ');
  for i:=1 to len-1 do
    if i <= length(title) then
      writeat(cx+i+1,cy,in_box,title[i]) else
      writeat(cx+i+1,cy,in_box,'อ');
  writeat(cx+len+1,cy,in_box,'ป');
  s:='บ';
  while length(s) <= len do s:=s+' ';
  s:=s+'บ';
  for i:=1 to hgt do
    writeat(cx,cy+i,in_box,s);
  writeat(cx,cy+hgt+1,in_box,'ศ');
  for i:=cx+1 to cx+len do writeat(i,cy+hgt+1,in_box,'อ');
  writeat(cx+len+1,cy+hgt+1,in_box,'ผ');
end;

{--------------------------------------------------------------------------
{
{ function string_box_2
{
{ Parameters cx: integer;            topleft corner of box
{            cy: integer;            topleft corner of box
{            len: integer;           width of box
{            var ans: string;        answer typed by user
{            title: string;          title of box
{            one_char: boolean;      is a single char answer required
{
{ Return     boolean;
{
{ draws a box with a title
{ expects the user to type a string
{ initial value of string is ans
{
{ if one_char then
{     returns after the first character typed
{     if escape pressed or mouse clicked outside box then
{         returns original value of ans
{         returns false
{     else
{     if any char pressed or mouse clicked in box then
{         returns new (single char) value of ans
{         returns true
{ else
{     if return pressed or mouse clicked in box then
{         returns new value of ans
{         returns true
{     if escape pressed or mouse clicked outside box then
{         returns original value of ans
{         returns false
{     if any other char pressed than
{         edits the string
{
{--------------------------------------------------------------------------}
function string_box_2(cx,cy,len: integer; var ans: string; title: string;
    one_char: boolean): boolean;
  var buf: array [0..2,1..80] of word;
      x,y,i,f,p: integer;
      mouse_x,mouse_y,mx,my: word;
      c: char;
      s,t: string;
      first: boolean;
      old_win_xmin,old_win_ymin,old_win_xmax,old_win_ymax: byte;
begin
  get_window(old_win_xmin,old_win_ymin,old_win_xmax,old_win_ymax);

  if len > 78 then len:=78;
  if cx <= 0 then cx:=(80 - len) div 2 -1;
  if cy <= 0 then cy:=25 div 2;
  if cx > 78-len then cx:=78-len;
  if cy > 22-3 then cy:=22-3;

  for i:=0 to 2 do moveFromScreen(ScreenPtr^[cy+i,cx],Buf[i,1],(len+2)*2);
  writeat(cx,cy,in_box,'ษอ');
  for i:=1 to len-1 do
    if i <= length(title) then
      writeat(cx+i+1,cy,in_box,title[i]) else
      writeat(cx+i+1,cy,in_box,'อ');
  writeat(cx+len+1,cy,in_box,'ป');
  writeat(cx,cy+1,in_box,'บ');
  for i:=1 to len do writeat(cx+i,cy+1,entry,' ');
  writeat(cx+len+1,cy+1,in_box,'บ');
  writeat(cx,cy+2,in_box,'ศ');
  for i:=cx+1 to cx+len do writeat(i,cy+2,in_box,'อ');
  writeat(cx+len+1,cy+2,in_box,'ผ');

  s:=ans;
  x:=wherex; y:=wherey;
  first:=true;
  p:=length(s);
  f:=1;
  repeat
    if f < p-len+3 then f:=p-len+3;
    if f > p-3 then f:=p-3;
    if f < 1 then f:=1;
    if f = 1 then t:=s else t:='ฎ'+copy(s,f+1,255);
    if length(t) > len-1 then t:=copy(t,1,len-1)+'ฏ';
    while length(t) < len do t:=t+' ';
    writeat(cx+1,cy+1,entry,t);
    window(1,1,80,25);
    gotoxy(cx+1+p-f+1,cy+1);

    c:=#0;
    repeat
      show_mouse_moved;
      if mouse_mode(mouse_x,mouse_y,mx,my) = clicked then
        if (mouse_y = cy+1) and (mouse_x > cx) and (mouse_x <= cx+len) then
          c:=mouse_select else c:=Esc else
      if keyavail then
      begin
        hide_mouse;
        c:=getbigkey;
      end;
    until c <> #0;

    if not one_char then
    case c of
      ' '..#127: if length(s) < 255 then
          begin
          if first then
          begin
            s:=c;
            p:=1;
          end else
          begin
            s:=copy(s,1,p)+c+copy(s,p+1,255);
            inc(p);
          end;
        end;
      BS: if p > 0 then
          begin
            s:=copy(s,1,p-1)+copy(s,p+1,255);
            dec(p);
          end;
      Del: if p < length(s) then
              s:=copy(s,1,p)+copy(s,p+2,255);
      Left: if p > 0 then dec(p);
      Right: if p < length(s) then inc(p);
      Home: p:=0;
      End_ch: p:=length(s);
      F1: Help_screen;
    end;
    first:=false;
  until (c = CR) or (c = Esc) or (c = mouse_select) or one_char;

  window(old_win_xmin,old_win_ymin,old_win_xmax,old_win_ymax);
  gotoxy(x,y);
  if c = Esc then
    string_box_2:=false else
  if one_char then
  begin
    ans:=c;
    string_box_2:=true;
  end else
  begin
    ans:=s;
    string_box_2:=true;
  end;
  hide_mouse;
  for i:=0 to 2 do moveToScreen(Buf[i,1],ScreenPtr^[cy+i,cx],(len+2)*2);
end;

{--------------------------------------------------------------------------
{
{ function string_box
{
{ Parameters cx: integer;            topleft corner of box
{            cy: integer;            topleft corner of box
{            len: integer;           width of box
{            var ans: string;        answer
{            title: string;          title of box
{
{ Return     boolean;
{
{ draws a box with a title
{ expects the user to type a string
{ initial value of string is ans
{
{ if return pressed or mouse clicked in box then
{     returns new value of ans
{     returns true
{ if escape pressed or mouse clicked outside box then
{     returns original value of ans
{     returns false
{ if any other char pressed than
{     edits the string:
{         BS: deletes the char before the cursor
{         Del: deletes the char at the cursor
{         Left: moves the cursor left
{         Right: moves the cursor right
{         Home: moves the cursor to the start of the string
{         End: moves the cursor to the end of the string
{         F1:  shows the current help screen
{         ' '..#127: if this is the first character to be typed then
{                      deletes the whole string
{                    inserts the character at the cursor
{
{
{--------------------------------------------------------------------------}
function string_box(cx,cy,len: integer; var ans: string; title: string):
    boolean;
begin
  string_box:=string_box_2(cx,cy,len,ans,title,false);
end;

{--------------------------------------------------------------------------
{
{ procedure message
{
{ Parameters cx: integer;            topleft corner of box
{            cy: integer;            topleft corner of box
{            msg: string;            message in box
{            title: string;          title of box
{
{ draws a box with a title and a message
{ returns when the user to types a single char
{
{ if either cx = 0 or cy = 0 then the box is centred in the screen
{
{--------------------------------------------------------------------------}
procedure message(cx,cy: integer; msg,title: string);
  var len: integer;
begin
  len:=length(msg)+2;
  if length(title)+2 > len then len:=length(title)+2;
  if string_box_2(cx,cy,len,msg,title,true) then;
end;

{--------------------------------------------------------------------------
{
{ function yes_or_no
{
{ Parameters cx: integer;            topleft corner of box
{            cy: integer;            topleft corner of box
{            len: integer;           width of box
{            title: string;          title of box
{            msg: string;            message in box
{
{ Return     char;
{
{ draws a box with a title and a message
{ expects the user to type a single char
{ if either cx = 0 or cy = 0 then the box is centred in the screen
{
{ if escape pressed or mouse clicked outside box then
{     returns escape
{ if 'Y' pressed or mouse clicked in box then
{     returns 'Y'
{ if 'N' then
{     returns 'N'
{ if any other char pressed than
{     ignores it
{
{--------------------------------------------------------------------------}
function yes_or_no(cx,cy: integer; title,msg: string): char;
  var s: string;
      len: integer;
begin
  yes_or_no:=Esc;
  len:=length(msg)+2;
  if length(title)+2 > len then len:=length(title)+2;

  repeat
    s:=msg;
    if not string_box_2(cx,cy,len,s,title,true) then exit;
    case upcase(s[1]) of
      'Y',mouse_select: begin yes_or_no:='Y'; exit end;
      'N': begin yes_or_no:='N'; exit end;
    end;
  until false;
end;

{--------------------------------------------------------------------------
{
{ function integer_box
{
{ Parameters cx: integer;            topleft corner of box
{            cy: integer;            topleft corner of box
{            len: integer;           width of box
{            var ans: longint;       answer
{            title: string;          title of box
{
{ Return     boolean;
{
{ draws a box with a title
{ expects the user to type a longint
{ initial value of longint is ans
{
{ if return pressed or mouse clicked in box then
{     returns new value of ans
{     returns true
{ if escape pressed or mouse clicked outside box then
{     returns original value of ans
{     returns false
{ if any other char pressed than
{     edits the longint
{
{--------------------------------------------------------------------------}
function integer_box(cx,cy,len: integer; var ans: longint; title: string):
    boolean;
  var s: string;
      i: longint;
      code: integer;
begin
  repeat
    str(ans,s);
    integer_box:=string_box(cx,cy,len,s,title);
    val(s,i,code);
  until code = 0;
  if code = 0 then ans:=i;
end;

{--------------------------------------------------------------------------
{
{ function real_str
{
{ Parameters r: real;      real
{            len: integer; length of string required
{
{ Return     string;
{
{ converts a real into a string
{
{--------------------------------------------------------------------------}
function real_str(r: real; len: integer): string;
  var s,t: string;
      i: integer;
begin
  if r < 0 then
  begin
    real_str:='-'+real_str(-r,len-1);
    exit;
  end;
  str(r:1:20,s);
  if pos('.',s) < len-2 then
  begin
    real_str:=copy(s,1,len);
    if s[1] = '-' then s:=copy(s,2,255);
    if s[1] <> '0' then exit;    s:=copy(s,2,255);
    if s[1] <> '.' then exit;    s:=copy(s,2,255);
    for i:=1 to len-7 do
    begin
      if s[1] <> '0' then exit;
      s:=copy(s,2,255);
    end;
  end;

  str(r:len+1,s);
  if s[1] = ' ' then s:=copy(s,2,255);
  real_str:=s;
end;

{--------------------------------------------------------------------------
{
{ function real_box
{
{ Parameters cx: integer;            topleft corner of box
{            cy: integer;            topleft corner of box
{            len: integer;           width of box
{            var ans: real;          answer
{            title: string;
{
{ Return     boolean;
{
{ draws a box with a title
{ expects the user to type a real
{ initial value of real is ans
{
{ if return pressed or mouse clicked in box then
{     returns new value of ans
{     returns true
{ if escape pressed or mouse clicked outside box then
{     returns original value of ans
{     returns false
{ if any other char pressed than
{     edits the real
{
{
{--------------------------------------------------------------------------}
function real_box(cx,cy,len: integer; var ans: real; title: string):
  boolean;
  var s: string;
      i: real;
      code: integer;
begin
  repeat
    s:=real_str(ans,len-4);
    real_box:=string_box(cx,cy,len,s,title);
    val(s,i,code);
  until code = 0;
  if code = 0 then ans:=i;
end;

{--------------------------------------------------------------------------
{
{ procedure push_Help_Page
{
{ Parameters h: integer;
{
{ Globals   Help_Page
{
{ pushes h onto the help-page stack
{ h becomes the current help page
{
{--------------------------------------------------------------------------}
procedure push_Help_Page(h: integer);
begin
  move(Help_Page[1],Help_Page[2],sizeof(Help_Page)-2);
  Help_Page[1]:=h;
end;

{--------------------------------------------------------------------------
{
{ procedure pop_Help_Page;
{
{ Parameters
{
{ Globals   Help_Page
{
{ pops the current help page
{
{--------------------------------------------------------------------------}
procedure pop_Help_Page;
begin
  move(Help_Page[2],Help_Page[1],sizeof(Help_Page)-2);
end;

{--------------------------------------------------------------------------
{
{ procedure Help_screen;
{
{ Parameters
{
{ Globals   Help_Page
{           HelpFile
{
{ shows the current help page from the HelpFile file
{
{--------------------------------------------------------------------------}
procedure Help_screen;
  const sx : integer = 9;
        sy : integer = 3;
        len = 55;
        hgt = 18;
        maxlink = 20;
  var cmd: char;
      f: text;
      thispage,findpage,helppage,i,j,k,h,nlink: integer;
      pad,s2: string[len];
      s: string;
      buf: buf_array_ptr;
      ok,lastpage: boolean;
      link: array [1..maxlink] of record x,y,l,p: integer; end;
  label error;
  function startpage(s: string): boolean;
  begin
    startpage:=copy(s,length(s)-7,255) = 'HELPPAGE';
  end;
begin
  HelpPage:=Help_Page[1];
  make_box(sx,sy,len+1,hgt,' Help ',buf);
  str(helppage,s); writeat(sx+3,sy+hgt+1,in_box,s);
  ok:=false;
  assign(f,HelpFile);  if IOresult <> 0 then goto error;
  reset(f);            if IOresult <> 0 then goto error;
  readln(f,s);
  thispage:=0;
  findpage:=1;
  fillchar(pad,sizeof(pad),' ');
  pad[0]:=char(len);

  if not startpage(s) then goto error;
  cmd:=' ';
  while (not eof(f)) and (cmd <> Esc) do
  begin
    while not startpage(s) and not eof(f) do
      readln(f,s);
    lastpage:=copy(s,1,4) = 'LAST';

    readln(f,h);
    if h = helppage then inc(thispage);
    nlink:=0;
    readln(f,s);
    if (h = helppage) and (thispage = findpage) then
    begin
      str(helppage,s2); writeat(sx+3,sy+hgt+1,in_box,s2+'อออ');
      writeat(sx+len-27,sy,in_box,'ออออออออออออออออออออออออออออ');
      writeat(sx+len+1-length(s)-4,sy,in_box,' '+s+' ');
      writeat(sx+len-27,sy+hgt+1,in_box,'ออออออออออออออออออออออออออออ');
      if not lastpage then
        writeat(sx+len-8,sy+hgt+1,in_box,' PgDn ');
      if findpage > 1 then
        writeat(sx+len-13,sy+hgt+1,in_box,' PgUp ');
      ok:=true;
    end else
      writeat(sx+len-27,sy+hgt+1,in_box,' Reading File - please wait ');
    for i:=1 to hgt do
    begin
      if not (startpage(s) or eof(f)) then readln(f,s);

      if (h = helppage) and (thispage = findpage) then
      begin
        while pos('`',s) <> 0 do
        begin
          inc(nlink);
          with link[nlink] do
          begin
            x:=pos('`',s);
            y:=i;
            delete(s,x,1);
            k:=pos('`',s);
            val(copy(s,x,k-x),p,l);
            delete(s,x,k-x+1);
            k:=pos('`',s);
            l:=k-x;
            if l < 0 then l:=0;
            delete(s,k,1);
          end;
        end;
        s2:=s;
        if startpage(s2) then s2:='';
        s2:=s2+pad;
        writeat(sx+2,sy+i,in_box,s2);
      end;
    end;

    if (h = helppage) and (thispage = findpage) then
    begin
      k:=1;
      repeat
        for i:=1 to nlink do
        with link[i] do
        begin
          for j:=sx+x+1 to sx+x+l do
            if i = k then
              ScreenPtr^[y+3,j].st:=select else
              ScreenPtr^[y+3,j].st:=normal;
        end;
        cmd:=getbigkey;
        case cmd of
          CR:      if nlink > 0 then
                   begin
                     findpage:=1;
                     helppage:=link[k].p;
                     close(f);            if IOresult <> 0 then goto error;
                     assign(f,HelpFile);  if IOresult <> 0 then goto error;
                     reset(f);            if IOresult <> 0 then goto error;
                     readln(f,s);
                     thispage:=0;
                     cmd:=Page_up;
                   end;
          Down,Right: if nlink > 0 then k:=k mod nlink +1;
          Up,Left: if nlink > 0 then k:=(k+2*nlink-2) mod nlink +1;
          Page_dn: if lastpage then
                     cmd:=#0 else
                     inc(findpage);
          Page_up: if findpage = 1 then
                     cmd:=#0 else
                   begin
                     dec(findpage);
                     close(f);            if IOresult <> 0 then goto error;
                     assign(f,HelpFile);  if IOresult <> 0 then goto error;
                     reset(f);            if IOresult <> 0 then goto error;
                     readln(f,s);
                     thispage:=0;
                   end;
        end;
      until cmd in [Esc,Page_dn,Page_up];
    end;
  end;
  for i:=0 to 1 do
  if (cmd <> Esc) and (helppage = 0) then
  begin
    writeat(sx+39,sy,in_box,'ออออ ASCII Codes อ');
    writeat(sx+len-27,sy+hgt+1,in_box,'ออออออออออออออออออออออออออออ');
    writeat(sx+1,sy+1,in_box,pad);
    for k:=0 to 15 do
      for j:=0 to 7 do
      begin
        str(i*128+j*16+k:4,s);
        writeat(sx+j*7+1,sy+k+2,in_box,s+' '+chr(i*128+j*16+k)+' ');
      end;
    cmd:=getbigkey;
  end;
  if not ok then
  begin
    writeat(sx+len-27,sy+hgt+1,in_box,'ออออออออออออออออออออออออออออ');
    writeat(sx+2,sy+2,in_box,'No help found for this item');
    cmd:=getbigkey;
  end;
  delete_box(buf);
  close(f);            if IOresult <> 0 then ;
  exit;


  error:
    writeat(sx+2,sy+2,in_box,'Error while reading:');
    writeat(sx+4,sy+3,in_box,HelpFile);
    cmd:=getbigkey;
    delete_box(buf);

    close(f);            if IOresult <> 0 then ;
end;

{--------------------------------------------------------------------------
{
{ Initialisation
{
{ Globals   Help_Page
{           autohelp
{           mouse_button
{
{ initialises the menu package
{
{ selects screen colours depending on the type of screen
{ assumes the help file is in the current directory the directory can (then
{   be changed and the help file won't be lost)
{ initialises the mouse
{
{--------------------------------------------------------------------------}
var s: string;
    b: word;

begin
  if DisplayType = HercMono then
  begin        { Hercules screen }
    in_box:=112; { Black On White}
    entry := 10; { bright }
    select:=  2; { dim }
    frame :=112; { Black On White}
    normal:= 10; { bright }
{  10; { bright }
{   2; { dim }
{ 112; { Black On White}
{   1; { dim underlined }
  end else
  if videoMode in [0,2,6,7] then
  begin        { monochrome screen }
  end else
  begin            { color screen }
    in_box:=15+4*16; { white on red }
    entry := 3+4*16; { cyan on red }
    select:=2+1*16;  { green on blue }
    frame :=     15; { white on black }
    normal:=yellow;  { yellow on black }
  end;

  getdir(0,s);
  if s <> '\' then s:=s+'\';
  HelpFile:=s+HelpFile;
  AutoHelp:=true;
  fillchar(Help_Page,sizeof(Help_Page),#0);

  if screen_mouse(b) then hide_mouse;
  mouse_button:=idle;
end.


{--------------------------------------------------------------------------
{ enhancements
{
{ bar menu anywhere and any size
{ check that the mouse works properly with stringbox and yesorno
{ if you call menu_item (etc.) after exec_menu then it hides the menu, adds
{   the new item and re-displays it
{ make mouse work with help pages
{
{--------------------------------------------------------------------------}


 กฃคฅฆงจชฉGซฌญฎฏฐฑฒÛÜ

ฤ ณ ฟ ภ ฺ ู ย ม ด ร ล
อ บ ป ศ ษ ผ ห ส น ฬ ฮ
ต ถ ท ธ ฝ พ ฦ ว ฯ ะ ั า ำ ิ ี ึ ื ุ

function mouseat(x,y,c: integer; s: string): boolean;
      repeat
        if mouse_exists then
        begin
          show_mouse_moved;
          mouse_button:=mouse_mode(mouse_x,mouse_y,mx,my);
        end;
      until keyavail or (mouse_button = clicked);

