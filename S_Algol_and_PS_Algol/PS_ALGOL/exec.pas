Unit Exec;

Interface

Uses
  DOS;

function execute(prog,parameters: string; var return_code: word): word;
procedure command_com(batch_file: string);
function exec_error(err: word): string;
function get_ps_return_code:integer;

Implementation

procedure call_DOS;
begin
  inline(
     $90/$90/$90/$90/$90/$90/$90/$90/$90/$90/$90/$90 {NOP}
    /$90/$90/$90/$90/$90/$90/$90/$90/$90/$90/$90/$90/$90
    /$2E                {  CS:                         }
    /$8C /$16 /$02 /$00 {  MOV     [old_SS],SS         }
    /$2E                {  CS:                         }
    /$89 /$26 /$04 /$00 {  MOV     [old_SP],SP         }
    /$1E                {  PUSH    DS                  }
    /$07                {  POP     ES                  }
    /$B8 /$00 /$4B      {  MOV     AX,$4B00            }
    /$CD /$21           {  INT     21                  }
    /$FA                {  CLI                         }
    /$2E                {  CS:                         }
    /$8E /$16 /$02 /$00 {  MOV     SS,[old_SS]         }
    /$2E                {  CS:                         }
    /$8B /$26 /$04 /$00 {  MOV     SP,[old_SP]         }
    /$FB                {  STI                         }
    /$CB                {  RETF                        }
  );
end;

function get_ps_return_code:integer;
{ this function does a return code call to the PS run time library }
{ it is assumed that int 18 is the entry point to the PS dynamic linker }
{ it obtains the line number on which the PS program aborted if it failed }
var err:integer;
begin
     inline(
      $CD /$18           {  INT     18                  }
      { /'R' /'e' /'t' /'u' /'r' /'n' /'c' /'o' /'d' /'e' /0}
       /$52  /$65 /$74 /$75 /$72 /$6e /$63 /$6f /$64 /$65 /0
      /$A3 />err         {  MOV  [err],AX               }
      );
      get_ps_return_code:=err;
 end;
   

function exec_prog(prog,parameters: string): word;
  const exec_block: record
          envseg: word;
          param_addr: pointer;
          fcb1ofs,fcb1seg,fcb2ofs,fcb2seg: word;
        end = (envseg: 0; param_addr: nil; fcb1ofs: $FFFF;
          fcb1seg: $FFFF; fcb2ofs: $FFFF; fcb2seg: $FFFF);
        prog_name: string = '';
        err: word = 0;
        rtn_addr: pointer = nil;
  var i: word;
      f,ax,bx: word;
begin
  with exec_block do
  begin
    move(prog[1],prog_name[0],length(prog));
    prog_name[length(prog)]:=#0;

    parameters[length(parameters)+1]:=#13;
    param_addr:=@parameters;

    i:=cseg+((ofs(call_DOS)+15) shr 4);
    while memw[i:0] <> $9090 do inc(i);
    rtn_addr:=@mem[i:6];

    inline(
       $BA />prog_name     {  MOV     DX,ofs(prog_name)    }
      /$BB />exec_block    {  MOV     BX,ofs(exec_block)   }
      /$1E                 {  PUSH    DS                   }
      /$55                 {  PUSH    BP                   }
      /$FF /$1E />rtn_addr {  CALL    FAR [rtn_addr]       }
      /$5D                 {  POP     BP                   }
      /$1F                 {  POP     DS                   }
      /$72 /$03            {  JC      L1                   }
      /$B8 />0             {  MOV     AX,0000              }
      /$A3 />err           { L1: MOV  [err],AX             }

      /$A3 />ax            { L1: MOV  [ax],AX             }
      /$9C                 {pushf}
      /$58                 {pop ax}
      /$A3 />f             { L1: MOV  [f],AX             }
      /$53                 {push bx}
      /$58                 {pop ax}
      /$A3 />bx            { L1: MOV  [bx],AX             }
    );

    exec_prog:=err;
  end;
end;

function psp_addr: pointer;
  var r: registers;
begin
  with r do
  begin
    ax:=$6200;
    msdos(r);
    psp_addr:=@mem[bx:0];
  end;
end;

procedure modify_mem_alloc(block_addr: pointer; var block_size,err: word);
  var r: registers;
begin
  with r do
  begin
    ax:=$4A00;
    bx:=block_size;
    es:=seg(block_addr^);
    msdos(r);
    block_size:=bx;
    if odd(flags) then err:=ax else err:=0;
  end;
end;

function get_return_code: word;
  var r: registers;
begin
  with r do
  begin
    ax:=$4D00;
    msdos(r);
    get_return_code:=ax;
  end;
end;

function execute(prog,parameters: string; var return_code: word): word;
  type pspa= array [0..63] of word;
  var  psp:  ^pspa;
       block_size,err: word;
begin
  psp:=psp_addr;

  move(freeptr^,mem[seg(heapptr^):$10],-ofs(freeptr^));

  block_size:=seg(heapptr^)+(((-ofs(freeptr^)) shr 4) and $FFF)
    +4-seg(psp^);
  modify_mem_alloc(psp_addr,block_size,err);
  if err <> 0 then begin execute:=err; exit; end;

  execute:=exec_prog(prog,parameters);

  block_size:=psp^[1]-seg(psp^);
  modify_mem_alloc(psp_addr,block_size,err);
  if err <> 0 then begin execute:=err; exit; end;

  move(mem[seg(heapptr^):$10],freeptr^,-ofs(freeptr^));

  return_code:=get_return_code;
end;

procedure command_com(batch_file: string);
  var err,return_code: word;
begin
  if batch_file <> '' then
    batch_file:='/C '+batch_file
  else
    writeln('Type EXIT to return to program');
  err:=execute('\command.com',batch_file,return_code);
  if err <> 0 then writeln(exec_error(err),': \command.com');
end;

function exec_error(err: word): string;
  var s: string;
begin
  case err of
     0:  exec_error:='';
     1:  exec_error:='EXEC: function invalid';
     2:  exec_error:='EXEC: file not found';
     5:  exec_error:='EXEC: access denied';
     7:  exec_error:='EXEC: memory control blocks destroyed';
     8:  exec_error:='EXEC: insufficient memory';
    $A:  exec_error:='EXEC: environment invalid';
    $B:  exec_error:='EXEC: format invalid';
    else begin str(err,s); exec_error:='EXEC error: '+s; end;
  end;
end;

end.
