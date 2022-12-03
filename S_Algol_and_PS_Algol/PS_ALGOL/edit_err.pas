{$R+,B-,S+,I-,N-,L-,V-}

Unit Edit_Err;

{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name:                                                      \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Error Message handling                                        \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

Interface

uses
  Crt,
  Screen,
  EditDecl;

var
  ErrMsg: TextLine;
  ErrRet,Errstack,ErrBP: pointer;
  ErrorCursor: word;
{
  Lex_Cursor: word;
{}
  last_lexeme: word;

procedure MsgLine (s: TextLine; MoveCur: boolean) ;
function MessageNum(Mno: byte): TextLine;
procedure ErrorMsg(s: TextLine; Wait: boolean);
procedure ReportError (st: string);
procedure ErrorReturn;
procedure ErrorReturnHere;
  inline(
      $90 /$90 /$90
     /$89 /$E0              {  MOV  AX,SP                }
     /$A3 />Errstack        {  MOV  [ofs(Errstack)],AX   }
     /$8C /$D0              {  MOV  AX,SS                }
     /$A3 />Errstack+2      {  MOV  [seg(Errstack)],AX   }
     /$89 /$2E />ErrBP      {  MOV  [ErrBP)],BP          }
     /$8C /$C8              {  MOV  AX,CS                }
     /$A3 />ErrRet+2        {  MOV  [seg(ErrRet)],AX     }
     /$E8 /$00 /$00         {  CALL +0                   }
     /$58                   {  POP  AX                   }
     /$A3 />ErrRet          {  MOV  [ofs(ErrRet)],AX     }
  );

Implementation

procedure MsgLine (s: TextLine; MoveCur: boolean) ;
{ put a string on the message line. If MoveCur then  put the cursor at the
{ current position }
  var l: ColCnt;
      line: array [1..PhyScrCols] of char;
begin
  if s='' then ErrorOnScreen:=false else
  begin
    s:=s+'  ';
    ErrorOnScreen:=true;
    l:=ord(s[0]);
    if l > PhyScrCols-4 then l:=PhyScrCols-4;

    FillChar(line[1],PhyScrCols,' ');
    Move(ShortName[1],line[PhyScrCols-length(ShortName)-1],
      length(ShortName));
    Move(s[1],line[2],l);
    StringToScreen(line[1],
                 ScreenPtr^[PhyScrRows,1],
                 PhyScrCols,CmdColour);
    if MoveCur then gotoxy(l+2,PhyScrRows);
  end;
end;

function MessageNum(Mno: byte): TextLine;
{ converts an Error number into an Error message }
var
  St: TextLine;
begin
  case Mno of
      { DOS meessages }
      1: MessageNum:='  cannot find/create file';
      2: MessageNum:='  File not found';
      3: MessageNum:='  Path not found';
      4: MessageNum:='  Too many files open';
      5: MessageNum:='  File access denied';
      6: MessageNum:='  Invalid file handle';
      7: MessageNum:='  Memory control blocks destroyed';
      8: MessageNum:='  Insufficient memory';
      9: MessageNum:='  Invalid memory block address';
     10: MessageNum:='  Invalid environment';
     11: MessageNum:='  Invalid format';
     12: MessageNum:='  Invalid access code';
     13: MessageNum:='  Data Invalid';
     15: MessageNum:='  Invalid drive number';
     16: MessageNum:='  Cannot remove current directory';
     17: MessageNum:='  Cannot rename across drives';
     18: MessageNum:='  No more files';
     19: MessageNum:='  Disk write protected';
     20: MessageNum:='  Unknown unit ID';
     21: MessageNum:='  Drive not ready';
     22: MessageNum:='  Unknown command error';
     23: MessageNum:='  Disk data error';
     24: MessageNum:='  Bad request structure';
     25: MessageNum:='  Seek error';
     26: MessageNum:='  Unknown medium error';
     27: MessageNum:='  Disc sector not found';
     28: MessageNum:='  Printer out of paper';
     29: MessageNum:='  Write error';
     30: MessageNum:='  Read error';
     31: MessageNum:='  General failure';
     32: MessageNum:='  File sharing violation';
     33: MessageNum:='  File locking violation';
     34: MessageNum:='  Improper disk change';
     35: MessageNum:='  FCB not available error';
     40: MessageNum:='  File too big, file not read'; {my own msg, not DOS}
     80: MessageNum:='  File already exists';
     82: MessageNum:='  Cannot make directory';
     83: MessageNum:='  Fail on critical error interrupt';
    100: MessageNum:='  Disk read error';
    101: MessageNum:='  Disk write error';
    102: MessageNum:='  File not assigned';
    103: MessageNum:='  File not open';
    104: MessageNum:='  File is not open for input';
    105: MessageNum:='  File is not open for output';
    106: MessageNum:='  Invalid numeric format';

    { Turbo messages }
    200: MessageNum:='  Division by zero';
    201: MessageNum:='  Range check';
    202: MessageNum:='  Stack Overflow';
    203: MessageNum:='  Heap Overflow';
    204: MessageNum:='  Invalid pointer';
    205: MessageNum:='  Floating point overflow';
    206: MessageNum:='  Floating point underflow';
    207: MessageNum:='  Invalid floating point';

    {Our message}
    255: MessageNum:='  Unable to read/write to drive';
    else begin
           Str(Mno,St);
           MessageNum:='  Error ' + St + ' - (Message not found)';
         end;
  end;
end;

procedure ErrorMsg(s: TextLine; Wait: boolean);
{ writes an Error message on the message line, }
{ IF Wait THEN Wait for the user to press esc   }
begin
  UpdPhyScr;
  if Wait then
  begin
    MsgLine(s + ' - Press <Esc>',false);
    repeat until GetBigKey=Esc;
  end else
  begin
    MsgLine(s,false);
    gotoxy(ColNo,LineNo);
    repeat until keyavail;
  end;
  MsgLine('',false);
end;

{ ErrorReturnHere stores the current CS,IP,SS,SP and BP registers.
{ If any procedure calls ErrorReturn (declared in LEX.PAS) then
{ control returns to just after this point with SS,SP,BP restored.
{ You can use ErrorMsg to signal whether this is the
{ first return from this procedure or a subsequent error return }


procedure ErrorReturn;
  const ErrRet2: pointer = nil;
  var   off: word absolute ErrRet2;
begin
  ErrRet2:=ErrRet;
  off:=off+4;
  inline(
$90/$90/$90/
      $FA                   {  CLI disable interrupts    }
     /$8E /$16 />Errstack+2 {  MOV SS,[seg(Errstack)]    }
     /$8B /$26 />Errstack   {  MOV SP,[ofs(Errstack)]    }
     /$8B /$2E />ErrBP      {  MOV BP,[ErrBP)]           }
     /$FB                   {  STI ensable interrupts    }
     /$FF /$2E />ErrRet2    {  JMP FAR [ErrRet2]         }
  );
end;

procedure ReportError (st: string);
begin
  ErrMsg:=st;
  ErrorCursor:=last_lexeme;

sound(1000); delay(30); nosound;
with AboveScreen do
write('<',errorcursor,'/',buf^[ErrorCursor],'>'); if readkey = ' ' then ;
{}
  ErrorReturn;
end;

end.


