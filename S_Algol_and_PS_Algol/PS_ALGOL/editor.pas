{$R+,B-,S+,I-,N-,L-,V-}

Unit editor;

Interface

uses
  Crt,
  Dos,
  Screen,
  EditDecl,
  Edit_Err,
  ExecT,
  Menu,
  Dir,
  Control,
  Compile;

procedure Edit;

Implementation

{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name:                                                      \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Text Buffer Data Structure Utilities                          \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

var heap_min:   ^byte; { min size of heap during compilation }

procedure CheckData(c: char);
{ are all the data structures intact ? }
  var m: mark_type;
begin
  if (ColNo < 1) or (ColNo > PhyScrCols) then
  begin
    ErrorMsg(c+' Data Corrupted: ColNo',true);
    ColNo:=1;
  end;

  if (LineNo < ScreenTop) or (LineNo > PhyScrRows) then
  begin
    ErrorMsg(c+' Data Corrupted: LineNo',true);
    LineNo:=1;
  end;

  with AboveScreen do
  begin
    if (top < TextBufLo) or (top > TextSize) or (top >= BelowScreen.top) then
    begin
      ErrorMsg(c+' Data Corrupted: AboveScreen.top',true);
      top:=TextBufLo;
    end;

    if Buf^[TextBufLo] <> CtrlZ then
    begin
      ErrorMsg(c+' Data Corrupted: AboveScreen.Buf^[TextBufLo]',true);
      Buf^[TextBufLo]:=CtrlZ;
    end;

    if (top > TextBufLo) and
       ((Buf^[top] <> LF) or (Buf^[top-1] <> CR)) then
    begin
      ErrorMsg(c+' Data Corrupted: AboveScreen.Buf^[top]',true);
      inc(top);         Buf^[top]:=CR;
      inc(top);         Buf^[top]:=LF;
    end;
  end;

  with BelowScreen do
  begin
    if (top > TextSize) or (top < TextBufLo) or (top <= AboveScreen.top) then
    begin
      ErrorMsg(c+' Data Corrupted: BelowScreen.top',true);
      top:=TextSize;
    end;

    if Buf^[TextSize] <> CtrlZ then
    begin
      ErrorMsg(c+' Data Corrupted: BelowScreen.Buf^[TextSize]',true);
      Buf^[TextSize]:=CtrlZ;
    end;

    if (top < TextSize) and
       ((Buf^[TextSize-1] <> LF) or (Buf^[TextSize-2] <> CR)) then
    begin
      ErrorMsg(c+' Data Corrupted: Below.Buf^[TextSize-1]',true);
      Buf^[TextSize-1]:=LF;
      Buf^[TextSize-2]:=CR;
    end;
  end;

  for m:=blk_beg to blk_end do
  with marker[m] do
  case class of
    scrn: begin
            if (cl < 1) or (cl > PhyScrCols) then
            begin
              ErrorMsg(c+' Data Corrupted: marker['+chr(ord(m)+48)+'].cl',true);
              cl:=1;
            end;
            if (ln < ScreenTop) or (ln > PhyScrRows) then
            begin
              ErrorMsg(c+' Data Corrupted: marker['+chr(ord(m)+48)+'].ln',true);
              ln:=1;
            end;
          end;
    abv:  with AboveScreen do
          begin
            if (index < TextBufLo+1) or (index > top) then
            begin
              ErrorMsg(c+' Data Corrupted: marker['+chr(ord(m)+48)+'].abv.index',true);
              index:=TextBufLo+1;
            end;
            if (Buf^[index] = LF) then
            begin
              ErrorMsg(c+' Data Corrupted: Buf^[marker['+chr(ord(m)+48)+'].abv.index]',true);
              dec(index);
            end;
          end;
    blw:  with BelowScreen do
          begin
            if (index < top) or (index > TextSize-1) then
            begin
              ErrorMsg(c+' Data Corrupted: marker['+chr(ord(m)+48)+'].blw.index',true);
              index:=top;
            end;
            if (Buf^[index] = LF) then
            begin
              ErrorMsg(c+' Data Corrupted: Buf^[marker['+chr(ord(m)+48)+'].blw.index]',true);
              dec(index);
            end;
          end;
  end;
end;

procedure Change;
{ the current file has been changed }
begin
  CompileChangeFlag:=true;
  WriteChangeFlag:=true;
end;

procedure Move_to_Screen(var source; line: LineCnt; col: ColCnt; len: ColCnt);
begin
  if len > PhyScrCols-1 then len:=PhyScrCols-1;
    Move(source,EditScreen[Line,Col],len);
end;

function LastNonBlankCol(line: LineCnt): ColCnt;
{ finds the last non-blank column of a line on the screen }
  var p: ^char;
      n: ColCnt;
begin
(*
  for n:=physcrcols downto 1 do
    if EditScreen[line,n] <> ' ' then
    begin
      lastnonblankcol:=n;
      exit;
    end;
    LastNonBlankCol:=0;
(**)
    p:=ptr(seg(EditScreen[line,PhyScrCols]),ofs(EditScreen[line,PhyScrCols]));
    inline(
                        { CX:=PhyScrCols;                  }
       $B9 />PhyScrCols {     MOV  CX,PhyScrCols           }
                        { ES:DI := p                       }
      /$C4 /$7E /<p     {     LES  DI,[BP-p]               }
                        { if [ES:DI] = ' ' then            }
      /$26              {  L1:ES:                          }
      /$8A /$05         {     MOV  AL,[DI]                 }
      /$3C /$20         {     CMP  AL,' '                  }
      /$75 /4           {     JNZ  L2                      }
                        { DI:=DI-1                         }
      /$4F              {     DEC  DI                      }
                        { CX:=CX-1;                        }
      /$49              {     DEC  CX                      }
                        { loop if CX > 0                   }
      /$75 /<-11        {     JNZ  L1                      }
                        { n:=CX                            }
      /$89 /$4E /<n     {  L2:MOV  [BP-n],CX               }
    );
    LastNonBlankCol:=n;
(**)
end;

function EndLine(LineNo: LineCnt): ColCnt;
{ returns the length of a line on the Screen }
  var col: ColCnt;
begin
  col:=LastNonBlankCol(LineNo);
  if col < PhyScrCols then
      EndLine:=succ(col) else EndLine:=PhyScrCols;
end;

procedure PushAboveScreen(line: LineCnt);
{ push a line from the Screen onto the AboveScreen Stack }
  var n: ColCnt;
      m: mark_type;
begin
  if EditScreen[Line,1] = #30 then exit;
  with AboveScreen do
  begin
    for m:=blk_beg to blk_end do
    with marker[m] do
      if (class = scrn) and (line = ln) then
      begin
        if cl > EndLine(Line) then cl:=EndLine(Line);
        class:=abv;
        index:=top+cl;
      end;

    n:=LastNonBlankCol(line);
    Move(EditScreen[line,1],Buf^[top+1],n);
    top:=succ(top+n); Buf^[top]:=CR;
    inc(top);         Buf^[top]:=LF;

    if BelowScreen.top-AboveScreen.top < 2000 then
      ErrorMsg('File buffer almost full, exit now',true);
  end;
  CheckData('A');
end;

procedure PushBelowScreen(line: LineCnt);
{ push a line from the Screen onto the BelowScreen Stack }
  var len: ColCnt;
      m: mark_type;
begin
  if EditScreen[Line,1] = #30 then exit;
  with BelowScreen do
  begin
    len:=LastNonBlankCol(line);
    dec(top); Buf^[top]:=LF;
    dec(top); Buf^[top]:=CR;
    top:=top-len;
    for m:=blk_beg to blk_end do
    with marker[m] do
      if (class = scrn) and (line = ln) then
      begin
        if cl > EndLine(Line) then cl:=EndLine(Line);
        class:=blw;
        index:=top+cl-1;
      end;
    Move(EditScreen[line,1],Buf^[top],len);
    if BelowScreen.top-AboveScreen.top < 2000 then
      ErrorMsg('File buffer almost full, exit now',true);
  end;
  CheckData('B');
end;

procedure PopAboveScreen(line: LineCnt);
{ Pop a line from the AboveScreen Stack onto the Screen }
  var i: integer;
      p: ^char;
      p1: integer absolute p;
      m: mark_type;
begin
  with AboveScreen do
  begin
    i:=0;
    if top <> TextBufLo then
    begin
      top:=top-2;
      p:=ptr(seg(Buf^[top]),ofs(Buf^[top]));
      i:=p1;
      while p^ >=' ' do dec(p1);
      i:=i-p1;
      top:=top-i;
      if i >= PhyScrCols then
      begin
        i:=i-PhyScrCols+5;
        top:=top+physcrcols-5;
        Move_to_Screen(Buf^[top+1],line,1,i);
        top:=top+1; Buf^[top]:=CR;
        top:=top+1; Buf^[top]:=LF;
        ErrorMsg('Line too long - will be split',true);
        Change;
      end else
        Move_to_Screen(Buf^[top+1],line,1,i);
    end;
    if PhyScrCols > i then
      FillChar(EditScreen[line,i+1],PhyScrCols-i,' ');

    for m:=blk_beg to blk_end do
    with marker[m] do
      if (class = abv) and (index > top) then
      begin
        class:=scrn;
        cl:=index-top;
        ln:=line;
      end;
  end;
  CheckData('C');
end;

procedure PopBelowScreen(line: LineCnt);
{ Pop a line from the BelowScreen Stack onto the Screen }
  var i: integer;
      p: ^char;
      p1: integer absolute p;
      m: mark_type;
begin
  CheckData('Z');
  with BelowScreen do
  begin
    i:=0;
    if top <> TextSize then
    begin
      p:=ptr(seg(Buf^[top]),ofs(Buf^[top]));
      i:=p1;
      while p^ >=' ' do inc(p1);
      i:=p1-i;
      if i >= PhyScrCols then
      begin
        i:=PhyScrCols-5;
        ErrorMsg('Line too long: will be split',true);
        Change;
      end;
      Move_to_Screen(Buf^[top],line,1,i);
      for m:=blk_beg to blk_end do
      with marker[m] do
        if (class = blw) and (index <= top+i) then
        begin
          class:=scrn;
          cl:=index-top+1;
          ln:=line;
        end;
      if top+i < TextSize then top:=top+i else top:=TextSize;
      if (top <> TextSize) and (Buf^[top] < ' ') then inc(top);
      if (top <> TextSize) and (Buf^[top] < ' ') then inc(top);
    end else
    if line > ScreenTop then
    begin
      i:=1;
      EditScreen[line,1]:=#30;
    end;
    if PhyScrCols > i then
      FillChar(EditScreen[line,i+1],PhyScrCols-i,' ');
  end;
  CheckData('D');
end;

procedure MakeUndoSpace(len: word);
{ make space on the undo stack }
  var i: word;
begin
  with UndoStack do
  begin
    while TextSize-top < len+5 do
    begin
      i:=1;
      while Buf^[TextBufLo+i] <> LF do inc(i);
      move(Buf^[succ(TextBufLo+i)],Buf^[succ(TextBufLo)],TextSize-TextBufLo-i);
      top:=top-i;
    end;
  end;
end;

procedure PushUndoStack(line: LineCnt);
{ push a line from the Screen onto the Undo Stack }
  var n: ColCnt;
begin
  with UndoStack do
  begin
    n:=LastNonBlankCol(line);
    MakeUndoSpace(n+1);
    Move(EditScreen[line,1],Buf^[top+1],n);
    top:=succ(top+n); Buf^[top]:=#1;
  end;
end;

procedure PushUndoStackStr(s: TextLine; c: char);
{ push a string onto the Undo Stack }
  var n: ColCnt;
begin
  if s = '' then exit;
  with UndoStack do
  begin
    n:=length(s);
    MakeUndoSpace(n+1);
    Move(s[1],Buf^[top+1],n);
    top:=succ(top+n); Buf^[top]:=c;
  end;
end;

procedure PushUndoStackBlock(start,len: word);
{ push a block from the buffer onto the Undo Stack }
  var i: word;
begin
  with UndoStack do
  begin
    MakeUndoSpace(len+1);
    Move(AboveScreen.Buf^[start],Buf^[top+1],len);
    top:=succ(top+len); Buf^[top]:=#4;
  end;
end;

procedure PopUndoStack(var s: string; var start,len: word);
{ Pop a line or string from the Undo Stack onto the Screen }
  var c: char;
      j: word;
begin
  with UndoStack do
  if top <> TextBufLo then
  begin
    c:=Buf^[top];
    dec(top);
    start:=top;
    while (Buf^[start] >= #9) and (start <> TextBufLo) do dec(start);
    len:=top-start;
    top:=start;
    inc(start);
    s:=c;
    case c of
      #1,#2,#3: begin
          s[0]:=chr(len+1);
          Move(Buf^[start],s[2],len);
        end;
    end;
  end else
    s:='';
end;

procedure ResetSmall;
  { reset heap to just after text }
  var p: word;
      heap: pointer;
begin
  with AboveScreen do
  begin
    p:=seg(buf^[top])+(ofs(buf^[top]) shr 4)+1;
	 heap:=ptr(p,0);
    release(heap);
  end;
end;

procedure ResetBig;
  { reset heap to just before UndoStack }
begin
  release(heap_min);
  with UndoStack do
  begin
    top:=TextBufLo;
    Buf^[top]:=CtrlZ;
  end;
end;

procedure SaveScreen;
{ save all the lines Above the cursor to AboveScreen and
{ all the lines at and Below the cursor to BelowScreen
{ if long then saves trailing blanks on current line }
  var i: ColCnt;
begin
  i:=EndLine(LineNo);
  if i < ColNo then ColNo:=i;
  for i:=ScreenTop to pred(LineNo) do PushAboveScreen(i);
  for i:=PhyScrRows downto LineNo do PushBelowScreen(i);
  if ScreenSaved then ErrorMsg('ScreenSaved error 1',true);
  ScreenSaved:=true;
end;

procedure move_buf(source,dest,len: longint; top: word);
{ move stuff around within the text buffer
    set AboveScreen.top:=top;
    takes care of markers }
  var m: mark_type;
begin
  move(AboveScreen.Buf^[source],AboveScreen.Buf^[dest],len);

  AboveScreen.top:=top;

  for m:=blk_beg to blk_end do
  with marker[m] do
  begin
    if ((class = blw) or (class = abv)) and (index >= source) and
        (index < source+len) then
    begin
{$R-}
      index:=index+dest-source;
      if index > AboveScreen.Top then
        class:=blw else class:=abv;
{$R+}
    end;
  end;
end;

procedure SetLineNo(line: integer);
begin
  if line < ScreenTop  then LineNo:=ScreenTop else
  if line > PhyScrRows then LineNo:=PhyScrRows else
    LineNo:=line;
  while (LineNo > ScreenTop) and (EditScreen[LineNo,1] = #30) do
    dec(LineNo);
end;

procedure Consolidate(var cursor: word);
{ gather all of the Text together into AboveScreen and
{ return the current cursor position in there }
var i: word;
begin
  SaveScreen;

  cursor:=AboveScreen.top+ColNo;
  move_buf(BelowScreen.top,succ(AboveScreen.top),
    succ(TextSize-BelowScreen.top),
    AboveScreen.top+TextSize-BelowScreen.top);
  BelowScreen.top:=TextSize;
  if cursor > AboveScreen.top then
    cursor:=pred(AboveScreen.top);
  CheckData('E');
end;

procedure Show(pos: word; line: LineCnt);
{ display the Text with pos at line if possible
{ there must be no Text in the Screen Buffer when this is called }
var
  w: longint;
  b: boolean;
  col: integer;
begin
  if not ScreenSaved then SaveScreen;
  CheckData('T');
  if line = 0 then line:=PhyScrRows div 2;
  if pos <= TextBufLo then pos:=TextBufLo+1;
  if pos > TextSize then pos:=TextSize;
  if (pos > AboveScreen.top) and (pos < BelowScreen.top) then
    pos:=BelowScreen.top;

  w:=pos;
  while (line >= ScreenTop) and (w > TextBufLo+1) do
  begin
    if w <> BelowScreen.top then
    begin
      dec(w);
      if AboveScreen.Buf^[w] = CR then dec(line);
    end else
      w:=AboveScreen.top+1;
  end;
  for col:=1 to 2 do
    if BelowScreen.Buf^[w] < CtrlZ then
      if (w >= BelowScreen.top) then
      begin
        if w <= TextSize then inc(w);
      end else
        if w <= AboveScreen.top then inc(w);

  CheckData('a');
  if w >= BelowScreen.top then
  begin
{writeln('w=',w,' pos=',pos); {}
    move_buf(BelowScreen.top,succ(AboveScreen.top),
      w-BelowScreen.top,AboveScreen.top+w-BelowScreen.top);
    BelowScreen.top:=w;
    CheckData('b');
  end else
  begin
{writeln('w=',w,' Pos=',pos); {}
    BelowScreen.top:=BelowScreen.top-(AboveScreen.top-w+1);
    if (pos <= AboveScreen.top) and (pos >= w) then
      pos:=BelowScreen.top+(pos-w);
    move_buf(w,BelowScreen.top,AboveScreen.top-w+1,w-1);
    CheckData('c');
  end;

  LineNo:=ScreenTop;
  ColNo:=1;
  for line:=ScreenTop to PhyScrRows do
  begin
    if pos >= BelowScreen.top then
    begin
      SetLineNo(line);
      Col:=succ(pos-BelowScreen.top);
      CheckData('j');
    end;
    PopBelowScreen(line);
  end;
  if Col < 1 then ColNo:=1 else
  if Col > PhyScrCols then ColNo:=PhyScrCols else
    ColNo:=Col;

  ScreenSaved:=false;
  CheckData('U');
end;

{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name:                                                      \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Screen Buffer Utilities                                       \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

procedure DownLine; forward;
procedure UpLine; forward;

procedure ProcessText(c: char);
{ insert c into the Text at the current cursor position all the
{ characters to the right of the cursor are shifted along }
  var c2: char;
      oldcol: ColCnt;
      i,lines: LineCnt;
      m: mark_type;
begin
  if ScreenSaved then ErrorMsg('ScreenSaved error 2',true);
  Change;
  if not InsertMode then
  begin
    EditScreen[LineNo,ColNo]:=c;
    if ColNo < PhyScrCols then inc(ColNo);
    exit;
  end;

  oldcol:=ColNo;
  lines:=0;

  while c <> Nul do
  begin
    c2:=EditScreen[LineNo,PhyScrCols];
    if c2 = ' ' then c2:=Nul;
    if ColNo < PhyScrCols then
    begin
      Move_to_Screen(EditScreen[LineNo,ColNo],LineNo,succ(ColNo),
           PhyScrCols-ColNo);
      for m:=blk_beg to blk_end do
      with marker[m] do
        if (class = scrn) and (lineNo = ln) and (ColNo < cl)
          and (cl < PhyScrCols) then inc(cl);
    end;
    EditScreen[LineNo,ColNo]:=c;
    c:=c2;
    ColNo:=1;
    if c <> Nul then
    begin
      inc(lines);
      DownLine;
    end;
  end;

  for i:=1 to lines do UpLine;
  if oldCol < PhyScrCols then
    ColNo:=succ(oldcol) else
  begin
    DownLine;
    ColNo:=1;
  end;
  CheckData('F');
end;

procedure ClearScreen;
{ clear the Screen array }
begin
  FillChar(EditScreen[1,1],PhyScrCols*PhyScrRows,' ');
end;

function AtFirstLine: boolean;
{ top of Screen = top of file ? }
begin
  AtFirstLine:=AboveScreen.top = TextBufLo;
end;

function AtLastChar: boolean;
{ at bot of Screen and bot of file ? }
begin
  AtLastChar:=(BelowScreen.top >= TextSize-5) and
    (LineNo = PhyScrRows) and (ColNo = pred(PhyScrCols));
end;

procedure Tab;
  var p,c: ColCnt;
begin
  if Tab_8 then
    if InsertMode then
    begin
      ProcessText(' ');
      while ColNo and 7 <> 0 do ProcessText(' ');
    end else
    begin
      ColNo:=(ColNo+8) and $FFF8;
    end else
  begin
    if AtFirstLine and (LineNo = ScreenTop) then exit;

    UpLine;
    p:=1;
    for c:=succ(ColNo) to EndLine(LineNo) do
      if (p = 1) and (EditScreen[LineNo,c] = ' ') then p:=c;
    for c:=EndLine(LineNo) downto p do
      if EditScreen[LineNo,c] <> ' ' then p:=c;

    DownLine;

    if InsertMode then
    begin
      while ColNo < p do ProcessText(' ');
    end else
      ColNo:=p;
  end;
end;

procedure InsertScreen(line: LineCnt);
{ insert a line into the Screen by scrolling the bottom part of the
{ Screen down }
  var m: mark_type;
begin
  PushBelowScreen(PhyScrRows);
  if line < PhyScrRows then
    Move(EditScreen[line,1],EditScreen[succ(line),1],
         (PhyScrRows-line)*PhyScrCols);

  for m:=blk_beg to blk_end do
  with marker[m] do
    if (class = scrn) and (line-2 < ln) then inc(ln);
  CheckData('G');
end;

procedure DeleteScreen(line: LineCnt);
{ Delete a line from the Screen by scrolling the bottom part of the
{ Screen up }
  var m: mark_type;
begin
  if line < PhyScrRows then
    Move(EditScreen[succ(line),1],EditScreen[line,1],
         (PhyScrRows-line)*PhyScrCols);
  for m:=blk_beg to blk_end do
  with marker[m] do
    if (class = scrn) and (line < ln) then dec(ln);

  PopBelowScreen(PhyScrRows);
  CheckData('H');
end;

procedure Scrollup;
{ scroll the screen up by one line }
begin
  PushAboveScreen(ScreenTop);
  DeleteScreen(ScreenTop);
end;

procedure Scrolldown;
{ scroll the screen down by one line }
begin
  InsertScreen(ScreenTop);
  PopAboveScreen(ScreenTop);
end;

function YorN(s: TextLine): char;
{ prints a message and waits for Y or N to be pressed }
  var c: char;
begin
  repeat
    UpdPhyScr;
    MsgLine(s,true);
    c:=UpCase(GetBigKey);
  until (c='Y') or (c='N') or (c=#27);
  YorN:=c;
end;

{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name:                                                      \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Text File I/O                                                 \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

function pos_dot(s: TextLine): integer;
{ position of '.' in s, ignoring '..' }
  var j,k: integer;
begin
  repeat
    k:=pos('..',s);
    if k > 0 then begin s[k]:='%'; s[k+1]:='%'; end;
  until k=0;
  pos_dot:=pos('.',s);
end;

procedure ShortFileName;
  var dir,name: string;
begin
  getdir(0,dir);
  if pos(dir,FileName) = 1 then
    ShortName:=copy(FileName,length(dir)+2,255) else
    ShortName:=FileName;
end;

procedure SetFileName(name: string);
  var
    disk,st,thisdir,olddir: string;
    i,j: integer;
begin
  if name = '' then begin FileName:=''; exit; end;
  if pos_dot(Name)=0 then Name:=Name+'.'+extension;
  FileName:=name;
  getdir(0,olddir);
  if (length(name) >=2) and (name[2] = ':') then
    begin disk:=copy(name,1,2); name:=copy(name,3,255); end else
    disk:='';
  i:=0;
  for j:=1 to length(name) do
    if name[j] = '\' then i:=j;
  st:=copy(name,i+1,255);
  if i = 0 then name:='' else
  if (name[1]='\') and (i=1) then name:='\' else
    name[0]:=char(i-1);
  chdir(disk+name);
  if IOresult <> 0 then
  begin
    chdir(olddir);
    ShortFileName;
    exit;
  end;
  getdir(0,thisdir);
  if thisdir[length(thisdir)] <> '\' then thisdir:=thisdir+'\';

  chdir(olddir);
  FileName:=thisdir+st;
  ShortFileName;
end;

procedure ReadFile(Fname: TextLine);
{ insert the Text of the file Fname into the current Text
{ There must be no Text in the Screen Buffer
{ Fails  if the file does not exist }
  const BlockSize=1024;
  var InputFile: file;
      IOres,oldcol,oldline: integer;
      Start: word;
      ch: char;
      File_Size: longint;
      Nrecsread: integer;
begin
  FailCmd:=true;
  with AboveScreen do
  begin
    MsgLine('reading:'+Fname,false);
    MsgLine('',false);
    if Fname='' then exit;
    if pos_dot(FName)=0 then FName:=FName+'.'+extension;
    start:=succ(top);
    Assign(InputFile,Fname);
    Reset(InputFile,1); IOres:=IOresult;
    if IOres=0 then
    begin
      File_Size:=FileSize(InputFile);
      Close(InputFile);
      if (BelowScreen.top-File_Size)-top < 2000 then IOres:=40;
      if IOres=0 then
      begin
        Assign(InputFile,Fname);
        Reset(InputFile,1);
        repeat
          Blockread(InputFile,Buf^[succ(top)],BlockSize,Nrecsread);
          IOres:=IOresult;
          top:=top+Nrecsread;
        until (Nrecsread < BlockSize) or (IOres <> 0);
        Close(InputFile); IOres:=IOresult;
      end;
    end;
    if IOres <> 0 then
    begin
      oldline:=lineno; oldcol:=colno;
      Show(BelowScreen.top,0);
      SetLineNo(oldline); colno:=oldcol;
      ErrorMsg(MessageNum(IOres),false);
      exit;
    end;
    if (top <> TextBufLo) and (Buf^[top]=CtrlZ) then dec(top);
    if Buf^[top] <> LF then
    begin
      inc(top); Buf^[top]:=CR;
      inc(top); Buf^[top]:=LF;
    end;
    if start > top then Show(BelowScreen.top,0) else
    if start = TextBufLo then Show(succ(TextBufLo),0) else
      Show(start,0);
  end;
  Change;
  FailCmd:=false;
  if FileName = '' then SetFileName(fname);
end;

function WriteFile(Fname: TextLine): boolean;
{ write the file to disk as Fname }
  const BlockSize=1024;
  var OutputFile: file;
      IOres: integer;
      j,k: integer;
      l: LineCnt;
      w: word;
      Nrecswritten: integer;
      BakName: TextLine;
begin
  WriteFile:=false;
  if (Fname='') or (pos_dot(Fname) = 1) then exit;

  if Fname <> 'PRN' then
  begin
    j:=pred(pos_dot(Fname));
    if j <=0 then BakName:=Fname+'.BAK' else BakName:=copy(Fname,1,j)+'.BAK';
    if BakName='.BAK' then exit;

    Assign(OutputFile,BakName);
    Reset(OutputFile,1); IOres:=IOresult;
    if IOres=0 then
    begin
      Close(OutputFile);
      if DeleteFile(BakName) <> 0 then
      begin
        ErrorMsg('Error while deleting backup file',true);
        exit;
      end;
    end;

    if pos_dot(FName)=0 then FName:=FName+'.'+extension;

    Assign(OutputFile,Fname);
    Reset(OutputFile,1); IOres:=IOresult;
    if IOres=0 then
    begin
      Close(OutputFile);
      Assign(OutputFile,Fname);
      Rename(OutputFile,BakName); IOres:=IOresult;
      if IOres <> 0 then
      begin
        ErrorMsg(MessageNum(IOres),true);
        exit;
      end;
    end;
  end;
  MsgLine('Writing file',false);

  SaveScreen;
  Assign(OutputFile,Fname);
  Rewrite(OutputFile,1); IOres:=IOresult;
  if IOres=0 then
    with AboveScreen do
    begin
      w:=succ(TextBufLo);
      while (w+longint(BlockSize) <= top) and (IOres=0) do
      begin
        Blockwrite(OutputFile,Buf^[w],BlockSize,Nrecswritten);
        IOres:=IOresult;
        w:=w+Nrecswritten;
      end;
      if IOres=0 then
      begin
        Blockwrite(OutputFile,Buf^[w],succ(top-w),Nrecswritten);
        IOres:=IOresult;
      end;
    end;
  if IOres=0 then
    with BelowScreen do
    begin
      w:=top;
      while (w+longint(BlockSize) < TextSize) and (IOres=0) do
      begin
        Blockwrite(OutputFile,Buf^[w],BlockSize,Nrecswritten);
        IOres:=IOresult;
        w:=w+Nrecswritten;
      end;
      if IOres=0 then
      begin
        Blockwrite(OutputFile,Buf^[w],TextSize-w,Nrecswritten);
        IOres:=IOresult;
      end;
    end;
  if IOres=0 then begin Close(OutputFile); IOres:=IOresult; end;
  Show(AboveScreen.top,LineNo);
  if IOres <> 0 then
  begin
    ErrorMsg(MessageNum(IOres),true);
    exit;
  end;
  WriteChangeFlag:=false;
  WriteFile:=true;
end;

function WriteBlock(Fname: Textline): boolean;
{ write the block to disk as Fname }
  const BlockSize=1024;
  var OutputFile: file;
      IOres: integer;
      j,k: integer;
      l: LineCnt;
      oldColNo: ColCnt;
      i,cursor: word;
      Nrecswritten: integer;
begin
  WriteBlock:=false;
  if marker[blk_beg].index >= marker[blk_end].index then exit;
  if (Fname='') or (pos_dot(Fname) = 1) then exit;

  SaveScreen;
  oldColNo:=ColNo;

  if Fname <> 'PRN' then
  begin

    if pos_dot(Fname) = 0 then Fname:=Fname+'.'+extension;

    Assign(OutputFile,Fname);
    Reset(OutputFile,1); IOres:=IOresult;
    if IOres=0 then
    begin
      Close(OutputFile);
      if YorN('Overwrite '+Fname+' (Y/N):') <> 'Y' then exit;
    end;
  end;

  MsgLine('Writing:'+Fname,false);

  Assign(OutputFile,Fname);
  Rewrite(OutputFile,1); IOres:=IOresult;
    if (marker[blk_beg].class = marker[blk_end].class) and (IOres=0) then
    begin
      i:=marker[blk_end].index-marker[blk_beg].index;
      Blockwrite(OutputFile,AboveScreen.Buf^[marker[blk_beg].index],
        i,Nrecswritten);
      if (IOres=0) and (Nrecswritten < i) then IOres:=29;
    end else
    begin
      i:=AboveScreen.top-marker[blk_beg].index+1;
      if IOres=0 then
        Blockwrite(OutputFile,AboveScreen.Buf^[marker[blk_beg].index],
          i,Nrecswritten);
      if (IOres=0) and (Nrecswritten < i) then IOres:=29;

      i:=marker[blk_end].index-BelowScreen.top;
      if IOres=0 then
        Blockwrite(OutputFile,AboveScreen.Buf^[BelowScreen.top],
          i,Nrecswritten);
      if (IOres=0) and (Nrecswritten < i) then IOres:=29;
    end;
  if IOres=0 then begin Close(OutputFile); IOres:=IOresult; end;

  Show(pred(BelowScreen.top)+oldColNo,LineNo);
  if IOres <> 0 then
  begin
    ErrorMsg(MessageNum(IOres),true);
    exit;
  end;

  WriteBlock:=true;
end;

{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name:                                                      \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Initialisation                                                \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

procedure PrintBanner;
{ print the copyright banner }
  var r: integer;
  const banner: array [11..18] of string[54] =
          ('ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿',
           '³Compiler Toolbox                       Version 2.00A³',
           '³                 ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿                 ³',
           '³                 ³ Salgol  System ³                 ³',
           '³                 ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ                 ³',
           '³Copyright (c) Analogue Information Systems Ltd. 1986³',
           '³Copyright (c) University of Strathclyde         1989³',
           'ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ');
begin
  ClearScreen;
  for r:=11 to 18 do move(banner[r][1],EditScreen[r,13],length(banner[r]));
  UpdPhyScr;
end;

procedure init_new(name: textline);
{ initialise all variables }
var
  Regs: Registers;
  s: TextLine;
  c: char;
  m: mark_type;
begin
  StopEditor:=false;
  LineNo:=ScreenTop;
  ColNo:=1;
  MsgLine('',false);
  CopyUndeleteCmdCnt:=1;
  CopyUndeleteLineCnt:=0;
  Change;

  for m:=blk_beg to blk_end do
  with marker[m] do
  begin
    class:=scrn;
    cl:=1;
    ln:=ScreenTop;
  end;

  clrscr;
  Show_Menu_Bar;
  PrintBanner;
  ClearScreen;

  with AboveScreen do
  begin
    top:=TextBufLo;
    Buf^[top]:=CtrlZ;
  end;

  with BelowScreen do
  begin
    Buf:=AboveScreen.Buf;
    top:=TextSize;
    Buf^[top]:=CtrlZ;
  end;

  ScreenSaved:=true;

  SetFileName(name);
  if FileName = '' then
  begin
    Show(BelowScreen.top,0);
    ScreenSaved:=false;
  end else
    ReadFile(FileName);

  WriteChangeFlag:=false;
  Show_Menu_Bar;
end;

procedure initialise;
{ initialise all variables and read input file }
  var name: Textline;
begin
  if paramCount=0 then name:='' else
    name:=ParamStr(1);

  new(AboveScreen.Buf);
  new(UndoStack.Buf);

  mark(heap_min);
  ResetBig;

  init_new(name);
end;

{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name:                                                      \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Commands Routines                                             \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

function alphanum(c: char): boolean;
begin
  case c of
    'A'..'Z','a'..'z','0'..'9','_': alphanum:=true;
    else alphanum:=false;
  end;
end;

procedure DeleteTextRight;
{ Deletes the character at the current cursor position, and all Text
{ to the right of it on the same line. }
  var s: string;
      l: word;
begin
  Change;
  l:=LastNonBlankCol(LineNo)-ColNo+1;
  s[0]:=chr(l);
  move(EditScreen[LineNo,ColNo],s[1],l);
  PushUndoStackStr(s,#3);
  FillChar(EditScreen[LineNo,ColNo],l,' ');
end;

procedure UpLine;
{ moves the cursor up one line, scrolling if necessary
{ Fails if at the top of the file }
begin
  if LineNo > ScreenTop then dec(LineNo) else
  if not AtFirstLine then
  begin
    ScrollDown;
  end else
    FailCmd:=true;
end;

procedure DownLine;
{ moves the cursor down one line, scrolling if necessary
{ Fails if at the bottom of the file }
begin
  if LineNo >= PhyScrRows then
  begin
    if BelowScreen.top = TextSize then FailCmd:=true else
      ScrollUp;
  end else
    SetLineNo(LineNo+1);
end;

procedure LeftChar;
{ moves the cursor left one character. If the cursor is in column one,
{ it is positioned immediately after the first non-blank character on
{ the previous line. Fails if at the top of the file }
begin
  if ColNo > 1 then dec(ColNo) else
  if (not AtFirstLine) or (LineNo > ScreenTop) then
  begin
    UpLine;
    ColNo:=EndLine(LineNo);
  end else
    FailCmd:=true;
end;

procedure RightChar;
{ moves the cursor one character to the right
{ goes to next line if at the rhs of the Screen }
begin
  if ColNo < PhyScrCols then inc(ColNo) else
  begin
    DownLine;
    ColNo:=1;
  end;
end;

procedure RightWord;
  var l: linecnt;
      c: colcnt;
begin
  l:=LineNo; c:=ColNo;
  while alphanum(EditScreen[LineNo,ColNo]) do
  begin
    if AtLastChar then begin SetLineNo(l); ColNo:=c; exit; end;
    RightChar;
  end;
  while not alphanum(EditScreen[LineNo,ColNo]) do
  begin
    if AtLastChar then begin SetLineNo(l); ColNo:=c; exit; end;
    RightChar;
  end;
end;

procedure LeftWord;
begin
  repeat
    if AtFirstLine and (LineNo = ScreenTop) and (ColNo = 1) then exit;
    LeftChar;
  until alphanum(EditScreen[LineNo,ColNo]);
  repeat
    if AtFirstLine and (LineNo = ScreenTop) and (ColNo = 1) then exit;
    LeftChar;
  until not alphanum(EditScreen[LineNo,ColNo]);
  RightWord;
end;

procedure UpPage;
{ scrolls the Text up by PhyScrRows lines }
  var i: LineCnt;
      c1,c2: array [mark_type] of integer;
      ln,j: integer;
      m: mark_type;
begin
  for m:=blk_beg to blk_end do
    begin c1[m]:=0; c2[m]:=0; end;
  i:=PhyScrRows;
  ln:=LineNo;
  while (i >=ScreenTop) and not AtFirstLine do
    begin
      for m:=blk_beg to blk_end do
        with marker[m] do
          if class = scrn then inc(c1[m]) else
          if class = abv then inc(c2[m]);
      PushBelowScreen(i);
      PopAboveScreen(i);
      inc(ln);
      dec(i);
    end;

  for m:=blk_beg to blk_end do
  with marker[m] do
    if class = scrn then
      if c2[m] = 0 then ln:=ln+c1[m] else
        ln:=ln-i+ScreenTop-1;

  while i >=ScreenTop do
  begin
    move(EditScreen[ScreenTop,1],AboveScreen.Buf^[succ(AboveScreen.top)],PhyScrCols);
    move(EditScreen[ScreenTop+1,1],EditScreen[ScreenTop,1],(PhyScrRows-ScreenTop)*PhyScrCols);
    move(AboveScreen.Buf^[succ(AboveScreen.top)],EditScreen[PhyScrRows,1],PhyScrCols);
    dec(i);
  end;
  SetLineNo(ln-PhyScrRows+ScreenTop-1);
  CheckData('I');
end;

procedure DownPage;
{ scrolls the Text down by PhyScrRows lines }
  var l,i: LineCnt;
      j: integer;
begin
  for i:=ScreenTop to PhyScrRows do
    if BelowScreen.top < TextSize then ScrollUp else
    if LineNo < PhyScrRows then SetLineNo(LineNo+1);
{
  for i:=ScreenTop to PhyScrRows do DownLine;
  if BelowScreen.top = TextSize then FailCmd:=true else
  begin
    PushAboveScreen(i);
    PopBelowScreen(i);
  end;
}
end;

procedure DeleteLine;
{ Deletes the current line onto the Undo Stack }
  var m: mark_type;
begin
  Change;
  PushUndoStack(LineNo);
  DeleteScreen(LineNo);
  ColNo:=1;
  for m:=blk_beg to blk_end do
  with marker[m] do
    if (class = scrn) and (lineNo = ln) then cl:=1;
  CheckData('J');
end;

procedure DelRightCh;
{ Deletes the character underneath the cursor }
  var m: mark_type;
begin
  Change;
  if ColNo < PhyScrCols then
  begin
    Move_to_Screen(EditScreen[LineNo,succ(ColNo)],LineNo,ColNo,
         (PhyScrCols-ColNo));
    for m:=blk_beg to blk_end do
    with marker[m] do
      if (class = scrn) and (lineNo = ln) and (ColNo < cl) then dec(cl);
  end;
  EditScreen[LineNo,PhyScrCols]:=' ';
  CheckData('K');
end;

procedure DeleteRightChar;
{ Deletes the character underneath the cursor }
begin
  PushUndoStackStr(EditScreen[LineNo,ColNo],#3);
  DelRightCh;
end;

procedure DeleteLeftChar;
{ Deletes the character to the left of the cursor. If the cursor is in
{ column one, the current line is joined to the  previous one                                                   }
{ Fails if at the top of the file }
  var i: ColCnt;
      j: integer;
      m: mark_type;
begin
  if ColNo > 1 then
  begin
    LeftChar;
    PushUndoStackStr(EditScreen[LineNo,ColNo],#2);
    DelRightCh;
  end else
  begin
    FailCmd:=true;
    if AtFirstLine and (LineNo=ScreenTop) then exit;

    if LineNo = ScreenTop then begin ScrollDown; SetLineNo(LineNo+1); end;
    i:=EndLine(pred(LineNo));
    if EndLine(LineNo)+i-3 >= PhyScrCols then exit;

    Move_to_Screen(EditScreen[LineNo,1],pred(LineNo),i,succ(PhyScrCols-i));
    DeleteScreen(LineNo);
    for m:=blk_beg to blk_end do
    with marker[m] do
      if (class = scrn) and (lineNo = ln) then
      begin
        dec(ln);
        j:=cl-ColNo+i;
        if j < 1 then cl:=1 else
        if j > PhyScrCols then cl:=PhyScrCols else
          cl:=j;
      end;
    dec(LineNo);
    ColNo:=i;
    FailCmd:=false;
    Change;
  end;
  CheckData('L');
end;

procedure DelWord;
  var n: integer;
      s: string;
      l: word;
  procedure del_ch;
  begin
    s:=s+EditScreen[LineNo,ColNo];
    dec(n);
    DelRightCh;
  end;
begin
  n:=EndLine(LineNo)-ColNo;
  s:='';
  if alphanum(EditScreen[LineNo,ColNo]) then
    while alphanum(EditScreen[LineNo,ColNo]) do del_ch else
    while (not alphanum(EditScreen[LineNo,ColNo])) and
        (EditScreen[LineNo,ColNo] <> ' ') do del_ch;
  while (EditScreen[LineNo,ColNo] = ' ') and (n > 0) do del_ch;
  PushUndoStackStr(s,#3);
end;

procedure NewLine;
{ splits the current line at the cursor and indents the new line to
{ under the first non-blank of the old line }
  var c,p: ColCnt;
    m: mark_type;
begin
  Change;
  p:=1;
  DownLine;
  if FailCmd then
    with BelowScreen do
    begin
      dec(top); Buf^[top]:=LF;
      dec(top); Buf^[top]:=CR;
      DownLine;
    end;

  if InsertMode then
  begin
    if AutoIndent then
      for c:=pred(ColNo) downto 1 do
        if EditScreen[pred(LineNo),c] > ' ' then p:=c;

    InsertScreen(LineNo);
    FillChar(EditScreen[LineNo,1],PhyScrCols,' ');
    Move_to_Screen(EditScreen[pred(LineNo),ColNo],LineNo,p,
         succ(PhyScrCols-ColNo));
    FillChar(EditScreen[pred(LineNo),ColNo],PhyScrCols-ColNo+1,' ');

    for m:=blk_beg to blk_end do
    with marker[m] do
      if (class = scrn) and (lineNo = ln) then
        if ColNo > cl then dec(ln) else
          cl:=cl-ColNo+p;
  end;
  ColNo:=p;
  CheckData('M');
end;

procedure MoveTo(m: mark_type);
begin
  with marker[m] do
    if class = scrn then
    begin
      ColNo:=cl;
      SetLineNo(ln);
    end else
      Show(index,0);
end;

function Scan(ch1,ch2: char; var start,finish: char; var offset: word):
    boolean;
{ scans through memory from Start to Finish. If it finds ch1 or ch2 then
{ returns the offset of the character from Start }
  var len: word;
begin
  len:=succ(ofs(finish)-ofs(start));
  if seg(start) <> seg(finish) then len:=1 div (len-len);
  if ch1 = ch2 then
    inline(
       $C4 /$7E /<start   {  LES    DI,[BP+s]     }
      /$8B /$4E /<len     {  MOV    CX,[BP+len]   }
      /$8A /$46 /<ch1     {  MOV    AL,[BP+ch1]   }
      /$FC                {  CLD                  }
      /$F2                {  repNZ                }
      /$AE                {  SCASB                }
      /$89 /$4E /<len     {  MOV    [BP+len],CX   }
  )
  else
  begin
    inline(
       $C4 /$7E /<start   {  LES    DI,[BP+s]     }
      /$8B /$4E /<len     {  MOV    CX,[BP+len]   }
      /$8A /$46 /<ch1     {  MOV    AL,[BP+ch1]   }
      /$8A /$66 /<ch2     {  MOV    AH,[BP+ch2]   }
      /$FC                {  CLD                  }
                          { loop:                 }
      /$AE                {  SCASB                }
      /$74 /$09           {  JZ      exit         }
      /$26                {  ES:                  }
      /$3A /$65 /$FF      {  CMP     AH,[DI-01]   }
      /$74 /$03           {  JZ      exit         }
      /$E2 /$F5           {  LOOP    loop         }
      /$B9 />1            {  MOV     CX,0         }
                          { exit:                 }
      /$89 /$4E /<len     {  MOV    [BP+len],CX   }
    );
    dec(len);
  end;
  Scan:=(len <> -1) and
       ((mem[seg(start):ofs(finish)-len] = ord(ch1))
     or (mem[seg(start):ofs(finish)-len] = ord(ch2)));
  offset:=ofs(finish)-ofs(start)-len;
end;

function Search(str: TextLine; var start,finish: char; IgnCase: boolean;
    var offset: word): boolean;
{ searches memory from Start to Finish. If it finds textline then returns
{ the offset of the string from Start }
  var p,first,last: ^char;
      str2: TextLine;
      found,Scanf: boolean;
      i: integer;
  function UpCaseStr(s: TextLine): TextLine;
    var i: integer;
  begin
    for i:=1 to length(s) do s[i]:=UpCase(s[i]);
    UpCaseStr:=s;
  end;
begin
  found:=false;
  str2:=str;
  if IgnCase then
    for i:=1 to length(str) do
      case str[i] of
        'a'..'z','A'..'Z':
          str2[i]:=chr(ord(str[i]) xor ord('a') xor ord('A'));
      end;

  p:=ptr(seg(start),ofs(start));
  if length(str) = 1 then
  begin
    found:=Scan(str[1],str2[1],p^,finish,offset);
  end else
  repeat
    Scanf:=Scan(str[1],str2[1],p^,finish,offset);
    if Scanf then
    begin
      last :=ptr(seg(p^),ofs(p^)+offset+length(str));
      p    :=ptr(seg(p^),succ(ofs(p^)+offset));
      first:=p;
      for i:=2 to length(str) do
        if Scanf then
        begin
          Scanf:=Scan(str[i],str2[i],p^,finish,offset);
          p:=ptr(seg(p^),succ(ofs(p^)+offset));
        end;
      found:=Scanf and (p = last);
      offset:=pred(ofs(first^)-ofs(start));
      p:=ptr(seg(p^),ofs(p^)-length(str));
    end;
  until found or not Scanf;
  Search:=found;
end;

procedure Find(Searchstr: TextLine; IgnCase,NoError: boolean);
{ finds a string in the current Text, beginning at the cursor
{ Fails if the string is not found }
  var oldColNo: ColCnt;
      offset: word;
      found: boolean;
begin
  SaveScreen;
  oldColNo:=ColNo;

  with BelowScreen do
  begin
    found:=Search(Searchstr,Buf^[pred(top+oldColNo)],Buf^[TextSize],IgnCase,
      offset);
    offset:=pred(top+oldColNo) + offset;
  end;

  if found then
    Show(offset+length(Searchstr),0)
  else
  begin
    Show(pred(BelowScreen.top)+oldColNo,0);
    FailCmd:=true;
    if not NoError then
      ErrorMsg('Search string not found - '+Searchstr,false);
  end;
end;

procedure Substitute(Searchstr,SubsStr: TextLine;IgnCase,Ask,Global: boolean);
{ routine finds a string in the Text, replacing it if a match is found
{ Fails if the string is not found }
  procedure subs;
	 var i: integer;
  begin
    Change;
    for i:=1 to length(SearchStr) do DeleteLeftChar;
    for i:=1 to length(subsStr) do ProcessText(subsStr[i]);
  end;
  var c: char;
      oldColNo: ColCnt;
      offset,off: word;
      found: boolean;
begin
  if Global then
  begin
    SaveScreen;
    Show(TextBufLo,0);
    FailCmd:=false;
  end;

  repeat
    Find(Searchstr,IgnCase,Global);
    if not FailCmd then
    begin
      if ask then
      begin
        repeat
          UpdPhyScr;
          MsgLine('Substitute (Yes/No/Global):',false);
          c:=UpCase(GetBigKey);
          if c = #27 then exit;
        until (c='Y') or (c='N') or (c='G');
        if c<>'N' then subs;
        ask:=c <> 'G';
      end else
      if not KeyAvail then
      begin
        subs;
        UpdPhyScr;
      end else
      with BelowScreen do
      begin
        if GetBigKey = #27 then exit;
        subs;
        UpdPhyScr;
        SaveScreen;
        oldColNo:=ColNo;
        offset:=pred(top+oldColNo);

        repeat
          if KeyAvail and (GetBigKey = #27) then exit;
          found:=Search(Searchstr,Buf^[offset],Buf^[TextSize],IgnCase,off);
          if found then
          begin
            offset:=offset + off;
            move_buf(top,top-length(SubsStr)+length(SearchStr),offset-top,
              AboveScreen.top);
            top:=top-length(SubsStr)+length(SearchStr);
            move(SubsStr[1],Buf^[offset+length(SearchStr)-length(SubsStr)],
              length(SubsStr));
            inc(offset);
          end;
        until not found;

        Show(pred(top)+oldColNo,0);
        exit;
      end;
    end;
  until FailCmd or not Global;
end;

procedure Undelete;
{ Pops a string off of the Undo Stack and inserts it at the cursor
{ Fails if the Undo Stack is empty }
  var s: string;
      cursor,start,len,l: word;
      p: pCharSeq;
begin
  PopUndoStack(s,start,len);
  if s <> '' then
  begin
    Change;
    case s[1] of
      #1: begin
          InsertScreen(LineNo);
          FillChar(EditScreen[LineNo,1],PhyScrCols,' ');
          Move_to_Screen(s[2],LineNo,1,length(s)-1);
          ColNo:=1;
        end;
      #2,#3: begin
            for l:=2 to length(s) do
            case s[l] of
              #10: ;
              #13: NewLine;
              else ProcessText(s[l]);
            end;
            if s[1] = #3 then
              for l:=2 to length(s) do if s[l] <> #10 then LeftChar;
          end;
      #4: with AboveScreen do
        begin
          p:=UndoStack.Buf;
          consolidate(cursor);
          move_buf(cursor,cursor+len,top-cursor+1,top+len);
          move(p^[start],Buf^[cursor],len);
          CheckData('X');
          show(cursor,LineNo);
        end;
    end;
  end else
    FailCmd:=true;
  CheckData('V');
end;

procedure CopyUndelete(Line: integer);
{ copies the n'th line of the Undo Stack onto the Screen at the cursor line
{ Fails if the Undo Stack is empty }
  var oldtop: word;
begin
  with UndoStack do
  begin
    oldtop:=top;
    while (Line > 0) and (top <> TextBufLo) do
    begin
      dec(top);
      if Buf^[top] < ' 'then dec(Line);
    end;
    Undelete;
    top:=oldtop;
  end;
end;

procedure mark_block(m: mark_type);
begin
  with marker[m] do
  begin
    class:=scrn;
    cl:=EndLine(LineNo);
    if ColNo < cl then cl:=colno;
    ln:=Lineno;
  end;
end;

function PickBlock: boolean;
  var len,i,l2: word;
      s: string;
begin
  PickBlock:=false;
  len:=EndLine(LineNo);
  if ColNo > len then
    EditScreen[LineNo,ColNo-1]:=#255;
  SaveScreen;
  if marker[blk_beg].index >= marker[blk_end].index then exit;

  if marker[blk_beg].class = marker[blk_end].class then
  begin
    len:=marker[blk_end].index-marker[blk_beg].index;
      PushUndoStackBlock(marker[blk_beg].index,len);
  end else
  begin
    len:=AboveScreen.top-marker[blk_beg].index+1;
    l2:=marker[blk_end].index-BelowScreen.top;
    PushUndoStackBlock(marker[blk_beg].index,len);
    dec(UndoStack.top);
    PushUndoStackBlock(BelowScreen.top,l2);
    len:=len+l2;
  end;
  PickBlock:=true;
  CheckData('N');
end;

function DeleteBlock: boolean;
  var len: word;
      oldline: LineCnt;
      m: mark_type;
begin
  DeleteBlock:=false;
  change;
  oldline:=LineNo;

  with marker[prv_cur] do
    begin class:=scrn; ln:=LineNo; cl:=ColNo end;
  if marker[blk_beg].class = scrn then SetLineNo(marker[blk_beg].ln);
  if not PickBlock then
    begin show(marker[prv_cur].index,oldline); exit; end;

  if marker[blk_beg].class <> marker[blk_end].class then
  begin
    AboveScreen.top:=marker[blk_beg].index-1;
    BelowScreen.top:=marker[blk_end].index;
    while (AboveScreen.Buf^[AboveScreen.top] <> LF) and
      (AboveScreen.top <> TextBufLo) do
    begin
      dec(BelowScreen.top);
      BelowScreen.Buf^[BelowScreen.top]:=AboveScreen.Buf^[AboveScreen.top];
      dec(AboveScreen.top);
    end;
    marker[blk_beg]:=marker[blk_end];
    marker[prv_cur]:=marker[blk_end];
    oldline:=LineNo;
  end else
  if marker[blk_beg].class = abv then
  with AboveScreen do
  begin
    len:=marker[blk_end].index-marker[blk_beg].index;
    move_buf(marker[blk_end].index,
             marker[blk_beg].index,
             top-marker[blk_end].index+1,
             top-len);
    marker[blk_end]:=marker[blk_beg];
  end else
  with BelowScreen do
  begin
    len:=marker[blk_end].index-marker[blk_beg].index;
    move_buf(top,top+len,marker[blk_beg].index-top,AboveScreen.top);
    top:=top+len;
    marker[blk_beg]:=marker[blk_end];

    if (marker[prv_cur].index > AboveScreen.top) and
       (marker[prv_cur].index < BelowScreen.top) then
    begin
      marker[prv_cur]:=marker[blk_end];
      oldline:=LineNo;
    end;
  end;

  for m:=this_cur to prv_cur do
  with marker[m] do
   if class = blw then
     with BelowScreen do
       if (index < top) or (index > TextSize-1) then
         index:=top;

  CheckData('W');
  show(marker[prv_cur].index,oldline);
  DeleteBlock:=true;
  CheckData('O');
end;

procedure UndelBlock;
begin
  mark_block(blk_end);
  Undelete;
  mark_block(blk_beg);
  with marker[blk_end] do
    if (class = scrn) and (cl > EndLine(LineNo)) then cl:=EndLine(LineNo);
  if (ColNo > 1) and (EditScreen[LineNo,ColNo-1] = #255) then
    EditScreen[LineNo,ColNo-1]:=' ';
  CheckData('P');
end;

procedure MoveBlock;
begin
  change;
  if DeleteBlock then
    UndelBlock;
  CheckData('Q');
end;

procedure CopyBlock;
  var oldline: LineCnt;
begin
  change;
  oldline:=LineNo;
  with marker[prv_cur] do
    begin class:=scrn; ln:=LineNo; cl:=ColNo end;
  if PickBlock then
  begin
    CheckData('Y');
    show(marker[prv_cur].index,oldline);
    UndelBlock;
  end else
    show(marker[prv_cur].index,oldline);
  CheckData('R');
end;

{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name:                                                      \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Command Dispatcher                                            \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

procedure PerformCmd(Cmd: char);
{ receives control from Editor to dispatch a built in command }
  const Searchstr: string = '';
        SubsStr: string = '';
        IgnCase: boolean = true;
        Ask: boolean = true;
        Global: boolean = true;
        fname: string = '';
        LastWasFind: boolean = true;
  var Cursor: word;
      ErrMsg: TextLine;
      i: integer;
begin
  Push_Help_Page(0);
    CheckData('S');
    if ScreenSaved then ErrorMsg('ScreenSaved error 3',true);
    FailCmd:=false;
    ErrMsg:='';
    menu_cmd(Cmd,AboveScreen,cursor,ErrMsg,CompileChangeFlag,
      Searchstr,SubsStr,fname,IgnCase,Ask,Global);
    if ErrMsg = '' then
    begin
      case Cmd of
        ' ',#27: ShortFileName;       { null command }
        'B': DeleteLeftChar;          { destructive backspace }
        'C': begin                    { copy from Undo Stack }
               CopyUndelete(CopyUndeleteLineCnt);
               CopyUndeleteCmdCnt:=0;
               CopyUndeleteLineCnt:=succ(CopyUndeleteLineCnt);
             end;
        'D': DeleteLine;              { Delete line }
        'E': Show(TextSize-2,PhyScrRows-3); { Bottom of file }
        'F': begin                    { find string }
               Find(Searchstr,IgnCase,false);
               LastWasFind:=true;
             end;
        'G': LineNo:=ScreenTop;       { Top of page }
        'H': SetLineNo(PhyScrRows);   { Bottom of page }
        'I': MoveTo(prv_cur);         { goto prev posn }
        'J': MoveTo(Blk_Beg);         { move to beginning block }
        'K': MoveTo(Blk_End);         { move to end block }
        'L': RightWord;               { move to next word }
        'M': LeftWord;                { move to prev word }
        'N': NewLine;                 { make a new line }
        'O': DelWord;                 { del next word }
        'R': begin                    { read block }
               MsgLine('[Wait]',false);
               SaveScreen;
               marker[blk_beg].class:=abv;
               marker[blk_beg].index:=AboveScreen.top+1;
               marker[blk_end].class:=blw;
               marker[blk_end].index:=BelowScreen.top;
               ReadFile(fname);
             end;
        'S': begin                    { substitue strings }
               Substitute(Searchstr,SubsStr,IgnCase,Ask,Global);
               LastWasFind:=false;
             end;
        'T': Show(TextBufLo+3,ScreenTop); { top of file }
        'U': Undelete;                { Undo last deletion }
        'V': DownPage;                { down page }
        'W': FailCmd:=not DeleteBlock;{ delete block }
        'X': CopyBlock;               { copy block }
        'Y': MoveBlock;               { move block }
        'Z': DeleteTextRight;         { Delete Text to eol }
        '[': ColNo:=1;                { Beginning of current line }
        ']': ColNo:=EndLine(LineNo);  { End of current line }
        '<': LeftChar;                { move cursor left }
        '>': RightChar;               { move cursor right }
        '}': begin                    { indent line }
               ColNo:=1;
               ProcessText(' ');
               ProcessText(' ');
               ColNo:=1;
               Downline;
             end;
        '{': begin                    { outdent line }
               ColNo:=1;
               for i:=1 to 2 do
                 if EditScreen[LineNo,ColNo  ] = ' ' then
                   DeleteRightChar else FailCmd:=true;
               ColNo:=1;
               Downline;
             end;
        '-': UpLine;                  { move cursor up }
        '+': DownLine;                { move cursor down }
        '.': DeleteRightChar;         { Delete right }
        '?': if LastWasFind then      { repeat last find/subs }
               Find(Searchstr,IgnCase,false){ find string }
             else                     { substitue strings }
               Substitute(Searchstr,SubsStr,IgnCase,Ask,Global);
        'a': init_new('');            { new }
        'b': FailCmd:=not WriteFile(FileName); { save Text }
        'c': begin                    { write Text to file }
               if fname <> 'PRN' then SetFileName(fname);
               FailCmd:=not WriteFile(fname);
             end;
        'd': begin                    { save file and exit }
               if WriteChangeFlag then
               case YorN('File changed, save file (Y/N):') of
                 'Y': StopEditor:=WriteFile(FileName);
                 'N': StopEditor:=true;
               end else
                 StopEditor:=true;
             end;
        'e': begin                    { load fname }
               if WriteChangeFlag then
               case YorN('File changed, save file (Y/N):') of
                 'Y': begin
                        FailCmd:=not WriteFile(FileName);
                        if not FailCmd then init_new(fname);
                      end;
                 'N': init_new(fname);
               end else
                 init_new(fname);
             end;
        '^': UpPage;                  { up page }
        'g': begin                    { insert line }
               ColNo:=1;
               NewLine;
               UpLine;
             end;
        'l': FailCmd:=WriteBlock(fname); { save block to file }
        't': tab;                     { tab }
        'u': begin                    { compile }
               consolidate(cursor);
               ResetSmall;
               compile_prog(AboveScreen,cursor,ErrMsg);
               ResetBig;
               CompileChangeFlag:=ErrMsg <> '';
               show(cursor,0);
               show_menu_bar;
             end;
        'v': begin                    { run }
               if CompileChangeFlag then
               begin
                 consolidate(cursor);
                 ResetSmall;
                 compile_prog(AboveScreen,cursor,ErrMsg);
                 ResetBig;
                 CompileChangeFlag:=ErrMsg <> '';
                 show(cursor,0);
               end;
               if not CompileChangeFlag then
               begin
                 if AutoSave and WriteChangeFlag then
                   FailCmd:=not WriteFile(FileName);
                 if not FailCmd then
                 begin
                   consolidate(cursor);
                   ResetSmall;
                   run_prog(AboveScreen,cursor,ErrMsg);
                   ResetBig;
                 end;
                 show(cursor,0);
               end;
               show_menu_bar;
             end;
        'w': mark_block(blk_beg);
        'x': mark_block(blk_end);
        'y': begin                             { OS shell }
               consolidate(cursor);
               ResetSmall;
               clrscr;
               command_com('');
               ResetBig;
               show_menu_bar;
               show(cursor,0);
             end;
        else help_screen;
      end;
    end;
    if ErrMsg <> '' then ErrorMsg(ErrMsg,true);

  marker[prv_cur]:=marker[this_cur];
  with marker[this_cur] do
    begin class:=scrn; ln:=LineNo; cl:=ColNo end;

  MsgLine('',false);
end;

procedure Edit;
{ initialises everything
{ repeat
{   update the Screen
{   get the next character from the Keyboard, and decide whether
{   it is a command, for PerformCmd to process, or a Text char for
{   ProcessText }
  var ch: char;
begin
  Initialise;

  repeat
   while FileName = '' do
      PerformCmd(#189);

    UpdPhyScr;
    ch:=GetBigKey;
    if (ch >= ' ') and (ch <= LastAlpha) then
      ProcessText(ch)
    else
      PerformCmd(ch);
  until StopEditor;

  clrscr;
end;

end.

