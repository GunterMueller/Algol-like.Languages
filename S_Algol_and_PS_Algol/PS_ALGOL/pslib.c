#define DEBUG 1
#include <stdio.h>
#include <math.h>
#include <dos.h>
#include "\7layer\layer3\layer3.h"
#include "\7layer\layer4.h"
#include "\7layer\layer5.h"      
#include "salib.h"
#include "\7layer\layer3\bases.h"
#ifdef DEBUG
#define REM(a,b) {esp= (int *)(a);context=b;};/* printf("%s\n",b);};*/
#endif
#ifndef DEBUG
#define REM(a,b)
#endif
#define SCREEN 1
static int * esp;
static char * context;
short *LINENUMBERLOC;
extern long Hash();
char *MAKEVEC(short,short,short);
/*#define testing 1*/
#define salgol 1
PID *PRL;
#define PIDREG (PRL[0])
#define LINENUMBER (*LINENUMBERLOC)
extern float REALREG;

/* ------------------------ div0err --------------------*/
div0err()
{
        saerror("Divide by zero ");
}
/* ------------------------ defpidreg ------------------*/
/*   called at the begining of the run                  */
#define DIV0 0
int (*divvec)();
DEFPIDREG(short bx,PID *p, short * ln)
{   extern char * stop;
    extern int errorline,(*getvec)();
        PRL=&(p[0]);
    stop=(char *)PRL;
        LINENUMBERLOC=ln;
        errorline = -1;
        opendb("salgol program initialisation");
        initfileio();
        initctab();
        divvec=getvect(DIV0);
        setvect(DIV0,div0err);
        initsegtab();
}
/*-------------------- DEFCSEG ----------------------------------
   this is given the length of the code segment by the ps program */
int codelen;
DEFCSEG(short bx,short len)
{
    codelen=len;
}
TERMINATE()
/* called just before program exits */
{
    /* restore old interrupt vect */
        setvect(DIV0,divvec);
        freesegs();
        layer3commit();
        commit();
}
/*------------------------ dynamic binding ----------------------*/
bind (int bx,PID seg,PID type, PID name)
{
    char cseg[50],ctype[200],cname[200];
        char * dbind();
        char ** res = (char **)&name;
        /* get the strings into buffers */
        scopy(seg,cseg);scopy(type,ctype);scopy(name,cname);
    res[3] = dbind(cname,ctype,cseg);
}
/*------------------- ---- WRITEP ----------------------*/
WRITEP(bx,s,w,p,handle)short bx,w, s,handle;PID p;
{

 SetRoot(&p);
 _write(handle,&p,sizeof(p));
 return 0;
}

