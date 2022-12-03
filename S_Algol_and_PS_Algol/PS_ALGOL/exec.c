#include <stdio.h>
#include <math.h>
#include <dos.h>
#include "\7layer\layer3\layer3.h"
#include "\7layer\layer4.h"
#include "\7layer\layer5.h"
#include "salib.h"
#include "process.h"
#define REM(a,b) {esp= (int *)(a);context=b;};/* printf("%s\n",b);};*/
static int * esp;
static char * context;
extern short LINENUMBER;
char *MAKEVEC(short,short,short);
/*#define testing 1*/
#define salgol 1
extern PID PIDREG;
extern float REALREG;
extern spawnp();
main(argc,argv)int argc; char ** argv;
{
        PID pid[3],p2;
        int i,j;
		REM(&i,"main");
		if( argc<2){
			printf("usage : psr salgol_exe_file [ params ] \n");
			exit(1);
		}
#ifdef testing
        cmstest();
#endif
#ifndef  testing
        spawnv(P_WAIT,argv[1],argv);
#endif
}
