/* open a file returns true if it had to create a new one */
static checkopen(s,d,mode) char *s; int mode, *d;
{
    FILE *f;
	if(( *d = _open(s,mode))== -1) {
		if(!(f=fopen(s,"wb+"))){
			cmserror("can not open %s\n",s);
			exit(1);
		}else {fclose(f);return checkopen(s,d)+1 ; }
	} else return 0;
}