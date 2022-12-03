UNIT sagen;
interface   uses
  crt,
  reals,
  editdecl,
  errors,
  fsm,
  dlb,
  idtypes,
  opcodes,assemble;

const
      nillabl=0;
      stride =2;
      int_size = 1;
      long_int_size = 2;
      proc_size = 2;
      real_size = 4;
      pntr_on_stack_size = 8;   { times stride to get bytes }
      heappntr_size = 8;
      codegen_version_number = 3.1;

type
  labl= integer;

var stack_ptr:integer;
     segment,class: textline;
     asmfile:text;
     pidreg,line_number:namedesc;


procedure init_cgen(seg,class:textline);

procedure start_program;
procedure end_program(seg:textline);
procedure start_library;
procedure end_library(seg:textline);
procedure finalise_cgen;


{ ---------------------- STACK ADJUSTMENT ----------------------------- }

procedure epilogop(proctype:typerec);
procedure prologop(ll:integer);
procedure decsp(var t:typerec);
function enter_stackframe:integer;
procedure pushpreg;
procedure retractstack(newstack:integer);
procedure expand_to_pntr(t:typerec);
{---------------------- STORE ALLOCATION -------------------------------}

function declare_local(var t:idrec):integer;
procedure discard(var i:idrec;var t:typerec);
procedure discard_param(var i:idrec);
function declare_global(var t:idrec):integer;
function fixup_result(var i:idrec;paramsize:byte):integer;
procedure correct_param(var i:idrec;paramsize,resultsize:byte);
function fixup_param(var i:idrec):integer;
procedure floatop;
procedure float2op;
{-------------------------- CONTROL FLOW -------------------------------}
procedure plant(l:labl);
procedure release_label(l:labl);
procedure form_closure(l:labl);
procedure call_proc( i:namedesc);
procedure jumpfar(s:textline);
procedure aliencall(s:textline);
procedure callc(s:textline);
procedure bindop;
procedure start_external(procname:textline);
procedure plant_string(s:textline);
procedure end_external(procname:textline);
procedure jumpop(var l:labl);
procedure bjump(l:labl);
procedure jumpt(var l:labl);
procedure form_boolean;
procedure jumpf(var l:labl);
procedure forprepop;
function newlab:integer;
procedure forstepop(complex:boolean; l:labl);
procedure fortestop(complex:boolean;var l:labl);

{ ---------------------------------------------------------------- }
{      LOAD OPERATIONS                                             }
{ ---------------------------------------------------------------- }
procedure ll_int(i:integer);
procedure loadtrademark(n:namedesc);
{procedure pushfloatreg;}
procedure datasegprefix;
procedure ll_real(r:real);
procedure load(n: namedesc);
function ll_nil:integer;
procedure assignop( n:namedesc);
procedure load_addr(n:namedesc);
{ ---------------------------------------------------------------- }
{       OPERATIONS ON HEAP OBJECTS                                 }
{ ---------------------------------------------------------------- }
function sizeof(var t:typerec):integer;   { return size in strides }
procedure make_vector;
procedure iliffeop(levels:integer; var t:typerec);
procedure makepntrarray(lower,upper:namedesc);
procedure ll_string(s:textline);
procedure declarestructure( class:textline;
                            pntrs,reals,ints:integer;
                            n:namedesc);
procedure formstruct(n:namedesc);
procedure loadstruct(n:namedesc);
procedure subsass(class,field:namedesc;var t:typerec);
procedure subs(class,field:namedesc;var t:typerec);
procedure subsinit(var field:idrec);
procedure substrop;
procedure upbop;
procedure lwbop;
procedure subv(var t:typerec);
procedure subvass(var t:typerec);
procedure mcktab;
procedure inittab;
procedure tab_insert(var t:typerec);
procedure tab_lookup(var t:typerec);
{------------------------ DATA MANIPULATION -----------------------}
procedure binaryop(operation:lexeme;T:typerec);
procedure negop(t:typerec);
procedure notop;
{ ----------------------- INPUT OUTPUT ----------------------------}
procedure readop(s:lexeme;var t:typerec);
procedure writeop(var t:typerec);
procedure end_write;
procedure out_byte_op;
procedure newlineop;
{ ----------------------- Version Control -------------------------}
procedure codever;
IMPLEMENTATION
const

      callform = 'far';
      param_offset = 2;
      stacksize = 1000;
      pstacksize =1000;
      stdout = 1;

