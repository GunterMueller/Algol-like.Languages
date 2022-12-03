{

-----------------------------------------------------------------
Module      : DLB.pas
Used in     : Compiler toolbox
Uses        : Fsm.cmp
Author      : W P Cockshott
Date        : 9 Oct 1986
Version     : 2
Changes     : changed to unit 21 - 9 - 88
Function    : This uses a de La Brandis tree to convert the
              lexemes to integer tokens
              next_symbol,have, and mustbe provide the basic
              entry points of the lexical analyser.
Copyright (C) WP Cockshott & P Balch
----------------------------------------------------------------
}
{$r+}
UNIT dlb;

INTERFACE

USES
  errors,
  editdecl,
  fsm,env;

type lexeme=(dummy_sy, { all valid lexemes > 0  so we put in a dummy }
BRACE_SY,
RBRACE_SY,
STAR_SY,
DOLLAR_SY,
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
ALIEN_SY,
AND_SY,
B_AND_SY,
B_OR_SY,
BEGIN_SY,
CBOOL_SY,BOOL_SY,
BY_SY,
CASE_SY,
CLASS_SY,
CLAUSE_SY,
COLOUR_SY,
CONDITION_SY,
DEFAULT_SY,
DIV_SY,
DO_SY,
ELSE_SY,
END_SY,
EOI_SY,
EXP1_SY,
EXP2_SY,
EXP3_SY,
EXP4_SY,
EXP5_SY,
EXPORT_SY,
EXPRESSION_SY,
FALSE_SY,
CFILE_SY,FILE_SY,
FOR_SY,
forward_sy,FROM_SY,
IF_SY,
IMPORT_SY,
IN_SY,
INCLUDE_SY,
CINT_SY,INT_SY,
IS_SY,
ISNT_SY,
LENGTH_SY,
LET_SY,
LISTON_SY,
LISTOFF_SY,
LWB_SY,
NIL_SY,
OF_SY,OR_SY,
OUT_BYTE_SY,
OUTPUT_SY,
PEEK_SY,
CPNTR_SY,PNTR_SY,
CPROC_SY,PROC_SY,
CPROCEDURE_SY,PROCEDURE_SY,
READ_SY,
READ_A_LINE_SY,
READ_BYTE_SY,
READ_NAME_SY,
READB_SY,
READI_SY,
READP_SY,
READR_SY,
READS_SY,
CREAL_SY,REAL_SY,
REM_SY,
REPEAT_SY,
ROTATE_SY,
SCALE_SY,
SEGMENT_SY,
SHIFT_SY,
SHIFT_L_SY,
SHIFT_R_SY,
CSTRING_SY,STRING_SY,
STRUCTURE_SY,
SUBSCRIPT_SY,
TAB_SY,
TEXT_SY,
THEN_SY,
TO_SY,
TRACEON_SY,
TRACEOFF_SY,
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
assign_sy,
comma_sy,
at_sy,
question_sy,
UNDEFINED);

const maxvar = 200;
      maxtoken = 900;
      maxstring=maxvar;
      tab =9;
      newline = 10;
      cr  =13;
      bs = 8;
      vtab = 11;
{
     *****************
     *  TYPES        *
     *****************}
type
lextoken = integer;
stringv  = string[maxstring];
var
       symbol : lextoken;
       lexsymbol :lexeme;
       the_string:stringv;
       the_real:real;
       the_integer:integer;

{
   *****************
   * PROCEDURES    *
   *****************

}
  function psym(l:lextoken): stringv;
  procedure mustbe(t : lexeme );
  function have( t: lexeme) : boolean;
  function tokenof(s:stringv):lextoken;
  procedure next_symbol;
  function currentid:stringv;
  procedure initlexanal;


IMPLEMENTATION
const
      minchar = ' ';
      maxchar = '~';
type
    lexnode = ^delabrandis;
    lexarray = array[minchar..maxchar] of lexnode;
    delabrandis = record
                      final:integer;
                      pref:char;
                      follower,alternates:lexnode;
               end;

