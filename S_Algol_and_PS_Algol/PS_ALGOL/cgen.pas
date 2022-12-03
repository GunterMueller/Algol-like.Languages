const

  execute = true;

  maxid = 500;
  nulid = 0;

type lexeme=(dummy_sy,
BRACE_SY,
RBRACE_SY,
STAR_SY,
PLUS_SY,
DPLUS_SY,
MINUS_SY,
ARROW_SY,
LT_SY,
LE_SY,
EQ_SY,
GT_SY,
GE_SY,
BRA_SY,
KET_SY,
ABORT_SY,
AND_SY,
BEGIN_SY,
CBOOL_SY,BOOL_SY,
BY_SY,
CASE_SY,
COLOUR_SY,
DEFAULT_SY,
DIV_SY,
DO_SY,
ELSE_SY,
END_SY,
EOF_SY,
FALSE_SY,
CFILE_SY,FILE_SY,
FOR_SY,
forward_sy,FROM_SY,
IF_SY,
IN_SY,
CINT_SY,INT_SY,
IS_SY,
ISNT_SY,
LET_SY,
LWB_SY,
NIL_SY,
OF_SY,OR_SY,
OUT_BYTE_SY,
PEEK_SY,
CPIC_SY,PIC_SY,
CPNTR_SY,PNTR_SY,
PROCEDURE_SY,
READ_SY,
READ_A_LINE_SY,
READ_BYTE_SY,
READ_NAME_SY,
READB_SY,
READI_SY,
READR_SY,
READS_SY,
CREAL_SY,REAL_SY,
REM_SY,
REPEAT_SY,
ROTATE_SY,
SCALE_SY,
SHIFT_SY,
CSTRING_SY,STRING_SY,
STRUCTURE_SY,
TEXT_SY,
THEN_SY,
TO_SY,
TRUE_SY,
UPB_SY,
VECTOR_SY,
WHILE_SY,
WRITE_SY,
CUR_SY,
LEY_SY,
TILDE_SY,
NEQ_SY,
SLASH_SY,
INT_LIT,
REAL_LIT,
STRING_LIT,
IDENTIFIER,
newline_sy,
bar_sy,
colon_sy,dcolon_sy,
assign_sy, comma_sy,  at_sy,
question_sy,
UNDEFINED);

Type
    namedesc = nulid..maxid;
    typerec  = record
               typeid     :lexeme;
               dimensions : byte;     { number of vector dimensions }
               field      : boolean;
               params,result: nulid..maxid;
               end;
    idrec    = record
               identifier : integer;
               offset: integer;
               lex_level    : byte;
               typeinfo   : typerec;
               end;

Type
  TextLine = string[80];
  string255 = string[255];
function ptype(var t: typerec): TextLine;
  var s: TextLine;
begin
  str(ord(t.typeid),s);
  ptype:=s;
end;

function psym(var t: lexeme): TextLine;
  var s: TextLine;
begin
  str(ord(t),s);
  psym:=s;
end;

{$I addr.cmp}
{$I rtl.cmp}
{$I assmcode.cmp}
{$I cgen.cmp}

procedure source(s: TextLine);
begin
  writeln(CON);
  writeln(CON,'------------------------------------');
  writeln(CON,s);
end;

var trA,trB,trC,trD,trP: idrec;
    lab1,lab2,lab3,lab4: integer;

begin
{------------------------------------
    let a:=9+10
    let c:=a*10

    procedure p(int d)
    begin
      let b:=1-a*(2 div a)
      if a>2 then
        b:=1-d
      else
        a:=b-1
      write(a,b,d)
    end

    p(c)
    ?
------------------------------------}

  init_cgen;

  source('let a:=9+10 ');
  ll_int(9);
  ll_int(10);
  binary_op(plus_sy);
  with trA,trA.typeinfo do
  begin
    typeid:=int_sy;
    dimensions:=0;
    field:=false;
    params:=0;
    result:=0;
    offset:=declare_global(trA);
    lex_level:=1;
  end;
    writeln (CON,'  new offset=',trA.offset);


  source('let c:=a*10');
  load(trA);
  ll_int(10);
  binary_op(star_sy);
  with trC,trC.typeinfo do
  begin
    typeid:=int_sy;
    dimensions:=0;
    field:=false;
    params:=0;
    result:=0;
    offset:=declare_global(trC);
    lex_level:=1;
  end;
    writeln (CON,'  new offset=',trC.offset);

  source('procedure p(int d)');
  lab4:=0;
  jump(lab4);
  with trP,trP.typeinfo do
  begin
    typeid:=PROCEDURE_SY;
    dimensions:=0;
    field:=false;
    params:=1;
    result:=0;
    offset:=program_counter;
    lex_level:=1;
  end;
  enter_stackframe;
  with trD,trD.typeinfo do
  begin
    typeid:=int_sy;
    dimensions:=0;
    field:=false;
    params:=0;
    result:=0;
    offset:=declare_param(trD);
    lex_level:=2;
  end;

  source('begin');
  fixup_param(trD);

  source('let b:=1-a*(2 div a); ');
  ll_int(1);
  load(trA);
  ll_int(2);
  load(trA);
  binary_op(div_sy);
  binary_op(star_sy);
  binary_op(minus_sy);
  with trB,trB.typeinfo do
  begin
    typeid:=int_sy;
    dimensions:=0;
    field:=false;
    params:=0;
    result:=0;
    offset:=declare_local(trB);
    lex_level:=2;
  end;

  source('if a>2 then ');
  load(trA);
  ll_int(2);
  binary_op(GT_SY);
  lab1:=0;
  jumpt(lab1);
  lab3:=0;
  jump(lab3);
  plant(lab1);
  release_label(lab1);

  source('b:=1-d');
  ll_int(1);
  load(trD);
  binary_op(minus_sy);
  assign_op(trB);

  source('else ');
  lab2:=0;
  jump(lab2);
  plant(lab3);
  release_label(lab3);

  source('a:=b-1 ');
  load(trB);
  ll_int(1);
  binary_op(minus_sy);
  assign_op(trA);
  plant(lab2);
  release_label(lab2);

  source('write(''a='',a,'' b='',b,'' d='',d)');
  start_write;
  write_str_const('a=');
  load(trA);
  ll_int(0);  { fieldwidth }
{  write_op(int_sy); {}
  write_str_const(' b=');
  load(trB);
  ll_int(0);  { fieldwidth }
{  write_op(int_sy); {}
  write_str_const(' d=');
  load(trD);
  ll_int(0);  { fieldwidth }
{  write_op(int_sy); {}
  end_write;

  source('end');
  discard_param(trD);
  exit_stackframe;
  plant(lab4);
  release_label(lab4);

  source('p(c)');
  load(trC);
  call_proc(trP);

  source('?');
  finalise_cgen;
end.

////////////////////////////////

