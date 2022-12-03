/* taxi.h 
 * Copyleft (C) 2009-2014 Antonio Maschio 
 * @ <ing dot antonio dot maschio at gmail dot com>
 * Update: 03 gennaio 2021
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This files contains variable declarations, constants, functions prototypes
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * It's GPL! Enjoy!
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <errno.h>
#include "errors.h"
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <fnmatch.h>
#include <stdarg.h>
#include <libgen.h>
#include <pwd.h>
#include <pthread.h>
#include "custom.h"

/* UNIX only features */
#ifdef _WIN16 
    #define NOUNIX
#endif
#ifdef _WIN32 
    #define NOUNIX
#endif
#ifdef _WIN64 
    #define NOUNIX
#endif
/* CygWin only features 
 * Note: some report that powl, erfl and tgammal are not available in
 * all CygWin distributions, so I disable long version for CygWin.
 * Change only if you are sure that you have those. */
#ifdef CYG_SYS_BASHRC
	#define NOUNIX
#endif

#ifndef NOUNIX
    #define POWL powl
    #define ERFL erfl
    #define TGAMMAL tgammal
    #include <termios.h>
#else
    #define POWL pow
    #define ERFL erf
    #define TGAMMAL tgamma
#endif

/* Hardwired version strings (please, dont' change) */
#define VERSION "1.0.Alpha.geographic"
#define INFOVERSION "1.06000"
#define CY "2016-2021"		  	                  /* Years */
#define AU "Antonio Maschio (tonibin)"		          /* Author */
#define EM "ing dot antonio dot maschio at gmail dot com" /* Email */

/* Hardwired constants (don't change!) */
#define TRUE -1
#define FALSE 0
#define NIL 0
#define CONSOLE 0			// terminal input channel
#define OCONSOLE 1			// terminal output channel
#define MEMLIMIT 15000	 	        // taxi's memory magnitude
#define SCREENLINE 255			// length of input line
#define COMPSCR 256			// length of c-string input line
#define COMPFIL 256			// length of c-string-file-input line
#define LIMIT 999*99			// taxi's entities extension
#define CORELIMIT 2147483647		// taxi's core extension chunk length
#define TABSTOP 8			// tab spacing by default
#define SCRLIM 72 			// screen limit
#define FILESCRLIM 72 			// file line limit
#define ALGSTR 256			// maximum length of a C-string
#define COMPSTR 256			// maximum length of a C-string
#define LABELLIM 999			// Labels limit number in a program
#define INFF 1.7E308		 	// greatest positive real
#define ZERO 1.73E-308	 		// smallest real not zero
#define REALMAX 1.0E16			// maximum (figurative) real
#define REALMIN 1.0E-16 		// miminum (figurative) real
#define LONGREALMAX 1.0E308 		// maximum (figurative) long real
#define LONGREALMIN 1.0E-308 		// miminum (figurative) long real
#define EPSILON 9.9999999999999999E-16  // smallest (figurative) real
#define LONGEPSILON 9.9999999999999999E-307 // smallest (figurative) long real
#define REALOFF 1E-14			// offset real anti-roundoff errors
#define INTMAX 2147483647	 	// greatest positive integer
#define INTMIN -2147483648	 	// greatest negative integer
#define HALFPI  1.570796326795		// pi/2
#define TAXIPI  3.14159265358979323846	// pi
#define FIELD   12			// field for OUTARRAY
#define ZEROCALC 1E-16			// adder value for trig checks
#define PSTACK 1024			// dimension of the inner RPN stack
#define NUMVARS 1024			// chars space for PROCEDURE arguments
#define OSTREAMS 	31		// max number of channels
#define STREAMS 	15		// max number of file channels
#define BSTREAM		2		// first user file channel number
#define SEQ     	100		// sequential file id
#define NOUSE  0			// fresh file
#define VREAD  1			// read/write modes
#define VWRITE 2
#define MAXDIM 16			// max dimensions for arrays dim 1÷15
#define FORIT 999			// max dimensions for FOR items
#define EXIT_STOP -2
#define EXTL ".alg"
#define EXTU ".ALG"
#define NAMEDIM 65			// limit for PROC and var names (64 chars)
#define LOGO "  |              _|\n_  _|         \n  |  __ \\   \\ /   |\n  |   _\\ |   \\    |\n _|  \\___| _/_\\  _|"
#define GREET "Notes:\n- to compile type \'make\' as user in taxi\' source directory.\n- to install type \'make install\' as root after compiling.\n  (but I'm sure you know better than me what to do!)\n\n"
#define WELCOME "\nWelcome, user! I hope you\'ll like this program.\nHere\'s some information about it and its license...\n"
#define SHELLEX "bash"
#define INVCOLOR "[7;37;40m"
#define RESETCOLOR "[1;m"
#define REDCOLOR  "[1;31m"
#define CRC '\n'
#define CRS "\n"
#define EXPCHAR 38			// &=ASCII 38
#define PU -87				// ©=ASCII 169 (=-87)
/* ALGOL var management */
#define LONGREALTYPEBOX 10
#define REALTYPEBOX     20
#define INTEGERTYPEBOX  30
#define BOOLEANTYPEBOX  40
#define STRINGTYPEBOX   50
#define LABELTYPEBOX    60
#define NOTYPEBOX       70
#define DECADD     1000000
#define BYREF          100
#define BYVAL          400
#define CHAR           600
#define ISVAR         1000
#define ISPROC        4000
#define ISARR         8000

