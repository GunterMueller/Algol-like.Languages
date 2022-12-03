{$R+,B-,S+,I-,N-,L-,V-}

Unit EditDecl;


{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name: BUFDECLS.CMP                                         \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Declarations of Types, Constants and Variables                \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

Interface

Uses
  Crt,
  Dos,
  screen;

{=================================================================}
{ below here is for the configuration file
{=================================================================}

const
  PhyScrCols=80;                { No. cols/physical line }

type
  TextLine  = string[PhyScrCols]; { general string type }

const
  extension:  textline = '';
  ASM:        textline = 's.asm';
  Parameters: textline = '';
  HelpFile:   textline = 'Help.msg';
  lexemefile: textline = 'lexemes.def';
  listfilename:textline= 's.lis';
  listprog  :boolean = false;
  syndebug  :boolean = false;
  trace     :boolean = false;
  MACROLIB:   textline = '';
  PASS2PROG:  textline = '';
  PSDIR:      textline = '.\';
  TxtColour: integer =$07;       { White on black}
  CmdColour: integer =$01;       { blue on black}
  BlkColour: integer =$50;       { black on violet}
  AutoIndent: boolean = true;
  InsertMode: boolean = true;
  Tab_8: boolean = false;
  AutoSave: boolean = true;

{=================================================================}
{ above here is for the configuration file
{=================================================================}

var
  FileName: TextLine;          { name of file being Edited }
  ShortName: TextLine;         { shortenned version of FileName }
  Listfile : text;             { listing file for errors }
{ declarations concerned with the text buffers }
const
  TextBufLo     =  0;            { lower bound of text buffer array }
{$ifndef smallmodel}
  TextSize      = 60000;         { number of chars in text buffer array }


{$endif}
{$ifdef smallmodel}
  TextSize      = 10000;         { number of chars in text buffer array }


{$endif}
type
  BufIndex = TextBufLo..TextSize;
  CharSeq = array [BufIndex] of char; { text buffer type }
  pCharSeq= ^CharSeq;
  TextStack = record             { record describing a text buffer }
      Buf: pCharSeq;
      Top: BufIndex;
    end;

{ miscellaneous useful constants }
const
  ScreenTop=2;                  { Top line used on Screen }
  PhyScrRows=25;                { No. lines/physical Screen }
  CR=#13; LF=#10; Esc=#27; Nul=#0; CtrlZ=#26;
  LastAlpha=#126; BS=#8; Left=#203; Del=#211;

{ miscellaneous useful type declarations }
type
  LineCnt = 0..PhyScrRows;
  ColCnt  = 0..PhyScrCols;

{ miscellaneous variables }
var
  LineNo: LineCnt;      { current cursor address}
  ColNo: ColCnt;        { current cursor address}
  CopyUndeleteCmdCnt,   { undo stack copy command count }
  CopyUndeleteLineCnt:  { undo stack copy line count }
    integer;

  StopEditor,           { editor being Stopped ? }
  FailCmd,              { set by various commands when they Fail ? }
  ErrorOnScreen,        { is there an Error message showing ? }
  ScreenSaved,          { is the screen buffer saved to the text buf ? }
  WriteChangeFlag,      { changes in Text after file write ? }
  CompileChangeFlag:    { changes in Text after compile ? }
    boolean;
  InCompiler:           { doing compilation ? }
    boolean;


var
  AboveScreen,BelowScreen,UndoStack: TextStack;


{ declarations concerned with markers }
type mark_rec = record
         case class: (abv,blw,scrn) of
           abv,blw: (index: BufIndex);
           scrn: (cl: colcnt; ln: linecnt);
         end;
     mark_type = (blk_beg,this_cur,prv_cur,blk_end);
var marker: array [mark_type] of mark_rec;

{ declarations concerned with the screen }
var
  EditScreen: array [1..PhyScrRows,1..PhyScrCols] of char; { copy of Screen }

function hexToStr(i: longint; l: integer): TextLine;
procedure UpdPhyScr;

Implementation

function hexToStr(i: longint; l: integer): TextLine;
begin
  if l < -1 then
    hexToStr:=hexToStr(i shr 4,l+1)+hexToStr(i and 15,1) else
  if l = -1 then
    hexToStr:=hexToStr(i and 15,1) else
  if (i shr 4 <> 0) or (l > 1) then
    hexToStr:=hexToStr(i shr 4,l-1)+hexToStr(i and 15,1) else
  if i < 10 then hexToStr:=chr(i+48) else
     hexToStr:=chr(i-10+ord('A'));
end;

procedure UpdPhyScr;
{ copy Screen to the actual screen, Stop if a key is pressed }
  var r,l: Linecnt;
      in_blk: boolean;
  procedure UpdLine(r: Linecnt);
    var st,fin: ColCnt;
  begin
    if (marker[blk_beg].class = blw) or (marker[blk_end].class = abv) or
       ((marker[blk_beg].class = scrn) and (marker[blk_beg].ln > r)) or
       ((marker[blk_end].class = scrn) and (marker[blk_end].ln < r)) then
    begin
      StringToScreen(EditScreen[r,1],
                   ScreenPtr^[r,1],
                   PhyScrCols,TxtColour);
    end else
    begin
      if (marker[blk_beg].class = abv) or (marker[blk_beg].ln < r) then
        st:=1 else st:=marker[blk_beg].cl;
      if (marker[blk_end].class = blw) or (marker[blk_end].ln > r) then
        fin:=PhyScrCols else fin:=marker[blk_end].cl;
      StringToScreen(EditScreen[r,1],
                   ScreenPtr^[r,1],
                   st-1,TxtColour);
      if fin > st then
        StringToScreen(EditScreen[r,st],
                     ScreenPtr^[r,st],
                     fin-st,BlkColour);
      if fin > 0 then
        StringToScreen(EditScreen[r,fin],
                     ScreenPtr^[r,fin],
                     PhyScrCols-fin,TxtColour);
    end;
  end;
begin
  gotoxy(ColNo,LineNo);
  if keyavail then exit;
  UpdLine(LineNo);
  if ErrorOnScreen then l:=PhyScrRows-1 else l:=PhyScrRows;

  for r:=ScreenTop to l do
  begin
    if keyavail then exit;
    UpdLine(r);
  end;
end;

end.



