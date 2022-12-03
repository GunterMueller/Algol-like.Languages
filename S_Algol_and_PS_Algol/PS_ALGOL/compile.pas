{ --------------------------------- }
{   COMPILER  DRIVER                }
{ --------------------------------- }

{$R+}    {Range checking off}
{$B+}    {Boolean short circuiting off}
{$S+}    {Stack checking on}
{$N-}    {No numeric coprocessor}

unit compile;

interface

Uses

  Crt,
  Dos,
  Printer,
  Screen,
  EditDecl,
  Edit_err,
  Menu,
  errors,
  fsm,

{$ifdef    assembler}
  cgendum ,
{$endif}
  symtab,
  sasyn,
  exect,
{$ifndef assembler}
  assemble,   sagen,
{$endif}
  env;


const CtrlZ = #26;
      ObjectFileSuffix = '.out';


procedure run_prog(Text: TextStack; var cursor: word;
    var ErrMsg: TextLine);
procedure compile_prog(Text: TextStack; var cursor: word;
    var ErrMsg: TextLine);

function batchcompileit:boolean;

Implementation

var comstr,filestem,fullfile:string[70];
    compiled: boolean;
    codefile:file;
    stoponline: integer; {0 means don't stop}

procedure transtext(var B:textbuffer;  ts:textstack);
var f:file of byte; i:integer; c:byte;
begin
   with B do begin
     textlen:=ts.top  -textbuflo+3;

     linenumber:=1;
     begin
          finish:=0;
          thetext:=ts.buf;
     end;
     thetext^[textlen-2]:='?';thetext^[textlen-1]:=' ';
     start:=1;finish:=1;
   end;
end;

procedure pause;
        begin
            gotoxy(1,24);writeln;
            write('type CR to return to editor >');readln;
        end;

procedure ping(freq:integer);
begin
                    sound(freq); delay(500);
                    nosound;
end;

   function objectfile:textline;
    var f:textline;
    begin
         f:=FileName;
         delete (f,length(f)-1,2);    { remove the S from the file.S }

         objectfile:=f;
     end;

    procedure assembler
    (Var errnum:integer;var errmsg:textline);

    { this runs the assembler as a  daughter process }
    var return_code,exerr: word;
        params:string[250];
    begin
       if not(batch in options) then clrscr;
       {$ifdef assembler}
       params:=MACROLIB+' '+ASM+' to '+Objectfile+ObjectFileSuffix;
       exerr:=execute(PASS2PROG,params,return_code);
       if exerr<>0 then begin
          errnum:=250;
          errmsg:=exec_error(exerr);
       end else
       if return_code <> 0 then begin
          errnum:=250;
          errmsg:='Error during assembly';
       end;{$endif }
       {$ifndef assembler}
       params:=Objectfile+ObjectFileSuffix;
       assign(codefile,params);
       rewrite(codefile,1);
       pass2 (codefile);
       close(codefile);

       {$endif}

    end;
    function batchcompileit:boolean;
                   var ErrNum: integer;
                   var ErrMsg: TextLine;
    begin
              synver;
              codever;
           clear_errors;
           rewind;
           assign(asmfile,ASM);
           rewrite(asmfile);
           ERRORRETURNHERE;
           batchcompileit:=TRUE;
           if errorfree then prog;
           if NOT ERRORFREE then begin
              batchCOMPILEit:=FALSE;
              close(asmfile);
              assign(asmfile,ASM);
              rewrite(asmfile) ;
           end ;
           close(asmfile);
           if errorfree  then  assembler(errnum,errmsg);
        end;


procedure compile_prog(Text: TextStack; var cursor: word;
    var ErrMsg: TextLine);
  var errnum: integer;
begin
           ErrMsg:='';
           stopline:=stoponline;
           clrscr;
           if stoponline=0 then begin
              { this is a standard compilation not an error finding }
              gotoxy(10,10);
              synver;
              GOTOXY(10,11);
              codever;
              gotoxy(10,12);write('Copyright (C) Paul Cockshott 1989');
              Gotoxy(1,1);write('Line :');
              gotoxy(1,15);

           end
           else begin
                gotoxy(10,10);
                write('Searching for Error ');
                gotoxy(1,15);
           end;
           transtext(THE_BUFFER,text);
           clear_errors;
           rewind;

           assign(asmfile,ASM);
           rewrite(asmfile);

           ErrorReturnHere;
           if ErrMsg = '' then
           begin
             prog;

             if NOT ERRORFREE then begin
                close(asmfile);
                assign(asmfile,ASM);
                rewrite(asmfile) ;
             end ;
             close(asmfile);
             if compilererror<>0 then begin
                errmsg:=compilermsg;
                cursor:=errorcursor;
                if cursor>THE_BUFFER.textlen then cursor:=0;
             end;
             if errorfree and (stoponline=0) then
               assembler(errnum,errmsg);
           end;
end;

procedure run_prog(Text: TextStack; var cursor: word;
    var ErrMsg: TextLine);
    { this runs the run time library which in turn runs the object file }
    var exerr,return_code: word;
        params:string[250];
begin
       Errmsg:='';
       clrscr;
       params:=parameters;
       exerr:=execute(Objectfile+objectfilesuffix,params,return_code);
       if exerr<>0 then begin
          errmsg:=exec_error(return_code);  { this returns an error from the }
                                            { failed attempt to run program }
       end else
       if return_code <> 0 then begin
          writeln;
          writeln('type cr to find error>');readln;
          textmode(co80);

          stoponline:=get_ps_return_code;
          compile_prog(Text,cursor,ErrMsg);
          stoponline:=0;
          errmsg:='Error during execution';
       end;
       writeln;
end;

begin
compiled:=false;
stoponline:=0;
extension:='S';
end.