/* declare structure */
struct t_declare {
	char name[NAMEDIM];	// identifier
	int type;		// = STRING|INTEGER|REAL|LONGREAL|BOOL|LABEL
	int isArr;		// TRUE/FALSE; name is an array
	int ref;		// value for dereferencing feature
	int dim; 		// type 0=undeclared, 1=vector, 2=matrix, etc..
	int ldim[MAXDIM];	// array dimensions widths
	int offset[MAXDIM];	// offsets for array dimensions, e.g. [-3:5]
	int coeff[MAXDIM];	// coefficient for the position calculus
	int size;		// array global space
	int strmem;		// dimension of string
	int isOwn;		// own variable flag
	char strdata[COMPSTR];	// data for reference array element
	char valdata[COMPSTR];	// data for evaluation
	int refcode;		// code for variable passing in execPROC
	int proccode;		// code of the proc that declared the variable
	int isFunc;		// TRUE/FALSE; a function is implicitly passed 
	int isFict;		// TRUE/FALSE; is a fictitious variable
	int line;		// debug purposes
	int level;		// procedure level for double instances
};
typedef struct t_declare TDeclare;

/* OWN ghost */
struct t_ghost {
	int code;
	double num;
	char str[COMPSTR];
	double *numarr;
	char *strarr;
	int size;
};
typedef struct t_ghost Ghost;

/* DIM structure for numeric arrays */
struct t_array {
	double *elem;		// pointer to the array base
};
typedef struct t_array Tarray;

/* DIM structure for string arrays */
struct ts_array {
	char *elem;		// pointer to the array base
};
typedef struct ts_array TSarray;

/* stream structure for I/O channels */
struct t_iostream {
	/* general parameters */
	FILE * stream;		// file descriptor
	char ioname[COMPSTR];	// file name
	int isopen;		// file open flag
	int type;		// NOUSE | SEQ | BIN
	int fcounter;		// printing position in the file (SEQ)
	int fprintn;		// flag TRUE | FALSE for printing CR
	int pageL;		// line number in page
	int pageN;		// page number (at present unused)
	int margin;		// right margin
	int wayout;		// NOUSE | VREAD | VWRITE
	int iochan;		// IOCHAN code
};
typedef struct t_iostream Tiostream;

