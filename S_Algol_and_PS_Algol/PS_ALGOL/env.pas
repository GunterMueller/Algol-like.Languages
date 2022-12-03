Unit env;
{ Copied from page 423 of Mastering Turbo Pascal }
{ this looks up a textline in the environment
  of the program
  }
interface

uses editdecl;

procedure GetEnvVar(variable:textline;var value:textline);

implementation
type charptr=^char;
var s:textline;


procedure AdvanceCP(var p:charptr);
begin
     p:=Ptr(seg(p^),ofs(P^)+1);
end;

Function getenv(s:textline):charptr;
var p:charptr;
    function match(Var s:textline;Var p:charptr):boolean;
    var i:integer;
    begin
         for i:=1 to length(s) do
         begin
              if p^<s[i] then
              begin
                   match:=false;
                   exit;
              end;
              AdvanceCP(p);
          end;
          match:=p^ = '=';
     end;
begin
     p:=ptr(Memw[Prefixseg:$002c],0);
     while p^<>chr(0) do
     begin
        if match(s,p) then
        begin
             advancecp(p);getenv:=p;exit;
        end;
        while p^<>chr(0) do advancecp(p);
        advancecp(p);
     end;
     getenv:=nil;
end;

procedure asciiztostr(p:charptr;var s:textline);
var count:integer;
begin
     count:=0;
     if p<>nil then while (p^<>chr(0)) and (count<255) do
     begin
          count:=count+1;
          s[count]:=p^;
          advancecp(p);
     end;
     s[0]:=chr(count);
end;



procedure GetEnvVar(variable:textline;var value:textline);
begin
   asciiztostr(getenv(variable),value);
end;
begin
  getenvvar('PSDIR',PSDIR);
  if psdir[length(psdir)]<>'\' then
     if length(psdir) >0 then
        psdir:=psdir+'\';
  MACROLIB :=PSDIR+ 'salib.8';
  PASS2PROG := PSDIR+'A86.com';


end.