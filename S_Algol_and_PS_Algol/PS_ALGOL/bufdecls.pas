{$R+}    {Range checking off}
{$B-}    {Boolean short circuiting off}
{$S+}    {Stack checking on}
{$I-}    {I/O checking off}
{$N-}    {No numeric coprocessor}
{$M 65500,16384,655360} {Turbo 3 default stack and heap}

Unit BufDecls;

Interface

Uses
  Crt,
  Dos,
  Printer;

const
  PhyScrCols=80;                { No. cols/physical line }

type
  TextLine  = string[PhyScrCols]; { general string type }

var
  FileName: TextLine;          { name of file being Edited }

{ declarations concerned with the text buffers }
const
  TextBufLo     =  0;            { lower bound of text buffer array }
  TextSize      = $8000; {}      { number of chars in text buffer array }

type
  CharSeq = array [TextBufLo..65534] of char; { text buffer type }
  pCharSeq= ^CharSeq;
  TextStack = record             { record describing a text buffer }
      Buf: pCharSeq;
      Top,TextBufHi: word;
      Class: (Above,Below,Undo);
    end;

function hexToStr(i: longint; l: integer): TextLine;

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

end.

