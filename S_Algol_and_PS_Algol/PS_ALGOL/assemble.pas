unit assemble;
{ this assembles the PS-algol abstract machine code into machine code }
{ it builds up an abstract program first then in a second pass it outputs }
{ this to a comfile }
interface uses errors,opcodes,editdecl;
const labmax =2000;
type codefile=file ;

procedure pass2(var comfile:codefile);
procedure assem(op:opcode;p1:integer;p2 : integer);
procedure initassem;

implementation
const pcmax = 12000;
      start =256;

type pseudo = record      { abstract instruction }
             operator:opcode;
             parm2:integer;
             parm1:integer;
             end;
     codevec =array[0..pcmax]of pseudo;
Var pc:word;
    pseudocode :^codevec;
procedure initassem;
begin
     pc:=0;
     new(pseudocode);
end;
procedure pass2(var comfile:codefile);
const bufsize =511;
var machinepc,trav:word;
    labels:array[0..labmax]of word;
    buff:array[0..bufsize] of byte;
    procedure flush;
    var w:word;
    begin
              blockwrite(comfile,buff,bufsize+1,w);
    end;
    procedure fixlabels;
      var i:word;
      begin
           machinepc:=start;
           for i:=0 to pc do
               with pseudocode^[i] do begin
                    if operator=plant_label then labels[parm1]:=machinepc;
                    machinepc:=machinepc+codelen[operator];
               end
       end;
     procedure emitcode(c:word);
     var i,p1,p2,offs:integer;dest:word;
         w :word;
         procedure emitbyte(b:byte);
         begin
              buff[(machinepc-start) and bufsize]:=b;
              machinepc:=machinepc+1;
              if(( machinepc-start) and bufsize)=0 then flush;
              i:=i+1;
         end;
         procedure advance;
         var b:integer;
         begin
             b:=codelib[i+offs];
             emitbyte(codelib[i+offs]);
         end;
         procedure emitword(w:integer);
         var r:record
                case boolean of
                true:(ww:integer);
                false:(b:array[0..1] of byte)
                end;
         begin
              r.ww:=w;
              emitbyte(r.b[0]);emitbyte(r.b[1]);

         end;

var conv:record case boolean of
         true:(pw:word);
         false:(pi:integer);
         end;

     begin

         with pseudocode^[c] do   begin
           offs:=codeoffset[operator];
           p1:=param1[operator]; p2:=param2[operator];
           i:=0 ;
           while i< codelen[operator] do begin
             if (i=p1) or (i =p2) then
             case codeparams[operator] of
             relative: if i=p1 then begin
                          dest:=labels[parm1]-2-machinepc;
                          conv.pw:=dest;
                          emitword(conv.pi);
                       end else advance;
             byterel: if i=p1 then begin
                          dest:=labels[parm1]-1-machinepc;
                          emitbyte(dest);
                       end else advance;
             abslabel: if i = p1 then emitword(labels[parm1] )
                       else advance;
             nonadic:advance;
             dyadic: if p1 = i then emitword(parm1)
                     else emitword( parm2);
             byteadic: if p1= i then emitbyte(parm1) else advance;
             monadic : if p1=i then
                        emitword(parm1)
                       else advance;
             end
             else advance;
           end;
         end;
     end;

begin
    pc:=pc-1;    { pc pointed at next free instruction so pull back 1 }
    fixlabels;
    machinepc:=start;
    for trav:=0 to pc do emitcode(trav);
    flush;
end;
procedure assem(op:opcode;p1:integer;p2:integer);
var conv:record case boolean of
         true:(pw:word);
         false:(pi:integer);
         end;
begin
     conv.pi:=p1;
     with pseudocode^ [pc] do begin
          parm1:=conv.pi;
          operator:=op;
          parm2:=p2;
     end;
     if trace then begin
       write(listfile,'     ',
                        pc:4,' ',codelits[op],' ',p1:6,' ',p2:6);
       if( 1<p1) and (p1<128) then writeln(listfile,' ',chr(p1))
       else writeln(listfile);
     end;
     pc:=pc+1;
     if pc>pcmax then begin
       pc:=pcmax;
       error('code to large');
     end;
end;

begin
end.