var
  next_label_num,
  psp,param_sp,discard_param_count:integer;
  code_str,comment_str:textline;
  opstack: array [1..100] of lexeme;
  opsp:1..100;
  endlab,past,stringcount,next_external:integer;


const
      dynalink:boolean = true;
      debugging:boolean = false;
procedure codever;
begin
     writeln('Macro code generator version ', codegen_version_number:2:3);
end;
{ ------------------------------------------------------------------ }
{       PRINTIDREC                                                   }
{------------------------------------------------------------------- }
procedure printidrec(var i:idrec);
var s:string[200];
begin
    with i do begin
         s:=ptype(typeinfo);
         writeln;
          write('ll =',lex_level,' offs =',offset,' id =',ord(identifier));
          write(' ',s);
     end;
end;
{ ------------------------------------------------------------------ }
{       PRINTSYMTAB                                                  }
{ ------------------------------------------------------------------ }
procedure printsymtab;
var i:integer;
begin
{     for i:=nametop-1 downto nulid do
         printidrec(scopelist [i] ^);
         writeln;}
end;
procedure Push(l:lexeme);
begin
     opstack[opsp]:=l; opsp:=opsp+1;
end;
function pop:lexeme;
begin
     opsp:=opsp-1;
     pop := opstack[opsp];
end;


function newlab:integer;
begin

     next_label_num:=next_label_num+1;
     if next_label_num >=(labmax-5) then error('too many labels');
     newlab:=next_label_num;
end;
function sizeof(var t:typerec):integer;   { return size in strides }
begin
      if (typecode(t )= 'p') then sizeof:=pntr_on_stack_size else
      case t.typeid of
      procedure_sy:sizeof:=proc_size;
      real_sy,creal_sy:sizeof:=real_size;
      int_sy,cint_sy,bool_sy,cbool_sy,file_sy,cfile_sy:sizeof:=int_size;
      else begin
        sizeof:=0;
      end;
    end;
end;
procedure dispsp;
begin
     if debugging then writeln('; sp=',stack_ptr);
end;
procedure incsp(var t:typerec);
var i:integer;
begin
     for i:= 1 to sizeof(t) do
     stack_ptr := stack_ptr -stride;
    dispsp;
end;
procedure decsp(var t:typerec);
var i:integer;
begin
     for i:=1 to sizeof(t) do
     stack_ptr := stack_ptr +stride;
     dispsp;
end;

procedure retractstack(newstack:integer);
begin
     if newstack<>stack_ptr then
        assem(add_sp,newstack-stack_ptr,0);
     stack_ptr:=newstack;
     dispsp;
end;

procedure drop(strides:integer);
begin
     retractstack(stack_ptr+strides*stride);
end;
procedure expand_to_pntr(t:typerec);
{ pads this out till it occupies same space as pntr }
begin
    drop(-(sizeof(pntr_type)-sizeof(t)));
end;
procedure pushax;
begin
     assem(push_ax,0,0);
     incsp(int_type);
end;
   procedure extrn(t:textline);
   begin
{$IFDEF obj}

        assem(' extrn ',t,':',callform);
{$ENDIF }
   end;
procedure decstr(var s:textline);
var i,c,c2:integer;
begin
    i:=1;
    while i<=length(s) do
    begin
        c:=ord(s[i]);
        if (length(s)-i)>2 then begin
        { pack two chars to a word }
           i:=i+1;
           c2:=ord(s[i])*256 + c;
           assem(dw,c2,c2);
        end
        else assem(db,c,c);
        i:=i+1;
    end;
end;
procedure callc(s:textline);
var i:integer;
begin
    if not dynalink then extrn(s);
    assem(push_bx,0,0);
    assem(int18,0,0);
    decstr(s);
    assem(db,0,0);

    assem(pop_bx,0,0);
end;

{------------------------------- assignment -----------------------}
procedure assignop( n:namedesc);
begin
    with scopelist[n]  ^ do
      begin
          decsp(typeinfo);
          if lex_level = lexical_level then { local }
                  begin
                  case typecode(typeinfo) of
                  'i':assem (localassi,offset,0);
                  'r':assem (localassr,offset,0);
                  'p':assem (localassp,offset,0);
                  else  assem (localassproc,offset,0);
                  end;
                  end else
          if lex_level = global_level then { global }
              begin
                  case typecode(typeinfo) of
                  'i':assem (globalassi,offset,0);
                  'r':assem (globalassr,offset,0);
                  'p':assem (globalassp,offset,0);
                  else  error('procedure assignment illegal');
                  end;
              end else
                  begin { other }
                  case typecode(typeinfo) of
                  'i':assem (assi,lex_level * - stride,offset);
                  'r':assem (assr,lex_level * - stride,offset);
                  'p':assem (assp,lex_level * - stride,offset);
                  else error('procedure assignment illegal');
                  end;
                  end;
        end;
