 /* dynamic linker for psalgol */
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include "link.h"
#define NOP 0x90    /* nop */
#define CALLFAR 0x9a
extern int maxproc;
#define LINK_VEC 0x18

/* first level linker to link to C procs */
	extern struct identifier PROCNAME[];
    extern int far *proctable[];
LINK(es,ds,pc,cs,psf)int es,ds,pc,cs,psf;
{
	 char far * origin ,*name;
	int len,i;
	name= (char *)MK_FP(cs,pc);
    len = strlen(name);
	origin= &(name[-2]);

    origin[0] = NOP;
	origin[1] = CALLFAR;
	if (replace(name)){
	  for(i=4;i<=(len);i++)name [i] = NOP;

	} else
	{
	   printf("linker error can not find %s\n",name);
	   saerror("called by linker");
	}
}
extern  int LINKER();

linkinstall()
{
  /* install the linker using the linker interrupt */
  int far (**iv)();
  (int *)iv =(int *) MK_FP(0,LINK_VEC * 4);
  *iv =  LINKER;
}
int freename;

replace (n)char *n;
{
	int i;
	int **p;
	(char *)p = &n[0];
	for(i=0;i<maxproc;i++){
		if(0==strcmp(n,PROCNAME[i].id)){
			*p = proctable[i];
			return 1;
		 }
	}
	return 0;

}
displayprocs()
{	int i;
	for(i=0;i<10;i++){
		printf("%s\t%x\t%x\n",PROCNAME[i].id,
							  FP_SEG(proctable[i]),
                              FP_OFF(proctable[i]));
	}
 }
 /* ------------------------- Segment table ------------------------- */

#define MAXSEG 16

#define PSP 0x100
#define JUMP 233
static struct segdesc{
	char sname[8];
	char * segbase;
    int used;
} segtab[MAXSEG];
static int topseg=0;
 initsegtab()
 /* initialise segment table */
 {
	 int i;
	 for (i=0;i<MAXSEG;i++)segtab[i].used=0;
	 topseg=MAXSEG;
 }
 static expandname(full,shortn)char * full,*shortn;
 {
	char *getenv();
	strcpy(full,getenv("PSDIR"));
	full = strcat(full,"\\");
	full = strcat(full,shortn);
	full = strcat(full,".out");
 }
 static char * loadseg(char *s)
 {
	char fullname [100];
    unsigned size;
	long ls;
	char * base,*heapalloc();
	int f;
	long tell();
	expandname(fullname,s);
    f = _open(fullname,0);
	if(f == -1 ) {
	   printf("cant open %s\n",fullname);
	   saerror("Segment not found");
	}
	lseek(f,0L,2);
	ls= tell (f);
	if ((ls+PSP)>0x0ffffL)saerror("segment too big");
	size =  PSP + ls;
    base = heapalloc(size);
	lseek(f,0L,0);
	_read(f,&base[PSP],size -PSP);
	_close(f);
	return base;
 }

 static char *findseg(char *s)
 /* look for and load a segment */
 { int i;
   for(i=0;i<topseg;i++)
   if (segtab[i].used)
   {
	  if(strcmp(s,&(segtab[i].sname[0]))== 0 ) return segtab[i].segbase;
   }
   for(i=0;(i<MAXSEG)&&(segtab[i].used);i++);
   if (i == MAXSEG) saerror("too many code segments");
   strcpy(&(segtab[i].sname[0]),s);
   segtab[i].segbase= loadseg(s);
   segtab[i].used=1;
   return(segtab[i].segbase);
 }
 freesegs()
 /* release any booked segments */
 {  int i;
	for (i=1;i<topseg;i++)relseg(i);
 }
 static relseg(i)int i;
 {   if(segtab[i].used)heapfree(segtab[i].segbase); segtab[i].used=0;}
 static matchname(sstring,cstring)char *sstring,*cstring;
 /* assume sstring points at a short containing a string length
	followed by the string */
 {
	int i;
	if (*sstring ==0) return 0;
	for(i=0;i< *sstring ; i++)

	{
	   if (cstring[i] != sstring[i+2]) return 0;
	}
	return 1;
 }

/* second level linker to link to salgol procedures */
 int * dbind(proc,type,segment)char * proc,*type,*segment;
 /*   find the procedure in segment */
 {	char *sb;
	int cursor,start,*jump;
	sb= findseg(segment);
    cursor =PSP;
	/* the segment looks like a com file */
	if( (255 & sb[cursor]) != JUMP) {
	   saerror("wrong magic number in segment");

	}

    jump = (int *) &(sb[++cursor]);
	cursor = cursor + 2+ *jump;
	/* we should now point at a jump instruction just in front of a
	   procedure declaration
	*/
    while((unsigned)(255 & sb[cursor++])==JUMP){
	   jump= (int*)&sb[cursor];

       start = cursor+=2;
	   if(matchname(&sb[cursor],proc))
	   {
		cursor +=( 2 + sb[cursor]); /* move on to the type */
		if(matchname(&sb[cursor],type))
		{
         cursor +=( 2 + sb[cursor]);/* move on to start of proc */
		 /* return procedure start address */

         return MK_FP(FP_SEG(sb),cursor);

		}else saerror("Type mismatch");
	   }
	   cursor= start + *jump;
	}
	printf("\n%s:%s in %s\n",proc,type,segment);
	saerror("procedure not found");

 }
