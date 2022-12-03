type lexeme=(BRACE_SY,
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
BOOL_SY,
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
FOR_SY,
FROM_SY,
IF_SY,
IN_SY,
INT_SY,
IS_SY,
ISNT_SY,
LET_SY,
LWB_SY,
NIL_SY,
OF_SY,
OUT_BYTE_SY,
PEEK_SY,
PIC_SY,
PNTR_SY,
PROCEDURE_SY,
READ_SY,
READ_A_LINE_SY,
READ_BYTE_SY,
READ_NAME_SY,
READB_SY,
READI_SY,
READR_SY,
READS_SY,
REAL_SY,
REM_SY,
REPEAT_SY,
ROTATE_SY,
SCALE_SY,
SHIFT_SY,
STRING_SY,
STRUCTURE_SY,
TAB_SY,
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
NEQ_SY,NON_TERM);
lexnode = ^delabrandis;
delabrandis = record
              final:lexeme;
              case single:boolean of
              true:(
                      pref:char;
                      follower,alternates:lexnode;
                      );
              false:(
                        last:string[12];
                      );
              end;

var    predefined:lexnode;
function putin(var s:string; t:lexnode;p:integer;val:lexeme):lexnode;
vat temp:lexnode;
begin
   putin:=t;
   if p<=length(s) then
   if t<>nil then
      if t^.single then
         if t^.pref=s[p] and p=length(s) then
            final:=val
         else
         if t^.pref=s[p] and p<>length(s) then
            putin:=putin(s,t^.follower,p+1,val) else
         if t^.pref >s[p] then begin
                 new(temp);
                 with temp^ do begin
                    alternates:=t;
                    single:=true;
                    if p=length(s) then begin
                       final:=val;follower:=nil;
                    end else begin
                       final:=NON_TERM; follower:=putin(s,nil,p+1,val);
                    end
                 end;
                 putin:=temp;
        end else
        if t^.pref <s[p] then putin:=putin(s,t^.alternates,p,val
begin
end.