end;

{ -------------------------------------------------------------- }
{            ARITHMETIC                                          }
{ -------------------------------------------------------------- }
procedure notop;
begin
  assem(not_op,0,0);
end;

procedure negop(t:typerec);
begin
  if typecode(t)='r' then callc('_negopr') else assem(neg_op,0,0);
end;
{----------------------------- arithmetic ---------------------------}
procedure binaryop(operation:lexeme;T:typerec);
  var s: textline;op:opcode;

  procedure operror;
  begin
       s:=psym(ord(operation));
       writeln(s);
       s:='illegal operator or bad type '+s;
       error(s);
  end;
  procedure callr(t:textline);{ call real arith subroutine }
  begin
    callc(t);
    retractstack(stack_ptr+(stride*sizeof(real_type)));
  end;
begin
  {s:=psym(operation);}
  if (operation = dplus_sy)and(t.dimensions>0) then begin
                   op:=concat_op;
                   decsp(string_type);
                   assem(op,0,0 );

       end
  else
  if (t.typeid=string_sy)or(t.typeid=cstring_sy)
  then begin

     case operation of
     dplus_sy:begin
                   op:=concat_op;
                   decsp(string_type);
                   assem(op,0,0 );
              end;
     lt_sy,
     le_sy,
     gt_sy,
     ge_sy,
     eq_sy,
     neq_sy:begin assem( cmpops,0,0); push(operation);
                  drop(2*pntr_on_stack_size);   { remove both operands }
                  assem(cmp_ax,0,0);
                  end;
     else operror;
     end;
  end else
  case t.typeid of
  structure_sy:begin

                    push(operation);
                    callc('_isop');
                    retractstaCK(STACK_PTR+(STRIDE*SIZEOF(PNTR_TYPE))
                                 +stride*2 *sizeof(int_type));

                    assem(cmp_ax,1,0);
               end;
  int_sy,cint_sy,file_sy,cfile_sy:  begin
     case operation of
     plus_sy:op:=plusop;
     minus_sy:op:=minusop;
     star_sy:op:=multop;
     div_sy: op:=divop;
     b_and_sy: op:=band;
     b_or_sy: op:= bor;
     shift_l_sy:op:=shiftl;
     shift_r_sy : op:=shiftr;
     rem_sy: op:=remop;
     lt_sy,
     le_sy,
     gt_sy,
     ge_sy,
     eq_sy,
     neq_sy:begin op:= cmpop; push(operation);
                  decsp(int_type);             { remove both operands }
                                               { leave result in flags }
                  end;
     else operror;
     end;
     assem(op,0,0   );
     decsp(int_type);
  end;
  real_sy,creal_sy:  begin
     case operation of
     plus_sy:callr('_plusr');
     minus_sy:callr('_minusr');
     star_sy:callr('_timesr');
     slash_sy: callr('_divr');
     lt_sy,
     le_sy,
     gt_sy,
     ge_sy,
     eq_sy,
     neq_sy:begin  push(operation);            { to be used in subs jumps }
                   callc('_compopr');
                   retractstack(stack_ptr+2*sizeof(real_type)*stride);
                   assem( cmp_ax,0,0);
                                               { leave result in flags }
                  end;
     else operror;
     end;
  end;
  else begin
       if operation in [eq_sy,neq_sy] then begin
           push(operation);
           assem(cmoppntr,0,0);
           drop(2*pntr_on_stack_size);
           assem(cmp_ax,0,0);
       end
       else operror;
       end;
  end;
end;

{ ---------------------------------------------------------------- }
{      LOAD OPERATIONS                                             }
{ ---------------------------------------------------------------- }
procedure ll_int(i:integer);
begin
  incsp(int_type);
  assem(llint ,i,0   )
end;
procedure loadtrademark(n:namedesc);
begin
     { load the address of the trademark record onto the stack }
     incsp(int_type);incsp(int_type);
     assem(pushfarpntr ,scopelist[n]^.offset,0);

end;

procedure float2op;
begin
     ll_int(0);ll_int(0);ll_int(0);
     callc('_float_lower');
