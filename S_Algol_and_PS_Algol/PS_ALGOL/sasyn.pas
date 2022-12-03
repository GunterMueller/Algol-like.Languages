{$r-
   SALGOL  SYNTAX ANALYSIS

----------------------------------------------------------------
Module      : SAsyn
Used in     : Compiler toolbox
Uses        : dlb.cmp, sasym.cmp
Author      : W P Cockshott
Date        : 15 Oct 1986
Version     : 2
Function    : To parse S-algol programs
Changes     : derived from file sasyn.cmp on 9-2-89
Copyright (C) WP Cockshott & P Balch
----------------------------------------------------------------}
{$s+}
UNIT SASYN;
INTERFACE USES
  editdecl,
  errors,fsm,
  reader,
  dlb,
  IDTYPES,
{$ifdef assembler}
cgendum,
{$endif }
{$ifndef assembler }
  sagen,
{$endif }
  symtab;

{
  The syntax analyser is a single pass top down recursive analyser.

  It has one procedure for each production rule in the grammer of
  the language. The language chosen in this example is S-algol.
  For a fuller description of the techniques used here you should
  refer to chapter 6 of "Recursive Descent Compiling" by A.J.T Davie
  and Ron Morrison.

  The syntax analyser calls 3 other modules:
      -      the lexical analyser
      -      the symbol table package
      -      the code generator

  The interface to all of these procedures is as specified in the
  book by Davie and Morrison, though the implementations of these
  modules differs from that in the book.


}

PROCEDURE PROG;
PROCEDURE SYNVER;
IMPLEMENTATION
procedure synver;
begin
     writeln ('P-salgol Syntax analyser for IBM AT version 2.1');

end;

procedure prog;

const syndebug = false;
      maxrule =350;
{ -------------------------------------------------------------- }
{ types for run time syntactic extensions                        }
{ ---------------------------------------------------------------}
type insertions= (clauserule,norule,expressionrule,exp1rule,exp2rule,
                  exp3rule,exp4rule,exp5rule,subscriptrule);
     matchtype = (none,partial,total);
     rulerange = 0..maxrule;
     ruletype  = (null,binding,alternative,compulsory,subrule,semantics);
     rule      = record
                   tail:rulerange;
                   prodtype:typerec;
                   case sort      :ruletype of
                   null,binding,
                   alternative    :(head:rulerange);
                   compulsory     :(mustbesym:lextoken);
                   subrule        :(proc:insertions;);
                   semantics      :(id :lextoken);

                 end;

var
     rules     : array [rulerange]of rule;
     nullrule,
     toprule   : rulerange;
     insertion : array[insertions] of record
                   rulestart: rulerange;
                   name     : lexeme ;
                 end;
     function nextrule:rulerange;
     begin
        toprule:=succ(toprule);
        nextrule:=pred(toprule);
        if toprule=(maxrule-3)then error('grammer too big');
     end;

{ -------------------------------------------------------------- }
{ Define Insertion Points                                        }
{        These are the grammer rules that can be extended        }
{ -------------------------------------------------------------- }
procedure defineinsertions;
const rulenames:array [insertions] of lexeme=
      ( clause_sy,UNDEFINED,expression_sy,exp1_sy,exp2_sy,exp3_sy,
        exp4_sy,exp5_sy,subscript_sy);
var
    ins :insertions;
    r:rulerange;
begin

    nullrule:=nextrule;
    rules[nullrule].sort:=null;
    for r:=succ(nullrule)to  maxrule do
      with rules[r] do begin
         prodtype:=void;sort:=null;    mustbesym:=0;tail:=nullrule;
      end;
    for ins:=clauserule to subscriptrule do
    with insertion[ins] do begin
      rulestart:=nullrule;
      name:=rulenames[ins];
    end;

end;

{ -------------------------------------------------------------- }
{ Find insertion                                                 }
{ -------------------------------------------------------------- }
function findinsertion(l:lexeme):insertions;
var i,j:insertions;
begin
      j:=norule;
      for i:=clauserule to subscriptrule do
      if insertion[i].name = l then j:=i;
      findinsertion:=j;
end;
{ -------------------------------------------------------------- }
{          FORWARD DECLARATIONS FOR RECURSIVE PRODUCTIONS        }
{ -------------------------------------------------------------- }
PROCEDURE     SEQUENCE(VAR t:TYPEREC);   FORWARD;
PROCEDURE     CLAUSE(VAR t:TYPEREC);     FORWARD;
PROCEDURE     TYPE1(VAR t:TYPEREC);                 FORWARD;
PROCEDURE     TYPE0(VAR t:TYPEREC);                 FORWARD;
PROCEDURE     IF_CLAUSE(var t:typerec);             FORWARD;
PROCEDURE     WHILE_CLAUSE;          FORWARD;
FUNCTION     EXTENSIONS(Var t:typerec;r:rulerange;
                            alreadyhave:boolean):matchtype; FORWARD;
{ -------------------------------------------------------------- }
{         SKIP NEWLINES                                          }
{ -------------------------------------------------------------- }
procedure skipnl;begin if have(newline_sy) then skipnl end;
{ -------------------------------------------------------------- }
{         MAIN PARSING PROCEDURES ONE PER GRAMMER RULE           }
{ -------------------------------------------------------------- }
procedure debug( s:textline);
begin if trace then begin writeln(listfile,s) end; end;
procedure conwritetype(var t:typerec); var s:textline;
begin  if trace then begin s:=ptype(t);debug(s);  end;end;
{ -------------------------------------------------------------- }
{            BOOLIFY                                             }
{ -------------------------------------------------------------- }
PROCEDURE BOOLIFY( var t:typerec);
{ converts a condition code into a boolean on the stack }
begin
     if eq(t,condition_type) then begin
        form_boolean;
        t:=bool_type;
     end;
end;

PROCEDURE CONDIFY(var t:typerec);
begin
     if not eq(t,condition_type) then begin
        match(t,bool_type);
        ll_int(1);
        binaryop(eq_sy,int_type);
        t:=condition_type;
        end;
end;

{ -------------------------------------------------------------- }
function newid(id:lextoken; t:typerec):namedesc;
var i:idref;
begin
 if lexical_level = global_level then newid:=newname(id,t,global,i)
     else newid:=newname(id,t,local,i);