Write(int handle,char *p,int size)
{  int i;
   if (handle==SCREEN) { for(i=0;i<size;i++)gputc(p[i]);
   }
   else _write(handle,p,size);
}
/*------------------- ---- WRITEI ----------------------*/
WRITEI(bx,s,w,i,handle)int bx,i,w,s;int handle;
{char buf[100];
 sprintf(buf,"%d",i);
 pad(buf,w);
 Write(handle,buf,strlen(buf));
 spaces(handle,s);
 return 0;
}
pad(s,w)char *s;int w;
{   int spaces,k;
    char c[50];

        spaces=w-(strlen(s));
        if(spaces>0){
                if((spaces+w)>49) return;


        for(k=0;k<spaces;)c[k++]=' ';
        for(k=spaces;k<w;k++)c[k]=s[k-spaces];
                for(k=0;k<w;k++)s[k]=c[k];
                s[k]=0;
        }
}
spaces(handle,number)int handle,number;
{Write(handle,"                                     ",number);
}
/*------------------- ---- WRITEI ----------------------*/
WRITEB(bx,s,w,i,handle)int bx,i,w,s;int handle;
{char buf[100];

 if(i==0){ write(handle,"false",5);} else
 Write(handle,"true ",5);
 spaces(handle,s);
 return 0;
}
/*-----------------------WRITER-----------------------*/
WRITER(bx,s,w,r,handle)int bx,w,s;int handle;double r;
{char buf[100];

 sprintf(buf,"%f",r);
 pad(buf,w);
 Write(handle,buf,strlen(buf));
 spaces(handle,s);
 return 0;
}
/*------------------------WRITES-----------------------*/
WRITES(bx,sw,w,s,handle)int bx,sw,w;int handle; PID s;
{
    int i,j;
        char crlf[2];
        INTEGER UPBOP();
    struct string *strp=(struct string *)whereis(&s);

    i=UPBOP(0,s);
        for(j=0;j<i;j++){
          if(strp->c[j] == '\n') {
                crlf[0]=13;crlf[1]=10;
                Write(handle,crlf,2);
          } else
      Write(handle,&(strp->c[j]),1);
        }
}
/*-------------------------  dispatack ---------------------*/
dispstack(i)int *i;
{          int j;
           for (j=9;j>=0;j--){
               printf("%x %x \n",&i[j], i[j]);
           };
        return(0);
}
/*----------------------MKSDESC--------------------------*/
MKSDESC(short bx, struct sinfo far * tmrec)
{ PID p;
REM(&bx,"MKSDESC")
}
FORMSTRUCT(short bx, struct sinfo far *tmrec,PID result)
{


        extern NewObj();
        extern SetBytes();
                char attr[1];
        long pref,pntrs,size;
                int i;
                PID * pid= &result;

        REM(&bx,"FORMSTRUCT")

                attr[0]=STRUCTURETAG*4;
        size =tmrec->ints* sizeof(INTEGER)+
                          tmrec->reals* sizeof(REAL)+
              sizeof(PID)* tmrec->pntrs;
        NewObj(size,tmrec->pntrs,pid);
            SetAttrib(pid,attr);
        SetPid(0,pid,tmrec);
}
printtmrec(struct sinfo far *tmrec)
{       printf("@%x:%x = %d %d %d %s\n",tmrec,tmrec->ints,
                           tmrec->reals,tmrec->pntrs,tmrec->name);
}
/*------------------------Class id -------------------------*/
classid(struct saframe pad, PID result)
{
    struct sinfo tm;
        int i;
        PID *pid = &result;
        GetPid(0,pid,(PID*)&tm);
        mkstr(&(tm.name[0]),pid,tm.len);
}

/*------------------------WHERESTRUCT-----------------------*/
char huge *WHERESTRUCT(short bx,PID far *pntr)
/* return address of a structure */
{
   REM(&bx,"WHERESTRUCT");return  whereis(pntr); }
/*------------------------VALIDSTRUCT-----------------------*/
char far *VALIDSTRUCT(short bx, struct sinfo far *tmrec,PID far *pntr)
{   struct sdesc *p1,*p2,s1,s2;
        /* checktype of struct and return start address */
        PID *p= (PID*)whereis(pntr);
        REM(&bx,"VALIDSTRUCT");
        /* first field of structure is trademark */
        if(eqpid(tmrec, p)) return (char *)(pntr= p);
        saerror("structure class violation");
}
/*-------------------ISOP-----------------------------*/
ISOP(short bx, struct sinfo far *tmrec,  PID pntr)
{   struct sdesc *p1,*p2,s1,s2;
        /* checktype of struct and return start address */
    PID *p;
        REM(&bx,"ISOP");
        p= (PID*)whereis(&pntr);
        /* first field of structure is trademark */
        if(eqpid((PID*)tmrec ,p)) return 1;
        return 0;
}
/*----------------------COMPPNTR--------------------------*/
COMPPNTR(bx,p1,p2)PID p1,p2;int bx;
{
    if( eqpid(&p1,&p2)) return 0;return 1;
}
mkstr(a,s,len)char *a;PID *s;int len;
{
        ARRAY(1,len,sizeof(char),s);
    SetBytes(len,4,s,a);
}
char firstglobal;
static PID ctab[128];
static char ctabflag[128];
char lastglobal;
 initctab()
{    int i;
         char c[2];
         for (i=0;i<128;i++)ctabflag[i]=0;
}
onechrstr(c,p)char c;PID *p;
{   char cc[2];
    if (ctabflag[c]){ *p=ctab[c];}
        else
        {
    cc[1]='\0';
        cc[0]=c;
        mkstr(cc,p,1);
        ctab[c]= *p;
        ctabflag[c]=1;
        }
}
/****-------------------LLSTRING-------------------------*/
LLSTRING(bx,strp)short bx,*strp;
{   int len,i,u; char *src;struct string *dest;PID p;
    REM(&bx,"LLSTRING");
        ARRAY(1,*strp,sizeof(char),&p);
                PIDREG=p;
        len=*strp++;dest=(struct string *) whereis(&p);
        blockmove(strp,dest->c,len);


}
/*----------------------SUBSTROP---------------------------*/
SUBSTROP(bx,newlen,index,s)PID s; int bx,newlen,index;
{
    int l1,esize,start,bsize;
    char *buf,*heapalloc();
        REM(&bx,"slice");
        esize=ElemSize(&s);
        bsize = newlen*esize +32;
        buf=heapalloc(bsize);
    l1 = index -LWBOP(0,s);
        if (l1<0) saerror("bounds error");
        start=(esize<sizeof(INTEGER) ? sizeof(INTEGER) :esize);
    GetBytes(newlen*esize,start +(l1*esize),&s,buf);
                         /* move to fixed pos incase a garbage collect happens*/
    ARRAY(1,newlen,esize,&s);   /* s1 now overwritten with new obj */
    SetBytes(newlen*esize,start,&s,buf);
        heapfree(buf);
}