end;
procedure floatop;
begin
     ll_int(0);ll_int(0);ll_int(0);
     callc('_float_top');
end;

          {
procedure pushfloatreg;
begin
    assem(pushrreg,0,0);
    incsp(real_type);
end;       }
procedure datasegprefix;
begin
     past:=newlab;stringcount:=newlab;
     assem(jump ,past,0);
     assem(plant_label,stringcount,0);
end;
procedure ll_real(r:real);
var f:ieee;i:integer;
begin
  incsp(real_type);
  datasegprefix;
  realtoieee(r,f);
  for i:=1 to 8 do assem( db ,f[i],f[i]);

  assem(plant_label,past,0);
  assem(llreal_op  ,stringcount,0)
end;



procedure load(n: namedesc);
begin
    with scopelist[n] ^ do
      begin
          incsp(typeinfo);

          if lex_level = global_level then { global }
              begin
                  case typecode(typeinfo) of
                  'i':assem (globali,offset,0);
                  'r':assem (globalr,offset,0);
                  'p':assem (globalp,offset,0);
                  'c':begin assem (globali,offset+2,0);
                            assem (globali,offset,0);
                            end;
                  end;
              end else
                  if lex_level = lexical_level then { local }
                  case typecode(typeinfo) of
                  'i':assem (locali,offset,0);
                  'r':assem (localr,offset,0);
                  'p':assem (localp,offset,0);
                  'c':begin assem (locali,offset+2,0);
                            assem (locali,offset,0);
                            end;
                  end else
                  case typecode(typeinfo) of
                  'i':assem (loadi,lex_level* -stride,offset);
                  'r':assem (loadr,lex_level* -stride,offset);
                  'p':assem (loadp,lex_level* -stride,offset);

                  'c':begin assem (loadi,lex_level* -stride,offset+2);
                            assem (loadi,lex_level* -stride,offset);
                            end;
                  end;
        end;
   end;

procedure load_addr(n: namedesc);

begin
    with scopelist[n] ^ do
      begin
          incsp(int_type); incsp(int_type); { addresses are double word }

          if lex_level = global_level then { global }
              begin
                    assem(globaladdr,offset,0);
              end else
                  if lex_level = lexical_level then { local }
                  begin
                    assem(localaddr,offset,0);
                  end else
                  begin { other }
                    assem(loadaddr,
                           ( lex_level)* -stride,offset);
                  end;
        end;
   end;

procedure pushpreg;
begin
     load(pidreg);
end;
function ll_nil:integer;
var i:integer;
begin
    for i:= 1 to pntr_on_stack_size do ll_int(0);
    ll_nil:=stack_ptr;
end;

{ -------------------------------------------------------------------- }
{         INPUT OPERATIONS                                             }
{ -------------------------------------------------------------------- }
procedure readop(s:lexeme;var t:typerec);
   procedure stringsetup;begin
                  t:=string_type;drop(int_size-sizeof(t));
                end;
begin
     case s of
     {-------------- read pntr ----------------------}
     readp_sy:begin
                   t:=pntr_type;drop(int_size-sizeof(t));
                   callc('_readp');
                   end;
     {--------------- STRING READS ------------------}
     read_sy:begin
                  stringsetup;
                  callc('_read');
             end;
     reads_sy:begin
                  stringsetup;
                  callc('_reads');
             end;
     read_a_line_sy:begin
                  stringsetup;
                  callc('_read_a_line');
             end;
     peek_sy:begin
                  stringsetup;
                  callc('_peek');
             end;
     {------------------ read REAL ---------------------}
     readr_sy:begin
                   t:=real_type;
                   drop(int_size-real_size);
                   callc('_readr');
              end;
     readb_sy:begin
                   t:=bool_type;
                   callc('_readb');
              end;
     eoi_sy:begin
                   t:=bool_type;
                   callc('_eoi');
              end;

     readi_sy:begin
                   t:=int_type;
                   callc('_readi');
              end;
     read_byte_sy:begin
                   t:=int_type;
                   callc('_read_byte');
              end;
     end;
end;

{ -------------------------------------------------------------------- }
{        OUTPUT OPERATIONS                                             }
{ -------------------------------------------------------------------- }

procedure out_byte_op;
begin
  assem(outbyte,0,0);
  decsp(int_type);decsp(int_type);
end;
procedure start_write;
begin
  ll_int(stdout);
end;