/* LABEL structure for the GOTO system */
struct t_label {
	int code;		// code of the label
	int ll;			// line number of the label
	int pl;			// position of command after :
};
typedef struct t_label TLabel;

/* FOR structure pointer */
struct t_forpointer {
	int varpL;
	int varpP;
	int doexL;
	int doexP;
	int termL;
	int termP;
	char cond[COMPSTR]; // for WHILE
};
typedef struct t_forpointer TForP;

/* FOR items structure */
struct t_foritem {
	int type;
	char val[COMPSTR];
	char step[COMPSTR];
	char epos[COMPSTR];
	char cond[COMPSTR];
};
typedef struct t_foritem TForItem;

/* IF structure pointer */
struct t_ifpointer {
	int condL;
	int condP;
	int thenL;
	int thenP;
	int elseL;
	int elseP;
	int termL;
	int termP;
};
typedef struct t_ifpointer TIfP;

/* PROCEDURE structure pointer */
struct t_procpointer {
	int procref;		// procedure dependence
	int procL;		// procedure source references
	int procP;
	int begnL;
	int begnP;
	int termL;
	int termP;
	int numvars;		// procedure number of arguments
	int vars[NUMVARS];	// procedure arguments data
	int number;		// sub number
	char name[NAMEDIM];	// sub name
	char strdata[COMPSTR];	// sub general data
	int type;		// = STR|INTEGER|REAL|LONGREAL|BOOL|LABEL
	double numres;		// result of a numeric sub function
	char strres[COMPSTR];  	// result of a string sub function
};
typedef struct t_procpointer TProcP;
struct rlimit taxirlim={RLIM_INFINITY,-1};
struct rlimit origrlim;

struct t_arg {
    double *s, *e, *r, *epsi;
    int ns;
    char *expr;
};
typedef struct t_arg Targ;

