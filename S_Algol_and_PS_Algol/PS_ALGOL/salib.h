#define maxstring 1000
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)
#define SAFRAME 5

struct sinfo{
		     char pntrs,reals, ints;
			 char len;
			 char name[12];};

struct string{INTEGER len;char c[1];};
struct intvec{INTEGER lower;int i[1];};
struct realvec{INTEGER lower,pad;double r[1];}  ;
struct pidv{INTEGER lower,p1,p2,p3;PID p[1];};
struct saframe{ short padding[SAFRAME] ; };
