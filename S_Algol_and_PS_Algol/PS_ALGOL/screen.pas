unit screen;

interface

uses
  dos,
  crt,
  graph;

const
  screen_width  = 80;
  screen_height = 25;

type
  screenChar= record
    ch: char;
    st: byte;
  end;
  ScreenArray = array [1..screen_height,1..screen_width] of ScreenChar;
  string20 = string[20];

var
  ScreenPtr: ^screenArray;
  DisplayType,DisplayMode: integer;

const
  CGA      = 1;
  MCGA     = 2;
  EGA      = 3;
  EGA64    = 4;
  EGAMono  = 5;
  HercMono = 7;
  ATT400   = 8;
  VGA      = 9;
  PC3270   = 10;

procedure MoveToScreen(Var Source,Dest; Length: Integer);
procedure MoveFromScreen(Var Source,Dest; Length: Integer);
procedure StringToScreen(Var Source,Dest; Length: integer; colour: byte);
procedure ScreenToString(Var Source,Dest; Length: integer);
function getkey: char;
procedure stuffkey(s: string);
function GetBigKey: char;
function KeyAvail: boolean;
procedure waitkey;
procedure writeAt(x,y,c: integer; s: string);
procedure clearBox(x1,y1,x2,y2,style: integer);
procedure cursor_size(start,fin: byte);
procedure hide_cursor;
function videoMode: integer;
procedure get_window(var xmin,ymin,xmax,ymax: byte);
function istr(i,len: longint): string20;
function rstr(r: real; len,adp: integer): string20;

const
F1         = #187;
F2         = #188;
F3         = #189;
F4         = #190;
F5         = #191;
F6         = #192;
F7         = #193;
F8         = #194;
F9         = #195;
F10        = #196;
Alt_F1     = #232;
Alt_F2     = #233;
Alt_F3     = #234;
Alt_F4     = #235;
Alt_F5     = #236;
Alt_F6     = #237;
Alt_F7     = #238;
Alt_F8     = #239;
Alt_F9     = #240;
Alt_F10    = #241;
Ctrl_F1    = #222;
Ctrl_F2    = #223;
Ctrl_F3    = #224;
Ctrl_F4    = #225;
Ctrl_F5    = #226;
Ctrl_F6    = #227;
Ctrl_F7    = #228;
Ctrl_F8    = #229;
Ctrl_F9    = #230;
Ctrl_F10   = #231;
Shift_F1   = #212;
Shift_F2   = #213;
Shift_F3   = #214;
Shift_F4   = #215;
Shift_F5   = #216;
Shift_F6   = #217;
Shift_F7   = #218;
Shift_F8   = #219;
Shift_F9   = #220;
Shift_F10  = #221;

Esc        = #027;
Home       = #199;
End_ch     = #207;
Up         = #200;
Down       = #208;
Left       = #203;
Right      = #205;
Page_up    = #201;
Page_dn    = #209;
Del        = #211;
Ins        = #210;
BS         = ^H;
Tab        = ^I;
CR         = ^M;
LF         = ^J;
Ctrl_Home  = #247;
Ctrl_End   = #245;
Ctrl_PgDn  = #132;
Ctrl_PgUp  = #246;
Ctrl_Right = #244;
Ctrl_Left  = #243;
Shift_Tab  = #143;

Alt_1           = #248;
Alt_2           = #249;
Alt_3           = #250;
Alt_4           = #251;
Alt_5           = #252;
Alt_6           = #253;
Alt_7           = #254;
Alt_8           = #255;
Alt_9           = #128;
Alt_0           = #129;

Alt_Q           = #144;
Alt_W           = #145;
Alt_E           = #146;
Alt_R           = #147;
Alt_T           = #148;
Alt_Y           = #149;
Alt_U           = #150;
Alt_I           = #151;
Alt_O           = #152;
Alt_P           = #153;

Alt_A           = #158;
Alt_S           = #159;
Alt_D           = #160;
Alt_F           = #161;
Alt_G           = #162;
Alt_H           = #163;
Alt_J           = #164;
Alt_K           = #165;
Alt_L           = #166;

Alt_Z           = #172;
Alt_X           = #173;
Alt_C           = #174;
Alt_V           = #175;
Alt_B           = #176;
Alt_N           = #177;
Alt_M           = #178;


implementation

var
  retracemode: boolean;