/* Functions prototypes (decidedly don't change!)*/
/* FILE and PRINT statements */
void setupName(char *fn);
/* declare statements */
int dimArray(int var, int T, int *M) ;
int isanystring(char *lp);
double getArrayVal(int var, int t, int *m) ;
void setArrayVal(int var, int t, int *m, double val);
int dimStrArray(int var, int T, int *M) ;
char *getArrayStr(int var, int t, int *m) ;
void setArrayStr(int var, int t, int *m, char *str);
int getBound(int l);
void declare(void) ;
void doPage(int ch);
void array(void) ;
/* ALGOL statements */
double getRandom(void);
void END(int s) ;
void FOR(void);
void EXIT(int m); // wrapper
void WHILE(void);
void IF(void);
void ELSE(void);
void RANDOMIZE(int s);
void READ(int c, int m);
void EMIT(int c, int w);
void INARRAY(void);
void OUTARRAY(void);
/* System parser statements */
int checkProcExec(void);
char *strcasestr_oq(char *pl, char *pr);
char *chrcat_(char *pl, char pr);
char *searchFirst(char *b, char *t1, char *t2);
char *strcasestr_(char *big, char *little);
int strncasecmp_(char *d, char *s, int N);
char *strncpy_(char *dst, char *src, int dw);
char *strncat_(char *dst, char *src, int dw);
int sprintw(char *dest, int len, char *format, ...);
char *SS(void) ;
double S(void) ;
int skip(void);
double getFunc(void);
double getVal(void);
int priority(const int op) ;
int strRel(void);
void optoken(char *q);
/* System functions */
void algolstart(void);
void doBegin(void);
int assign(void) ;
char *execPROC(int subn, char *pc, double *numres, char *strres);
int findPROC(char *p, char **p2, int *t);
int findDEC(char *p);
char *getMonthString(int t) ;
char *getString(int c) ;
char *getISOString(int c) ;
char getChar(int c) ;
int check(char *sb, char *sc);
int exec(void) ;
char *checkProc(char *p, char *t, int l);
void s_exit(int state);
int getDay(void) ;
int getHour(void) ;
int getMin(void) ;
int getMonth(void) ;
int getSec(void) ;
int getYear(void) ;
int sign(double h);
int storeSingleLineToCore(char *line, int l) ;
struct tm *getTime(void) ;
void FCR(int ch) ;
void updatePage(int ch);
void updateCounter(int ch);
void emitChar(int ch, int c);
void doExp(char *pos, double *base, int *exp) ;
void parse(char **m, int s, int e) ;
void preParse(char **m, int s, int e) ;
void stripBlanks(char *s) ;
void sigquit(int sig);
void sigint(int sig);
void sigtrap(int sig);
void *salloc(size_t size);
double *nalloc(size_t size);
int setAddr(char *l, char **p);
int fprintb(char *res, int n, int w, int mode);
void fprinto(char *str, double val, int type);
void memcpy_(void *dst, void *src, int size);
void showPROC(int p, int e);
void showDEC(int decCount, char *d);
char *findLabel(char *pl);
/* running Interface (BASIC fashion ;-) */
void OLD(void) ;
int LOAD(char **m, char *f, int s);
void RUN(void) ;
/* Error messaging system */
void perror_(int line, int errNum, int l) ;
/* Info messaging collection */
void printConditions(void) ;
void printExistence(void);
void printHelp(int h) ;
void printVersion(void) ;
void printHelpBanner(void) ;
void printWarranty(void) ;
int isInnerAssign(char *t);
int isInn=FALSE;
int isCDC=FALSE;
int isDelComm=FALSE;
int isOwn=FALSE;
void PRINT(int ch, int mode) ;
void reload(void);
void go(int l);
int isIn(int l, int p, int j);
int getLabelIndex(void);
int isEOF(int ch);
void setResultType(int n);
/* Inspective macros (imperatively don't change!) 
 * isleq check if p points to a <=
 * isgeq check if p points to a >= 
 * isnm check if entity pointed by p is a literal number
 * ismo checks if p is a math operator
 * isro checks if p is a relational operator
 * isop checks if p is an operator (that is math, relational or logical)
 * islabelnamechar checks if p points to a legal label name
 * isnamechar checks if p points to a legal identifier name
 * isante and ispost check if it is an operator or anything else
 * checkstd checks if h is a standard UNIX channel (stdin, stdout, stderr)
 * pe is a macro for perror_ */
#define ister(p)  ((*(p)=='<' && !*(p+1)))
#define isdc(p)   ((*(p)=='\'' && *(p+1)=='\''))
#define isleq(p)  ((*(p)=='<' && *(p+1)=='='))
#define isgeq(p)  ((*(p)=='>' && *(p+1)=='='))
#define isnm(p) (isdigit(*(p)) || *(p)=='.' || *(p)=='-' || *(p)=='&' || *(p)=='@' || (*(p)=='E' && isdigit(*(p-1))))
#define ismo(p) (*(p)=='+' || *(p)=='-' || *(p)=='*' || *(p)=='/' || *(p)=='^' || *(p)=='\\' || *(p)=='`' )
#define isro(p) (*(p)=='<' || *(p)=='{' || *(p)=='=' || *(p)=='}' || *(p)=='>' || *(p)=='#')
#define islo(p) (*(p)=='_' || *(p)=='|' || *(p)=='\?' || *(p)=='!')
#define isop(p) ((ismo(p)) || (isro(p)) || (islo(p)))
#define islabelnamechar(p) (*(p)=='.' || (isalnum(*(p))))
#define isbeginnamechar(p) ((isalpha(*(p))))
#define isnamechar(p) ((isalnum(*(p))) || *p=='.')
#define isbeginnumberchar(c) (isdigit(c) || (c)=='.' || (c)=='&' || (c)=='@' || (c)=='-' || (c)=='+')
#define isnumberchar(c) (isbeginnumberchar(c) || (c)=='E' || (c)=='D')
#define isante(p) (*((p)-1)==')' || isblank(*((p)-1)))
#define ispost(p) (*((p)+1)=='(' || isblank(*((p)+1)))
#define checkstd(h) (channel[(h)].stream==stdin || channel[(h)].stream==stdout || channel[(h)].stream==stderr)
#define pe(l,c) perror_(__LINE__,(l),(c))
#define isterm(p) (*(p)==';' || *(p)=='.')
#define sysRet  if(LL)return