/*----------------------COMPSTR--------------------------*/
COMPSTR (bx,s2,s1)PID s2,s1;int bx;
{
    char a[maxstring],b[maxstring];
        REM(&bx,"COMPSTR");
    scopy(s1,a);scopy(s2,b);

    return strcmp(a,b);
}
/*-------------------- scopy ----------------------------
                       copy a string to a buffer
*/
scopy(s,b)PID s; char *b;
{
      int l;
      l=ObjSize(&s)-sizeof(INTEGER);
      if (l>=maxstring) l=maxstring-1;
      blockmove(((struct string *)whereis(&s))->c,b,l);
      b[l] = '\0';
}
/*----------------------CONCAT---------------------------*/
CONCAT(bx,s2,s1)int bx;PID s1,s2;         /* concat two vectors */
{
    int newlen,l1,l2,esize,start,bsize;
    char *buf,*p,*heapalloc();
    char * dest;
        REM(&bx,"CONCAT");
    newlen=(l1=arraylen(&s1))+(l2=arraylen(&s2));
        esize=ElemSize(&s2);
        bsize = newlen*esize +32;
        buf=heapalloc(bsize);
        start=(esize<sizeof(INTEGER) ? sizeof(INTEGER) :esize);
    p=whereis(&s1);
    blockmove(&p[start],buf,l1*esize);
    p=whereis(&s2);
    blockmove(&p[start],&(buf[l1*esize]),l2*esize);

                         /* move to fixed pos incase a garbage collect happens*/

    ARRAY(1,newlen,esize,&s1);   /* s1 now overwritten with new obj */
    dest=(struct string *)whereis(&s1);
        blockmove(buf,&dest[start],esize*newlen);
        heapfree(buf);
}
/*----------------------ILIFFE------------------------*/
/*   these only deal with 1 d arrays now */
int *ILIFFEINT(bx,dimensions,tag,init,upb,lwb)
int tag,bx, dimensions,init,upb,lwb;
{
 int length;
 register int *p;
    length =1+upb -lwb;
    ARRAY(lwb,length,sizeof(int),&PIDREG);
    p= &((int*)whereis(&PIDREG))[1];
    for(;lwb<=upb;lwb++)*++p=init;
        return(p);
 }
double* ILIFFEREAL(bx,dimensions,tag,init,upb,lwb)
int tag,bx, dimensions,upb,lwb;
double init;
{
 int length,i;
 register struct realvec *d;
    length =1+upb -lwb;
    ARRAY(lwb,length,sizeof(double),&PIDREG);
    d= ((struct realvec*)whereis(&PIDREG));
    for(i=0;lwb<=upb;lwb++,i++){d->r[i]=init;};
        return((double *)d);
 }
ILIFFEPNTR(bx,dimensions,tag,init,upb,lwb)
int tag,bx, dimensions,upb,lwb;
PID init;
{
 int length;
 struct pidv  *vec;
 register PID *pid;
 PID p;
        REM(&bx,"ILIFFEPNTR");
    length =1+upb -lwb;
    ARRAY(lwb,length,sizeof(PID),&PIDREG);
    vec= ((struct pidv*)whereis(&PIDREG));
    pid=(PID *)   &vec->p;
    for(;lwb<=upb;lwb++){*pid++ =init;}
 }