var    predefined:lexarray;
       maxpredefined:lextoken;
       linebuf:stringv;
       converted:string[30];
       lasttoken:lextoken;
       {
       ------------------------- de la brandis tree manipulation -------------}

function displaydlb(p:lexnode; indent:byte;token :lextoken):boolean;
var i:byte;
begin
   if p<>nil then   begin
     converted[indent]:=p^.pref;
     converted[0]:=chr(0);

     if p^.final = token
     then begin displaydlb:=true;
                converted[0]:=chr(indent);
     end
     else if displaydlb(p^.follower,indent+1,token) then displaydlb:=true
     else
     if p^.alternates <>nil then begin
        displaydlb:=
        displaydlb(p^.alternates,indent,token);
     end else begin
       displaydlb:=false;
     end;
  end  else displaydlb:=false;
end;

{
       ****************
       * INSERT_TOKEN *
       ****************
       }
function insert_token(var B:textbuffer; var n:lexarray):lextoken;
{  inserts the string into the tree }
{$S-}
var p      : lexnode;
    charno : integer;
    c      : char;
    hit ,inserted   :boolean;
    procedure newnode(var next:lexnode;c:char);
    begin
        new(next);
        with next^ do begin
          pref:=c; final:=0;
          follower:=nil;
          alternates:=nil;
        end;
    end;
    procedure ins(var n:lexnode;charno: word );
    var t:lexnode;
        c:char;
    begin
       c:=b.thetext^[charno];
       if charno <B.finish then with B do
          if n=nil then begin
             newnode(t,c);
             n:=t;
             ins(n,charno)
          end
          else with  n ^ do
            if c = pref then begin
                 if charno=finish -1 then begin
                    { a  hit }
                    hit:=true;
                    if final = 0 then { first time we have encountered this word}
                    begin
                       final:=lasttoken;
                       lasttoken:=succ(lasttoken);
                    end;
                    insert_token:=final;
                 end
                 else ins(follower,charno+1);
            end
            else if c< pref then ins(alternates,charno)
            else
            begin { char less than pref }
                  t:=n;
                  newnode(n,c);
                  n^.alternates:=t;
                  ins(n,charno);
            end ;
    end;
begin
 {$r-}
 ins(  n[B.thetext^[B.start]], B.start);

end;
{
        ***************
        * INITLEXANAL *
        ***************
}
procedure initlexanal;
var B:textbuffer;
    i:integer;
    c:char;
    s:string[90];
    f:text;
begin
 lasttoken:=0;

  { create the symbol tree }
  for c:=minchar to maxchar do begin
   new(predefined[c]);
   with predefined[c]^ do begin
       follower:=nil;
       alternates:=nil;
       pref:=chr(0);
   end;
  end;
  B.thetext:=addr(s[1]);
  { load up the predefined variables }
  assign(f,PSDIR+lexemefile);
  reset(f);
  with B do repeat
      linenumber:=1;
      start:=textbuflo; readln(f,s);
      finish:=length(s)+start;
      maxpredefined:=insert_token(B,predefined);
    until  eof(f);
  close(f);
     assign(listfile,listfilename);rewrite(listfile);
end;
{
        ***************
        * TokenOf     *
        ***************
}
function tokenof(s:stringv):lextoken;
var B:textbuffer;
begin
  { create the symbol tree }
  B.thetext:=addr(s[1]);
  with B do begin
      start:=textbuflo;
      finish:=length(s)+start;
      tokenof:=insert_token(B,predefined);
  end;

