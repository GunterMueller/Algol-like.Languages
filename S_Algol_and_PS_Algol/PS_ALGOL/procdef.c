/*
   SALGOL dynamic linker driver table
		  this contains two arrays used by the dynamic linker
          proctable : this contains the addresses of all
					  the C and assembler routines called
					  by Salgol
          PROCNAME  : this contains the names by which these
					  c routines will be called from within
					  salgol

  to make a C procedure callable from salgol carry out
  the following steps:
  1. write your C procedure
	 suppose you have a procedure called 'bazonka'
     in a file bazonka.c
	  bazonka(struct saframe pad ,int a,int b)
	  {
	  }

  2. Compile it with Turbo C
       tcc -mh -c bazonka

  3. Include this in the salgol libraries
	   tlib salibf.lib +-bazonka.obj
	   tlib salibh.lib +-bazonka.obj

  4. Edit this file so that the new first entry in proctable is
	 (int*) bazonka,

     and the first entry in  PROCNAME is
	 {"_bazonka"}

  5. Compile this file and add it to the salgol libraries
	   tcc -mh -c procdef
	   tlib salibf.lib +-procdef.obj
       tlib salibh.lib +-procdef.obj

  6. Recompile the compiler driver and run time driver files
       tcc -mh -M s.obj salibf.lib 7layerh.lib
       tcc -mh -M psr.obj salibf.lib  7layerh.lib

  7. Declare the c routine in your salgol program
	   procedure bazonka(int b,a); alien "_bazonka"

*/


#include "link.h"
#define MAXPROC 100
int maxproc = MAXPROC;
int  ARGC(),
ARGV(),abortps(),
  clearscreen(),
  rasterop(),
  plane(),
  SWHEREX(),
  SWHEREY(),
  SGOTOXY(),
  SLINE(),
  SGETCH(),
  SSETCUR(),
  SSETPAL(),
  SSETCH(),
  textmode(),
  ginit(),
  insline(),
  delline(),
  STEXTCOL(),
  STEXTBACK(),
  Smousex(),
  Smousey(),
  Smousebut(),
  Smousepres(),
  MOUSESHOW(),
  MOUSEHIDE(),
  WRITEB(),
  WRITEI(),
  WRITER(),
  WRITES(),
  WRITEP(),
  LWBOP(),
  UPBOP(),
  MAKEVEC(),
  MCKTAB(),
  INITTAB(),
  TAB_INSP(),
  TAB_INSI(),
  TAB_INSR(),
  S_LOOKUP(),
  ILIFFEINT(),
  ILIFFEREAL(),
  ILIFFEPNTR(),
  LLSTRING(),
  SUBVASSIB(),
  SUBVASSR(),
  SUBVASSP(),
  SUBVIB(),
  SUBVR(),
  CONCAT(),
  DIVR(),
  TIMESR(),
  NEGOPR(),
  MINUSR(),
  PLUSR(),
  COMPOPR(),
  SUBVP(),
  SUBSTROP(),
  COMPSTR(),
  CODE(),
  DECODE(),
COMPPNTR(),
  MKSDESC(),
 FORMSTRUCT(),
  VALIDSTRUCT(),
  WHERESTRUCT(),
  TRACE(),
  ILIFFEINT(),
  DEFPIDREG(), DEFCSEG(),
  OPEN(),
  CREATE(),
  READ(),
  CLOSE(),
  READR(),
  PEEK(),
  READS(),
  READP(),
  READ_A_LINE(),
  READI(),
  EOI(),
  READ_BYTE(),
  SEEK(),
  ISOP(),
  classid(),
  SQRT(),
  EXP(),
  LN(),
  SIN(),
  COS(),
  TAN(),
  ATAN(),
  TRUNCATE(),
  FLOAT_LOWER(),
  FLOAT_TOP(),
  Returncode(), /* called by compiler */
  TERMINATE(),
  bind(),
  saerror();