end;
{----------------------- STD_DECLS-------------------------------}
var s_i,s_o,i_w,s_w,r_w,null_file:namedesc;
procedure std_decls;
var I:integer;scope :namedesc;
{ delare standard identifiers }
begin
     ll_int(0);line_number:=newid(tokenof('line.number'),INT_type);
     i:=ll_nil;pidreg:=newid(tokenof('pid_reg'),PNTR_type);
     i:=stack_ptr;
     load_addr(line_number);
     load_addr(pidreg);
     callc('_defpidreg');
     retractstack(i);

     ll_int(0);s_i:=newid(tokenof('s.i'),FILE_type);
     ll_int(1);s_o:=newid(tokenof('s.o'),FILE_type);
     ll_int(-1);null_file:=newid(tokenof('nullfile'),File_type);
     ll_int(12);i_w:=newid(tokenof('i.w'),INT_type);
     ll_int(2);s_w:=newid(tokenof('s.w'),INT_type);
     ll_int(14);r_w:=newid(tokenof('r.w'),INT_type);
end;
{ -------------------------------------------------------------- }
{            MAKEVECTOR                                          }
{            This parses the syntax
             vector <lower>::<upper>[,<lower>::<upper>] of init
             it generates the same code as if the above is a shorthand
             for :
             begin
                  let lower = <lower>
                  let upper = <upper>
                  let a=array(lower,upper-lower+1,sizeof(pntr))
                  for i= lower to upper do a(i):=
                  begin
                       let lower=<lower2>
                       let upper=<upper2>
                       let a=array(lower,upper-lower+1,sizeof(init)
                       for i=lower to upper do a(i):=init
                       a
                  end

                  a
              end
              dim1()

{ -------------------------------------------------------------- }

PROCEDURE MAKEVECTOR(VAR T:TYPEREC);
    function inner(var t:typerec):integer;

    var t1,t2:typerec;
        l1,l2:labl;
        oldscope,forscope,lower,upper,A,I:namedesc;
        oldsp,depth:integer;
    begin
     enterscope(oldscope);

              clause(t1); match(t1,int_type);lower:=newid(1,t1);
              mustbe(dcolon_sy);
              clause(t2); match(t2,int_type);upper:=newid(2,t1);
              if  have(comma_sy) then
              begin
                   l1:=newlab;l2:=newlab;
                   t:=pntr_type;oldsp:=stack_ptr;
                   makepntrarray(lower,upper);{ result in preg }
                   retractstack(oldsp);
                   pushpreg;  { pntr to vector on top of stack }
                   A:=newid(3,t);
                   {------------- Begin code of forloop ---------------}
                   enterscope (forscope);
                   load(lower);I:=newid(4,int_type);
                   load(upper);
                   forprepop;
                   plant(l1);fortestop(false,l2);
                   oldsp:=stack_ptr;
                   load(A);load(I);
                   depth:=inner(t)+1;     { generate inner vector }
                   subvass(t);
                   retractstack(oldsp);   { remove temps of assignment }
                   forstepop(false,l1);plant(l2);
                   exitblock(forscope,VOID);
                   {--------------- End code of forloop ---------------}
                   load(A);               { result on stack }
              end
              else
              begin

                  mustbe(of_sy);
                  oldsp:=stack_ptr;
                  clause(t);
                  iliffeop(1,t);
                  retractstack(oldsp);  { just leave the lower and upper bounds}
                  pushpreg;
                  depth:=1+t.dimensions;
              end;
              inner:=depth;
              t.dimensions:=depth;
     exitblock(oldscope,t);
     { this removes all temp variables from the stack }
    end;
begin
     next_symbol;
     t.dimensions:=inner(t);
end;

{------------------------- STD_PROC ---------------------------------}
procedure std_proc(var t:typerec);
var sym:lexeme;
begin
     sym:=lexsymbol;
     case lexsymbol of
     read_sy,readi_sy,readr_sy,readb_sy,
     reads_sy,peek_sy,read_a_line_sy,readp_sy,
     read_byte_sy,eoi_sy:begin
                              next_symbol;
                              mustbe(brace_sy);
                              if not have(rbrace_sy) then begin
                                 clause(t);match(t,file_type);
                                 mustbe(rbrace_sy);
                              end else
                              begin
                                   load(s_i);
                              end;
                              readop(sym,t);
                         end;
     length_sy,lwb_sy,upb_sy :begin
              next_symbol;
              mustbe(brace_sy);
              clause(t);
              mustbe(rbrace_sy);
              if t.dimensions =0 then
                 if t.typeid <> string_sy then
                    error('vector expected');
              t:=int_type;
              if sym = lwb_sy then lwbop else upbop;
           end;
   else error('standard procedure expected ');
   end;
end;
{ -------------------------------------------------------------- }
{     EXP5
      this parses the syntax rule
           <exp5>::= <name >|
                     <literal>|
                     (<clause>)|
                     <cur> <sequence> <ley>|
                     begin <sequence> end |
                     <name> (<clause><bar><clause>)|
                     @<clause> of <type1><bra><clauselist<ket>|
                     vector<bounds> of <clause>

      it takes one var parameter that is used to return the
      type of the value yielded by the expression
}
{ -------------------------------------------------------------- }
procedure exp5std(var t:typerec);
var t1,t2,t3,proctype:typerec;
    os,elems, pntr_pos,expstartlev:integer;
    vec_lev:byte;
    p:paramref;
    thevar,oldscope:namedesc;
    pendingop:(nothing,subvec,substruct,subtab,simplevar,procval);
    sym,procsym:lextoken;
    i:integer;
    m:matchtype;
function checkid:namedesc;
         var n:namedesc;
          begin
               n:= lookup(sym)   ;
               if n= nulid then error('Undeclared variable <'+psym(sym)+'>');
               checkid := n
          end;
function fieldid:namedesc;
         var n:namedesc;t:typerec;
         begin
              n:=checkid;
              typeof(sym,t);
              if not t.field then error('field expected');
              fieldid:=n;
         end;
procedure withdraw; { return stack to starting point }
          begin retractstack(expstartlev); end;
procedure SUBSCRIPT(var t:typerec);
          {
     HANDLE SUBSCRIPTION OPERATIONS
     ------------------------------
  note that if we have a bracket after an expression we must delay
  the subscript operation if any until we have checked if the expression
  is followed by an assignment.
  if it is, then we generate a subscript assign else we generate a
  dereference.
  The variable "pendingop" holds whether or not there is a subscription
  pending or not
 }
          procedure svec; { subscript a vector }
          begin subv(t); end;
          procedure sstruct; { subscript a structure }
          begin subs(classof(sym),fieldid,t); end;
          procedure stab; { subscript a table }
          begin tablecontents(t,t);tab_lookup(t); end;
          var rep:typerec;
 begin
    p:=t.params;
    proctype:=t;
    conwritetype(t);
    if pendingop = simplevar then
    begin
         thevar:=checkid;
         load(thevar)
    end;
    pendingop:=nothing;
      if t.typeid = procedure_sy then
      begin
         { create space for result }
      end;
      repeat
        representation(t,rep);
        if (t.typeid=pntr_sy) and (rep.dimensions = 0 )then begin
           { subscript a pointer }
           sym:=symbol;
           mustbe(identifier);
           if lexsymbol = comma_sy then    sstruct
           else pendingop:=substruct;
           p:=nil;
        end else begin
           clause(t1);
           if rep.dimensions>0 then begin
              { ARRAY }
              match(t1,int_type);
              if have(bar_sy) then begin
                 { slice operation }
                  clause(t2);match(t2,int_type);
                  substrop;
              end else  Begin
                if t.dimensions <=0 then { only array types can be indexed }
                error('not a vector');
                if lexsymbol= comma_sy then svec
                else pendingop:=subvec
              end
           end else
           if t.typeid=dollar_sy then   begin
              { TABLE }
              match(t1,string_type);
              if lexsymbol= comma_sy then stab
              else pendingop:=subtab
           end else
           if have(bar_sy) then begin
              { STRING }
              clause(t2);
              match(t1,int_type);
              match(t1,t2);
              match(t,string_type);
              substrop;
           end else  if proctype.typeid = procedure_sy then
           begin
               if p = nil then match(void,t1)
               else begin
                    boolify(t1);
                    match(p^.paramname^.typeinfo,t1);
                    p:=p^.next;
               end
           end else error(' subscript not allowed here ');
        end;
      until (not errorfree) or (not have(comma_sy) );
      mustbe(RBRACE_sy);
      if proctype.typeid =  procedure_sy then
      begin
           { the identifier in the expression was a procedure }
           call_proc(checkid);
           pendingop:=nothing;
           t:=proctype.result^.typeinfo;
           conwritetype(t);
      end;
      if lexsymbol <> assign_sy then
            case pendingop of
                 substruct : sstruct;
                 subvec    : svec;
                 subtab    : stab;
            end;
      if have(brace_sy) then subscript(t);
 end;






begin
   debug(' exp5 '); pendingop:=nothing;
   skipnl;
   expstartlev:=stack_ptr;

   case lexsymbol of
   end_sy,comma_sy,
   rbrace_sy,
   ley_sy,
   question_sy : begin end; (* these are terminators *)
   identifier  : begin
                      typeof(symbol,t); t3:=t;
                      sym:=symbol;

                      next_symbol;
                      if t.typeid<>structure_sy then begin
                         { it is an L value or R value or a procedure }
                         if t.typeid<>procedure_sy then pendingop:=simplevar
                         else pendingop:=procval;
                       end else
                       begin
                            thevar:=checkid;
                            p:=t.params;
                            if have(brace_sy) then begin
                            { this is a structure literal }
                               formstruct(thevar);
                               repeat
                                  clause(t1);
                                  t1.field:=true;
                                  match(p^.paramname^.typeinfo,t1);
                                  subsinit(p^.paramname^);
                                  p:=p^.next;
                               until not have(comma_sy);
                               mustbe(rbrace_sy);
                               t:=pntr_type;
                            end else loadtrademark(thevar);
                       end;
                 end;
   cur_SY      : begin next_symbol;
                       enterscope(oldscope);
                       sequence(t);
                       exitblock(oldscope,t);
                       mustbe(LEY_SY);
                       end;
   brace_sy    : begin next_symbol;clause(t);mustbe(RBRACE_sy);end;
   BEGIN_SY    : begin next_symbol;
                       enterscope(oldscope);
                       sequence(t);
                       exitblock(oldscope,t);
                       mustbe(END_SY)
                       end;
   INT_LIT     :begin t:=int_type;ll_int(the_integer);next_symbol end;
   REAL_LIT    :begin t:=real_type;ll_real(the_real);next_symbol end;
   STRING_LIT  :begin t:=string_type;ll_string(the_string);next_symbol end;
   TRUE_SY: begin t:=bool_type;next_symbol; ll_int(1) end;
   FALSE_SY: begin t:=bool_type;next_symbol; ll_int(0) end;
   nil_sy      :begin next_symbol;t:=pntr_type;i:=ll_nil; end;
   vector_sy   :makevector(t);
   at_sy,bra_sy    :begin
                     if have(at_sy) then begin
                     clause(t);match(int_type,t);
                     mustbe(of_sy);
                     type1(t);
                     end else begin
                         ll_int(1);t:=void
                     end;
                     mustbe(bra_sy);
                     elems:=0;
                     repeat
                           clause (t1);
                           if eq(t,void) then t:=t1 else match(t,t1) ;
                           elems:=elems+1;
                     until not have(comma_sy);
                     mustbe(ket_sy); t.dimensions:=t.dimensions+1;
                     ll_int(sizeof(t1)*stride); { push size of array elems in bytes }
                     ll_int(elems);
                     make_vector;
                     withdraw;
                     pushpreg;
                end;
     else std_proc(t);
     end;
 if have(brace_sy) then subscript(t);
 if not have(assign_sy) then
        case pendingop of
                  procval,
                 simplevar : begin
                             thevar:=checkid;
                             load(thevar)
                            end;
        end
 else
 {
      HANDLE ASSIGNEMENT OPERATIONS
      -----------------------------
 }
   begin
      if IsVariable(t) then
        begin
            if t.typeid=procedure_sy then
             if not inblock(checkid) then
               error('no procedure assignments out of blocks');
            { evaluate the RHS }
            clause (t1);
            boolify(t1);

            case pendingop of
                 substruct : subsass(classof(sym),fieldid,t);
                 subvec    : begin
                                  t.dimensions:=pred(t.dimensions);
                                  subvass(t1);
                             end;
                 subtab    : begin
                                 tab_insert(t1);
                                 tablecontents(t,t);
                             end;
                 else assignop(checkid);
            end;
            match(t,t1);
            withdraw;
            t:=VOID   ;
        end
      else error({102,}'not a variable');
   end ;
end;
procedure exp5 (var t:typerec);
var
    m:matchtype;
begin

     m:=extensions(t,insertion[exp5rule].rulestart,false);
     if m= none then exp5std(t);
end;

{ -------------------------------------------------------------- }
{     EXP4
          this parses the syntax rules
               <exp4>::=[<addop>]<exp5>[<multop><exp5>]*
               <multop>::=<star>|div|rem|/|++
               <addop>::= +|-
}
{ -------------------------------------------------------------- }
procedure exp4 (var t:typerec);
var continue:boolean; t1:typerec;   sym:lexeme;
    adop:(plus,minus,noadop);
    m:matchtype;
begin
     debug(' exp4 ');
     adop:=noadop;
     if have(plus_sy) then adop:=plus;
     if have(minus_sy) then adop:=minus;

     m:=extensions(t,insertion[exp4rule].rulestart,false);
     if m= none then exp5(t);
     if m<>total then begin
     sym:=lexsymbol;
     if adop= minus then negop(t);
     continue:=true;
     if adop<>noadop then if not eq(t,int_type) then  match(t,real_type);
     repeat
           sym:=lexsymbol;
           case lexsymbol of
           star_sy        : begin
                                 next_symbol;exp5(t1);
                                 balance(t,t1);binaryop(sym,t)
                                 end  ;
           slash_sy       : begin
                                 next_symbol;
                                 balance(real_type,t);
                                 exp5(t1);
                                 balance(real_type,t1);
                                 binaryop(sym,t)
                                 end   ;
           div_sy,rem_sy  : begin
                                 next_symbol;
                                 match(t,int_type);
                                 exp5(t1); match(int_type,t1);
                                 binaryop(sym,t)
                                 end    ;
           dplus_sy       : begin
                                 next_symbol;
                                 if t.dimensions =0 then  match(t,string_type);
                                 exp5(t1);
                                 match(t,t1);
                                 binaryop(sym,t)
                                 end     ;
           else continue:=false;
           end;
     until not continue ;
     end;
end;
{ -------------------------------------------------------------- }
{     EXP3
         this parses the rule
              <exp3> ::= <exp4>[<addop><exp4>]*
}
{ -------------------------------------------------------------- }
procedure exp3 (var t:typerec);
var t1:typerec; sym:lexeme; m:matchtype;
begin
     debug(' exp3 ');
     m:=extensions(t,insertion[exp3rule].rulestart,false);
     if m= none then exp4 (t);
     sym:=lexsymbol;
     if m<>total then
     while errorfree and( have(plus_SY) or have(MINUS_SY)) do
     begin
           exp4(t1);
           balance(t,t1);
           binaryop(sym,t);
           sym:=lexsymbol;
     end;
end;
{ -------------------------------------------------------------- }
{     EXP2
         this parses the rules
              <exp2> ::= [~] <exp3> [<relop> <exp3>]
              <relop>::= is|isnt|<|>|>=|<=|~=|=

}
{ -------------------------------------------------------------- }
procedure exp2 (var t:typerec);
var neg:boolean;t1:typerec; sym:lexeme;m:matchtype;
begin
   debug(' exp2 ');
   neg:=have(TILDE_SY);
   m:=extensions(t,insertion[exp2rule].rulestart,false);

   if m= none then exp3 (t);
   if neg then begin match(t,bool_type);notop; end;
   sym:=lexsymbol;
   if m<>total then
   case lexsymbol of
   is_sy,isnt_sy  :
         begin
             match(t,pntr_type);
             next_symbol;
             exp3(t);

             match(t,structure_type) ;
             binaryop(sym,structure_type);
             t:=condition_type;
         end;
   eq_sy,neq_sy,ge_sy,gt_sy,le_sy,lt_sy:
         begin
              next_symbol;
              exp3 (t1 );
              balance(t,t1) ;
              binaryop(sym,t);
              t:=condition_type;
         end;
   else begin end;
   end;
end;
{ -------------------------------------------------------------- }
{     EXP1
          this parses the rule
               <exp1> ::= <exp2>[and <exp2>]*

}
{ -------------------------------------------------------------- }
procedure exp1 (var t:typerec);
var ok,fail,l2, part2:labl;m:matchtype;
begin
     debug(' exp 1 ');
     m:=extensions(t,insertion[exp1rule].rulestart,false);
     if m= none then exp2 (t);
     if m<>total then
     if have(AND_SY) then
     begin
          condify(t)   ;
          fail:=newlab;
          jumpf(fail);
          exp1(t); condify(t);ok:=newlab;jumpt(ok);
          plant(fail); ll_int(0);l2:=newlab;jumpop(l2);  { the and has failed }
          decsp(int_type);               { we only return a single value }
          plant(ok);ll_int(1);plant(l2); { the and has succeded }
          t:=bool_type;
          end;
end;
{ -------------------------------------------------------------- }
{     EXPRESSION
                this parses the rule
                     <expression>::= <exp1>[or <exp1>]*

}
{ -------------------------------------------------------------- }
procedure expression (var t:typerec);
var ok,ok2,fail,past:labl;sym:lexeme;
    m:matchtype;
begin
     debug(' expression ');
     sym := lexsymbol;
     m:=extensions(t,insertion[expressionrule].rulestart,false);
     if m= none then exp1 (t);
     if m<>total then
     if  have(OR_SY) then
     begin

         condify(t);
         ok:=newlab;fail:=newlab;ok2:=newlab;
         jumpf(ok2);jumpop(ok);plant (ok2);
         expression(t);condify(t);jumpt(ok);
         ll_int(0); jumpop(fail);
         decsp(int_type);               { we only return a single value }
         plant(ok);ll_int(1);
         plant(fail);
         t:=bool_type;
         end;
end;
{ -------------------------------------------------------------- }
{     IF_CLAUSE
               this parses the rule
               <ifclause> ::= if <clause> do <clause> |
                              if <clause> then <clause> else <clause>

}
{ -------------------------------------------------------------- }
procedure if_clause ;
var t1:typerec;l,l1,l3:labl;
begin
     l1:=newlab; l:=newlab;l3:=newlab;
     next_symbol;
     clause(t);  condify(t);
     jumpt(l);jumpop(l3);plant(l);
     if have(do_sy) then begin clause(t);plant(l3); match(t,VOID) end else
     begin
          mustbe(then_sy);
          clause(t1);jumpop(l1);decsp(t1);
          mustbe(else_sy);
          plant(l3);
          clause(t);balance(t,t1); plant(l1); release_label(l1);
     end;
     release_label(l3);
end;
{ -------------------------------------------------------------- }
procedure write_clause;
var t:typerec;   oldsp:integer;
begin
     if have (out_byte_sy) then begin
        load(s_o);
        clause(t);match(t,int_type);mustbe(comma_sy);
        clause(t);match(t,int_type);
        out_byte_op;
     end else
     if have (output_sy) then begin
        clause(t);match(t,file_type);mustbe(comma_sy);
     end else begin
         next_symbol;
         load(s_o);
     end;
     repeat
           oldsp:=stack_ptr;
           clause (t); boolify(t);
           if have (colon_sy) then clause(t) else
           begin
              if t.typeid = real_sy then load(r_w) else load(i_w);
           end;
           load(s_w);
           writeop(t);
           retractstack(oldsp);
     until not have(comma_sy);
     end_write;
end;
{ -------------------------------------------------------------- }
procedure for_clause;
var t:typerec;l1,l2:labl;id:lextoken; os,   n:namedesc;

    complex:boolean;
begin
     enterscope(os);
     l1:=newlab;l2:=newlab;
     next_symbol;
     id:=symbol;
     mustbe(identifier);
     mustbe(eq_sy);
     clause(t);match(t,int_type);
     n:=newid(id,cint_type);
     mustbe(to_sy);
     clause(t);match(t,int_type);
     if have(by_sy) then begin complex:=true; clause(t);match(t,int_type);end
     else complex:=false;
     mustbe(do_sy); if not complex then forprepop;
     plant(l1);fortestop(complex,l2);
     clause(t);match(t,VOID);
     forstepop(complex,l1);plant(l2);
     exitblock(os,VOID);
end;
{ -------------------------------------------------------------- }
procedure repeat_clause;
var t:typerec;
    l1,l2,l3:labl;
begin
     l1:=newlab;l2:=newlab;l3:=newlab;
     next_symbol;
     plant(l1);
     clause(t); match(t,VOID);
     mustbe(while_sy); clause (t); condify(t);
     jumpt(l3);jumpop(l2);plant(l3);
     if have (do_sy) then
     begin clause(t); match(t,VOID); end;
     jumpop(l1);plant(l2);
end;
{ -------------------------------------------------------------- }
{       WHILE CLAUSE                                             }
{             recognises: while <bool> do <void>                 }
{ -------------------------------------------------------------- }
procedure while_clause;
var t:typerec;
    l1,l2,l3:labl;
begin
     l1:=newlab; l3:=newlab;l2:=newlab;
     plant(l1);
     mustbe(while_sy); clause (t); condify(t); jumpt(l3);jumpop(l2);
     plant(l3);
     if have (do_sy) then
     begin clause(t); match(t,VOID); end;
     bjump(l1);plant(l2);
end;
{ -------------------------------------------------------------- }
procedure case_clause (var t:typerec);
var t1,t2:typerec; first:boolean; l1,l2,l3:labl;os,n:namedesc;
begin
   enterscope(os);
   next_symbol;
   clause(t1);
   n:=newid(tokenof('..cv'),t1);
   mustbe(of_sy);
   first:=true;
   l3:=newlab;
   while errorfree and not have(default_sy) do begin
     l1:=newlab;;l2:=newlab;
         repeat clause (t);match(t,t1); load(n);binaryop(eq_sy,t);jumpt(l1);
         until not have(comma_sy);
         mustbe(colon_sy);
         jumpop(l2);plant(l1);
         clause(t);
         if not first then begin match(t2,t);end;
         decsp(t) ;
         t2:=t; first:=false;
         skipnl;
         bjump(l3);  plant(l2);
   end;
   mustbe(colon_sy); clause(t); match(t,t2);plant(l3);
   exitblock(os,t2);
end;
{ -------------------------------------------------------------- }
{   RECOGNISES                                                   }
{      tab                                                       }
{          clause : clause                                       }
{          clause : clause                                       }
{          ....                                                  }
{      end                                                       }
{ -------------------------------------------------------------- }
procedure tab_clause (var t:typerec);
var t1,t2:typerec;
    pairs:integer;os,n:namedesc;
    procedure singleval;
    begin
         mustbe(colon_sy);
         clause(t);
         expand_to_pntr(t);
         match(t1,t);
   end;
begin
   next_symbol;
   type0(t1);
   mustbe(of_sy);
   pairs:=0;
   skipnl;
   while not have(default_sy) and errorfree do begin
         clause (t);match(t,string_type);
         singleval;
         pairs:=pairs+1;
         skipnl;
   end;
   { all pairs now on the stack }
   singleval;
   { default on the stack }
   mcktab;
   { empty table on the stack }
   while errorfree and( pairs>0) do begin
         inittab;
         pairs:=pairs-1;
   end;
   { initialised table on the stack }
   tableof(t1,t);

end;
{ -------------------------------------------------------------- }
{    PROCEDURE_CLAUSE                                            }
{    ---------------- handles procedure declarations             }
{      proc(<typelist>-><type>);<body>                           }
{      <body>:== import <string>|alien <string>|<clause>         }
{ -------------------------------------------------------------- }
procedure procedure_clause(procclass:lexeme; pname:idref;
                           var proctype:typerec;extern:boolean);
var t,restype       :typerec;
    id              :lextoken;
    oldsp           :integer;
    idnumber,
    resultloc,
    oldscope        :namedesc;
    n1              :idref;
    l,l2            :labl;
    n               :namedesc;
       procedure get_result;
       begin
            type0(restype);
            resultloc:=newname(ord(undefined),restype,param,n1);
            setresult(proctype,n1);
       end  ;
       procedure voidproc;
       begin
                      resultloc:=newname(ord(undefined),restype,param,n1);
                      setresult(proctype,n1);
       end;
begin
     if procclass = proc_sy then mustbe(proc_sy);
     l2:=newlab;                   { starting address of proc }
     proctype:=void;restype:=void;proctype.typeid:=procedure_sy;
     proctype.aliasid:=ord(procedure_sy);
     if not extern then
     if procclass = proc_sy then begin
           form_closure(l2);l:=newlab;jumpop(l);
     end;
     oldsp:=stack_ptr;
     enterlexlevel; enterscope(oldscope); stack_ptr:=enter_stackframe;
     if have(brace_sy) then begin
       { Declare the Parameters }
       if   have (rbrace_sy) then voidproc else begin
        if   have (arrow_sy) then get_result
        else begin
          repeat
             type1(t);
             repeat
                   id:=symbol;
                   mustbe( IDENTIFIER);
                   idnumber:=newname(id,t,param,n1);
                   appendparam(proctype,n1);
             until not have(comma_sy);
          until not have(newline_sy);
          if have(arrow_sy) then   { put the result on the list }
             get_result
          else voidproc;
        end;
        mustbe(rbrace_sy);
       end;
     end else voidproc;
     fixup_params(proctype);    { assign addresses to the params }
     { plant type check info }
     if extern then plant_string(ptype(proctype));
     skipnl;
     plant(l2);                 { start of procedure             }
     if extern then   bindlabel(l2,pname);
     if have(alien_sy) then begin
        mustbe(string_lit);
        prologop(1);            { assume all aliens at lex level 1 }
        aliencall(the_string);
        exit_proc(oldscope); exitlexlevel;
        epilogop(proctype);
     end
     else
     begin
        { build actual body of procedure }
        prologop(lexical_level);
        clause(t);
        boolify(t);
        match(t,restype);
        if not eq(t,VOID) then begin
              assignop(resultloc);
              exit_proc(oldscope); exitlexlevel;
              epilogop(proctype);
        end else
        begin
                exit_proc(oldscope); exitlexlevel;
                epilogop(proctype);
        end;
     end;
     stack_ptr := oldsp;
     if not extern then if procclass = proc_sy then
     begin plant(l); release_label(l) end;

end;

{ --------------------------------------------------------------- }
{    IMPORT CLAUSE                                                }
{    -------------                                                }
{                 recognises                                      }
{                 import (<string>) <type1> segment string          }
{ --------------------------------------------------------------- }

procedure import_clause(var t:typerec);
var t2:typerec;
begin
     mustbe(import_sy);
     mustbe(brace_sy);
     expression(t);
     mustbe(rbrace_sy);
     match(t,string_type);
     type1(t);
     ll_string(ptype(t));
     skipnl;
     mustbe(segment_sy);
     clause(t2);
     match(t2,string_type);
     bindop;
end;

{ --------------------------------------------------------------- }
{    PREFIX OP                                                    }
{    ---------   handles bit operations                           }
{ --------------------------------------------------------------- }
procedure prefixop(var t:typerec);
var s:lexeme;  t1:typerec;
begin
     s:=lexsymbol;
     next_symbol;
     mustbe(brace_sy);
     clause(t1);
     mustbe(comma_sy);
     clause(t);balance(t,t1); binaryop(s,t);
     mustbe(rbrace_sy);
end;
{ -------------------------------------------------------------- }
{     CLAUSE                                                     }
{     ------                                                     }
{            handles                                             }
{            if ....                                             }
{            repeat ...                                          }
{            while ...                                           }
{            procedure ...                                       }
{            for ...                                             }
{            case ...                                            }
{            abort ...                                           }
{            bit operations                                      }
{            write ...                                           }
{            output ...                                          }
{            tab ...                                             }
{ -------------------------------------------------------------- }
procedure clause ;
var i:integer;
    n:idref;
begin
     debug(' clause ');
     t:=void;
     if none= extensions(t,insertion[clauserule].rulestart,false) then
     case lexsymbol of
     newline_sy  :begin next_symbol; clause(t) end;
     if_sy       :if_clause(t);
     repeat_sy   :repeat_clause;
     while_sy    :while_clause;
     proc_sy     :procedure_clause(proc_sy,n,t,false);
     import_sy   :import_clause(t);
     for_sy      :for_clause;
     case_sy     :case_clause(t);
     abort_sy    :next_symbol;
     b_and_sy,b_or_sy,shift_l_sy,shift_r_sy:prefixop(t);
     out_byte_sy,
     output_sy,
     write_sy    :write_clause;
     tab_sy      :tab_clause(t);
     else        expression(t)
     end;
end;
{----------------------------------------------------------------}
{  prevent global declarations                                   }
{  a library segment must be pure code we must not allow any     }
{  stack data to be declared at gloabl level                     }
{  the exception to this are the std decls known to all procs    }
{----------------------------------------------------------------}
procedure validateGlobals;
begin
     if lexical_level = global_level then
        if library in options then error('no let decls in segment');

end;
{ -------------------------------------------------------------- }
{    PROCEDURE_DECL                                              }
{    --------------                                              }
{                  handle salgol style procedure declarations    }
{ -------------------------------------------------------------- }
procedure procedure_decl;
var t,restype,proctype:typerec;  id:lextoken; n:namedesc;
    n1:idref;
    l,l2:labl;
    name:textline;
begin
     mustbe(procedure_sy);
     name:=currentid;
     id:=symbol;
     mustbe(identifier);
     if (library in options) and (lexical_level =global_level) then begin

        start_external(name);
        n:=newname(id,proctype,static,n1);
        procedure_clause(procedure_sy,n1,scopelist[n]^.typeinfo,true);
        end_external(name);
     end
     else
     begin
          proctype:=void;
          l2:=newlab; l:=newlab;
          form_closure(l2);jumpop(l);plant(l2);
          if lexical_level =global_level then n:=newname(id,proctype,global,n1)
          else n:=newname(id,proctype,local,n1);

          procedure_clause(procedure_sy,n1,scopelist[n]^.typeinfo,false);
          plant(l); release_label(l);
     end;
end;


{ -------------------------------------------------------------- }
{          DO_INCLUSION                                          }
{            include <stringlit>                                 }
{ -------------------------------------------------------------- }
procedure do_inclusion;
var name:textline;   t:typerec;
begin
     mustbe(include_sy);
     mustbe(string_lit);name:=the_string;
     if push_buffer then loadtext(the_buffer,name,false)
     else error('Depth of Includes too great');

end;

{ -------------------------------------------------------------- }
{    LET_DECL                                                    }
{    --------  handles let <id> [:]= <clause>                    }
{ -------------------------------------------------------------- }
procedure let_decl;
var t:typerec; id:lextoken;constant:boolean;n:namedesc;
begin
     debug(' let_decl ');
     validateGlobals;
     next_symbol;
     id:=symbol;
     mustbe(identifier);
     constant:= have (eq_sy);
     if not constant then mustbe(assign_sy);
     clause(t);
     boolify(t);
     t.constant:=constant;
     n:=newid(id,t) ;
end;
{ -------------------------------------------------------------- }
{ CLASS DECL                                                     }
{ __________  Handles declarations of new classes                }
{             syntax:                                            }
{                    class <identifier> is <type0> and <cbody>   }
{ -------------------------------------------------------------- }
procedure class_decl;
var classid:lextoken;representation,self:typerec;
    isconst:boolean;
    ins:insertions;
    { ---------------------------------------------------------- }
    { PREDEFINED                                                 }
    { ---------- check that we have a valid production label     }
    { ---------------------------------------------------------- }
    procedure predefined(var lins:insertions);

    begin
        lins:=findinsertion(lexsymbol);
        if lins=norule then error('rule id expected');
        next_symbol;
    end;
    { ---------------------------------------------------------- }
    { LHS                                                        }
    { --- parses the left hand side of a grammer rule            }
    {     <lt_sy> <type0> - <predefinedproduction> <gt_sy>       }
    { ---------------------------------------------------------- }
    procedure lhs(var t:typerec;var ins:insertions);
    begin
       mustbe(lt_sy);
       type0(t);

       mustbe(minus_sy);
       predefined(ins);
       mustbe(gt_sy);
    end;
    { ---------------------------------------------------------- }
    { PATTERN                                                    }
    { -------  an alternation of strings and lhses               }
    { ---------------------------------------------------------- }
    function pattern(var t:typerec):rulerange;
    var s:textline;
        r:rulerange;
        ins:insertions;
    begin
       t:=void;
       skipnl;
       s:=the_string;
       if have(string_lit) then
       begin
             r:=nextrule;
             with rules[r] do begin
                  sort:= compulsory;
                  mustbesym:=tokenof(s);
                  tail:=pattern(t);
             end;
             pattern:=r;
       end else
       if lexsymbol=lt_sy then
       begin

             lhs(t,ins);
             r:=nextrule;
             with rules[r] do begin
                  sort:= subrule;
                  prodtype:=t;
                  proc:=ins;
             end;
             pattern:=r;
             if errorfree then rules[r].tail:= pattern(t);
       end
       else pattern:=nullrule;

    end;
    { ---------------------------------------------------------- }
    { PRODUCTION                                                 }
    { ---------- Handles <lhs> is <rhs>                          }
    { ---------------------------------------------------------- }
    procedure production(var prodyields:typerec);
    var ins:insertions;r:rulerange;
        function reduce(r:rulerange):rulerange;
        { this dis ambiguates a grammer }
        var secondhead:rulerange;
        begin
          with rules[r] do
          case sort of
          alternative :begin
                         head:= reduce(head);
                         tail:= reduce(tail);
                         if rules[tail].sort = alternative then
                         begin
                            secondhead:=rules[tail].head;
                            if rules[secondhead].sort <>rules[head].sort
                            then error('ambiguous grammer ')
                         end;
                         reduce:=r;
                       end
          else reduce:=r;
          end;
        end;




    { ---------------------------------------------------------- }
    { RHS                                                        }
    { --- parses the right hand side of a grammer rule           }
    {     <cur> <pattern> <ley> = <identifier                    }
    { ---------------------------------------------------------- }
    function rhs:rulerange;
    var t:typerec;
        meaning:lextoken;
        p:paramref;
        r,q,u:rulerange;
    begin
       mustbe(cur_sy);
       u:=nextrule;
       rhs:=u;
       with rules[u] do begin
          head:=pattern(t);
          sort:=binding;
       end;
       mustbe(ley_sy);
       mustbe(eq_sy);
       meaning:=symbol;
       mustbe(identifier);
       if lookup(meaning)>0 then begin
        typeof(meaning,t);
        { t should be the type of a procedure that implements the }
        { sematics of this grammer rule we must check to see if   }
        { the grammer rule put the right types on the stack for   }
        { what the procedure expects                              }
        if t.typeid <> procedure_sy then error('should be procedure');
        r:=nextrule;
        q:=r;
        with rules[r] do begin
                  sort:= semantics;
                  id:=meaning;
                  tail:=nullrule;
             end;
        p:=t.params;
        coerce(prodyields,t.result^.typeinfo);
        r:=rules[u].head;
        while (p <> nil) and errorfree do begin
          while rules[r].sort = compulsory do
          begin
                r:=rules[r].tail;
          end;
          if rules[r].sort<>subrule then begin
             error('wrong number of arguments')
          end
          else begin

               coerce(rules[r].prodtype,p^.paramname^.typeinfo);
               p:= p^.next;
               r:=rules[r].tail;
          end;

        end;
        rules[u].tail:=q;{ attach semantics to the binding }
       end else error('undeclared procedure');
    end;



    begin  { production }
         skipnl;
         lhs(prodyields,ins);
         mustbe(is_sy);
         r:=nextrule;
         with rules[r] do begin
             sort:=alternative;
             prodtype:=prodyields;
             head:=rhs;
             tail:=  insertion[ins].rulestart;
         end;
         insertion[ins].rulestart:= reduce(r);
    end;
    { ---------------------------------------------------------- }
    { CBODY                                                      }
    { -----  Body of class declaration                           }
    {        <cur> <production> [ or <production>] <ley>         }
    { ---------------------------------------------------------- }
    procedure cbody;
    var t:typerec;
    begin
       skipnl;
       mustbe(cur_sy);
       production(t);
       skipnl;
       while have (or_sy) and errorfree do
       begin production(t);skipnl end;
       mustbe(ley_sy);
    end;
begin
   mustbe(class_sy);
   classid :=symbol;
   mustbe(identifier);
   isconst:=have(eq_sy);
   if not isconst then mustbe(is_sy);
   type0(representation);
   mkalias(self,representation,classid,false);

   if have(and_sy) then cbody;
end;
{ -------------------------------------------------------------- }
{  STRUCTURE_DECL                                                }
{  -------------- handles structure(<typelist>)                  }
{ -------------------------------------------------------------- }
procedure structure_decl;
var structype:typerec;
var t,stype:typerec; id,struct:lextoken;constant:boolean;
    idnumber,n:namedesc;
    class:textline;
    reals,ints,pntrs:integer;
    n1:idref;
begin
     next_symbol;
     struct:=symbol;
     class := currentid;
     mustbe(identifier);
     mustbe(brace_sy);
     structype:=structure_type;structype.typeid:=structure_sy;
          repeat
             type0(t);
             t.field:=true;
             repeat
                   id:=symbol;
                   mustbe( IDENTIFIER);
                   idnumber:=newname(id,t,local,n1);
                   appendparam(structype,n1);
             until not have(comma_sy);
          until not have(newline_sy);
     n:=newid(struct,structype);

     mustbe(rbrace_sy);
     fixupstruct(structype,pntrs,reals,ints);
     declarestructure(class,pntrs,reals,ints,n);
end;
{ -------------------------------------------------------------- }
{  SEQUENCE                                                      }
{  -------- handles                                              }
{                 let ...                                        }
{                 include ...                                    }
{                 <clause>                                       }
{                 structure ...                                  }
{                 procedure ...                                  }
{                 TRACEON ...                                    }
{                 TRACEOFF ...                                   }
{                 export ...                                     }
{ -------------------------------------------------------------- }
var statements:integer;
procedure sequence;
begin
     t:=VOID;
     repeat  debug('sequence');

           { it is necessary to distinguish between the case where we
             are at the end of a sequence and the where there is another
             statement in the sequence yet to come.
             If we find that there is another statement to come,
             then we must make sure that the last statement left a void
             result on the stack
             }
           case lexsymbol of
            newline_sy,end_sy,ley_sy:begin end;
            else begin
                 newlineop;
                  match(t,VOID);
                 end;
           end;
           case lexsymbol of
           end_sy,ley_sy,
           newline_sy  : begin end;
           let_sy      :let_decl;
           class_sy    :class_decl;
           include_sy  :do_inclusion;
           traceon_sy  :begin next_symbol;trace:=true;end;
           traceoff_sy :begin next_symbol;trace:=false;end;
           liston_sy   :begin next_symbol;listprog:=true;end;
           listoff_sy  :begin next_symbol;listprog:=false;end;
           structure_sy:structure_decl;
           procedure_sy:procedure_decl;
           else clause (t)
           end;
           statements:=statements+1;
     until (not have(newline_sy)) or(not( errorfree or (batch in options)));
end;
{ -------------------------------------------------------------- }
{    TYPE0                                                       }
{    -----                                                       }
{         handles declaration of base types                      }
{ -------------------------------------------------------------- }
procedure type0 ;
begin
   if istype(symbol)
   then begin  findtype(symbol,t);
               next_symbol;
        end
   else if have(star_sy) then begin
           type0(t); t.dimensions:=succ(t.dimensions);
        end
   else if have(dollar_sy) then begin
           type0(t); tableof(t,t);
        end
   else t:=void;
end;
{ -------------------------------------------------------------- }
{    TYPE1                                                       }
{    -----  handles declaration of procedure types               }
{           when used as procedure parameters                    }
{ -------------------------------------------------------------- }
procedure type1 ;

var restype,
    proctype ,t1    :typerec;
    id              :lextoken;
    oldsp           :namedesc;
    os,
    idnumber,
    resultloc       :namedesc;
    n1              :idref;
    l,l2            :labl;
    n               :namedesc;
       procedure get_result;
       begin
            type0(restype);
            resultloc:=newdummy(ord(begin_sy),restype,param,n1);
            setresult(proctype,n1);
       end  ;
       procedure voidproc;
       begin
                      resultloc:=newdummy(ord(begin_sy),restype,param,n1);
                      setresult(proctype,n1);
       end;

begin

     proctype:=void;restype:=void;proctype.typeid:=procedure_sy;
     proctype.aliasid:=ord(procedure_sy);
           case lexsymbol of
           proc_sy: begin
                      next_symbol;
                      if have(brace_sy) then begin
                   {   enterscope(oldsp);}
                       if have (rbrace_sy) then voidproc
                       else begin
                        if not  have (arrow_sy) then begin
                         repeat
                          type1(t1);
                          id:=ord(end_sy);
                          idnumber:=newdummy(id,t1,local,n1);
                          appendparam(proctype,n1)
                         until not have(comma_sy);
                         if have(arrow_sy) then begin
                            get_result;
                         end else voidproc;
                        end
                        else get_result;
                        mustbe(rbrace_sy);
                       end;
                      end else voidproc;
              fixup_params(proctype);    { assign addresses to the params to work }
                                         { out space for result }

                      t:=proctype;
{                     exit_proc(oldsp);   }
             end;
          { structure_sy:structure_decl; }
           else type0(t)
           end;
end;
{ -------------------------------------------------------------- }
{ EXTENSIONS                                                     }
{      handle dynamic syntax extensions                          }
{ -------------------------------------------------------------- }

FUNCTION     EXTENSIONS{(Var t:typrec;r:rulerange); FORWARD};
var t2:typerec;
    n:namedesc;
    comp:rulerange;
    try:matchtype;

label 2;
BEGIN
     extensions:=none;
     with rules[r] do
     case sort of
     null:begin t:=void; extensions:=none;end;
     binding  : begin
                try:= extensions(t,head,alreadyhave);
                if try =total then
                extensions:= extensions(t2,tail,alreadyhave)
                else extensions:=try;

                end;
     semantics: begin
               n:= lookup(id)   ;
               if n= nulid then error('Semantic function out of scope');
               call_proc(n);
               extensions:=total;
                end;

     subrule : begin
                    if not alreadyhave then
                    case proc of
                    clauserule:clause(t);
                    expressionrule:expression(t);
                    exp1rule:exp1(t);
                    exp2rule:exp2(t);
                    exp3rule:exp3(t);
                    exp4rule:exp4(t);
                    exp5rule:exp5(t);
                    end;
                    t2:=prodtype;
                    {if alreadyhave then begin
                    write('left ');
                    conwritetype(t);
                    write(' right ');conwritetype(t2);writeln('      ');
                    end;}
                    { for a clause the type is compulsory }
                    if eq(t,t2) then
                    { we should take this alternative }
                    begin
                     2:  try:=extensions(t,tail,false);
                       if try = none then begin
                            try:=partial
                       end;
                       extensions:=try;
                    end
                    else if eq(t,int_type) and eq(t2,real_type) then
                    begin
                        floatop;goto 2;
                    end
                    else extensions:=partial;
              end;
     compulsory: begin
                 if symbol = mustbesym then begin
                    next_symbol;
                    extensions:= extensions(t,tail,alreadyhave);
                    extensions:=total;     { by this point we can not back out }
                   end else extensions:=none;
                 end;
     alternative:
                 begin
                    { head points at a bind }
                    { head of the bind may be compulsory }

                    case extensions(t,head,alreadyhave) of
                    none : try:=extensions(t,tail,alreadyhave);
                    total: begin
                                try:=total;
                                t:=prodtype;

                           end;
                    partial:begin
                                  t2:=t;
                                  try:=extensions(t2,tail,true);
                                  if try=none then
                                  begin

                                     try:=partial
                                  end
                                  else if try = total then t:=t2;

                            end;
                    end;
                    extensions:=try;
                end;
     end;

END;

{ -------------------------------------------------------------- }
{               LIB                                              }
{               this parses the production                       }
{               segment <string lit> class <string lit >         }
{ -------------------------------------------------------------- }
procedure lib(var segname:textline);
begin

     mustbe(string_lit); segname:=the_string;
end;

{ -------------------------------------------------------------- }
{   PROG                                                         }
{   ----                                                         }
{ -------------------------------------------------------------- }
{procedure prog;                                                 }
var t:typerec;scope:namedesc;
    p:^integer;
begin
     incompiler:=true;
     mark(p);
     initlexanal;initsymtab;
     enterscope(scope);
     toprule:=0;
     statements:=0;
     defineinsertions;
     if library in options then options := options - [library];
     segment:='_text';class:='code';
     next_symbol;
     if have (segment_sy) then options := options +[library];
     if library in options then lib(segment);
     init_cgen(segment,class);
     if not( library in options )then
     begin
          start_program;

          std_decls;
               sequence(t);
          end_program(segment);
          end
     else begin
          start_library;
          std_decls;

          sequence(t);
          end_library(segment);
          end;
     match(VOID,t);
     mustbe(question_sy);
     exitblock(scope,VOID);

     finalise_cgen;
     release(p);
     close(listfile);
     incompiler:=false;
end;
begin

end.