/*----------------------------SUBVASSIB--------------------------*/
SUBVASSIB(bx,rvalue,offset,pntr)
short bx,offset,rvalue;
PID pntr;
{
  int *a;
  int size,i;
  size = ObjSize(&pntr);
  a = ( int *)(whereis(&pntr));
  offset = offset - *a + 2;
  if(offset>=2)
  if(offset*sizeof(short) < (size) ){
     a[offset]=rvalue;
     return ;
  };

  bounds(offset,*a);return ;
}


/*----------------------------SUBVIB--------------------------*/
SUBVIB(bx,offset,pntr)
short bx,offset;
PID pntr;
{
  int *res;
  register char *cres;
  int *a;
  int size,i;
  int disp = sizeof(PID) - sizeof(int);
  REM(&bx,"SUBVIB");
  cres =(char *) &pntr;
  res =(int *) &(cres[ disp]);
  size = ObjSize(&pntr);
  a = ( int *)(whereis(&pntr));
  offset = offset - *a + 2;
  if(offset>=2)
  if(offset*sizeof(short) < (size) ){
     *res =a[offset];
     return ;
  };
        bounds(offset,*a);return ;

}

/*----------------------------SUBVASSR--------------------------*/
SUBVASSR(bx,rvalue,offset,pntr)
short bx,offset;float rvalue;
PID pntr;
{
  struct realvec *a;
  int size,i;
  size = ObjSize(&pntr);
  a = ( struct realvec *)(whereis(&pntr));
  if (offset < a->lower)bounds(offset,a->lower);
  offset = offset - a->lower +1;
  if(offset*sizeof(float) < size ){
     a->r[offset-1]=rvalue;
     return 0 ;
  };
  bounds(offset -1+a->lower,size / sizeof(float) - 1);
}
bounds(index,limit) int index,limit;
{
  if(index < limit )
  printf("\n%d < %d\n",index,limit);
  else   printf("\n%d > %d\n",index,limit);
  saerror("array bounds fault");return 0;
}


/*----------------------------SUBVR--------------------------*/
SUBVR(bx,offset,pntr)
short bx,offset;
PID pntr;
{
  struct realvec *a;
  double *res;
  register char *cres;
  int size,i;
  int disp = sizeof(PID) - sizeof(double);
  cres =(char *) &pntr;
  res =(double *) &(cres[ disp]);
  size = ObjSize(&pntr);
  a = ( struct realvec *)(whereis(&pntr));
  if (offset < a->lower)bounds(offset,a->lower);
  offset = offset - a->lower + 1;        /* adjust for lower bound */
  if(offset*sizeof(float) < size ){
     *res= a->r[offset-1];
  } else
  bounds(offset+a->lower-1,size / sizeof(float) -1);
}


/*----------------------------SUBVASSP--------------------------*/
INTEGER *SUBVASSP(bx,rvalue,offset,pntr)
short bx,offset;
PID pntr,rvalue;
{
  INTEGER *a;
  int size,i;
  size = ObjSize(&pntr);
  a = ( INTEGER *)(whereis(&pntr));
  offset = offset - *a + 1;
  if(offset >=1)
  if(offset*sizeof(PID) < size ){
     ((PID *)a)[offset]=rvalue;
     return a ;
  };

  bounds(offset + *a-1,*a);return 0;
}


/*----------------------------SUBVP--------------------------*/
INTEGER *SUBVP(bx,offset,pntr)
short bx,offset;
PID pntr;
{
  register INTEGER *a;
  PID *p;
  int size,i;
  REM(&bx,"SUBVP");
  size = ObjSize(&pntr);
  a = ( INTEGER *)(whereis(&pntr));
  offset = offset - *a + 1;
  if(offset>=1)
  if(offset*sizeof(PID) < size ){
     p= (PID *)a;
     pntr= p[offset];return a;
  };

  bounds(offset + *a -1,*a);return 0;
}


/*----------------------------LWBOP--------------------------*/
LWBOP(bx,pntr)
short bx;
PID pntr;
{    int l= *( INTEGER *)(whereis(&pntr));
     return(l)    ;
}


/*----------------------------UPBOP--------------------------*/
INTEGER UPBOP(bx,pntr)
short bx;
PID pntr;
{
  int size,esize =ElemSize(&pntr);
  size = ObjSize(&pntr);
  if (esize <4){ size = size-4; } else size -= esize;
  return (size/ esize) + (LWBOP(0,pntr))-1;
}

