{
File       : error.cmp
Part of    : Compiler Tool Box
Author     : W P Cockshott, & P Balch
Date       : 21 -9- 88
Function   : To provide error reporting mechanism to be used
             in the compiler
}
UNIT errors;
INTERFACE
   USES
     editdecl,
     edit_err,
     fsm;

var CompilerError:byte;
    Compilermsg: textline;
    CompilerCursor:word;
    ErrorCursor: word;
    options : set of (library, batch,listing);
procedure Error(msg: textline);
function errorfree:boolean;
procedure clear_errors;
IMPLEMENTATION

const safety = 500;
var execute : boolean;
    parameter : integer;

procedure Error(msg: textline);
const leadin='<***';leadout='***>';
begin
 if batch in options then writeln(leadin,msg,leadout);
   writeln(listfile,leadin,msg,leadout);
 if CompilerError = 0 then begin
  CompilerError:=220;
  errorcursor:=CompilerCursor;
  CompilerMsg:=msg;
  ErrorReturn;
end;

end;

function errorfree:boolean;
begin
     if Maxavail < safety then error('Memory low');
     errorfree:=compilererror=0;
end;

{$f+}
function heaperrortrap(size:word):integer;
begin
  if incompiler then
  begin
    error('out of memory');
    heaperrortrap:=1;
  end else
    heaperrortrap:=0;
end;

var oldexitproc:pointer;

{$s-}
procedure errorhandler;
begin
  if erroraddr<>nil then
  begin
     write('Run time error ',MessageNum(exitcode));
     if incompiler then
       write(' in compiler: ') else
       write(': ');

     writeln(hextostr(seg(erroraddr^),4),':',hextostr(ofs(erroraddr^),4));
     if exitcode=202 then begin
        writeln('Check your grammer rules they may be left recursive ');
     end;

     {erroraddr:=nil;exitcode:=0;}
  end;
  exitproc:=oldexitproc;
end;
{$s+}

{$f-}
procedure clear_errors;
begin
    compilererror :=0;
    CompilerMsg:='';
    execute:=false;
end;
begin
  incompiler:=false;
  heaperror:=@heaperrortrap;
  oldexitproc:=exitproc;
  exitproc:=@errorhandler;
  clear_errors;
end.