{-------------------------------------------------------------------}
{  Move memory, as Turbo Move, but assume that the target is in
{  video memory; prevent screen flicker based on this assumption,
{  unless RetraceMode is false.  Timing is VERY tight: if the code
{  were 1 clock cycle slower, it would cause flicker.
{-------------------------------------------------------------------}
procedure MoveToScreen(Var Source,Dest; Length: Integer);
Begin
  if length = 0 then exit;
  If Not RetraceMode Then Move(Source,Dest,Length) else
  if length<>0 then
    Inline(
       $1E               {   PUSH DS            ; Save Turbo's DS                              }
      /$55               {   PUSH BP            ;   and BP                                     }
      /$BA /$03DA        {   MOV DX,03DA        ; Point DX to CGA status port                  }
      /$C5 /$B6 />source {   LDS SI,[BP+source] ; Source pointer into DS:SI                    }
      /$C4 /$BE />dest   {   LES DI,[BP+dest]   ; Dest pointer into ES:DI                      }
      /$8B /$8E />length {   MOV CX,[BP+length] ; Length value into CX                         }
      /$FC               {   CLD                ; Set string direction to forward              }
      /$AC               {  L0:LODSB            ; Grab a video byte                            }
      /$89 /$C5          {   MOV BP,AX          ; Save it in BP                                }
      /$B4 /$09          {   MOV AH,09          ; Move horiz. + vertical retrace mask to fast  }
                         {                      ;   storage                                    }
      /$EC               {  L1:IN AL,DX         ; Get 6845 status                              }
      /$D0 /$D8          {   RCR AL,1           ; Check horizontal retrace                     }
      /$72 /<-5          {   JB L1              ; Loop if in horizontal retrace: this prevents }
                         {                      ;   starting in mid-retrace, since there is    }
                         {                      ;   exactly enough time for 1 and only 1 STOSW }
                         {                      ;   during horizontal retrace                  }
      /$FA               {   CLI                ; No ints during critical section              }
      /$EC               {  L2:IN AL,DX         ; Get 6845 status                              }
      /$20 /$E0          {   AND AL,AH          ; Check for both kinds of retrace: IF the      }
                         {                      ;   video board does not report horizontal     }
                         {                      ;   retrace while in vertical retrace, this    }
                         {                      ;   will allow several characters to be        }
                         {                      ;   stuffed in during vertical retrace         }
      /$74 /<-5          {   JZ L2              ; Loop if not equal zero                       }
      /$89 /$E8          {   MOV AX,BP          ; Get the video word                           }
      /$AA               {   STOSB              ; Store the video byte                         }
      /$FB               {   STI                ; Allow interrupts                             }
      /$E2 /<-22         {   LOOP L0            ; Go do next word                              }
      /$5D               {   POP BP             ; Restore Turbo's BP                           }
      /$1F               {   POP DS             ;   and DS                                     }
    );
End;

{-------------------------------------------------------------------}
{  Move memory, as Turbo Move, but assume that the source is in     }
{  video memory; prevent screen flicker based on this assumption,   }
{  unless RetraceMode is false.  Timing is VERY tight: if the code  }
{  were 1 clock cycle slower, it would cause flicker.               }
{-------------------------------------------------------------------}
procedure MoveFromScreen(Var Source,Dest; Length: Integer);
Begin
  if length = 0 then exit;
  If Not RetraceMode Then Move(Source,Dest,Length) else
  if length<>0 then
    Inline(
       $1E               {  PUSH DS            ; Save Turbo's DS                              }
      /$55               {  PUSH BP            ;   and BP                                     }
      /$BA /$03DA        {  MOV DX,03DA        ; Point DX to CGA status port                  }
      /$C5 /$B6 />source {  LDS SI,[BP+source] ; Source pointer into DS:SI                    }
      /$C4 /$BE />dest   {  LES DI,[BP+dest]   ; Dest pointer into ES:DI                      }
      /$8B /$8E />length {  MOV CX,[BP+length] ; Length value into CX                         }
      /$FC               {  CLD                ; Set string direction to forward              }
      /$EC               { L0:IN AL,DX         ; Get 6845 status                              }
      /$D0 /$D8          {  RCR AL,1           ; Check horizontal retrace                     }
      /$72 /$FB          {  JB L0              ; Loop if in horizontal retrace: this prevents }
                         {                     ;   starting in mid-retrace, since there is    }
                         {                     ;   exactly enough time for 1 and only 1 LODSW }
                         {                     ;   during horizontal retrace                  }
      /$FA               {  CLI                ; No ints during critical section              }
      /$EC               { L1:IN AL,DX         ; Get 6845 status                              }
      /$D0 /$D8          {  RCR AL,1           ; Check for horizontal retrace: LODSW is 1     }
                         {                     ;   clock cycle slower than STOSW; because of  }
                         {                     ;   this, the vertical retrace trick can't be  }
                         {                     ;   used because it causes flicker!  (RCR AL,1 }
                         {                     ;   is 1 cycle faster than AND AL,AH)          }
      /$73 /$FB          {  JNB L1             ; Loop if not in retrace                       }
      /$AC               {  LODSB              ; Load the video byte                            }
      /$FB               {  STI                ; Allow interrupts                             }
      /$AA               {  STOSB              ; Store the video byte                         }
      /$E2 /<-16         {  LOOP L0            ; Go do next word                              }
      /$5D               {  POP BP             ; Restore Turbo's BP                           }
      /$1F               {  POP DS             ;   and DS                                     }
    );
End;

procedure StringToScreen(Var Source,Dest; Length: integer; colour: byte);
{ Move Length bytes from Source to alternate even bytes of Dest. Set the
{ alternate odd bytes of Dest to Colour. Only write to the Dest if the
{ screen is in a horizontal and vertical retrace  }
  const CRTCstatus = $3DA; { CRTC status port }
  var s: array[0..32767] of byte absolute source;
      d: array[0..32767] of byte absolute dest;
      i: integer;
begin
  if length = 0 then exit;
(*
  for i:=0 to length-1 do
  begin
    d[i*2]:=s[i];
    d[i*2+1]:=colour;
  end;
*)
  inline(
     $1E               {  PUSH    DS                                  }
    /$BA />CRTCstatus  {  MOV     DX,CRTCstatus                       }
    /$C5 /$76 /<dest   {  LDS     SI,[BP+dest]                        }
    /$C4 /$7E /<source {  LES     DI,[BP+source]                      }
    /$8B /$4E /<length {  MOV     CX,[BP+length]                      }
    /$51               { L1: PUSH CX                                  }
    /$8A /$6E /<colour {     MOV     CH,[BP+colour]                   }
    /$26               {     ES:                                      }
    /$8A /$0D          {     MOV     CL,[DI]   CL:= source byte       }
    /$EC               {   L2: IN   AL,DX      CRTC status            }
    /$24 /$01          {       AND     AL,1    horiz retrace          }
    /$75 /<-5          {       JNZ     L2      Loop until zero        }
    /$FA               {     CLI               interrupts off         }
    /$EC               {   L3: IN   AL,DX      CRTC status            }
    /$24 /$09          {       AND     AL,9    horiz and vert retrace }
    /$74 /<-5          {       JZ      L3      Loop until non-zero    }
    /$89 /$0C          {     MOV     [SI],CX   dest word := CX        }
    /$FB               {     STI               interrupts on          }
    /$47               {     INC     DI                               }
    /$46               {     INC     SI                               }
    /$46               {     INC     SI                               }
    /$59               {     POP     CX                               }
    /$E2 /<-27         {     LOOP    L1                               }
    /$1F               {  POP     DS                                  }
  );
(**)
end;

procedure ScreenToString(Var Source,Dest; Length: integer);
{ Move Length alternate even bytes from Source to Dest.
{ Only read from the Source if the
{ screen is in a horizontal and vertical retrace  }
  const CRTCstatus = $3DA; { CRTC status port }
  var s: array[0..32767] of byte absolute source;
      d: array[0..32767] of byte absolute dest;
      i: integer;
begin
  if length = 0 then exit;
  for i:=0 to length-1 do
    d[i]:=s[i*2];
end;

{-------------------------------------------------------------------}
{ keyboard input
{-------------------------------------------------------------------}
var stuffed_keys: string[10];
    macro_keys: string;
    macro_index: integer;
    learn_macro,macro_loop: boolean;
    shifts: byte absolute 0:$417;
const learn_key = #13;
      do_key = #7;

function getkey: char;
  var ch: char;
begin
  if keypressed then macro_loop:=false;
  if macro_loop and (macro_index > ord(macro_keys[0])) then macro_index:=1;

  if stuffed_keys <> '' then
  begin
    getkey:=stuffed_keys[1];
    stuffed_keys:=copy(stuffed_keys,2,255);
  end else
  if learn_macro then
  begin
    ch:=readkey;
    getkey:=ch;
    if (ch = learn_key) and ((shifts and 4) <> 0) then
    begin
      sound(250); delay(20); nosound;
      learn_macro:=false;
      getkey:=getkey;
    end else
    if (ch = do_key) and ((shifts and 4) <> 0) then
    begin
      sound(50); delay(100); nosound;
      getkey:=getkey;
    end else
    begin
      macro_keys:=macro_keys+ch;
      sound(1000); delay(2); nosound;
    end;
  end else
  if macro_index <= ord(macro_keys[0]) then
  begin
    getkey:=macro_keys[macro_index];
    inc(macro_index);
  end else
  begin
    ch:=readkey;
    if (ch = learn_key) and ((shifts and 4) <> 0) then
    begin
      sound(500); delay(20); nosound;
      learn_macro:=true;
      macro_keys:='';
      macro_index:=1000;
      getkey:=getkey;
    end else
    if (ch = do_key) and ((shifts and 4) <> 0) then
    begin
      macro_loop:=(shifts and 3) <> 0;
      macro_index:=1;
      getkey:=getkey;
    end else
      getkey:=ch;
  end;
end;

procedure stuffkey(s: string);
  var i: integer;
begin
  if stuffed_keys = '' then
    for i:=1 to length(s) do
      if s[i] > #127 then
        stuffed_keys:=stuffed_keys+#0+chr(ord(s[i]) and 127) else
        stuffed_keys:=stuffed_keys+s[i];
end;

function GetBigKey: char;
{ reads a single character from the keyboard. Special characters are
{ returned as > #127 }
  var ch: char;
begin
  ch:=getkey;
  if ch = #0 then
    GetBigKey:=char(ord(getkey) or 128)
  else
    GetBigKey:=ch;
end;

function KeyAvail: boolean;
  var mx,my,b: word;
begin
  KeyAvail:=keypressed or (macro_index <= ord(macro_keys[0])) or
    (stuffed_keys <> '');
end;

procedure waitkey;
begin
  if getbigkey = ' ' then;
end;

function videoMode: integer;
  var regs: Registers;
begin
  with regs do
  begin
    ax:=$0F00;
    intr($10,regs);
    videomode:=al;
  end;
end;

{-------------------------------------------------------------------}
{ screen writes
{-------------------------------------------------------------------}
procedure writeAt(x,y,c: integer; s: string);
  var i,j: integer;
begin
  StringToScreen(s[1], ScreenPtr^[y,x], length(s),c);
end;

procedure clearBox(x1,y1,x2,y2,style: integer);
  var x,y: integer;
      line: array [1..screen_width] of Char;
begin
  for x:=x1 to x2 do line[x]:=' ';
  for y:=y1 to y2 do StringToScreen(line[x1], ScreenPtr^[y,x1], (x2-x1+1),
    style);
end;

procedure cursor_size(start,fin: byte);
  var regs: Registers;
begin
  with regs do
  begin
    ax:=$0100;
    ch:=start;
    cl:=fin;
    intr($10,regs);
  end;
end;

procedure hide_cursor;
begin
  cursor_size(15,1);
end;

procedure get_window(var xmin,ymin,xmax,ymax: byte);
begin
  xmin:=lo(windmin)+1;
  ymin:=hi(windmin)+1;
  xmax:=lo(windmax)+1;
  ymax:=hi(windmax)+1;
end;

function istr(i,len: longint): string20;
  var s: string20;
begin
  str(i:len,s);
  istr:=s;
end;

function rstr(r: real; len,adp: integer): string20;
  var s: string20;
begin
  str(r:len:adp,s);
  rstr:=s;
end;

{-------------------------------------------------------------------}
{ Determine screen type for screen updating procedures
{-------------------------------------------------------------------}

var b: word;

begin
  DisplayType:=detect;
  DetectGraph(DisplayType,DisplayMode);

  if DisplayType = HercMono then
  begin        { monochrome screen }
    ScreenPtr:=ptr($B000,0);
    Retracemode:=false;
  end else
  begin            { color screen }
    ScreenPtr:=ptr($B800,0);
    Retracemode:=DisplayType = CGA;
  end;
  stuffed_keys:='';
  macro_index:=1000;
  learn_macro:=false;
  macro_loop:=false;
  macro_keys:='';
end.