procedure writeop(var t:typerec);
var s:string[10];
begin
  if t.dimensions=0 then
  case t.typeid of
  int_sy,cint_sy:assem(writeint,0,0);
  real_sy,creal_sy:assem(writereal,0,0);
  string_sy,cstring_sy:assem(writestring,0,0);
  pntr_sy,cpntr_sy:assem(writepntr,0,0);
  bool_sy,cbool_sy:assem(writebool,0,0);
  else error('cant write')
  end
  else error('cant write');
end;

procedure end_write;
begin
  retractstack(stack_ptr +(stride * int_size)) ;                { remove file }
end;

{ ----------------------------------------------------------------- }
{      STACK TIDYING OPERATIONS                                     }
{ ----------------------------------------------------------------- }
procedure discard_param(var i:idrec);
begin
end;

procedure discard(var i:idrec;var t:typerec);
{ pop tos and assign to i, then make i top of stack }
var newsp,ownsize,dif:integer;tc :string[5];
begin
       tc:=typecode(t);
       ownsize:= stride* sizeof(i.typeinfo);
       newsp := i.offset + ownsize- (stride * sizeof(t));
       if tc<>'v' then
       case tc [1] of
       'i':   assem(localmovi,newsp,0);{ return result of block }
       'r':   assem(localmovr,newsp,0);
       'p':   assem(localmovp,newsp,0);
       end;                                        { adjust internal sp }
       retractstack(newsp);
end;
function enter_stackframe:integer;
begin
     param_sp:=6;
     enter_stackframe := 0;
end;
function fixup_param(var i:idrec):integer;
var size:byte;
begin
     size:=stride*sizeof(i.typeinfo);
     i.offset:=param_sp;
     param_sp := size + param_sp;
     fixup_param:=size;
end;
function fixup_result(var i:idrec;paramsize:byte):integer;
var size:byte;
begin
     size:=stride*sizeof(i.typeinfo);
     i.offset:=param_sp - size;
     if size>paramsize then i.offset:=i.offset+size-paramsize;
     param_sp := size + param_sp;
     fixup_result:=size;
end;
procedure correct_param(var i:idrec;paramsize,resultsize:byte);
begin
    if resultsize >paramsize then i.offset:=i.offset +resultsize-paramsize;
end;

{ ----------------------------------------------------------------- }
{             CONTROL FLOW INSTRUCTIONS                             }
{ ----------------------------------------------------------------- }

procedure plant(l:labl);
begin
  assem(plant_label,ord(l),   0)
end;

procedure form_closure(l:labl);
begin
     assem(formclos ,l,0);
     stack_ptr := stack_ptr - 4;
end;
procedure bindop;
begin
     callc('_bind');
     drop(3* pntr_on_stack_size - proc_size);
end;
procedure call_proc( i:namedesc);
begin
 with scopelist[i] ^ do begin
     if typeinfo.paramspace<typeinfo.resultspace then  { leave space for result }
        retractstack(stack_ptr+typeinfo.paramspace-typeinfo.resultspace);
     if name_type = static then begin
        assem(call_static ,offset,0)
     end else
     if lex_level = global_level then { global }
              begin
                    assem(call_glob ,offset,0);
              end else
                  if lex_level = lexical_level then { local }
                  begin
                    assem(call_loc ,offset,0);
                  end else
                  begin { other }
                  assem(call_,lex_level* -stride,offset);
                  end;
     if typeinfo.paramspace > typeinfo.resultspace then
     { forget params  they  have been discarded by the ret instruction }
     stack_ptr:=stack_ptr+typeinfo.paramspace-typeinfo.resultspace;
    end;
end;
procedure jumpfar(s:textline);
begin
    error('only allowed in compiling obj files');
end;
procedure aliencall(s:textline);
begin
     callc(s);
end;
procedure jumpop(var l:labl);
begin
  if l <=0 then l:=newlab;
  assem(jump ,ord(l),0   )
end;
procedure bjump(l:labl);
begin
     assem(jump ,ord(l),0);
end;
function writecond:opcode;
  var s:opcode;
begin
  s:=jl;
  case pop of
  lt_sy:s:=jl;
  le_sy:s:=jle;
  gt_sy:s:=jg;
  ge_sy:s:=jge;
  eq_sy,is_sy:s:=je;
  neq_sy,isnt_sy: s:=jnz;
  else error('Unkown comparison operator ');
  end;
  writecond:=s
 end;


procedure jumpt(var l:labl);
begin
  assem(writecond,ord(l),0   )
