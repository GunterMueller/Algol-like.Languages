{---------------------------------------------------}
{    CODETAB GEN                                    }
{        used to build code generator tables        }
{        this reads in a file of abstract opcodes   }
{        on the standard input,                     }
{        sorted into 0, 1 , 2 operand classes       }
{        the format of this file is a set of lines  }
{        a line starting with ; is a comment        }
{        a line sarting with = followed by a digit  }
{        specifies the number of operands in the    }
{        instructions that follow                   }
{        1 = 1 16 bit operand                       }
{        2 = 1 16 bit operand and 1  8 bit operand  }
{        3 = 1 string operand                       }
{        4 = 1 byte operand                         }
{        5 = a relative branch                      }
{        6 = a 1 byte relative branch               }
{        7 = an absolute label                      }
{        a line starting with a * marks the end of  }
{        the file                                   }
{        other lines are treated as opcodenames     }
{        It generates as output                     }
{           a) a assembler program on standard      }
{              output                               }
{           b) a pascal unit that defines types     }
{              for the abstract machine instructions}
{---------------------------------------------------}
{   Copyright Paul Cockshott 1989                    }
program codetabgen;
uses exect;
const opcodelen = 17;
      opcodemax = 255;
      param1 =12345;
      param2 =23456;
      param3 :string[4] ='abcd';
      param4 = 199;
      param6 =99;
      maximum_op_type = 7;
      codemax=8000;
      halfcodemax=4000;
type
      opcode =  string[opcodelen];
      coderange = 0..opcodemax;
var   codenumber:coderange;
      codelist: array[coderange] of opcode;
      lengths:array [coderange] of integer;
      operands : array [0..maximum_op_type] of set of coderange;
      unitfile,assemfile,def:text;
      codefile:file of byte;
      code:record
                 case boolean of
                 true :(bytestream:array [-1..codemax] of byte);
                 false:(wordstream:array [-2..opcodemax] of integer);
                 end;
      chartoint:record
                      case boolean of
                      true :(int:integer);
                      false:(ch:array[1..2]of byte);
                      end;
      current_operands:0..maximum_op_type;
      lastcode:integer;