/* arraylen */
arraylen(PID *a)
{ return(UPBOP(0,*a)+1-LWBOP(0,*a)); }
/*----------------------------- saerror -------------------------*/
saerror(s) char * s;
{extern int errorline;
 fprintf(stderr,
   "\nPS-algol Run time library error:\n\t %s\nline %d\n",s,LINENUMBER);
#ifdef DEBUG
 fprintf(stderr,"%s %x\n",context,esp);

#endif
 if(errorline !=LINENUMBER){
 errorline=LINENUMBER;
 TERMINATE();

 }
 PSABORT(1);
 return 0;
}
abortps(bx)int bx;
{
 saerror("abort called");
}
/*----------------------------- Returncode -----------------------------*/
Returncode()
{ extern int errorline;
  return errorline; }

/*----------------------------- trace ----------------------------------*/
TRACE(int bx)
{
#define STEP 1
#define BREAK 2
#define RUN 0
     static int mode = STEP;
         static unsigned breaklo =0;
         static unsigned breakhi =0;
         int * sp,i,j;
         char l[80];
                printf("Trace @%d\n",LINENUMBER);
         if(mode == RUN)return;
         if(mode == BREAK)
                if(breaklo<=LINENUMBER && breakhi>=LINENUMBER) mode = STEP;

         if(mode==STEP){
           while(1){
                printf("Debugger at line @%d>",LINENUMBER);
                scanf("%s",l);
                if(l[0]){
                        if(l[0]=='r'){
                                mode=RUN; return;
                                }
                         else if('n'== *l) {return;}
                         else if('q'== *l) {PSABORT(0);}
                         else if(*l =='s'){
                  sp= &bx;
                                  for(i=0;i<10;i++){
                                         printf("%x  :",&sp[i*8]);
                                     for(j=0;j<8;j++)printf("%x\t",sp[i*8+j]);
                                         printf("\n");
                  }
                         }
                         else if(l[0]=='b'){
                                scanf("%d %d",&i,&j);
                                breaklo=i;breakhi=j;
                                mode = BREAK;return;
                         }
                         else if(l[0]=='h'||l[0]=='?'){
                                printf("Trace options:\nr\tRUN\nb <lo>,<hi>\tbreak point");
                                printf(" on any line between lo and hi\ns\tstackdump\n");
                                printf("n\texecute next line\nq\tquit\n");
                         }
                }
          }
        }

 }


/*-----------------------------MAKEVEC----------------------------------*/
char *MAKEVEC(bx,len,typecode) /* return address to retract stack to */
int len,typecode,bx;
{ /* on entry the length of the array is in len
     and below it is the typecode  of the elements to
         go in the array.
         Above type code are len elements followed by the lower bound
         */
     int size= typelength(typecode)*len ; /* size holds number of bytes on stack
                                          taken up by initial values */
     INTEGER lwb,length,tag;
     short *sa,*sv;
     long *la,*lv;
     double *da,*dv;
     PID *pa,*pv;
         char * v,*s =(char *)&typecode;
         s= &(s[size + sizeof(typecode)]) ;
                                /* address of lwb on stack*/

     lwb = *(int *)s;
     length = len;
     tag = typecode;
         ARRAY(lwb,length,tag,&PIDREG);
         /* we now have an blank array */

     v = (char *)whereis(&PIDREG)+
         MAX( typelength(typecode) ,sizeof(INTEGER));
     s = s - typelength(typecode);
     if(tag==sizeof(char)){
           for (;len;len--)*v++= *s--;
     }else if(typelength(tag) == sizeof(short)){
           sa = (short *)s;
           sv=(short*) v;
           for (;len;len--)*(sv++) = *(sa --);
     } else if(typelength(tag) == sizeof(long)) {

            la = (long *)s;
            lv = (long *) v;
            for(;size;size -= typelength(typecode)) *(lv++) = *(la --);
     } else if(typelength(tag) == sizeof(double)) {
            da = (double *)s;
            dv = (double *) v;
            for(;size;size -= typelength(typecode)) *(dv++) = *(da --);
     } else {
            pa = (PID *)s;
            pv = (PID *) v;
            for(;size;size -= typelength(typecode)) *(pv++) = *(pa --);
     };
  return ( (char *) v);
  }