end;
procedure form_boolean;
{ there is a condition code set, convert it to a boolean }
var l1,l2:labl;
begin
     l1:=newlab;l2:=newlab;
     jumpt(l1);ll_int(0);jumpop(l2);decsp(int_type);
     plant(l1);ll_int(1);plant(l2);
end;

procedure jumpf(var l:labl);
var skip:labl;
begin
  skip:=newlab;
  jumpt(skip);jumpop(l);plant(skip);
end;
procedure forprepop;
begin
     assem(forprep,0,0);
end;
procedure forstepop(complex:boolean; l:labl);
begin
  if not complex then assem(minforstep,ord(l),0)else
  assem(forstep ,ord(l) ,0  );
  decsp(int_type);decsp(int_type);if complex then decsp(int_type);
end;

procedure fortestop(complex:boolean;var l:labl);
begin
  l:=newlab;
  if not complex then assem(minfortest,ord(l),0) else
  assem(fortest ,ord(l)  ,0 )
end;


{ ---------------------------------------------------------------- }
{       OPERATIONS ON HEAP OBJECTS                                 }
{ ---------------------------------------------------------------- }
procedure make_vector;
begin
  assem(makevec_op,0,0);
end;

procedure mcktab;
begin
     callc('_mcktab');
end;
procedure inittab;
begin
    callc('_inittab');
    drop(2*sizeof(pntr_type));   { discard all but the table itself }
end;

procedure iliffeop(levels:integer; var t:typerec);
var s:string[7];
    sz:integer;
begin
  sz:=sizeof(t);
  ll_int(sz*stride);
  ll_int(levels);
  case sz of
  int_size:callc('_iliffeint');
  real_size:callc('_iliffereal');
  else callc('_iliffepntr');
  end;
end;
procedure makepntrarray(lower,upper:namedesc);
var t:typerec;
    j:integer;
begin
     t:=pntr_type;
     load(lower);load(upper);j:=ll_nil;iliffeop(1,t);
end;
procedure plant_string(s:textline);
var i :integer;
begin

     assem( dw ,length(s),0);decstr(s);

end;
procedure ll_string(s:textline);
begin
     datasegprefix;
     plant_string(s); plant(past);
     assem(pushfarpntr ,stringcount,0);
     callc('_llstring');
     assem( add_sp,4,0); { discard the address of string }
     pushpreg;
end;
procedure declarestructure( class:textline;
                            pntrs,reals,ints:integer;
                            n:namedesc);
{ this writes information into the datasegment to desscribe a structure class }
{ this information is of the form
       <pntrs>     a 8 bit field holding number of pntrs in class
       <reals>     a 8 bit field holding number of reals in class
       <ints>                             number of integers in class
       <len>       length of the name of the class
       <c>*n       the name itself padded out to at leat 13 bytes
}
var i :integer;
begin
   with scopelist[n]^ do begin

     datasegprefix;
     offset:=stringcount;
     assem(plant_label,offset,0);
     { plant a nil pid that will be initialised at run time to point to }
     { the sdesc for the structure class                                }
     assem(db ,pntrs,pntrs) ;
      assem(db,reals,reals);
      assem(db,ints,ints);
     assem(db  ,length(class),length(class));
     for i:=1 to length (class) do     assem(db ,ord(class[i]),ord(class[i]));
     if length(class)<14 then
        for i:=length(class) to  14 do assem(db,0,0);
     assem(plant_label ,past,0);
     assem(pushfarpntr ,offset,0);
     callc( '_mksdesc');
     assem(add_sp,4,0);
   end;
end;
procedure formstruct(n:namedesc);
{ this is used to create an instance of a structure }
begin
     retractstack(stack_ptr - pntr_on_stack_size*stride); { space for result }
     assem(pushfarpntr ,scopelist[n]^.offset,0);
     callc( '_formstruct');
     assem(add_sp,2*int_size*stride,0);
end;
procedure loadstruct(n:namedesc);
begin
     assem(pushfarpntr ,scopelist[n]^.offset,0);
     incsp(int_type);incsp(int_type);
end;

procedure subsass(class,field:namedesc;var t:typerec);
{  pntr, t ->void  }