end;
{ ------------------------------------------------------------------ }
{      NEXTSYMBOL                                                    }
{      reads the next lexeme, converts to token                      }
{      leaves results in symbol, the.integer, the.real, the.string   }
{ ------------------------------------------------------------------ }
procedure next_symbol;
var S:fsmstate;
    function numconv:lexeme;
    var n:stringv;
        i,p:integer;
        isint:boolean;
    begin
        isint:=true;
        with the_buffer do begin
           n:='';
           for i:=start to finish-1 do begin
              n:=n+thetext^[i];
              isint:=isint and (thetext^[i] in ['0'..'9']);
           end;
           if isint then begin val(n,the_integer,p); numconv:=INT_LIT end
                    else begin val(n,the_real,p);  numconv:=REAL_LIT end;

       end;
    end;
    procedure printsymbol;
    var i:integer;
        c: char;
    begin
         with the_buffer  do
         if lexsymbol <> newline_sy then begin
            for i:=start to finish -1 do write(thetext^[i]);
            write(' ');
         end else
         begin
           writeln;
         end;
    end;

    function stringconv:lexeme;
    var n:stringv;
        i,p:integer;
        escape:boolean;
        c:char;

       procedure append(c:char);
       begin    if length(n)<MAXSTRING then n:=n+c; end;
    begin
        escape:=false;
        with the_buffer do begin
           n:='';
           for i:=start+1 to finish -2 do begin
              c:=thetext^[i];
              if not escape then begin
                 escape:=classtab[ord(c) and 127]=quote;
                 if not escape then    append(c);

              end
              else begin
              { this implements the S-algol rules for control chars }
              { 'n = newline                                        }
              { 't = tab                                            }
                  case c of
                  'n' : append(chr(NEWLINE));
                  't' : append(chr(TAB));
                  'o' : append(chr(CR));
                  'b' : append(chr(BS));
                  'p' : append(chr(VTAB));
                  else append(c);
                  end ;
                  escape:=false;
              end;
           end;
        the_string:=n;
        stringconv:=string_lit;
       end;
    end;
var coerce:record
        case boolean of
        true:(l1:lextoken);
        false:(l2:lexeme;dummy:byte);
        end;
begin
  coerce.dummy:=0;
  compilercursor:=the_buffer.start;
  S:=newlexeme(the_buffer);
  with coerce do
  if s in [opstate,brackstate,idstate] then
     l1:=insert_token(the_buffer,predefined)
  else if s in[numstate,expstate]  then l2:=numconv
  else if s in [stringstate,lastquotestate] then l2:=stringconv
  else
      l2:=newline_sy;
  symbol:=coerce.l1;
  if symbol >maxpredefined then lexsymbol:=identifier
  else lexsymbol:=coerce.l2;
  if stopline >0 then
      if coerce.l2=newline_sy then
             begin
                  if the_line > stopline then
                  begin
                       if  errorfree then
                       error('Run time Error');
                  end
             end;
end;
{ -------------------------------------------------------------- }
{       CURRENTID                                                }
{       returns the identifier as a string                       }
{ -------------------------------------------------------------- }
function currentid:stringv;
    var n:stringv;
        i,p:integer;
    begin
        with the_buffer do begin
           n:='';
           for i:=start to finish-1 do begin
              n:=n+thetext^[i];
           end;
        end;
        currentid:=n;
    end;
{ ------------------------------------------------------------------- }
{          PSYM                                                       }
{          print out a lexeme                                         }
{ ------------------------------------------------------------------- }
  function psym(l:lextoken): stringv;
  var
    f:text;
    s:string[30];
    i:integer;
    c:char;

  begin
     s:='';
     for c:= minchar to maxchar do
     if displaydlb(predefined[c],1,l) then s:=converted;
     psym:=s;
  end;
{ ------------------------------------------------------------------- }
{           HAVE                                                      }
{           conditionally matches a token                             }
{ ------------------------------------------------------------------- }
function have( t: lexeme) : boolean;
begin
     if t = lexsymbol then
       begin next_symbol;
             have:=true

       end
     else  have:=false;
end;
{ ------------------------------------------------------------------- }
{          SYNTAX                                                     }
{          report error and stop                                      }
{ ------------------------------------------------------------------- }
procedure syntax( t : lexeme);
var m :stringv;
begin
     m:= currentid +' found instead of '+ psym(ord(t));
     Error(m);
end;
{ ------------------------------------------------------------------- }
{           MUSTBE                                                    }
{           compulsorily matches a token                              }
{ ------------------------------------------------------------------- }
procedure mustbe(t : lexeme );
begin
     if not have(t) then  begin
     if have(newline_sy) then mustbe(t) else syntax(t);
     end;
end;


{-------------------------------------------------------------------- }
BEGIN
converted:='';
         stopline:=0;

END.