/* Lotsa global variables! */
/* System variables (Don't change at all!) */
int stopKey=FALSE;
int isQuit=FALSE;
int globalInsertedLines=0;
int typeBox=0;
double nres;
char sres[COMPSTR];
int isLabelAssign=FALSE;
int currentOUTPUT=CONSOLE;
int currentINPUT=CONSOLE;
char filename[COMPSCR];
char line[COMPSCR];
char dline[COMPSTR];
int endCore;
int isDebug=FALSE;
int isVerboseDebug=FALSE;
int isInterrupt=FALSE;
int isNoExecute=FALSE;
int isRunOn=FALSE;
int isPrintSourceCode=FALSE;
int forceArray=FALSE;
int resultType=0;
double saveRand=0;
int expChar=EXPCHAR;
struct stat statbuf;
struct termios modes, savemodes;
double exp_(int t1, double x, int t2, double n);
/* System strings */
static char *month[]=
{"", "JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC" };
static char *cmonth[]=
{"", "JANUARY","FEBRUARY","MARCH","APRIL","MAY","JUNE","JULY","AUGUST","SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER" };
static char *Rtype[]=
{"", "Long real", "Real", "Integer", "Boolean", "String", "Label", "Untyped" };

/* Program flow and var variables
 * l holds the current program line number
 * P is an double array holding variables contents 
 * PS is a character-pointers-array holding references to strings
 * own is the array of OWN variables data
 * ghostCount is the number of OWN variables so far
 * pads are temporary string buffers */
int l;
double P[LIMIT];
char *PS[LIMIT];
Ghost own[LIMIT];
int ghostCount=0;
char sspad[COMPSTR], pad[COMPSTR], *PAD=pad, *MYPAD=sspad;

/* findDEC aid 
 * _len holds complete name length 
 * _rettype hold return type code
 * _arr hold array flag */
int _len,_type,_arr;

/* arrays data management 
 * dim is the numerical array 
 * dimS is the string array */
Tarray dim[LIMIT];
TSarray dimS[LIMIT];

/* Memory system
 * m is an array holding pointers to compact program lines 
 * p is the main char pointer to the current buffer position
 * s and d are various char pointer helpers used into RUN 
 * mb is an array holding pointers to original full program lines
 * BASESTR is the buffer for current program line evaluation
 * ext, st and en keep track of libraries loaded by means of EXTERNAL */
char *m[MEMLIMIT+10], *p,*s,*d;
char *mb[MEMLIMIT+10];
char BASESTR[COMPSTR];
char *ext[EXTERNLIBMAX];
int st[EXTERNLIBMAX],en[EXTERNLIBMAX], libTOS=0;

/* parser variable - TRUE when an operator is expected */
//int opturn=0;	// don't change!

/* Process helpers
 * B is the general input buffer;
 * J is a general purpose buffer used in preParse()
 * F is a character-holder used by the blank stripper 
 * isList is set TRUE by option -l to list the original program and exit 
 * isMetaList is set TRUE by option -m to list compact program and exit
 * isPurge is set TRUE by option --purge to purge ticks from program
 * tabDef holds current tabulation length
 * incrLin and incrementer update line number in error messages */
char B[SCREENLINE+1], J[SCREENLINE+1], F[2];
int isList=FALSE;
int isMetaList=FALSE;
int isPurge=FALSE;
int isVerbose=FALSE;
#define TABDEF 3
int tabDef=TABDEF;
int incrLin[MEMLIMIT+10],incrementer=0;
int totchars=0;