/*
-----------------   real arithmetic ----------------------------
*/
FLOAT_LOWER(bx,i,j,k,topreal,lowerint)int bx,i,j,k,lowerint;double topreal;
{ double r1,*realpair;  r1=topreal; realpair=(double *)&i;
  realpair[0]=r1;realpair[1]=lowerint;}

FLOAT_TOP(bx,i,j,k,lowerint)int bx,i,j,k,lowerint;
{ double *realpair;   realpair=(double *)&i;realpair[0]=lowerint;}
PLUSR(bx,r2,r1)int bx;double r1,r2;
{r1 += r2; }
NEGOPR(bx,r1)int bx;double r1;
{ r1 = -r1; }
TIMESR(bx,r2,r1)int bx;double r1,r2;
{r1 *= r2; }
MINUSR(bx,r2,r1)int bx;double r1,r2;
{r1 -= r2; }
DIVR(bx,r2,r1)int bx;double r1,r2;
{

   if (r2==0) saerror("Real divide by zero");
    r1 /= r2;
 }
COMPOPR(bx,r2,r1)int bx; double r1,r2;
{ if(r1==r2)return 0;
  if(r1 <r2)return -1;
  return 1;
}
FLOAT(int bx,union{ int t1,t2,t3,val;double r;}p)
{
        p.r = 1.0*p.val;
}
/************************--   test  -- ************************/
/*************************** Table Functions **********************/
/* derived from SALGOL source                                                                     */
/* As Follows:
! Include file to provide persistence support
structure Table(* *pntr objects; int card)
! assert the number of entries in the table = card
! assert the number of pointers in the objects table > card
! assert the object pointers point at either nil or an Tabent

structure Tabent(string Tabkey;int Tabhash; pntr Tabentry,Tabnext)
! assert Tabkey is the key associated with entry
! assert only one Tabent in whole table with given key
! assert Tabnext points at nil or a Tabent

procedure new.Vect(int low,upper->**pntr);vector 0::low,0::upper of nil

procedure table(->pntr)
    {
    let v= new.Vect(1,1)
    let t= Table(v,0)
    t
    }
let test=table()

procedure cardinality(pntr p ->int); p(card)

procedure s.enter(string key; pntr table, value)
begin
    let obj = table (objects )
    let u1 = upb(obj)
    let l2 = obj(0)
    let u2 = upb(l2)
    let hash = proc ( string p -> int ) ; alien "_HASH"
    let hkey = hash(key)
    let top = hkey rem u1
    let bottom = (hkey div u1) rem u2
    procedure subs(pntr p ->pntr)
    if p = nil then
                {
                table(card) := table(card) +1
                Tabent(key,hkey, value,nil)
                }
             else if p(Tabkey)=key then
                  {
                  p(Tabentry) := value
                  if value = nil then nil else p
                  }
             else
                 {
                 p(Tabnext):= subs(p(Tabnext))
                 p
                 }
    let level1 = obj(top)
    level1(bottom) := subs( level1( bottom ) )
    if u1*u2 < table( card) do begin
        ! double the size of the hash table

         table(objects) := new.Vect( if u1=u2 then  2* u1 else u1,
                                     if u1=u2 then u2 else 2*u2)
         table(card):=0
         for i = 0 to u1 do
             {
             let level2 := obj(i)
             for j= 0 to u2 do
                 {
                 let list := level2(j)
                 while list ~= nil do
                       {
                       s.enter(list(Tabkey),table,list(Tabentry))
                       list := list(Tabnext)
                       }
                 }
            }
    end

end

procedure s.lookup(string key; pntr table -> pntr)
begin
    let res:=nil
    {
    let hash = proc ( string p -> int ) ; alien "_HASH"
    let hkey = hash(key)
    let obj = table (objects )
    let u1 = upb(obj)
    let u2 = upb(obj(0))
    let top = hkey rem u1
    let bottom = (hkey div u1) rem u2
         procedure find(pntr p ->pntr)
             if p = nil then p
             else if p(Tabkey)=key then p(Tabentry)
             else find(p(Tabnext))
    res:=find( obj( top,bottom ) )
    }
    res
end

----------------------------------- END OF SALGOL COMMENT */


struct Table{struct sinfo trademark;PID objects,defaultval; INTEGER card;};
struct sinfo tabletm={'\3','\0','\1','\12',"Table(**pnt"};
char tabattr[1]={STRUCTURETAG*4};
/*
! assert the number of entries in the table = card
! assert the number of pointers in the objects table > card
! assert the object pointers point at either nil or an Tabent
*/
union any{
        PID pval;
        float rval[2];
        int ival[8];
        }    ;