procedure startunit;
       var i:coderange;
       begin
            writeln(unitfile,'unit opcodes;');
            writeln(unitfile,'{ Copyright (c) Paul Cockshott}');
            writeln(unitfile,'interface');
            writeln(unitfile,'type opcode = (');
            for i:=0 to codenumber do writeln(unitfile,' ',codelist[i],',');
            writeln(unitfile,'stringlit,reallit,trademark,plant_label);');
            writeln(unitfile,'opmnemonic=string[17];');
            writeln(unitfile,'opstr = string[50];');
            writeln(unitfile,'optype =(nonadic,monadic,dyadic,stringadic,byteadic,relative,byterel,abslabel);');
            writeln(unitfile,'const codeparams:array[opcode]of optype =(');
            for i:=0 to codenumber do begin
                write(unitfile,'{',codelist[i],'}');
                if (i in operands[0])  then writeln(unitfile,'nonadic,')
                else if i in operands[1] then writeln(unitfile,'monadic,')
                else if i in operands[2] then writeln(unitfile,'dyadic,')
                else if i in operands[5] then writeln(unitfile,'relative,')
                else if i in operands[6] then writeln(unitfile,'byterel,')
                else if i in operands[7] then writeln(unitfile,'abslabel,')
                else if i in operands[3] then writeln(unitfile,'stringadic,')
                else writeln(unitfile,'byteadic,');
            end;
            writeln(unitfile,'nonadic,nonadic,nonadic,nonadic);');
            writeln(unitfile,'const codelits:array[opcode]of opmnemonic=(');

            for i:=0 to codenumber do writeln(unitfile,'''',codelist[i],''',');
            writeln(unitfile,'''stringlit'',''reallit'',''trademark'', ''plant_label'');');

       end;
function findparam1(c:coderange):integer;

       var found:boolean;
       procedure find(param1:integer);
       var i:integer;
       begin
            chartoint.int:=param1;
            found:=false;
            for i:=code.wordstream[c] to code.wordstream[c]+lengths[c] -1 do
            begin
                if (chartoint.ch[1]=code.bytestream[i]) and
                   (chartoint.ch[2]=code.bytestream[i+1])
                then begin
                    if found then writeln('duplicate param in ',codelist[c])
                    else begin
                       found:=true;
                       findparam1:=i-code.wordstream[c];
                    end;
                end;
            end;
       end;
       begin
            find(param1);
            if found=false then writeln('param 1 not found in ',
                                        codelist[c],
                                        code.wordstream[c]);
       end;
function findparam2(c:coderange):integer;
       var i:integer;found:boolean;
       begin
            chartoint.int:=param2;
            found:=false;
            for i:=code.wordstream[c] to code.wordstream[c]+lengths[c] -1 do
            begin

                if (chartoint.ch[1]=code.bytestream[i]) and
                   (chartoint.ch[2]=code.bytestream[i+1])
                then begin
                    if found then writeln('duplicate param in ',codelist[c])
                    else begin
                       found:=true;
                       findparam2:=i-code.wordstream[c];
                    end;
                end;
            end;
            if found=false then writeln('param 2 not found in ',codelist[c]);
       end;
function findparam5(c:coderange):integer;
       var i:integer;found:boolean;
       begin
            found:=false;
            for i:=code.wordstream[c] to code.wordstream[c]+lengths[c] -1 do
            begin
                { search for branch address }
                chartoint.int:=param1-(256+i+2) ;{ allow for com file to be at 100h}
                                                 { allow for 2 bytes of the address}
                if (chartoint.ch[1]=code.bytestream[i]) and
                   (chartoint.ch[2]=code.bytestream[i+1])
                then begin
                    if found then writeln('duplicate param in ',codelist[c])
                    else begin
                       found:=true;
                       findparam5:=i-code.wordstream[c];
                    end;
                end;
            end;
            if found=false then writeln('param 5 not found in ',codelist[c]);
       end;
function findparam3(c:coderange):integer;
       var i:integer;found:boolean;
       begin
            found:=false;
            for i:=code.wordstream[c] to code.wordstream[c]+lengths[c] -1 do
            begin
                if (ord(param3[1])=code.bytestream[i]) and
                   (ord(param3[2])=code.bytestream[i+1]) and
                   (ord(param3[3])=code.bytestream[i+2])
                then begin
                    if found then writeln('duplicate param in ',codelist[c])
                    else begin
                       found:=true;
                       findparam3:=i-code.wordstream[c];
                    end;
                end;
            end;
            if found=false then writeln('param 3 not found in ',codelist[c]);
       end;
function findparam4(c:coderange):integer;
       var i:integer;found:boolean;
       begin
            found:=false;
            for i:=code.wordstream[c] to code.wordstream[c]+lengths[c] -1 do
            begin
                if (param4=code.bytestream[i])
                then begin
                    if found then writeln('duplicate param in ',codelist[c])
                    else begin
                       found:=true;
                       findparam4:=i-code.wordstream[c];
                    end;
                end;
            end;
            if found=false then writeln('param 4 not found in ',codelist[c]);
       end;
function findparam6(c:coderange):integer;
       var i:integer;found:boolean;
       begin
            found:=false;
            for i:=code.wordstream[c] to code.wordstream[c]+lengths[c] -1 do
            begin
                if (param6=code.bytestream[i]+2)
                then begin
                    if found then writeln('duplicate param in ',codelist[c])
                    else begin
                       found:=true;
                       findparam6:=i-code.wordstream[c];
                    end;
                end;
            end;
            if found=false then writeln('param 4 not found in ',codelist[c]);
       end;
procedure finishunit;
       var i ,j,k,l:integer;
           base,limit:integer;
           p1,p2:array [coderange] of integer;
       begin
            writeln(unitfile,'codelen:array[opcode] of byte=(');
            for i:=0 to codenumber do writeln(unitfile,lengths[i],',');
            writeln(unitfile,'0,0,0,0);');
            writeln(unitfile,'codeoffset:array[opcode]of integer=(');
            for i:=0 to codenumber do
              writeln(unitfile,'{',codelist[i],'}   ',code.wordstream[i],',');
            writeln(unitfile,'0,0,0,0);');
            base:=code.wordstream[0];
            limit:=code.wordstream[codenumber]+lengths[codenumber];

            writeln(unitfile,'const codelib:array[',base,'..',limit+1,']of byte=(');
            j:=-1;k:=0;
            for i:=base to limit do begin
               { j holds current opcode }
               if k mod 8 = 0 then writeln(unitfile);
               if k<=0 then begin
                  writeln(unitfile);
                  j:=j+1; k:=lengths[j];

                write(unitfile,'{',codelist[j],'}');
                  p1[j]:=0;p2[j]:=0;
                  if j in operands[1]+operands[7] then p1[j]:=findparam1(j)
                  else if j in operands[2] then begin
                       p1[j]:=findparam1(j);
                       p2[j]:=findparam2(j);
                  end
                  else if j in operands[6] then p1[j]:=findparam6(j)
                  else if j in operands[5] then p1[j]:=findparam5(j)
                  else if j in operands[3] then
                       p1[j]:=findparam3(j)
                  else if j in operands[4] then p1[j]:=findparam4(j);

               end;
               write(unitfile,code.bytestream[i],',');
               k := k - 1;

            end;
            writeln (unitfile,'0);');
            writeln (unitfile,'param1:array[opcode] of byte=(');
            for i:=0 to codenumber do begin

                write(unitfile,'{',codelist[i],'}'); writeln(unitfile,p1[i],',');
            end;
            writeln (unitfile,'0,0,0,0);');
            writeln (unitfile,'param2:array[opcode] of byte=(');
            for i:=0 to codenumber do begin
                write(unitfile,'{',codelist[i],'}');
               writeln(unitfile,p2[i],',');
            end;
            writeln (unitfile,'0,0,0,0);');
            writeln(unitfile,'implementation begin end.');
            close(unitfile)
       end;
procedure getcode;
      var i:integer;
      begin
           lastcode:=-1;
           while not eof(codefile) do
           begin
                lastcode:=lastcode+1;
                read(codefile,code.bytestream[lastcode]);
           end;
           for i:=0 to codenumber  do
           begin
                lengths[i]:=code.wordstream[i+1]-code.wordstream[i];
                code.wordstream[i]:=  code.wordstream[i]-256;
           end;
      end;

procedure assemprog;
      var i:coderange;
          retc,rc:word;
      begin
            writeln(assemfile,'_samain:  ');
            for i:=0 to codenumber do
            begin
                 writeln(assemfile,' dw ',codelist[i],'start');
            end;
            writeln(assemfile,' dw fin');
            for i:= 0 to codenumber do begin
                 write(assemfile,codelist[i],'start: ');
                 write(assemfile,codelist[i],' ');
                 if (i in operands[1] ) or (i in operands[5]+operands[7])
                 then writeln(assemfile,param1)
                 else if i in operands[2] then writeln(assemfile,param1,', ',param2)
                 else if i in operands[3] then { string param }
                      writeln(assemfile,param3)
                 else if i in operands[4] then  writeln(assemfile,param4)
                 else if i in operands[6] then  writeln(assemfile,'$+',param6)
                 else writeln(assemfile);
            end;
            writeln (assemfile,'fin: db 0');writeln(assemfile,' end ');
            close(assemfile);
            rc:=execute('a86.com','salib.8 opcodes.asm to opcodes.com',retc);
            assign(codefile,'opcodes.com');
            reset(codefile);
       end;




procedure init;
      var i:0..maximum_op_type;
      begin
           assign(assemfile,'opcodes.asm');
           rewrite(assemfile);
           assign(unitfile,'opcodes.pas');
           rewrite(unitfile);
           assign(def,'opcodes.def');
           reset(def);
           codenumber:=0;
           current_operands:=0;
           for i:=0 to maximum_op_type do operands[i]:=[];
      end;
procedure readlines;
      var finished:boolean; op:opcode;
      begin
           finished:=false;
           while not finished do
           begin
                readln(def, op );
                if length(op)>0 then
                if op[1]<> ';' then begin
                   if op[1]='*' then finished:=true
                   else if op[1]='=' then begin
                        if length(op)>1 then  begin
                          if op[2] in ['0'..'7'] then
                            current_operands:=ord(op[2])  - ord('0')
                        end else begin
                            writeln('= without digit in opcodes.def ');
                            halt;
                        end
                   end
                   else begin
                       operands[current_operands]:=
                           operands[current_operands]+[codenumber];
                       codelist[codenumber]:=op;
                       codenumber:=codenumber+1;
                   end;
                end;
            end;
            codenumber:=codenumber-1;
       end;

begin
     init;
     readlines;
     startunit;
     assemprog;
     getcode;
     finishunit;
end.