begin
  with scopelist[field]^do begin
  assem(mov_ax_sp,0,0);
  assem(add_ax,sizeof(typeinfo)*stride,0);
  assem(push_ss,0,0);
  assem(push_ax,0,0);            { push address of pid on stack }
  assem(sspushfarpntr ,scopelist[class]^.offset,0);
                                         { push address of trademark }
  callc ('_validstruct'); { address of struct in dx:ax }
  assem(add_sp,8,0);          { remove params &pntr}

  assem(mov_di_ax,0,0);          { get offset }
       case typecode(typeinfo) of
       'i': begin
             assem(mov_es_dx,0,0);          { get segment }
             assem(pop_es_di,offset,0);     { pop es:[di+offset]}
             end;
       'r':  begin
             assem(add_di,offset,0);
             assem(heappopreal,0,0);
             end;
        'p': begin
             assem(add_di,offset,0);
             assem(heappopp,0,0);
             end;
        end;
        decsp(typeinfo);
        t:=typeinfo;
        t.field:=false;
  end;
  drop(pntr_on_stack_size);    { throw away the pntr }
end;

procedure subs(class,field:namedesc;var t:typerec);
{  pntr, t ->pntr  }
begin
  assem(mov_ax_sp,0,0);
  assem(push_ss,0,0);
  assem(push_ax,0,0);            { push address of pid on stack }
  incsp(int_type);  incsp(int_type);
  assem(sspushfarpntr ,scopelist[class]^.offset,0);
                                         { push address of trademark }
  incsp(int_type);  incsp(int_type);
  callc ('_validstruct'); { address of struct in ax }
  drop(4*int_size+pntr_on_stack_size );   { remove params & pid address}
  assem(mov_si_ax,0,0);          { get offset }
  with scopelist[field]^do begin
       t:=typeinfo;
       t.field:=false;
       case typecode(t) of
       'i': begin
             assem(mov_es_dx,0,0);          { get segment }
             assem(push_es_si,offset,0);  {push es:[si+offset]}
             end;
       'r':  begin
             assem(add_si,offset,0);
             assem(heappushreal,0,0);
             end;
        'p': begin
             assem(add_si,offset,0);
             assem(heappushp,0,0);
             end;
        end;
        incsp(t);
   end;
end;

procedure subsinit(var field:idrec);
{  pntr ->t   }
begin
  with field do begin
  assem(mov_ax_sp,0,0);
  assem(add_ax,sizeof(typeinfo)*stride,0);
  assem(push_ss,0,0);
  assem(push_ax,0,0);            { push address of pid on stack }
  callc ('_wherestruct'); { address of struct in ax }
  assem(add_sp,4,0);          { remove  param}
  assem(mov_di_ax,0,0);          { get offset }
       case typecode(typeinfo) of
       'i': begin
              assem(mov_es_dx,0,0);          { get segment }
              assem(pop_es_di,offset,0);   {pop es:[di+offset]}
             end;
       'r':  begin
             assem(add_di,offset,0);
             assem(heappopreal,0,0);
             end;
        'p': begin
             assem(add_di,offset,0);
             assem(heappopp,0,0);
             end;
        end;
        decsp(typeinfo);
    end;
end;

procedure substrop;
begin
  assem(substr_op,0,0);
  drop(2*int_size);
end;

procedure upbop;
begin
     callc('_upbop');
     drop((pntr_on_stack_size));
     pushax;
end;
procedure lwbop;
begin
     callc('_lwbop');
     drop((pntr_on_stack_size));
     pushax
end;
procedure subv(var t:typerec);
var s:string[6];target:integer;
begin
  t.dimensions:=t.dimensions - 1;
  s:=typecode(t);
  case s [1] of
  'i':assem(subv_opi,0,0);
  'p':assem(subv_opp,0,0);
  'r':assem(subv_opr,0,0);
  end;
  target := stack_ptr + (int_size +pntr_on_stack_size - sizeof(t))*stride;
  retractstack( target );

end;

procedure subvass(var t:typerec);
var s:string[6];
begin
  s:=typecode(t);
  case s [1] of
  'i':assem(subvass_opi,0,0);
  'p':assem(subvass_opp,0,0);
  'r':assem(subvass_opr,0,0);
  end;
end;
procedure tab_insert(var t:typerec);
var s:string[6];
begin
     s:=typecode(t);
     callc('_tab_ins'+s);
     drop(sizeof(t)+sizeof(pntr_type)+sizeof(string_type));
end;

procedure tab_lookup(var t:typerec);
var s:string[6];target:integer;
begin
  s:=typecode(t);
  callc('_s_lookup');
  drop( sizeof(pntr_type)+sizeof(string_type)-sizeof(t) );

end;

function declare_global(var t:idrec):integer;
begin
    declare_global := stack_ptr;