int far *proctable [MAXPROC]={
(int*) ARGC,
(int*)ARGV,
(int*)abortps,
(int*)clearscreen,
(int*)rasterop,
(int*)plane,
(int*)SWHEREX,
(int*)SWHEREY,
(int*)SGOTOXY,
(int*)SLINE,
(int*)SGETCH ,
(int*)SSETCUR ,
(int*)SSETPAL ,
(int*)SSETCH,
(int*)ginit,
(int*)textmode,
(int*)insline,
(int*)delline,
(int*)STEXTCOL,
(int*)STEXTBACK,
(int*)Smousex,
(int*)Smousey,
(int*)Smousebut,
(int*)Smousepres,
(int*)MOUSESHOW,
(int*)MOUSEHIDE,
(int*)WRITEB,
(int*)WRITEI,(int*)
  WRITER,(int*)
  WRITES,(int*)
  WRITEP,(int*)
  LWBOP,(int*)
  UPBOP,(int*)
  MAKEVEC,(int*)
  MCKTAB,(int*)
  INITTAB,(int*)
  TAB_INSP,(int*)
  TAB_INSI,(int*)
  TAB_INSR,(int*)
  S_LOOKUP,(int*)
  ILIFFEINT,(int*)
  ILIFFEREAL,(int*)
  ILIFFEPNTR,(int*)
  LLSTRING,(int*)
  SUBVASSIB,(int*)
  SUBVASSR,(int*)
  SUBVASSP,(int*)
  SUBVIB,(int*)
  SUBVR,(int*)
  CONCAT,(int*)
  DIVR,(int*)
  TIMESR,(int*)
  NEGOPR,(int*)
  MINUSR,(int*)
  PLUSR,(int*)
  COMPOPR,(int*)
  SUBVP,(int*)
  SUBSTROP,(int*)
  COMPSTR,(int*)
  CODE,(int*)
  DECODE,(int*)
  COMPPNTR,(int*)
  MKSDESC,(int*)
 FORMSTRUCT,(int*)
  VALIDSTRUCT,(int*)
  WHERESTRUCT,(int*)
  TRACE,(int*)
  ILIFFEINT,
  (int*)DEFPIDREG,
  (int*)DEFCSEG,
  (int*) OPEN,
  (int*) CREATE,
  (int*) READ,
  (int*) CLOSE,
   (int*)READR,
   (int*)PEEK,
   (int*)READS,
   (int*)READP,
   (int*)READ_A_LINE,
   (int*)READI,
   (int*)EOI,
   (int*)READ_BYTE,
   (int*)SEEK,
   (int*)ISOP,
   (int*)classid,
   (int*)SQRT,(int* )
  EXP,(int* )
  LN,(int* )
  SIN,(int* )
  COS,(int* )
  TAN,(int* )
  ATAN,(int* )
  TRUNCATE,(int* )
  FLOAT_LOWER,(int* )
  FLOAT_TOP,(int* )
  Returncode,
  (int*)TERMINATE,
  (int*)bind,
        /* padding */
  (int*)saerror
  } ;

#define install(a) {a}

struct identifier PROCNAME[MAXPROC] ={
 install("_ARGC"),
 install("_ARGV"),
 {"_abort"},
 {"_clearscreen"},
 {"_rasterop"},
 {"_plane"},
 {"_wherex"},
 {"_wherey"},
 {"_gotoxy"},
 {"_line"},
 {"_getch"},
 {"_setcur"},
 {"_setpal"},
 {"_setch"},
 {"_ginit"},
 {"_textmode"},
 {"_insline"},
 {"_delline"},
 {"_textcol"},
 {"_textback"},
 {"_mousex"},
 {"_mousey"},
 {"_mousebut"},
 {"_mousepres"},
 {"_mouseshow"},
 {"_mousehide"},
 install("_writeb"),
 install("_writei" ),
 install("_writer"),
 install("_writes"),
 install("_writep"),
 install("_lwbop"),
 install("_upbop"),
 install("_makevec"),
  install("_mcktab"),
 install("_inittab"),
 install("_tab_insp"),
 install("_tab_insi"),
 install("_tab_insr"),
 install("_s_lookup"),
 install("_iliffeint"),
 install("_iliffereal"),
 install("_iliffepntr"),
 install("_llstring"),
 install("_subvassib"),
 install("_subvassr"),
 install("_subvassp"),
 install("_subvib"),
 install("_subvr"),
 install("_concat"),
 install("_divr"),
 install("_timesr"),
 install("_negopr"),
 install("_minusr"),
 install("_plusr"),
 install("_compopr"),
 install("_subvp"),
 install("_substrop"),
 install("_compstr"),
  install("_CODE"),
  install("_DECODE"),
 install("_comppntr"),
 install("_mksdesc"),
 install("_formstruct"),
 install("_validstruct"),
 install("_wherestruct"),
 install("_trace"),
 install("_iliffeint") ,
 install("_defpidreg") ,
 install("_defcseg") ,
 install("_OPEN"),
  install("_CREATE"),
  install("_read"),
  install("_CLOSE") ,
  install("_readr"),
  install("_peek"),
  install("_reads"),
  install("_readp"),
  install("_read_a_line"),
  install("_readi"),
  install("_eoi"),
  install("_read_byte") ,
  install("_SEEK"),
  install("_isop"),
  install("_classid"),
  install("_SQRT"),
  install("_EXP"),
  install("_LN"),
  install("_SIN"),
  install("_COS"),
  install("_TAN"),
  install("_ATAN"),
  install("_TRUNCATE"),
  install("_float_lower"),
  install("_float_top"),
 install("Returncode"),
 install("_terminate"),
 install("_bind"),
 install("_saerror")
};