struct Tabent{struct sinfo Trademark;PID Tabkey,Tabnext;union any Tabentry;long Tabhash;};
struct sinfo Tabenttm={4,0,1,12,"Tabent(stri"};        /*
! assert Tabkey is the key associated with entry
! assert only one Tabent in whole table with given key
! assert Tabnext points at nil or a Tabent
*/
new_Vect(int low,int upper,PID *result)
{
/* same effect as salgol : vector 0::low,0::upper of nil*/
    int i;
        PID v1,v2;
        REM(&low,"new_Vect");
    ILIFFEPNTR(0,1,sizeof(PID),v1,low,0);
    v1=PIDREG;
    for (i=0;i<=low;i++)
        {
        ILIFFEPNTR(0,1,sizeof(PID),NILPID,upper,0);
                REM(whereis(&PIDREG),"newvect 2");
        SUBVASSP(0,PIDREG,i,v1);
     }
         *result = v1;
 }

table(PID *pntr)
    {
    PID  p,v;
        struct Table *Tp;
        new_Vect(1,1,&v);
        REM(pntr,"table");
        NewObj(sizeof(struct Table),1,&p);
        Tp= (struct Table *)whereis(&p);
		Tp->trademark=tabletm;
		tabattr[0]=STRUCTURETAG*4;
		SetAttrib(&p,tabattr);

        Tp->card=0;Tp->objects= v; Tp->defaultval= *pntr;*pntr =p;
    }
/*-------------------------MCKTAB--------------------------------*/
MCKTAB(bx,pntr)
short bx;
PID pntr;
{ table (&pntr); }
/*----------------------------CARDOP--------------------------*/
INTEGER CARDOP(bx,pntr)
short bx;
PID pntr;
{ struct Table *Tp=(struct Table *)whereis(&pntr);
  return Tp->card;
}

subs(PID p,PID *table,PID *key,INTEGER hkey,PID *value, PID *new )
{
   register struct Tabent *Tp;
   struct Table *Tpp;
   PID  temp;
   REM(new,"subs");
   if (eqpid(&p,& NILPID))
                {
                    Tpp=(struct Table *)whereis(table);
                ++Tpp->card;
                NewObj(sizeof(struct Tabent),3,new);
				SetAttrib(new,tabattr);
                Tp=(struct Tabent *)whereis(new);
                                Tp->Tabkey= *key;
                                Tp->Tabhash=hkey;
                                Tp->Tabentry.pval= *value ;
                                Tp->Tabnext=NILPID;
								Tp->Trademark=Tabenttm;

                }
    else {
                    Tp=(struct Tabent *)whereis(&p);

                                  if (0==COMPSTR(0,Tp->Tabkey,*key))
                  {
                         Tp=(struct Tabent *)whereis(&p);
                     Tp->Tabentry.pval= *value;
                     if (eqpid(value ,&NILPID)) {*new = Tp->Tabnext;} else
                                     {*new = p ;}
                  }
                  else
                  {

                          Tp=(struct Tabent *)whereis(&p);
                      subs( Tp->Tabnext,table,key,hkey,value,&temp);
                          Tp=(struct Tabent *)whereis(&p);
                      Tp->Tabnext= temp;
                      *new =p;
                 }
                 }
  }