end;

function declare_local(var t:idrec):integer;
begin
    declare_local := stack_ptr;
end;


procedure release_label(l:integer);
begin
{     assem('release_label ',l);}
end;
{ external procedures written in c return integer results in ax }
procedure pushres(t:typerec);
begin
    case sizeof(t) of
    int_size:pushax;
    real_size:error('cant push floatreg');
    else pushpreg;
    end;
end;
procedure declareexternals;
begin  extrn(' _writei');
       extrn(' _writer');
       extrn(' _writes');
       extrn(' _writep');
       extrn(' _lwbop') ;
       extrn(' _upbop');
       extrn(' _makevec');
       extrn(' _iliffeint');
       extrn(' _iliffereal');
       extrn(' _iliffepntr');
       extrn(' _llstring');
       extrn(' _subvassib');
       extrn(' _subvassr');
       extrn(' _subvassp');
       extrn(' _subvib');
       extrn(' _subvr');
       extrn(' _concat');
       extrn(' _divr');
       extrn(' _timesr');
       extrn(' _minusr');
       extrn(' _plusr');
       extrn(' _float_top');
       extrn(' _float_lower');
       extrn(' _compopr');
       extrn(' _subvp');
       extrn(' _substrop');
       extrn(' _compstr');
       extrn(' _comppntr');
       extrn(' _mksdesc');
       extrn(' _formstruct');
       extrn(' _validstruct');
       extrn(' _wherestruct');
       extrn(' _trace');
end;
procedure newlineop;
var i:integer;
begin
     i:=the_line;
     ll_int (i);
     assignop(line_number);
     if (i mod 10)=0 then
     if not(listing in options ) then begin
        if batch in options then write('.') else begin
           gotoxy(10,1);write(i:6);
        end;
     end;
     if trace then callc('_trace');
end;
procedure init_cgen(seg,class:textline);
begin
  initassem;
  next_label_num:=1;
  next_external:=newlab;
  stack_ptr:=0;psp:=0; opsp:=1;stringcount:=1;
{  assem(' include salib.mac');
  if not dynalink then begin
     assem(seg,' segment byte public ''',class,''' ');
     assem(' assume cs:',seg );
  end;}
end;
procedure start_program;
var i :integer;
begin
  i:=newlab;
  jumpop(i);plant(i);
  assem(   shrink,0,0);
  assem(   mov_bx_sp,0,0);
  assem(  mov_bp_sp,0,0);
  endlab:=newlab;
  assem(  cslea_ax,endlab,0);
  assem(  push_ax,0,0);
  callc('_defcseg');
  assem(  pop_ax,0,0);

end;
procedure start_library;
begin
    jumpop(next_external);
end;
procedure start_external(procname:textline);
begin
{$IFNDEF obj}
     plant(next_external);
     next_external:=newlab;
     jumpop(next_external);
     plant_string(procname);

{$ENDIF}
{$IFDEF obj}
     assem(procname,' proc far ');
     assem( 'public ',procname);
{$ENDIF}
end;
procedure end_external(procname:textline);
begin
{$IFDEF obj}
     assem(procname,'  endp');
{$ENDIF}
end;

procedure end_program(seg:textline);
begin
   callc('_terminate');
   assem( mov_ax,$4c00,0);
   assem( int21, 0,0);
   plant(endlab);
end;

procedure end_library(seg:textline);
begin
{$IFNDEF obj}
   plant(next_external);
   assem( mov_ax,$4c00,0);
   assem( int21, 0,0);
   assem(db,0,0);

{$ENDIF}
{$IFDEF obj}
 assem(seg,' ends');
{$ENDIF}
end;
procedure finalise_cgen;
begin
{$IFDEF obj}
  assem(' end');

{$ENDIF}

end;



procedure prologop(ll:integer);
begin
  assem(prolog86 ,ll,0);stack_ptr:=stack_ptr-ll*stride ;

  while ll >1 do begin
    assem(pushdisp,0,0); { push an element of display for each lex level }
    ll := pred(ll);
  end;
  if ll >0 then assem(push_bp,0,0);
end;

procedure epilogop(proctype:typerec);
var discard:integer;
begin
  with proctype do begin

          if params = nil then discard:=0
          else
          if resultspace>paramspace then discard:=0
          else discard := paramspace - resultspace;
  end;
  assem(epilog ,discard,0);
end;

begin
      trace:=false;

      next_label_num:=1;
end.