/* declare management
 * vn holds the declare definitions in a proper struct
 * decCount holds current number of declared variables */ 
TDeclare vn[LIMIT];
int decCount=0;

/* PROC management
 * numRes holds numerical SUB return value
 * strRes holds string SUB return value
 * rememberCode and rememberName are used in findPROC as a means to return
 * immediately SUB var code in case it is called a second time afterward */ 
double numRes=0.0;
char strRes[COMPSTR];
int rememberCode;
char rememberName[COMPSTR];

/* FOR-DO management 
 * Forp holds data for jumps
 * forTOS is the pointer to Forp 
 * forExecTOS is to pointer to current FOR cycle - runtime */
TForP Forp[LIMIT];
int forTOS=0;
int forExecTOS=0;

/* WHILE-DO management 
 * Whilep holds data for jumps
 * whileTOS is the pointer to Whilep 
 * whileExecTOS is the pointer to current WHILE cycle - runtime*/
TForP Whilep[LIMIT];
int whileTOS=0;
int whileExecTOS=0;

/* BEGIN-END management 
 * Begin holds position of limits of the BEGIN-END block
 * beginTOS is the pointer to Beginp 
 * beginDepth holds BEGIN data
 * beginExecTOS is the pointer to current block - runtime*/
TForP Beginp[LIMIT];
int beginTOS=0;
int beginDepth[LIMIT];

/* IF THEN management 
 * Ifp holds data for jumps
 * ifTOS is the pointer to Ifp 
 * ifExecTOS is the pointer to current IF structure - runtime */
TIfP Ifp[LIMIT];
int ifTOS=0;
int ifExecTOS=0;

/* PROCEDURES management 
 * Procp holds data of all procedures
 * procTOS is the number of declared procedures 
 * pexec holds data of procedure in runtime
 * pexecTOS is the pointer to current PROCEDURE - runtime
 * isProcTime is a flag to signal a PROCEDURE is executing */
TProcP Procp[LIMIT];
int procTOS=0;
int pexec[LIMIT];
int pexecTOS=0;
int isProcTime=FALSE;

/* Timing variables 
 * tv is used for time calculations 
 * beginS is calculated at start for the TIME function and the timer
 * endS is used to calculate inter-time
 * beginM is the start in milliseconds (for CLOCK)
 * now contains seed for current number generator 
 * isTiming is a flag to write final timing after execution */
struct timeval tv;  
double beginS=-1.0, endS=0.0;
int beginM=0;
time_t now;
int isTiming=FALSE;

/* I/O variables 
 * channel contains the channel 0-15 data (0 = console) and 15-31 logical
 * devices data 
 * bytePos and byteVal hold data of logical channels 
 * rbuff is the INPUT buffer */
Tiostream channel[OSTREAMS+1];		// file channels 1-32
int bytePos[OSTREAMS];
int byteVar[OSTREAMS];
char rbuff[COMPSTR], *rb=rbuff;
int addMainRootBlock=FALSE;

/* virtual printing 
 * isPrinterCH holds data of printing channel, or FALSE if none 
 * PAGEHEIGHT is the default page number of lines
 * prTEMP is the default printer output temporary file */
int isPrinterCH=FALSE;
int PAGEHEIGHT=DEFPAGEHEIGHT;
#define prTEMP "/tmp/taxi_temp"

/* Random number generator algorithm variables 
 * Rand2Seed contains first seed (recalculated each time)
 * randDivi contains the divisor value
 * randSeed contains first seed (recalculated each time) */
int randDivi=32768; // should always be even
int randSeed=7139;

/* LABEL system */
TLabel label[LABELLIM+1];
int labelIndex[MEMLIMIT+10];
int labelTOS=0;
int LL=0;
int PL=0;
int errLINE=-1;
int errCODE=-1;
/* end of taxi.h */