/* ---------------- generic table insertion ------------------*/
TAB_INSP(int bx,PID value,       PID key,       PID table)
{
    struct Table *Tp ;
        struct Tabent *Te;
        PID obj,l2,level1,list,level2,newobj,*p;
        int u1,u2,i,j;
        INTEGER *ob1,*ob2,hkey,top,bottom,tcard;
        /* find appropriate hash chain */
        REM(&bx,"start TAB_INSP");
        Tp=(struct Table *)whereis(&table);
    obj = Tp->objects ;
        tcard = Tp->card;
        REM(&bx," TAB_INSP 2");
    ob1 = (INTEGER *)whereis(&obj);
        u1 = UPBOP(0,obj);
        l2 = ((PID *)ob1)[1];
        REM(&bx," TAB_INSP 3");
        u2= UPBOP(0,l2);
        REM(&bx,"TAB_INSP 3.5");
    hkey =Hash(key);
    top = hkey % u1;
    bottom = (hkey / u1) % u2 ;
        REM(&bx," TAB_INSP 4");
        p = (PID *)whereis(&obj);
    level1 = p[top+1];
        /* put item on list at bottom of queue */
        REM(&bx," TAB_INSP 5");
        subs(((PID *)whereis(&level1))[bottom+1],&table,&key,hkey,&value,&l2);
        /* l2 now contains the new hash chain pointer */
        REM(&bx," TAB_INSP 6");
        ((PID *)whereis(&level1))[bottom+1] = l2;
    if (u1*u2 < tcard ){
        /* double the size of the hash table*/
         new_Vect( ( u1==u2 ?  2* u1 : u1),
                   ( u1==u2 ? u2 : 2*u2) ,
                                         &newobj);

             REM(&bx," TAB_INSP 7");
             Tp=(struct Table *)whereis(&table);
                 Tp->objects=newobj;
                 Tp->card = 0;
                 /* we now have an empty table */
         for (i = 0 ; i<=u1 ;i++)
             {
                 REM(&bx," TAB_INSP 8");
             level2 = ((PID *)whereis(&obj))[i+1];
             for(j= 0 ;j<=u2 ;j++)
                 {
                     REM(&bx," TAB_INSP 9");
                 list =  ((PID *)whereis(&level2))[j+1];
                 while (!eqpid(&list ,& NILPID) )
                       {
                           REM(&bx," TAB_INSP 10");
                                           Te=(struct Tabent *)whereis(&list);
                                           list=Te->Tabnext;
                       TAB_INSP(0,Te->Tabentry.pval,Te->Tabkey,table);
                       }
                 }
            }
    }

}
/* ---------------- generic table initialisation -------------*/
INITTAB(int bx,PID table, PID value,PID key)
/* stack on entry
       key
           value
           table
           bx
   on exit
           table
           discard
           discard
           bx
           */
{
    REM(&bx,"INITTAB");
        TAB_INSP(bx,value,key,table);
        key=table;
}
/* -------------------- Table Insertion by type ------------------------*/
TAB_INSI(short bx,short i, PID key,PID table)
{
        union any v;
        v.ival[7]=i;
        TAB_INSP(bx,v.pval,key,table)
        ;
}
TAB_INSR(short bx,float i, PID key,PID table)
{
        union any v;
        v.rval[1]=i;
        TAB_INSP(bx,v.pval,key,table);
}
find(PID p,PID * key, PID *pntr,PID * thetab)
{
  struct Tabent *Tp;
  struct Table *tabp;
             if (eqpid(&p ,& NILPID)){
                         /* not found at end of list return default */
                             tabp= (struct Table *)whereis(thetab);
                                 *pntr = tabp->defaultval;
                         }
             else {
                                Tp=(struct Tabent *)whereis(&p);
                                if(0==COMPSTR( 0,Tp->Tabkey,*key)){
                                /* found - return value */
                                   Tp=(struct Tabent *)whereis(&p);
                                   *pntr = Tp->Tabentry.pval;
                                }
                else{
                                   /* try rest of list */
                                   Tp=(struct Tabent *)whereis(&p);
                                   find(Tp->Tabnext,key,pntr,thetab);
                                }
                         }
}
/*  ----------------------- Generic lookup Routine ------------------------ */
S_LOOKUP(short bx,PID key, PID table )
    {
        struct Tabent *Te;
        PID obj,l2,level1,newobj;
        INTEGER *ob1,*ob2,hkey,top,bottom;
        int u1,u2;
        /* find appropriate hash chain */
        struct Table *Tp;
        Tp=(struct Table *)whereis(&table);
    obj = Tp->objects ;
    ob1 = (INTEGER *)whereis(&obj);
        u1 = UPBOP(0,obj);
        l2 = ((PID *)ob1)[1];
        REM(&bx," slookup 2");
        u2= UPBOP(0,l2);
    hkey =Hash(key);
    top = hkey % u1;
    bottom = (hkey / u1) % u2 ;
    level1 = ((PID *)whereis(&obj))[top+1];
        /* find item on appropriate list */
        l2 = (((PID *)whereis(&level1))[bottom+1]);
        REM(&bx,"before find");
    find( l2,&key,&table,&table);
        REM(&bx,"end of s lookup");
    }
