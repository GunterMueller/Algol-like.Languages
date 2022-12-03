{
-----------------------------------------------------------------
Module      : symbol.cmp
Used in     : Compiler toolbox
Author      : W P Cockshott
Date        : 14 Oct 1986
Version     : 1
Function    : enumerated type for S-algol terminal symbols

Copyright (C) WP Cockshott & P Balch
----------------------------------------------------------------
}
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
COLOUR_SY,
CONDITION_SY,
DEFAULT_SY,
DIV_SY,
DO_SY,
ELSE_SY,
END_SY,
EOI_SY,
EXPORT_SY,
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
LWB_SY,
NIL_SY,
OF_SY,OR_SY,
OUT_BYTE_SY,
OUTPUT_SY,
PEEK_SY,
CPIC_SY,PIC_SY,
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
assign_sy, comma_sy,  at_sy,
question_sy,
UNDEFINED);