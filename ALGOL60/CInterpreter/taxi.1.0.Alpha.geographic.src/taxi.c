/* taxi.c 
 * Copyleft (C) 2017 Antonio Maschio 
 * @ <ing dot antonio dot maschio at gmail dot com>
 * Update: 04 gennaio 2021
 * 
 * This is taxi, an Algol interpreter written in c99 for the UNIX world.
 * You must have at least gcc 3.X to compile (though version 2.9X may suffice).
 * 
 * This program is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the Free 
 * Software Foundation, either version 3 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with 
 * this program. If not, see <http:/www.gnu.org/licenses/>.
 *
 * It's GPL! Enjoy!
 */
#include "taxi.h"	// catch-all header file


/***************************************************************************
 * PAUSE procedure
 ***************************************************************************/

/* PAUSE()
 * exit to the shell, to allow execution of task out of the program,
 * to be continued by typing exit */
void PAUSE(void) {
	fprintf(stdout,"%s\n","Paused. Type 'exit' to restore.");
	system(SHELLEX);
	fprintf(stdout,"%s\n","Restoring the program.");
}


/***************************************************************************
 * INARRAY and OUTARRAY procedures
 ***************************************************************************/

/* doprint()
 * print the number r with 'field' digits (from the total field expunge
 * the staring sign or space, the dot and the final space, which totals 3).
 * 'field' includes the E+00 figure of the exponential form, that reduces
 * the number of digits after the dot correspondingly 
 * ch is the output channel
 * System function for the OUTARRAY procedure */
void doprint(double r, int fld, int ch) {
	char temp[COMPSTR], output[COMPSTR];
	char format[COMPSTR];
	char minus=' ';
	int field=fld-3; // expunge the three characters
	int A=field-(int)(log(fabsl(r))/log(10))-1;
	int B=field-5;

	/* remove sign */
	if (r<0.0) r=-r, minus='-';

	/* check for zero 
	 * Note: values too near to zero cause A to be incorretly set
	 * (because the logarithm of zero is infinite), and since 
	 * sometimes real numbers are returned as -0.0 (something that 
	 * gets me crazy) I reset the value to zero and use fabs() 
	 * even if the number should be positive */
	if (fabsl(r)<LONGREALMIN) r=0, A=field-1, minus=' ';
	if (A<0) A=0;

	/* print temporarily with free format */
	sprintw(temp,COMPSTR,"%g",r);

	/* Note: %g prints a lower 'e' in case it prints in exponential;
	 * so delegate %g to decide whether printing in exponential
	 * or normal floating-point mode */
	/* print in exponential form */ 
	if (strchr(temp,'e')) sprintw(format,COMPSTR,"%%c%%.%de ",B);
	/* print in normal floating-point form */
	else sprintw(format,COMPSTR,"%%c%%%d.%df ",field,A);

	/* format number in output string and print it */
	sprintw(output,COMPSTR,format,minus,r);
	fprintf(channel[ch].stream,"%s",output);
}


/* update()
 * update index count of the whole indices of the array
 * cs: array code
 * id: current dim
 * val: the array indices (see get & setArrayVal())  
 * System function for the OUTARRAY procedure */
int update(int cs, int id, int*val) {
	/* update current dim */
	val[id]++;
	/* if update is beyond the upper bound dimension, update lower */
	if (val[id]>vn[cs].offset[id]+vn[cs].ldim[id]) {
		if (id>1) {
			val[id]=vn[cs].offset[id];
			update(cs,id-1,val);
		}
		else {
			return FALSE;
		}
	}
	return TRUE;
}


/* OUTARRAY ()
 * Not DEC nor ISO
 * execute the outarray(ch, ar) procedure
 * ch: the channel to which array must be printed
 * ar: the array code */
void OUTARRAY(void) {
	int ch=0, cs, dim, field=FIELD, lim=1;
	int counter=0;
	double re=0.0;
	int val[MAXDIM+1];
	int bfa=forceArray;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(59,l);
	}

	/* get array code */
	if (*p==',') p++;
	else pe(2,l);
	forceArray=TRUE;
	cs=findDEC(p);
	forceArray=bfa;
	if (!cs) pe(70,l);
	if (!_arr) pe(70,l);
	if (_type>=STRINGTYPEBOX) pe(87,l);
	p+=_len;

	/* conduct to basic referenced item */
	while (cs!=vn[cs].ref) cs=vn[cs].ref;

	/* calculate limit of the base matrix */
	if (vn[cs].dim>=1) {
		int di=vn[cs].dim;
		lim=(vn[cs].ldim[di]+1)*(vn[cs].ldim[di-1]+1);
	}

	/* get optional width field */
	if (*p==',') {
		p++;
		field=(int)S();
		sysRet;
		if (field<2) field=2;
	}
	
	/* store dimension */
	dim=vn[cs].dim;

	fputc('\n',channel[ch].stream);
	/* vectors are printed vertically */
	if (dim<2) {
		int i;
		for (i=vn[cs].offset[1];i<=vn[cs].ldim[1]+vn[cs].offset[1];i++){
			val[1]=i;
			/* numeric vectors */
			doprint(getArrayVal(cs,1,val),field,ch);
			fputc('\n',channel[ch].stream);
		}
	}

	/* multidimensional arrays are printed sequentially 
	 * by matrices groups */
	else {
		int n;
		/* set reference array to lowers indices */
		for (n=1; n<=dim-1; n++) val[n]=vn[cs].offset[n];
		val[dim]=vn[cs].offset[n]-1;

		/* get value and repeat for all */
		while (update(cs,dim,val)) {
			/* if current index is reset, check end of whole loop */
			if (val[dim]==vn[cs].offset[dim]) {
				/* stop when the first dimension is over */
				if (val[1]>vn[cs].offset[1]+vn[cs].ldim[1]) {
					break;
				}
				else {
					fputc('\n',channel[ch].stream);
				}
			}
			if (counter>=lim) {
				fputc('\n',channel[ch].stream);
				counter=0;
			}
			/* numeric arrays */
			re=getArrayVal(cs,dim,val);
			doprint(re,field,ch);
			counter++;
		}
	}
	fputc('\n',channel[ch].stream);

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);
}


/* inUpdate()
 * update index count of the whole indices of the array
 * cs: array code
 * id: current dim
 * val: the array indices (see get & setArrayVal())  
 * System function for the INARRAY procedure */
int inUpdate(int cs, int id, int*val) {
	/* update current dim */
	val[id]++;
	/* if update is beyond the upper bound dimension, update lower */
	if (val[id]>vn[cs].offset[id]+vn[cs].ldim[id]) {
		if (id>1) {
			val[id]=vn[cs].offset[id];
			inUpdate(cs,id-1,val);
		}
		else {
			return FALSE;
		}
	}
	return TRUE;
}


/* INARRAY ()
 * Not DEC nor ISO
 * execute the inarray(ch, ar) procedure
 * ch: the channel to which array must be printed
 * ar: the array code */
void INARRAY(void) {
	int ch=0, cs, c, dim, n;
	int val[MAXDIM+1];
	int bfa=forceArray;
	double r=0.0;
	char temp[COMPSTR], *t=temp;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (ch==1) pe(76,l);
	if (ch>0 && ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(58,l);
	}
	
	/* get array code */
	if (*p==',') p++;
	else pe(2,l);
	forceArray=TRUE;
	cs=findDEC(p);
	forceArray=bfa;
	if (!cs) pe(70,l);
	if (!_arr) pe(70,l);
	if (_type>=STRINGTYPEBOX) pe(87,l);
	p+=_len;

	/* reconduct to basic element */
	while (cs!=vn[cs].ref) cs=vn[cs].ref;

	/* store dimension */
	dim=vn[cs].dim;
		
	/* set reference array to lowers indices */
	for (n=1; n<dim; n++) val[n]=vn[cs].offset[n];
	val[dim]=vn[cs].offset[n]-1;

	/* get value and repeat for all */
	while (inUpdate(cs,dim,val)) {
		/* if current index is reset, check end of whole loop */
		if (val[dim]==vn[cs].offset[dim]) {
			/* stop when the first dimension is over */
			if (val[1]>vn[cs].offset[1]+vn[cs].ldim[1]) {
				break;
			}
		}

		/* pass over characters not being suitable for 
		 * starting a number */
		if (isEOF(ch)) return;
		c=fgetc(channel[ch].stream);
		while (!isbeginnumberchar(c)) {
			if (isEOF(ch)) return;
			c=fgetc(channel[ch].stream);
		}

		/* store anything which is a number character */
		t=temp;
		while (isnumberchar(c)) {
			*t++=c;
			if (isEOF(ch)) break;
			c=fgetc(channel[ch].stream);
		}
		*t='\0';

		/* if the number starts with '&' or '@' it is a valid ALGOL
		 * exponential, but it won't be recognized by strtold(), so
		 * add 1 in front of it */
		t=temp;
		if (*t=='+' || *t=='-') t++;
		if (*t=='&' || *t=='@') {
			char temp2[strlen(t)+2];
			strcpy(temp2,"1");
			strcat(temp2,t);
			strncpy_(t,temp2,strlen(t)+2);
		}
		
		/* change any instance of &, @ and D to 'E', and let
		 * strtold do the rest */
		t=temp; while ((t=strchr(t,'&'))) *t='E';
		t=temp; while ((t=strchr(t,'@'))) *t='E';
		t=temp; while ((t=strchr(t,'D'))) *t='E';
		t=temp;	r=strtold(t,&t);

		/* the number must be entirely comsumed */
		if (!isEOF(ch) && *t) pe(40,l);

		/* restore last character read if not discardable */
		if (c>0 && c!=CRC && c!=10 && c!=13 && c!=' ' && c!='\t') 
			ungetc(c,channel[ch].stream);
		
		/* store in the array at current position */
		if (_type==BOOLEANTYPEBOX) setArrayVal(cs,dim,val,r!=0.0);
		else if (_type==INTEGERTYPEBOX) setArrayVal(cs,dim,val,(int)r);
		else setArrayVal(cs,dim,val,r);
	}
	
	/* close reading */
	if (*p==')') p++;
	else pe(2,l);
}


/***************************************************************************
 * SFIELD procedure
 ***************************************************************************/

/* SFIELD()
 * execute the sfield(num,start,length,value) procedure
 * set length bits of num, starting from bit start, to bits of value
 * Note: it acts ONLY on integers.
 * Ref. AA-O196C-TK - 17.3.3 "Field Manipulation" */
void SFIELD(void) {
	int var, A, start, length, value, val[MAXDIM+1];
	int k=0, res=0;

	/* get var id */
	if (*p=='(') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(60,l);
	p+=_len;
	if (_type>=STRINGTYPEBOX) pe(60,l);
	if (_arr) {
		int i=0;
		if (*p=='[') p++;
		else pe(80,l);
		for (i=1;i<=vn[var].dim;i++) {
			val[i]=(int)S();
			sysRet;
			if (*p==',') p++;
			if (i>MAXDIM) pe(7,l);
		}
		if (*p==']') p++;
		else pe(70,l);
	}

	/* get value to process */
	if (!_arr) A=(int)P[vn[var].ref];
	else A=(int)getArrayVal(vn[var].ref,vn[vn[var].ref].dim,val);

	/* read start and length (and resize if values are senseless) */
	if (*p==',') p++;
	else pe(2,l);
	start=(int)S();
	sysRet;
	if (start<0) start=0;
	if (start>31) start=31;
	if (*p==',') p++;
	else pe(2,l);
	length=(int)S();
	sysRet;
	if (length<0) length=0;
	if (start+length>31) length=31-start;

	/* read setting value */
	if (*p==',') p++;
	else pe(2,l);
	value=(int)S();
	sysRet;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* calculate */
	while (k<start) res+=(A&1)*pow(2,k), A/=2, k++;
	while (k<start+length) res+=(value&1)*pow(2,k), value/=2, A/=2, k++; 
	while (A) res+=(A&1)*pow(2,k), A/=2, k++;

	/* assign */
	if (!_arr) P[vn[var].ref]=res;
	else setArrayVal(vn[var].ref,vn[vn[var].ref].dim,val,res);
}


/***************************************************************************
 * DEC I/O procedures
 ***************************************************************************/

/* CALL()
 * execute a different version of the call(X) procedure.
 * call the X ALGOL program through a different taxi call. 
 * Note: this CALL() procedure has a different scope of the original DEC ALGOL;
 * in DEC ALGOL, CALL was used to invoke an external Fortran procedure 
 * (predeclared with the EXTERNAL procedure, which could or not have a return 
 * value; DEC provided the following procedures: CALL (untyped), ICALL (integer 
 * type), RCALL (real type), DCALL (double precision Fortran type), LCALL 
 * (logical Fortran type); these had to be used with Fortran version F-40; if a 
 * Fortran F-10 had to used, the same procedure names were appended to the 
 * "F10" token, thus they were: F10CALL (untyped), F10ICALL (integer type), 
 * F10RCALL (real type), F10DCALL (double precision Fortran type), F10LCALL 
 * (logical Fortran type). 
 * In taxi, only CALL is enabled, with a different usage.
 * The syntax CALL(X) executes ALGOL program X as a sub program
 * The syntax CALL(X,N) returns in N the exit status of the execution of X.
 * Ref. AA-O196C-TK - 17.5 "FORTRAN INTERFACE PROCEDURES" */
void CALL(void) {
	char name[COMPSTR], basename[COMPSTR];
	int errN=0, var=0;

	/* get file name */
	if (*p=='(') p++;
	else pe(2,l);
	if (!isanystring(p)) pe(10,l);
	strncpy_(name,SS(),COMPSTR);
	sysRet;
	setupName(name);

	/* get optional error code variable id */
	if (*p==',') {
		p++;
		var=findDEC(p);
		if (!var) pe(60,l);
		p+=_len;
		if (_type>=STRINGTYPEBOX) pe(60,l);
	}

	/* call program */
	strncpy_(basename,CALLEX,COMPSTR);
	strncat_(basename,name,COMPSTR);
	errN=system(basename);

	/* close reading */ 
	if (*p==')') p++;
	else pe(2,l);

	if (var) P[vn[var].ref]=errN/256;
}


/* TRANSFILE()
 * execute the parameterless TRANSFILE procedure.
 * copy file on INPUT channel to file on OUTPUT channel.
 * Note: acts only from physical channels (excluding TTY) for INPUT, and on
 * physical channels included TTY for OUTPUT
 * Ref. AA-O196C-TK - 16.11 "TRANSFERRING FILES" */
void TRANSFILE(void) {
	char buffer[COMPSTR], *B=buffer;
	/* make all necessary checks */
	if (currentINPUT <=0 || currentINPUT>STREAMS)
		pe(66,l);
	if (currentOUTPUT <1 || currentOUTPUT>STREAMS)
		pe(66,l);
	if (!channel[currentINPUT].isopen)
	       	pe(34,l);
	if (!channel[currentOUTPUT].isopen) 
		pe(34,l);
	if (!(channel[currentINPUT].wayout & VREAD))
		pe(58,l);
	if (!(channel[currentOUTPUT].wayout & VWRITE))
		pe(59,l);

	/* perform the transfer */
	while(fgets(B, COMPSTR, channel[currentINPUT].stream)) {
		fputs(B, channel[currentOUTPUT].stream);
	}

	/* set EOF on INPUT (of course file has be read entirely) */
	channel[currentINPUT].iochan+=0000100;
}


/* INPUT()
 * execute the INPUT(channel, id) procedure:
 * establish a connection for input between a device (represented by string
 * id) and channel.
 * - for devices: TTY and DSK are considered;
 * - for logical I/O: normal not-indexed strings are considered.
 * Ref. AA-O196C-TK - 16.2 "ALLOCATION OF PERIPHERAL DEVICES" */
void INPUT(void) {
	char name[COMPSTR], Tlabel[COMPSTR], *pb;
	int ch=0, isLabel=FALSE, errC=0, count=0;
	errCODE=-1;
	errLINE=-1;

	/* get channel */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	/* avoid reassigning channel 0 */
	if (!ch) return;

	/* check for channel 1 */
	if (ch==1) pe(76,l);

	/* get string value */
	if (*p==',') p++;
	else pe(2,l);
	pb=p;
	if (!isanystring(p)) pe(10,l);
	strncpy_(name,SS(),COMPSTR);
	sysRet;

	/* close reading after second required parameter */
	if (*p==')') p++;
	/* third optional parameter (ignored): was data type (binary/textual) */
	else if (*p==',') {
		p++;
		S();
		sysRet;
		/* close reading after third optional parameter */
		if (*p==')') p++;
		/* fourth optional parameter (ignored): was buffer size */
		else if (*p==',') {
			p++;
			S();
			sysRet;
			/* close reading after fourth optional parameter */
			if (*p==')') p++;
			/* fifth optional parameter: label */
			else if (*p==',') {
				char *t=Tlabel;
				p++;
				while (islabelnamechar(p)) 
					*t++=*p++;
				*t='\0';
				isLabel=TRUE;
				/* close reading after 
				 * fifth optional parameter */
				if (*p==')') p++;
				else pe(2,l);
			}
			else pe(2,l);
		}
		else pe(2,l);
	}
	else pe(2,l);
	
	/* check channel */
	if (ch<0 || ch>OSTREAMS) {
		errC=44;
		goto inputLabelError;
	}

	/* proceed to input */
	if (ch<=STREAMS) {
		/* TTY device */
		if (!strncasecmp_(name,"TTY",3)) {
			channel[ch].stream=stdout; // don't change!
			channel[ch].isopen=TRUE;
			channel[ch].type=SEQ;
			channel[ch].fcounter=0;
			channel[ch].fprintn=1;
			channel[ch].pageL=1;
			channel[ch].pageN=1;
			channel[ch].wayout=VREAD+VWRITE;
			channel[ch].margin=SCRLIM;	
			channel[ch].iochan=0547675;
		}
		else if (!strncasecmp_(name,"DSK",3)) {
			channel[ch].wayout=VREAD;
			channel[ch].iochan=0643420;
		}
		else {
			errC=32;
			goto inputLabelError;
		}
	}
	/* logical device (string) */
	else if (ch > STREAMS && ch<=OSTREAMS) {
		int var;
		p=pb;
		var=findDEC(p);
		if (!var || _arr || _type!=STRINGTYPEBOX) {
			errC=56;
			goto inputLabelError;
		}
		p+=_len;
		byteVar[ch]=var;
		bytePos[ch]=0;
		channel[ch].isopen=TRUE;
		channel[ch].type=SEQ;
		channel[ch].pageL=0;
		channel[ch].pageN=0;
		channel[ch].wayout=VREAD;
		channel[ch].iochan=0041660;
		if (*p==')') p++;
		else pe(2,l);
	}
	else {
		errC=43;
		goto inputLabelError;
	}
	return;

inputLabelError:
	if (!isLabel) pe(errC,l);
	else {
		p=Tlabel;
		/* get label index and set new position */
		count=getLabelIndex();	
		sysRet;
		if (count>0) {
			LL=label[count].ll;
			PL=label[count].pl;
			if (isDebug) {
				fprintf(stdout,"%s",INVCOLOR);
				fprintf(stdout,"Jumping to line=%d, pos=%d\n",LL,PL);
				fprintf(stdout,"%s",RESETCOLOR);
			}
			errCODE=errC;
			errLINE=l;
		}
		/* Note: in the AA-0196C-TK manual is specified, at 16.2.3 that
		 * "...If the actual parameter is a switch whose subscript is 
		 * out of range, the procedures behave as though the label
		 * parameter were absent."; I assume the same in case the label
		 * is inexistent, and thus I emit errC and not error=11 */
		else if (!count && errC) pe(errC,l);
		else if (!count) pe(11,l);
		else pe(-count,l);
	}
}


/* OUTPUT()
 * execute the OUTPUT(channel, id, binaryid, bufferid, errlabel)
 * establish a connection for output between a device (represented by string
 * id) and a channel. 
 * If third option binaryid is used, write data in binary mode.
 * If fourth option bufferid is used, used bufferid buffers
 * If fifth option errlabel is used, in case of errors go to label errlabel.
 * Note: third and fourth numeric arguments are currently ignored, while fifth
 * is the label to jump to in case of error.
 * - for devices: TTY, DSK and LPT/PRN/PLT are considered;
 * - for logical I/O: normal not-indexed strings are considered.
 * Ref. AA-O196C-TK - 16.2 "ALLOCATION OF PERIPHERAL DEVICES" */
void OUTPUT(void) {
	char name[COMPSTR], Tlabel[COMPSTR], *pb;
	int ch=0, isLabel=FALSE, errC=0, count=0;
	errCODE=-1;
	errLINE=-1;

	/* get channel */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	
	/* avoid using channel 0 */
	if (!ch) pe(82,l);

	/* get string value */
	if (*p==',') p++;
	else pe(2,l);
	pb=p;
	if (!isanystring(p)) pe(10,l);
	strncpy_(name,SS(),COMPSTR);
	sysRet;

	/* close reading after second required parameter */
	if (*p==')') p++;
	/* third optional parameter (ignored): was data type (binary/textual) */
	else if (*p==',') {
		p++;
		S();
		sysRet;
		/* close reading after third optional parameter */
		if (*p==')') p++;
		/* fourth optional parameter (ignored): was buffer size */
		else if (*p==',') {
			p++;
			S();
			sysRet;
			/* close reading after fourth optional parameter */
			if (*p==')') p++;
			/* fifth optional parameter: label */
			else if (*p==',') {
				char *t=Tlabel;
				p++;
				while (islabelnamechar(p))
					*t++=*p++;
				*t='\0';
				isLabel=TRUE;
				/* close reading after 
				 * fifth optional parameter */
				if (*p==')') p++;
				else pe(2,l);
			}
			else pe(2,l);
		}
		else pe(2,l);
	}
	else pe(2,l);
	
	/* check channel */
	if (ch<0 || ch>OSTREAMS) {
		errC=44;
		goto outputLabelError;
	}

	/* proceed to output */
	if (ch<=STREAMS) {
		/* TTY device */
		if (!strncasecmp_(name,"TTY",3)) {
			channel[ch].stream=stdout;
			channel[ch].isopen=TRUE;
			channel[ch].type=SEQ;
			channel[ch].fcounter=0;
			channel[ch].fprintn=1;
			channel[ch].pageL=1;
			channel[ch].pageN=1;
			channel[ch].wayout=VWRITE+VREAD;
			channel[ch].margin=SCRLIM;	
			channel[ch].iochan=0547675;
		}
		else if (!strncasecmp_(name,"DSK",3)) {
			channel[ch].wayout=VWRITE;
			channel[ch].iochan=0643030;
		}
		else if (!strncasecmp_(name,"PRN",3) ||\
				!strncasecmp_(name,"PLT",3) ||\
				!strncasecmp_(name,"PLT",3)) {
			channel[ch].stream=fopen(prTEMP,"w");
			if (channel[ch].stream==NULL) {
				errC=64;
				goto outputLabelError;
			}
			channel[ch].isopen=TRUE;
			channel[ch].type=SEQ;
			channel[ch].fcounter=0;
			channel[ch].fprintn=1;
			channel[ch].pageL=1;
			channel[ch].pageN=1;
			channel[ch].margin=SCRLIM;	
			channel[ch].iochan=0470035;
			channel[ch].wayout=VWRITE;
			isPrinterCH=ch;
		}
		else {
			errC=3;
			goto outputLabelError;
		}
	}
	/* logical device (string) */
	else if (ch>STREAMS && ch <= OSTREAMS) {
		int var;
		p=pb;
		var=findDEC(p);
		if (!var || _arr || _type!=STRINGTYPEBOX) {
			errC=56;
			goto outputLabelError;
		}
		p+=_len;
		byteVar[ch]=var;
		bytePos[ch]=0;
		channel[ch].isopen=TRUE;
		channel[ch].type=SEQ;
		channel[ch].wayout=VWRITE;
		channel[ch].pageL=0;
		channel[ch].pageN=0;
		channel[ch].iochan=0041035;
		if (*p==')') p++;
		else pe(2,l);
	}
	else {
		errC=43;
		goto outputLabelError;
	}
	return;

outputLabelError:
	if (!isLabel) pe(errC,l);
	else {
		p=Tlabel;
		/* get label index and set new position */
		count=getLabelIndex();	
		sysRet;
		if (count>0) {
			LL=label[count].ll;
			PL=label[count].pl;
			if (isDebug) {
				fprintf(stdout,"%s",INVCOLOR);
				fprintf(stdout,"Jumping to line=%d, pos=%d\n",LL,PL);
				fprintf(stdout,"%s",RESETCOLOR);
			}
			errCODE=errC;
			errLINE=l;
		}
		/* Note: in the AA-0196C-TK manual is specified, at 16.2.3 that
		 * "...If the actual parameter is a switch whose subscript is 
		 * out of range, the procedures behave as though the label
		 * parameter were absent."; I assume the same in case the label
		 * is inexistent, and thus I emit errC and not error=11 */
		else if (!count && errC) pe(errC,l);
		else if (!count) pe(11,l);
		else pe(-count,l);
	}
}


/* CREATEFILE()
 * execute the createfile(filename) 
 * create an empty ASCII file on the file system with name filename; 
 * file name may be given with the complete path.
 * Note: this procedure did not belong to the DEC-system 10/20 ALGOL */
void CREATEFILE(void) {
	char name[COMPSTR];
	FILE *f;

	/* get file name */
	if (*p=='(') p++;
	else pe(2,l);
	if (!isanystring(p)) pe(10,l);
	strncpy_(name,SS(),COMPSTR);
	sysRet;
	f=fopen(name,"w+");
	if (f==NULL) pe(67,l);
	else fclose(f);

	/* close reading */ 
	if (*p==')') p++;
	else pe(2,l);
}


/* OPENFILE()
 * execute the openfile(channel, filename, protid, eraseid, errlabel, errnum)
 * open a file with name filename on channel (which must have been opened by
 * INPUT/OUTPUT); if channel was open in input, open for read, if it was open 
 * in output, open for write.
 * filename may be given with the complete path.
 * Set user permissions on file with third optional parameter protid, and the 
 * erase flag with optional fourth parameter eraseid (if TRUE, erase file 
 * before opening it).
 * Note: the permission flag protid should be an octal value for safety reasons;
 * Note: the fourth parameter is not the same fourth parameter that was 
 * available on the DEC-20 ALGOL, because here the project-programmer code is 
 * useless.
 * Optional fifth parameter is the label to which the program must be 
 * redirected to in case of errors in reading, while sixth optional parameter
 * errnum is the error code (to be treated in the label section).
 * Note: if the file is opened on an existing opened file with a new name, the
 * file is renamed to the new name; if the file is opened on an existing opened 
 * file with a void name, the file is deleted (removed).
 * Ref. AA-O196C-TK - 16 "DATA TRANSMISSION" 
 * Ref. AA-O196C-TK - 16.4 "FILE DEVICES" */
void OPENFILE(void) {
	int ch=0, type=SEQ, protectFile=FALSE, eraseFile=FALSE;
	char name[COMPSTR], Tlabel[COMPSTR];
	int isLabel=FALSE, isErrC=FALSE, errC=0, count=0;
	errCODE=-1;
	errLINE=-1;

	/* get channel */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;

	/* first check: device open */
	if (channel[ch].wayout==NOUSE) pe(68,l);
	
	/* get file name */
	if (*p==',') p++;
	else pe(2,l);
	if (!isanystring(p)) pe(10,l);
	strncpy_(name,SS(),COMPSTR);
	sysRet;

	/* close reading after second required parameter */
	if (*p==')') p++;
	/* third optional parameter: file protection octal code */
	else if (*p==',') {
		p++;
		protectFile=(int)S(); // must be an octal code
		sysRet;
		/* close reading after third optional parameter */
		if (*p==')') p++;
		/* fourth optional parameter: file erasure flag */
		else if (*p==',') {
			p++;
			eraseFile=(int)S();
			sysRet;
			/* close reading after fourth optional parameter */
			if (*p==')') p++;
			/* fifth optional parameter: label */
			else if (*p==',') {
				char *t=Tlabel;
				p++;
				while (islabelnamechar(p)) *t++=*p++;
				*t='\0';
				isLabel=TRUE;
				/* close reading after 
				 * fifth optional parameter */
				if (*p==')') p++;
				/* sixth optional parameter: error code */
				else if (*p==',') {
					p++;
					isErrC=findDEC(p);
					if (!isErrC) pe(60,l);
					if (_arr) pe(60,l); 
					if (_type>=STRINGTYPEBOX) 
						pe(60,l);
					p+=_len;
					/* close reading after sixth and last 
					 * optional parameter */
					if (*p==')') p++;
					else pe(2,l);
				}
				else pe(2,l);
			}
			else pe(2,l);
		}
		else pe(2,l);
	}
	else pe(2,l);

	/* check channel */
	if (ch<0 || ch>STREAMS) {
		errC=44;
		goto openLabelError;
	}

	/* check if file exists */
	if (channel[ch].isopen) {
		/* names differ and 'name' is not void: rename 
		 * Ref. AA-O196C-TK - 16.4 "FILE DEVICES", page 16-4, last
		 * lines: "...if a file is already open, use of OPENFILE causes 
		 * the file to be renamed with the new name supplied." */
		if (strcmp(name,channel[ch].ioname) && name[0]) {
			if (rename(channel[ch].ioname,name)) {
				errC=37;
				goto openLabelError;
			}
			strncpy_(channel[ch].ioname,name,COMPSTR);
		}
		/* names differ but 'name' is void: delete file 
		 * Ref. AA-O196C-TK - 16.4 "FILE DEVICES", page 16-4, last
		 * lines and page 16-5, first lines: "If the string containing 
		 * the new name is null, the original file is deleted."
		 * NOTE: deleted here may also signify, in a collateral sense,
		 * "emptied", which means the file is not removed from the
		 * File System; I stick to the original sense of deleted as
		 * "removed" */
		else if (strcmp(name,channel[ch].ioname) && !name[0]) {
			if (remove(channel[ch].ioname)) {
				errC=37;
				goto openLabelError;
			}
		}
		/* names match: file is already open. */
		else {
			errC=78;
			goto openLabelError;
		}
		return;
	}

	/* copy name */
	strncpy_(channel[ch].ioname,name,COMPSTR);
	
	/* erase file if required */	
	if (eraseFile) {
		channel[ch].stream=fopen(channel[ch].ioname,"w");
		if (channel[ch].stream==NULL) {
			errC=64;
			goto openLabelError;
		}
		fclose(channel[ch].stream);
	}

	/* open channel in current mode */
	if (channel[ch].wayout & VREAD) {
		channel[ch].stream=fopen(channel[ch].ioname,"r");
		if (channel[ch].stream==NULL) {
			errC=64;
			goto openLabelError;
		}
		channel[ch].iochan+=0000240;
	}
	else if (channel[ch].wayout & VWRITE) {
		channel[ch].stream=fopen(channel[ch].ioname,"a");
		if (channel[ch].stream==NULL) {
			errC=64;
			goto openLabelError;
		}
		channel[ch].iochan+=0000005;
	}
	/* reset channels parameters */
	channel[ch].isopen=TRUE;
	channel[ch].type=type;	
	channel[ch].fcounter=0;
	channel[ch].fprintn=TRUE;
	channel[ch].margin=FILESCRLIM;
	
	/* set file permissions */
	if (protectFile) {
		chmod(channel[ch].ioname, (mode_t) protectFile);
	}
	return;

openLabelError:
	if (!isLabel) pe(errC,l);
	else {
		p=Tlabel;
		/* get label index and set new position */
		count=getLabelIndex();	
		sysRet;
		if (count>0) {
			LL=label[count].ll;
			PL=label[count].pl;
			if (isDebug) {
				fprintf(stdout,"%s",INVCOLOR);
				fprintf(stdout,"Jumping to line=%d, pos=%d\n",LL,PL);
				fprintf(stdout,"%s",RESETCOLOR);
			}
			if (isErrC) P[vn[isErrC].ref]=errC;
			errCODE=errC;
			errLINE=l;
		}
		/* Note: in the AA-0196C-TK manual is specified, at 16.4.1 that
		 * "...If the actual label parameter is a switch whose 
		 * subscript is out of range, the procedure behaves as though 
		 * the label parameter were absent..."; I assume the same in 
		 * case the label is inexistent, and thus I emit errC and 
		 * not error=11 */
		else if (!count && errC) pe(errC,l);
		else if (!count) pe(11,l);
		else pe(-count,l);
	}
}


/* SELECTINPUT()
 * execute the SELECTINPUT(channel) procedure.
 * set the argument channel as current input channel.
 * Ref. AA-O196C-TK - 16.3 "SELECTING INPUT/OUTPUT CHANNELS" */
void SELECTINPUT(void) {
	int ch=0;

	/* get channel */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;

	/* check */
	if (ch==1) pe(76,l);
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (!(channel[ch].wayout & VREAD)) pe(58,l);
	if (ch==currentOUTPUT) pe(65,l);

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* set current INPUT channel */
	currentINPUT=ch;
}


/* SELECTOUTPUT() 
 * execute the SELECTOUTPUT(channel) procedure.
 * set the argument channel as current output channel.
 * Ref. AA-O196C-TK - 16.3 "SELECTING INPUT/OUTPUT CHANNELS" */
void SELECTOUTPUT(void) {
	int ch=0;

	/* get channel */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;

	/* check */
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (!(channel[ch].wayout & VWRITE)) pe(59,l);
	if (currentINPUT==ch && ch>0) pe(65,l);
	
	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* set current OUTPUT channel */
	currentOUTPUT=ch;
}


/* CLOSEFILE()
 * execute the CLOSEFILE(channel1, channel2, ...)
 * close each file channel in argument list; if the channel given is 0, 
 * returns safely, and if it is a logical channel, an error message is printed.
 * Note: no mention appears on the AA-0196C-TK about error messages in case
 * the channel is not open, or it is open in either read or write mode. 
 * Ref. AA-O196C-TK - 16.4 "FILE DEVICES" */
void CLOSEFILE(void) {
	int ch=0, ec=0;

	if (*p=='(') p++;
	else pe(2,l);
	do {
		/* get channel number */
		if (*p==',') p++;
		ch=(int)S();
		if (isCDC && ch>=60) ch-=60;
		sysRet;
		
		/* channels 0 and 1 cannot be released */
		if (!ch || ch==1) continue;
		/* channel out of range */
		if (ch<0 || ch>STREAMS) pe(44,l);
	
		/* close file channel and reset parameters */
		fflush(channel[ch].stream);
		if (channel[ch].stream) {
			ec=fclose(channel[ch].stream);
			/* error condition on closing file */
			if (ec && ec==EOF) pe(42,l);
		}	
		channel[ch].isopen=FALSE;

	} while (*p==',');
	
	/* close reading */
	if (*p==')') p++;
	else pe(2,l);
}


/* RELEASE()
 * execute the release(channel) procedure.
 * release the channel in argument; acts on logical string, detaching the
 * associated channel.
 * Note: no mention appears on the AA-0196C-TK about error messages in case
 * the channel is not open, or it is open in either read or write mode. 
 * Ref. AA-O196C-TK - 16.5 "RELEASING DEVICES" */
void RELEASE(void) {
	int ch=0;

	/* get channel */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;

	/* channels 0 and 1 cannot be released */
	if (!ch || ch==1) return;
	/* channel out of range */
	if (ch<0 || ch > OSTREAMS) pe(44,l);

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* reset current if in case */
	if (currentINPUT==ch) {
		currentINPUT=CONSOLE;
	}
	if (currentOUTPUT==ch) {
		currentOUTPUT=OCONSOLE;
	}

	/* device/file channels */
	if (ch<=STREAMS) {
		if (channel[ch].isopen) pe(77,l);
		/* close device/file channel and reset parameters */
		strncpy_(channel[ch].ioname,"\0\0\0\0\0",COMPSTR);
		channel[ch].type=NOUSE;
		channel[ch].fcounter=0;
		channel[ch].fprintn=TRUE;
		channel[ch].margin=FILESCRLIM;
		channel[ch].wayout=NOUSE;
	}
	/* logical devices */
	else {
		byteVar[ch]=0;
		bytePos[ch]=0;
		channel[ch].isopen=FALSE;
		channel[ch].type=NOUSE;
		channel[ch].wayout=NOUSE;
	}
	channel[ch].iochan=0000000;

	/* if it was a printer channel, direct to printer the result,
	 * close file and reset; in case of error while removing the
	 * temp file, print a non-blocking message */
	if (ch==isPrinterCH) {
		char command[COMPSTR];
		if (channel[ch].stream) fclose(channel[ch].stream);
		sprintw(command,COMPSTR,"%s %s",ROUTESTRING,prTEMP);
		system(command);
		isPrinterCH=FALSE;
		if (remove(prTEMP)) fprintf(stderr,"%s\n", error[79]);
	}
}


/* INSYMBOL()
 * execute the INSYMBOL(S) procedure
 * read next byte from current input channel and put it in the INTEGER var S.
 * Actually for taxi, S may also be a Boolean or a real/long real.
 * Ref. AA-O196C-TK - 16.6 "INPUT/OUTPUT PROCEDURES" */
void INSYMBOL(void) {
	int var=0, isArr_=FALSE, val[MAXDIM+1];
	int ch=currentINPUT;

	/* get variable id */
	if (*p=='(') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(60,l);
	p+=_len;
	if (_type>=STRINGTYPEBOX) pe(60,l);
	if (_arr) {
		int c=1,i=0;
		isArr_=TRUE;
		if (*p=='[') p++;
		else pe(80,l);
		for (i=1;i<=vn[var].dim;i++) {
			val[c++]=(int)S();
			sysRet;
			if (*p==',') p++;
			if (c>MAXDIM) pe(7,l);
		}
		if (*p==']') p++;
		else pe(70,l);
	}

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* check EOF */
	if (isEOF(ch)) pe(38,l);
	
	/* Assign next byte from channel */
	if (ch==1) pe(76,l);
	else if (ch>0 && ch<=STREAMS) {
		if (!isArr_) P[vn[var].ref]=(double)fgetc(channel[ch].stream);
		else setArrayVal(vn[var].ref,vn[vn[var].ref].dim,val,(double)fgetc(channel[ch].stream));
		/* check EOF */
		isEOF(ch);
	}
	/* Assign next byte from logical channel */
	else if (ch>STREAMS && ch<=OSTREAMS) {
		P[vn[var].ref]=(double)PS[vn[byteVar[ch]].ref][bytePos[ch]];
		bytePos[ch]++;
		if (bytePos[ch]>strlen(PS[vn[byteVar[ch]].ref])) bytePos[ch]=0;
	}
	else pe(44,l);
}


/* OUTSYMBOL()
 * execute the OUTSYMBOL(S) procedure
 * put a byte in the INTEGER value S to current output channel
 * Note: in taxi, S may also be a math expression, and not obligatorily
 * an INTEGER variable
 * Ref. AA-O196C-TK - 16.6 "INPUT/OUTPUT PROCEDURES" */
void OUTSYMBOL(void) {
	int val=0;
	int ch=currentOUTPUT;

	/* get byte */
	if (*p=='(') p++;
	else pe(2,l);
	val=(int)S();
	sysRet;
		
	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* Put value to current file channel */
	if (ch>0 && ch<=STREAMS) {
		fputc((char)val, channel[ch].stream);
		channel[ch].fcounter++;
		if (channel[ch].fcounter>FILESCRLIM) FCR(ch);
	}
	/* Put value to logical channel current position */
	else if (ch>STREAMS && ch<=OSTREAMS) {
		PS[vn[byteVar[ch]].ref][bytePos[ch]]=(char)val;
		bytePos[ch]++;
		if (bytePos[ch]>vn[byteVar[ch]].strmem) bytePos[ch]=0;
	}
	else pe(44,l); 
}


/* NEXTSYMBOL()
 * execute the NEXTSYMBOL(S) procedure
 * read next byte from current input channel, and put it in the INTEGER var S
 * without advancing the file pointer
 * Note: for files, this is done by using ungetc once.
 * Ref. AA-O196C-TK - 16.6 "INPUT/OUTPUT PROCEDURES" */
void NEXTSYMBOL(void) {
	int var=0, isArr_=FALSE, val[MAXDIM+1];
	int ch=currentINPUT;
	double res;

	/* get variable id */
	if (*p=='(') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(60,l);
	p+=_len;
	if (_type>=STRINGTYPEBOX) pe(60,l);
	if (_arr) {
		int c=1,i=0;
		isArr_=TRUE;
		if (*p=='[') p++;
		else pe(80,l);
		for (i=1;i<=vn[var].dim;i++) {
			val[c++]=(int)S();
			sysRet;
			if (*p==',') p++;
			if (c>MAXDIM) pe(7,l);
		}
		if (*p==']') p++;
		else pe(70,l);
	}
		
	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* check EOF */
	if (isEOF(ch)) pe(38,l);
	
	/* get result */
	if (ch==1) pe(76,l);
	else if (ch>=0 && ch<=STREAMS) {
		int h=fgetc(channel[ch].stream);
		res=(double)h;
		ungetc(h, channel[ch].stream);
		/* check EOF */
		isEOF(ch);
	}
	else if (ch>STREAMS && ch<=OSTREAMS) {
		res=(double)PS[vn[byteVar[ch]].ref][bytePos[ch]];
	}
	else pe(44,l); 

	/* Assign next byte from channel */
	if (!isArr_) P[vn[var].ref]=res;
	else setArrayVal(vn[var].ref,vn[vn[var].ref].dim,val,res);

}


/* SKIPSYMBOL()
 * execute the parameterless SKIPSYMBOL procedure
 * read next byte from current channel and discard it.
 * Ref. AA-O196C-TK - 16.6 "INPUT/OUTPUT PROCEDURES" */
void SKIPSYMBOL(void) {
	int ch=currentINPUT;
	if (ch==1) pe(76,l);
	/* Assign next byte from channel */
	else if (ch>=0 && ch<=STREAMS) {
		if (isEOF(ch)) pe(38,l);
		fgetc(channel[ch].stream);
		/* check EOF */
		isEOF(ch);
	}
	/* Assign next byte from logical channel */
	else if (ch>STREAMS && ch<=OSTREAMS) {
		bytePos[ch]++;
		if (bytePos[ch]>strlen(PS[vn[byteVar[ch]].ref])) bytePos[ch]=0;
	}

}


/* BREAKOUTPUT()
 * execute the parameterless BREAKOUTPUT procedure
 * empty current output buffer.
 * Note: this is done through flush on the current output buffer, but
 * this is mainly a no-op procedure, because taxi does not maintain its own
 * output buffer, leaving the task to the UNIX OS. 
 * Ref. AA-O196C-TK - 16.6 "INPUT/OUTPUT PROCEDURES" */
void BREAKOUTPUT(void) {
	if (currentOUTPUT>0 && currentOUTPUT<=STREAMS)
		fflush(channel[currentOUTPUT].stream);
}


/***************************************************************************
 * ISO I/O procedures
 ***************************************************************************/

/* inchar_()
 * ch is the channel number;
 * compstr is the comparing string.
 * Return C-position (starting from 0) added by 1 (ALGOL position starting
 * from 1)
 * Low-level system function for INCHAR and related procedures. */
int inchar_(int ch, char *compstr) {
	int i=0, val=0;

	/* scan string with file character */
	if (ch>=0 && ch<=STREAMS) {
		val=fgetc(channel[ch].stream);
		while (val>0 && (val=='\n' || val==10 || val==13))
			val=fgetc(channel[ch].stream);
		for (i=0;i<strlen(compstr);i++)
			if (*(compstr+i)==val) break;
	}
	/* scan string with string-channel character */
	else if (ch>STREAMS && ch<=OSTREAMS) {
		val=PS[vn[byteVar[ch]].ref][bytePos[ch]];
		bytePos[ch]++;
		if (bytePos[ch]>strlen(PS[vn[byteVar[ch]].ref])) bytePos[ch]=0;
		for (i=0;i<strlen(compstr);i++)
			if (*(compstr+i)==val) break;
	}
	else pe(44,l); 

	/* check irregular results */
	if (i>=strlen(compstr)) i=-1;

	return i+1;
}


/* outchar_()
 * ch is the channel number;
 * compstr is the comparing string;
 * pos is the position in compstr (ALGOL-position starting from 1).
 * Low-level system function for OUTCHAR and related procedures. */
void outchar_(int ch, char *compstr, int pos) {
	if (ch<=STREAMS) {
		fputc(*(compstr+pos-1),channel[ch].stream);
		channel[ch].fcounter++;
	}
	else {
		PS[vn[byteVar[ch]].ref][bytePos[ch]]=*(compstr+pos-1);
		bytePos[ch]++;
		if (bytePos[ch]>strlen(PS[vn[byteVar[ch]].ref])) bytePos[ch]=0;
	}
}


/* parseISOLiteral()
 * analyses the string in str and returns the string with escaping chars 
 * according to the ISO and C rules 
 * return in newp the updated str pointer
 * System function for the INCHAR procedure */
char *parseISOLiteral(char *str, char **newp) {
	char output[COMPSTR], *o=output;
	/* parse double-quoted string */
	if (*str=='\"') {
		char q=*str++;
		while (*str) {
			if (*str=='\\') {
				str++;
				switch (*str++) {
					case 'b': *o++='\b'; break;
					case 'f': *o++='\f'; break;
					case 'n': *o++='\n'; break;
					case 'r': *o++='\r'; break;
					case 't': *o++='\t'; break;
					case 'v': *o++='\v'; break;
					case '\\': *o++='\\'; break;
					case '\'': *o++='\''; break;
					case '\"': *o++='\"'; break;
				}

			}
			else if (*str==q && *(str+1)==q) {
				str+=2;
				*o++='\"';
			}
			else if (*str==q && *(str+1)!=q) {
				break;
			}
			else *o++=*str++;
		}
		if (*str==q) str++;
	}
	/* parse quoted string */
	else if (*str=='\'' && *(str+1)!='\'') {
		str+=2;
		while (*str) {
			if (*str=='\\') {
				str++;
				switch (*str++) {
					case 'b': *o++='\b'; break;
					case 'f': *o++='\f'; break;
					case 'n': *o++='\n'; break;
					case 'r': *o++='\r'; break;
					case 't': *o++='\t'; break;
					case 'v': *o++='\v'; break;
					case '\\': *o++='\\'; break;
					case '\'': *o++='\''; break;
					case '\"': *o++='\"'; break;
				}

			}
			else if (*str=='\"' && *(str+1)=='\"') {
				str+=2;
				*o++='\"';
			}
			else if (*str=='\'' && *(str+1)=='\'') {
				break;
			}
			else *o++=*str++;
		}
		if (*str=='\'' && *(str+1)=='\'') str+=2;
	}
	*o='\0';
	if (newp) *newp=str;
	return MYPAD=output;
}


/* INCHAR() 
 * execute the INCHAR(channel, str, intvar) procedure:
 * channel is the channel number;
 * str is the comparing string;
 * intvar contains the position in the string (result); it must be a 
 * valid integer variable name.
 * All values are taken from the characters following in the program.
 * Set intvar to value corresponding to the first occurrence in str of current 
 * character from channel buffer. Set intvar to zero if character not in str. 
 * Update channel pointer in case of logical devices (file pointer is 
 * maintained by the OS) 
 * Ref. ISO 1538-1984 (E) - Appendix 2 "The environmental block" */
void INCHAR(void) {
	int ch=0, var=0, isArr_=FALSE, val[MAXDIM+1];
	char compstr[COMPSTR];

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (ch==1) pe(76,l);
	if (ch>0 && ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(58,l);
	}
	
	/* get comparing string */
	if (*p==',') p++;
	else pe(2,l);
	if (!isanystring(p)) pe(10,l);
	strncpy_(compstr,parseISOLiteral(p,&p),COMPSTR);
	sysRet;

	/* get variable id */
	if (*p==',') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(60,l);
	p+=_len;
	if (_type>=STRINGTYPEBOX) pe(60,l);
	if (_arr) {
		int c=1,i=0;
		isArr_=TRUE;
		if (*p=='[') p++;
		else pe(80,l);
		for (i=1;i<=vn[var].dim;i++) {
			val[c++]=(int)S();
			sysRet;
			if (*p==',') p++;
			if (c>MAXDIM) pe(7,l);
		}
		if (*p==']') p++;
		else pe(70,l);
	}
	
	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* set return value */
	if (isEOF(ch)) pe(38,l);
	if (!isArr_) P[vn[var].ref]=inchar_(ch,compstr);
	else 
	   setArrayVal(vn[var].ref,vn[vn[var].ref].dim,val,inchar_(ch,compstr));
	isEOF(ch);
}


/* OUTCHAR() 
 * execute the OUTCHAR(channel, str, intvar) ISO procedure:
 * channel is the channel number;
 * str is the comparing string;
 * intvar contains the value to pass; it must be a valid integer variable name.
 * All values are taken from the characters following in the program.
 * Pass to channel the character in str, corresponding to the value of intvar. 
 * Manually update channel pointer in case of logical devices, whereas file 
 * pointer is maintained by the OS.
 * Ref. ISO 1538-1984 (E) - Appendix 2 "The environmental block" */
void OUTCHAR(void) {
	int ch=0, pos=0;
	char compstr[COMPSTR];
	
	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VWRITE)) pe(59,l);
	}

	/* get comparing string */
	if (*p==',') p++;
	else pe(2,l);
	if (!isanystring(p)) pe(10,l);
	strncpy_(compstr,SS(),COMPSTR);
	sysRet;

	/* get position */
	if (*p==',') p++;
	else pe(2,l);
	pos=(int)S();
	sysRet;
	if (pos<=0 || pos > strlen(compstr)) pe(63,l);
	
	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* emit to channel */
	outchar_(ch,compstr,pos);
	channel[ch].fcounter++;
}


/* INSTRING() 
 * Not ISO nor DEC
 * Execute the INSTRING(channel,str) procedure:
 * channel is the channel number;
 * str is the output string. */
void INSTRING(void) {
	int ch,var=0,len=0, isArr_=FALSE, val[MAXDIM+1];
	char temp[COMPSTR], *t=temp;
	
	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (ch==1) pe(76,l);
	if (ch>0 && ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(58,l);
	}

	/* get string variable id */
	if (*p==',') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(56,l);
	if (_type!=STRINGTYPEBOX) pe(56,l);
	p+=_len;
	if (_arr) {
		int c=1,i=0;
		isArr_=TRUE;
		if (*p=='[') p++;
		else pe(80,l);
		for (i=1;i<=vn[var].dim;i++) {
			val[c++]=(int)S();
			sysRet;
			if (*p==',') p++;
			if (c>MAXDIM) pe(7,l);
		}
		if (*p==']') p++;
		else pe(70,l);
	}
	len=vn[var].strmem;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* copy from logical channel */
	if (ch>STREAMS && ch<=OSTREAMS) {
		if (!isArr_) 
		   strncpy_(PS[vn[var].ref],PS[vn[byteVar[ch]].ref]+bytePos[ch],len);
		else 
		   setArrayStr(vn[var].ref,vn[vn[var].ref].dim,val,PS[vn[byteVar[ch]].ref]+bytePos[ch]);
		return;
	}

	/* read string from keyboard (ch=0) or channel */
	if (isEOF(ch)) pe(38,l);
	strncpy_(temp,getISOString(ch),COMPSTR);

	/* store in variable */
	if (!isArr_) strncpy_(PS[vn[var].ref],t,len);
	else setArrayStr(vn[var].ref,vn[vn[var].ref].dim,val,t);
	
	/* test EOF */
	isEOF(ch);
}


/* OUTSTRING()  
 * Execute the OUTSTRING(channel, str) ISO procedure:
 * channel is the channel number;
 * str is the output string.
 * Note: this procedure uses the low-level outchar_() system function.
 * Ref. ISO 1538-1984 (E) Appendix 2 "The environmental block" */
void OUTSTRING(void) {
	int ch=0, mi,n;
	char compstr[COMPSTR];
	
	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VWRITE)) pe(59,l);
	}

	/* get output string */
	if (*p==',') p++;
	else pe(2,l);
	if (!isanystring(p)) pe(10,l);
	strncpy_(compstr,SS(),COMPSTR);
	sysRet;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* ISO compliant procedure */
	n=strlen(compstr);
	for (mi=0;mi<n;mi++) {
		if (compstr[mi]=='\\') {
			int c1=compstr[mi+1];
			switch (c1) {
				case 'n': 
				case 'r':
					emitChar(ch,'\n');
					break;
				case 't': 
					emitChar(ch,'\t');
					break;
				case 'b': 
					emitChar(ch,'\b');
					break;
				case 'f': 
					doPage(ch);
					break;
				case 'v': 
					emitChar(ch,'\v');
					break;
				case '\\': 
					emitChar(ch,'\\');
					break;
				case '\'': 
					emitChar(ch,'\'');
					break;
				case '\"': 
					emitChar(ch,'\"');
					break;

				default: 
					emitChar(ch,c1);
					break;
			}
			mi++;
		}
		else emitChar(ch,compstr[mi]);
	}
}


/* outterminator_()
 * Low-level system function for OUTTERMINATOR and those who use 
 * OUTTERMINATOR */
void outterminator_(int ch) {
	/* emit a space */
	if (ch<=STREAMS) {
		fputc(' ',channel[ch].stream);
	}
	else if (ch<=OSTREAMS) {
		PS[vn[byteVar[ch]].ref][bytePos[ch]]=' ';
		bytePos[ch]++;
		if (bytePos[ch]>strlen(PS[vn[byteVar[ch]].ref])) bytePos[ch]=0;
	}
}


/* OUTTERMINATOR() 
 * Execute the OUTTERMINATOR(channel) procedure
 * outputs a terminator (space in this installation) to channel 
 * Note: this procedure uses the low-level outterminator_() system function.
 * Ref. ISO 1538-1984 (E) Appendix 2 "The environmental block" */
void OUTTERMINATOR(void) {
	int ch=0;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch>=0 && ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VWRITE)) pe(59,l);
	}

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* output terminator */
	outterminator_(ch);
	channel[ch].fcounter++;
}


/* INCHARACTER() 
 * Not ISO nor DEC
 * Execute the INCHARACTER(channel, char) procedure.
 * char takes the value of an integer, read from channel, as a 7 bit number. 
 * The terminator of the integer may be either a space, a newline or a 
 * semicolon (if other terminators are to be allowed, they may be added to the 
 * end of the string parameter of the call of inchar. No other change is 
 * necessary). Any number of spaces or newlines may precede the first character 
 * of the character */
void INCHARACTER(void) {
	int ch=0, c, var;
	int isArr_=FALSE, val[MAXDIM+1];
	struct termios thismodes, thissavemodes;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (ch==1) pe(76,l);
	if (ch>0 && ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(58,l);
	}

	/* get variable id */
	if (*p==',') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(60,l);
	p+=_len;
	if (_type>=STRINGTYPEBOX) pe(60,l);
	if (_arr) {
		int c=1,i=0;
		isArr_=TRUE;
		if (*p=='[') p++;
		else pe(80,l);
		for (i=1;i<=vn[var].dim;i++) {
			val[c++]=(int)S();
			sysRet;
			if (*p==',') p++;
			if (c>MAXDIM) pe(7,l);
		}
		if (*p==']') p++;
		else pe(70,l);
	}

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* test EOF */
	if (isEOF(ch)) pe(38,l);

	/* only from keyboard */
	if (channel[ch].stream==stdin) {
		/* set tcgetattr values backing up current state */
		tcgetattr(0, &thismodes);
		thissavemodes = thismodes;
		thismodes.c_lflag &= ~ICANON;
		thismodes.c_lflag &= ~ECHO;
		tcsetattr(0, 0, &thismodes);
		c = (char)fgetc(stdin);
		/* reset tcgetattr values */
		tcsetattr(0, 0, &thissavemodes);
	}
	/* from file or any other channel type */
	else {	
		c=fgetc(channel[ch].stream);
	}

	/* take the character read as a 7-bit number*/
	c=c&127;

	/* assign number so far */
	if (!isArr_) P[vn[var].ref] = c;
	else setArrayVal(vn[var].ref,vn[vn[var].ref].dim, val,c);
	
	/* set EOF */
	isEOF(ch);
}


/* OUTCHARACTER() 
 * Not ISO nor DEC
 * Execute the OUTCHARACTER(channel, char) procedure.
 * Passes to channel the character representing the value of char as an
 * ASCII character */
void OUTCHARACTER(void) {
	int ch=0, i;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(59,l);
	}

	/* get integer value */
	if (*p==',') p++;
	else pe(2,l);
	i=(int)S()&127;
	sysRet;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* emit character */
	EMIT(ch,i);
}


/* ININTEGER() 
 * Execute the ININTEGER(channel, int) procedure.
 * int takes the value of an integer, as defined in Section 2.5.1, read from 
 * channel. The terminator of the integer may be either a space, a newline or a 
 * semicolon (if other terminators are to be allowed, they may be added to the 
 * end of the string parameter of the call of inchar. No other change is 
 * necessary). Any number of spaces or newlines may precede the first character 
 * of the integer 
 * Ref. ISO 1538-1984 (E) Appendix 2 "The environmental block" */
void ININTEGER(void) {
	int ch=0, c, num=0, sign=1, valid, var;
	int isArr_=FALSE, val[MAXDIM+1];

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (ch==1) pe(76,l);
	if (ch>0 && ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(58,l);
	}

	/* get variable id */
	if (*p==',') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(60,l);
	p+=_len;
	if (_type>=STRINGTYPEBOX) pe(60,l);
	if (_arr) {
		int c=1,i=0;
		isArr_=TRUE;
		if (*p=='[') p++;
		else pe(80,l);
		for (i=1;i<=vn[var].dim;i++) {
			val[c++]=(int)S();
			sysRet;
			if (*p==',') p++;
			if (c>MAXDIM) pe(7,l);
		}
		if (*p==']') p++;
		else pe(70,l);
	}

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* ISO compliant procedure */

	/* pass over initial spaces or newlines, commas or semicolon */
	if (isEOF(ch)) pe(38,l);
	c=fgetc(channel[ch].stream);
	while (c == '\t' || c==' ' || c==CRC || c==';' || c==',') {
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
	}

	/* fault anything except sign or digit */
	if (!isdigit(c) && c!='-' && c!='+') pe(40,l);
	
	/* sign found, valid indicates digit not found yet, sign indicates 
	 * whether + or -, num is value so far */
	if (!isdigit(c)) { 
		valid = FALSE;
		sign = c=='-'?-1:1; 
		num = 0;
	}	
	/* valid indicates digit found, sign indicates +, 
	 * num is value so  far */
	else {
		valid = TRUE; 
		sign = 1;
		num = c - '0';
	}

	/* deal with further digits */
	if (isEOF(ch)) pe(38,l);
	c=fgetc(channel[ch].stream);
	while (isdigit(c)) {
		num = num * 10 + (c - '0');
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
		valid=TRUE;
	}

	/* further data treatment in case of channel different from CONSOLE */
	if (ch) {
		/* deal with further characters */
		while (c==' ' || c=='\t' || c==CRC || c==10 || c==13) {
			if (isEOF(ch)) break;
			c=fgetc(channel[ch].stream);
		}
	}
	
	/* restore last character read if not a blank one */
	if (c>0 && c!=CRC && c!=10 && c!=13 && c!=' ' && c!='\t') 
		ungetc(c,channel[ch].stream);

	/* fault if no digit has been found */
	if (!valid) pe(40,l);

	/* assign number so far */
	if (!isArr_) P[vn[var].ref] = (double)(num*sign);
	else setArrayVal(vn[var].ref,vn[vn[var].ref].dim,\
			val,(double)(num*sign));
	
	/* test EOF */
	isEOF(ch);
}


/* digits()
 * print number i to channel ch, evaluating recursively the digits of the
 * number in base 10.
 * ch is current channel
 * i is the integer value to print
 * system procedure for OUTINTEGER() */
void digits(int ch, int i) {
	int j=i/10;
	i=i-10*j;
	if (j) digits(ch,j);
	outchar_(ch,"0123456789",i+1);
	channel[ch].fcounter++;
}


/* OUTINTEGER() 
 * Execute the OUTINTEGER(channel, int) procedure.
 * Passes to channel the characters representing the value of int, followed
 * by a terminator 
 * Ref. ISO 1538-1984 (E) Appendix 2 "The environmental block"*/
void OUTINTEGER(void) {
	int ch=0, i,k;
	int minus=0;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(59,l);
	}

	/* get integer value */
	if (*p==',') p++;
	else pe(2,l);
	i=(int)S();
	sysRet;
	if (i<0) minus=1, i=-i;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* set for the INTMIN case (print independently) */
	k = i & INTMAX;
	if (k!=i) {
		fprintf(channel[ch].stream,"%ld",INTMIN);
		outterminator_(ch);
		channel[ch].fcounter+=12;
		return;
	}

	/* ISO compliant procedure */
	if (minus) {
		outchar_(ch,"-",1);
	}
	channel[ch].fcounter++;
	/* use recursion to evaluate digits from right to left, but
	 * print them from left to right */
	digits(ch,i);
	outterminator_(ch);
	channel[ch].fcounter++;
}


/* OUTBOOLEAN() 
 * Execute the OUTBOOLEAN(channel, bool) procedure.
 * Passes to channel the characters representing the value of bool, followed
 * by a terminator 
 * Not an ISO procedure - taxi extension */
void OUTBOOLEAN(void) {
	int ch=0, i=0;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VWRITE)) pe(59,l);
	}

	/* get Boolean value */
	if (*p==',') p++;
	else pe(2,l);
	i=(int)S();
	sysRet;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* print value */
	if (i) {
		fprintf(channel[ch].stream,"true ");
		channel[ch].fcounter+=5;
	}
	else {
		fprintf(channel[ch].stream,"false ");
		channel[ch].fcounter+=6;
	}
}


/* INREAL() 
 * Execute the INREAL(channel, re) procedure
 * re takes the value of a number read from channel. Except for the different 
 * definitions of a number and an integer the rules are exactly as for 
 * ININTEGER. Spaces or newlines may follow the symbol EXPCHAR 
 * Note: since the exponent char may be & or @ or E (option --exp), the
 * confronting string could start with, say, E4 (for 1E4), but this would
 * be interpreted instead as a variable name. So, the exponent char is set
 * to & (the DEC symbol) whenever it is not & or @, which are unambiguous 
 * symbols. 
 * Ref. ISO 1538-1984 (E) Appendix 2 "The environmental block" */
void INREAL(void) {
	int ch=0, c,sign=1,esign=1,valid,var;
	double r=0.0;
	int exp=0;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (ch==1) pe(76,l);
	if (ch>0 && ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(58,l);
	}

	/* get variable id */
	if (*p==',') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(60,l);
	if (_type>=STRINGTYPEBOX) pe(60,l);
	if (_arr) pe(60,l);
	p+=_len;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* ISO compliant procedure */
	
	/* pass over initial spaces, newlines, semicolon and commas */
	if (isEOF(ch)) pe(38,l);
	c=fgetc(channel[ch].stream);
	while (c == '\t' || c==' ' || c==CRC || c==';' || c==',' ||\
			c=='\n' || c==10 || c==13) {
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
	}

	/* fault anything except sign, digit, point or expchar */
	if (!isdigit(c) && c!='-' && c!='+' && c!='.' && c!='&' && c!='@') 
		if (ch) pe(40,l);

	/* sign found, valid indicates digit not found yet, sign indicates 
	 * whether + or -, r is value so far */ 
	if (c=='-' || c=='+') {
		sign = c=='-'?-1:1, valid=FALSE;
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
		r=0.0;
	}
	/* exponent part right now */
	if (c=='&' || c=='@') r=1.0, valid=TRUE;
	else if (c=='E' || c=='D') valid=FALSE;

	/* build integer part */
	if (isdigit(c)) { 
		while (isdigit(c)) {
			r = r * 10.0 + (c - '0');
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
			valid=TRUE;
		}
	}	
	/* the dot begins the fractional part */
	if (c=='.') {
		int f=0;
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
		while (isdigit(c)) {
			r = r + (c - '0')/pow(10.0,++f);
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
			valid=TRUE;
		}
	}
	/* the exponent symbol begins the exponent */
	if (c=='&' || c=='@' || c=='E' || c=='e') {
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
		/* the sign if present begins the sign of the exponent */
		if (c=='+' || c=='-') {
			esign = c=='-'?-1:1;
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
		}
		while (isdigit(c)) {
			exp = exp * 10 + (c - '0');
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
		}
	}
	
	/* further data treatment in case of channel different from CONSOLE */
	if (ch) {
		/* deal with further characters */
		while (c==' ' || c=='\t' || c==CRC || c==10 || c==13) {
			if (isEOF(ch)) break;
			c=fgetc(channel[ch].stream);
		}
	}
	/* restore last character read */
	if (c>0 && c!=CRC && c!=10 && c!=13 && c!=' ' && c!='\t') 
		ungetc(c,channel[ch].stream);
	
	/* fault if no digit has been found */
	if (!valid) pe(40,l);

	/* assign number so far */
	P[vn[var].ref] = (double)(sign*(r*pow(10,esign*exp)));

	/* check EOF */
	isEOF(ch);
}


/* OUTREAL() 
 * perform the OUTREAL(channel, re) procedure
 * Pass to channel the characters representing the value of re, followed by
 * a terminator.
 * Ref. ISO 1538-1984 (E) Appendix 2 "The environmental block" */
void OUTREAL(void) {
	int ch=0, n, isInt=FALSE;
	double re=0.0;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(59,l);
	}

	/* get real value */
	if (*p==',') p++;
	else pe(2,l);
	re=S();
	sysRet;

	/* if integer, print exact */
	if (re==(int)re) isInt=TRUE;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);
	
	/* ISO compliant procedure */

	/* n gives number of digits to print. It is given as a constant */
	n=10; // like DEC real (outer limit - real last=9)  ISO would be 15

	/* zero (not ISO) */
	if (fabsl(re)<REALMIN) {
		int i;
		outterminator_(ch);
		outchar_(ch,"0",1);
		outchar_(ch,".",1);
		for (i=0;i<n-1;i++) outchar_(ch,"0",1);
		outterminator_(ch);
		channel[ch].fcounter+=13;	
		return;
	}
	/* pseudo integer (not ISO) */
	else if (isInt) {
		int i, minus=' ',len;
		if (re<0) re=-re, minus='-';
		fprintf(stdout,"%c%d%n",minus,(int)re,&len);
		outchar_(ch,".",1);
		for (i=0;i<n-len+1;i++) outchar_(ch,"0",1);
		outterminator_(ch);
		channel[ch].fcounter+=12+len-1;	
		return;
	}

	/* extract sign apart */
	if (re<0.0) {
		outchar_(ch,"-",1);
		re=-re;
	}
	channel[ch].fcounter++;	
	/* infinitive (not ISO) */
	if (re>REALMAX) {
		outchar_(ch,"i",1);
		outchar_(ch,"n",1);
		outchar_(ch,"f",1);
		outterminator_(ch);
		channel[ch].fcounter+=4;	
	}
	/* print full number */
	else {
		int mi, j, k, pi;
		int flot, nines;
		
		/* mi will hold number of places point must be moved to
		 * standardize value of re to have one digit before point */
		mi = 0;
		/* standardize value of re */
		while ((re+REALOFF)>=10.0) mi++, re/=10.0;
		while ((re+REALOFF)<1.0) mi--, re*=10.0;
		/* this can occur only by rounding error, but this is
		 * a necessary safeguard */
		if (re >= 10.0) re=1.0, mi++;
		/* printing to be in exponent form */
		if (mi >= n || mi < -2) {
			flot = TRUE;
			pi = 1;
		}
		/* printing to be in non-exponent form */
		else {
			flot = FALSE;
			/* if pi = 0 point will not be printed.
			 * Otherwise point will be after pi digits */
			if (mi==n-1 || mi < 0) pi=0;
			else pi=mi+1;
			if (mi<0) {
				outchar_(ch,"0",1);
				outchar_(ch,".",1);
				channel[ch].fcounter+=2;
			}
			if (mi==-2) {
				outchar_(ch,"0",1);
				channel[ch].fcounter++;
			}
		}
		nines=FALSE;
		for (j=1;j<=n;j++) {
			/* if nines is true, all remaining digits must be 9.
			 * This can occur only by rounding error, but it is
			 * a necessary safeguard */
			if (nines) k=9;
			else {
				/* find digit to print */
				k=(int)re;
				if (k>9) k=9, nines=TRUE;
				/* move next digit to before point */
				else {
					re=10*(re-k)+REALOFF;
				}
			}
			outchar_(ch,"0123456789",k+1);
			channel[ch].fcounter++;
			if (j==pi) {
				outchar_(ch,".",1);
				channel[ch].fcounter++;
			}
		}
		/* print exponent part */
		if (flot) {
			char exp[16];
			sprintw(exp,16,"%d",mi);
			fputc(expChar, channel[ch].stream);
			channel[ch].fcounter++;
			fprintf(channel[ch].stream,"%s",exp);
			channel[ch].fcounter+=strlen(exp);
		}
		outterminator_(ch);
		channel[ch].fcounter++;
	}
}


/* INLONGREAL() 
 * perform the INLONGREAL(channel, lre) procedure
 * lre takes the value of a number read from channel. Except for the different 
 * definitions of a number and an integer the rules are exactly as for 
 * ININTEGER. Spaces or newlines may follow the symbol EXPCHAR 
 * Note: since the exponent char may be && or @@ or E (option --exp), the
 * confronting string could start with, say, E4 (for 1E4), but this would
 * be interpreted instead as a variable name. So, the exponent char is set
 * to & (the DEC symbol) whenever it is not & or @, which are unambiguous 
 * symbols. 
 * Note: not on ISO 1538 */
void INLONGREAL(void) {
	int ch=0, c,sign=1,esign=1,valid,var;
	double r=0.0;
	int exp=0;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<0 || ch>OSTREAMS) pe(44,l);
	if (ch==1) pe(76,l);
	if (ch>1 && ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(58,l);
	}
	
	/* get variable id */
	if (*p==',') p++;
	else pe(2,l);
	var=findDEC(p);
	if (!var) pe(60,l);
	if (_type>=STRINGTYPEBOX) pe(60,l);
	if (_arr) pe(60,l);
	p+=_len;

	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* ISO compliant procedure */
	
	/* pass over initial spaces, newlines, semicolon and commas */
	if (isEOF(ch)) pe(38,l);
	c=fgetc(channel[ch].stream);
	while (c == '\t' || c==' ' || c==CRC || c==';' || c==',') {
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
	}

	/* fault anything except sign, digit, point or expchar */
	if (!isdigit(c) && c!='-' && c!='+' && c!='.' && c!='&' && c!='@') 
		pe(40,l);

	/* sign found, valid indicates digit not found yet, sign indicates 
	 * whether + or -, r is value so far */ 
	if (c=='-' || c=='+') {
		sign = c=='-'?-1:1, valid=FALSE;
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
		r=0.0;
	}
	/* exponent part right now */
	if (c=='&' || c=='@') {
		int c2;
		/* read second instance of exponent */
		if (isEOF(ch)) pe(38,l);
		c2=fgetc(channel[ch].stream);
		if (c==c2) r=1.0, valid=TRUE;
		else if (isdigit(c2) || c2=='-' || c2=='+') {
			ungetc(c2,channel[ch].stream);
			r=1.0, valid=TRUE;
		}
		else valid=FALSE;
		c=c2;
	}
	else if (c=='E' || c=='D') valid=FALSE;

	/* build integer part */
	if (isdigit(c)) { 
		while (isdigit(c)) {
			r = r * 10.0 + (c - '0');
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
			valid=TRUE;
		}
	}	
	/* the dot begins the fractional part */
	if (c=='.') {
		int f=0;
		if (isEOF(ch)) pe(38,l);
		c=fgetc(channel[ch].stream);
		while (isdigit(c)) {
			r = r + (c - '0')/pow(10.0,++f);
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
			valid=TRUE;
		}
	}
	/* the exponent symbol begins the exponent */
	if (c=='&' || c=='@' || c=='E' || c=='D') {
		int c2;
		if (isEOF(ch)) pe(38,l);
		c2=fgetc(channel[ch].stream);
		if (c==c2) {
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
		}
		else c=c2;
		/* the sign if present begins the sign of the exponent */
		if (c=='+' || c=='-') {
			esign = c=='-'?-1:1;
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
		}
		while (isdigit(c)) {
			exp = exp * 10 + (c - '0');
			if (isEOF(ch)) pe(38,l);
			c=fgetc(channel[ch].stream);
		}
	}
	
	/* further data treatment in case of channel different from CONSOLE */
	if (ch) {
		/* deal with further characters */
		while (c==CRC || c==10 || c==13 || c==' ' || c=='\t') {
			if (isEOF(ch)) break;
			c=fgetc(channel[ch].stream);
		}
	}
	
	/* restore last character read if not discardable */
	if (c>0 && c!=CRC && c!=10 && c!=13 && c!=' ' && c!='\t') 
		ungetc(c,channel[ch].stream);
	
	/* fault if no digit has been found */
	if (!valid) pe(40,l);
	
	/* assign number so far */
	P[vn[var].ref] = (double)(sign*(r*pow(10,esign*exp)));

	/* check EOF */
	isEOF(ch);
}


/* OUTLONGREAL() 
 * perform the OUTLONGREAL(channel, lre) procedure
 * Pass to channel the characters representing the value of lre, followed by
 * a terminator.
 * Note: not on ISO 1538 */
void OUTLONGREAL(void) {
	int ch=0, n,  isInt=FALSE;
	long double re=0.0;

	/* get channel number */
	if (*p=='(') p++;
	else pe(2,l);
	ch=(int)S();
	if (isCDC && ch>=60) ch-=60;
	sysRet;
	if (ch<1 || ch>OSTREAMS) pe(44,l);
	if (ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VREAD)) pe(59,l);
	}

	/* get long real value */
	if (*p==',') p++;
	else pe(2,l);
	re=(long double)S();
	sysRet;
	
	/* close reading */
	if (*p==')') p++;
	else pe(2,l);

	/* if integer, print exact */
	if (re==(int)re) isInt=TRUE;

	/* ISO compliant procedure */

	/* n gives number of digits to print. It is given as a constant */
	n=18;  // like DEC long real (outer limit - real last=17)

	/* zero (not ISO) */
	if (fabsl(re)<LONGREALMIN) {
		int i;
		outterminator_(ch);
		outchar_(ch,"0",1);
		outchar_(ch,".",1);
		for (i=0;i<n-1;i++) outchar_(ch,"0",1);
		outterminator_(ch);
		channel[ch].fcounter+=21;
		return;	
	}
	/* pseudo integer (not ISO) */
	else if (isInt) {
		int i,minus=' ', len;
		if (re<0) re=-re, minus='-';
		fprintf(channel[ch].stream,"%c%d%n",minus,(int)re,&len);
		outchar_(ch,".",1);
		for (i=0;i<n-len+1;i++) outchar_(ch,"0",1);
		outterminator_(ch);
		channel[ch].fcounter+=19+len-1;
		return;	
	}

	/* extract sign apart */
	if (re<0.0) {
		outchar_(ch,"-",1);
		re*=-1;
	}
	channel[ch].fcounter++;	

	/* infinitive (not ISO) */
	if (re>LONGREALMAX) {
		outterminator_(ch);
		outchar_(ch,"i",1);
		outchar_(ch,"n",1);
		outchar_(ch,"f",1);
		outterminator_(ch);
		channel[ch].fcounter+=5;	
	}

	/* print full number */
	else {
		int mi, j, k, pi;
		int flot, nines;
		
		/* mi will hold number of places point must be moved to
		 * standardize value of re to have one digit before point */
		mi=0;
		/* standardize value of re */
		while (re>=10.0L) mi++, re/=10.0L;
		while (re<1.0L) mi--, re*=10.0L;

		/* this can occur only by rounding error, but this is
		 * a necessary safeguard */
		if (re >= 10.0L) re=1.0L, mi++;
		/* printing to be in exponent form */
		if (mi >= n || mi < -2) {
			flot = TRUE;
			pi = 1;
		}
		/* printing to be in non-exponent form */
		else {
			flot = FALSE;
			/* if pi = 0 point will not be printed.
			 * Otherwise point will be after pi digits */
			if (mi==n-1 || mi < 0) pi=0;
			else pi=mi+1;
			if (mi<0) {
				outchar_(ch,"0",1);
				outchar_(ch,".",1);
				channel[ch].fcounter+=2;
			}
			if (mi==-2) outchar_(ch,"0",1);
		}
		/* proceed */
		nines=FALSE;
		for (j=1;j<=n;j++) {
			/* if nines is true, all remaining digits must be 9.
			 * This can occur only by rounding error, but it is
			 * a necessary safeguard */
			if (nines) k=9;
			else {
				/* find digit to print */
				k=(int)re;
				if (k>9) k=9, nines=TRUE;
				/* move next digit to before point */
				else {
					re=10.0L*(re-k)+POWL(10L,j-n);
				}
			}
			outchar_(ch,"0123456789",k+1);
			channel[ch].fcounter++;
			if (j==pi) outchar_(ch,".",1);
		}
		/* print exponent part */
		if (flot) {
			char exp[16];
			int len=(expChar!='&' && expChar!='@')?1:2;
			sprintw(exp,16,"%d",mi);
			if (len==1) {
				fputc(expChar, channel[ch].stream);
				channel[ch].fcounter++;
			}
			else {
				fputc(expChar, channel[ch].stream);
				fputc(expChar, channel[ch].stream);
				channel[ch].fcounter+=2;
			}
			fprintf(channel[ch].stream,"%s",exp);
			channel[ch].fcounter+=strlen(exp);
		}
		outterminator_(ch);
		channel[ch].fcounter++;
	}
}


/***************************************************************************
 * SWITCH procedure
 ***************************************************************************/

/* getLabelIndex()
 * Analyze the string in current position, detecting if a label is there;
 * a label may be:
 * - a literal label: it's passed through setAddr()
 * - a label variable: its count is passed
 * - a switch: the index is read, the string is passed through setAddr()
 * In case of error, return 0 (not found) or a negative error code.
 * Otherwise, return label code
 * System function for GOTO()/assign() */
int getLabelIndex(void) {
	int ns=0;
	int res=0;
	int type;
	/* designational expression: literal label which begins with IF */
	if (!strncasecmp_(p,"IF",2)) {
		int cond=FALSE;
		p+=2;
		if (!check(p,"THEN")) return -2;
		cond=(int)S();
		if (*p==' ') *p='T';
		p+=4;
		if (!strncasecmp_(p,"IF",2)) return -22;
		if (!check(p,"ELSE")) return -2;
		if (cond) res=getLabelIndex();
		else {
			p=strcasestr_oq(p," LSE"); // advance till next ELSE
			if (*p==' ') *p='E';
			p+=4;
			res=getLabelIndex();
		}
	}
	/* label or string procedure */
	else if ((ns=findPROC(p,NULL,&type))) {
		double numres;
		char labelText[COMPSTR];
		if (type<STRINGTYPEBOX) return -11;
		p+=_len;
		execPROC(ns,p,&numres,labelText);
		res=numres;
	}
	/* label variable */
	else if ((ns=findDEC(p)) && !_arr) {
		if (_type==LABELTYPEBOX) {
			ns=vn[ns].ref;
			p+=_len;
			res=P[ns];
		}
		/* other variable */
		else res=-11;
	}
	/* switch item */
	else if ((ns=findDEC(p)) && _arr) {
		int val[2];
		char labelText[COMPSTR], *pb;
	       	if (_type==LABELTYPEBOX) {
			int ind=0;
			p+=_len;
			if (*p=='[') p++;
			ind=S();
			if (*p==']') p++;
			pb=p;
			val[0]=0;val[1]=ind;
			strncpy_(labelText,getArrayStr(ns,1,val),COMPSTR);
			p=labelText;
			res=getLabelIndex();
			p=pb;
		}
		else res=-11;
	}
	/* case of a literal with ELSE inside; check that is an effective
	 * label name and not a case such THEN GOTO <label> ELSE GOTO ...*/
	else if (strcasestr_oq(p,"ELSE")) {
		int code=0, count=0;
		char labelText[COMPSTR], *k=labelText;
		while (isnamechar(p)) *k++=*p++;
		*k='\0';
		code=setAddr(labelText,&k);
		while (*p && *p!=';' && strncasecmp_(p,"END",3)) p++;
		/* if found label, return its count */
		for(count=1;count<=labelTOS;count++) {
			if (label[count].code==code) {
				res=count;
			}
		}
		/* label not found: it's an ELSE case */
		if (!res) {
			char *pp;
			pp=strcasestr_(labelText,"ELSE");
			*pp='\0';
			code=setAddr(labelText,&k);
			while (*p && *p!=';' && strncasecmp_(p,"END",3)) p++;
			/* if found label, return its count */
			for(count=1;count<=labelTOS;count++) {
				if (label[count].code==code) {
					res=count;
				}
			}
			if (!res) res=-11;
		}
	}
	/* literal label which does not begin with IF */
	else {
		int code=0, count=0;
		char labelText[COMPSTR], *k=labelText;
		while (isnamechar(p)) *k++=*p++;
		*k='\0';
		code=setAddr(labelText,&k);
		while (*p && *p!=';' && strncasecmp_(p,"END",3)) {
			if (*p=='.') p++;
			p++;
		}
		/* if found label, return its count */
		for(count=1;count<=labelTOS;count++) {
			if (label[count].code==code) {
				res=count;
			}
		}
		/* label not found */
		if (!res) res=-11;
	}
	return res;
}


/* findInstance()
 * find if an instance of a variable/array name was already declared elsewhere.
 * Used in findDEC, array, SWITCH() 
 * return the variable index if found, zero otherwise */
int findInstance(char *name) {
	int i=0;
	while (i<=decCount) {
		if (!strcmp(vn[i].name,name)) {
			if (pexecTOS==vn[i].level) return i;
		}
		i++;
	}
	return 0;
}


/* SWITCH()
 * reads the label in the stream queue, solve their address and store the
 * reference in an 1-dim array of strings, created with the given name.
 * Ref. AA-O196C-TK - 12 "SWITCHES" */
void SWITCH(void) {
	char decname[NAMEDIM], *sp=decname;
	int end=0, i=0,td, another=0;
	char temp[16][NAMEDIM];
	int val[2];

	/* get switch name */
	if (isbeginnamechar(p)) {
		while (isnamechar(p) && i<NAMEDIM) {
			if (*p=='.') {
				p++;
				continue;
			}
			*sp++=*p++;
			i++;
		}
		*sp='\0';
	}
	else pe(2,l);
	/* check for too long identifiers */
	if (i>=NAMEDIM && isnamechar(p-1)) pe(26,l);
	
	/* found if there is another instance */
	//another=findDEC(decname);
	another=findInstance(decname);
	if (another) {
		fprintf(stderr,error[99],decname,l,vn[another].line);
	}

	/* name must be read all */
	if (*p!=PU) pe(55,l);
	else p++;
	
	/* get and store elements, updating 'end' (which counts items) */
	while (*p!=';') {
		char labelText[NAMEDIM+1], *k=labelText;
		if (isbeginnamechar(p)) {
			end++;
			/* store IF-THEN-ELSE clause */
			if (!strncasecmp_(p,"IF",2)) {
				while (strncasecmp_(p,"ELSE",2)) *k++=*p++;
		       		while (isnamechar(p) || *p=='[' || *p==']')
					*k++=*p++;
			}
			/* store label, label var or switch item */
			else while (isnamechar(p) || *p=='[' || *p==']') 
				*k++=*p++;
			*k='\0';
			strncpy_(temp[end],labelText,NAMEDIM+1);
			if (*p==',') p++;
		}
	}


	/* declare label var */
	decCount++;
	if (decCount>=LIMIT) pe(7,l);
	td=decCount;
	vn[td].type=LABELTYPEBOX;
	vn[td].dim=1;
	vn[td].ref=decCount;
	vn[td].refcode=decCount;
	vn[td].proccode=0;
	vn[td].isArr=TRUE;
	vn[td].isFunc=FALSE;
	vn[td].isFict=FALSE;
	vn[td].ldim[1]=end-1;
	vn[td].offset[1]=1;
	vn[td].coeff[1]=1;
	vn[td].isOwn=FALSE;
	vn[td].strmem=ALGSTR;
	vn[td].line=l;
	vn[td].level=pexecTOS;
	for (i=0;i<COMPSTR;i++) vn[td].valdata[i]=vn[td].strdata[i]='\0';
	vn[td].size=dimStrArray(td,1,vn[td].ldim);
	
	/* set name/type */
	strncpy_(vn[td].name,decname,NAMEDIM);

	/* copy back stored temporary values */
	for (i=1;i<=end;i++) {
		val[0]=0; val[1]=i;
		setArrayStr(td,1,val,temp[i]);
	}

	/* debug features */
	if (isDebug) showDEC(td, "Declared switch ");
}


/***************************************************************************
 * FAULT ISO procedure
 ***************************************************************************/

/* FAULT()
 * user error routine, fully compliant to what is defined in the Appendix 2 of 
 * the ISO 1538 document, with extensions.
 * This error routine can be called from the ALGOL source, printing a default
 * text string with a user error message.
 * FAULT(string str[, real r])
 * FAULT(integer int[, real r])
 * - string str as the first argument is the user error string. 
 * - the optional number as second argument is the number that generated the 
 * error. In order to print only the string, the second argument can be omitted 
 * (this enhancement was not present in the ISO 1538 document).
 * FAULT(errcode)
 * - if the integer errcode is provided as the first and unique argument, it 
 *   is used to print the system error message associated to the number itself.
 * (this enhancement was not present in the ISO 1538 document).
 * Ref. ISO 1538-1984 - "The Environmental block" */
void FAULT(void) {
	char errs[COMPSTR];
	double errn=0;
	int strOnly=TRUE, var=0, errCODE=0;
	
	/* get string */
	if (*p=='(') p++;
	else pe(2,l);
	
	/* print immediately a literal numerical error code */
	if (*p>='0' && *p<='9') {
		errCODE=(int)S();
		sysRet;
		strncpy_(errs,error[errCODE],COMPSTR);
	}
	/* evaluate numerical var or string */
	else {
		/* find variable */
		var=findDEC(p);
		/* numeric var */
		if (var && _type<STRINGTYPEBOX) {
			errCODE=(int)S();
			sysRet;
			strncpy_(errs,error[errCODE],COMPSTR);
		}
		/* string variable or literal string */
		else strncpy_(errs,SS(),COMPSTR);
		sysRet;
	}
	
	/* get numerical entity */
	if (*p==',') {
		p++;
		errn=S();
		sysRet;
		strOnly=FALSE;
	}
	
	/* complete reading */
	if (*p==')') p++;
	else pe(2,l);

	/* print error message (and number if required) */
	if (strOnly) fprintf(stderr,"fault  %s\n",errs);
	else fprintf(stderr,"fault  %s  %f\n",errs,errn);

	/* exit with proper error code */
	if (errCODE) END(errCODE);
	else END(EXIT_FAILURE);
}


/***************************************************************************
 * DUMP and TRACE procedures (ONTRACE / OFFTRACE)
 ***************************************************************************/

/* TRACE()
 * implements the ONTRACE/OFFTRACE procedures 
 * mode: if TRUE execute ONTRACE; if FALSE execute OFFTRACE 
 * Ref. AA-O196C-TK - 17.9 "ONTRACE AND OFFTRACE" */
void TRACE(int mode) {
	if (mode) {
		if (!isDebug) {
			fprintf(stdout,"%s",INVCOLOR);
			fprintf(stdout,"%s","\n<Trace enabled>\n");
			fprintf(stdout,"%s",RESETCOLOR);
			stopKey=FALSE;
		}
		isDebug=TRUE;
	}
	else {
		if (isDebug) {
			fprintf(stdout,"%s",INVCOLOR);
			fprintf(stdout,"%s","\n<Trace disabled>\n");
			fprintf(stdout,"%s",RESETCOLOR);
		}
		isDebug=FALSE;
	}
}


/* level()
 * return level of line in argument, finding the corresponding BEGIN/END block;
 * the main BEGIN/END block has level 1
 * varn: the program line to check for.
 * System function used by DUMP */
int level(int varn) {
	int j=beginTOS;
	while (j>0) {
		if (varn>beginDepth[j]) return beginTOS-j+1;
		else j--;	
	}
	return 0;
}


/* DUMP 
 * Perform the DUMP procedure
 * Ref. AA-O196C-TK - 17.11 "DUMP"
 * Ref. AA-O196C-TK - 20.7 "DUMP" (debugging system) */
void DUMP(void) {
	int j, n=1,ce=FALSE;
	char output[COMPSTR];

	fprintf(stdout,"%s",CRS);
	/* there's the argument: depth */
	if (*p=='(') {
		p++;
		n=(int)S();
		sysRet;
		if (*p==')') p++;
	}
	/* ALL flag */
	else if (!strncasecmp_(p,"ALL",3)) {
		p+=3;
		n=beginTOS;
	}
	/* or else search for the variable */
	else if (*p && *p!=';') {
		char *c=output;
		int cn=0;
		if (*p==',') p++;
		while (isnamechar(p)) *c++=*p++;
		*c='\0';
		cn=findDEC(output);
		if (!cn) pe(62,l);
		sprintw(output,COMPSTR,"Level:%d  Name:",level(cn));
		showDEC(cn,output);
		if (*p==',') {
			p++;
			DUMP();
		}
		return;
	}

	/* print variables levels */
	for (j=beginDepth[beginTOS-n+1]+1;j<=decCount;j++) {
		sprintw(output,COMPSTR,"Level:%d  Name:",level(j));
		showDEC(j,output);
		ce=TRUE;
	}

	/* in case no print was done, print an informative message */
	if (!ce) {
		fprintf(stdout,"%s",INVCOLOR);
		fprintf(stdout,"DUMP: no variables in line %d level.\n",l);
		fprintf(stdout,"%s",RESETCOLOR);
	}

	/* repeat, if there is something afterwards */
	fprintf(stdout,"%s",CRS);
	if (*p==',') {
		p++;
		DUMP();
	}
}


/***************************************************************************
 * PRINTOCTAL and READOCTAL aid procedures (see PRINT and READ)
 ***************************************************************************/

/* fprinto()  
 * print octal value in string str
 * if type = LONGREALTYPEBOX uses two figures
 * dval is the value to return in sres 
 * Ref. AA-O196C-TK - 16.6.4.3 "Octal Input/Output"*/
void fprinto(char *str, double dval, int type) {
	int it, q=0;
	char *istr, *sstr;
	istr=sstr=str;
	if (dval<0) dval*=-1;
	/* normal case */
	if (type>LONGREALTYPEBOX && type<STRINGTYPEBOX) {
		int val=(int)dval;
		*sstr++='%';
		while (val) {
			*sstr++='0' + (val % 8);
			val/=8;
			q++;
		}
		while (q<11) *sstr++='0', q++;
		*sstr='\0';
		/* reverse result string */
		sstr--; istr++;
		while (q>6) 
			it=*istr, *istr=*sstr, *sstr=it, q--, istr++, sstr--;
	}
	/* long real case */
	else if (type==LONGREALTYPEBOX) {
		int high=(int)(dval/2147483647);
		int val=dval-high*2147483647;
		*sstr++='%';
		/* first half */
		while (val) {
			*sstr++='0' + (val % 8);
			val/=8;
			q++;
		}
		while (q<11) *sstr++='0', q++;
		*sstr++=' '; q++;
		/* second half */
		while (high) {
			*sstr++='0' + (high % 8);
			high/=8;
			q++;
		}
		while (q<23) *sstr++='0', q++;
		*sstr='\0';
		/* reverse result string */
		sstr--; istr++;
		while (q>13) 
			it=*istr, *istr=*sstr, *sstr=it, q--, istr++, sstr--;
	}
	else *str='\0';

	/* add terminator (a space) */
	if (*str) {
		sstr=str+strlen(str);
		*sstr++=' ';
		*sstr='\0';
	}
}


/* getOctal()
 * retrieve the octal number(s) contained from string f in argument.
 * If type is Boolean, Integer or Real, the number is %nnnnn
 * If type is Long Real, the number is %nnnn [%]nnnn
 * Ref. AA-O196C-TK - 16.6.4.3 "Octal Input/Output"
 * System function used by READOCTAL */
double getOctal(char *f, int type, int *pos) {
	char base[COMPSTR], *b=base, *fb=f;
	int val1=0, val2=0;
	if (*f!='%') pe(2,l);
	else f++;
	while (*f>='0' && *f<='7' && (b-base)<11) *b++=*f++;
	*b='\0';
	val1=strtol(base,NULL,8);
	if (type==LONGREALTYPEBOX) {
		b=base;
		while (isblank(*f)) f++;
		if (*f=='%') f++;
		while (*f>='0' && *f<='7' && (b-base)<11) *b++=*f++;
		*b='\0';
		val2=strtol(base,NULL,8);
		return (double)(val1*2147483647+val2);
	}
	*pos=(int)(f-fb);
	return (double) val1;
}


/***************************************************************************
 * FOR-DO and WHILE-DO procedures, plus BREAK and CONTINUE
 ***************************************************************************/

void reload(void) {
	/* reload */
	if (!*p) {
		l++;
		while (!m[l]) l++;
		strncpy_(BASESTR,m[l],COMPSTR);
		p=BASESTR;
	}
}


/* WHILE()
 * Perform the WHILE-DO procedure;
 * Ref. AA-O196C-TK - 8 "FOR AND WHILE STATEMENTS" */
void WHILE(void) {
	char *k;
	int locTOS=1, cond;
	
	/* upgrade counter */
	whileExecTOS++;
	
	reload();
	while (Whilep[locTOS].varpL!=l||Whilep[locTOS].varpP!=(int)(p-BASESTR)){
		locTOS++;
		if (locTOS>whileTOS) pe(7,l);
	}
	
	reload();
	k=Whilep[locTOS].cond;
	reload();
	check(p,"DO");
	while(*p && *p!=' ') *k++=*p++;
	*k='\0';
	p=Whilep[locTOS].cond;
	cond=(int)S();
	sysRet;
	*p='D';
	while (cond) {
		/* execute current type 1 */
		strncpy_(BASESTR,m[Whilep[locTOS].doexL],COMPSTR);
		p=BASESTR+Whilep[locTOS].doexP;
		l=Whilep[locTOS].doexL;
		go(l);
		sysRet;
		p=Whilep[locTOS].cond;
		cond=S();
		sysRet;
	}
	l=Whilep[locTOS].termL;
	p=m[l]+Whilep[locTOS].termP;
	
	/* upgrade counter */
	if (whileExecTOS>0) whileExecTOS--;
}


void kcopy(char *t) {
	while (*p && *p!=',' && *p!=' ') {
		if (*p=='(') {
			int st=0;
			*t++=*p++; // copy (
			while (*p) {
				if (*p=='(') st++;
				else if (*p==')' && st) st--;
				else if (*p==')' && !st) break;
				*t++=*p++;
			}
		}
		else if (*p=='[') {
			int st=0;
			*t++=*p++; // copy [
			while (*p) {
				if (*p=='[') st++;
				else if (*p==']' && st) st--;
				else if (*p==']' && !st) break;
				*t++=*p++;
			}
		}
		*t++=*p++;
	}	
}


/* FOR()
 * Perform the FOR-DO procedure;
 * Ref. AA-O196C-TK - 8 "FOR AND WHILE STATEMENTS" */
void FOR(void) {
	TForItem foritem[65];
	int fi=0, ns=0, i; // type = 1,2,3
	double step,end;
	double prev;
	int locTOS=1;

	for (i=0;i<65;i++) {
		foritem[i].epos[0]='\0';
		foritem[i].val[0]='\0';
		foritem[i].cond[0]='\0';
		foritem[i].step[0]='\0';
	}

	/* search for related FOR cycle */
	reload();
	while (locTOS<=forTOS) {
		if (Forp[locTOS].varpL==l &&\
				(int)(p-BASESTR)==Forp[locTOS].varpP) break;
		locTOS++;
       		if (locTOS>forTOS) pe(90,l);
	}		

	/* upgrade counter */
	forExecTOS++;

	reload();
	ns=findDEC(p);
	if (!ns) pe(60,l);
	/* get to the original referenced item */
	while (ns!=vn[ns].ref) ns=vn[ns].ref;
	p+=_len;
	if (vn[ns].type>INTEGERTYPEBOX) pe(60,l);
	if (vn[ns].isArr) pe(60,l);
	reload();
	if (*p==PU) p++;
	else pe(16,l);


	/* gather FOR indices */
	do {
		if (!strncasecmp_(p," O",2) || !strncasecmp_(p,"DO",2)) {
			*p='D';
			p+=2;
			break;
		}
		fi++;
		if (fi>64) pe(7,l);
		reload();
		check(p,"STEP");
		check(p,"WHILE");
		check(p,"UNTIL");
		check(p,"DO");
		kcopy(foritem[fi].val);
		reload();
		check(p,"STEP");
		check(p,"WHILE");
		check(p,"UNTIL");
		check(p,"DO");
		/* type 1: FOR list (Ref 8.1 point 1) */
		if (*p==',' || !strncasecmp_(p," O",2) ||\
			       !strncasecmp_(p,"DO",2)) {
			foritem[fi].type=1;
			if (*p==',') p++;
		}
		/* type 2 shortened: FOR UNTIL (Ref 8.1, Note) */
		else if (!strncasecmp_(p," NTIL",5)) {
			foritem[fi].type=2;
			strncpy_(foritem[fi].step,"1",COMPSTR);
			*p='U';
			p+=5; // skip UNTIL
			reload();
			check(p,"DO");
			kcopy(foritem[fi].cond);
			if (*p==',') p++;
		}
		/* type 2: FOR STEP UNTIL (Ref 8.1 point 2) */
		else if (!strncasecmp_(p," TEP",4)) {
			foritem[fi].type=2;
			*p='S'; // skip STEP
			p+=4;
			reload();
			check(p,"UNTIL");
			kcopy(foritem[fi].step);
			reload();
			check(p,"UNTIL");
			if (!strncasecmp_(p," NTIL",5)) {
				*p='U';
				p+=5; // skip UNTIL
				reload();
				check(p,"DO");
				kcopy(foritem[fi].cond);
			}
			if (*p==',') p++;
		}
		/* type 3: FOR WHILE (Ref 8.1 point 3) */
		else if (!strncasecmp_(p," HILE",5)) {
			*p='W';
			p+=5; // skip WHILE
			foritem[fi].type=3;
			strncpy_(foritem[fi].epos,foritem[fi].val,COMPSTR);
			reload();
			check(p,"DO");
			kcopy(foritem[fi].cond);
			if (*p==',') p++;
		}
		
	} while (*p && strncasecmp_(p,"DO",2));

	i=1;
	/* perform the FOR procedure */
	while (i<=fi) {
		/* execute current type 2: STEP UNTIL */
		if (foritem[i].type==2) {
			p=foritem[i].val;
			P[ns]=S();
			sysRet;
			p=foritem[i].cond;
			end=S();
			sysRet;
			p=foritem[i].step;
			step=S();
			sysRet;
			if (!step) step=1;
			if (step>0) while (P[ns]<=end) {
				prev=P[ns];
				/* execute current type 1 - positive step */
				strncpy_(BASESTR,m[Forp[locTOS].doexL],COMPSTR);
				p=BASESTR+Forp[locTOS].doexP;
				l=Forp[locTOS].doexL;
				go(l);
				sysRet;
				p=foritem[i].step;
				step=S();
				sysRet;
				P[ns]+=step;
				p=foritem[i].cond;
				end=S();
				sysRet;
			}
			else while (P[ns]>=end) {
				prev=P[ns];
				/* execute current type 1 - negative step */
				strncpy_(BASESTR,m[Forp[locTOS].doexL],COMPSTR);
				p=BASESTR+Forp[locTOS].doexP;
				l=Forp[locTOS].doexL;
				go(l);
				sysRet;
				p=foritem[i].step;
				step=S();
				sysRet;
				P[ns]+=step;
				p=foritem[i].cond;
				end=S();
				sysRet;
			}
			P[ns]=prev;
			sysRet;
		}
		/* execute current type 3: WHILE */
		else if (foritem[i].type==3) {
			int cond;
			p=foritem[i].epos;
			P[ns]=S();
			sysRet;
			p=foritem[i].cond;
			cond=(int)S();
			sysRet;
			while (cond) {
				prev=P[ns];
				/* execute current type 1 */
				strncpy_(BASESTR,m[Forp[locTOS].doexL],COMPSTR);
				p=BASESTR+Forp[locTOS].doexP;
				l=Forp[locTOS].doexL;
				go(l);
				sysRet;
				p=foritem[i].epos;
				P[ns]=S();
				p=foritem[i].cond;
				cond=S();
				sysRet;
			}
			P[ns]=prev;
			sysRet;
		}
		/* execute current type 1: LIST */
		else if (foritem[i].type==1) {
			p=foritem[i].val;
			P[ns]=S();
			sysRet;
			strncpy_(BASESTR,m[Forp[locTOS].doexL],COMPSTR);
			p=BASESTR+Forp[locTOS].doexP;
			l=Forp[locTOS].doexL;
			go(l);
			sysRet;
			prev=P[ns];
		}
		i++;
	}
	strncpy_(BASESTR,m[Forp[locTOS].termL],COMPSTR);
	p=BASESTR+Forp[locTOS].termP;
	l=Forp[locTOS].termL;
	/* upgrade counter */
	if (forExecTOS>0) forExecTOS--;
}


/***************************************************************************
 * GOTO and IF-THEN-ELSE procedure
 ***************************************************************************/

/* ELSE()
 * This is a dummy function that prints an error message in case ELSE is
 * invoked out of an IF structure 
 * System function for exec() */
void ELSE(void) {
	pe(21,l);
}


/* GOTO()
 * Perform the GOTO unconditional procedure by setting two global references
 * (LL and PL) which will alert doBegin() in order to set the new address.
 * Ref. AA-O196C-TK - 7.2 "UNCONDITIONAL CONTROL TRANSFERS" */
void GOTO(void) {
	int count=0;

	if (!*p) {
		l++;
		while (!m[l]) l++;
		if (l>endCore) pe(20,l);
		strncpy_(BASESTR,m[l],COMPSTR);
		p=BASESTR;
	}
	
	/* get label index and set GOTO position */
	count=getLabelIndex();
	if (count>0) {
		int i;
		LL=label[count].ll;
		PL=label[count].pl;
		/* discard all BEGIN/IF/FOR/WHILE references of the cycles
		 * that we are leaving, by searching the right position */
		/* search for IF */
		for (i=ifExecTOS;i>0;i--) {
			if (Ifp[i].condL>LL || Ifp[i].termL<LL) { 
				if (ifExecTOS>0) ifExecTOS--;
			}
			else if (Ifp[i].condL==LL) {
				if (Ifp[i].condP>PL) {
					if (ifExecTOS>0) ifExecTOS--;
				}
			}
			else if (Ifp[i].termL==LL) {
				if (Ifp[i].termP<PL) {
					if (ifExecTOS>0) ifExecTOS--;
				}
			}
			else break;
		}
		/* search for FOR */
		for (i=forExecTOS;i>0;i--) {
			if (Forp[i].varpL>LL || Forp[i].termL<LL) {
				if (forExecTOS>0) forExecTOS--;
			}
			else if (Forp[i].varpL==LL) {
				if (Forp[i].varpP>PL) {
					if (forExecTOS>0) forExecTOS--;
				}
			}
			else if (Forp[i].termL==LL) {
				if (Forp[i].termP<PL) {
					if (forExecTOS>0) forExecTOS--;
				}
			}
			else break;
		}
		/* search for WHILE */
		for (i=whileExecTOS;i>0;i--) {
			if (Whilep[i].varpL>LL || Whilep[i].termL<LL) {
				if (whileExecTOS>0) whileExecTOS--;
			}
			else if (Whilep[i].varpL==LL) {
				if (Whilep[i].varpP>PL) {
					if (whileExecTOS>0) whileExecTOS--;
				}
			}
			else if (Whilep[i].termL==LL) {
				if (Whilep[i].termP<PL) {
					if (whileExecTOS>0) whileExecTOS--;
				}
			}
			else break;
		}
		/* search for BEGIN */
		for (i=beginTOS;i>0;i--) {
			if (Beginp[i].varpL>LL || Beginp[i].termL<LL) {
				if (beginTOS>0) beginTOS--;
			}
			else if (Beginp[i].varpL==LL) {
				if (Beginp[i].varpP>PL) {
					if (beginTOS>0) beginTOS--;
				}
			}
			else if (Beginp[i].termL==LL) {
				if (Beginp[i].termP<PL) {
					if (beginTOS>0) beginTOS--;
				}
			}
			else break;
		}
		if (isDebug) {
			fprintf(stdout,"%s",INVCOLOR);
			fprintf(stdout,"Jumping to line=%d, pos=%d\n",LL,PL);
			fprintf(stdout,"%s",RESETCOLOR);
		}
	}
	else if (count<=0) pe(11,l);
	/* if LL is empty, no label was found */
	if (!LL) pe(11,l);
}


/* IF()
 * Perform the IF-THEN-ELSE conditional statement.
 * Ref. AA-O196C-TK - 7.3 "CONDITIONAL STATEMENT" */
void IF(void) {
	char *k=p;
	int cond, locTOS=0;

	/* upgrade counter */
	ifExecTOS++;

	/* reload */
	if (!*p) {
		l++;
		while (!m[l]) l++;
		strncpy_(BASESTR,m[l],COMPSTR);
		p=BASESTR;
	}
		
	/* search the IF clause in this place */
	while (Ifp[locTOS].condL!=l || strcmp(m[l]+Ifp[locTOS].condP,p)){
		locTOS++;
		if (locTOS>ifTOS) pe(7,l);
	}

	/* calculate condition*/
	check(p,"THEN");
	/* Note: strRel() (for string comparisons) is calculated in S() */
	cond=(int)S();
	sysRet;
	*p='T';
	if (*p==')') p++;

	/* in case the evaluated condition is true, execute the THEN part */
	if (cond) {
		strncpy_(BASESTR,m[Ifp[locTOS].thenL],COMPSTR);
		p=BASESTR+Ifp[locTOS].thenP;
		/* ELSE is on the same line, prepare for string evaluation */
		if (Ifp[locTOS].elseL==l) {
			k=BASESTR+Ifp[locTOS].elseP-4; // regain ELSE
			*k=' ';
		}
		if (!strncasecmp_(p,"IF",2)) pe(22,l);
		if (!strncasecmp_(p,"FOR",3)) pe(22,l);
		if (!strncasecmp_(p,"WHILE",5)) pe(22,l);
		l=Ifp[locTOS].thenL;
		/* debugging the THEN part */
		if (isDebug && isVerbose) {
			char output[COMPSTR], *o=output;
			char *pb=p;
			/* check ELSE against " LSE" because the
			 * line is the tokenized-blank-stripped one */
			while (*pb && strncasecmp_(pb," LSE",4) && *p!=';')
				*o++=*pb++;
			*o='\0';
			fprintf(stdout,"%s",INVCOLOR);
			fprintf(stdout,"\n[%s]\n",output);
			fprintf(stdout,"%s",RESETCOLOR);
		}
		go(l);
		sysRet;
	}
	/* in case the evaluated condition is false and there is an ELSE clause,
	 * execute the ELSE part */
	else if (Ifp[locTOS].elseL>0) {
		strncpy_(BASESTR,m[Ifp[locTOS].elseL],COMPSTR);
		p=BASESTR+Ifp[locTOS].elseP;
		if (!strncasecmp_(p,"FOR",3)) pe(22,l);
		if (!strncasecmp_(p,"WHILE",5)) pe(22,l);
		l=Ifp[locTOS].elseL;
		/* debugging the ELSE part */
		if (isDebug && isVerbose) {
			char output[COMPSTR], *o=output;
			char *pb=p;
			while (*pb && *p!=';')
				*o++=*pb++;
			*o='\0';
			/* No check here, because the text is beyond ELSE */
			fprintf(stdout,"%s",INVCOLOR);
			fprintf(stdout,"\n[%s]\n",output);
			fprintf(stdout,"%s",RESETCOLOR);
		}
		go(l);
		sysRet;
	}
	/* at the end jump to the IF terminator */
	l=Ifp[locTOS].termL;
	strncpy_(BASESTR,m[l],COMPSTR);
	p=BASESTR+Ifp[locTOS].termP;
	
	/* upgrade counter */
	if (ifExecTOS>0) ifExecTOS--;
}
	

/***************************************************************************
 * READ procedure
 ***************************************************************************/

/* isEOF()
 * return TRUE if current file has passed the last character/item, or FALSE.
 * Note: it does NOT control if stream is open.
 * ch: the testing channel id 
 * System function for the file management */
int isEOF(int ch) {
	int res=FALSE, i=0;
	/* test for file open of file set for output */
	if (channel[ch].wayout!=VREAD) return res;
	if (!channel[ch].isopen) return res;
	/* test for channel range */
	if (ch<2) return res;
	else if (ch>STREAMS) return res;
	else {
		i=fgetc(channel[ch].stream);
		if (i<0 /*&& !*rb*/) res=TRUE, channel[ch].iochan+=0100;
		ungetc(i,channel[ch].stream);
	}
	return res;
}


/* getString()
 * ch: channel input number; if null, get string from keyboard
 * System function for the READ procedure */
char *getString(int ch) {
	char *lp=line;
	int i;

	/* get string */
	for (i=0; i<SCREENLINE; i++) line[i]='\0';
	while (TRUE) {
		if (!ch || channel[ch].iochan & 0100000) *lp=getchar();
		else {
			if (!channel[ch].isopen) pe(14,l);
			if (!(channel[ch].wayout & VREAD)) 
				pe(58,l);
			*lp=fgetc(channel[ch].stream);
		}
		if (*lp==10 || *lp==13 || *lp==CRC) {
			*lp='\0';
			break;
		}
		else lp++;
	}

	/* force reading blanks */
	if (ch) {
		i=fgetc(channel[ch].stream);
		while (i>=0 && (i=='\0' || i=='\n' || i=='\t' || i==' ')) {
			if (isEOF(ch)) break;
			i=fgetc(channel[ch].stream);
		}
		ungetc(i, channel[ch].stream);
	}

	/* return string */
	return lp=line;
}


/* getISOString()
 * ch: channel input number; if null, get string from keyboard
 * System function for the INSTRING procedure */
char *getISOString(int ch) {
	char *lp=line;
	int i;

	/* get string */
	for (i=0; i<SCREENLINE; i++) line[i]='\0';
	while (TRUE) {
		if (!ch || channel[ch].iochan & 0100000) *lp=getchar();
		else {
			if (!channel[ch].isopen) pe(14,l);
			if (!(channel[ch].wayout & VREAD)) 
				pe(58,l);
			*lp=fgetc(channel[ch].stream);
		}
		if (*lp=='\\') {
			char next=getchar();
			switch (next) {
				case 'b': *lp='\b'; break;
				case 'f': *lp='\f'; break;
				case 'n': *lp='\n'; break;
				case 'r': *lp='\r'; break;
				case 't': *lp='\t'; break;
				case '\\': *lp='\\'; break;
				case '\"': *lp='\"'; break;
				case '\'': *lp='\''; break;
				default:   *lp=next;
			}	
		}
		else if (*lp==10 || *lp==13 || *lp==CRC) {
			*lp='\0';
			break;
		}
		lp++;
	}

	/* force reading blanks */
	if (ch) {
		i=fgetc(channel[ch].stream);
		while (i>=0 && (i=='\0' || i=='\n' || i=='\t' || i==' ')) {
			if (isEOF(ch)) break;
			i=fgetc(channel[ch].stream);
		}
		ungetc(i, channel[ch].stream);
	}

	/* return string */
	return lp=line;
}


/* READ()
 * Perform the READ/READOCTAL procedures
 * ch is the read channel
 * mode: if TRUE perform READ, otherwise perform READOCTAL
 * Note: in case of READOCTAL numbers must be in the %nnnn form
 * Ref. AA-O196C-TK - 16.6.4.1 "Numeric Input Data" */
void READ(int ch, int mode) {
	int type, isArr_=FALSE, val[MAXDIM+1];
	char temp[COMPSTR], *t=temp;

	/* enter */
	if (*p=='(') p++;
	else pe(2,l);
	*t='\0';

	/* process each variable */
	while (*p!=')') {
		int cs, j;
		while (isblank(*rb)) rb++;
		if (!checkstd(ch) && isEOF(ch)) pe(38,l);
		/* read from file input */
		if (ch!=1 && ch<=STREAMS && (!*rb || *rb==CRC)) {
			strncpy_(temp, getString(ch),COMPSTR);
			t=temp;
			while (isblank(*t)) t++;
			/* discard void lines
			 * Ref. AA-O196C-TK - 16.6.4.1, fourth paragraph */
			while (!*t || *t==CRC) {
				if (ch>0 && ch<=STREAMS && !checkstd(ch)) 
				if (isEOF(ch)) pe(38,l);
				strncpy_(temp, getString(ch),COMPSTR);
				t=temp;
				while (isblank(*t)) t++;
			}
			if (*t=='\"') strncpy_(rbuff,t,COMPSTR);
			else {
				strncpy_(rbuff,"\"",COMPSTR);
				strncat_(rbuff,t,COMPSTR);
				strncat_(rbuff,"\"",COMPSTR);
			}
			rb=rbuff;
			while (isblank(*rb)) rb++;
		}
		/* read from logical channel input */
		else if (ch>STREAMS && ch<=OSTREAMS) {
			char *f=rbuff,*t=PS[vn[byteVar[ch]].ref]+bytePos[ch];
			*f++='\"';
			while (*t) *f++=*t++;
			*f++='\"';
			*f='\0';
			rb=rbuff;
		}

		/* read next variable name */
		cs=findDEC(p);
		if (!cs) pe(2,l);
		type=vn[vn[cs].ref].type;
		p+=_len;
		isArr_=FALSE;
		if (_arr) {
			int c=1;
			isArr_=TRUE;
			if (*p=='[') p++;
			else pe(80,l);
			for (j=1;j<=vn[cs].dim;j++) {
				val[c++]=(int)S();
				sysRet;
				if (*p==',') p++;
				if (c>MAXDIM) pe(7,l);
			}
			/* Note: since val is reusable, erase remaining values 
			 * that may interfere with following assignments */
			for (;c<=MAXDIM;c++) val[c++]=0;
			if (*p==']') p++;
			else pe(70,l);
		}
		/* get string and string array type and attribute */
		if (type==STRINGTYPEBOX) {
			/* Ref. AA-O196C-TK - 16.6.4.1, fifth paragraph 
			 * point 3*/
			char output[COMPSTR], *o=output;
			if (*rb!='\"') pe(10,l);
			else rb++;
			while (*rb) {
				/* Ref. AA-O196C-TK - 16.6.4.1, sixth par. */
				if (*rb=='\"' && *(rb+1)=='\"') rb++;
				else if (*rb=='\"') break;
				*o++=*rb++;
			}
			*o='\0';
			if (!isArr_) strncpy_(PS[vn[cs].ref],output,COMPSTR);
			else 
			  setArrayStr(vn[cs].ref,vn[vn[cs].ref].dim,val,output);
		}
		/* get number and array number type and attribute */
		else if (type<STRINGTYPEBOX) {
			double value;
			int off;
			if (*rb=='\"') rb++;
			/* readoctal mode */
			if (!mode) {
				value=getOctal(rb,type,&off); 
				rb+=off;
			}
			/* read mode */
			else {
				char *pb=p;
				p=rb;
				stripBlanks(p);
				/* Note: S() can read both real and long real
				 * numbers*/
				value=S();
				sysRet;
				rb=p;
				p=pb;
			}
			if (*rb=='\"') rb++;
			/* number assignment */ 
			if (!isArr_) {
			 	/* Boolean assignment
				 * Ref. AA-O196C-TK - 16.6.4.1, fifth paragraph 
				 * point 2*/
				if (type==BOOLEANTYPEBOX) 
					P[vn[cs].ref]=(int)value;
				/* integer assignment 
				 * Ref. AA-O196C-TK - 16.6.4.1, fifth paragraph 
				 * point 1*/
				else if (type==INTEGERTYPEBOX) {
					if (value>0.0) 
					   P[vn[cs].ref]=((int)(value+0.5));
					else 
					   P[vn[cs].ref]=((int)(value-0.5));
				}
				/* real and long real assignment
				 * Ref. AA-O196C-TK - 16.6.4.1, fifth paragraph 
				 * point 1*/
				else if (type==REALTYPEBOX) 
					P[vn[cs].ref]=value;
				else if (type==LONGREALTYPEBOX) 
					P[vn[cs].ref]=value;
			}
			/* array element assignment */
			else {
				if (type==BOOLEANTYPEBOX) 
				setArrayVal(vn[cs].ref,vn[vn[cs].ref].dim,val,(int)value);
				/* Ref. AA-O196C-TK - 16.6.4.1, fifth paragraph 
				 * point 1*/
				else if (type==INTEGERTYPEBOX) {
				  if (value>0.0)	
				  setArrayVal(vn[cs].ref,vn[vn[cs].ref].dim,val,(int)value+0.5);
				  else
				  setArrayVal(vn[cs].ref,vn[vn[cs].ref].dim,val,(int)value-0.5);
				}
				else if (type==REALTYPEBOX) 
				setArrayVal(vn[cs].ref,vn[vn[cs].ref].dim,val,value);
				else if (type==LONGREALTYPEBOX) 
				setArrayVal(vn[cs].ref,vn[vn[cs].ref].dim,val,value);
			}
		}

		/* strip inner blanks */
		if (*rb=='\"') rb++;
		while (isblank(*rb)) rb++;
		if (*rb==',' || *rb==';') rb++;
		while (isblank(*rb)) rb++;
		if (*p==',') p++;

		/* update logical channel position */
		if (ch > STREAMS && ch <= OSTREAMS) {
			int off=(int)(rb-rbuff);
			bytePos[ch]+=off;
			if (bytePos[ch]>vn[byteVar[ch]].strmem) bytePos[ch]=0;
		}
	}
	if (*p==')') p++;
}


/***************************************************************************
 * PRINT procedure
 ***************************************************************************/

/* printExp()
 * print the number val in exponential format
 * ch: the channel stream
 * val: the number to be printed (always positive)
 * minus: the character space or '-'
 * type: the type of the variable (only INTEGER, REAL and LONG REAL)
 * N: the parameter used in exponential cases 
 * System function for the PRINT procedure */
void printExp(int ch, double val, int minus, int type, int N, char *ext) {
	int exp=0, eminus='+', l=0;
	char format[COMPSTR], output[COMPSTR], gexp[3], *k=output;
	char zeroes[COMPSTR];
	gexp[0]=expChar, gexp[1]='\0';
	
	/* setup for long real */
	if (type==LONGREALTYPEBOX && (expChar=='&' || expChar=='@')) 
		gexp[1]=expChar, gexp[2]='\0';

	/* build zeroes-padding 
	 * Ref. AA-O196C-TK - 16.6.4.2 seventh and last paragraph */
	zeroes[0]='\0';
	if (type==REALTYPEBOX && N>9) {
		char *w=zeroes;
		l=N-9; 
		if (l>=COMPSTR) l=COMPSTR-1;
		N=9;
		while (l>0) *w++='0',l--;
		*w++='\0';
	}
	else if (type==LONGREALTYPEBOX && N>17) {
		char *w=zeroes;
		l=N-17;
		if (l>=COMPSTR) l=COMPSTR-1;
		N=17;
		while (l>0) *w++='0',l--;
		*w++='\0';
	}

	/* build number separating mantissa and exponent */
	if (val!=0.0) {
		while ((val+REALOFF)>=10.0) val/=10.0, exp++;
		while ((val+REALOFF)<1.0) val*=10.0, exp--;
		if (exp<0) exp*=-1, eminus='-';
	}
	/* depict output string */
	sprintw(format,COMPSTR,"%%c%%0%d.%df%s%s%%c%%d",N+2,N,zeroes,gexp);
	sprintw(output,COMPSTR,format,minus,val,eminus,exp);
	/* consume zeroes at the beginning*/
	k++;
	if (val!=0.0) while (*k==' ') *k=*(k-1),*(k-1)=32,k++;
	
	/* export output string if required */
	if (ext) {
		strncpy_(ext,output,COMPSTR);
	}
	/* print buffer to physical channel */
	else if (ch<=STREAMS) {
		if (channel[ch].fcounter+strlen(output)>FILESCRLIM) FCR(ch);
		fprintf(channel[ch].stream,"%s ",output);
		channel[ch].fcounter+=strlen(output)+1;
	}
	/* print buffer to logical channel */
	else if (ch<=OSTREAMS) {
		int size=vn[byteVar[ch]].strmem;
		char *ls=PS[vn[byteVar[ch]].ref];
		char *bu=output;
		while (*bu) {
			*(ls+bytePos[ch])=*bu++, bytePos[ch]++;
			if (bytePos[ch]>=size-1) bytePos[ch]=0;
		}
		*(ls+bytePos[ch])='\0';
	}
	
	/* print immediately */
	fflush(channel[ch].stream);
}


/* PRINT()
 * perform the PRINT/PRINTOCTAL procedures
 * ch is the output channel
 * mode: if TRUE perform PRINT, otherwise PRINTOCTAL
 * Ref. AA-O196C-TK - 16.6.4.2 "Numeric Output Data" 
 * Note: I was quite surprised (with modern feelings) to read on the site
 * http://comjnl.oxfordjournals.org/content/5/4/341.full.pdf :
 * "The absence from the ALGOL 60 Report (Naur, ed. 1960) of any explicit 
 * reference to the ideas of "data" or "results" for a program has led to the 
 * situation where almost every implementation has its own peculiar mechanism 
 * for input and output. 
 * The ALCOR group, for example, have two standard procedures built into their 
 * computers which when activated read and print a number in standard form. 
 * The MC-translator (Dijkstra, l962) has similar provisions. 
 * The DASK user (Jensen et al., 1960) has at his disposal a much more 
 * elaborate set of standard procedures which give very line control over the 
 * appearance of the printed sheet of results. These DASK procedures, however, 
 * are not procedures in the strict ALGOL sense, since their treatment of 
 * parameters goes beyond that prescribed by the Report. 
 * The Elliott scheme (Hoare, 1962) provides input-output facilities of 
 * considerable power, but they are based on a structure which is additional to 
 * that of ALGOL itself. The system described by McCracken (1962) is likewise 
 * an extension of ALGOL 60." 
 *
 * This, in addition to the fact that I couldn't find any example in the 
 * AA-196C-TK manual of printed output of DEC ALGOL 60 programs, has led me 
 * to design a printing system that adheres to the concepts of the manual, 
 * covering peculiar problems (whose solutions were not found) with my
 * own inventions and modifications. */
void PRINT(int ch, int mode) {
	double val;
	int minus=' ', type=0;
	int M=-1, N=-1, exitnow=FALSE, isInf=FALSE;
	char format[COMPSTR];
	char output[COMPSTR];

	if (*p=='(') p++;
	else pe(2,l);

	/* check file integrity for output */
	if (ch>1 || ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VWRITE)) pe(59,l);
	}
	else if (!ch) pe(44,l);

	/* evaluate  number */
	val=S();
	sysRet;
	type=resultType;
	
	/* rule sign */
	if (val<0.0) minus='-', val=-val;

	/* rule limits */
	if (type==REALTYPEBOX) {
		if (val>REALMAX) isInf=TRUE;
		if (val<REALMIN) minus=' ', val=0;
	}
	else if (type==LONGREALTYPEBOX) {
		if (val>LONGREALMAX) isInf=TRUE;
		if (val<LONGREALMIN) minus=' ', val=0;
	}

	/* get printing arguments (if any) */
	if (*p==',') {
		p++;
	       	M=(int)S();
		sysRet;
	}
	if (*p==',') {
		p++;
	       	N=(int)S();
		sysRet;
	}

	/* octal case */
	if (!mode) {
		fprinto(output,val,type);
	}
	/* Not a Number: print nan and don't proceed */
	else if (isnan(val)) {
		sprintw(output,COMPSTR," nan");
	}

	/* infinitive: print 'inf' in any case */
	else if (isInf) {
		sprintw(format,COMPSTR,"%cinf",minus);
		sprintw(output,COMPSTR,format);
	}
	/* Boolean type: print 'true' or 'false' */
	else if (type==BOOLEANTYPEBOX) {
		int res=(int)val;
		if (res) sprintw(output,COMPSTR," true");
		else sprintw(output,COMPSTR," false");
	}
	/* numeric type: print according to the formatting arguments M,N
	 * A) M > 0 and N > 0 
	 * Ref. AA-O196C-TK - 16.6.4.2 second paragraph */
	else if (M>0 && N>0) {
		char *k=output;
		sprintw(format,COMPSTR," %%0%d.%df",M+N+1,N);
		sprintw(output,COMPSTR,format,fabs(val));
		/* consume zeroes at the beginning*/
		*k=minus;
		k++; // skip minus
		while (*k=='0' && *(k+1)!='.') *k=*(k-1),*(k-1)=32,k++;
	}
	/* B) M > 0 and N = 0  (N given) - it's a forced integer type
	 * Ref. AA-O196C-TK - 16.6.4.2 third paragraph  */
	else if (M>0 && !N) {
		char *k=output;
		sprintw(format,COMPSTR," %%0%dld",M);
		sprintw(output,COMPSTR,format,(long int)val);
		/* consume zeroes at the beginning*/
		*k=minus;
		k++;
		while (*k=='0' && *(k+1)!='.') *k=*(k-1),*(k-1)=32,k++;
		/* if zero, all are nulls */
		if (!*k) *--k='0';
	}
	/* C) M = 0 and N = 0 (both given) or M<0 and N<0 (none given)  
	 * Ref. AA-O196C-TK - 16.6.4.2 sixth paragraph */
	else if (M<=0 && N<=0) {
		if (type==INTEGERTYPEBOX) {
			int ival=0;
			char *k=output;
			/* solve overflowing:
			 * Note: some Linux machines showed that overflow is
			 * catched here, and in case of negative numbers
			 * the minus sign is printed twice. */
			if (val>CORELIMIT) ival=CORELIMIT;
			else ival=(int)val;
			sprintw(format,COMPSTR," %%0%ldd",11);
			if (!ival) 
				sprintw(output,COMPSTR,format,0L);
			else sprintw(output,COMPSTR,format,(long int)ival);
			*k=minus;
			k++;
			/* remove trailing zeroes */
			while (*k=='0' && *(k+1)!='.') *k=*(k-1),*(k-1)=32,k++;
			if (!*k) k--, *k='0'; // it's zero, let's rebuild it
		}
		else if (type==REALTYPEBOX) {
			printExp(ch,val,minus,type,9,NULL);
			exitnow=TRUE;
		}
		else if (type==LONGREALTYPEBOX) {
			printExp(ch,val,minus,type,17,NULL);
			exitnow=TRUE;
		}
	}
	/* D) M = 0 and N > 0 (both given) 
	 * Ref. AA-O196C-TK - 16.6.4.2 fourth paragraph */
	else if (!M && N>0) {
		printExp(ch,val,minus,type,N,NULL);
		exitnow=TRUE;
	}
	/* E) M > 0 and N < 0 (only M given) 
	 * Ref. AA-O196C-TK - 16.6.4.2 fifth paragraph*/
	else if (M>0 && N<0) {
		if (type==INTEGERTYPEBOX) {
			char *k=output;
			sprintw(format,COMPSTR," %%0%ldd",M);
			sprintw(output,COMPSTR,format,(long int)val);
			/* consume zeroes at the beginning*/
			*k=minus;
			k++;
			while (*k=='0' && *(k+1)!='.') *k=*(k-1),*(k-1)=32,k++;
		}
		else if (type==REALTYPEBOX) {
			printExp(ch,val,minus,type,M,NULL);
			exitnow=TRUE;
		}
		else if (type==LONGREALTYPEBOX) {
			printExp(ch,val,minus,type,M,NULL);
			exitnow=TRUE;
		}
	}
	if (*p==')') p++;
	else pe(2,l);
	
	/* Proceed to print if not already directed to printExp() 
	 * otherwise exit now */
	if (exitnow) return;

	/* print buffer to physical channel */
	if (ch<=STREAMS) {
		if (channel[ch].fcounter+strlen(output)>FILESCRLIM) FCR(ch);
		fprintf(channel[ch].stream,"%s ",output);
		channel[ch].fcounter+=strlen(output)+1;
	}
	/* print buffer to logical channel */
	else if (ch<=OSTREAMS) {
		int size=vn[byteVar[ch]].strmem;
		char *ls=PS[vn[byteVar[ch]].ref];
		char *bu=output;
		while (*bu) {
			*(ls+bytePos[ch])=*bu++, bytePos[ch]++;
			if (bytePos[ch]>=size-1) bytePos[ch]=0;
		}
		*(ls+bytePos[ch])='\0';
	}
	
	/* print immediately */
	fflush(channel[ch].stream);
}


/***************************************************************************
 * WRITE, VPRINT and DELETE procedures
 ***************************************************************************/


/* EMIT(int n)
 * emit a control character n times.
 * ch = output channel number
 * what = control character for SPACE (what=' '), NEWLINE (what='\n'),
 * TAB (what='\t'), PAGE (what = 0 using the new page for screen).
 * Note: there is no procedure called EMIT; System function for the symbol 
 * procedures contained in:
 * Ref: AA-O196C-TK - 16.6.3 "Miscellaneous Symbol Procedures" */
void EMIT(int ch, int what) {
	int i,t=1;

	/* check file integrity for output */
	if (ch>1 || ch<=STREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VWRITE)) pe(59,l); //?
	}

	/* get control code */
	if (*p=='(') {
		p++;
		t=(int)S();
		sysRet;
		/* check closing */
		if (*p!=')') pe(2,l);
		else p++;
	}

	/* do PAGE for screens */
	if (!what) {
		if (channel[ch].stream==stdout) {
			/* clear screen */
			fprintf(channel[ch].stream,"%s","[2J");
			/* reposition cursor */
			fprintf(channel[ch].stream,"%s"," [H");
		}
		return;
	}

	/* EMIT on file channel */
	if (ch<=STREAMS) {
		/* emit control code */
		for (i=1;i<=t;i++) {
			fputc(what,channel[ch].stream);
			if (what=='\t') {
				channel[ch].fcounter+=TABSTOP;
				if (channel[ch].fcounter+TABSTOP>FILESCRLIM) 
					FCR(ch);
			}
			else {
				channel[ch].fcounter++;
				if (channel[ch].fcounter+1>FILESCRLIM) FCR(ch);
			}
		}
		if (what==CRC) {
			channel[ch].fcounter=0;
			channel[ch].pageL++;
			if (ch<=STREAMS && channel[ch].pageL>PAGEHEIGHT) 
				channel[ch].pageL=1, channel[ch].pageN++;
		}
	}
	/* EMIT on logical channels (newlines are not allowed here) */
	else {
		/* emit control code */
		if (what!=CRC) {
			for (i=1;i<=t;i++) {
				if (channel[ch].fcounter+1>FILESCRLIM) FCR(ch);
				fputc(what,channel[ch].stream);
				if (what=='\t') channel[ch].fcounter+=TABSTOP;
				else channel[ch].fcounter++;
			}
		}
	}
}


/* WRITE()
 * Perform the WRITE command for strings output
 * ch = output channel number
 * Ref. AA-O196C-TK - 16.6.2 "String Output" */
void WRITE(int ch) {
	char BASEOUT[COMPSTR],*q=BASEOUT;
	char buffer[COMPSTR], *bu=buffer;

	/* check output channel integrity */
	if (ch>1 || ch<=OSTREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VWRITE)) pe(59,l);
	}
	else if (!ch) pe(44,l);

	/* write to buffer */
	if (*p!='(') pe(2,l);
	else p++;
	if (!isanystring(p)) pe(10,l);
	strncpy_(BASEOUT,SS(),COMPSTR);
	sysRet;
	while (*q) {
		if (*q=='\"' && *(q+1)=='\"') *bu++=*q++;
		else if (*q==';' && *(q+1)==';') *bu++=*q++;
		else if (*q=='[' && *(q+1)=='[') *bu++=*q++, q++;
		else if (*q==']' && *(q+1)==']') *bu++=*q++, q++;
		else if (*q=='[') {
			int i,t=1;
			q++; // skip [
			while (*q) {
				if (*q==']') break;
				if (isdigit(*q)) t=strtol(q,&q,10);
				/* secure value */
				if (t<0) t*=-1;
				/* page throws are not countable */
				if (toupper(*q)=='P') {
					for (i=1;i<=t;i++) *bu++='\v';
				}
				else if (toupper(*q)=='S') {
					for (i=1;i<=t;i++) *bu++=' ';
				}
				else if (toupper(*q)=='C' || toupper(*q)=='N') {
					for (i=1;i<=t;i++) *bu++=CRC;
				}
				else if (toupper(*q)=='T') {
					for (i=1;i<=t;i++) *bu++='\t';
				}
				else if (toupper(*q)=='B') {
					for (i=1;i<=t;i++) *bu++='\b';
				}
				else pe(19,l);
				t=1;
				q++;
			}
			if (!*q || *q!=']') pe(45,l);
			else q++;
		}
		else *bu++=*q++;
	}
	if (*p!=')') pe(2,l);
	else p++;
	*bu='\0';

	/* print buffer to physical channel, taking care of the carriage
	 * return character that may be inside the string */
	if (ch<=STREAMS) {
		bu=buffer;
		while (*bu) emitChar(ch,*bu++);
	}
	/* print buffer to logical channel, take care of not passing over
	 * string length, and restarting from beginning in case */
	else if (ch<=OSTREAMS) {
		int size=vn[byteVar[ch]].strmem;
		char *ls=PS[vn[byteVar[ch]].ref];
		bu=buffer;
		while (*bu) {
			if (*bu==CRC || *bu=='\b' || *bu=='\v') bu++;
			else *(ls+bytePos[ch])=*bu++, bytePos[ch]++;
			if (bytePos[ch]>=size-1) bytePos[ch]=0;
		}
		*(ls+bytePos[ch])='\0';
	}

	/* print immediately */
	fflush(channel[ch].stream);
}


/* DELETE()
 * Perform the DELETE command for strings
 * Note: the string space is retained, so if it was resized by NEWSTRING, it
 * is not restored to the default characters size
 * Ref. AA-O196C-TK - 13.8.4 "Delete" */
void DELETE(void) {
	int ns=0, isArr_=FALSE;
	if (*p!='(') pe(2,l);
	else p++;

	/* check var */
	ns=findDEC(p);
	/* if it's not a variable, it may be a string array */
	if (!ns) {
		forceArray=TRUE;
		ns=findDEC(p);
		forceArray=FALSE;
		if (!ns) pe(56,l);
		isArr_=TRUE;
	}
	if (_type<STRINGTYPEBOX) pe(56,l);

	/* get to the original referenced item */
	while (ns!=vn[ns].ref) ns=vn[ns].ref;
	
	p+=_len;
	if (*p!=')') pe(2,l);
	else p++;
	
	/* delete variable */
	if (!isArr_) {
		int i;
		for (i=0;i<vn[ns].strmem;i++) *(PS[vn[ns].ref]+i)='\0';
	}
	/* delete array */
	else {
		memset(dimS[ns].elem,'\0',vn[ns].size*sizeof(char));
	}
}


/* VPRINT()
 * Perform the VPRINT command for strings output
 * ch = output channel number
 * This version prints simple strings, interprets escape characters, and
 * does not interpret DEC special command in square brackets. After the print,
 * a New Line is added.
 * Note: this procedure didn't belong to DEC nor to ISO, but some
 * compilers adopted it as a counterpart for PRINT (which is directed 
 * to print numbers) */
void VPRINT(int ch) {
	char BASEOUT[COMPSTR], *q=BASEOUT;

	/* check output channel integrity */
	if (ch>1 || ch<=OSTREAMS) {
		if (!channel[ch].isopen) pe(34,l);
		if (!(channel[ch].wayout & VWRITE)) pe(59,l);
	}
	else if (!ch) pe(44,l);

	/* read argument string */
	if (*p!='(') pe(2,l);
	else p++;
	if (!isanystring(p)) pe(10,l);
	strncpy_(BASEOUT,SS(),COMPSTR);
	sysRet;
	
	/* print string, evaluating escape characters */
	while (*q) {
		/* parsing escape characters */
		if (*q=='\\') {
			q++;
			/* print escape values */
			if (*q=='a') emitChar(ch,'\a');
			else if (*q=='b') emitChar(ch,'\b');
			else if (*q=='f') emitChar(ch,'\f');
			else if (*q=='n') emitChar(ch,'\n');
			else if (*q=='r') emitChar(ch,'\r');
			else if (*q=='t') emitChar(ch,'\t');
			else if (*q=='v') emitChar(ch,'\v');
			else if (*q=='\\') emitChar(ch,'\\');
			else if (*q=='\"') emitChar(ch,'\'');
			else if (*q=='\'') emitChar(ch,'\"');
			else emitChar(ch,*q);
		}
		/* any other character is printed as-is and advances one 
		 * position in fcounter */
		else emitChar(ch,*q);
		q++;
	}

	/* print Carriage return */
	fputc('\n',channel[ch].stream);

	/* check closure */
	if (*p!=')') pe(2,l);
	else p++;
}


/****************************************************************************
 * The following routines define system function for variable declarations
 * showPROC() - print content of declared procedures
 * showDEC()  - print content of declared variables
 * declare()  - declare simple variables
 * array()    - declare arrays
 ****************************************************************************/

/* showPROC()
 * DEBUG FUNCTION - print data of declared procedures
 * i: number of variable
 * lb: line number where procedure is declared (only used by preParse()) */
void showPROC(int i, int lb) {
	int type=Procp[i].type;
	int index=Procp[i].numvars;
	int ref=Procp[i].procref;
	int bs;
	int ic=1;

	fprintf(stdout,"%s",INVCOLOR);
	fprintf(stdout,"\nPROCEDURE [#%d] %s",i,Procp[i].name);
	if (ref>=0) fprintf(stdout," (subordinated to procedure [#%d] %s)",ref,Procp[ref].name);
	if (lb) fprintf(stdout," at line %d has:\n",lb);
	else fprintf(stdout," has:\n");
	fprintf(stdout,"type = %s [%d]\n",Rtype[type/10], type);
	fprintf(stdout,"arguments number = %d\n",index);
	for (bs=1;bs<=index;bs++) {
		int mode, state;
		fprintf(stdout,"  #%d ",bs);
		while (Procp[i].vars[ic]>0) 
			fputc(Procp[i].vars[ic++],stdout);
		ic++; // skip -1
		type=Procp[i].vars[ic++]-DECADD;
		mode=Procp[i].vars[ic++]-DECADD;
		state=Procp[i].vars[ic++]-DECADD;
		ic++; // skip -2
		fprintf(stdout,"%s",", ");
		switch(type) {
			case LONGREALTYPEBOX:
				fprintf(stdout,"%s","long real"); break;
			case REALTYPEBOX: 
				fprintf(stdout,"%s","real"); break;
			case INTEGERTYPEBOX:
			       	fprintf(stdout,"%s","integer"); break;
			case BOOLEANTYPEBOX:
			       	fprintf(stdout,"%s","Boolean"); break;
			case STRINGTYPEBOX:
			       	fprintf(stdout,"%s","string"); break;
			case LABELTYPEBOX:
			       	fprintf(stdout,"%s","label");break;
			default:
				fprintf(stdout,"%s","typeless");
		}
		switch(mode) {
			case BYREF:
				fprintf(stdout,"%s"," referenced"); break;
			case BYVAL:
				fprintf(stdout,"%s"," valuated"); break;
			default:
				;
		}
		switch(state) {
			case ISVAR:
				fprintf(stdout,"%s"," variable\n"); break;
			case ISPROC:
				fprintf(stdout,"%s"," procedure\n"); break;
			case ISARR:
				fprintf(stdout,"%s"," array\n"); break;
			default:
				fprintf(stdout,"%s"," item\n");
		}
	}
	if (isVerbose) fprintf(stdout,"proc data : l=%d, p=%d\n",Procp[i].procL,Procp[i].procP);
	if (isVerbose) fprintf(stdout,"begin data: l=%d, p=%d\n",Procp[i].begnL,Procp[i].begnP);
	if (isVerbose) fprintf(stdout,"term data : l=%d, p=%d\n",Procp[i].termL,Procp[i].termP);
	fprintf(stdout,"%s",RESETCOLOR);
}


/* showDEC()
 * DEBUG FUNCTION - print data of declared variables
 * decCount: number of variable
 * declaration: if it starts with Level: use the DUMP scheme, otherwise 
 * the normal debug scheme */
void showDEC(int decCount, char *declaration) {
	int isDump=FALSE;
	if (!strncasecmp_(declaration,"Level:",6)) isDump=TRUE;
	/* DEBUGGING show var contents */
	fprintf(stdout,"%s",INVCOLOR);
	fprintf(stdout,"%s%s [#%d], Type is ", declaration, vn[decCount].name,decCount);
	if (vn[decCount].type==LONGREALTYPEBOX) fprintf(stdout,"%s","long real");
	else if (vn[decCount].type==REALTYPEBOX) fprintf(stdout,"%s","real");
	else if (vn[decCount].type==INTEGERTYPEBOX) fprintf(stdout,"%s","integer");
	else if (vn[decCount].type==BOOLEANTYPEBOX) fprintf(stdout,"%s","Boolean");
	else if (vn[decCount].type==STRINGTYPEBOX) fprintf(stdout,"%s","string");
	/* add bound variable name */
	if ((int)abs(vn[decCount].ref)!=decCount) {
		int f=vn[decCount].ref;
		if (f<0) f=-f;
		fprintf(stdout," (bound to %s [#%d])",vn[f].name,f);
	}
	/* add array specifics */
	if (vn[decCount].isArr) {
		fprintf(stdout,"%s"," array");
		fprintf(stdout," (%d dim)",vn[decCount].dim);
	}
	/* print values */
	if (vn[decCount].isFict) {
		/* fictitious variables are printed with ns */
		int ns=decCount;
		if (vn[ns].type<INTEGERTYPEBOX)
			fprintf(stdout,", Value is %f",P[ns]);
		else if (vn[ns].type<STRINGTYPEBOX)
			fprintf(stdout,", Value is %d",(int)P[ns]);
		else if (PS[ns][0]) 
			fprintf(stdout,", Value is\"%s\"",PS[ns]);
		else fprintf(stdout,"%s",", No value");
	}
	else if (!vn[decCount].isArr && isDump) {
		/* normal variables and dumped variables 
		 * are reconducted to the reference */
		int ns=decCount;
		while (ns>0 && ns!=vn[decCount].ref) ns=vn[decCount].ref;
		if (vn[ns].type<INTEGERTYPEBOX)
			fprintf(stdout,", Value is %f",P[ns]);
		else if (vn[ns].type<STRINGTYPEBOX)
			fprintf(stdout,", Value is %d",(int)P[ns]);
		else if (PS[ns][0]) 
			fprintf(stdout,", Value is\"%s\"",PS[ns]);
		else fprintf(stdout,"%s",", No value");
	}
	else if (!vn[decCount].isArr) {
		/* normal variables are printed with the value stored
		 * in valdata if present */
		if (vn[decCount].valdata[0]) {
			fprintf(stdout,", Value is %s",vn[decCount].valdata);
		}
		else {
			if (vn[decCount].type<INTEGERTYPEBOX)
				fprintf(stdout,", Value is %f",P[decCount]);
			if (vn[decCount].type<STRINGTYPEBOX)
				fprintf(stdout,", Value is %d",(int)P[decCount]);
			else if (vn[decCount].type==STRINGTYPEBOX)
				fprintf(stdout,", Value is \"%s\"",PS[decCount]);
			else fprintf(stdout,"%s",", No value");
		}
	}
	fprintf(stdout,"%s",CRS);
	fprintf(stdout,"%s",RESETCOLOR);
}	


/* declare()
 * The inner statement for any-name variables declaration
 * Note: this function is not run-time.
 * Note: this function executes a storing only; the actual command (the 
 * function interpretation and execution) is performed by findDEC(), called
 * into getFunc() and SS() for function names and in assign() for assignments, 
 * and in all functions dealing with variables. */
void declare(void) {
	char decname[NAMEDIM], *sp=decname;
	int i=0;
	int code=0, doOwn=FALSE, another=0;

	/* store var code (based on position in the source) */
	code=l*1000000+(int)(p-BASESTR);

	/* get var name */
	if (isbeginnamechar(p)) {
		while (isnamechar(p) && i<NAMEDIM) {
			if (*p=='.') {
				p++;
				continue;
			}
			*sp++=*p++;
			i++;
		}
		*sp='\0';
	}
	else pe(2,l);

	/* check for too long identifiers */
	if (i>=NAMEDIM && isnamechar(p-1)) pe(26,l);
	
	/* found if there is another instance */
	//another=findDEC(decname);
	another=findInstance(decname);
	if (another) {
		fprintf(stderr,error[99],decname,l,vn[another].line);
	}

	/* update counter */
	decCount++;
	if (decCount>=LIMIT) pe(7,l);
	vn[decCount].type=typeBox;
	vn[decCount].ref=decCount;
	vn[decCount].refcode=decCount;
	vn[decCount].proccode=0;
	vn[decCount].isArr=FALSE;
	vn[decCount].isFunc=FALSE;
	vn[decCount].isFict=FALSE;
	vn[decCount].line=l;
	vn[decCount].level=pexecTOS;
	vn[decCount].dim=0;
	for (i=0;i<MAXDIM;i++) {
		vn[decCount].ldim[i]=0;
		vn[decCount].offset[i]=0;
		vn[decCount].coeff[i]=0;
	}
	for (i=0;i<COMPSTR;i++) {
		vn[decCount].strdata[i]='\0';
		vn[decCount].valdata[i]='\0';
	}
	
	/* set procedure reference */
	if (!pexecTOS) vn[decCount].proccode=0;
	else vn[decCount].proccode=pexec[pexecTOS-1];

	/* decide what base to use in case of OWN variables 
	 * If a variable is found, use that value, other set for nulls */
	if (isOwn) {
		int i=1;
		while (i<=ghostCount) {
			if (own[i].code==code) {
				doOwn=i;
				break;
			}
			i++;
		}
	}

	/* OWN variables with existing ghost */
	if (isOwn && doOwn) {
		/* numeric variables */
		if (typeBox<STRINGTYPEBOX) {
			P[decCount]=own[doOwn].num;
			vn[decCount].strmem=0;
		}
		/* string variables */
		else if (typeBox>=STRINGTYPEBOX) { 
			PS[decCount]=salloc(ALGSTR);
			if (!PS[decCount]) pe(6,l);
			strncpy_(PS[decCount],own[doOwn].str,ALGSTR);
			vn[decCount].strmem=ALGSTR;
		}
		vn[decCount].isOwn=code;
	}
	/* OWN variables created for the first time: create ghost */
	else if (isOwn && !doOwn) {
		ghostCount++;
		own[ghostCount].code=code;
		/* numeric variables */
		if (typeBox<STRINGTYPEBOX) {
			P[decCount]=0.0;
			own[ghostCount].num=0.0;
			vn[decCount].strmem=0;
		}
		/* string variables */
		else if (typeBox>=STRINGTYPEBOX) {
			int j;
			PS[decCount]=salloc(ALGSTR);
			if (!PS[decCount]) pe(6,l);
			for (j=0;j<ALGSTR;j++) PS[decCount][j]='\0',
				own[ghostCount].str[j]='\0';
			vn[decCount].strmem=ALGSTR;
		}
		vn[decCount].isOwn=code;
	}
	/* preset null values for uninstantiated variables
	 * Note: not guaranteed by DEC-ALGOL 20, but it's safer */
	else {
		/* numeric variables */
		if (typeBox<STRINGTYPEBOX) {
			P[decCount]=0.0;
			vn[decCount].strmem=0;
		}
		/* string variables */
		else if (typeBox>=STRINGTYPEBOX) { 
			int j;
			PS[decCount]=salloc(ALGSTR);
			if (!PS[decCount]) pe(6,l);
			for (j=0;j<ALGSTR;j++) PS[decCount][0]='\0';
			vn[decCount].strmem=ALGSTR;
		}
		vn[decCount].isOwn=FALSE;
	}

	/* set name/type */
	strncpy_(vn[decCount].name,decname,NAMEDIM);
	vn[decCount].size=0;

	/* debug features */
	if (isDebug) showDEC(decCount, "Declared ");
	/* next assignment */
	if (*p==',') {
		p++;
		declare();
	}

	typeBox=0;
	// Note: no check for ; here!
}


/* array()
 * The inner statement for any-name array declaration
 * Note: this function is not run-time.
 * Note: this function executes a storing only; the actual command (the 
 * function interpretation and execution) is performed by findDEC(), called
 * into getFunc() and SS() for function names and in assign() for assignments, 
 * and in all functions dealing with variables. */
void array(void) {
	char decname[COMPSTR], *sp=decname;
	int dimq=1;
	int start[MAXDIM+1], end[MAXDIM+1];
	int i=0,td;
	int code=0, doOwn=FALSE, another=0;

	/* store var code (based on position in the source) */
	code=l*1000000+(int)(p-BASESTR);

	/* get var name */
	if (isbeginnamechar(p)) {
		while (isnamechar(p) && i<NAMEDIM) {
			if (*p=='.') {
				p++;
				continue;
			}
			*sp++=*p++;
			i++;
		}
		*sp='\0';
	}
	else pe(2,l);

	/* check for too long identifier */
	if (i>=NAMEDIM && isnamechar(p-1)) pe(26,l);

	/* found if there is another instance */
	//another=findDEC(decname);
	another=findInstance(decname);
	if (another) {
		fprintf(stderr,error[99],decname,l,vn[another].line);
	}

	/* update counter */
	decCount++;
	if (decCount>=LIMIT) pe(7,l);
	td=decCount;
	
	/* dimensions are specified */
	if (*p=='[') {
		p++;
		start[dimq]=(int)S();
		sysRet;
		if (*p!=':') pe(24,l);
		else p++;
		end[dimq]=(int)S();
		sysRet;
		if (end[dimq]<start[dimq]) pe(29,l);
		while (*p==',') {
			p++;
			dimq++;
			start[dimq]=(int)S();
			sysRet;
			if (*p!=':') pe(24,l);
			else p++;
			end[dimq]=(int)S();
			sysRet;
			if (end[dimq]<start[dimq]) pe(29,l);
		}
		if (*p!=']') pe(2,l);
		else p++;
	}
	/* defer dimensions */
	else if (*p==',') {
		p++;
		if (!*p) pe(2,l);
		array();
		dimq=vn[td+1].dim;
		for (i=1;i<=dimq;i++) {
			start[i]=vn[td+1].offset[i];
			end[i]=vn[td+1].offset[i]+vn[td+1].ldim[i];
		}
	}
	/* these are not simple declare() variables! */
	else pe(70,l);
	
	/* decide what base to use in case of OWN variables; if a ghost is 
	 * found, use that value, otherwise there are no ghosts */
	if (isOwn) {
		int i=1;
		while (i<=ghostCount) {
			if (own[i].code==code) {
				doOwn=i;
				break;
			}
			i++;
		}
	}

	/* build common attributes */
	vn[td].type=typeBox;
	vn[td].dim=dimq;
	vn[td].isArr=TRUE;
	vn[td].isFunc=FALSE;
	vn[td].isFict=FALSE;
	vn[td].ref=td;
	vn[td].refcode=td;
	vn[td].proccode=0;
	vn[td].strmem=ALGSTR;
	vn[td].line=l;
	vn[td].level=pexecTOS;
	for (i=1;i<=dimq;i++) {
		vn[td].ldim[i]=end[i]-start[i];
		vn[td].offset[i]=start[i];
	}
	vn[td].coeff[dimq]=1;
	for (i=dimq-1;i>=1;i--) {
		vn[td].coeff[i]=vn[td].coeff[i+1]*(vn[td].ldim[i+1]+1);
	}
	for (i=0;i<COMPSTR;i++) {
		vn[td].valdata[i]='\0';
		vn[td].strdata[i]='\0';
	}

	/* set procedure reference */
	if (!pexecTOS) vn[td].proccode=0;
	else vn[td].proccode=pexec[pexecTOS-1];


	/* OWN variables with existing ghost */
	if (isOwn && doOwn) {
		int asize;
		int gsize=own[doOwn].size;
		int size, nowcreate=FALSE;
		/* copy existing array ghost space on newly created array */
		/* numeric arrays */
		if (typeBox<STRINGTYPEBOX) {
			asize=dimArray(td,dimq,vn[td].ldim);
			if (asize>gsize) size=gsize, nowcreate=TRUE;
			else size=asize;
			memcpy_(dim[td].elem,own[doOwn].numarr,size);
		}
		/* string arrays */
		else {	
			asize=dimStrArray(td,dimq,vn[td].ldim);
			if (asize>gsize) size=gsize, nowcreate=TRUE;
			else size=asize;
			memcpy_(dimS[td].elem,own[doOwn].strarr,size);
		}
		vn[td].size=asize;
		/* in case newly created array is greater than existing
		 * ghost, recreate it as a bigger ghost */
		/* numeric arrays */
		if (nowcreate && typeBox<STRINGTYPEBOX) {
			if (own[ghostCount].numarr) 
				free(own[ghostCount].numarr);
			own[ghostCount].numarr=nalloc((size_t)asize);  
			if (!own[ghostCount].numarr) pe(13,l);
			own[ghostCount].size=asize;			
		}
		/* string arrays */
		else if (nowcreate) {
			if (own[ghostCount].strarr) 
				free(own[ghostCount].strarr);
			own[ghostCount].strarr=salloc((size_t)asize);  
			if (!own[ghostCount].strarr) pe(13,l);
			own[ghostCount].size=asize;			
		}
		else own[ghostCount].size=asize;
	}
	/* OWN variables created for the first time: create ghost */
	else if (isOwn && !doOwn) {
		int asize;
		ghostCount++;
		/* create ghost */
		/* numeric arrays */
		if (typeBox<STRINGTYPEBOX) {
			asize=dimArray(td,dimq,vn[td].ldim);
			own[ghostCount].numarr=nalloc((size_t)asize);  
			if (!own[ghostCount].numarr) pe(13,l);
		}
		/* string arrays */
		else {	
			asize=dimStrArray(td,dimq,vn[td].ldim);
			own[ghostCount].strarr=salloc((size_t)asize);  
			if (!own[ghostCount].strarr) pe(13,l);
		}
		own[ghostCount].size=asize;
		vn[td].size=asize;
		own[ghostCount].code=code;
		vn[td].isOwn=code;
	}
	/* simple arrays (no OWN) */
	else {
		/* numbers */
		if (typeBox<STRINGTYPEBOX) {
			vn[td].size=dimArray(td,dimq,vn[td].ldim);
		}
		/* strings */
		else {
			vn[td].size=dimStrArray(td,dimq,vn[td].ldim); 
		}
		vn[td].isOwn=FALSE;
	}
	
	/* set name/type */
	strncpy_(vn[td].name,decname,NAMEDIM);
	vn[td].valdata[0]='\0';

	/* debug features */
	if (isDebug) showDEC(td, "Declared ");
	/* next assignment */
	if (*p==',') {
		p++;
		array();
	}

	// Note: no check for ; here!
}


/******************************************
 *          RUNNING  INTERFACE            *
 ******************************************/

/* searchFirst()
 * wrapper for strcasestr_oq to check the first occurrence of term1 or term2 
 * and return its position in base as a char pointer
 * Return NIL in case none is found.
 * System function for storeSingleLineToCore() */
char *searchFirst(char *base, char *term1, char *term2) {
	char *y1, *y2;
	y1=strcasestr_oq(base,term1);
	y2=strcasestr_oq(base,term2);
	if (y1 && !y2) return y1;
	else if (!y1 && y2) return y2;
	else if (y1 && y2) {
		if (y1<y2) return y1;
		return y2;
	}
	return NIL;
}


/* storeSingleLineToCore()
 * saves into core memory the line in argument, storing also labels info;
 * return TRUE on successful storing;
 * line holds the textual statements part
 * lnum holds the physical line number
 * a void line is transparent for this command
 * return current updated line number (which changes only with EXTERNAL
 * procedures)
 * System function */ 
int storeSingleLineToCore(char *line, int lnum) {
	char *lp=line, *z, *cp;
	char thislabel[COMPSTR], *lab=thislabel;
	int lk=0;

	/* check NIL (empty line...) */
	if (!*line) return TRUE;

	/* adjust line */
	if ((int)strlen(line)>0) lp=line+(int)strlen(line)-1;
	else lp=line;
	/* get rid of final blanks */
	while ((int)(lp-line)>0 && isblank(*lp)) *lp--='\0';
	lp=line;

	/* store original line (debug and error printing purposes) */
	while (isblank(*lp)) lp++;
	lk=((int)strlen(lp)+1)*sizeof(char);
	mb[lnum]=salloc((size_t)lk);
	if (!mb[lnum]) pe(6,lnum);
	strncpy_(mb[lnum],lp,lk);
	labelIndex[lnum]=0;
	
	/* remove comments */
	lp=line;
	/* remove comment on this line originated in a previous line */
	if (isDelComm) {
		while (*lp && *lp!=';') *lp++=' ';
		if (*lp==';') *lp=' ', isDelComm=FALSE;
	}
	/* start removing a comment */
	while ((cp=searchFirst(lp,"!","COMMENT"))) {
		/* the figure != is not a comment */
		if (*cp=='!' && *(cp+1)=='=') break;
		while (*cp && *cp!=';') *cp++=' ';
		if (*cp==';') *cp=' ', isDelComm=FALSE;
		else if (!*cp) isDelComm=TRUE;
	}
	/* take care of comments after END*/
	while (!isDelComm && (cp=searchFirst(lp,"END","<NULL>"))) {
		cp+=3; // skip END
		while (isblank(*cp)) cp++;
		if (strncasecmp_(cp,"ELSE",4)) {
			while (*cp && *cp!=';' && *cp!='.') *cp++=' ';
		}
		lp+=3;
	}

	/* get label code number 
	 * labels are terminated by colon and may be more than one on the
	 * same line, and may be followed by any legal statement; 
	 * labels are constituted by numbers, letters and the underscore */
	lp=line;
	while ((z=findLabel(lp))) {
		int code, pos, i;
		char *za,*zb;
		--z;
		lab=thislabel;
		while (z>lp && islabelnamechar(z)) z--;
		if (*z==';') z++;
		za=z;
		while (isblank(*z)) z++;
		while (*z!=':') *lab++=*z++;
		*lab='\0';
		code=setAddr(thislabel, &lab);
		labelTOS++;
		/* too much labels */
		if (labelTOS>LABELLIM) pe(7,l);
		/* check if there is a duplicated value */
		i=0;
		while (i<labelTOS) {
			if (label[i].code==code) {
				pe(69,lnum);
			}
			i++;
		}
		label[labelTOS].code=code;
		label[labelTOS].ll=lnum;
		label[labelTOS].pl=pos;
		labelIndex[lnum]=labelTOS;
		zb=z+1;
		while (z>=za) *z--=' ';
		pos=(int)(z-lp);
		if (pos<0) pos=0;
		lp=zb;
		if (isVerboseDebug) {
			fprintf(stdout,"%s",INVCOLOR);
			fprintf(stdout,"Stored label #%d %s (%d, %d, %d)\n",labelTOS,thislabel,code,lnum,pos);
			fprintf(stdout,"%s",RESETCOLOR);
		}
	}

	lp=line;
	stripBlanks(lp);
	/* evaluate EXTERNAL procedures: load external file instead
	 * of current EXTERNAL procedure in case EXTERNAL is followed
	 * by a file name */
	if (!strncasecmp_(lp,"EXTERNAL",8) || !strncasecmp_(lp,"INCLUDE",7)) {
		char extfile[COMPSTR], *ef=extfile;
		int insertedLines=lnum;
		lp+=7; // skip INCLUDE or EXTERNA;
		if (toupper(*lp)=='L') lp++; // skip L of EXTERNAL;
		while (isblank(*lp)) lp++;
		/* if a file name is given load it */
		if (*lp=='\"' || *lp=='\'') {
			if (*lp=='\"') lp++;
			else if (*lp=='\'' && *(lp+1)=='\'') lp+=2;
			else pe(2,l);
			while (*lp && *lp!='\"' && *lp!='\'') 
				*ef++=*lp++;
			*ef='\0';
			mb[lnum][0]='\0';
			if (!strcmp(basename(filename),basename(extfile)))
				pe(84,lnum);
			lnum=LOAD(m,extfile,lnum);
			/* store library data */
			libTOS++;
			if (libTOS>=EXTERNLIBMAX) pe(7,0);
			lk=((int)strlen(extfile)+1)*sizeof(char);
			ext[libTOS]=salloc((size_t)lk);
			if (!ext[libTOS]) pe(6,lnum);
			strncpy_(ext[libTOS],extfile,lk);
			st[libTOS]=insertedLines;
			en[libTOS]=lnum;
		}
		globalInsertedLines+=lnum-insertedLines;
		if (isVerboseDebug) {
			fprintf(stdout,"%s",INVCOLOR);
			fprintf(stdout,"Loaded external file %s [#%d]\n",extfile,libTOS);
			fprintf(stdout,"%s",RESETCOLOR);
		}
		return lnum;
	}

	/* store line; if lp is a void line with only spaces and/or tabs, 
	 * it's stored anyway, but it won't be executed (see up) */
	if (*lp && (int)strlen(lp)>0) {
		lk=((int)strlen(lp)+1)*sizeof(char);
		m[lnum]=salloc((size_t)lk);
		if (!m[lnum]) pe(6,lnum);
		strncpy_(m[lnum],lp,lk);
	}
	/* if line is void, fill it with the null string */
	else {
		lk=(3)*sizeof(char);
		m[lnum]=salloc((size_t)lk);
		if (!m[lnum]) pe(6,lnum);
		strncpy_(m[lnum],"",lk); // null string
	}

	/* normal behavior */
	return lnum;
}


/* OLD() 
 * erase memory and call LOAD */
void OLD(void) {
	int i;

	/* erase core memory */
	for (i=0;i<MEMLIMIT;i++) {
		if (m[i]!=NIL && (int)strlen(m[i])>0) {
			free(m[i]);
			m[i]=NIL;
		}
	}

	/* LOAD */
	endCore=LOAD(m,filename,0);
	
}


/* getNextLine()
 * read file line, joins with next if ending with comma, deprive lines from
 * spurious characters left by DOS files, and from final blanks.
 * Return TRUE on correct reading, FALSE on missed reading */
/* TODO: specialize output:
 * - source not existing
 * - unsuitable lim
 * - unreferenced tank
 * - end of file on source
 * Return number of lines joined. 
 * Note: source for ASCII 10/13 in https://en.wikipedia.org/wiki/Newline */
int getNextLine(char *tank, int lim, FILE *source) {
	int count=1;
	if (!fgets(tank,lim,source)) return FALSE;
	/* strip away ASCII 13 left by DOS and Classic MacOS files */
	if (tank[(int)strlen(tank)-1]==13) tank[(int)strlen(tank)-1]='\0';
	/* strip away ASCII 10 left by DOS, UNIX, Linux, files */
	if (tank[(int)strlen(tank)-1]==10) tank[(int)strlen(tank)-1]='\0';
	/* strip away second ASCII 13 left by Acorn and Risc OS files */
	if (tank[(int)strlen(tank)-1]==13) tank[(int)strlen(tank)-1]='\0';
	/* strip away ending blanks */
	while (isblank(tank[(int)strlen(tank)-1])) 
		tank[(int)strlen(tank)-1]='\0';
	totchars+=strlen(tank);
	/* append next line if previous ends with comma */
	if (tank[(int)strlen(tank)-1]==',') {
		char addon[COMPSTR], *pA=addon;
		int incr;
		if(!(incr=getNextLine(addon,COMPSTR,source))) return FALSE;
		while (isblank(*pA)) pA++;
		strncat_(tank," ",COMPSTR);
		strncat_(tank,pA,COMPSTR);
		count+=incr;
	}
	return count;
}


/* checkForEND()
 * check if e points to the final dotted END 
 * system function for LOAD() */
int checkForEND(char *e) {
	if (!strncasecmp_(e,"END",3)) {
		e+=3;
		while (*e && *e!='.') e++;
		if (*e=='.') return TRUE;
	}
	return FALSE;
}


/* LOAD() 
 * load file in memory, from position following 'stt'
 * filestr: name of file to open
 * m: memory where program is stored
 * Note: this statement is is not available as a programming statement.
 * If '#' is set as first non blank character of a line, loading is ignored;
 * this lets using shell commands; not belonging to the DEC ALGOL 60 */
int LOAD(char **m, char *filestr, int stt) {
	FILE *f;
	char B[COMPSTR], *pB=B;
	int linenum=stt,incr;

	/* option -b add BEGIN */
	if (addMainRootBlock) {
		int lk=6*sizeof(char);
		linenum++;
		m[linenum]=salloc((size_t)lk);
		if (!m[linenum]) pe(6,0);
		strncpy_(m[linenum],"BEGIN",lk);
		mb[linenum]=salloc((size_t)lk);
		if (!mb[linenum]) pe(6,0);
		strncpy_(mb[linenum],"BEGIN",lk);
	}

	/* open file */
	if (!(f=fopen(filestr,"r"))) {
		beginS=-1;
		if (stt) pe(14,stt); 
		else pe(14,-1);
	}

	/* NOTE: if the file is a binary file, leave the error detection
	 * to the parsing phase (error #74 detection was removed because it interfered
	 * with execution under DOS and was meaningless)  */

	/* load lines one by one */
	while ((incr=getNextLine(B,COMPSTR,f)) && linenum<MEMLIMIT-1) {
		int stop=FALSE;
		linenum+=incr; pB=B;
		/* skip starting blanks */
		while (isblank(*pB)) pB++;
		/* discard void lines and lines beginning with # */
		if (!*pB || *pB=='#') continue;
		/* discard dash-commented lines (not ISO nor DEC) */
		if (*pB=='-') continue;
		/* set for stop at the final end */
		if (checkForEND(pB)) {
			stop=TRUE;
		}
		/* store line */
		linenum=storeSingleLineToCore(pB,linenum);
		incrLin[linenum]=incrementer-incr+1;

		if (stop) break;
	}

	/* check if file was entirely loaded */
	if (!feof(f) && linenum==MEMLIMIT-1) pe(7,-2);
	
	/* option -b add final END. (with dot) */
	if (addMainRootBlock) {
		int lk=5*sizeof(char);
		linenum++;
		m[linenum]=salloc((size_t)lk);
		if (!m[linenum]) pe(6,0);
		strncpy_(m[linenum],"END.",lk);
		mb[linenum]=salloc((size_t)lk);
		if (!mb[linenum]) pe(6,0);
		strncpy_(mb[linenum],"END.",lk);
	}

	/* close file channel */
	if (f) fclose(f);

	return linenum;
}


/* RUN()
 * the RUN command 
 * Note: this statement is executed at start automatically and is not 
 * available as a programming statement */
void RUN(void) {
	int i;

	//opturn=0; // for safety

	/* set numerical variables and relative flags*/
	for(i=0; i<LIMIT;i++) P[i]=pexec[i]=0.0;
	/* set numerical arrays references */
	for(i=0; i<LIMIT;i++) if (dim[i].elem) dim[i].elem=NIL;
	/* set string arrays references */
	for(i=0; i<LIMIT;i++) if (dimS[i].elem) dimS[i].elem=NIL;
	/* set PROCEDURE data */
	procTOS=pexecTOS=decCount=0;
	pexec[pexecTOS++]=0; // main level is zero, first proc call is one

	/* reset channels info specifics */
	for(i=1; i<=STREAMS; i++) {
		channel[i].type=NOUSE;
		channel[i].wayout=NOUSE;
		channel[i].isopen=FALSE;
	}
	rbuff[0]='\0'; rb=rbuff;

	/* set parameters for root level */
	Procp[0].procref=-1;		
	Procp[0].procL=0;		
	Procp[0].procP=0;
	Procp[0].begnL=0;
	Procp[0].begnP=0;
	Procp[0].termL=0;
	Procp[0].termP=0;
	Procp[0].numvars=0;		
	Procp[0].number=0;		
	Procp[0].type=0;		
	strcpy(Procp[0].name,"Root level - taxi");

	/* enable channel 0 for input */
	channel[CONSOLE].stream=stdin;
	channel[CONSOLE].isopen=TRUE;
	channel[CONSOLE].type=SEQ;
	channel[CONSOLE].fcounter=0;
	channel[CONSOLE].fprintn=1;
	channel[CONSOLE].pageL=1;
	channel[CONSOLE].pageN=1;
	channel[CONSOLE].wayout=VREAD;
	channel[CONSOLE].margin=SCRLIM;
	channel[CONSOLE].iochan=0547675;

	/* enable channel 1 for output */
	channel[1].stream=stdout;
	channel[1].isopen=TRUE;
	channel[1].type=SEQ;
	channel[1].fcounter=0;
	channel[1].fprintn=1;
	channel[1].pageL=1;
	channel[1].pageN=1;
	channel[1].wayout=VREAD+VWRITE;
	channel[1].margin=SCRLIM;
	channel[1].iochan=0547675;
	currentINPUT=CONSOLE;
	currentOUTPUT=OCONSOLE;

	/* erase remaining channels */
	for (i=2; i<=STREAMS; i++) {
		channel[i].stream=NIL;
	}

	/* reset FOR references and FOR/NEXT address array 
	 * specific for each RUN(), and RANDOMIZE */
	forTOS=ifTOS=0;
	RANDOMIZE(0);

	/* pre-parsing operations */
	preParse(m,1,endCore);
	isRunOn=TRUE;
	beginTOS=0;
	if (isNoExecute) return;

	/* print start message in case of debug */
	if (isVerboseDebug) {
		fprintf(stdout,"%s",INVCOLOR);
		fprintf(stdout,"%s","\n--PROGRAM STARTS--\n");
		fprintf(stdout,"%s",RESETCOLOR);
	}
	/* print a void line to separate taxi output */
	fputc('\n',stdout);
	/* and finally, execute program if everything in preParse() is OK */
	algolstart();
}


/* fprintb() - used with BIN$()
 * return an integer in binary format according to the following format:
 *   - bit 0 appears on the right (e.g. 13 => 1101);
 *   - negative numbers occupy all 32 bits;
 *   - if mode=FALSE return a number with leading zeroes
 * 'res' is the output string;
 * 'n' is the integer to be converted. 
 * 'w' is the bits width */
int fprintb(char *res, int n, int w, int mode) {
	int num[w+1], c=-1;
	char *pp=res;
	if (!n && mode) *pp++='0';
	while (n && (c < (w - 1))) num[++c] = n & 1, n >>= 1;
	if (!mode) {
		int y=w;
		while (--y>c) *pp++='0';
	}
	while (c >= 0) *pp++='0'+num[c--];
	*pp='\0';
	return (int)strlen(res);
}


/* closeFiles()
 * close all physical files 
 * return number of files that were closed (future usage) */
int closeFiles(void) {
	int i=0, none=0;

	for (i=BSTREAM; i<=STREAMS;i++) {
		if (channel[i].isopen) {
			if (channel[i].stream) 
				fclose(channel[i].stream);
			none++;
		}
	}
	return none;
}


/* END()
 * end program, accomplishing various tasks, such as closing opened channels,
 * fixing graphical stuff and closing regularly as C requires.
 * Note: as exit state, return the state passed to it as argument.
 * Note: as the returned states, I use:
 * EXIT_SUCCESS (which is 0) to indicate a proper successful execution
 * EXIT_FAILURE (which is 1) to indicate a stop after an error condition
 * EXIT_STOP (which is 2) to indicate an invoked STOP termination (CTRL-C) */
void END(int state) {
	int none=TRUE,i; // open file remnants flag

	/* GRAPHICAL OPERATIONS */
	/* CTRL-C was pressed: print to signal */
	if (isInterrupt) fprintf(stdout, "\n\ntaxi stopped after user interrupt at line %d\n\n",l);
	/* CTRL-C was pressed: print to signal */
	else if (state==EXIT_STOP) fprintf(stdout, "\ntaxi execution interrupted by STOP at line %d\n\n",l);
	/* add a Carriage Return in case of suspended PRINT */
	else if (channel[OCONSOLE].fcounter) fprintf(stdout, CRS);

	/* DEBUGGING INFO */
	if (isVerboseDebug && ! isNoExecute) {
		fprintf(stdout,"%s",INVCOLOR);
		if (state==EXIT_STOP) fprintf(stdout,"%s","\n--PROGRAM STOPPED---\n\n\n");
		else fprintf(stdout,"%s","\n--PROGRAM ENDS---\n\n\n");
		fprintf(stdout,"%s",RESETCOLOR);
	}

	if (isDebug) {
		fprintf(stdout,"%s",INVCOLOR);
		fprintf(stdout,"%s","Freeing memory... ");
		fprintf(stdout,"%s",RESETCOLOR);
	}

	for (i=0;i<=decCount;i++) {
		if (dim[i].elem) free(dim[i].elem);
		if (dimS[i].elem) free(dimS[i].elem);
		if (PS[i]) free(PS[i]);
	}
	for (i=0;i<=libTOS;i++) {
		if (ext[i]) free(ext[i]);
	}
	for (i=0;i<=ghostCount;i++) {
		if (own[i].numarr) free(own[i].numarr);
		if (own[i].strarr) free(own[i].strarr);
	}
	for (i=0;i<=endCore;i++) {
		if (m[i]) free(m[i]);
		if (mb[i]) free(mb[i]);
	}

	if (isDebug) {
		fprintf(stdout,"%s",INVCOLOR);
		fprintf(stdout,"%s","done\nClosing files... ");
		fprintf(stdout,"%s",RESETCOLOR);
	}
	/* close and setup all files */
	none=closeFiles();
	if (isDebug) {
		fprintf(stdout,"%s",INVCOLOR);
		fprintf(stdout,"%s","done\n");
		fprintf(stdout,"%s",RESETCOLOR);
	}
	
	if (isDebug) {
		int i;
		fprintf(stdout,"%s",INVCOLOR);
		if (ifExecTOS+ifExecTOS+ifExecTOS+beginTOS) {
			fprintf(stdout,"%s","\ndebug: structures remnants:\n");
			if (beginTOS) 
				fprintf(stdout,"  left open BEGIN-END : %d\n",beginTOS);
			if (ifExecTOS) 
				fprintf(stdout,"  unterminated IF     : %d\n",ifExecTOS);
			if (forExecTOS)
				fprintf(stdout,"  still hooked FOR    : %d\n",forExecTOS);
			if (whileExecTOS)
				fprintf(stdout,"  still hooked WHILE  : %d\n",whileExecTOS);
		}
	
		if (none) {
			fprintf(stdout,"%s","\ndebug: left open streams= : ");
			for (i=1; i<=OSTREAMS;i++) {
				if (channel[i].isopen) {
				fprintf(stdout,"%d, ",i);
				}
			}
			fprintf(stdout,"%s","\n\n");
		}
		fprintf(stdout,"%s",RESETCOLOR);
	}
	
	if (isDebug) {
		fprintf(stdout,"%s",INVCOLOR);
		fprintf(stdout,"%s","Resetting all channels... ");
		fprintf(stdout,"%s",RESETCOLOR);
	}
	/* FILE OPERATIONS */
	/* close channels */
	for (i=1;i<=OSTREAMS;i++) {
		channel[i].stream=NULL;
		channel[i].ioname[0]='\0';
		channel[i].isopen=0;
		channel[i].type=0;
		channel[i].fcounter=0;
		channel[i].pageL=0;
		channel[i].pageN=0;
		channel[i].fprintn=0;
		channel[i].margin=0;
		channel[i].wayout=0;
		channel[i].iochan=0;
	}
	if (isDebug) {
		fprintf(stdout,"%s",INVCOLOR);
		fprintf(stdout,"%s","done\nCompleting all printer jobs... ");
		fprintf(stdout,"%s",RESETCOLOR);
	}
	/* if there was a printer channel, direct to printer the result,
	 * close file and reset */
	if (isPrinterCH) {
		char command[COMPSTR];
		if (channel[isPrinterCH].stream) 
			fclose(channel[isPrinterCH].stream);
		sprintw(command,COMPSTR,"%s %s",ROUTESTRING,prTEMP);
		system(command);
		isPrinterCH=FALSE;
		if (remove(prTEMP)) fprintf(stderr,"%s\n", error[79]);
	}

	/* TIMING OPERATIONS */
	/* record end time 
	 * Note: if file was not found, beginS remains negative */
	if (beginS<0) endS=0.0;
	else {
		gettimeofday(&tv, NULL);
	       	endS = ((double)tv.tv_sec + tv.tv_usec / 1000000.0);
		endS=endS-beginS;
	}

	if (isDebug) {
		fprintf(stdout,"%s",INVCOLOR);
		fprintf(stdout,"%s","done\nClosing.\n\n");
		fprintf(stdout,"%s",RESETCOLOR);
	}
	/* in case of timing, add the timed calculation string */
	if (isTiming) {
		int hour=0,min=0;
		double secs=endS;
		/* pre-calculate intermediate time values */
		if (endS>60) {
			min=endS/60;
			endS=endS-min*60;
		}
		if (min>60) {
			hour=min/60;
			min=min-hour*60;
		}
		fputc('\n',stdout);
		/* depict time in condensed form */
		if (!hour && !min) {
			if (secs<1) 
			     fprintf(stdout,"TIME:  %.2f MSECS.\n\n",endS*1000);
			else 
			     fprintf(stdout,"TIME:  %.2f SECS.\n\n",endS);
		}
		else if (!hour)
			fprintf(stdout,"TIME:  %.2d:%.2d MIN%c	(%.2f SECS).\n\n",min,(int)(endS+0.5),min!=1?'S':' ',secs);
		else 	
			fprintf(stdout,"TIME:  %.2d:%.2d:%.2d HOUR%c   (%.2f SECS).\n\n",hour,min,(int)(endS+0.5),hour!=1?'S':' ',secs);
	}
	
	/* QUITTING */
	/* flush all open streams, to ensure file data writing is completed */
	fflush(NULL);
	/* print a void line to separate taxi output */
	fputc('\n',stdout);
	s_exit(state);
	system("reset");
}


/* s_exit()
 * safe exit - avoid messing up of colours, and force the screen resetting
 * return 'state' as current state
 * system function */
void s_exit(int state) {
	/* reset screen */
	fprintf(stdout,"%s",RESETCOLOR);
	exit(state);
}


/******************************************************************************
 * The following functions define the whole parser engine:
 * SS() 	   : the strings expression parser
 * strRel()        : the strings relation evaluator (returns a truth value)
 * S() 		   : the numerical expression parser
 * getVal()	   : the single numerical value parser 
 * getFunc()	   : the numerical function parser
 * priority()	   : return numerical operator precedence
 * setResultType() : set result type of an expression depending on a scale
 * conceived April 2008 for the dib project
 * modified for vibes with strings insertion September-November 2009
 * updated and verified for taxi - February 2013, February-Summer 2014
 * updated and typized for taxi - February-Summer 2016
 * Copyleft Antonio Maschio - 2008-2016 
 ******************************************************************************/

/* SS()
 * The PARSER for string expression; it's a general routine.
 * The address of starting position into the flow stream is pointed by p 
 * The algorithm is as follows: 
 * until there is a string in any form, append it to mypad; return mypad */
char *SS(void) {
	int i,ns,type=0;
	double numres;
	char mypad[COMPSTR], strres[COMPSTR], *opT;

	/* create local pad and reset */
	for (i=0;i<COMPSTR;i++) mypad[i]='\0';
	MYPAD=mypad;
	p--;

	/* calculate string result */
	do {
		p++;
		/* literal or var label */
		if (isLabelAssign) {
			char lab[COMPSTR], *k;
			int count=0, code;
			strncat_(lab,SS(),COMPSTR);
			code=setAddr(lab,&k);
			for (count=1; count<=labelTOS; count++) {
				if (label[count].code==code) {
					isLabelAssign=count;
					break;
				}
			}
			if (isLabelAssign==-1) pe(11,l);
		}
		/* open/close parenthesis */
		else if (*p=='(') {
			p++;
			strncat_(mypad,SS(),COMPSTR);
			if (*p==')') p++;
			else pe(10,l);
		}
		/* IF..THEN..ELSE
		 * conditional expression
		 * Ref. AA-O196C-TK - 14 "Conditional Expressions and 
		 * Statements" */
		else if (!strncasecmp_(p,"IF",2)) {
			int cond=FALSE;
			char op1[COMPSTR], op2[COMPSTR];
			p+=2;
			if (!check(p,"THEN") && !check(p," HEN")) pe(2,l);
			cond=(int)S();
			*p='T';
			p+=4; // skip THEN
			if (!check(p,"ELSE") && !check(p," LSE")) pe(2,l);
			strncpy_(op1,SS(),COMPSTR);
			*p='E';
			p+=4; // skip ELSE
			strncpy_(op2,SS(),COMPSTR);
			if (cond) {
				if((int)strlen(mypad)+(int)strlen(op1)>=COMPSTR)
					pe(30,l);
				strncat_(mypad,op1,COMPSTR);
			}
			else {
				if((int)strlen(mypad)+(int)strlen(op2)>=COMPSTR)
					pe(30,l);
				strncat_(mypad,op2,COMPSTR);
			}
		}
		/* expicit string surrounded by double quotes " */
		else if (*p=='\"') {
			char op[COMPSTR], *P1=op;
			int len=0;
			p++; // skip "
			while (*p && len<COMPSTR) {
				if (*p=='\"' && *(p+1)=='\"') p++;
				else if (*p==';' && *(p+1)==';') p++;
				else if (!*p || *p=='\"' || ister(p)) break;
				*P1++=*p++, len++; 
			}
			*P1='\0';
			if (*p=='\"') p++; // skip closing quotes
			/* join subsequent string */
			else if (ister(p)) while (ister(p)) {
				l++;
				while (!m[l]) l++;
				if (l>endCore) pe(23,l);
				strncpy_(BASESTR,m[l],COMPSTR);
				p=BASESTR;
				while (*p && len<COMPSTR) {
					if (*p=='\"' && *(p+1)=='\"') p++;
					else if (*p==';' && *(p+1)==';') p++;
					else if (*p=='\"' || ister(p)) break;
					*P1++=*p++, len++; 
				}
				*P1='\0';
				if (*p=='\"') {
					p++;
					break;
				}
			}
			if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR) 
				pe(30,l);
			strncat_(mypad,op,COMPSTR);
		}
		/* expicit string surrounded by two single quotes '' */
		else if (*p=='\'' && *(p+1)=='\'') {
			char op[COMPSTR], *P1=op;
			int len=0;
			p+=2;
			while (*p && len<COMPSTR) {
				if (*p=='\"' && *(p+1)=='\"') p++;
				else if (*p==';' && *(p+1)==';') p++;
				else if (isdc(p) || ister(p)) break;
				*P1++=*p++, len++; 
			}
			*P1='\0';
			if (isdc(p)) p+=2; // skip quotes
			/* join subsequent string */
			else if (ister(p)) while (ister(p)) {
				l++;
				while (!m[l]) l++;
				if (l>endCore) pe(23,l);
				strncpy_(BASESTR,m[l],COMPSTR);
				p=BASESTR;
				while (*p && len<COMPSTR) {
					if (*p=='\"' && *(p+1)=='\"') p++;
					else if (*p==';' && *(p+1)==';') p++;
					else if (isdc(p) || ister(p)) break;
					*P1++=*p++, len++; 
				}
				*P1='\0';
				if (isdc(p)) {
					p+=2;
					break;
				}
			}
			else pe(18,l);
			if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR) 
				pe(30,l);
			strncat_(mypad,op,COMPSTR);
		}
		/* STRING VARIABLES */
		else if ((ns=findDEC(p)) && *(p+_len)!='(') {
			char op[COMPSTR];
			p+=_len;
			/* variable to which a function or a string expression
			 * is passed. In this case the string is
			 * taken from valdata */
			if (vn[ns].isFunc || vn[ns].refcode==2*DECADD){
				strncpy(op,vn[ns].valdata,COMPSTR);
				if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR)
					pe(30,l);
				strncat_(mypad,op,COMPSTR);
				continue;
			}
			/* variable reference to an array element 
			 * Ref. AA-O196C-TK - 11.5.1 "Jensen's Device" */
			if (vn[ns].refcode<-DECADD){
				char *pp=p;
				p=vn[ns].valdata;
				strncpy(op,SS(),COMPSTR);
				p=pp;
				if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR)
					pe(30,l);
				strncat_(mypad,op,COMPSTR);
				continue;
			}
			/* variable or array passed by reference */
			else if (vn[ns].refcode<0) {
				ns=vn[ns].ref;
			}
			
			/* return a string variable */
			if (*(p)!='[' && !_arr) {
				int var=vn[ns].ref;
				if ((int)strlen(mypad)+(int)strlen(PS[var])>=COMPSTR)
					pe(30,l);
				strncat_(mypad,PS[var],COMPSTR);
			}
			/* return a string array element */
			else if (*(p)=='[' && _arr) {
				int val[MAXDIM+1];
				int T=1;
				p++;
				val[T]=(int)S();
				while (*p==',') {
					p++; T++;
					if (T>MAXDIM) pe(36,l);
					val[T]=(int)S();
				}
				if (*p==']') p++;
				else pe(2,l);
				strncpy_(op,getArrayStr(vn[ns].ref,vn[vn[ns].ref].dim,val),COMPSTR);
				if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR)
					pe(30,l);
				strncat_(mypad,op,COMPSTR); 
			}
		}
		/* STRING PROCEDUREs  
	         * Ref. AA-O196C-TK - 11 "PROCEDURES"*/
		else if ((ns=findPROC(p,&opT,&type))) {
			char op[COMPSTR];
			if (type==STRINGTYPEBOX)	{
				/* normal user procedure-delegate execPROC() */
				if (Procp[ns].procL>0) {
					p=execPROC(ns,opT,&numres,strres);
					strncpy_(op,strres,COMPSTR);
				}
				/* argument procedure - use SS() */
				else if (Procp[ns].procL<0) {
					char exstring[COMPSTR], *e;
					char *pb=p+strlen(Procp[ns].name);
					e=exstring;
					strncpy_(exstring,Procp[ns].strdata,COMPSTR);
					e+=strlen(Procp[ns].strdata);
					if (*pb=='(') {
						int beg=0;
						*e++=*pb++; // copy (
						while (*pb) {
							if (*pb=='(') beg++;
							else if (*pb==')' && beg) 
								beg--;
							else if (*pb==')' && !beg) 
								break;
							*e++=*pb++;
						}
						if (*pb==')') *e++=*pb++;
						else pe(2,l);
						*e='\0';
					}
					else pe(2,l);
					p=exstring;
					strncpy_(op,SS(),COMPSTR);
					p=pb;
				}
				else pe(31,l);
				resultType=Procp[ns].type;
			}
			else pe(31,l);
			if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR)
				pe(30,l);
			strncat_(mypad,op,COMPSTR); 
		}
		/* CONCAT string function */
		else if (!strncasecmp_(p,"CONCAT(",7))  {
			char op1[COMPSTR],op2[COMPSTR];
			p+=7;
			/* first string */
			strncpy_(op1,SS(),COMPSTR);
			/* further strings */
			while (skip() || *p==',') {
				if (*p==',') p++; // skip comma
				strncpy_(op2,SS(),COMPSTR);
				strncat_(op1,op2,COMPSTR);
			}
			if (*p==')') p++;
			else pe(2,l);
			if ((int)strlen(mypad)+(int)strlen(op1)>=COMPSTR) 
				pe(30,l);
			strncat_(mypad,op1,COMPSTR);
		}
		/* COPY string function */
		else if (!strncasecmp_(p,"COPY(",5))  {
			char op[COMPSTR], output[COMPSTR];
			int start=0, end=0;
			p+=5;
			/* model string */
			strncpy_(op,SS(),COMPSTR);
			/*  optional arguments */
			if (skip() || *p==',') {
				if (*p==',') p++;
				end=(int)S();
				if (skip() || *p==',') {
					if (*p==',') p++;
					start=end-1;
					end=(int)S();
				}
			}
			else end=COMPSTR;
			if (start<0) start=0;
			if (end>strlen(op)) end=strlen(op);
			if (start>end) start=end=0;
			if (*p==')') p++;
			else pe(2,l);
			strncpy_(output,op+start,end-start+1);
			if ((int)strlen(mypad)+(int)strlen(output)>=COMPSTR) 
				pe(30,l);
			strncat_(mypad,output,COMPSTR);
		}
		/* FDATE string function */
		else if (!strncasecmp_(p,"FDATE",5) && !isnamechar((p+5)))  {
			char op[COMPSTR];
			int day=getDay();
			int year=getYear()-2000;
			p+=5;
			sprintw(op,COMPSTR,"%02d-%s-%02d",day,getMonthString(1),year);
			if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR) 
				pe(30,l);
			strncpy_(mypad,op,COMPSTR);
		}
		/* TIME string function */
		else if (!strncasecmp_(p,"TIME",4) && !isnamechar((p+4)))  {
			char op[COMPSTR];
			int hour=getHour();
			int minute=getMin();
			int sec=getSec();
			p+=4;
			sprintw(op,COMPSTR,"%02d-%02d-%02d",hour,minute,sec);
			if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR)
					pe(30,l);
			strncpy_(mypad,op,COMPSTR);
		}
		/* VDATE string function */
		else if (!strncasecmp_(p,"VDATE",5) && !isnamechar((p+7)))  {
			char op[COMPSTR];
			int day=getDay();
			int year=getYear()-2000;
			p+=5;
			sprintw(op,COMPSTR,"%02d-%s-%02d",day,getMonthString(0),year);
			if ((int)strlen(mypad)+(int)strlen(op)>=COMPSTR)
					pe(30,l);
			strncpy_(mypad,op,COMPSTR);
		}
		/* CONVERT string function */
		else if (!strncasecmp_(p,"CONVERT(",8)) {
			char base[COMPSTR];
			double n;
			char spc=' ';
			p+=8;
			n=S();
			if (n<0) n=-n, spc='-';
			/* integer case */
			if (resultType==INTEGERTYPEBOX) {
				sprintw(base,COMPSTR,"%c%d ",spc,(int)n);
			}
			/* not integer case */
			else {
				/* use %g for outsource the number */
				sprintw(base,COMPSTR,"%c%g ",spc,n);
				/* if it contains 'e' it was in exponential form */
				if (strchr(base,'e')) {
					if (resultType==REALTYPEBOX) {
						printExp(0,n,spc,resultType,9,base);
					}
					else {
						printExp(0,n,spc,resultType,17,base);
					}
				}
			}
			if (*p==')') p++;
			else pe(2,l);
			if ((int)strlen(mypad)+(int)strlen(base)>=COMPSTR)
					pe(30,l);
			strncpy_(mypad,base,COMPSTR);
		}
		/* TAKE string function */
		else if (!strncasecmp_(p,"TAKE(",5)) {
			char base[COMPSTR],*b=base;
			int c;
			p+=5;
			strncpy_(base,SS(),COMPSTR);
			if (*p==',') p++;
			else if (skip());
			else pe(83,l);
			c=(int)S();
			if (c<0) c=0;
			if (c>strlen(base)) c=strlen(base);
			if (*p==')') p++;
			else pe(2,l);
			/* copy c characters */
			while (c) {
				if ((int)strlen(mypad)+1>=COMPSTR) pe(30,l);
				chrcat_(mypad,*b++);
				c--;
			}	
		}
		/* DROP string function */
		else if (!strncasecmp_(p,"DROP(",5)) {
			char base[COMPSTR],*b=base;
			int c;
			p+=5;
			strncpy_(base,SS(),COMPSTR);
			if (*p==',') p++;
			else if (skip());
			else pe(83,l);
			c=(int)S();
			if (c<0) c=0;
			if (c>strlen(base)) c=strlen(base);
			if (*p==')') p++;
			else pe(2,l);
			/* discard c characters and copy until the end */
			b+=c;
			while (*b) {
				if ((int)strlen(mypad)+1>=COMPSTR) pe(30,l);
				chrcat_(mypad,*b++);
			}	
		}
		/* HEAD string function */
		else if (!strncasecmp_(p,"HEAD(",5)) {
			char base[COMPSTR],*b=base;
			char match[COMPSTR];
			p+=5;
			strncpy_(base,SS(),COMPSTR);
			if (*p==',') p++;
			else if (skip());
			else pe(83,l);
			strncpy_(match,SS(),COMPSTR);
			if (*p==')') p++;
			else pe(2,l);
			/* copy until one in match is found */
			while (!strchr(match,*b)) {
				if ((int)strlen(mypad)+1>=COMPSTR) pe(30,l);
				chrcat_(mypad,*b++);
			}	
		}
		/* TAIL string function */
		else if (!strncasecmp_(p,"TAIL(",5)) {
			char base[COMPSTR],*b=base;
			char match[COMPSTR];
			p+=5;
			strncpy_(base,SS(),COMPSTR);
			if (*p==',') p++;
			else if (skip());
			else pe(83,l);
			strncpy_(match,SS(),COMPSTR);
			if (*p==')') p++;
			else pe(2,l);
			/* discard until one in match is found and then 
			 * copy until the end */
			while (!strchr(match,*b)) b++;
			while (*b) {
				if ((int)strlen(mypad)+1>=COMPSTR) pe(30,l);
				chrcat_(mypad,*b++);
			}	
		}
		/* intercept NEWSTRING (it is evaluated in assign() )*/
		else if (!strncasecmp_(p,"NEWSTRING(",10));
		/* numeric variables cause error */
		else if ((ns=findDEC(p))&&(_type<STRINGTYPEBOX||_type==CHAR)){
			pe(9,l);
		}
		/* no possible evaluation */
		else pe(10,l); 
	} while ((*p=='\"'|| (*p=='\'' && *(p+1)=='\'')) && p--);
	strncpy_(sspad,mypad,COMPSTR);
	MYPAD=mypad;
	return MYPAD;
}


/* strRel()
 * evaluate a string expression; operators <, >, =, #, { (<=) and } (>=); 
 * always return a truth value
 * Note: in relations, any kind of string variable (even string arrays)
 * may be used into the formula.
 * Note: since evaluation of string relations returns a truth value, which is
 * a number, it is useful only into IF..THEN statements and into S(); so they 
 * are recognized/used there, and not into SS() */
int strRel(void) {
	int rel=FALSE, operator;
	char op1[COMPSTR], op2[COMPSTR], *P1=op1, *P2=op2;

	/* read operands and operator */
	strncpy_(op1,SS(),COMPSTR);
	if (isleq(p)) operator='{', p+=2;	// <=
	else if (isgeq(p)) operator='}', p+=2;	// >=
	else if (*p=='<') operator='<', p++;	// <
	else if (*p=='>') operator='>', p++;	// >
	else if (*p=='#') operator='#', p++;	// #
	else if (*p=='=') operator='=', p++;	// =
	else pe(81,l); 
	strncpy_(op2,SS(),COMPSTR);

	/* delete trailing spaces before and after strings */
	if ((int)strlen(op1)>0) {
		P1=op1+(int)strlen(op1)-1; 
		while ((int)(P1-op1)>0 && isblank(*P1)) *P1--='\0';
	}
	if ((int)strlen(op2)>0) {
		P2=op2+(int)strlen(op2)-1; 
		while ((int)(P2-op2)>0 && isblank(*P2)) *P2--='\0';
	}
	P1=op1; while ((int)(P1-op1)<COMPSTR && isblank(*P1)) P1++;
	P2=op2; while ((int)(P2-op2)<COMPSTR && isblank(*P2)) P2++;

	/* apply operator ; case matter, so we use strcmp() */
	switch(operator) {
		case '<': if (strcmp(P1,P2) < 0)  rel=TRUE; break;
		case '>': if (strcmp(P1,P2) > 0)  rel=TRUE; break;
		case '=': if (!strcmp(P1,P2))     rel=TRUE; break;
		case '#': if (strcmp(P1,P2))      rel=TRUE; break;
		case '{': if (strcmp(P1,P2) <= 0) rel=TRUE; break;
		case '}': if (strcmp(P1,P2) >= 0) rel=TRUE; break;
		default:  rel=FALSE;
	}
	return rel;
}


/* priority() 
 * return math priority for operator op, according to the following table:
 * 10: the minus and the NOT (caught in S()) are evaluated immediately
 * 9: (..) parentheses (caught in getVal) have the highest precedence
 * caught here:
 * 8: power
 * 7: multiplication, division, DIV \ ÷ REM `
 * 6: addition, subtraction
 * 5: <, <= {, =, >, >= }, # 
 * 4: AND _
 * 3: OR |
 * 2: IMP !
 * 1: EQV ?
 * 0: anything else has NIL priority and should cause error 
 * Note: The NOT operator has the higher precedence because it refers to the
 * term that immediately follows (in contrast to the ISO directives); 
 * the same for the minus sign. */
int priority(const int op) {
	if (op=='^') return 8;
	else if (op=='*'||op=='/'||op=='\\'||op=='`') return 7;
	else if (op=='+'||op=='-') return 6;
	else if (op=='<'||op=='{'||op=='='||op=='>'||op=='}'||op=='#') 
		return 5;
	else if (op=='_') return 4;
	else if (op=='|') return 3;
	else if (op=='!') return 2;
	else if (op=='?') return 1;
	return 0;
}
#define PRIOR 8


/* S()
 * The PARSER for numerical expressions
 * The address of starting position into the flow stream is pointed by p 
 * The algorithm is as follows: 
 * an algebraic expression is turned into an RPN stack-based expression, 
 * with numbers and operators in order in their own stacks; operations are 
 * performed from higher priorities down to lower and from left to right;  
 * priorities are given by the priority() function, which is part of this tool.
 * Note: the < and > operators (with their companion <= and >=) may not give 
 * correct results with big numbers, because of the ways of storing numbers; 
 * for instance, 1E29*10 is not stored in the same way of 1E30, because of 
 * rounding errors in calculating the multiplication (the error propagates 
 * rapidly when big numbers are calculated in series) */
double S(void) {
	double numStack[PSTACK];
	int opStack[PSTACK], prStack[PSTACK], tpStack[PSTACK];
	int opturn=0, numTOS=0, opTOS=0, ii,jj, level;
	char *pcopy=NIL;

	/* reset result type */
	resultType=0;

	/* catch unary plus at start (simply discarded because redundant)
	 * Note: taxi parser is built in such a way that the unary sign 
	 * (+ or -) is tightly bound to the entity following it, so that 
	 * operations like -3+-2, +3+-2, -3-+2, +3-+2 are accepted */
	if (*p=='+') p++;
	/* catch unary minus at start */
	else if (*p=='-') {
		double res;
		p++; // skip '-'
		res=getVal();
		if (*p=='^') {
			/* power following a unary minus has higher priority */
			double expon;
			int sign=1;
			if (*p=='^') p++;
			else p+=2;
			if (*p=='-') p++, sign=-1;
			else if (*p=='+') p++;
			expon=sign*getVal();
			res=-pow(res,expon);
		}
		else res=-res;
		tpStack[numTOS]=resultType;
		numStack[numTOS++]=res;
		opturn=1;
		/* short circuit: if number is followed by comma, semicolon
		 * or colon or closed parentheses or backslash or void, 
		 * return result immediately */
		if (!*p||*p==','||*p==';'||*p==':'||*p==')' ||*p==']') 
			return res;
		/* short circuit #2: if the -a^x exponent is not followed
		 * by an operator, return result immediately */
		if (!isop(p)) return res;
	}
	pcopy=p;
	/* reverse algebraic expression into a stack-based structure */
	while (*p && *p!=';' && !isblank(*p)){
		/* reload */
		if (!*p) {
			l++;
			while (!m[l]) l++;
			if (l>endCore) pe(23,l);
			strncpy_(BASESTR,m[l],COMPSTR);
			p=BASESTR;
		}
		/* catch an operator */
		if ((isop(p)) && opturn) {
			opStack[opTOS]=*p;
			prStack[opTOS]=priority(opStack[opTOS]);
			opTOS++;
			p++;
			opturn=0;
		}
		/* catch a closed parenthesis */
		else if (*p==')') break;
		/* catch the unary minus after an operator */
		else if (*p=='-' && !opturn) {
			p++;
			numStack[numTOS]=-getVal();
			tpStack[numTOS++]=resultType;
			opturn=1;
		}
		/* catch the NOT as keyword after an operator */
		else if (!strncasecmp_(p,"NOT",3) && !opturn) {
			p+=3;
			numStack[numTOS]=-(double)(!((int)getVal()));
			tpStack[numTOS++]=resultType;
			opturn=1;
		}
		/* catch the NOT as symbol after an operator */
		else if (*p=='~' && !opturn) {
			p++;
			numStack[numTOS]=-(double)(!((int)getVal()));
			tpStack[numTOS++]=resultType;
			opturn=1;
		}
		/* catch the unary plus after an operator */
		else if (*p=='+' && !opturn) {
			p++;
			numStack[numTOS]=getVal();
			tpStack[numTOS++]=resultType;
			opturn=1;
		}
		/* catch a number (literal or variable) */
		else {
			numStack[numTOS]=getVal();
			tpStack[numTOS++]=resultType;
			opturn=1;
			if (*p && !isop(p) && opturn) break;
		}
		
		/* reload if number is followed by void */
		if (!*p && !opturn && l<endCore) {
			l++;
			while (!m[l]) l++;
			if (l>endCore) pe(23,l);
			strncpy_(BASESTR,m[l],COMPSTR);
			p=BASESTR;
		}
		
		/* Since the string was evaluated, change pcopy to signal
		 * the work is done */
		pcopy=NIL;
		if (*p==',') break;
	}
	/* No parsing was done if the string is not void */
	if (pcopy==p && *p!=';' && strncasecmp_(p,"END",3) &&\
			strncasecmp_(p,"ELSE",4)) 
		pe(2,l);
	/* perform calculations from higher priorities downward
	 * Note: for taxi, any relation is also a math expression */
	for (ii=PRIOR;ii>=0;ii--) {
		/* calculations are performed from left to right */
		for (level=0;level<opTOS;level++) {
			if (prStack[level]==ii) {
				/* collapse stacks */
				switch(opStack[level]) {
// HERE BEGINS OPERATORS ANALYSIS
/* lesser than < */
case '<': numStack[level]=-(double)(numStack[level]<numStack[level+1]); 
	  setResultType(BOOLEANTYPEBOX); break; 
/* greater than > */
case '>': numStack[level]=-(double)(numStack[level]>numStack[level+1]); 
	  setResultType(BOOLEANTYPEBOX); break; 
/* not equal # */
case '#': numStack[level]=-(double)(numStack[level]!=numStack[level+1]); 
	  setResultType(BOOLEANTYPEBOX); break;
/* greater than or equal >= */
case '}': numStack[level]=-(double)(numStack[level]>=numStack[level+1]); 
	  setResultType(BOOLEANTYPEBOX); break;
/* lesser than or equal <= */
case '{': numStack[level]=-(double)(numStack[level]<=numStack[level+1]); 
	  setResultType(BOOLEANTYPEBOX); break;
/* equal to */
case '=': numStack[level]=-(double)(numStack[level]==numStack[level+1]); 
	  setResultType(BOOLEANTYPEBOX); break;
/* power ^ */
case '^': numStack[level]=pow(numStack[level],numStack[level+1]); 
	  setResultType(tpStack[level]);
	  setResultType(tpStack[level+1]);
	  break;
/* subtraction - */
case '-': numStack[level]-=numStack[level+1]; 
	  setResultType(tpStack[level]);
	  setResultType(tpStack[level+1]);
	  break;
/* addition + */
case '+': numStack[level]+=numStack[level+1]; 
	  setResultType(tpStack[level]);
	  setResultType(tpStack[level+1]);
	  break;
/* division */
case '/': {
		double div=numStack[level+1];
		if (fabsl(div)<ZERO) {
			pe(47,l);
		}
		else numStack[level]/=div;
	  }
	  setResultType(REALTYPEBOX);
	  break;
/* multiplication * */
case '*': numStack[level]*=numStack[level+1]; 
	  setResultType(tpStack[level]);
	  setResultType(tpStack[level+1]);
	  break;
/* bitwise AND */
case '_': 
	numStack[level]=(double)((int)(numStack[level])&((int)numStack[level+1]));
	setResultType(INTEGERTYPEBOX);
	break;
/* bitwise OR */
case '|': 
	numStack[level]=(double)((int)(numStack[level])|((int)numStack[level+1]));
	setResultType(INTEGERTYPEBOX);
	break;
/* logical IMP */
case '!': 
        if (!(!numStack[level]) & !(!(!numStack[level+1]))) numStack[level]=0;
	else numStack[level]=-1;
	setResultType(INTEGERTYPEBOX);
	break;
/* logical EQV */
case '?': 
        if (!(!numStack[level]) == !(!numStack[level+1])) numStack[level]=-1;
	else numStack[level]=0;
	setResultType(INTEGERTYPEBOX);
	break;
/* integer division DIV */
case '\\': 
        numStack[level]=1.0*(int)numStack[level]/(int)numStack[level+1]; 
	setResultType(INTEGERTYPEBOX);
	break;
/* integer remainder REM */
case '`': 
	{
	int div=(int)numStack[level]/(int)numStack[level+1];
	numStack[level]=(int)(numStack[level]-div*(int)numStack[level+1]);
	setResultType(INTEGERTYPEBOX);
	break;
	}
// HERE ENDS OPERATORS ANALYSIS
				}
				/* shift stack positions */
				for (jj=level;jj<opTOS-1;jj++) {
					numStack[jj+1]=numStack[jj+2];
					opStack[jj]=opStack[jj+1];
					prStack[jj]=prStack[jj+1];
				}
				opTOS--;
				level--;
			}
		}
	}
	
	/* At this point, we have the final result into cell [0] */
	return numStack[0];
}


/* setResultType()
 * update result type of an expression according to this rules:
 * ~- a LONG REAL absorbs all others
 * ~- otherwise a REAL absorbs INTEGERS and BOOLEANS
 * ~- otherwise an INTEGER absorbs BOOLEANS 
 * ~- otherwise expression type is BOOLEAN
 * Note: this routine does not control if tpye is STRING or LABEL */
void setResultType(int n) {
	if (n<resultType || !resultType) resultType=n;
}


/* getVal()
 * get single value (number, variable, function or array element) 
 * the case of the unary minus for entities has been caught in S(); here is
 * treated the case of open parentheses
 * The address of starting position into the flow stream is pointed by p */
double getVal(void){
	double o=0;

	/* label assignment 
	 * evaluate string, and check if there's a label variable or
	 * a literal label string */
	if (isLabelAssign) {
		int ns=findDEC(p);
		if (ns && _type==LABELTYPEBOX) 
			isLabelAssign=vn[ns].strmem; // var label code
		else SS();  // consume string
		return 0.0; // unused
	}
	/* case of a real number 
	 * evaluate also the &, &&, @, @@, E and D exponential characters */
	if (isnm(p)) {
		char *q, *pp=p;
		double mant;
		int exp, offset=1;
		char num[COMPSTR], *n=num;
		/* get value */
		if (*p=='+' || *p=='-') *n++=*p++;
		if (*p=='.') *n++=*p++;
		while (isdigit(*p)) *n++=*p++;
		if (*p=='.') *n++=*p++;
		while (isdigit(*p)) *n++=*p++;
		if (!strncasecmp_(p,"@@",2)) *n++=*p++,*n++=*p++;
		else if (!strncasecmp_(p,"&&",2)) *n++=*p++,*n++=*p++;
		else if (*p=='&') *n++=*p++;
		else if (*p=='@') *n++=*p++;
		else if (*p=='E') *n++=*p++;
		else if (*p=='D') *n++=*p++;
		if (*p=='+' || *p=='-') *n++=*p++;
		while (isdigit(*p)) *n++=*p++;
		*n='\0';
		n=num;
		if ((q=strcasestr_oq(n,"&&"))) {
			setResultType(LONGREALTYPEBOX);
			offset++;
		}
		else if ((q=strcasestr_oq(n,"@@"))) {
			setResultType(LONGREALTYPEBOX);
			offset++;
		}
		else if ((q=strcasestr_oq(n,"D"))) {
			setResultType(LONGREALTYPEBOX);
		}
		else if ((q=strcasestr_oq(n,"E"))) {
			setResultType(REALTYPEBOX);
		}
		else if ((q=strcasestr_oq(n,"&"))) {
			setResultType(REALTYPEBOX);
		}
		else if ((q=strcasestr_oq(n,"@"))) {
			setResultType(REALTYPEBOX);
		}
		/* exponential (only exponent given) */
		if (q==n) {
			exp=strtol(q+offset,NULL,10);
			o=1.0*pow(10.0,exp);
			setResultType(REALTYPEBOX);
		}
		/* exponential */
		else if (q) {
			*q='\0';
			mant=strtold(n,&n);
			exp=strtol(q+offset,NULL,10);
			o=mant*pow(10.0,exp);
			setResultType(REALTYPEBOX);
		}
		/* number */
		else {
			p=pp;
			o=strtold(p,&p);
			if (o==(int)o) setResultType(INTEGERTYPEBOX);
			else setResultType(REALTYPEBOX);
		}
	}
	/* case of exponential with unity digit discarded 
	 * (e.g. &-6 for 1&-6 or &&+6 for 1&&+6) */
	else if (*p=='&' || *p=='@') {
		char store[COMPSTR], *n=store;
		*n++='1';
		setResultType(REALTYPEBOX);
		if (!strncasecmp_(p,"@@",2)) {
			*n++='E', p++, p++;
			setResultType(LONGREALTYPEBOX);
		}
		else if (!strncasecmp_(p,"&&",2)) {
			*n++='E', p++, p++;
			setResultType(LONGREALTYPEBOX);
		}
		else if (*p=='&') *n++='E', p++;
		else if (*p=='@') *n++='E', p++;
		*s='\0';
		o=strtold(store,NULL);
	}
	/* case of unary minus 
	 * (for general cases, e.g. before a parenthesis) */
	else if (*p=='-') {
		p++;
		o=-getVal();
	}
	/* case of NOT
	 * (for general cases, e.g. before a parenthesis) */
	else if (!strncasecmp_(p,"NOT",3)) {
		p+=3;
		o=-(double)(!((int)getVal()));
	}
	/* case of ~ (NOT - machine token )
	 * (for general cases, e.g. before a parenthesis) */
	else if (*p=='~') {
		p++;
		o=-(double)(!((int)getVal()));
	}
	/* perform the inner assignment 
	 * Ref. 6.3 "Multiple Assignments" 
	 * Note: the result type here is the same of the assignment */
	else if (*p=='(' && isInnerAssign(p)) {
		int cs;
		p++;	// skip (
		isInn=TRUE;
		if (!(cs=assign())) pe(2,l);
		else p++; 	// skip )
		o=P[vn[cs].ref];
		setResultType(vn[vn[cs].ref].type);
	}
	/* case of ASCII value */ 
	else if (*p=='$') { 
		int i=0, sep;
		char nres[8], asciiv[ALGSTR], *k=asciiv;
		int ires;
		p++;
		sep=*p++;
		while (*p && *p!=sep && i<4) nres[i++]=*p++;
		nres[i]='\0';
		if (*p==sep) p++;
		else pe(2,l);
		i=0;
		while (nres[i]) {
			fprintb(k,nres[i]%128,8,FALSE);
			i++;
			k+=8;
		}
		*k='\0';
		ires=strtol(asciiv,NULL,2);
		o=(double)ires;
		setResultType(INTEGERTYPEBOX);
	}
	/* case of octal literal */ 
	else if (*p=='%') { 
		int i=0;
		char nres[20];
		p++;
		while (*p && *p<='7' && *p>='0' && i<11) nres[i++]=*p++;
		nres[i]='\0';
		o=strtol(nres,NULL,8);
		setResultType(INTEGERTYPEBOX);
	}
	/* case of open parenthesis 
	 * Type does not change and is eventually set into the body of
	 * parentheses and passed bare */ 
	else if (*p=='(') { 
		p++;
		o=S();
		if (*p!=')') pe(2,l);
		else p++; 
	}
	/* case of a string relation (which returns a numeric truth value) */
	else if (isanystring(p)) {
		o=strRel();
		setResultType(BOOLEANTYPEBOX);
	}
	/* delegate function calculations to getFunc() */
	else if (isbeginnamechar(p)) { 
		o=getFunc();
		p++; 
	}
	/* unrecognized object */
	else pe(9,l);
	return o;
}


/* skip()
 * ignores the characters that form the parameters comments */
int skip(void) {
	if (*p==')' && *(p+1)==';') return FALSE;
	else if (*p==')' && strchr(p,':')) {
		p++; // skip );
		while (*p!=':') p++;
		p++; // skip :
		if (*p=='(') p++;
		else return FALSE;
	}
	return TRUE;
}


/* getFunc()
 * math functions calculator
 * searched first for procedures, then for default math functions, and
 * last for numeric variables.
 * The address of starting position into the flow stream is pointed by p 
 * Ref. AA-O196C-TK - 5.1.2 "Special Functions"
 * Ref. AA-O196C-TK - 6.3 "MULTIPLE ASSIGNMENTS"
 * Ref. AA-O196C-TK - 14 "CONDITIONAL EXPRESSIONS AND STATEMENTS"
 * Ref. AA-O196C-TK - 17.1 "MATHEMATICAL PROCEDURES" */
double getFunc(void) {
	double o=0.0;
	int paren=TRUE, ns=0, type=0;
	char strres[COMPSTR], *op;
	int ERRVAL=-1, isFunc=TRUE;

	/* check if it's a fictitious variable */
	if ((ns=findDEC(p)) && *(p+_len)!='(') {
		if (vn[ns].isFict) isFunc=FALSE;
	}

        /* NUMERIC PROCEDURES 
	 * Ref. AA-O196C-TK - 11 "PROCEDURES" */
	if (isFunc && (ns=findPROC(p,&op,&type))) {
		int thistype=Procp[ns].type;
		if (type<STRINGTYPEBOX)	{
			/* normal user procedure  - delegate execPROC() */
			if (Procp[ns].procL>0) {
				p=execPROC(ns,op,&o,strres);
			}
			/* argument procedure - use S() */
			else if (Procp[ns].procL<0) {
				char exstring[COMPSTR], *e;
				char *pb=p+strlen(Procp[ns].name);
				e=exstring;
				strncpy_(exstring,Procp[ns].strdata,COMPSTR);
				e+=strlen(Procp[ns].strdata);
				if (*pb=='(') {
					int beg=0;
					*e++=*pb++; // copy (
					while (*pb) {
						if (*pb=='(') beg++;
						else if (*pb==')' && beg) 
							beg--;
						else if (*pb==')' && !beg) 
							break;
						*e++=*pb++;
					}
					if (*pb==')') *e++=*pb++;
					else pe(2,l);
					*e='\0';
				}
				p=exstring;
				o=S();
				p=pb;
				paren=FALSE;
			}
			else pe(31,l);
			paren=FALSE;
			p--;
		}
		else pe(52,l);
		resultType=0;
		setResultType(thistype);
	}
	/* NUMERIC VARIABLES 
	 * (excluding those who are functions with the same name) 
	 * Ref. AA-O196C-TK - 3 "IDENTIFIERS AND DECLARATION" */
	else if ((ns=findDEC(p)) && *(p+_len)!='(') {
		char *pb=p;
		p+=_len;
		/* variables to which a function or expression is passed. In 
		 * this case the function is recalculated using valdata */
		if (vn[ns].isFunc || vn[ns].refcode==2*DECADD) {
			char *pb=p;
			p=vn[ns].valdata;
                        o=S();
			resultType=0;
			setResultType(vn[ns].type);
			paren=FALSE;
			p=pb;
			p--;
			return o;
		}
		/* variable reference to an array element
		 * Ref. AA-O196C-TK - 11.5.1 "Jensen's Device" 
		 * or an entire expression that must be calculated 
		 * each time using valdata */
		else if (vn[ns].refcode<-DECADD) {
			char *pb=p;
			p=vn[ns].valdata;
                        o=S();
			resultType=0;
			setResultType(vn[ns].type);
			paren=FALSE;
			p=pb;
			p--;
			return o;
		}
		/* variable reference to a numeric variable or array;
		 * in this case, since the variable is passed by reference,
		 * we alter current ns to match the referenced variable */
		else if (vn[ns].refcode<0) {
			ns=vn[ns].ref;
		}

		/* byte subscripted string 
		 * Ref. AA-O196C-TK - 13.4 "BYTE SUBSCRIPTING" */
		if (_type==CHAR) {
			int pos=0;
			p++; // skip dot
			if (*p!='[') pe(2,l);
			else p++; // skip [
			pos=(int)S();
			if (*p!=']') pe(2,l);
			else p++; // skip ]
			if (pos<1 || pos>vn[vn[ns].ref].strmem) pe(57,l);
			else o=PS[vn[ns].ref][pos-1];
			paren=FALSE;
			p--;
			resultType=0;
			setResultType(INTEGERTYPEBOX);
		}
		/* string relation 
		 * Ref. AA-O196C-TK - 13.6 "STRING COMPARISONS" */
		else if (_type==STRINGTYPEBOX) {
			p=pb;
			o=strRel();
			paren=FALSE;
			resultType=0;
			setResultType(BOOLEANTYPEBOX);
		}
		/* return a variable content (not an array) 
		 * Ref. AA-O196C-TK - 4 "CONSTANTS" */
		else if (*(p)!='[' && !_arr) {
			resultType=0;
			if (vn[vn[ns].ref].type==INTEGERTYPEBOX) {
				o=(int)P[vn[ns].ref];
				setResultType(INTEGERTYPEBOX);
			}
			else if (vn[vn[ns].ref].type==BOOLEANTYPEBOX) {
				o=(int)P[vn[ns].ref]; 
				setResultType(BOOLEANTYPEBOX);
			}
			else if (vn[vn[ns].ref].type==LONGREALTYPEBOX) { 
				o=P[vn[ns].ref]; 
				setResultType(LONGREALTYPEBOX);
			}
			else {
				o=P[vn[ns].ref];
				setResultType(REALTYPEBOX);
			}
			p--;
			paren=FALSE;
		}
		/* return an array element 
		 * Ref. AA-O196C-TK - 9 "ARRAYS" */
		else if (*(p)=='[' && _arr) {
			int val[MAXDIM+1];
			int T=1;
			p++; // skip [
			val[T]=(int)S();
			while (*p==',') {
				p++; T++;
				if (T>MAXDIM) pe(36,l);
				val[T]=(int)S();
			}
			/* check types */
			resultType=0;
			if (vn[vn[ns].ref].type==INTEGERTYPEBOX) {
				o=(int)getArrayVal(vn[ns].ref,vn[vn[ns].ref].dim,val);
				setResultType(INTEGERTYPEBOX);
			}
			else if (vn[vn[ns].ref].type==BOOLEANTYPEBOX) {
				o=(int)getArrayVal(vn[ns].ref,vn[vn[ns].ref].dim,val);
				setResultType(BOOLEANTYPEBOX);
			}
			else if (vn[vn[ns].ref].type==LONGREALTYPEBOX) {
			        o=getArrayVal(vn[ns].ref,vn[vn[ns].ref].dim,val);
				setResultType(LONGREALTYPEBOX);
			}
			else {
				o=getArrayVal(vn[ns].ref,vn[vn[ns].ref].dim,val); 
				setResultType(REALTYPEBOX);
			}
			if (*p==']') p++;
			else pe(2,l);
			paren=FALSE;
			p--;
		}
	}
	/* MATH FUNCTIONS AND CONSTANTS */ 
	else switch(tolower(*p)) {
		case 'a':
			/* ABS(x) absolute value function
			 * Domain: x -> R 
			 * Range:  y -> R+ : y >= 0 
			 * Ref. AA-O196C-TK - "5.1.2 "Special Functions"  
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Simple Functions"*/
			if (!strncasecmp_(p,"ABS(",4)) {
				p+=4;
				o=S();
				o=o>=0.0?o:-o;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCCOS(x) arc cosine function 
			 * Domain: x -> R : -1 <= x <= 1 
			 * Range:  y -> R : 0 <= y < pi (angular measure) 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"ARCCOS(",7)) {
				p+=7;
				o=S();
				/* domain error */
				if (o<-1.0 || o>1.0) pe(51,l);
				o=acos(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCCOSH(x) hyperbolic arc cosine function 
			 * Domain: x -> R : x >= 1
			 * Range:  y -> R : y >= 0 */
			else if (!strncasecmp_(p,"ARCCOSH(",8)) {
				p+=8;
				o=S();
				/* domain error */
				if (o<1.0) pe(51,l);
				o=log(o+sqrt(o*o-1.0));
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCCOSEC(x) arc cosecant function 
			 * Domain: x -> R - { ]-1,1[ }
			 * Range:  y -> R : -pi/2 < y < pi/2 (angular measure)*/
			else if (!strncasecmp_(p,"ARCCOSEC(",9)) {
				p+=9;
				o=S();
				/* domain error */
				if (o>-1 && o<1) pe(51,l);
				o=asin(1.0/o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCCOSECH(x) hyperbolic arc cosecant function
			 * Domain: x -> R - { 0 }
			 * Range:  y -> R - { 0 }
			 * Note: case of x=0 is treated */
			else if (!strncasecmp_(p,"ARCCOSECH(",10)) {
				p+=10;
				o=S();
				if (fabsl(o)<ZEROCALC) o=(o<0.0?-INFF:INFF);
				else {
					o=1.0/o;
					o=log(o+sqrt(1.0+o*o));
				}
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCCOTAN(x) arc cotangent function 
			 * Domain: x -> R 
			 * Range:  y -> R : 0 < y < pi (angular measure) */
			else if (!strncasecmp_(p,"ARCCOTAN(",9)) {
				p+=9;
				o=S();
				if (fabsl(o)<ZEROCALC) o=o<0?-HALFPI:HALFPI;
				else o=atan(1.0/o);
				if (o<0) o+=TAXIPI;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCCOTANH(x) hyperbolic arc cotangent function
			 * Domain: x -> R - { ]-1,1[ } 
			 * Range:  y -> R - { 0 } */
			else if (!strncasecmp_(p,"ARCCOTANH(",10)) {
				p+=10;
				o=S();
				/* domain error */
				if (o>-1.0 && o<1.0) pe(51,l);
				else if (o==-1.0) o=-INFF;
				else if (o==1.0) o=INFF;
				else o=0.5*log((o+1.0)/(o-1.0));
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCSEC(x) arc secant function 
			 * Domain: x -> R - { ]-1,1[ }
			 * Range:  y -> R : 0 <= y < pi (angular measure) */
			else if (!strncasecmp_(p,"ARCSEC(",7)) {
				p+=7;
				o=S();
				/* domain error */
				if (o>-1.0 && o<1.0) pe(51,l);
				o=acos(1.0/o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCSECH(x) hyperbolic arc secant function 
			 * Domain: x -> R : 0 < x <= 1
			 * Range:  y -> R : y >= 0 
			 * Note: the cases x=0 and x=1 are treated */
			else if (!strncasecmp_(p,"ARCSECH(",8)) {
				p+=8;
				o=S();
				/* domain error */
				if (o<0.0 || o>1.0) pe(51,l);
				else if (fabsl(o)<ZERO) o=INFF; // o>0 here!
				else if (o==1.0) o=0.0;
				else {
					o=1.0/o;
					o=log(o+sqrt(o*o-1.0));
				}
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCSIN(x) arc sin function 
			 * Domain: x -> R : -1 <= x <=1 
			 * Range:  y -> R : -pi/2 < y < pi/2 (angular measure) 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"ARCSIN(",7)) {
				p+=7;
				o=S();
				/* domain error */
				if (o<-1.0 || o>1.0) pe(51,l);
				o=asin(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCSINH(x) hyperbolic arc sin function 
			 * Domain: x -> R 
			 * Range:  y -> R */
			else if (!strncasecmp_(p,"ARCSINH(",8)) {
				p+=8;
				o=S();
				o=log(o+sqrt(1+o*o));
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCTAN(y) arc tangent function
			 * Domain: x -> R
			 * Range:  y -> R : -pi/2 < y < pi/2 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures"   
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Mathematical Functions" */
			else if (!strncasecmp_(p,"ARCTAN(",7)) {
				p+=7;
				o=S();
				o=atan(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ARCTANH(x) hyperbolic arc tangent function 
			 * Domain: x -> R : -1 <= x <= 1
			 * Range:  y -> R 
			 * Note the cases x=1 and x=-1 are treated */
			else if (!strncasecmp_(p,"ARCTANH(",8)) {
				p+=8;
				o=S();
				/* domain error */
				if (o<-1.0 || o>1.0) pe(51,l);
				else if (o==-1.0) o=-INFF;
				else if (o==1.0) o=INFF;
				else o=0.5*log((1.0+o)/(1.0-o));
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'b':
			/* BOOL(x) integer function
			 * ~convert an integer to boolean type
			 * Domain: x -> N
			 * Range:  y -> B  
			 * Ref. AA-O196C-TK - 5.3 "INTEGER AND BOOLEAN 
			 * CONVERSIONS" */
			if (!strncasecmp_(p,"BOOL(",5)) {
				p+=5;
				o=(int)S();
				resultType=0;
				setResultType(BOOLEANTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'c':
			/* CLOCK(x) timing function
			 * return the timing since program start in 
			 * milliseconds */
			if (!strncasecmp_(p,"CLOCK(",6)) {
				int N,nowM;
				p+=6;
				N=(int)S();
		        	gettimeofday(&tv, NULL);
		        	nowM = (int)(tv.tv_sec *1000 + tv.tv_usec / 1000);
				o=(int)(nowM-N-beginM);
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* COS(x) cosine function 
			 * Domain: x -> R (angular measure) 
			 * Range:  y -> R : -1 <= y <= 1 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures"  
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Mathematical Functions" */
			else if (!strncasecmp_(p,"COS(",4)) {
				p+=4;
				o=S();
				o=cos(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* COSH(x) hyperbolic cosine function 
			 * Domain: x -> R
			 * Range:  y -> R : y >= 1 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"COSH(",5)) {
				p+=5;
				o=S();
				o=(exp(-o)+exp(o))/2.0;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* COSEC(x) cosecant function 
			 * Domain: x -> R - { pi/2+kpi } (angular measure) 
			 * Range:  y -> R - { ]-1,1[ } 
			 * Note: the case x=pi/2+kpi is treated after having 
			 * calculated the sine value, which must not be null */
			else if (!strncasecmp_(p,"COSEC(",6)) {
				p+=6;
				o=S();
				o=sin(o);
				/* Note: I check here against ZEROCALC=2E-13 
				 * and not ZERO because the rendition of sin(pi)
				 * is never really zero, since the argument is 
				 * never really pi; so the absolute value of
				 * sin(pi) is never lesser than ZERO, and the 
				 * least value it can reach is around 2E-13 */
				if (fabsl(o)<ZEROCALC) o=o<0.0?-INFF:INFF;
				else o=1.0/o;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* COSECH(x) hyperbolic cosecant function
			 * Domain: x -> R - {0}
			 * Range:  y -> R - {0} 
			 * Note: the case x=0 is treated */
			else if (!strncasecmp_(p,"COSECH(",7)) {
				p+=7;
				o=S();
			       	if (fabsl(o)<ZEROCALC) o=o<0.0?-INFF:INFF;
				else o=2.0/(exp(o)-exp(-o));
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* COTAN(x) cotangent function 
			 * Domain: x -> R - { kpi } (angular measure) 
			 * Range:  y -> R 
			 * Note: the case x=kpi is treated */
			else if (!strncasecmp_(p,"COTAN(",6)) {
				p+=6;
				o=S();
				/* set value to the range 0-pi */
				if (fabsl(o)<ZERO) o=INFF;
				else o=1.0/tan(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* COTANH(x) hyperbolic cotangent function
			 * Domain: x -> R - { 0 }
			 * Range:  y -> R - { ]-1,1[ } 
			 * Note: the case x=0 is treated */
			else if (!strncasecmp_(p,"COTANH(",7)) {
				p+=7;
				o=S();
				if (fabsl(o)<ZEROCALC) o=o<0.0?-INFF:INFF;
				else o=(exp(o)+exp(-o))/(exp(o)-exp(-o));
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'd':
			/* DEG(X) - conversion to degrees */
			if (!strncasecmp_(p,"DEGREES(",8)) {
				p+=8;
				o=S();
				o=o*180.0/TAXIPI;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* DIM(x) array dimension 
			 * Domain: x: array reference (name only)
			 * Range:  y -> N : y >= 1 */
			else if (!strncasecmp_(p,"DIM(",4)) {
				p+=4;
				o=getBound(MAXDIM);
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			} 
			else ERRVAL=0;
			break;
		case 'e':
			/* ENTIER(x) integer value
			 * return the largest integer value not exceeding the 
			 * argument
			 * Domain: x -> R 
			 * Range:  y -> R : y >= 0 
			 * Ref. AA-O196C-TK - "5.1.2 "Special Functions"  
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Simple Functions"*/
			if (!strncasecmp_(p,"ENTIER(",7)) {
				int j;
				p+=7;
				o=S();
				j=(int)o;
				if (j>o) o=(double)j-1.0;
				else o=(double)j;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* EOF(x) Boolean value
			 * return TRUE if channel x has reached EOF, FALSE 
			 * otherwise 
			 * Domain: x -> I : 1 <= x <= 15 
			 * Range:  y -> I : y =-1 or y = 0 
			 * Note: neither in the DEC ALGOL nor in the ISO 
			 * 1538-1984 (E) */
			else if (!strncasecmp_(p,"EOF(",4)) {
				int ch;
				p+=4;
				ch=(int)S();
				if (isCDC && ch>=60) ch-=60;
				o=isEOF(ch);
				resultType=0;
				setResultType(BOOLEANTYPEBOX);
			}
			/* EPSILON constant
			 * ~return the smallest real value 
			 * Note: neither in the DEC ALGOL nor in the ISO 
			 * 1538-1984 (E) */
			else if (!strncasecmp_(p,"EPSILON",7) && !isnamechar((p+7))) {
				o=EPSILON;
				p+=6;
				paren=FALSE;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ERF(X) function
			 * ~return the Gauss error function of X.
			 * The computed function of erf(x) is:
			 * 2/sqrt(pi) * integral [from 0 to x] of exp(-(t*t))dt
			 * The returned values (from the man page of erf):
			 * On  success, these functions return the error function of x, 
			 * a value in the range [-1, 1].
			 * If x is a NaN, a NaN is returned.
			 * If x is +0 (-0), +0 (-0) is returned.
			 * If x is positive infinity (negative infinity), 
			 * +1 (-1) is returned.
			 * If x is subnormal, a range error occurs, and the 
			 * return value is 2*x/sqrt(pi).	
			 * Domain: x -> R 
			 * Range:  y -> R : -1 < y < 1
			 * from Wikipedia:
			 * This integral is a special (non-elementary) and sigmoid function 
			 * that occurs often in probability, statistics, and partial differential 
			 * equations describing diffusion. In statistics, for nonnegative values 
			 * of x, the error function has the following interpretation: 
			 * for a random variable Y that is normally distributed with mean 0 and 
			 * variance 1/2, erf x is the probability that Y falls in the range [-x,x].
			 * The complementary error function, erfc, is defined as 
			 * erfc(x) = 1 - erf(x)
			 * Note: neither in the DEC ALGOL nor in the ISO 
			 * 1538-1984 (E) */
			else if (!strncasecmp_(p,"ERF(",4)) {
				p+=4;
				o=S();
				o=ERFL(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* ERRC constant
			 * ~return the error code of the error occurred; 
			 * Note: for all procedures that accept a label in 
			 * case of error */
			else if (!strncasecmp_(p,"ERRC",4) && !isnamechar((p+5))) {
				o=errCODE;
				p+=3;
				paren=FALSE;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* ERRL constant
			 * ~return the number of line in which the error 
			 * occurred; 
			 * Note: for all procedures that accept a label in case
			 * of error */
			else if (!strncasecmp_(p,"ERRL",4) && !isnamechar((p+5))) {
				o=errLINE;
				p+=3;
				paren=FALSE;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* EXP(x) exponential function (base e)
			 * Domain: x -> R 
			 * Range:  y -> R : y >= 0 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures"   
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Mathematical Functions" */
			else if (!strncasecmp_(p,"EXP(",4)) {
				p+=4;
				o=S();
				if (o>709.726836893) pe(50,l);
				o=exp(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* EXPR(x,n) exponentiation (ISO)
			 * Domain: x,n -> R 
			 * Range:  y -> R */
			else if (!strncasecmp_(p,"EXPR(",5)) {
				double x,n;
				p+=5;
				x=S();
				if (*p==',') p++;
				else if (skip());
				else pe(2,l);
				n=S();
				o=exp_(REALTYPEBOX,x,REALTYPEBOX,n);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* EXPI(x,n) exponentiation (ISO)
			 * Domain: x,n -> N 
			 * Range:  y -> N */
			else if (!strncasecmp_(p,"EXPI(",5)) {
				int x,n;
				p+=5;
				x=(int)S();
				if (*p==',') p++;
				else if (skip());
				else pe(2,l);
				n=(int)S();
				o=exp_(INTEGERTYPEBOX,x,INTEGERTYPEBOX,n);
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* EXPN(x,n) exponentiation (ISO)
			 * Domain: x -> R 
			 * Domain: n -> N
			 * Range:  y -> R */
			else if (!strncasecmp_(p,"EXPN(",5)) {
				double x;
				int n;
				p+=5;
				x=S();
				if (*p==',') p++;
				else if (skip());
				else pe(2,l);
				n=(int)S();
				o=exp_(REALTYPEBOX,x,INTEGERTYPEBOX,n);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'f':
			/* FALSE constant
			 * ~return the false condition 0 
			 * Ref. AA-O196C-TK - 4.2 "OCTAL AND BOOLEAN 
			 * CONSTANTS" */
			if (!strncasecmp_(p,"FALSE",5) && !isnamechar((p+5))) {
				o=FALSE;
				p+=4;
				paren=FALSE;
				resultType=0;
				setResultType(BOOLEANTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'g':
			/* GAMMA(x) function 
			 * ~return the gamma function value of x 
			 * (from Wikipedia:)
			 * In mathematics, the gamma function (represented 
			 * by the capital Greek L) is commonly used as an 
			 * extension of the factorial function. 
			 * The gamma function is defined for all complex 
			 * numbers except the non-positive integers. 
			 * For any positive integer:
			 * Domain: x -> R : integer x > 0
			 * Range:  y -> R ; y <> 0
			 * Gamma(n) = (n-1)!
			 * Note: neither in the DEC ALGOL nor in the ISO 
			 * 1538-1984 (E) */
			if (!strncasecmp_(p,"GAMMA(",6)) {
				p+=6;
				o=S();
				o=TGAMMAL(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* GFIELD constant
			 * ~return a sub-set of an integer as integer 
			 * Ref. AA-O196C-TK - 17.3.3 "Field Manipulation" */
			else if (!strncasecmp_(p,"GFIELD(",7)) {
				int A,start,length;
				int k=0,res=0;
				p+=7;
				A=(int)S();
				if (*p==',') p++;
				else if (skip());
				else pe(2,l);
				start=(int)S();
				if (start<0) start=0;
				if (start>31) start=31;
				if (*p==',') p++;
				else if (skip());
				else pe(2,l);
				length=(int)S();
				if (length<0) length=0;
				if (length+start>31) length=31-start;
				k=0; while (k<start) A/=2, k++;
				k=0; while (k<length) 
					res+=(A&1)*pow(2,k), A/=2, k++;
				o=res;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'i':
			/* IABS(x) absolute value function
			 * Domain: x -> I
			 * Range:  y -> I+ : y >= 0 
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Simple Functions" */
			if (!strncasecmp_(p,"IABS(",5)) {
				p+=5;
				o=(int)S();
				o=o>=0?o:-o;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* IF..THEN..ELSE
			 * ~conditional expression
			 * Ref. AA-O196C-TK - 14 "CONDITIONAL EXPRESSIONS AND 
			 * STATEMENTS" */
			else if (!strncasecmp_(p,"IF",2)) {
				int cond=FALSE;
				int res1,res2;
				double o1=0,o2=0;
				p+=2;
				if (!check(p,"THEN") && !check(p," HEN")) 
					pe(2,l);
				cond=(int)S();
				*p='T';
				p+=4; // skip THEN
				/* reload */
				o1=S(); res1=resultType;
				if (!*p) {
					l++;
					while (!m[l]) l++;
					if (l>endCore) pe(23,l);
					strncpy_(BASESTR,m[l],COMPSTR);
					p=BASESTR;
				}
				if (!check(p,"ELSE") && !check(p," LSE")) 
					pe(2,l);
				*p='E';
				p+=4; // skip ELSE
				o2=S(); res2=resultType;
				/* reload */
				if (!*p) {
					l++;
					while (!m[l]) l++;
					if (l>endCore) pe(23,l);
					strncpy_(BASESTR,m[l],COMPSTR);
					p=BASESTR;
				}
				if (cond) {
					o=o1;
					setResultType(res1);
				}
				else {
					o=o2;
					setResultType(res2);
				}
				p--;
				paren=FALSE;
			}
			/* IMAX(x,y,...) integer maxima function
			 * ~return the greatest value in the integer values 
			 * list
			 * Domain: x1,x2,...,xn -> I
			 * Range:  y -> N  
			 * Ref. AA-O196C-TK - 17.2.1 "Minima and Maxima 
			 * Procedures" */
			else if (!strncasecmp_(p,"IMAX(",5)) {
				int temp;
				p+=5;
				o=temp=(int)S();
				while (skip() || *p==',') {
					if (*p==',') p++; // skip ,
					if (o<temp) o=temp;
					temp=(int)S();
				}
				if (o<temp) o=temp; // last case
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* IMIN(x,y,...) integer minima function
			 * ~return the lesser value in the integer values list
			 * Domain: x1,x2,...,xn -> I
			 * Range:  y -> N  
			 * Ref. AA-O196C-TK - 17.2.1 "Minima and Maxima 
			 * Procedures" */
			else if (!strncasecmp_(p,"IMIN(",5)) {
				int temp;
				p+=5;
				o=temp=(int)S();
				while (skip() || *p==',') {
					if (*p==',') p++; // skip ,
					if (o>temp) o=temp;
					temp=(int)S();
				}
				if (o>temp) o=temp; // last case
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* INFO(x) integer function
			 * ~return specific system values
			 * Domain: x -> N 
			 * Range:  y -> N  */
			else if (!strncasecmp_(p,"INFO(",5)) {
				p+=5;
				int arg=(int)S();
				/* core size */
				if (!arg) {
					int i;
					o=0;
					for (i=0;i<=endCore;i++) if (m[i]) o+=strlen(m[i]);
					resultType=0;
					setResultType(INTEGERTYPEBOX);
				}
				/* date */
				else if (arg==1) {
					o=getYear()*10000;
					o+=getMonth()*100;
					o+=getDay();
					resultType=0;
					setResultType(INTEGERTYPEBOX);
				}
				/* time (ticks since midnight) */
				else if (arg==2) {
			        	int val;
					val=getHour();
					o=val*3600000.0;
					val=getMin();
					o+=val*60000.0;
					val=getSec();
					o+=val*1000.0;
			        	gettimeofday(&tv, NULL);
		        	   	o+= tv.tv_usec/1000.0;
					o/=1000;
					o*=CLOCKS_PER_SEC;
					resultType=0;
					setResultType(INTEGERTYPEBOX);
				}
				/* time (milliseconds since midnight) */
				else if (arg==3) {
			        	int val;
					val=getHour();
					o=val*3600000.0;
					val=getMin();
					o+=val*60000.0;
					val=getSec();
					o+=val*1000.0;
			        	gettimeofday(&tv, NULL);
		        	   	o+= tv.tv_usec/1000.0;
					resultType=0;
					setResultType(INTEGERTYPEBOX);
				}
				/* run time in milliseconds*/
				else if (arg==4) {
					double endS;
			        	gettimeofday(&tv, NULL);
			           	endS = ((double)tv.tv_sec + tv.tv_usec / 1000000.0);
					o=(endS-beginS)*1000.0;
					resultType=0;
					setResultType(INTEGERTYPEBOX);
				}
				/* interpreter version */
				else if (arg==7) {
					char *f=INFOVERSION;
					o=atoi(f)*100000;
					while (*f!='.') f++; 
					f++;
					o+=atoi(f)*1000;
					while (*f!='.') f++; 
					f++;
					o+=atoi(f);
					resultType=0;
					setResultType(INTEGERTYPEBOX);
				}
				/* other values */
				else {
					o=0;
					resultType=0;
					setResultType(INTEGERTYPEBOX);
				}
			}
			/* INTEGRAL(f,a,b,x,epsilon) Integral function */
			else if (!strncasecmp_(p,"INTEGRAL(",9)) {
				double start, end;
				int ns,k;
				double n=100000, o2;
				char var[COMPSTR], *v=var;
				char expr[COMPSTR];
				char *pb;
				/* acquire data */
				p+=9;
				strncpy_(expr,SS(),COMPSTR); // find expression
				if (*p==',') p++;
				else if (skip());
				else pe(2,l);
				start=S(); // find start
				if (*p==',') p++;
				else if (skip());
				else pe(2,l);
				end=S();   // find end
				if (*p==',') p++;
				else if (skip());
				else pe(2,l);
				while (isnamechar(p)) *v++=*p++;
				*v='\0';
				ns=findDEC(var); // find integration var
				while (vn[ns].refcode!=ns) ns=-vn[ns].refcode;
				if (!ns) pe(89,l);
				if (skip() ||*p==',') {
					int exp;
					if (*p==',') p++; // skip ,
					exp=(int)S();  // find epsilon
					if (exp<0) exp=-exp;
					n=pow(10.0L,exp);
				}
				pb=p;
				o2=0.0L;
				/* integrate */
				P[ns]=start;
				p=expr;
				o2=S()/2.0;
				for (k=1;k<=n;k++) {
					P[ns]=start+k*(end-start)/n;
					p=expr;
					o2+=S();
				}
				P[ns]=end;
				p=expr;
				o2+=S()/2.0;
				o2*=(end-start)/n;
				o=o2;
				resultType=0;
				setResultType(REALTYPEBOX);
				p=pb;
			}
			/* IOCHAN(x) integer function
			 * ~return the channel status
			 * Domain: x -> N 
			 * Range:  y -> N */
			else if (!strncasecmp_(p,"IOCHAN(",7)) {
				int ch;
				p+=7;
				ch=(int)S(); 
				if (isCDC && ch>=60) ch-=60;
				o=channel[ch].iochan;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* INCHAN integer function
			 * ~return the current input channel number
			 * Domain: x -> N 
			 * Range:  y -> N */
			else if (!strncasecmp_(p,"INCHAN",6) && !isnamechar((p+6))) {
				p+=5;
				o=currentINPUT;
				paren=FALSE;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* INFINITY constant
			 * ~return the number beyond the maximum allowable 
			 * positive real value
			 * Note: neither in the DEC ALGOL nor in the ISO 
			 * 1538-1984 (E) */
			else if (!strncasecmp_(p,"INFINITY",8) && !isnamechar((p+8))) {
				o=1.L/0.L;
				p+=7;
				paren=FALSE;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* INF constant
			 * ~return the maximum computable positive real value
			 * Note: neither in the DEC ALGOL nor in the ISO 
			 * 1538-1984 (E) */
			else if (!strncasecmp_(p,"INF",3) && !isnamechar((p+3))) {
				o=INFF;
				p+=2;
				paren=FALSE;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* INT(x) integer function
			 * ~convert a Boolean to integer
			 * Domain: x -> R 
			 * Range:  y -> N  
			 * Ref. AA-O196C-TK - 5.3 "INTEGER AND BOOLEAN 
			 * CONVERSIONS" */
			else if (!strncasecmp_(p,"INT(",4)) {
				p+=4;
				o=(int)S();
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'l':
			/* LABS(x) absolute value function
			 * Domain: x -> R
			 * Range:  y -> R+ : y >= 0 
			 * Neither DEC nor ISO */
			if (!strncasecmp_(p,"LABS(",5)) {
				p+=5;
				o=(int)S();
				o=o>=0?o:-o;
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LONGEPSILON constant
			 * ~return the smallest long real value 
			 * Note: neither in the DEC ALGOL nor in the ISO 
			 * 1538-1984 (E) */
			else if (!strncasecmp_(p,"LONGEPSILON",11) && !isnamechar((p+11))) {
				o=LONGEPSILON;
				p+=10;
				paren=FALSE;
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LARCTAN(y) arc tangent function
			 * Domain:       x -> R
			 * Range: ATAN:  y -> R : -pi/2 < y < pi/2 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"LARCTAN(",8)) {
				p+=8;
				o=S();
				o=atan(o);
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LCOS(x) cosine function 
			 * Domain: x -> R (angular measure) 
			 * Range:  y -> R : -1 <= y <= 1 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"LCOS(",5)) {
				p+=5;
				o=S();
				o=cos(o);
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LENGTH(x) string length function 
			 * ~return length of the argument string */
			else if (!strncasecmp_(p,"LENGTH(",7)) {
				char lop[COMPSTR];
				p+=7;
				strncpy_(lop,SS(),COMPSTR);
				o=(double)strlen(lop);
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* LEXP(x) exponential function (base e)
			 * Domain: x -> R 
			 * Range:  y -> R : y >= 0 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"LEXP(",5)) {
				p+=5;
				o=S();
				if (o>709.726836893) pe(50,l);
				o=exp(o);
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LLN(x) natural logarithm function
			 * ~return the natural logarithm
			 * Domain: x -> R : x > 0 
			 * Range:  y -> R 
			 * Note: the case x=0 is treated 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"LLN(",4)) {
				p+=4;
				o=S();
				/* domain errors (negative and equal to zero) */
				if (o<=0) pe(49,l); 
				else o=log(o);
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LMAX(x,y,...) long real maxima function
			 * ~return the greatest value in the long real values 
			 * list
			 * Domain: x1,x2,...,xn -> R
			 * Range:  y -> R  
			 * Ref. AA-O196C-TK - 17.2.1 "Minima and Maxima 
			 * Procedures" */
			else if (!strncasecmp_(p,"LMAX(",5)) {
				double temp;
				p+=5;
				o=temp=S();
				while (skip() || *p==',') {
					if (*p==',') p++; // skip ,
					if (o<temp) o=temp;
					temp=S();
				}
				if (o<temp) o=temp; // last case
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LMIN(x,y,...) long real minima function
			 * ~return the lesser value in the long real values 
			 * list
			 * Domain: x1,x2,...,xn -> R
			 * Range:  y -> R  
			 * Ref. AA-O196C-TK - 17.2.1 "Minima and Maxima 
			 * Procedures" */
			else if (!strncasecmp_(p,"LMIN(",5)) {
				double temp;
				p+=5;
				o=temp=S();
				while (skip() || *p==',') {
					if (*p==',') p++; // skip ,
					if (o>temp) o=temp;
					temp=S();
				}
				if (o>temp) o=temp; // last case
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LN(x) natural logarithm function
			 * ~return the natural logarithm
			 * Domain: x -> R : x > 0 
			 * Range:  y -> R 
			 * Note: the case x=0 is treated 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures"   
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Mathematical Functions" */
			else if (!strncasecmp_(p,"LN(",3)) {
				p+=3;
				o=S();
				/* domain errors (negative and equal to zero) */
				if (o<0 || o<ZERO) { // o>0 here!
					pe(49,l);
					o=-REALMAX; 
				}
				else o=log(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* LSIN(x) sin function 
			 * Domain: x -> R (angular measure) 
			 * Range:  y -> R : -1 <= y <= 1 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"LSIN(",5)) {
				p+=5;
				o=S();
				o=sin(o);
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LSQRT(x) square root function 
			 * Domain: x -> R : x >= 0 
			 * Range:  y -> R : y >= 0 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */ 
			else if (!strncasecmp_(p,"LSQRT(",6)) {
				p+=6;
				o=S();
				/* domain error */
				if (o<0.0) pe(48,l); 
				else o=sqrt(o);
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* LB() function
			 * ~return the lowest index */
			else if (!strncasecmp_(p,"LB(",3)) {
				p+=3;
				o=getBound(FALSE);
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'm':
			/* MAXINT constant
			 * ~return the maximum allowable positive integer 
			 * value */
			if (!strncasecmp_(p,"MAXINT",6) && !isnamechar((p+6))){
				o=INTMAX;
				p+=5;
				paren=FALSE;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* MININT constant
			 * ~return the maximum allowable negative integer 
			 * value */
			else if (!strncasecmp_(p,"MININT",6) && !isnamechar((p+6))) {
				o=INTMIN;
				p+=5;
				paren=FALSE;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* MAXREAL constant
			 * ~return the maximum figurative positive real value */
			else if (!strncasecmp_(p,"MAXREAL",7) && !isnamechar((p+7))) {
				o=REALMAX;
				p+=6;
				paren=FALSE;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* MINREAL constant
			 * ~return the minimum figurative positive real value */
			else if (!strncasecmp_(p,"MINREAL",7) && !isnamechar((p+7))) {
				o=REALMIN;
				p+=6;
				paren=FALSE;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* MAXLONGREAL constant
			 * ~return the maximum figurative positive real value */
			else if (!strncasecmp_(p,"MAXLONGREAL",11) && !isnamechar((p+11))) {
				o=LONGREALMAX;
				p+=10;
				paren=FALSE;
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* MINLONGREAL constant
			 * ~return the minimum figurative positive real value */
			else if (!strncasecmp_(p,"MINLONGREAL",11) && !isnamechar((p+11))) {
				o=LONGREALMIN;
				p+=10;
				paren=FALSE;
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'o':
			/* OUTCHAN integer function
			 * ~return the current output channel number
			 * Domain: x -> N 
			 * Range:  y -> N */
			if (!strncasecmp_(p,"OUTCHAN",7) && !isnamechar((p+7))){
				p+=6;
				o=currentOUTPUT;
				paren=FALSE;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'p':
			/* PI constant
			 * ~return the Greek Pi
			 * Note: neither in the DEC ALGOL nor in the ISO 
			 * 1538-1984 (E) */
			if (!strncasecmp_(p,"PI",2) && !isnamechar((p+2))) {
				o=TAXIPI;
				p++;
				paren=FALSE;
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'r':
			/* RADIANS(X) - conversion to radians */
			if (!strncasecmp_(p,"RADIANS(",8)) {
				p+=8;
				o=S();
				o=o*TAXIPI/180.0;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* RAND random number generator
			 * ~return the next random number
			 * Range:  y -> R */
			else if (!strncasecmp_(p,"RAND",4) && !isnamechar((p+2))) {
				p+=3;
				o=getRandom();
				saveRand=o;
				paren=FALSE;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* RMAX(x,y,...) real maxima function
			 * ~return the greatest value in the real values list
			 * Domain: x1,x2,...,xn -> R
			 * Range:  y -> R  
			 * Ref. AA-O196C-TK - 17.2.1 "Minima and Maxima 
			 * Procedures" */
			else if (!strncasecmp_(p,"RMAX(",5)) {
				double temp;
				p+=5;
				o=temp=S();
				while (skip() || *p==',') {
					if (*p==',') p++; // skip ,
					if (o<temp) o=temp;
					temp=S();
				}
				if (o<temp) o=temp; // last case
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* RMIN(x,y,...) real minima function
			 * ~return the lesser value in the real values list
			 * Domain: x1,x2,...,xn -> R
			 * Range:  y -> R  
			 * Ref. AA-O196C-TK - 17.2.1 "Minima and Maxima 
			 * Procedures" */
			else if (!strncasecmp_(p,"RMIN(",5)) {
				double temp;
				p+=5;
				o=temp=S();
				while (skip() || *p==',') {
					if (*p==',') p++; // skip ,
					if (o>temp) o=temp;
					temp=S();
				}
				if (o>temp) o=temp; // last case
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 's':
			/* SAVRAN return last random number 
			 * ~return the last random number
			 * Range:  y -> R */
			if (!strncasecmp_(p,"SAVRAN",6) && !isnamechar((p+2))){
				p+=5;
				o=saveRand;
				resultType=0;
				setResultType(REALTYPEBOX);
				paren=FALSE;
			}
			/* SEC(x) secant function
			 * Domain: x -> R - { kpi } (angular measure) 
			 * Range:  y -> R -{ ]-1,1[ } 
			 * Note: the case x=kpi is treated */
			if (!strncasecmp_(p,"SEC(",4)) {
				p+=4;
				o=S();
				o=cos(o);
				/* Note: I check here against ZEROCALC=2E-13 
				 * and not ZERO because the rendition of 
				 * cos(pi/2) is never really zero, 
				 * since the argument is never really pi/2 */
				if (fabsl(o)<ZEROCALC) o=o<0?-INFF:INFF;
				else o=1.0/o;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* SECH(x) hyperbolic secant function 
			 * Domain: x -> R 
			 * Range:  y -> R : 0 <= y <= 1 */
			else if (!strncasecmp_(p,"SECH(",5)) {
				p+=5;
				o=S();
				o=2.0/(exp(o)+exp(-o));
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* SIGN(x) sign function 
			 * Domain: x -> R
			 * Range   y -> N, : y = -1 or 0 or 1 
			 * Ref. AA-O196C-TK - "5.1.2 "Special Functions"  
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Simple Functions"*/
			else if (!strncasecmp_(p,"SIGN(",5)) {
				p+=5;
				o=sign(S()); 
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* SIN(x) sin function 
			 * Domain: x -> R (angular measure) 
			 * Range:  y -> R : -1 <= y <= 1 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures"  
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Mathematical Functions" */
			else if (!strncasecmp_(p,"SIN(",4)) {
				p+=4;
				o=S();
				o=sin(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* SINH(x) hyperbolic sin function 
			 * Domain: x -> R 
			 * Range:  y -> R 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"SINH(",5)) {
				p+=5;
				o=S();
				o=(-exp(-o)+exp(o))/2;
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* SIZE(x) string length function 
			 * ~return size of x string memory (unlike the original
			 * DEC-20 ALGOL instruction, which reported the unity 
			 * char of the argument string, at present useless 
			 * because 8-bits ASCII characters are used.)
			 * Note: neither in the DEC ALGOL (with this scope) 
			 * nor in the ISO 1538-1984 (E) */
			else if (!strncasecmp_(p,"SIZE(",5)) {
				int ns;
				p+=5;
				ns=findDEC(p);
				if (!ns || vn[ns].type!=STRINGTYPEBOX) pe(56,l);
				p+=_len;
				o=vn[ns].strmem-1;
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			/* SQRT(x) square root function 
			 * Domain: x -> R : x >= 0 
			 * Range:  y -> R : y >= 0 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures"  
			 * Ref. ISO 1538-1984 (E) "The environmental block: 
			 * Mathematical Functions" */ 
			else if (!strncasecmp_(p,"SQRT(",5)) {
				p+=5;
				o=S();
				/* domain error */
				if (o<0.0) pe(48,l); 
				else o=sqrt(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 't':
			/* TAN(x) tangent function 
			 * Domain: x -> R - { pi/2+kpi } (angular measure)
			 * Range:  y -> R 
			 * Note: the case x=pi/2+kpi is treated 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			if (!strncasecmp_(p,"TAN(",4)) {
				p+=4;
				o=S();
				o=tan(o);
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* TANH(x) hyperbolic tangent function
			 * Domain: x -> R
			 * Range:  y -> R : -1 < y < 1 
			 * Ref. AA-O196C-TK - 17.1 "Mathematical Procedures" */
			else if (!strncasecmp_(p,"TANH(",5)) {
				p+=5;
				o=S();
				o=(exp(o)-exp(-o))/(exp(o)+exp(-o));
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* TOLONG
			 * ~return the long real result of the argument */
			else if (!strncasecmp_(p,"TOLONG(",7)) {
				p+=7;
				o=S();
				resultType=0;
				setResultType(LONGREALTYPEBOX);
			}
			/* TOREAL
			 * ~return the real result of the argument */
			else if (!strncasecmp_(p,"TOREAL(",7)) {
				p+=7;
				o=S();
				resultType=0;
				setResultType(REALTYPEBOX);
			}
			/* TRUE constant
			 * ~return the true condition 
			 * Ref. AA-O196C-TK - 4.2 "OCTAL AND BOOLEAN 
			 * CONSTANTS" */
			else if (!strncasecmp_(p,"TRUE",4) && !isalnum(*(p+4))) {
				o=TRUE;
				p+=3;
				paren=FALSE;
				resultType=0;
				setResultType(BOOLEANTYPEBOX);
			}
			else ERRVAL=0;
			break;
		case 'u':
			/* UB() function
			 * ~return the highest index */
			if (!strncasecmp_(p,"UB(",3)) {
				p+=3;
				o=getBound(TRUE);
				resultType=0;
				setResultType(INTEGERTYPEBOX);
			}
			else ERRVAL=0;
			break;
		default:
			ERRVAL=0;
	}
	/* any other sequence of characters is an illegal function or
	 * illegal variable name */
	if (!ERRVAL) {
		resultType=0;
		printf("unrecognized function: %s\n",p);
		pe(81,l);
	}

	/* check closing parenthesis for functions requiring an argument;
	 * p is updated into getVal() */
	if (paren) {
		if (!*p) pe(2,l);
		else if (*p!=')') pe(2,l);
	}
	/* return the sweated value */  
	return o;
}


/* stripBlanks()
 * strip 'in site' blanks from str; stop stripping while into a string
 * Note: two consecutive literal strings are joined, removing the quotes
 * and the inner blanks. The quoting must be consistent.  */
void stripBlanks(char *str) {
	char *sp=str, *kp=str;
	static int isStr=0;

	while (*kp) {
		if (*kp=='\"' && *(kp+1)=='\"') *sp++=*kp++, *sp++=*kp++;
		else if (*kp=='\"' && !isStr) isStr=1;
		else if (*kp=='\"' && isStr==1) isStr=0;
		else if (isdc(kp) && !isStr) isStr=2;
		else if (isdc(kp) && isStr==2) isStr=0;
		
		if (isStr==1) {
			*sp++=*kp++;
			if (*(sp-1)=='\"' && *(sp-2)=='\"') sp-=2;
		}
		else if (isStr==2) {
			*sp++=*kp++;
			if (*(sp-1)=='\'' && *(sp-2)=='\'' &&\
				*(sp-3)=='\'' && *(sp-4)=='\'') 
				sp-=4;
		}
		else if (isblank(*kp)) while (isblank(*kp)) kp++;
		else {
			optoken(kp);
			*sp++=*kp++;
		}

	}
	while (kp>=sp) *kp--='\0';
}


/* optoken()
 * substitute per-character all found operators names with establishes symbols,
 * according to the following list:
 * ~PU for := and <- (assignment) - PU is @
 * ~= for EQL
 * ~# for <> NOTEQUAL != >< NEQ ( ~= is performed internally)
 * ~^ for **
 * ~_ for AND or /\
 * ~| for OR or \/
 * ~? for EQV == EQIV <->
 * ~! for IMP or -> IMPL >>
 * ~< for LSS
 * ~{ for <= LEQ =<
 * ~} for >= GEQ =>
 * ~> for GTR
 * ~\ for DIV or ÷ (÷ is symbolized by -61 and -73, resp. 0xC3 and 0xB7)
 * ~. for · (· is symbolized by -62 and -73, resp. 0xC4 and 0xB7)
 * ~ tilde for REM and MOD
 * ~The substitution has the advantage to consider operators as one-char items
 * Note: using option d (debug), current line is always printed after 
 * tokenizing and blank-stripping, to see the 'real' string under evaluation
 * System function */
void optoken(char *q) {
	/* assignment */
	if (!strncasecmp_(q,":=",2)) 
		*q++=PU, *q=' ';
	else if (!strncasecmp_(q,"<-",2)) 
		*q++=PU, *q=' ';
	/* equality */
	else if (!strncasecmp_(q,"EQL",3)  && isante(q) && ispost(q+2))
		*q++='=', *q++=' ', *q=' ';
	/* inequality */
	else if (!strncasecmp_(q,"<>",2)) 
		*q++='#', *q=' ';
	else if (!strncasecmp_(q,"NOTEQUAL",8))
		*q++='#', *q++=' ',*q++=' ', *q++=' ',
			*q++=' ', *q++=' ', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,"!=",2)) 
		*q++='#', *q=' ';
	else if (!strncasecmp_(q,"><",2)) 
		*q++='#', *q=' ';
	else if (!strncasecmp_(q,"NEQ",3)  && isante(q) && ispost(q+2))		
		*q++='#', *q++=' ', *q=' ';
	/* exponentiation */
	else if (!strncasecmp_(q,"**",2)) 
		*q++='^', *q=' ';
	/* AND */
	else if (!strncasecmp_(q,"AND",3) && isante(q) && ispost(q+2))
		*q++='_', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,"/\\",2)) *q++='_', *q=' ';
	/* OR */
	else if (!strncasecmp_(q,"OR",2) && isante(q) && ispost(q+1))
		*q++='|', *q=' ';
	else if (!strncasecmp_(q,"\\/",2)) *q++='|', *q=' ';
	/* equivalence */
	else if (!strncasecmp_(q,"==",2)) 
		*q++='\?', *q=' ';
	else if (!strncasecmp_(q,"EQV",3) && isante(q) && ispost(q+2))
		*q++='\?', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,"EQIV",4) && isante(q) && ispost(q+3))		
		*q++='\?', *q++=' ', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,"<->",3))
		*q++='\?', *q++=' ', *q=' ';
	/* implication */
	else if (!strncasecmp_(q,"->",2)) 
		*q++='!', *q=' ';
	else if (!strncasecmp_(q,"IMP",3) && isante(q) && ispost(q+2))
		*q++='!', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,"IMPL",4) && isante(q) && ispost(q+3))
		*q++='!', *q++=' ', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,">>",2)) 
		*q++='!', *q=' ';
	/* lesser */
	else if (!strncasecmp_(q,"LSS",3) && isante(q) && ispost(q+2))
		*q++='<', *q++=' ', *q=' ';
	/* lesser or equal */
	else if (!strncasecmp_(q,"<=",2)) 
		*q++='{', *q=' ';
	else if (!strncasecmp_(q,"LEQ",3) && isante(q) && ispost(q+2))
		*q++='{', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,"=<",2)) 
		*q++='{', *q=' ';
	/* greater or equal */
	else if (!strncasecmp_(q,">=",2)) 
		*q++='}', *q=' ';
	else if (!strncasecmp_(q,"GEQ",3) && isante(q) && ispost(q+2))		
		*q++='}', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,"=>",2)) 
		*q++='}', *q=' ';
	/* greater */
	else if (!strncasecmp_(q,"GTR",3) && isante(q) && ispost(q+2))
		*q++='>', *q++=' ', *q=' ';
	/* integer division */
	else if (!strncasecmp_(q,"DIV",3) && isante(q) && ispost(q+2))
		*q++='\\', *q++=' ', *q=' ';
	else if (*q==-61 && *(q+1)==-73) 
		*q++='\\', *q=' ';
	/* multiplication with the central dot */
	else if (*q==-62 && *(q+1)==-73) 
		*q++='*', *q=' ';
	/* remainder */
	else if (!strncasecmp_(q,"REM",3) && isante(q) && ispost(q+2))
		*q++='`', *q++=' ', *q=' ';
	else if (!strncasecmp_(q,"MOD",3) && isante(q) && ispost(q+2))
		*q++='`', *q++=' ', *q=' ';
}


/******************************************************************************
 * The following functions define general system environment subroutines: 
 * they are internal routines not directly accessible as ALGOL 60 statements.
 ******************************************************************************/

/*****************************************
 *            STATEMENTS AID             *
 *****************************************/


/* setupName()
 * check file name, returning correct file name with extension "alg", if not
 * specified for an inexistent file
 * System function for file treatment */
void setupName(char *fn) {
	int isExt=TRUE;
        char *q=fn, ext[COMPSTR], tempf[COMPFIL];
	FILE *fd;

	/* setup defaults */
	ext[0]='\0';

	/* check if the input file is a directory */
	stat(fn, &statbuf);
	if (S_ISDIR(statbuf.st_mode)) {
		beginS=-1;
		pe(15,-1);
	}
	
	/* find extension start (and check eventual specifiers set after
	 * extension in place of after the name) */
	while (*q) q++;
	while (*fn=='.') fn++;
	while (*q !='.' && q > fn) q--;

	/* there is an extension */
	if (q > fn) {
		strncpy_(ext,q,COMPSTR);
		*q='\0';
		q--;
	}
	/* there's no extension */
	else {
		if ((int)strlen(q)>0) q=fn+(int)strlen(q)-1;
		isExt=FALSE;
	}

	/* if a dot was printed, but no extension specified, remove the dot */
	if (!strcasecmp(ext,".")) ext[0]='\0', isExt=FALSE;

	/* if an extension was given as input, attach it */
	if (isExt) strncat_(fn, ext, COMPSCR);
	/* search for implicit extension or check if it's needed */
	else {
		/* first attempt: the name alone, extension not present */
		sprintw(tempf,COMPFIL,"%s",fn);
		if ((fd=fopen(tempf,"r"))) {
			fclose(fd);
			return;
		}
		/* second attempt: .ALG extension not specified */
		sprintw(tempf,COMPFIL,"%s%s",fn,EXTU);
		if ((fd=fopen(tempf,"r"))) {
			fclose(fd);
			strncat_(fn,EXTU,COMPSCR);
			return;
		}
		/* third attempt: .alg extension not specified */
		sprintw(tempf,COMPFIL,"%s%s",fn,EXTL);
		if ((fd=fopen(tempf,"r"))) {
			fclose(fd);
			strncat_(fn,EXTL,COMPSCR);
			return;
		}
	}
}


/* isanystring(string-position)
 * System function */
int isanystring(char *lp) {
	int ns=findDEC(lp);
	if (ns && _type==STRINGTYPEBOX) return TRUE;
	else if (*lp=='\"') return TRUE;
	else if (*lp=='\'' && *(lp+1)=='\'')  return TRUE;
	/* consider all string functions */
	else if (!strncasecmp_(p,"CONCAT(",7)) return TRUE;
	else if (!strncasecmp_(p,"COPY(",5)) return TRUE;
	else if (!strncasecmp_(p,"FDATE(",6)) return TRUE;
	else if (!strncasecmp_(p,"TIME",4)) return TRUE;
	else if (!strncasecmp_(p,"VDATE(",6)) return TRUE;
	else if (!strncasecmp_(p,"TAKE(",5)) return TRUE;
	else if (!strncasecmp_(p,"DROP(",5)) return TRUE;
	else if (!strncasecmp_(p,"TAIL(",5)) return TRUE;
	else if (!strncasecmp_(p,"NEWSTRING(",10)) return TRUE;
	return FALSE;
}	


/* dimArray(matrix-index,type,dimensions width)
 * dimension a numerical array allocating the necessary memory space.
 * var: the index of the variable holding the matrix name
 * T: matrix type, 1 for unidimensional vectors, 2 for matrices
 * val: the array of dimensions
 * Note: all elements of declared array elements are set to 0.0 = zero. 
 * System function for the ARRAY management */
int dimArray(int var, int T, int *val) {
	int i;
	int size=1;
	for (i=1;i<=T;i++) size*=(val[i]+1);
	size+=1; // safety space
	size*=sizeof(double);

	/* reserve memory space for the array */
	dim[var].elem=nalloc((size_t)size);  
	if (!dim[var].elem) pe(13,l);

	/* erase every element */
	for (i=0;i<=size;i++) *(dim[var].elem+i)=0;
	return size;
}	


/* getArrayVal(matrix-index,type,positions)
 * get value of an element of a numerical array, given its coordinates.
 * var: the index of the variable holding the matrix name
 * t: matrix dimension coming from the calling instance
 * val: the local dimensions required array
 * System function for the ARRAY management */
double getArrayVal(int var, int t, int *val) {
	int i, res=0;
	/* check dimensions */
	if (t != vn[var].dim) pe(27,l);
	/* check bounds and update values to var offsets */
	for (i=1; i<=t; i++) {
		if (val[i]<vn[var].offset[i] || val[i]>vn[var].ldim[i]+vn[var].offset[i]) 
			pe(27,l);
		res+=vn[var].coeff[i]*(val[i]-vn[var].offset[i]);
	}
	return *(dim[var].elem+res);
}


/* setArrayVal(matrix-index,type,positions)
 * set an element of a numerical array to a specific value, given the value 
 * itself and the element coordinates.
 * var: the index of the variable holding the matrix name
 * t: matrix dimension coming from the calling instance
 * val: the local dimensions required array
 * num: the value to be stored
 * System function for the ARRAY management */
void setArrayVal(int var, int t, int *val, double num){
	int i, res=0;
	/* check dimensions */
	if (t != vn[var].dim) pe(27,l);
	/* check bounds and update values to var offsets */
	for (i=1; i<=t; i++) {
		if (val[i]<vn[var].offset[i] || val[i]>vn[var].ldim[i]+vn[var].offset[i]) 
			pe(27,l);
		res+=vn[var].coeff[i]*(val[i]-vn[var].offset[i]);
	}
	*(dim[var].elem+res)=num;
}


/* dimStrArray(matrix-index,type,dimensions width)
 * dimension a string array, allocating the necessary memory space; 
 * var: the index of the variable holding the matrix name
 * T: the matrix type, 1 for unidimensional vectors, 2 for matrices (currently
 *    not available)
 * val: the array of dimensions
 * Note: all elements of declared array are set to the void string.
 * System function for the ARRAY management */
int dimStrArray(int var, int T, int *val) {
	int i;
	int size=1;
	for (i=1;i<=T;i++) size*=(val[i]+1);
	size+=1; // safety space
	size*=COMPSTR;

	/* reserve memory space for the array */
	dimS[var].elem=salloc((size_t)size);  
	if (!dimS[var].elem) pe(13,l);

	/* erase all array elements */
	for (i=0;i<size;i++) *(dimS[var].elem+i)='\0';
	return size;
}		


/* getArrayStr(matrix-index,type,positions)
 * get value of an element of a string array, given its coordinates.
 * var: the index of the variable holding the matrix name
 * T: matrix dimension, 1 for unidimensional vectors, 2 for matrices, 3,4,...
 * val: the local dimensions required numbers
 * System function for the ARRAY management */
char *getArrayStr(int var, int t, int *val) {
	int i, res=0;
	/* check dimensions */
	if (t != vn[var].dim) pe(27,l);
	/* check bounds and update values to var offsets */
	for (i=1; i<=t; i++) {
		if (val[i]<vn[var].offset[i] || val[i]>vn[var].ldim[i]+vn[var].offset[i]) 
			pe(27,l);
		res+=vn[var].coeff[i]*(val[i]-vn[var].offset[i]);
	}
	res*=COMPSTR;
	return dimS[var].elem+res;
}


/* setArrayStr(matrix-index,type,positions,value)
 * set an element of a string array to a specific value, given the value itself 
 * and the element coordinates 
 * t: matrix type, 1 for unidimensional vectors, 2 for matrices
 * var: the index of the variable holding the matrix name
 * val: the local dimensions required numbers
 * str: the value to be stored
 * System function for the ARRAY management */
void setArrayStr(int var, int t, int *val, char *str){
	int i, res=0;
	/* check dimensions */
	if (t != vn[var].dim) pe(27,l);
	/* check bounds and update values to var offsets */
	for (i=1; i<=t; i++) {
		if (val[i]<vn[var].offset[i] || val[i]>vn[var].ldim[i]+vn[var].offset[i]) {
			pe(27,l);
		}
		res+=vn[var].coeff[i]*(val[i]-vn[var].offset[i]);
	}
	res*=COMPSTR;
	strncpy_(dimS[var].elem+res,str,COMPSTR);
}


/* getBound()
 * return the lowest or the upper bound of array in argument.
 * The request is in the form LB/UB(V,N), where V is the array name and
 * N is the dimension.
 * if mode = TRUE search for UB (upper bound)
 * if mode = FALSE, search for LB (lower bound)
 * if mode = MAXDIM return dimension for DIM
 * system function for the LB and UB functions */
int getBound(int mode) {
	int ub=0,cs=0;
	int alt=1;

	/* get array name */
	forceArray=TRUE;
	cs=findDEC(p);
	if (cs && _arr) p+=_len;
	else pe(9,l);
	forceArray=FALSE;
	/* DIM returns dimension of the array */
	if (mode==MAXDIM) return vn[vn[cs].ref].dim;
	/* dimension index is expicit */
	if (*p==',') {
		p++;
		alt=(int)S();
	}
	else pe(2,l);

	/* LB returns always the offset */
	if (!mode) ub=vn[vn[cs].ref].offset[alt];
	/* UB returns the upper value */
	else ub=vn[vn[cs].ref].offset[alt]+vn[vn[cs].ref].ldim[alt];
	return ub;
}


/* SETRAN()
 * perform the SETRAN procedure 
 * Ref. AA-O196C-TK - 17.8 "Random Number Routine" */
void SETRAN(void) {
	if (*p=='(') p++;
	else pe(2,l);
	/* evaluate number */
	RANDOMIZE((int)S());
	if (*p==')') p++;
	else pe(2,l);
}


/* RANDOMIZE()
 * perform the SETRAN procedure.
 * If called with argument null, it set the random generation depending on
 * current time, according to the following algorithm:
 *   - divide current 'usec' measure with the sum of the value obtained summing 
 *     months (12), days (31), hours (23), minutes (59) and seconds (59), which 
 *     always gives 184;
 *   - divide this value (always in the range [1,184]) with the logarithm of 
 *     276.00 which is 184.00x1.5, obtaining a value between 0.177923252 and 
 *     32.73787838, a range with medium value 16.45790082, the starting default
 *     value of the getRandom() function 
 * if an argument different from zero is given, this argument is used to set 
 * the random generation.
 * Note: at start, ALGOL calls RANDOMIZE(0), whereas SETRAN(X) calls 
 * RANDOMIZE(x).
 * Used in conjunction with the getRandom() function. */
void RANDOMIZE(int seed) {
	int b;
	if (!seed) {
		gettimeofday(&tv, NULL);
	       	b = tv.tv_usec%184;
		randSeed=(int)(133.5611413*b+4095.875);
	}
	else randSeed=abs(seed);
	if (!randSeed%2) randSeed++;
}


/* getRandom()
 * return a pseudo-random number, called by the RND function.
 * The algorithm is the following:
 *  1. randSeed and randDivi at start are respectively 7139 and 32768
 *  2. divide randSeed by randDivi, and get the decimals part, from third digit
 *     to obtain a number greater than 0 (excluded) and lower than 1 (excluded) 
 *     as the generated random number (0.217864990 -> 0.7864990)
 *  3. the first five decimals of this number are taken as the new randSeed for 
 *     the next iteration (randSeed=21787)
 *  4. proceed from step 2.
 * Note: to reduce the risk of circular ranges I add two variables, varying
 * continuously: 'fix' that alters randSeed and div that alters randDivi; to
 * reduce also the risk of having 'exact' divisions (like 0.250000, which is 
 * 'unfair' as random number) randSeed is always odd, randDivi is even.
 * Note: This algorithm, after a variable length list of generated numbers, 
 * falls into a cycle, whose length is exactly 8532532; the start of the cycle
 * is variable, and depends on the initial values for randSeed and randDivi.
 * Note: the values of 27268 and 32767 for the starting values of randSeed and
 * randDivi were chosen to return as the first random number the same first
 * random number generated by the DEC-ALGOL 60
 * System function for the pseudo-random number generation management */
double getRandom(void) {
	static double fix=0.3; // variable randSeed part
	static double div=0.1; // variable randDivi part
	double o=100.0*randSeed/randDivi;

	/* take fractional part of the division of randSeed by randDivi
	 * (take digits from 3rd to 8th position) */
	o-=(int)o;

	/* update randSeed */
	randSeed=(unsigned int)((o*1E5)+fix);
	if (randSeed<1E3) randSeed=27268;  
	if (!randSeed%2) randSeed++;

	/* update randDivi */
	randDivi=(unsigned int)((randDivi*randSeed)%0xFFFFF+div);
	if (randDivi<1E3) randDivi=32767; 
	if (randDivi%2) randDivi++;

	/* update variable parts */
	div+=3.0; if (div>499) div=-997;
	fix+=167.0; if (fix>4999) fix=0;

	/* ensure limits are respected (see previous note) */
	if (o<ZERO||o>=1.0) return getRandom(); // discard also neg. results
	return o;
}


/* findPROC() 
 * find procedure in current parsing line searching among data found by 
 * preParse()
 * p points to the PROC name in getFunc(), exec() or SS() 
 * return in p2 updated position of p (just after the proc name), unless
 * p2 is NIL
 * store in type and mode relative values for an immediate check 
 * type=TYPEBOX
 * System function for the PROCEDURE management */
int findPROC(char *p, char **p2, int *type) {
	int i=procTOS, dots=0,cn=0;
	char name[COMPSTR], *n=name;

	/* reset length */
	_len=0;

	if (!*p) return FALSE;

	/* get PROC name */
	if (isbeginnamechar(p)) {
		while (isnamechar(p) && cn<NAMEDIM) {
			if (*p=='.' && isalnum(*(p+1))) {
				p++;
				dots++;
				continue;
			}
			*n++=*p++;
			cn++;
		}
		*n='\0';
	}
	else return FALSE;
	if (cn>=NAMEDIM) return FALSE;
	_len=(int)strlen(name)+dots;
	if (!_len || !name[0]) return FALSE;

	/* return immediately if it's last call */
	if (!strcmp(rememberName,name)) {
		if (p2) *p2=p;
		*type=Procp[rememberCode].type;
		return rememberCode;
	}

	/*  find PROC */
	while (i>0) {
		if (!strcmp(Procp[i].name,name)) {
			*type=Procp[i].type;
			if (p2) *p2=p;
			/* remember last... */
			strncpy_(rememberName,Procp[i].name,NAMEDIM);
			rememberCode=i;
			return i;
		}
		i--;
	}
	/* restore p: PROC not found */
	if (p2) *p2=p+strlen(name);
	return FALSE;
}


/* checkProcCall()
 * check if procedure is retrievable 
 * (basing on the fact that a recursive call must not see the 
 * sister procedures as proper */
int checkProcCall(int cnp) {
	if (pexec[pexecTOS-2]==pexec[pexecTOS-1]) return FALSE;
	return TRUE;
}


/* checkProcExec() 
 * check if procedure is callable (dependence) 
 * ~Step 1: at level pexecTOS-1 there's current procedure code,
 * at level pexecTOS-2 there's caller procedure code; if they
 * don't match the procedure is not calling itself.
 * ~Step 2: in Procp[cnp].procref there's the reference code
 * of the procedure (the code of the ambient where it lies); if it's
 * null no check is done because any procedure at level 0 is callable.
 * ~Step 3: in case a control has to be done, the check is done
 * by confronting the the caller procedure code with the reference
 * code of the current procedure: if they don't match the procedure
 * is not callable. */
int checkProcExec(void) {
	int this, caller, number, callernumber, procref, callerprocref;
	this=pexec[pexecTOS-1];
	caller=pexec[pexecTOS-2];
	procref=Procp[this].procref;
	number=Procp[this].number;
	callerprocref=Procp[caller].procref;
	callernumber=Procp[caller].number;
	do {
		if (procref<0) return TRUE;
		if (procref==callernumber) return TRUE;
		if (procref==callerprocref) return TRUE;
		if (number==callerprocref) return TRUE;
		caller=Procp[caller].procref;
		callerprocref=Procp[caller].procref;
		callernumber=Procp[caller].number;
	} while (callerprocref>=0);
	return FALSE;
}


/* execPROC() 
 * execute a subroutine; 
 * data relative to the PROC are preprocessed by preParse(), in order for 
 * execPROC to run. Must be run if findPROC returned a positive value.
 * cnp: the procedure number found by findPROC()
 * pc: the position in the incoming string where arguments (if any) are stored,
 *     pointing to the character after PROC name. pc is used local, instead of
 *     global p, because execPROC() is supposed to execute any type of string
 * numres: the double result pointer
 * strres; the string result pointer
 * return the updated position of program pointer in pc
 * manage the recursion
 * System function for the PROC management 
 * Ref. AA-0196C-TK - 11 "Procedures" */
char *execPROC(int cnp, char *pc, double *numres, char *strres) {
	char cnpname[COMPSTR], *pcb;
	char BB[COMPSTR]; // for BASESTR backup
	int decBack=decCount, procBack=procTOS, cnt, ic;
	int lb=l, isBackProcTime;
	TProcP backP;
	int wasPassedByName=FALSE; // to recover cnp data at the end 

	/* set execution reference */
	pexec[pexecTOS++]=cnp;
	if (pexecTOS>=LIMIT) {
		p=pc;
		pe(7,l);
	}
	
	/* set for functions called by name-passing: in this case
	 * determine the original function and set this current cnp to
	 * be perfectly equal to the called qs */
	while (Procp[cnp].numvars<0) {
		int type;
		int qs=findPROC(Procp[cnp].strdata,NIL,&type);
		if (qs) {
			backP=Procp[cnp];	
			Procp[cnp]=Procp[qs];
			wasPassedByName=TRUE;
		}
	}

	/* PROC does not exist */
	if (!Procp[cnp].number) {
		p=pc;
		pe(17,l);
	}
	
	/* create fictitious block */
	beginTOS++;
	if (beginTOS>=LIMIT) {
		p=pc;
		pe(12,l); // passed recursion limit
	}
	beginDepth[beginTOS]=decCount;
	
	/* check if procedure is callable  */
	if (!checkProcExec()) {
		p=pc;
		pe(73,l);
	}

	Procp[cnp].numres=0.0;
	Procp[cnp].strres[0]='\0';
	
	/* store current proc name for future usage */
	strncpy_(cnpname,Procp[cnp].name,NAMEDIM);

	/* in case of return value, declare a fictitious variable with 
	 * the same name of the procedure
	 * Note: return value for procedure is a simple value, not
	 * an array (at least I've never read of such feature neither in 
	 * the DEC manual nor in the ISO document. */
	if (Procp[cnp].type<NOTYPEBOX) {
		int i;
		decCount++;
		if (decCount>=LIMIT) {
			p=pc;
			pe(7,l);
		}
		/* FICTITIOUS VARIABLE */
		strncpy_(vn[decCount].name,cnpname,NAMEDIM);
		vn[decCount].type=Procp[cnp].type;
		vn[decCount].isArr=FALSE;		
		vn[decCount].ref=decCount;
		vn[decCount].refcode=decCount;
		vn[decCount].proccode=0;
		vn[decCount].dim=0;
		vn[decCount].size=0;
		vn[decCount].strmem=ALGSTR;
		vn[decCount].isOwn=0;
		vn[decCount].proccode=Procp[cnp].number;
		vn[decCount].isFict=TRUE;
		vn[decCount].isFunc=FALSE;
		vn[decCount].line=l;
		vn[decCount].level=pexecTOS;
		for (i=0;i<MAXDIM;i++) {
			vn[decCount].ldim[i]=0;
			vn[decCount].offset[i]=0;
			vn[decCount].coeff[i]=0;
		}
		for (i=0;i<COMPSTR;i++) {
			vn[decCount].valdata[i]='\0';
			vn[decCount].strdata[i]='\0';
		}
		/* reserve memory for string variables and reset values */
		if (Procp[cnp].type>=STRINGTYPEBOX) {
			PS[decCount]=calloc(vn[decCount].strmem,sizeof(char)*vn[decCount].strmem);
			if (!PS[decCount]) {
				p=pc;
				pe(6,l);
			}
			PS[decCount][0]='\0';
		}
		else P[decCount]=0.0;
		
		/* debug features */
		if (isDebug) 
			showDEC(decCount, "Declared fictitious variable ");
	}

	/* if there are arguments store them... */
	if (*pc=='(')  {
		cnt=0;
		ic=1;
		++pc; // skip '('
		/* cycle through PROC arguments and get values to be used in
		 * case of BYVAL; in case of BYREF, set reference to the one
		 * of the calling entity. 
		 * cnt holds number of arguments of called proc, stored by 
		 * preParse() in Proc[cnp].numvars (which must be respected) */
		while (cnt < Procp[cnp].numvars) {
			char wname[COMPSTR],*wp=wname;
			char ename[COMPSTR],*ep=ename;
			int isNotAFunc=FALSE;
			int type, mode, state;
			int isP=0;
			cnt++;
			if (cnt>MAXDIM) {
				p=pc;
				pe(7,l);
			}
		
			/* wrong number of arguments */
			if (cnt>Procp[cnp].numvars) {
				p=pc;
				pe(25,l);
			}
			if (*pc==')' && cnt<=Procp[cnp].numvars) {
				p=pc;
				pe(25,l);
			}
			/* get name of runtime argument (WW) 
			 * Note: argument may be a variable name, a string
			 * enclosed in quotes or a number, whose first
			 * character is the dot, the minus or &,@ or a digit.*/
			while (*pc && *pc!=',' && *pc!=')' && *pc!=']') {
				if (*pc=='(') {
					int st=0;
					*wp++=*pc++; // copy (
					while (*pc) {
						if (*pc=='(') st++;
						else if (*pc==')' && st) st--;
						else if (*pc==')' && !st) break;
						*wp++=*pc++;
					}
				}
				else if (*pc=='[') {
					int st=0;
					*wp++=*pc++; // copy [
					while (*pc) {
						if (*pc=='[') st++;
						else if (*pc==']' && st) st--;
						else if (*pc==']' && !st) break;
						*wp++=*pc++;
					}
					if (*pc==',' || *pc==')');
					else isNotAFunc=TRUE;	
				}
				*wp++=*pc++;
			}
			*wp='\0';
			/* get name of stored argument (E) */
			while (Procp[cnp].vars[ic]>0) 
				*ep++=Procp[cnp].vars[ic++];
			*ep='\0';
			ic++; // skip -1
			type=Procp[cnp].vars[ic++]-DECADD;
			mode=Procp[cnp].vars[ic++]-DECADD;
			state=Procp[cnp].vars[ic++]-DECADD;
			ic++; // skip -2

			/* FUNCTION CALL IN PLACE */
			/* maybe a procedure is passed? */
			isP=findPROC(wname,NIL,&type);
			/* recognized: instantiate procedure; */
			if (state==ISPROC && isP && !isNotAFunc) {
				procTOS++;
				if (procTOS>=LIMIT) {
					p=pc;
					pe(7,l);
				}
				strncpy_(Procp[procTOS].name,ename,NAMEDIM);
				strncpy_(Procp[procTOS].strdata,wname,COMPSTR);
				Procp[procTOS].procref=cnp;
				Procp[procTOS].procL=-1;
				Procp[procTOS].procP=-1;
				Procp[procTOS].begnL=-1;
				Procp[procTOS].begnP=-1;
				Procp[procTOS].termL=-1;
				Procp[procTOS].termP=-1;
				Procp[procTOS].numvars=-1;
				Procp[procTOS].number=procTOS;
				Procp[procTOS].type=type;
				if (isDebug) {
					fprintf(stdout,"%s",INVCOLOR);
					fprintf(stdout,"Created reference of procedure %s called %s in procedure %s\n",wname,ename,Procp[cnp].name);
					fprintf(stdout,"%s",RESETCOLOR);
				}
			}
			else if (state==ISPROC) {
				pe(72,l);
			}
			/* INSTANTIATE VARIABLE OR ARRAY */
			else {
				int isEArr=FALSE; // is Array Element
				int isExpr=FALSE; // is an Expression
				double valres=0.0;
				char strres[COMPSTR];
				int labres=0, ns=0, j;
				int bforceArray=forceArray;
				int thisdecCount=decCount;

				/* FIND IF THERE IS AN ASSOCIATED VARIABLE
				 * (for possible later usage for BYREF)
				 * reduce decCount otherwise it would search 
				 * in the already declared variables... */
				decCount=decBack;
				if (state==ISARR) forceArray=TRUE;
				ns=findDEC(wname);
				if (state==ISARR) forceArray=bforceArray;
				decCount=thisdecCount;  // restore
				/* search if found */	
				if (ns) {
					char *q=wname+_len;
					int st=0;
					while (ns!=vn[ns].ref) ns=vn[ns].ref; 
					/* the variable is an expicit array */
					if (*q=='[') {
						q++; // skip [
						while (*q && *q!=']') {
							if (*q=='[') st++;
							else if (*q==']'&&st) 
								st--;
							else if (*q==']'&&!st) 
								break;
							q++;
						}
						if (*q==']') q++; // skip ]
						if (!*q) isEArr=TRUE;
						else isExpr=TRUE;
					}
					/* variable is part of an expression */
					else if (*q) {
						ns=0;
						isExpr=TRUE;
					}
				}
				else isExpr=TRUE;

				/* DETERMINE CURRENT BARE VALUE 
				 * of the item passed to the procedure */
				if (!isP && type<STRINGTYPEBOX && state!=ISARR && mode==BYVAL) {
					char *pb=p;
					p=wname;
					valres=S();
					p=pb;
				}
				else if (!isP && type==STRINGTYPEBOX && state!=ISARR) {
					char *pb=p;
					p=wname;
					strncpy_(strres,SS(),COMPSTR);
					p=pb;
				}
				else if (!isP && type==LABELTYPEBOX && state!=ISARR) {
					char *pb=p;
					p=wname;
					labres=getLabelIndex();
					p=pb;
				}

				/* DO THE INSTANTIATION */
				decCount++;
				if (decCount>=LIMIT) {
					p=pc;
					pe(7,l);
				}
				/* SET FUNDAMENTAL PARAMETERS
				 * name, type and procedure coding */
				for (j=0;j<COMPSTR;j++) vn[decCount].strdata[j]='\0';
				j=0;
				while (ename[j]) vn[decCount].name[j]=ename[j], j++;
				while (j<COMPSTR) vn[decCount].name[j]='\0', j++;
				vn[decCount].type=type;
				vn[decCount].proccode=Procp[cnp].number;
				vn[decCount].isFict=FALSE;
				vn[decCount].line=l;
				vn[decCount].level=pexecTOS;
				/* label items are always passed by value 
				 * TODO: decide if it's an error */
				if (type==LABELTYPEBOX) mode=BYVAL;
				
				/* SET THE ASSOCIATED FUNCTION IF ANY
				 * (for possible later usage as expression) */
				vn[decCount].isFunc=findPROC(wname,NIL,&type);
				/* DEFINE VARIABLE PARAMETERS:
				 * case of expression (we know that ns 
				 * is null or it does not matter)*/
				if (isExpr) {
					for (j=0;j<COMPSTR;j++) vn[decCount].valdata[j]=wname[j];
					vn[decCount].isOwn=FALSE;
					vn[decCount].isArr=FALSE;
					vn[decCount].strmem=ALGSTR;
					if (mode==BYVAL) {
					   vn[decCount].ref=decCount;
					   vn[decCount].refcode=decCount;
					}
					else if (mode==BYREF) {
					   vn[decCount].ref=decCount;
					   vn[decCount].refcode=-decCount-DECADD;
					}
				}
				/* DEFINE VARIABLE PARAMETERS:
				 * case of simple variable or array:
				 * isExpr FALSE and isEArr FALSE */
				else if (!isExpr && !isEArr) {
					for (j=0;j<COMPSTR;j++) vn[decCount].valdata[j]=wname[j];
					vn[decCount].isOwn=FALSE;
					vn[decCount].isArr=state==ISARR;
					vn[decCount].strmem=ALGSTR;
					if (mode==BYVAL) { 
					   vn[decCount].ref=decCount;
					   vn[decCount].refcode=decCount;
					}
					else if (mode==BYREF) {
					   vn[decCount].ref=ns;
					   vn[decCount].refcode=-ns;
					}
				}
				/* DEFINE VARIABLE PARAMETERS:
				 * case of an array element passed */
				else if (!isExpr && isEArr) {
					for (j=0;j<COMPSTR;j++) vn[decCount].valdata[j]=wname[j];
					vn[decCount].isOwn=FALSE;
					vn[decCount].isArr=FALSE;
					vn[decCount].strmem=ALGSTR;
					if (mode==BYVAL) {
					   vn[decCount].ref=decCount;
					   vn[decCount].refcode=-decCount-DECADD;
					}
					else if (mode==BYREF) {
					   vn[decCount].ref=ns;
					   vn[decCount].refcode=-ns-DECADD; 
					}
				}

				/* SET CURRENT VALUES: NUMERIC ARRAYS
				 * instantiate numeric array */
				if (state==ISARR && mode==BYVAL && type<STRINGTYPEBOX && !isExpr && !isEArr) {
					int j;
					for (j=0;j<MAXDIM;j++) {
					   vn[decCount].ldim[j]=vn[ns].ldim[j];
					   vn[decCount].offset[j]=vn[ns].offset[j];
					   vn[decCount].coeff[j]=vn[ns].coeff[j];
					}
					vn[decCount].size=vn[ns].size;
					vn[decCount].dim=vn[ns].dim;
					dimArray(decCount,vn[decCount].dim,vn[decCount].ldim);
					memcpy(dim[decCount].elem,dim[ns].elem,vn[ns].size);
				}
				/* SET CURRENT VALUES: STRING ARRAYS
				 * instantiate string array */
				else if (state==ISARR && mode==BYVAL && type<NOTYPEBOX && !isExpr && !isEArr) {
					int j;
					for (j=0;j<MAXDIM;j++) {
					   vn[decCount].ldim[j]=vn[ns].ldim[j];
					   vn[decCount].offset[j]=vn[ns].offset[j];
					   vn[decCount].coeff[j]=vn[ns].coeff[j];
					}
					vn[decCount].size=vn[ns].size;
					vn[decCount].dim=vn[ns].dim;
					dimStrArray(decCount,vn[decCount].dim,vn[decCount].ldim);
					memcpy(dimS[decCount].elem,dimS[ns].elem,vn[ns].size);
				}
				/* SET CURRENT VALUE: NUMERIC VARIABLE
				 * instantiate numeric variable */
				else if (type<STRINGTYPEBOX) {
					P[decCount]=valres;
					for (j=0;j<MAXDIM;j++) {
					   vn[decCount].ldim[j]=0;
					   vn[decCount].offset[j]=0;
					   vn[decCount].coeff[j]=0;
					}
					vn[decCount].dim=0;
					vn[decCount].size=0;
				}
				/* SET CURRENT VALUE: STRING VARIABLE
				 * instantiate string variable */
				else if (type==STRINGTYPEBOX) {
					PS[decCount]=salloc(strlen(strres)+1);
					if (!PS[decCount]) pe(6,l);
					strncpy_(PS[decCount],strres,strlen(strres)+1);
					strncpy_(vn[decCount].valdata,strres,COMPSTR);
					for (j=0;j<MAXDIM;j++) {
					   vn[decCount].ldim[j]=0;
					   vn[decCount].offset[j]=0;
					   vn[decCount].coeff[j]=0;
					}
					vn[decCount].dim=0;
					vn[decCount].size=0;
				}
				/* SET CURRENT VALUE: LABEL VARIABLE
				 * instantiate label variable */
				else if (type<NOTYPEBOX) {
					P[decCount]=labres;
					for (j=0;j<MAXDIM;j++) {
					   vn[decCount].ldim[j]=0;
					   vn[decCount].offset[j]=0;
					   vn[decCount].coeff[j]=0;
					}
					vn[decCount].dim=0;
					vn[decCount].size=0;
				}

				/* a BYREF which is passed an expression
				 * can be read but not written 
				 * 2*DECADD forces assign() to emit an error */
				if (isExpr && mode==BYREF)
					vn[decCount].refcode=2*DECADD;
			}
			/* debug features */
			if (isDebug && state!=ISPROC) 
				showDEC(decCount, "Declared ");
			
			/* next variable */
			if (*pc==',') pc++;
			/* manage comments interspersed with arguments */
			else if (*pc==')' && cnt<Procp[cnp].numvars && strcasestr_oq(pc,":(")) {
				while (*pc!=':') pc++;
				pc++; // skip :
				pc++; // skip (
			}
		}
	}
	/* ...otherwise set current counter to void */
	else cnt=0;

	/* wrong number of arguments */
	if (Procp[cnp].numvars!=cnt) {
		p=pc;
		pe(25,l);
	}
	if (cnt) {
	       if (*pc==')') pc++;
	       else {
		       p=pc;
		       pe(25,l);
	       }
	}

	/* execute procedure */
	isBackProcTime=isProcTime;
	isProcTime=cnp;
	strncpy_(BB,BASESTR,COMPSTR);
	l=Procp[cnp].begnL;
	strncpy_(BASESTR,m[l],COMPSTR);
	p=BASESTR+Procp[cnp].begnP;
	pcb=pc;
	/* execute procedure.
	 * If a GOTO occurs, LL is set, signalling to the system that a
	 * a GOTO has occurred; it's doBegin() that, using LL and PL,
	 * will set the new Program Counter; so execPROC() ceases any 
	 * responsibility for what about the destination */
	go(l);

	/* restore line */
	l=lb;
	pc=pcb;

	/* assign results */
	if (Procp[cnp].type<NOTYPEBOX) {
		if (Procp[cnp].type==LABELTYPEBOX)
			*numres=Procp[cnp].numres;
		else if (Procp[cnp].type<STRINGTYPEBOX)
			*numres=Procp[cnp].numres;
		else strncpy_(strres,Procp[cnp].strres,COMPSTR);
	}
	
	/* restore and reset values as priorly to the procedure execution, 
	 * and reset variables used data */
	for (ic=decCount;ic>decBack;ic--) {
		int i;
		for (i=0;i<COMPSTR;i++) {
			vn[ic].name[i]='\0';
			vn[ic].strdata[i]='\0';
			vn[ic].valdata[i]='\0';
		}
		vn[ic].type=NOTYPEBOX;
		vn[ic].isArr=FALSE;
		vn[ic].ref=0;
		vn[ic].dim=0;
		for (i=0;i<MAXDIM;i++) {
			vn[ic].ldim[i]=0;
			vn[ic].offset[i]=0;
			vn[ic].coeff[i]=0;
		}
		vn[ic].size=0;
		vn[ic].strmem=0;
		vn[ic].isOwn=FALSE;
		vn[ic].refcode=0;
		vn[ic].proccode=0;
		vn[ic].isFunc=FALSE;
		vn[ic].isFict=0;
		vn[ic].line=0;
		vn[ic].level=-1;
	}
	decCount=decBack;
	isProcTime=isBackProcTime;
	strncpy_(BASESTR,BB,COMPSTR);
	procTOS=procBack;
	if (wasPassedByName) Procp[cnp]=backP;	
	pexecTOS--;
	if (beginTOS>0) beginTOS--;
	return p=pc;
}


/* isValid()
 * return TRUE if str does not point to THEN, ELSE,DO,STEP,WHILE,UNTIL 
 * which interfere witht the parses */
int isValid(char *str) {
	if (!strncasecmp_(str,"THEN",4))  return FALSE;
	if (!strncasecmp_(str,"ELSE",4))  return FALSE;
	if (!strncasecmp_(str,"DO",2))    return FALSE;
	if (!strncasecmp_(str,"STEP",4))  return FALSE;
	if (!strncasecmp_(str,"WHILE",5)) return FALSE;
	if (!strncasecmp_(str,"UNTIL",5)) return FALSE;
	return TRUE;
}


/* findDEC() 
 * find a declared variable/array (DEC) in the vn[] structure;
 * lp points to the DEC name in getFunc(), assign() or SS() and get its value;
 * store in _len the length of the variable name
 * store in _type relative values for an immediate check
 * store in _arr the array flag
 * System function for the declare management */
int findDEC(char *lp) {
	int i=decCount, dots=0,cn=0;
	char name[COMPSTR], *n=name;
	int lArray=FALSE;	// locally detection of arrays
	int found=FALSE;
	int subscript=FALSE;

	/* set anyway a valid value as a safety measure */	
	_len=0;
	_arr=0;
	_type=0;

	if (!*lp) return FALSE;

	/* get DEC name (up to NAMEDIM characters) */
	if (isbeginnamechar(lp)) {
		while (isnamechar(lp) && cn<NAMEDIM && isValid(lp)) {
			if (*lp=='.' && isalnum(*(lp+1))) {
				lp++;
				dots++;
				continue;
			}
			*n++=*lp++, cn++;
		}
		*n='\0';
	}
	else return FALSE;
	if (cn>=NAMEDIM && isnamechar(lp)) return FALSE;
	_len=(int)strlen(name)+dots;
	if (!_len || !name[0]) return FALSE;
	/* determine byte subscript */
	if (*(lp-1)=='.' && *lp=='[') {
		subscript=TRUE;
		*--n='\0';
		_len--;
	}
	else if (*(lp-1)!='.' && *lp=='[') lArray=TRUE;

	/*  find DEC and attribute values */
	while (i>0) {	// search all variables
		if (!strcmp(vn[i].name,name)) {
			if (!vn[i].isArr && !forceArray && !lArray)
				found=TRUE;
			else if (vn[i].isArr && lArray) 
				found=TRUE;
			else if (vn[i].isArr && !lArray && forceArray)
				found=TRUE;
			if (found) {
				if (!subscript) _type=vn[i].type;
				else _type=CHAR;
				_arr=vn[i].isArr;
				return i;
			}
		}
		found=FALSE;
		i--;
	}

	/* DEC irremediably not found */
	_type=0;
	_len=0;
	return FALSE;
}


/*****************************************
 *             MATH & TIME               *
 *****************************************/

/* sign()
 * if argument is positive, return 1
 * if argument is negative, return -1
 * if argument is exactly zero, return 0 */
int sign(double s) {
	if (s<0) return -1;
	else if (s>0) return 1;
	return 0;
}


/* getTime()
 * return current time structure */
struct tm *getTime(void) {
	time_t st_lt;
	st_lt=time(NULL);
	return localtime(&st_lt);
}


/* getMin()
 * return current min (0-59) */
int getMin(void) {
	struct tm* vt = getTime();
	return (int) vt->tm_min;
}


/* getSec()
 * return current second (0-59) */
int getSec(void) {
	struct tm* vt = getTime();
	return (int) vt->tm_sec;
}


/* getHour()
 * return current hour (0-23) */
int getHour(void) {
	struct tm* vt = getTime();
	return (int) vt->tm_hour;
}


/* getDay()
 * return current day (1-31) */
int getDay(void) {
	struct tm* vt = getTime();
	return (int) vt->tm_mday;
}


/* getMonth()
 * return current month (1-12) */
int getMonth(void) {
	struct tm* vt = getTime();
	return (int) (vt->tm_mon+1);
}


/* getYear()
 * return current year */
int getYear(void) {
	struct tm* vt = getTime();
	return (int) (vt->tm_year+1900);
}


/* getMonthString()
 * return current month string 
 * if t is TRUE perform FDATE (abbreviated month)
 * while if FALSE perform VDATE (complete month) */
char *getMonthString(int t) {
	if (t) return cmonth[getMonth()];
	return month[getMonth()];
}


/* exp_()
 * wrapper for the exp functions 
 * Ref. ISO 1538-1984 (E) - page 7, par. 3.3.4.3 */
double exp_(int typex, double x, int typei, double i) {
	/* real exponent on any base */
	if (typex<=INTEGERTYPEBOX && typei<INTEGERTYPEBOX) {
		if (x>0.0) return exp(i*log(x));
		else if (x==0.0 && i>0.0) return 0.0;
		else if (x==0.0 && i==0.0) return 1.0;
		else pe(28,l);
	}
	/* integer exponent on integer base */
	else if (typex==INTEGERTYPEBOX && typei==INTEGERTYPEBOX) {
		if (i<0 || (x==0.0 && i==0)) pe(28,l); 
		else {
			int k, res=1.0;
			for (k=1;k<=i;k++) res=res*x;
			return res;
		}
	}
	/* integer exponenet on real base */
	else if (typex<INTEGERTYPEBOX && typei==INTEGERTYPEBOX) {
		if (i==0.0 && x==0.0) pe(28,l); 
		else {
			int k;
			double res=1.0;
			for (k=(int)fabsl(i);k>=1;k--) res=res*x;
			if (i<0) return 1.0/res;
			else return res;
		}
	}
	/* safe measure to avoid error in compilation */
	return 0.0;
}


/*****************************************
 *        INPUT/OUTPUT FORMATTING        *
 *****************************************/

/* FCR()
 * Carriage Return onto file stream 
 * ch: file stream channel number */
void FCR(int ch) {
	channel[ch].fcounter=0;
	if (ch<=STREAMS) fputc(CRC,channel[ch].stream);
	channel[ch].pageL++;
	updatePage(ch);
}


/* updateCounter()
 * update counter (and pageL and pageN) for the specified channel number */
void updateCounter(int ch) {
	if (ch>STREAMS) return;
	if (channel[ch].fcounter>FILESCRLIM) {
		channel[ch].fcounter=0;
		channel[ch].pageL++;
	}
	updatePage(ch);
}


/* updatePage()
 * update pageL and pageN for the specified channel number */
void updatePage(int ch) {
	if (ch>STREAMS) return;
	if (channel[ch].pageL>PAGEHEIGHT) {
		channel[ch].pageL=1;
		channel[ch].pageN++;
	}
}


/* doPage()
 * execute the [P] command in strings.
 * ch: the channel to where put the page-throw (specified in the [] token for 
 * specifier P in WRITE). */
void doPage(int ch) {
	while (channel[ch].pageL<=PAGEHEIGHT) {
		fputc('\n',channel[ch].stream);
		channel[ch].pageL++;
	}
	channel[ch].pageL=1;
	channel[ch].pageN++;
	return;
}


/* emitChar()
 * put char c on the output channel stream identified by ch, checking also the 
 * counter and pages updating values */
void emitChar(int ch, int c) {
	if (c=='\n' || c=='\r') FCR(ch);
	else if (c=='\t') {
		fputc(c,channel[ch].stream);
		channel[ch].fcounter+=TABSTOP-(channel[ch].fcounter%TABSTOP);
		updateCounter(ch);
	}
	else if (c=='\b') {
		if (channel[ch].fcounter) {
			fputc(c,channel[ch].stream);
			channel[ch].fcounter--;
		}
	}
	else if (c=='\f') {
		doPage(ch);
	}
	else if (c=='\v') {
		fputc(c,channel[ch].stream);
		channel[ch].pageL++;
		updatePage(ch);
	}
	else {
		fputc(c,channel[ch].stream);
		channel[ch].fcounter++;
		updateCounter(ch);
	}
}


/*****************************************
 *             ASSIGNMENTS               *
 *****************************************/

/* assign()
 * The assign procedure
 * perform the assignment; checks if iterate for assignments in series; set up
 * for embedded assignments (performed in S() and SS())
 * return var number if assignment went OK, false otherwise (currently unused)
 * Ref. AA-O196C-TK - 6.2 "Assignments"
 * Ref. AA-O196C-TK - 6.3 "Multiple Assignment" 
 * System function */
int assign(void) {
	int var=0, isArr=0, val[MAXDIM+1], isOwn=0;
	int ind=0, udim=1;
	char *p1;

	reload();
	var=findDEC(p);
	p+=_len;
	reload();
	/* assign to variables */
	if (var) {
		int ttype, subscript, thislen, isRef=FALSE;
		char *pp;
		pp=p;
		/* case of referenced variable which was passed an
		 * expression, which cannot be reassigned */
		if (vn[var].refcode==2*DECADD) pe(88,l);
		/* case of referenced array element */
		if (vn[var].refcode<-DECADD) {
			p=vn[var].valdata;
			while (*p && *p!='[') p++;
			/* reassign p for reading array values */
			var=vn[var].ref;
			isArr=TRUE;
			isOwn=FALSE;
			isRef=TRUE;
		}
		/* case of referenced variable/array */
		else if (vn[var].refcode<0) {

			var=vn[var].ref;
			isArr=vn[var].isArr;
			isOwn=vn[var].isOwn;
		}
		else {
			isArr=vn[vn[var].ref].isArr;
			isOwn=vn[vn[var].ref].isOwn;
		}
		ttype=vn[vn[var].ref].type;
		subscript=_type;
		thislen=vn[vn[var].ref].strmem;
		/* get string byte subscript index */
		if (subscript==CHAR) {
			char *pb;
			p++; // skip dot
			if (*p=='[') {
				p++; // skip [
				pb=p;
				ind=(int)S();
				if (*p==']') p++; // skip ]
				else pe(2,l);
				if (ind<1 || ind>thislen) {
				       	p=pb; // for perror_
					pe(57,l);
				}
			}
			else pe(2,l);
			ttype=INTEGERTYPEBOX;
		}
		/* get array indices */
		else if (isArr && *p=='[') {
			p++; // skip [
			val[udim]=(int)S();
			while (*p==',' && udim<MAXDIM) {
				p++;
				udim++;
				val[udim]=(int)S();
			}
			if (udim>=MAXDIM) pe(7,l);
			if (*p==']') p++;
			else pe(2,l);
		}
		/* restore p in case of array referencing */
		if (isRef) p=pp;
		reload();
		/* user value for variables */
		if (*p==PU) {
			/* the first part takes the value to instantiate 
			 * but does not instantiate */
			char sop[COMPSTR];
			double nop;
			p++; // skip @ (:=)
			reload();
			p1=p;
			/* NEWSTRING string function 
			 * Ref. AA-O196C-TK - 13.8.3 "Newstring"*/
			if (!strncasecmp_(p,"NEWSTRING(",10) &&\
					ttype==STRINGTYPEBOX)  {
				int i, len=0, sp=vn[vn[var].ref].strmem;
				char b3[sp];
				p+=10;
				/* make a copy of the original string */
				strncpy_(b3,PS[vn[var].ref],sp);
				/* get size and intantiate */
				i=(int)S();
				if (PS[vn[var].ref]) free(PS[vn[var].ref]);
				PS[vn[var].ref]=salloc(i+1);
				if (!PS[vn[var].ref]) pe(6,l);
				vn[vn[var].ref].strmem=i+1;
				/* discard second item */
				if (*p==',') {
					p++;
					S();
				}
				len=vn[vn[var].ref].strmem;
				/* erase memory */
				for (i=0;i<=len;i++) *(PS[vn[var].ref]+i)='\0';
				if (*p!=')') pe(2,l);
				else p++;
				/* recopy original data, at least what fits */
				strncpy_(sop,b3,len);
			}
			/* string variables value */
			else if (ttype==STRINGTYPEBOX) {
				strncpy_(sop,SS(),COMPSTR);
			}
			/* numeric variables value: consume it */
			else if (ttype<STRINGTYPEBOX) {
				nop=S();
			}
			/* label variables value */
			else if (ttype==LABELTYPEBOX) {
				int count=0;
				if (*p=='\"') p++;
				count=getLabelIndex();
				if (*p=='\"') p++;
				if (!count) pe(11,l);
				else if (count<0) pe(-count,l);
				else P[vn[var].ref]=count;
			}
			reload();	
			/* the expression is over, so now INSTANTIATE
			 * (also for embedded assignments) 
			 * Ref. AA-O196C-TK - 6.3 "Multiple Assignment" */
			if (*p!=PU || (*p==')' && isInn)) {
				isInn=FALSE;
				/* byte subscripts */
				if (subscript==CHAR) {
					PS[vn[var].ref][ind-1]=(char)nop;
				}
				/* string variables */
				else if (ttype==STRINGTYPEBOX && !isArr) {
					strncpy_(PS[vn[var].ref],sop,thislen);
					strncpy_(sres,sop,thislen);
				}
				/* string arrays */
				else if (ttype==STRINGTYPEBOX && isArr) {
					setArrayStr(vn[var].ref,udim,val,sop);
					strncpy_(sres,sop,COMPSTR);
				}
				/* real variables */
				else if (ttype<INTEGERTYPEBOX && !isArr) {
					P[vn[var].ref]=nop;
					nres=nop;
				}
				/* real arrays */
				else if (ttype<INTEGERTYPEBOX && isArr) {
					setArrayVal(vn[var].ref,udim,val,nop);
					nres=nop;
				}
				/* Boolean variables */
				else if (ttype==BOOLEANTYPEBOX && !isArr) {
					P[vn[var].ref]=(int)nop;
					nres=(int)nop;
				}
				/* Boolean arrays */
				else if (ttype==BOOLEANTYPEBOX && isArr) {
					nres=(int)nop;
					setArrayVal(vn[var].ref,udim,val,nres);
				}
				/* Ref. AA-O196C-TK - 6.2 "If a real or long 
				 * real value is assigned to an integer type 
				 * variable, a rounding process occurs.
				 * I := X
				 * results in an integral value equal to 
				 * ENTIER(X + 0.5) being assigned to I.*/
				/* integer variables */
				else if (ttype==INTEGERTYPEBOX && !isArr) {
					nres=(int)(nop+sign(nop)*0.5);
					P[vn[var].ref]=nres;
				}
				/* integer arrays */
				else if (ttype==INTEGERTYPEBOX && isArr) {
					nres=(int)(nop+sign(nop)*0.5);
					setArrayVal(vn[var].ref,udim,val,nres);
				}
			}
			/* it is a multi-assignment, so iterate and then
			 * assign returned values 
			 * Ref. AA-O196C-TK - 6.3 "Multiple Assignments" */
			else if (*p==PU) {
				p=p1;
				assign();
				/* byte subscripts */
				if (subscript==CHAR) {
					*(PS[vn[var].ref]+ind-1)=(char)nres;
				}
				/* string variables */
				else if (ttype==STRINGTYPEBOX && !isArr) {
					strncpy_(PS[vn[var].ref],sres,thislen);
				}
				/* string arrays */
				else if (ttype==STRINGTYPEBOX && isArr) {
					setArrayStr(vn[var].ref,udim,val,sres);
				}
				/* real variables */
				else if (ttype<INTEGERTYPEBOX && !isArr) {
					P[vn[vn[var].ref].ref]=nres;
				}
				/* real arrays */
				else if (ttype<INTEGERTYPEBOX && isArr) {
					setArrayVal(vn[var].ref,udim,val,nres);
				}
				/* Boolean variables */
				else if (ttype==BOOLEANTYPEBOX && !isArr) {
					P[vn[var].ref]=(int)nres;
				}
				/* Boolean arrays */
				else if (ttype==BOOLEANTYPEBOX && isArr) {
					setArrayVal(vn[var].ref,udim,val,(int)nres);
				}
				/* Ref. AA-O196C-TK - 6.2 "If a real or long 
				 * real value is assigned to an integer type 
				 * variable, a rounding process occurs.
				 * I := X
				 * results in an integral value equal to 
				 * ENTIER(X + 0.5) being assigned to I.*/
				/* integer variables */
				else if (ttype==INTEGERTYPEBOX) {
					P[vn[var].ref]=nres;
				}
				/* integer arrays */
				else if (ttype==INTEGERTYPEBOX && isArr) {
					setArrayVal(vn[var].ref,udim,val,nres);
				}
			}
			/* treat OWN elements (variables and arrays) */
			if (isOwn) {
				int i=1;
				while (i<=ghostCount) {
				   /* if it's an OWN variable, assign the ghost 
				    * using the reference code in isOwn */
				   if (own[i].code==isOwn && !isArr) {
				      if (ttype<STRINGTYPEBOX)
				         own[i].num=nres;
				      else 
				         strncpy_(own[i].str,sres,COMPSTR);
				      break;
				   }
				   /* if it's an OWN array, assign the ghost 
				    * using the reference code in isOwn 
				    * and copying the memory portion */
				   else if (own[i].code==isOwn && isArr) {
				      if (ttype<STRINGTYPEBOX) {
				         memcpy_(own[i].numarr,dim[var].elem,vn[var].size);
				      }
				      else {
				         memcpy_(own[i].strarr,dimS[var].elem,vn[var].size);
				      }
				      break;
				   }
				   i++;
				}
			}
			
			/* treat assignment of PROCEDURE-pseudo-variables */
			if (isProcTime && !strcmp(vn[var].name,Procp[isProcTime].name)) {
				if (Procp[isProcTime].type==LABELTYPEBOX)
					Procp[isProcTime].numres=P[var];
				else if (Procp[isProcTime].type<STRINGTYPEBOX)
					Procp[isProcTime].numres=nres;
				else strncpy_(Procp[isProcTime].strres,sres,COMPSTR);
			}
		}
		else pe(55,l);
	}
	/* if there is no variable name it's not an assignment */
	else pe(55,l);


	/* debug  for fictitious variable */
	if (isDebug && vn[var].isFict) {
		showDEC(var, "Assigned fictitious variable ");
	}

	/* return TRUE, returning the variable code */
	return var;
}


/*****************************************
 *             MATH & STRINGS            *
 *****************************************/

/* setAddr()
 * calculate the numerical correspondence of any string label 
 * in chunks of 8 characters*/
int setAddr(char *lp, char **rp) {
	int val=0, c=0, i=0;
	while (islabelnamechar(lp) && i<NAMEDIM) {
		c=0;
		while (islabelnamechar(lp) && c<8) {
			if (islabelnamechar(lp) && *lp!='.')
				val+=pow(10,c++)*(toupper(*lp++)-'A'+1);
			else lp++;
			i++;
		}
	}
	if (**rp) *rp=lp;
	return val;
}


/* check(string1,string2)
 * check if sb syntax is correct against sc; if it is, put a space in the 
 * position found, so that a subsequent parsing by S() may stop without 
 * issuing an error message from getFunc() and return true; 
 * if syntax it's not correct, return false.
 * This is done in statements with some core-words, like IF <Test> THEN
 * where the parsing of <Test> must stop just before THEN; check() inserts 
 * a space in place of the T of THEN, causing the parser to stop there */
int check(char *sb, char *sc){
	char *qq=strcasestr_oq(sb,sc);
	/* put a space to stop S() parsing */
	if (qq) { 
		*qq=' '; 
		return TRUE; 
	}
	return FALSE;
}


/* Note
 * The following strncpy_() and strncat_() are my own personal versions for 
 * strcpy/strncpy and strcat/strncat respectively.
 * I must use mine because strncpy/strncat standard functions in C (gcc) have 
 * erratic behaviors within different operating systems like Linux-Suse, 
 * OpenBSD and Mac OS X 10.4 (the one I use).
 * Since many reported strange things happening because of strncpy/strncat
 * (from error in compilation to completely wrong string functions output), I 
 * made up my mind and built these substitutes for both. */

/* strncpy_()
 * dst: the destination string (which must have its proper dimension)
 * src: the source to be copied onto dst
 * dw:  the destination width, the unity used to declare dst (for the sake
 * of precision, if dst is declared as 10 characters wide, it means that text
 * goes from pos. 0 to pos 8, and in pos. 9 is the last terminator; thus a width
 * equal to 10 means 9 characters at most for the string; dw must be put equal 
 * to the width, in this example 10; of course, any lower value is also good.)
 * The process will take care of never going past the limit of dst; 
 * System function replacing strcpy/strncpy */
char *strncpy_(char *dst, char *src, int dw) {
	char *dp=dst;
	/* check length of copy and source */
	if (dw<=1) *dst='\0';
	else {
		/* erase destination */
		*dst='\0';
		/* copy src onto dst but not past the end of dst */
		while ((int)(dp-dst)<(dw-1) && *src) *dp++=*src++;
		*dp++='\0';
		/* fill the rest with nulls */
		while ((int)(dp-dst)<(dw)) *dp++='\0';
	}
	/* Note: return the same value returned by strcpy/strncpy */
	return dst;
}


/* chrcat_()
 * str: the string to be appended
 * chr: the character to append
 * NOTE: this function does not control if there is space in str, so use
 * it with care and only before checking that adding a character is legal
 * System function for HEAD/TAIL/TAKE/DROP */
char *chrcat_(char *str, char chr) {
	char *bs=str;
	while (*bs) bs++;
	*bs++=chr;
	*bs='\0';
	return str;
}


/* strncat_()
 * dst: the destination string (which must have its proper dimension)
 * src: the source to be copied onto dst
 * dw:  the destination width, the unity used to declare dst (for the sake
 * of precision, if dst is declared as 10 characters wide, it means that text
 * goes from pos. 0 to pos 8, and in pos. 9 is the last terminator; thus width
 * equal to 10 means 9 characters at most for the string; dw must be put equal 
 * to the width, in this example 10; of course, any lower value is good.)
 * The process will take care of never going past the limit of dst.
 * If dst is found not initialized  (that is nowhere the terminator is found) 
 * it is initialized as void.
 * System function replacing strcat/strncat */
char *strncat_(char *dst, char *src, int dw) {
	char *dp=dst;
	/* search for current terminator */
	while (*dp && (int)(dp-dst)<dw) dp++;
	/* terminator not found: initialize destination */
	if ((int)(dp-dst)>=dw) dp=dst, *dp='\0';
	/* copy src onto dst but not past the end of dst */
	while ((int)(dp-dst)<(dw-1) && *src) *dp++=*src++;
	*dp='\0';
	/* Note: return the same value returned by strcat/strncat */
	return dst;
}


/* Note
 * The following sprintw() is my own personal version wrapper for snprintf(), 
 * which appear to behave erratically on my system (is does not work with 
 * function return values, but works with real arrays) and probably the same 
 * (with inverse properties) on other systems.
 * This wrapper assures that only physical arrays are treated by sprintw() */

/* sprintw() 
 * the snprintf wrapper 
 * dest is the destination string,
 * len is the string width (including the terminator character)
 * format is the formatting string (with the same printf rules)
 * '...' is the printing string argument (it may be a function return value) 
 * return the number of characters written in the host string 
 * System function replacing sprintf/snprintf */
int sprintw(char *dest, int len, char *format, ...) {
	va_list ap;
	char base[COMPSTR];
	va_start(ap,format);
	vsnprintf(base,COMPSTR,format,ap);
	snprintf(dest,len,"%s",base);
	va_end(ap);
	return (int)strlen(base);
}


/* Note
 * The following strncasecmp_() is my own personal version of strncasecmp.
 * I must use mine because strncasecmp seems to cause some problems if taxi is
 * compiled under openSuse with gcc and or Linux/openBSD with gcc.
 * Since many reported errors in compiling when the standard strncasecmp is
 * used, I made up my mind and built this substitute. */

/* strncasecmp_()
 * the string compare function
 * dst and src are the strings to be compared;
 * N is the number of characters to be checked against;
 * return the flag for positive comparison result, 0 for match, -1 for failure.
 * Note: return values mimic the original return values, with the difference
 * that I don't check here for dst being greater or less than src, and in case
 * they differ, -1 is returned anyway; this is done for speed issues. */
int strncasecmp_(char *dst, char *src, int N) {
	if (N<=0) return -1;
	while (N && *dst && *src && toupper(*dst)==toupper(*src)) 
		src++, dst++, N--;
	if (!N) return 0;
	return -1;
}


/* Note
 * The following memcpy_() is my own personal version of memcpy and memmove.
 * I must use mine because memcpy seems to be sometimes rejected by openBSD.
 * Since many reported errors in compiling if the standard memcpy/memmove is
 * used, I made up my mind and built this substitute. */

/* memcpy_()
 * check if any of arrays are null, or if size is zero/negative; 
 * if not, copy a portion of memory from src to dst, using memmove (that takes 
 * care of overlapping), for the number of characters specified in size */
void memcpy_(void *dst, void *src, int size) {
	if (src==NIL || dst==NIL || size<=0) return;
	memmove(dst,src,size);
}


/* Note
 * The following strcasestr_() is my own personal version of strcasestr().
 * I must use mine because strcasestr() in C has someway an erratic behavior
 * with Linux.
 * Since many reported errors in compiling if the standard strcasestr() is
 * used, I made up my mind and built this substitute. */

/* strcasestr_()
 * find the first occurrence of the substring 'little' in string 'big', 
 * ignoring the case of both arguments.
 * The terminating null bytes (i.e. '\0') are not compared. 
 * If 'little' is null, return NIL; if the substring is found, return the 
 * updated address in 'big', otherwise return NIL. */
char *strcasestr_(char *big, char *little) {
	int l=(int)strlen(little);
	if (!l) return NIL;
	while (*big) {
		if (!strncasecmp_(big,little,l)) return big;
		big++;
	}
	return NIL;
}


/* strcasestr_oq()
 * find the first occurrence of the substring string 'pr' into string 'pl'
 * this function has the same usage of strcasestr_(), but it works only if 
 * outside of a string, in ALGOL 60 normally surrounded by double quotes or
 * two instances of single quotes.
 * (strcasestr_oq = strcasestr_ Out of Quotes) */
char *strcasestr_oq(char *pl, char *pr) {
	int valid=1, l=(int)strlen(pr);
	if (!l) return pl;
	while (*pl) {
		if (*pl=='\"') valid++;
		if (*pl=='\'' && *(pl+1)=='\'') valid++;
		if (!strncasecmp_(pl,pr,l) && (valid & 1)) return pl;
		pl++;
	}
	return NIL;
}


/* findLabel()
 * find the first occurrence of the substring ":" into string 'pl'
 * this function has the same usage of strcasestr_(), but it works only if 
 * outside of a string, in ALGOL 60 normally surrounded by double quotes and
 * only out of square brackets (strcasestr_oq = strcasestr_ Out of Quotes),
 * discarding also the case of := (assignment) and :( (alternate comment) */
char *findLabel(char *pl) {
	int beg=0;
	while (*pl) {
		if (*pl=='\"' && *(pl+1)=='\"' && beg) pl++;
		else if (*pl==':' && *(pl+1)=='=') pl++;
		else if (*pl==':' && *(pl+1)=='(') pl++;
		else if (*pl=='\"' && !beg) beg++;
		else if (*pl=='\"' && beg) beg--;
		else if (*pl=='[') beg++;
		else if (*pl==']') beg--;
		if (*pl==':' && !beg) {
			return pl;
		}
		pl++;
	}
	return NIL;
}


/* sEND()
 * check if p points to END and not to something like THEN D... 
 * system function for doBegin() and preParse() */
int sEND(char *p) {
	return (!strncasecmp_(p,"END",3) && toupper(*(p-1))!='H');
}


/******************************************
 *                 PARSING                *
 ******************************************/

/* preParse()
 * A fundamental subroutine to be run BEFORE each process.
 * Scan source in core memory and executes setting addresses for all statements
 * that requires jumps and preprocesses PRECEDUREs.
 * m: the array of the string pointers containing the program lines to be
 * executed.
 * System basic function */
void preParse(char **m, int start, int end) {
	int l=start-1, evalProc=FALSE, proclen=0, procpos=0, k,y,wq=FALSE;
	char *q, *q2;

	/* scan source */
	do {

	    l++;
	    if (l>end) break;
	    if (!m[l]) continue;
	    q=m[l];

	    while (*q) {
		while (isblank(*q)) q++;
		/* skip strings */
		if (*q=='\"') {
			q++; // skip "
			while (*q && *q!='\"') q++;
			if (*q=='\"') q++;
		}
		else if (*q=='\'' && *(q+1)=='\'') {
			q+=2; // skip ' and '
			while (*q && !(*q=='\'' && *(q+1)=='\'')) q++;
			if (*q=='\'' && *(q+1)=='\'') q+=2;
		}
		/* last END. stops parsing
		 * Note: comments after END were removed 
		 * in storeSingleLineToCore() */ 
		else if ((!strncasecmp_(q,"END.",4))) {
			l=end;
			break;
		}
		/* IF-THEN-ELSE position search */ 
		else if ((!strncasecmp_(q,"IF",2))) {
			/* IF position is certain */
			int bt=l,be, beg=0, flip=0, flipd=0;
			int isThereElse;
			isThereElse=FALSE;
			ifTOS++;
			if (ifTOS>=LIMIT) pe(7,l);
			Ifp[ifTOS].condL=Ifp[ifTOS].condP=-1;
			Ifp[ifTOS].thenL=Ifp[ifTOS].thenP=-1;
			Ifp[ifTOS].elseL=Ifp[ifTOS].elseP=-1;
			Ifp[ifTOS].termL=Ifp[ifTOS].termP=-1;
			q+=1; // skip IF
			q2=q+1;
			/* refill after IF*/
			if (!*q2 && bt<end) {
				bt++;
				while (!m[bt] && bt<end) bt++;
				q2=m[bt];
			}
			if (bt==end) break;
			Ifp[ifTOS].condL=bt;
			Ifp[ifTOS].condP=(int)(q2-m[bt]);
			/* look for next THEN */
			beg=flip=0;
			while(TRUE) {
				if (!*q2 && bt<end) {
					bt++;
					while (!m[bt] && bt<end) bt++;
					q2=m[bt];
				}
				if (bt==end) break;
				if (*q2=='\"' && !flip) flip++;
				else if (*q2=='\"' && flip) flip--;
				else if (isdc(q2) && !flipd) q2++, flipd++;
				else if (isdc(q2) && flipd) q2++, flipd--;
				else if (!strncasecmp_(q2,"THEN",4) && !flip && ! flipd && !beg){ 
					break;
				}
				else if (!strncasecmp_(q2,"IF",2) && !flip && !flipd) 
					q2++, beg++;
				else if (!strncasecmp_(q2,"THEN",2) && !flip && !flipd && beg) 
					q2+=3, beg--;
				q2++;
			}
			q2+=4; // skip THEN
			if (!*q2 && bt<end) {
				bt++;
				while (!m[bt] && bt<end) bt++;
				q2=m[bt];
			}
			if (bt==end) break;
			Ifp[ifTOS].thenL=bt;
			Ifp[ifTOS].thenP=(int)(q2-m[bt]);
			/* look for next ELSE or terminator */
			be=bt; beg=flip=flipd=0;
			while(TRUE) {
				if (!*q2 && be<end) {
					be++;
					while (!m[be] && be<end) be++;
					q2=m[be];
				}
				if (be==end) break;
				if (*q2=='\"' && !flip) flip++;
				else if (*q2=='\"' && flip) flip--;
				else if (isdc(q2) && !flipd) q2++, flipd++;
				else if (isdc(q2) && flipd) q2++, flipd--;
				else if (!strncasecmp_(q2,"BEGIN",5) && !flip && !flipd) 
					q2+=4, beg++;
				else if (*q2==';' && !flip && !flipd && !beg) 
					break;
				else if (!strncasecmp_(q2,"ELSE",4) && !beg && !flip && !flipd) {
					isThereElse=TRUE;
					break;
				}
				else if (sEND(q2) && !beg && !flip && !flipd) {
					q2+=3;
					if (*q2 && *q2!=';' && *q2!='.') {
						continue;
					}
					else break;
				}
				else if (sEND(q2) && beg && !flip && !flipd) 
					q2+=2, beg--;
				q2++;
			}
			/* ELSE found */
			if (isThereElse) {
				q2+=4; // skip ELSE
				if (!*q2 && be < end) {
					be++;
					while (!m[be] && be<end) be++;
					q2=m[be];
				}
				if (be==end) break;
				Ifp[ifTOS].elseL=be;
				Ifp[ifTOS].elseP=(int)(q2-m[be]);
			}
			beg=flip=0;
			/* look for terminator */
			while(*q2) {
				if (!*q2 && be<end) {
					be++;
					while (!m[be] && be<end) be++;
					q2=m[be];
				}
				if (be==end) break;
				if (*q2=='\"' && !flip) flip++;
				else if (*q2=='\"' && flip) flip--;
				else if (isdc(q2) && !flip) q2++, flip++;
				else if (isdc(q2) && flip) q2++, flip--;
				else if (!strncasecmp_(q2,"BEGIN",5) && !flip) 
					q2+=4, beg++;
				else if (sEND(q2) && !beg && !flip) 
					break;
				else if (sEND(q2) && beg && !flip) 
					q2+=2, beg--;
				else if (*q2==';' && !beg && !flip) break;
				q2++;
			}
			Ifp[ifTOS].termL=be;
			Ifp[ifTOS].termP=(int)(q2-m[be]);
			if (isVerboseDebug && isVerbose) {
				fprintf(stdout,"%s",INVCOLOR);
				fprintf(stdout,"\nIF [#%d] at line %d has:\n",ifTOS,l);		
				fprintf(stdout,"cond data: l=%d, p=%d\n",Ifp[ifTOS].condL,Ifp[ifTOS].condP);
				fprintf(stdout,"THEN data: l=%d, p=%d\n",Ifp[ifTOS].thenL,Ifp[ifTOS].thenP);
				fprintf(stdout,"ELSE data: l=%d, p=%d\n",Ifp[ifTOS].elseL,Ifp[ifTOS].elseP);
				fprintf(stdout,"term data: l=%d, p=%d\n",Ifp[ifTOS].termL,Ifp[ifTOS].termP);
				fprintf(stdout,"%s",RESETCOLOR);
			}
		}
		/* FOR-DO position search (skip FORWARD) */ 
		if ((!strncasecmp_(q,"FOR",3)) && strncasecmp_(q,"FORWARD",7)) {
			/* FOR position is certain */
			int bf=l, beg=0, flip=0, flipd=0, isForEnd=FALSE;
			forTOS++;
			if (forTOS>=LIMIT) pe(7,l);
			Forp[forTOS].varpL=Forp[forTOS].varpP=-1;
			Forp[forTOS].doexL=Forp[forTOS].doexP=-1;
			Forp[forTOS].termL=Forp[forTOS].termP=-1;
			q+=2; // skip FOR
			q2=q+1; 
			if (!*q2) {
				bf++;
				while (!m[bf]) bf++;
				if (bf>end) isForEnd=TRUE;
				q2=m[bf];
			}
			if (isForEnd) continue;
			isForEnd=FALSE;
			Forp[forTOS].varpL=bf;
			Forp[forTOS].varpP=(int)(q2-m[bf]);
			/* look for DO */
			while(TRUE) {
				if (!*q2) {
					bf++;
					while (!m[bf]) bf++;
					if (bf>end) break;
					q2=m[bf];
				}
				if (!strncasecmp_(q2,"DO",2)) break;
				q2++;
			}
			if (bf>end) pe(3,l);
			else if (isForEnd) continue;
			isForEnd=FALSE;
			q2+=2; // skip DO
			Forp[forTOS].doexL=bf;
			Forp[forTOS].doexP=(int)(q2-m[bf]);
			/* look for terminator */
			while(TRUE) {
				if (!*q2) {
					bf++;
					while (!m[bf]) bf++;
					if (bf>end) break;
					q2=m[bf];
				}
				if (bf>end) {isForEnd=TRUE; break;}
				else if (*q2=='\"' && !flip) flip++;
				else if (*q2=='\"' && flip) flip--;
				else if (isdc(q2) && !flipd) q2++, flipd++;
				else if (isdc(q2) && flipd) q2++, flipd--;
				else if (!strncasecmp_(q2,"BEGIN",5) && !flip && !flipd) 
					q2+=4, beg++;
				else if (sEND(q2) &&\
						!beg && !flip && !flipd) 
					break;
				else if (sEND(q2) &&\
					       beg && !flip && !flipd) 
					q2+=2, beg--;
				else if (*q2==';' && !beg && !flip && !flipd) break;
				q2++;
			}
			if (isForEnd) continue;
			isForEnd=FALSE;
			if (!*q2) {
				bf++;
				while (!m[bf]) bf++;
				if (bf>end) isForEnd=TRUE;
				q2=m[bf];
			}
			if (isForEnd) continue;
			Forp[forTOS].termL=bf;
			Forp[forTOS].termP=(int)(q2-m[bf]);
			if (isVerboseDebug && isVerbose) {
				fprintf(stdout,"%s",INVCOLOR);
				fprintf(stdout,"\nFOR [#%d] at line %d has:\n",forTOS, l);		
				fprintf(stdout,"var data : l=%d, p=%d\n",Forp[forTOS].varpL,Forp[forTOS].varpP);
				fprintf(stdout,"term data: l=%d, p=%d\n",Forp[forTOS].termL,Forp[forTOS].termP);
				fprintf(stdout,"%s",RESETCOLOR);
			}
		}
		/* BEGIN-END position search 
		 * Note: here I don't take any countermeasure for letting
		 * the word BEGIN to be part of an identifier (like for IF,
		 * FOR or WHILE); this is because BEGIN and END may be misread
		 * by the parser and not correctly aligned. Thus I pose the
		 * rule to not use BEGIN in identifiers and strongly suggest
		 * not to use END (its counterpart) in identifiers as well. */ 
		else if ((!strncasecmp_(q,"BEGIN",5))) {
			/* BEGIN position is certain */
			int bs=l, beg=0, flip=0, flipd=0;
			beginTOS++;
			if (beginTOS>=LIMIT) pe(7,l);
			q+=4; // skip BEGIN
			q2=q+1; 
			Beginp[beginTOS].varpL=bs;
			Beginp[beginTOS].varpP=(int)(q2-m[bs]);
			if (!*q2) {
				bs++;
				while (!m[bs]) bs++;
				if (bs>end) pe(23,l);
				q2=m[bs];
			}
			/* look for terminator */
			while(TRUE) {
				if (!*q2) {
					bs++;
					while (!m[bs]) bs++;
					if (bs>end) break;
					q2=m[bs];
				}
				if (bs>end) pe(23,l);
				else if (*q2=='\"' && !flip) flip++;
				else if (*q2=='\"' && flip) flip--;
				else if (isdc(q2) && !flipd) q2++, flipd++;
				else if (isdc(q2) && flipd) q2++, flipd--;
				else if (!strncasecmp_(q2,"BEGIN",5) && !flip && !flipd) 
					q2+=4, beg++;
				else if (sEND(q2) && beg && !flip && !flipd) 
					q2+=2, beg--;
				else if (sEND(q2) && !beg && !flip && !flipd) 
					break;
				q2++;
			}
			if (!*q2) {
				bs++;
				while (!m[bs]) bs++;
				if (bs>end) pe(23,l);
				q2=m[bs];
			}
			Beginp[beginTOS].termL=bs;
			Beginp[beginTOS].termP=(int)(q2-m[bs]);
			if (isVerboseDebug && isVerbose) {
				fprintf(stdout,"%s",INVCOLOR);
				fprintf(stdout,"\nBEGIN [#%d] at line %d has:\n",beginTOS,l);		
				fprintf(stdout,"begin data: l=%d, p=%d\n",Beginp[beginTOS].varpL,Beginp[beginTOS].varpP);
				fprintf(stdout,"end data  : l=%d, p=%d\n",Beginp[beginTOS].termL,Beginp[beginTOS].termP);
				fprintf(stdout,"%s",RESETCOLOR);
			}
		}
		/* WHILE-DO position search */ 
		else if ((!strncasecmp_(q,"WHILE",5))) {
			/* WHILE position is certain */
			int wf=l, beg=0, flip=0, flipd=0, isWhileEnd=FALSE;
			whileTOS++;
			if (whileTOS>=LIMIT) pe(7,l);
			Whilep[whileTOS].varpL=Whilep[whileTOS].varpP=-1;
			Whilep[whileTOS].doexL=Whilep[whileTOS].doexP=-1;
			Whilep[whileTOS].termL=Whilep[whileTOS].termP=-1;
			q+=4; // skip WHILE
			q2=q+1; 
			if (!*q2) {
				wf++;
				while (!m[wf]) wf++;
				if (wf>end) isWhileEnd=TRUE;
				q2=m[wf];
			}
			if (isWhileEnd) continue;
			isWhileEnd=FALSE;
			Whilep[whileTOS].varpL=wf;
			Whilep[whileTOS].varpP=(int)(q2-m[wf]);
			/* look for DO */
			while(TRUE) {
				if (!*q2) {
					wf++;
					while (!m[wf]) wf++;
					if (wf>end) break;
					q2=m[wf];
				}
				if (!strncasecmp_(q2,"DO",2)) break;
				q2++;
			}
			if (wf>end) pe(3,l);
			else if (isWhileEnd) continue;
			isWhileEnd=FALSE;
			q2+=2; // skip DO
			Whilep[whileTOS].doexL=wf;
			Whilep[whileTOS].doexP=(int)(q2-m[wf]);
			/* look for terminator */
			while(TRUE) {
				if (!*q2) {
					wf++;
					while (!m[wf]) wf++;
					if (wf>end) break;
					q2=m[wf];
				}
				if (wf>end) {isWhileEnd=TRUE;break;}
				else if (*q2=='\"' && !flip) flip++;
				else if (*q2=='\"' && flip) flip--;
				else if (isdc(q2) && !flipd) q2++, flipd++;
				else if (isdc(q2) && flipd) q2++, flipd--;
				else if (!strncasecmp_(q2,"BEGIN",5) && !flip && !flipd) 
					q2+=4, beg++;
				else if (sEND(q2) &&\
						!beg && !flip && !flipd) 
					break;
				else if (sEND(q2) &&\
					       beg && !flip && ! flipd) 
					q2+=2, beg--;
				else if (*q2==';' && !beg && !flip && !flipd) 
					break;
				q2++;
			}
			if (isWhileEnd) continue;
			isWhileEnd=FALSE;
			if (!*q2) {
				wf++;
				while (!m[wf]) wf++;
				if (wf>end) isWhileEnd=FALSE;
				q2=m[wf];
			}
			if (isWhileEnd) continue;
			Whilep[whileTOS].termL=wf;
			Whilep[whileTOS].termP=(int)(q2-m[wf]);
			if (isVerboseDebug && isVerbose) {
				fprintf(stdout,"%s",INVCOLOR);
				fprintf(stdout,"\nWHILE [#%d] at line %d has:\n",whileTOS,l);		
				fprintf(stdout,"cond data: l=%d, p=%d\n",Whilep[whileTOS].varpL,Whilep[whileTOS].varpP);
				fprintf(stdout,"term data: l=%d, p=%d\n",Whilep[whileTOS].termL,Whilep[whileTOS].termP);
				fprintf(stdout,"%s",RESETCOLOR);
			}
		}
		/* PROCEDURE detection */
		else if ((!strncasecmp_(q,"INTEGERPROCEDURE",16))) {
			evalProc=INTEGERTYPEBOX;
			proclen=16;
		}
		else if ((!strncasecmp_(q,"REALPROCEDURE",13))) {
			evalProc=REALTYPEBOX;
			proclen=13;
		}
		else if ((!strncasecmp_(q,"LONGREALPROCEDURE",17))) {
			evalProc=LONGREALTYPEBOX;
			proclen=17;
		}
		else if ((!strncasecmp_(q,"BOOLEANPROCEDURE",16))) {
			evalProc=BOOLEANTYPEBOX;
			proclen=16;
		}
		else if ((!strncasecmp_(q,"STRINGPROCEDURE",15))) {
			evalProc=STRINGTYPEBOX;
			proclen=15;
		}
		else if ((!strncasecmp_(q,"LABELPROCEDURE",14))) {
			evalProc=LABELTYPEBOX;
			proclen=14;
		}
		else if ((!strncasecmp_(q,"PROCEDURE",9))) {
			evalProc=NOTYPEBOX;
			proclen=9;
		}

		/* PROCEDURE 
		 * Arguments, begin-end position and masking */ 
		if (evalProc) {
			int varcount=0, index=0;
			int bs=l, beg=0, flip=0, flipd=0, k, lb;
			int types[MAXDIM], modes[MAXDIM],e;
			int state[MAXDIM];
			char names[MAXDIM][COMPSTR], *nm;
			char pname[COMPSTR], *pn=pname;

			for (e=0;e<MAXDIM;e++) 
				types[e]=modes[e]=state[e]=0;

			/* PROCEDURE starting position is certain */
			procpos=(int)(q-m[l]);
			procTOS++;
			if (procTOS>=LIMIT) pe(7,l);
			Procp[procTOS].procL=l;
			Procp[procTOS].procP=procpos;
			q+=proclen; // skip PROCEDURE
			q2=q;
			/* get sub name */
			if (isbeginnamechar(q2)) 
				while (isnamechar(q2)) *pn++=*q2++;
			else pe(2,l);
			*pn='\0';
			strncpy_(Procp[procTOS].name,pname,NAMEDIM);
			Procp[procTOS].type=evalProc;
			Procp[procTOS].number=procTOS;
			Procp[procTOS].procref=0; // by default
			/* there are arguments*/
			if (*q2=='(') {
				q2++;
				index=0;
				while (*q2!=')') {
					if (*q2==';') pe(2,l);
					index++;
					nm=names[index];
					while (isnamechar(q2)) *nm++=*q2++;
					*nm='\0';
					types[index]=REALTYPEBOX;
					modes[index]=BYREF;
					if (*q2==',') q2++;
					/* after the arguments list there 
					 * must be the terminator */
					else if (*q2==')' && !*(q2+1)) {
						pe(2,l);
					}
					/* enrichment 
					 * Ref. AA-196C-TK 11.9.2 */
					else if (*q2==')' && *(q2+1)!=';') {
						q2++; // skip )
					       	while (*q2!=':') q2++;
						q2++; // skip :
						if (*q2=='(') q2++;
						else pe(2,l);
					}
				}
				if (*q2==')') q2++;
				else pe(2,l);
				if (*q2==';') q2++;
				else pe(5,l);
			}
			/* establish true type and mode of argument variables 
			 * OUTER circle */
			if (index) while (TRUE) {
				int deftype, defmode, defstat;
				int evaluate=0;
				deftype=REALTYPEBOX;
				defmode=BYREF;
				defstat=ISVAR;
				/* reload */
				while (!*q2) {
					bs++;
					while (!m[bs]) bs++;
					if (bs>end) pe(23,l);
					q2=m[bs];
				}
				/* detect change of type/mode */
				if (!strncasecmp_(q2,"INTEGERARRAY",12)) {
					q2+=12;
					deftype=INTEGERTYPEBOX;
					defmode=0;
					defstat=ISARR;
				}
				else if (!strncasecmp_(q2,"INTEGERPROCEDURE",16)) {
					q2+=16;
					deftype=INTEGERTYPEBOX;
					defmode=0;
					defstat=ISPROC;
				}
				else if (!strncasecmp_(q2,"INTEGER",7)) {
					q2+=7;
					deftype=INTEGERTYPEBOX;
					defmode=0;
				}
				else if (!strncasecmp_(q2,"REALARRAY",9)) {
					q2+=9;
					deftype=REALTYPEBOX;
					defmode=0;
					defstat=ISARR;
				}
				else if (!strncasecmp_(q2,"ARRAY",5)) {
					q2+=5;
					deftype=REALTYPEBOX;
					defmode=0;
					defstat=ISARR;
				}
				else if (!strncasecmp_(q2,"REALPROCEDURE",13)) {
					q2+=13;
					deftype=REALTYPEBOX;
					defmode=0;
					defstat=ISPROC;
				}
				else if (!strncasecmp_(q2,"REAL",4)) {
					q2+=4;
					deftype=REALTYPEBOX;
					defmode=0;
				}
				else if (!strncasecmp_(q2,"LONGREALARRAY",13)) {
					q2+=13;
					deftype=LONGREALTYPEBOX;
					defmode=0;
					defstat=ISARR;
				}
				else if (!strncasecmp_(q2,"LONGREALPROCEDURE",17)) {
					q2+=17;
					deftype=LONGREALTYPEBOX;
					defmode=0;
					defstat=ISPROC;
				}
				else if (!strncasecmp_(q2,"LONGREAL",8)) {
					q2+=8;
					deftype=LONGREALTYPEBOX;
					defmode=0;
				}
				else if (!strncasecmp_(q2,"BOOLEANARRAY",12)) {
					q2+=12;
					deftype=BOOLEANTYPEBOX;
					defmode=0;
					defstat=ISARR;
				}
				else if (!strncasecmp_(q2,"BOOLEANPROCEDURE",16)) {
					q2+=16;
					deftype=BOOLEANTYPEBOX;
					defmode=0;
					defstat=ISPROC;
				}
				else if (!strncasecmp_(q2,"BOOLEAN",7)) {
					q2+=7;
					deftype=BOOLEANTYPEBOX;
					defmode=0;
				}
				else if (!strncasecmp_(q2,"STRINGARRAY",11)) {
					q2+=11;
					deftype=STRINGTYPEBOX;
					defmode=0;
					defstat=ISARR;
				}
				else if (!strncasecmp_(q2,"SWITCH",6)) {
					q2+=6;
					deftype=LABELTYPEBOX;
					defmode=0;
					defstat=ISARR;
				}
				else if (!strncasecmp_(q2,"STRINGPROCEDURE",15)) {
					q2+=15;
					deftype=STRINGTYPEBOX;
					defmode=0;
					defstat=ISPROC;
				}
				else if (!strncasecmp_(q2,"PROCEDURE",9)) {
					q2+=9;
					deftype=NOTYPEBOX;
					defmode=0;
					defstat=ISPROC;
				}
				else if (!strncasecmp_(q2,"STRING",6)) {
					q2+=6;
					deftype=STRINGTYPEBOX;
					defmode=0;
				}
				else if (!strncasecmp_(q2,"LABELPROCEDURE",14)){
					q2+=14;
					deftype=LABELTYPEBOX;
					defmode=0;
					defstat=ISPROC;
				}
				else if (!strncasecmp_(q2,"LABEL",5)) {
					q2+=5;
					deftype=LABELTYPEBOX;
					defmode=BYVAL;
				}
				else if (!strncasecmp_(q2,"VALUE",5)) {
					q2+=5;
					deftype=0;
					defmode=BYVAL;
					defstat=0;
				}
				else break;
				/* INNER circle */
				while (*q2 && *q2!=';') {
					char tname[COMPSTR], *tn;
					int i;
					tn=tname;
					while (isnamechar(q2)) *tn++=*q2++;
					*tn='\0';
					if (*q2) {
					/* search among arguments */
 					   for (i=index;i>0;i--) {
						if (!strcmp(tname,names[i])) {
						  if(deftype) types[i]=deftype;
						  if(defmode) modes[i]=defmode;
						  if(defstat) state[i]=defstat;
						  evaluate++;
						  break;
						}
					   }
					   if (i<1) pe(71,bs);
					   if (*q2==',') q2++;
					}
					/* reload */
					else {
						bs++;
						while (!m[bs]) bs++;
						if (bs>end) pe(23,bs);
						q2=m[bs];
					}
				}
				if (!evaluate) pe(72,bs);
				if (*q2==';') q2++;
				/* reload */
				if (!*q2) {
					bs++;
					while (!m[bs]) bs++;
					if (bs>end) pe(23,bs);
					q2=m[bs];
				}
			}
			for (k=index;k>0;k--) if (!state[k]) state[k]=ISVAR;
			Procp[procTOS].numvars=index;
			Procp[procTOS].procref=-1;
			/* look for proc start */
			if (*q2==';') q2++;
			/* reload */
			if (!*q2) {
				bs++;
				while (!m[bs]) bs++;
				if (bs>end) pe(23,bs);
				q2=m[bs];
			}
			Procp[procTOS].begnL=bs;
			Procp[procTOS].begnP=(int)(q2-m[bs]);;
			lb=l, l=bs; q=m[l]+Procp[procTOS].begnP;
			q--;
			/* look for terminator */
			while(TRUE) {
				if (!*q2) {
					bs++;
					while (!m[bs]) bs++;
					if (bs>end) break;
					q2=m[bs];
				}
				if (bs>end) pe(23,l);
				else if (*q2=='\"' && !flip) flip++;
				else if (*q2=='\"' && flip) flip--;
				else if (isdc(q2) && !flipd) q2++, flipd++;
				else if (isdc(q2) && flipd) q2++, flipd--;
				else if (!strncasecmp_(q2,"BEGIN",5) && !flip && !flipd) 
					q2+=4, beg++;
				else if (sEND(q2) && !beg && !flip && !flipd) 
					break;
				else if (sEND(q2) && beg && !flip && !flipd) 
					q2+=2, beg--;
				else if (*q2==';' && !beg && !flip && !flipd) 
					break;
				q2++;
			}
			if (!*q2) {
				bs++;
				while (!m[bs]) bs++;
				if (bs>end) pe(23,l);
				q2=m[bs];
			}
			Procp[procTOS].termL=bs;
			Procp[procTOS].termP=(int)(q2-m[bs]);
			/* check body evidence */
			if (Procp[procTOS].begnL==Procp[procTOS].termL &&\
				Procp[procTOS].begnP==Procp[procTOS].termP)
				pe(75,bs);
			/* build [].vars line */
			varcount=0;
			for (bs=1;bs<=index;bs++) {
			  nm=names[bs];
			  while (*nm) 
			  	Procp[procTOS].vars[++varcount]=*nm++;
			  Procp[procTOS].vars[++varcount]=-1; //end name
			  Procp[procTOS].vars[++varcount]=DECADD+types[bs];
			  Procp[procTOS].vars[++varcount]=DECADD+modes[bs];
			  Procp[procTOS].vars[++varcount]=DECADD+state[bs];
			  Procp[procTOS].vars[++varcount]=-2; //end var data  
			}
			/* print Debug line */
			if (isVerboseDebug) {
				showPROC(procTOS, lb);
			}
			evalProc=FALSE;
		}
	    q++;
	    }
	} while (TRUE);

	/* categorize procedures levels 
	 * Note: wq is used to print the title only before the first line */
	for (k=1;k<=procTOS;k++) {
		int tsl, tsp, tel, tep; // start end L,P of current base
		tsl=Procp[k].begnL;
		tsp=Procp[k].begnP;
		tel=Procp[k].termL;
		tep=Procp[k].termP;
		for (y=k+1;y<=procTOS;y++) {
			if (y==k) continue;
			if ((Procp[y].procL>tsl && Procp[y].termL<tel) ||\
				(Procp[y].procL==tsl && Procp[y].procP>tsp &&\
				Procp[y].termL==tel && Procp[y].termP<tep)) {
					Procp[y].procref=k;
				if (isVerboseDebug) {
					fprintf(stdout,"%s",INVCOLOR);
					if (!wq) fprintf(stdout,"%s","\nProcedures categorizing:\n");
					fprintf(stdout,"set procedure %s as subordinated to procedure %s\n",Procp[y].name,Procp[k].name);
					fprintf(stdout,"%s",RESETCOLOR);
					wq=TRUE;
				}
			}
			else Procp[y].procref=0;
		}
	}
}


/* isInnerAssign() 
 * check if the string in t is an assignment enclosed in parenthesis to 
 * perform first the assignment in parenthesis (inner assignment) and then
 * to assign the external left part.
 * System function for getVal() */
int isInnerAssign(char *t) {
	char *f=NULL,*m=NULL,*e=NULL;
	char yt[2];
	int v=0;
	if (*t!='(') return FALSE;
	f=t++; // jump to ( and skip it
	yt[0]=PU;
	yt[1]='\0';
	m=strcasestr_(t,yt);
	if (!m) return FALSE;
	t=m+1; // jump to © and skip it
	while (*t) {
		if (*t=='(') v++;
		else if (*t==')' && v) v--;
		else if (*t==')' && !v) {
			e=t-1;
			break;
		}
		t++;
	}
	if (f<m && m<e) return (int)(e-f+1);
	return FALSE;
}


/* SKIPPROCEDURE()
 * skips procedure body during runtime */
void SKIPPROCEDURE(void) {
	/* get position */
	int L=l, P=strlen(m[l])-strlen(p), i;
	/* look for data in Procp[] with the same position */
	LL=PL=0;
	for (i=1;i<=procTOS;i++) {
		/* jump to term position */
		if (L==Procp[i].procL && P==Procp[i].procP) {
			LL=Procp[i].termL;
			PL=Procp[i].termP;
			if (isDebug) {
				fprintf(stdout,"%s",INVCOLOR);
				fprintf(stdout,"Skipping procedure %s\n",Procp[i].name);
				fprintf(stdout,"%s",RESETCOLOR);
			}
			break;
		}
	}
	if (i>procTOS) pe(23,l);
	/* if LL is still empty, no data was found */
	if (!LL) pe(23,l);
}


/* parseComment()
 * parse text at current program pointer, assuming it is a comment, until
 * first ; and in case the current line comes to an end without a comment
 * end, check on next line, parsing it again. 
 * Used in algolstart() as a comment pre-BEGIN and in exec() for standard
 * COMMENT and ! procedures keyword. */
void parseComment(void) {
	while (*p && *p!=';') {
		p++;
		if (!*p) {
			l++;
			if (l>endCore) return;
			while ((!m[l] || !m[l][0])) l++;
			if (l>endCore) return;
			strncpy_(BASESTR,m[l],COMPSCR);
			p=BASESTR;
		}
	}
}


/* isIn()
 * check if a destination jump is beyond the scope of a BEGIN-END block
 * l = current line 
 * LL,PL = line and position in line
 * System procedure for doBegin() to check block position */
int isIn(int l, int LL, int PL) {
	/* tl,tp = line and position in line of BEGIN+5 */
	/* te,pe = line and position in line of END */
	int tl, tp, te, pe; 
	int locTOS=1;
	while (Beginp[locTOS].varpL!=l) locTOS++;
	tl=Beginp[locTOS].varpL;
	tp=Beginp[locTOS].varpP;
	te=Beginp[locTOS].termL;
	pe=Beginp[locTOS].termP;
	if ((LL>te) || (LL==te && PL>pe) || (LL<tl) || (LL==tl && PL<tp))
	       return FALSE;
	return TRUE;	
}


/* depured()
 * separate a statement string from the following statements, saving return
 * string in a global server-string
 * System procedure for doBegin() in case the debug line is to be printed */
char *depured(char *f) {
	char *o=dline, *t;
	/* return null on void line */
	if (!f || !*f) {
		*o='\0';
		return dline;
	}
	/* check for terminator */
	t=strcasestr_oq(f,";");
	/* return null only if argument is empty */
	if (!t && !strlen(f)) {
		*o='\0';
		return dline;
	}
	/* line is full but terminator absent */
	else t=f+strlen(f)-1;
	/* copy string */
	while (f<=t) *o++=*f++;
	*o='\0';
	return dline;
}


/* normal variables declarative procedures 
 * Ref. AA-0196C-TK 3.2 "Scalar declarations" */
void INTEGER(void) {
	isOwn=FALSE;
	typeBox=INTEGERTYPEBOX;
	declare();
}
void REAL(void) {
	isOwn=FALSE;
	typeBox=REALTYPEBOX;
	declare();
}
void LONGREAL(void) {
	isOwn=FALSE;
	typeBox=LONGREALTYPEBOX;
	declare();
}
void BOOLEAN(void) {
	isOwn=FALSE;
	typeBox=BOOLEANTYPEBOX;
	declare();
}
void STRING(void) {
	isOwn=FALSE;
	typeBox=STRINGTYPEBOX;
	declare();
}
void LABEL(void) {
	isOwn=FALSE;
	typeBox=LABELTYPEBOX;
	declare();
}


/* array variables declarative procedures 
 * Ref. AA-0196C-TK 9.2 "Array declarations" */
void INTEGERARRAY(void) {
	isOwn=FALSE;
	typeBox=INTEGERTYPEBOX;
	array();
	typeBox=0;
}
void REALARRAY(void) {
	isOwn=FALSE;
	typeBox=REALTYPEBOX;
	array();
	typeBox=0;
}
void LONGREALARRAY(void) {
	isOwn=FALSE;
	typeBox=LONGREALTYPEBOX;
	array();
	typeBox=0;
}
void BOOLEANARRAY(void) {
	isOwn=FALSE;
	typeBox=BOOLEANTYPEBOX;
	array();
	typeBox=0;
}
void STRINGARRAY(void) {
	isOwn=FALSE;
	typeBox=STRINGTYPEBOX;
	array();
	typeBox=0;
}


/* own (static) variables declarative procedures  
 * Ref. AA-0196C-TK 15 "Own variables" */
void OWNINTEGER(void) {
	isOwn=TRUE;
	typeBox=INTEGERTYPEBOX;
	declare();
	isOwn=FALSE;
}
void OWNREAL(void) {
	isOwn=TRUE;
	typeBox=REALTYPEBOX;
	declare();
	isOwn=FALSE;
}
void OWNLONGREAL(void) {
	isOwn=TRUE;
	typeBox=LONGREALTYPEBOX;
	declare();
	isOwn=FALSE;
}
void OWNBOOLEAN(void) {
	isOwn=TRUE;
	typeBox=BOOLEANTYPEBOX;
	declare();
	isOwn=FALSE;
}
void OWNSTRING(void) {
	isOwn=TRUE;
	typeBox=STRINGTYPEBOX;
	declare();
	isOwn=FALSE;
}
void OWNLABEL(void) {
	isOwn=TRUE;
	typeBox=LABELTYPEBOX;
	declare();
	isOwn=FALSE;
}


/* own (static) array variables declarative procedures 
 * Ref. AA-0196C-TK 15 "Own variables" */
void OWNINTEGERARRAY(void) {
	isOwn=TRUE;
	typeBox=INTEGERTYPEBOX;
	array();
	typeBox=0;
	isOwn=FALSE;
}
void OWNREALARRAY(void) {
	isOwn=TRUE;
	typeBox=REALTYPEBOX;
	array();
	typeBox=0;
	isOwn=FALSE;
}
void OWNLONGREALARRAY(void) {
	isOwn=TRUE;
	typeBox=LONGREALTYPEBOX;
	array();
	typeBox=0;
	isOwn=FALSE;
}
void OWNBOOLEANARRAY(void) {
	isOwn=TRUE;
	typeBox=BOOLEANTYPEBOX;
	array();
	typeBox=0;
	isOwn=FALSE;
}
void OWNSTRINGARRAY(void) {
	isOwn=TRUE;
	typeBox=STRINGTYPEBOX;
	array();
	typeBox=0;
	isOwn=FALSE;
}


/*****************************************
 *             ALGOL ENGINE              *
 *****************************************/

char *getType(int ty) {
	if (ty==BOOLEANTYPEBOX) return "Boolean";
	else if (ty==INTEGERTYPEBOX) return "integer";
	else if (ty==REALTYPEBOX) return "real";
	else if (ty==LONGREALTYPEBOX) return "long real";
	else if (ty==STRINGTYPEBOX) return "string";
	else if (ty==LABELTYPEBOX) return "label";
	return "unknown";
}


/* taxiDebugConsole()
 * the console that applies commands for the debugger 
 * actioned in doBegin() in case ONTRACE is met. 
 * System function for the debugger. */
void taxiDebugConsole(void) {
	char cc=1;
	struct termios thismodes, thissavemodes;
	/* set tcgetattr values backing up current state */
	tcgetattr(0, &thismodes);
	thissavemodes = thismodes;
	thismodes.c_lflag &= ~ICANON;
	thismodes.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &thismodes);
	cc = toupper(fgetc(stdin));
	tcsetattr(0, TCSANOW, &thissavemodes);
	switch (cc) {
		case 'H':
			fputc('\n',stdout);
			fprintf(stdout,"%s\n","C - continue in debug mode		G - continue out of debug mode");
			fprintf(stdout,"%s\n","H - print this help			P - dump procedure");
			fprintf(stdout,"%s\n","V - dump variable			X - stop and exit debug");
			fputc('\n',stdout);
			taxiDebugConsole();
			break;
		case 'X':
			fprintf(stdout,"\n%s\n","(terminated)");
			stopKey=FALSE;
			isDebug=FALSE;
			isQuit=TRUE;
			break;
		case 'C':
			stopKey=TRUE;
			break;
		case 'G':
			isDebug=FALSE;
			stopKey=TRUE;
			break;
		case 'P': 
			{
				int ns, type;
				char procname[COMPSTR], *lpr;
				fprintf(stdout,"\n>");
				strncpy_(procname,getString(CONSOLE),COMPSTR);
				ns=findPROC(procname,&lpr,&type);
				if (ns) showPROC(ns,0);
				else fprintf(stdout,"%s\n","Proedure not found or out of scope");
			}
			taxiDebugConsole();
			break;
		case 'V': 
			{
				int ns;
				char varname[COMPSTR];
				fprintf(stdout,"\n>");
				strncpy_(varname,getString(CONSOLE),COMPSTR);
				ns=findDEC(varname);
				if (ns) {
					char output[COMPSTR];
					sprintw(output,COMPSTR,"Level:%d  Name:",level(ns));
					showDEC(ns,output);
				}
				else fprintf(stdout,"%s\n","Variable not found or out of scope");

			}
			taxiDebugConsole();
			break;
	}
}


/* go()
 * the runner...
 * evaluates first if the expression at the program pointer is a procedure
 * and then if it's an assignment.
 * line is used only to print the error message */
void go(int line) {
	if (*p==';' || *p<0) return;
	if (exec());
	else if (assign());
	else pe(73,line);
	if (currentINPUT>0 && currentINPUT<=STREAMS && !checkstd(currentINPUT))
		isEOF(currentINPUT);
}


/* doBegin()
 * This is the block evaluator, and it works even in case of a single 
 * instructions; in case of a block, it executes every instruction in the 
 * inside, and stops and the END found, that may contain any character 
 * different from ';' as a free-text specifier but must end in the same line 
 * and with ';'. If a BEGIN is found, call itself again (via go() ). */
void doBegin(void) {
	int decBackup=decCount;
	int thisl=l, ic;
	
	/* update */
	beginTOS++;
	if (beginTOS>=LIMIT) pe(7,l);

	beginDepth[beginTOS]=decCount;

	/* cycle: exit only after END or after a GOTO out of block */
	while (TRUE) {
		if (isDebug && !stopKey) taxiDebugConsole();
		if (isQuit) break;
		/* a GOTO occurred */
		if (LL) {
			/* the jump occurs inside the BEGIN-END block */
			if (isIn(thisl,LL,PL)) {
				l=LL;
				strncpy_(BASESTR,m[l],COMPSTR);
				p=BASESTR+PL;
				LL=PL=0;
				if (!*p) {
					l++;
					if (l>endCore) pe(23,l);
					while ((!m[l] || !m[l][0])) 
						l++;
					if (l>endCore) pe(23,l);
					strncpy_(BASESTR,m[l],COMPSCR);
					p=BASESTR;
				}
			}
			/* the jump occurs outside of the BEGIN-END block */
			else break;
		}
		/* an END ends current block*/
		else if (sEND(p)) {
			p+=3;
			while (*p && !isterm(p) && isalnum(*p)) p++;
			resultType=0;
			break;
		}
		/* any other procedure is executed
		 * Note: a GOTO sets a new address that is evaluated at 
		 * the start of the cycle */
		else {
			/* reload in case of void string */
			if (!*p) {
				l++;
				while (l<=endCore && (!m[l]||!m[l][0])) 
					l++;
				if (l>endCore) pe(23,l);
				strncpy_(BASESTR,m[l],COMPSCR);
				p=BASESTR;
			}
			/* DEBUGGING PURPOSES - print pure/stripped line 
			 * Note: in case of ontrace/offtrace, since a proper 
			 * string is printed by TRACE() for each one, their
			 * debug output is disabled */
			if (isDebug && *p && !strncasecmp_(p,"IF",2)) 	{
				char output[COMPSTR], *o=output;
				char *pb=p;
				while (*pb && strncasecmp_(pb,"THEN",4))
					*o++=*pb++;
				*o='\0';
				fprintf(stdout,"%s",INVCOLOR);
				if (output[0] && strcmp(output,";")) 
					fprintf(stdout,"\n[%s]\n",output);
				fprintf(stdout,"%s",RESETCOLOR);
			}
			else if (isDebug && *p && strncasecmp_(p,"OFFTRACE",8)\
				        && strncasecmp_(p,"ONTRACE",7)) {
				char output[COMPSTR], *o;
				strncpy_(output,depured(p),COMPSTR);
				o=strcasestr_oq(output,";");
				if (o && *o) *o='\0';
				fprintf(stdout,"%s",INVCOLOR);
				if (output[0] && strcmp(output,";")) 
					fprintf(stdout,"\n[%s]\n",output);
				fprintf(stdout,"%s",RESETCOLOR);
			}
			/* execute procedure
			 * Note: in exec() if a BEGIN is found, doBegin()
			 * is called again */
			go(l);
			if (!LL) {
				/* reload */
				if (!*p || *p<0) {
					l++;
					if (l>endCore) pe(23,l);
					while ((!m[l] || !m[l][0])) 
						l++;
					if (l>endCore) pe(23,l);
					strncpy_(BASESTR,m[l],COMPSCR);
					p=BASESTR;
				}
				/*  look for the terminator */
				if (!(*p==';' || sEND(p) ||\
					       !strncasecmp_(p,"ELSE",4)))
					pe(5,l);
				if (*p==';') p++;
			}
		}
	}

	/* if a GOTO occurred and the new address is out of the block, 
	 * the cycle-break jumps here and the new address is set, 
	 * but references of LL and PL are not erased */
	if (LL) {
		l=LL;
		strncpy_(BASESTR,m[l],COMPSTR);
		p=BASESTR+PL;
		if (!*p) {
			l++;
			if (l>endCore) pe(23,l);
			while ((!m[l] || !m[l][0])) 
				l++;
			if (l>endCore) pe(23,l);
			strncpy_(BASESTR,m[l],COMPSCR);
			p=BASESTR;
		}
	}

	/* reset variables created in the block, and reset decCount */
	for (ic=decCount;ic>decBackup;ic--) {
		int i;
		for (i=0;i<COMPSTR;i++) {
			vn[ic].name[i]='\0';
			vn[ic].strdata[i]='\0';
			vn[ic].valdata[i]='\0';
		}
		vn[ic].type=NOTYPEBOX;
		vn[ic].isArr=FALSE;
		vn[ic].ref=0;
		vn[ic].dim=0;
		for (i=0;i<MAXDIM;i++) {
			vn[ic].ldim[i]=0;
			vn[ic].offset[i]=0;
			vn[ic].coeff[i]=0;
		}
		vn[ic].size=0;
		vn[ic].strmem=0;
		vn[ic].isOwn=FALSE;
		vn[ic].refcode=0;
		vn[ic].proccode=0;
		vn[ic].isFunc=FALSE;
		vn[ic].isFict=0;
		vn[ic].line=0;
		vn[ic].level=-1;
	}
	decCount=decBackup;
	if (beginTOS>0) beginTOS--;
}


/* algolstart()
 * This is the main outer cycle; in this cycle only comments and BEGIN are 
 * admitted; the first BEGIN begins the main BEGIN-END cicle by calling
 * Here doBegin(). */
void algolstart(void) {
	l=0;
	/* read first useful line */
	while (!m[l] || !m[l][0]) {
		l++;
		if (l>endCore) break; // error
		while ((!m[l] || !m[l][0])) 
			l++;
		if (l>endCore) break; // error
		strncpy_(BASESTR,m[l],COMPSCR);
		p=BASESTR;
	}

	while (l<=endCore) {
		/* a COMMENT pre-comment starts */
		if (!strncasecmp_(p,"COMMENT",7)) {
			p+=7;
			parseComment();
			if (*p==';') p++;
			else pe(5,l);
		}
		/* a ! pre-comment starts */
		else if (*p=='!') {
			p++;
			parseComment();
			if (*p==';') p++;
			else pe(5,l);
		}
		/* BEGIN main structure */
		else if (!strncasecmp_(p,"BEGIN",5)) {
			p+=5;
			doBegin();
			/* discard main block final comment */
			while (*p && *p!='.' && isalnum(*p)) p++;
			/* missing final dot: print warning */
			if (*p!='.' && !isQuit) 
				fprintf(stderr,"\n\ntaxi: %s\n",error[4]);
			else p++;
			break;
		}
		/* illegal command */
		else {
			pe(23,l);
		}

		/* reload string */
		if (!*p) {
			l++;
			if (l>endCore) break; // error
			while ((!m[l] || !m[l][0])) 
				l++;
			if (l>endCore) break; // error
			strncpy_(BASESTR,m[l],COMPSCR);
			p=BASESTR;
		}
	}
	END(EXIT_SUCCESS);
}


/* checkProc()
 * check procedure name, discarding dots 
 * return updated position of lp after proc name, or NULL if 
 * check failed 
 * System procedure for the exec() system function */
char *checkProc(char *lp, char *test, int len) {
	while (*lp && *test && len>0) {
		if (*lp=='.') lp++;
		if (toupper(*lp)==toupper(*test)) lp++, test++, len--;
		else break;
	}
	if (!len) return lp;
	return NULL;
}


/* exec()
 * the statement execution subroutine (the core of the engine, which calls
 * all the statements references in order to execute the commands)
 * lpc is the address of the execution line (the first character after the
 * line number)
 * return TRUE if execution goes well, FALSE otherwise
 * System basic function */
int exec(void) {
	int ns, type, ERRVAL=-1;
	char *lpr;

	/* reload */
	if (!*p) {
		l++;
		if (l>endCore) return FALSE;
		while ((!m[l] || !m[l][0])) l++;
		if (l>endCore) return FALSE;
		strncpy_(BASESTR,m[l],COMPSTR);
		p=BASESTR;
	}

	/* check user procedures before builtin statements */
	if ((ns=findPROC(p,&lpr,&type))) {
		/* it's an assignment to the fictitious procedure variable */
		if (*(p+_len)==PU) return FALSE;
		lpr=execPROC(ns,lpr,&numRes,NIL);
	}
	/* perform builtin statements */
	else switch(tolower(*p)) {
		case 'a':
			/* ARRAY RESERVED */
			if ((lpr=checkProc(p,"ARRAY",5))) 
				p=lpr, REALARRAY();
			else ERRVAL=0;
			break;
		case 'b':
			/* BEGIN RESERVED */
			if ((lpr=checkProc(p,"BEGIN",5))) 
				p=lpr, doBegin();
			/* BOOLEAN and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"BOOLEANARRAY",12))) 
				p=lpr, BOOLEANARRAY();
			/* BOOLEAN and PROCEDURE RESERVED */
			else if ((lpr=checkProc(p,"BOOLEANPROCEDURE",16))) 
				SKIPPROCEDURE();
			/* BOOLEAN */
			else if ((lpr=checkProc(p,"BOOLEAN",7))) 
				p=lpr, BOOLEAN();
			else if ((lpr=checkProc(p,"BREAKOUTPUT",11)) && !isnamechar(lpr)) 
				p=lpr, BREAKOUTPUT();
			else ERRVAL=0;
			break;
		case 'c':
			if ((lpr=checkProc(p,"CALL(",5))) 
				p=lpr-1, CALL();
			else if ((lpr=checkProc(p,"CLOSEFILE(",10))) 
				p=lpr-1, CLOSEFILE();
			else if ((lpr=checkProc(p,"CREATEFILE(",11))) 
				p=lpr-1, CREATEFILE();
			/* COMMENT RESERVED */
			else if ((lpr=checkProc(p,"COMMENT",7))) 
				p=lpr, parseComment();
			else ERRVAL=0;
			break;
		case 'd':
			if ((lpr=checkProc(p,"DELETE(",7))) 
				p=lpr-1, DELETE();
			/* DUMP RESERVED */
			else if ((lpr=checkProc(p,"DUMP",4))) 
				p=lpr, DUMP();
			else ERRVAL=0;
			break;
		case 'e':
			/* ELSE RESERVED */
			if ((lpr=checkProc(p,"ELSE",4))) 
				p=lpr, ELSE();
			/* END RESERVED */
			else if ((lpr=checkProc(p,"END",3)));
			else ERRVAL=0;
			break;
		case 'f':
			if ((lpr=checkProc(p,"FAULT(",6))) 
				p=lpr-1, FAULT();
			/* FORWARD RESERVED */
			else if ((lpr=checkProc(p,"FORWARD",7))) 
				p=lpr, parseComment();
			/* FOR RESERVED */
			else if ((lpr=checkProc(p,"FOR",3))) 
				p=lpr, FOR();
			else ERRVAL=0;
			break;
		case 'g':
			/* GOTO and GO TO RESERVED */
			if ((lpr=checkProc(p,"GOTO",4))) 
				p=lpr, GOTO();
			else ERRVAL=0;
			break;
		case 'i':
			/* IF RESERVED */
			if ((lpr=checkProc(p,"IF",2))) 
				p=lpr, IF();
			else if ((lpr=checkProc(p,"INARRAY(",8))) 
				p=lpr-1, INARRAY();
			else if ((lpr=checkProc(p,"INCHARACTER(",12))) 
				p=lpr-1, INCHARACTER();
			else if ((lpr=checkProc(p,"INCHAR(",7))) 
				p=lpr-1, INCHAR();
			else if ((lpr=checkProc(p,"ININTEGER(",10))) 
				p=lpr-1, ININTEGER();
			else if ((lpr=checkProc(p,"INLONGREAL(",11))) 
				p=lpr-1, INLONGREAL();
			else if ((lpr=checkProc(p,"INPUT(",6))) 
				p=lpr-1, INPUT();
			else if ((lpr=checkProc(p,"INREAL(",7))) 
				p=lpr-1, INREAL();
			else if ((lpr=checkProc(p,"INSTRING(",9))) 
				p=lpr-1, INSTRING();
			else if ((lpr=checkProc(p,"INSYMBOL(",9))) 
				p=lpr-1, INSYMBOL();
			/* INTEGER and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"INTEGERARRAY",12))) 
				p=lpr, INTEGERARRAY();
			/* INTEGER and PROCEDURE RESERVED */
			else if ((lpr=checkProc(p,"INTEGERPROCEDURE",16))) 
				SKIPPROCEDURE();
			/* INTEGER RESERVED */
			else if ((lpr=checkProc(p,"INTEGER",7))) 
				p=lpr, INTEGER();
			else ERRVAL=0;
			break;
		case 'l':
			/* LABEL AND PROCEDURE RESERVED */
			if ((lpr=checkProc(p,"LABELPROCEDURE",14))) 
				SKIPPROCEDURE();
			/* LABEL RESERVED */
			else if ((lpr=checkProc(p,"LABEL",5))) 
				p=lpr, LABEL();
			/* LONG, REAL and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"LONGREALARRAY",13))) 
				p=lpr, LONGREALARRAY();
			/* LONG, REAL and PROCEDURE RESERVED */
			else if ((lpr=checkProc(p,"LONGREALPROCEDURE",17))) 
				SKIPPROCEDURE();
			/* LONG, REAL RESERVED */
			else if ((lpr=checkProc(p,"LONGREAL",8))) 
				p=lpr, LONGREAL();
			else ERRVAL=0;
			break;
		case 'n':
			if ((lpr=checkProc(p,"NEWLINE",7)) && (!isnamechar(lpr))) 
				p=lpr, EMIT(currentOUTPUT,CRC);
			else if ((lpr=checkProc(p,"NEXTSYMBOL(",11))) 
				p=lpr-1, NEXTSYMBOL();
			else ERRVAL=0;
			break;
		case 'o':
			if ((lpr=checkProc(p,"OFFTRACE",8)) && (!isnamechar(lpr))) 
				p=lpr,TRACE(FALSE);
			else if ((lpr=checkProc(p,"ONTRACE",7)) && (!isnamechar(lpr))) 
				p=lpr,TRACE(TRUE);
			else if ((lpr=checkProc(p,"OPENFILE(",9))) 
				p=lpr-1,OPENFILE();
			else if ((lpr=checkProc(p,"OUTARRAY(",9))) 
				p=lpr-1, OUTARRAY();
			else if ((lpr=checkProc(p,"OUTCHARACTER(",13))) 
				p=lpr-1, OUTCHARACTER();
			else if ((lpr=checkProc(p,"OUTCHAR(",8))) 
				p=lpr-1, OUTCHAR();
			else if ((lpr=checkProc(p,"OUTBOOLEAN(",11))) 
				p=lpr-1, OUTBOOLEAN();
			else if ((lpr=checkProc(p,"OUTINTEGER(",11))) 
				p=lpr-1, OUTINTEGER();
			else if ((lpr=checkProc(p,"OUTLONGREAL(",12))) 
				p=lpr-1, OUTLONGREAL();
			else if ((lpr=checkProc(p,"OUTPUT(",7))) 
				p=lpr-1, OUTPUT();
			else if ((lpr=checkProc(p,"OUTREAL(",8))) 
				p=lpr-1, OUTREAL();
			else if ((lpr=checkProc(p,"OUTSTRING(",10))) 
				p=lpr-1, OUTSTRING();
			else if ((lpr=checkProc(p,"OUTSYMBOL(",10))) 
				p=lpr-1, OUTSYMBOL();
			else if ((lpr=checkProc(p,"OUTTERMINATOR(",14)))
				p=lpr-1, OUTTERMINATOR();
			/* OWN and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"OWNARRAY",8))) 
				p=lpr, OWNREALARRAY();
			/* OWN, BOOLEAN and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"OWNBOOLEANARRAY",15))) 
				p=lpr, OWNBOOLEANARRAY();
			/* OWN and BOOLEAN RESERVED */
			else if ((lpr=checkProc(p,"OWNBOOLEAN",10))) 
				p=lpr, OWNBOOLEAN();
			/* OWN, INTEGER and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"OWNINTEGERARRAY",15))) 
				p=lpr, OWNINTEGERARRAY();
			/* OWN and INTEGER RESERVED */
			else if ((lpr=checkProc(p,"OWNINTEGER",10))) 
				p=lpr, OWNINTEGER();
			/* OWN and LABEL RESERVED */
			else if ((lpr=checkProc(p,"OWNLABEL",8))) 
				p=lpr, OWNLABEL();
			/* OWN, LONG, REAL and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"OWNLONGREALARRAY",16))) 
				p=lpr, OWNLONGREALARRAY();
			/* OWN, LONG and REAL RESERVED */
			else if ((lpr=checkProc(p,"OWNLONGREAL",11))) 
				p=lpr, OWNLONGREAL();
			/* OWN, REAL and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"OWNREALARRAY",12))) 
				p=lpr, OWNREALARRAY();
			/* OWN and REAL RESERVED */
			else if ((lpr=checkProc(p,"OWNREAL",7))) 
				p=lpr, OWNREAL();
			/* OWN, STRING and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"OWNSTRINGARRAY",14))) 
				p=lpr, OWNSTRINGARRAY();
			/* OWN and STRING RESERVED */
			else if ((lpr=checkProc(p,"OWNSTRING",9))) 
				p=lpr, OWNSTRING();
			/* OWN RESERVED */
			else if ((lpr=checkProc(p,"OWN",3))) 
				p=lpr, OWNREAL();
			else ERRVAL=0;
			break;
		case 'p':
			if ((lpr=checkProc(p,"PAGE",4)) && (!isnamechar(lpr)))
				p=lpr, EMIT(currentOUTPUT,0);
			else if ((lpr=checkProc(p,"PAUSE",5)) && (!isnamechar(lpr)))
				p=lpr, PAUSE();
			else if ((lpr=checkProc(p,"PRINTOCTAL(",11))) 
				p=lpr-1, PRINT(currentOUTPUT,FALSE);
			else if ((lpr=checkProc(p,"PRINTLN(",8))) { 
				p=lpr-1; PRINT(currentOUTPUT,TRUE);
				fputc(CRC,channel[currentOUTPUT].stream);
				channel[currentOUTPUT].fcounter=0;
			}
			else if ((lpr=checkProc(p,"PRINT(",6))) 
				p=lpr-1, PRINT(currentOUTPUT,TRUE);
			/* PROCEDURE RESERVED */ 
			else if ((lpr=checkProc(p,"PROCEDURE",9))) 
				SKIPPROCEDURE();
			else ERRVAL=0;
			break;
		case 'r':
			if ((lpr=checkProc(p,"READOCTAL(",10))) 
				p=lpr-1, READ(currentINPUT,FALSE);
			else if ((lpr=checkProc(p,"READ(",5))) 
				p=lpr-1, READ(currentINPUT,TRUE);
			/* REAL and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"REALARRAY",9))) 
				p=lpr, REALARRAY();
			/* REAL and PROCEDURE RESERVED */
			else if ((lpr=checkProc(p,"REALPROCEDURE",13))) 
				SKIPPROCEDURE();
			/* REAL RESERVED */
			else if ((lpr=checkProc(p,"REAL",4))) 
				p=lpr, REAL();
			else if ((lpr=checkProc(p,"RELEASE(",8))) 
				p=lpr-1, RELEASE();
			else ERRVAL=0;
			break;
		case 's':
			if ((lpr=checkProc(p,"SELECTINPUT(",12))) 
				p=lpr-1, SELECTINPUT();
			else if ((lpr=checkProc(p,"SELECTOUTPUT(",13))) 
				p=lpr-1, SELECTOUTPUT();
			else if ((lpr=checkProc(p,"SETRAN",6)) && (!isnamechar(lpr))) 
				p=lpr, SETRAN();
			else if ((lpr=checkProc(p,"SFIELD(",7))) 
				p=lpr-1, SFIELD();
			else if ((lpr=checkProc(p,"SKIPSYMBOL",10)) && (!isnamechar(lpr))) 
				p=lpr, SKIPSYMBOL();
			else if ((lpr=checkProc(p,"SPACE(",6))) 
				p=lpr-1, EMIT(currentOUTPUT,' ');
			else if ((lpr=checkProc(p,"SPACE",5)) && (!isnamechar(lpr))) 
				p=lpr, EMIT(currentOUTPUT,' ');
			else if ((lpr=checkProc(p,"STOP",4)) && (!isnamechar(lpr))) 
				p=lpr, END(EXIT_STOP);
			/* STRING and ARRAY RESERVED */
			else if ((lpr=checkProc(p,"STRINGARRAY",11))) 
				p=lpr, STRINGARRAY();
			/* STRING and PROCEDURE RESERVED */
			else if ((lpr=checkProc(p,"STRINGPROCEDURE",15))) 
				SKIPPROCEDURE();
			/* STRING RESERVED */
			else if ((lpr=checkProc(p,"STRING",6))) 
				p=lpr, STRING();
			/* SWITCH RESERVED */
			else if ((lpr=checkProc(p,"SWITCH",6))) 
				p=lpr, SWITCH();
			else ERRVAL=0;
			break;
		case 't':
			if ((lpr=checkProc(p,"TAB",3)) && (!isnamechar(lpr))) 
				p=lpr, EMIT(currentOUTPUT,'\t');
			else if ((lpr=checkProc(p,"TRANSFILE",9)) && (!isnamechar(lpr))) 
				p=lpr, TRANSFILE();
			else ERRVAL=0;
			break;
		case 'v':
			if ((lpr=checkProc(p,"VPRINT(",7))) 
				p=lpr-1, VPRINT(currentOUTPUT);
			else ERRVAL=0;
			break;
		case 'w':
			/* WHILE RESERVED */
			if ((lpr=checkProc(p,"WHILE",5))) 
				p=lpr, WHILE();
			else if ((lpr=checkProc(p,"WRITELN(",8))) {
				p=lpr-1, WRITE(currentOUTPUT);
				fputc(CRC,channel[currentOUTPUT].stream);
				channel[currentOUTPUT].fcounter=0;
			}
			else if ((lpr=checkProc(p,"WRITE(",6))) 
				p=lpr-1, WRITE(currentOUTPUT);
			else ERRVAL=0;
			break;
		default:
			ERRVAL=0;
	}

	/* in case an error was found */
	if (!ERRVAL) return FALSE;

	return TRUE;
}


/*****************************************
 *      SIGNALS & ERROR TREATMENT        *
 *****************************************/

/* perror_()
 * print a string message based upon the following information data:
 * - errNum is the error code number defined in errors.h and must always be
 *   given as first argument 
 * - ll is the line number in which the error is occurred; if null, the 
 *   line reference is not printed (general errors); this must always be given 
 *   as the second argument */
void perror_(int line, int errNum, int ll) {
	int delay=0, recLine=0, j=0;

	/* reset color in case of debug */
	fprintf(stdout,RESETCOLOR);
	fprintf(stderr,RESETCOLOR);

	/* calculate current line */
	j=libTOS;
	while (j>0) {
		if (ll>=st[j] && ll<=en[j]) {
			recLine=ll-st[j];
			break;
		}
		j--;
	}
	if (!recLine) recLine=ll-globalInsertedLines;
	if (recLine<0) recLine=0;
	if (ll<0) recLine=-1;
	
	/* print preliminary info */
	fprintf(stderr,CRS);
	if (!isRunOn) fprintf(stderr,"taxi preprocessor error ");
	else fprintf(stderr,"taxi runtime processor error ");
	if (j && isProcTime) fprintf(stderr,"in library %s (in procedure %s), ",ext[j],Procp[isProcTime].name);
	else if (j && !isProcTime) fprintf(stderr,"in library %s, ",ext[j]);
	else if (isProcTime) fprintf(stderr,"in procedure %s, ",Procp[isProcTime].name);
	else fprintf(stderr,"in program"); 
	if (recLine>=0) {
		if (addMainRootBlock) recLine--;
		fprintf(stderr," at line %d",recLine+incrLin[recLine]);
	}
	if (isPrintSourceCode) fprintf(stderr," #%d#:\n",line);
	else fprintf(stderr,":\n");
	/* print error message */
	fprintf(stderr,"%s",error[errNum]);
	fprintf(stderr,".");
	fprintf(stderr,CRS);
	/* print mistaken line and error position 
	 * Note: if p is empty, the error occurred at the end */
	if (isRunOn && *p) {
		fprintf(stderr,"%s\n",mb[ll]);
		delay=(int)(p-BASESTR)+strlen(mb[ll])-strlen(m[ll]);
		if (delay>COMPSTR) delay=COMPSTR;
		while (delay>0) fputc(' ',stderr), delay--;
		fprintf(stderr,"^\n");
	}
	else if (isRunOn) {
		fprintf(stderr,"%s\n",mb[ll]);
		delay=strlen(mb[ll]);
		while (delay>0) fputc(' ',stderr), delay--;
		fprintf(stderr,"^\n");
	}
	/* print additional info */
	fprintf(stderr,"Error code %d\n",errNum);
	
	/* stop execution */
	END(errNum);
}


/* sigtrap()
 * trap procedure for ON ATTENTION when an interrupt has been trapped */
void sigtrap(int sig) {
	/* reassign user destination SIGINT */
	sigint(sig);
}


/* sigquit()
 * end procedure for stopping when a SIGBUS or a SIGSEGV signal 
 * has been trapped */
void sigquit(int sig) {
	/* reassign the normal behavior to SIGINT */
	signal(SIGBUS, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	closeFiles();
	pe(6,-1);
}


/* sigint()
 * end procedure if a SIGHUP or a SIGINT signal was trapped */
void sigint(int sig) {
	/* reassign the normal behavior to signals */
	signal(SIGINT, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	isInterrupt=TRUE;
	END(EXIT_STOP);
}


/* salloc()
 * create a memory space for strings
 * size is the size of the string (included the terminator) 
 * Note: this function was created to be portable among all UNIX systems
 * with gcc installed. */
void *salloc(size_t size) {
	void *value = calloc(1,size);
	if (value == NIL) pe(6,l);
	return value;
}


/* nalloc()
 * create a memory space for numbers (double)
 * size is the size of the array (calculated elsewhere) 
 * Note: this function was created to be portable among all UNIX systems
 * with gcc installed. */
double*nalloc(size_t size) {
	double *value = calloc(sizeof(double),size);
	if (value == NIL) pe(6,l);
	return value;
}


/***************************************
 *      DOCUMENTATION AND LICENSE      *
 ***************************************/

/* printHidderHelp()
 * complete help message routine
 * if hid is true, hide the undocumented features */
void printHelp(int hid) {
	fprintf(stdout,"Usage: taxi [options] [filename]\n");
	fprintf(stdout,"\nOptions:\n");
	fprintf(stdout,"      --conditions   Print GPL3 redistribution conditions and exit\n");
	fprintf(stdout,"  -c  --cdc          Enable interpreting channel 60/61 as 0/1 (CDC)\n");
	fprintf(stdout,"  -b  --root-block   Add external main root block\n");
	fprintf(stdout,"  -d  --debug        Debug mode (run-time only)\n");
	fprintf(stdout,"  -e  --ext-debug    Set extended debug mode (preparsing)\n");
	fprintf(stdout,"      --errors-list  Print a complete errors list and exit\n");
	fprintf(stdout,"      --exp=C        Set C as floating-point numbers default exponent\n");
	if (!hid) fprintf(stdout,"  -h  --help         Display normal help and exit\n");

	else fprintf(stdout,"  -h  --help         Display this information and exit\n");
	if (!hid) fprintf(stdout,"      --hidden       Display this extended help and exit\n");
	if (!hid) fprintf(stdout,"  -L                 List plain program in meta-text and exit\n");
	fprintf(stdout,"  -l  --list         List program (with indentation) and exit\n");
	fprintf(stdout,"      --page=N       Set N as the default printer page height\n");
	fprintf(stdout,"      --purge        Convert program to reserved mode (purging ticks)\n");
	if (!hid) fprintf(stdout,"  -S                 Display C-source line in the error message\n");
	fprintf(stdout,"  -t  --timing       Display timing after execution\n");
	fprintf(stdout,"      --tab=N        Set N as tabulation width in -l/--list options\n");
	if (!hid) fprintf(stdout,"  -V                 Display version number and exit\n");
	fprintf(stdout,"      --verbose      Add extra information to option -e/--ext-debug\n");
	fprintf(stdout,"  -v  --version      Display the version banner and exit\n");
	if (!hid) fprintf(stdout,"  -X                 Preprocess in debug mode and exit (debug purposes)\n");
	fprintf(stdout,"      --warranty     Print warranty conditions from GPL3 and exit\n");
	fprintf(stdout,"\nYou should have received a copy of the GNU General Public License\n");
	fprintf(stdout,"along with this program. If not, see <http:/www.gnu.org/licenses/>.\n");
	fprintf(stdout,"Email bugs reports to <ing dot antonio dot maschio at gmail dot com>.\n"); 
	fprintf(stdout,"Consult the man/info pages or the manual to know taxi in detail.\n");
	fprintf(stdout,"Come on board and drive this taxi! It's GPL! Enjoy!\n");

}


/* printLogo()
 * taxi logo */
void printLogo(void) {
	fprintf(stdout,"\n%s", LOGO);
}


/* printExistence()
 * existence message */
void printExistence(void) {
	fprintf(stdout," %sT%sonibin's %sA%slgol si%sX%sty %sI%snterpreter\n\nAlgol 60 boy Interpreter, based on the DEC-10/20 ALGOL, with ISO integrations.\n",REDCOLOR,RESETCOLOR,REDCOLOR,RESETCOLOR,REDCOLOR,RESETCOLOR,REDCOLOR,RESETCOLOR);
}


/* printVersion()
 * help banner */
void printVersion(void) {
	fprintf(stdout,"version %s (built on %s, at %s)", 
		VERSION, __DATE__ ,__TIME__);
	fprintf(stdout,"\nby %s <%s>.\nCopyleft (C) %s - DEC AA-0196C-TK - ISO 1538-1984 (E)\n", AU, EM, CY);
}


/* printHelpBanner()
 * version message */
void printHelpBanner(void) {
	fprintf(stdout,"This free software comes with ABSOLUTELY NO WARRANTY; for details about this,\ntype 'taxi --warranty'.  You are welcome to redistribute it under certain\nconditions; type 'taxi --conditions' for details.\nThis software is released under the terms of the GNU General Public License 3.\n\n");
}


/* printWarranty()
 * warranty message (required by GPL3) */
void printWarranty(void) {
	fprintf(stdout,"THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\nAPPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT\nHOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT\nWARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT\nLIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A\nPARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF\nTHE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME\nTHE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n");
}


/* printConditions()
 * conditions message (required by GPL3) */
void printConditions(void) {
	fprintf(stdout,"You may make, run and propagate covered works that you do not convey,\nwithout conditions so long as your license otherwise remains in force.\nYou may convey covered works to others for the sole purpose of having\nthem make modifications exclusively for you, or provide you with\nfacilities for running those works, provided that you comply with the\nterms of this License in conveying all material for which you do not\ncontrol copyright. Those thus making or running the covered works for\nyou must do so exclusively on your behalf, under your direction and\ncontrol, on terms that prohibit them from making any copies of your\ncopyrighted material outside their relationship with you.\n\n");
}


/******************************************************************************
 * MAIN SECTION
 ******************************************************************************/

/* main() 
 * parse options, get file name, setup environment and perform, at request, 
 * the following actions:
 * OLD to create the file in memory
 * RUN to execute the file
 * Here is where it all begins... and where it all ends */
int main(int argc, char **argv) {
	int c, ac=0;

	/* record starting time; beginS is used in the INFO function */
        gettimeofday(&tv, NULL);
        beginS = ((double)tv.tv_sec + tv.tv_usec / 1000000.0);
        beginM = (int)(tv.tv_sec *1000 + tv.tv_usec / 1000);

	/* set features that depend on starting time */
	now=time(NULL);

	/* parse options; double and single dash options may be alternated,
	 * options requiring an argument may or not be attached (but must
	 * be the last options of that group);
	 * ~Antonio Maschio - March 2014 for the decb project */
	while (++ac < argc) {
		if (*argv[ac]=='-') {
			/* PARSE DOUBLE-DASH OPTIONS */
			/* option --help */
			if (!strcasecmp(argv[ac], "--help")) { 
				printLogo();
				printExistence();
				printHelp(TRUE); 
				exit(EXIT_SUCCESS); 
			}
			/* option --hidden */
			else if (!strcasecmp(argv[ac], "--hidden")) { 
				printLogo();
				printExistence();
				printHelp(FALSE); 
				exit(EXIT_SUCCESS); 
			}
			/* option --errors-list */
			else if (!strcasecmp(argv[ac], "--errors-list") ||\
				!strcasecmp(argv[ac], "--errorslist") ||\
				!strcasecmp(argv[ac], "--errors")) { 
				int i;
				char *y;
				fprintf(stdout,"\nErrors list for taxi ");
				fprintf(stdout,"version %s\n\n",VERSION);
				for (i=0;i<ERRLIM;i++) {
				    y=error[i];
				    if (*y) fprintf(stdout,"%3d %s\n",i,y);
				}
				fprintf(stdout,CRS);
				exit(EXIT_SUCCESS); 
			}
			/* option --version */
			else if (!strcasecmp(argv[ac], "--version")) {
				printLogo();
				printExistence();
				printVersion();
				exit(EXIT_SUCCESS); 
			}
			/* option --warranty */
			else if (!strcasecmp(argv[ac], "--warranty")) {
				printWarranty();
				exit(EXIT_SUCCESS);
			}
			/* option --conditions */
			else if (!strcasecmp(argv[ac], "--conditions")) {
				printConditions();
				exit(EXIT_SUCCESS);
			}
			/* option --root-block 
			 * set root block */
			else if (!strncmp(argv[ac],"--root-block",12)) {
				addMainRootBlock=TRUE;
				continue;
			}
			/* option --list 
			 * list program */
			else if (!strncmp(argv[ac],"--list",6)) {
				isList=TRUE;
				continue;
			}
			/* option --verbose 
			 * enable extra info for debug*/
			else if (!strncmp(argv[ac],"--verbose",9)) {
				isVerbose=TRUE;
				continue;
			}
			/* option --cdc 
			 * to set up channel 60/61 as 0/1 */
			else if (!strncmp(argv[ac],"--cdc",5)) {
				isCDC=TRUE;
				continue;
			}
			/* option --purge 
			 * purge program from ticks*/
			else if (!strncmp(argv[ac],"--purge",7)) {
				isPurge=TRUE;
				continue;
			}
			/* option --exp 
			 * exponent output char */
			else if (!strncmp(argv[ac],"--exp=",6)) {
				expChar=*(argv[ac]+6);
				if (expChar!='&'&&expChar!='@'&&expChar!='E')
					expChar='&';
				continue;
			}
			/* option --timing 
			 * set timing */
			else if (!strncmp(argv[ac],"--timing",8)) {
				isTiming=TRUE;
				continue;
			}
			/* option --page set 
			 * page height */
			else if (!strncmp(argv[ac],"--page=",7)) {
				PAGEHEIGHT=strtol(argv[ac]+7,NULL,10);
				if (PAGEHEIGHT<=0) PAGEHEIGHT=DEFPAGEHEIGHT;
				continue;
			}
			/* option --tab 
			 * tabulation in option --list/-l */
			else if (!strncmp(argv[ac],"--tab=",6)) {
				tabDef=strtol(argv[ac]+6,NULL,10);
				continue;
			}
			/* option --ext-debug 
			 * set extended Debug mode */
			else if (!strncmp(argv[ac],"--ext-debug",11)) {
				isDebug=TRUE;
				isVerboseDebug=TRUE;
				continue;
			}
			/* option --debug 
			 * set normal Debug mode */
			else if (!strncmp(argv[ac],"--debug",7)) {
				isDebug=TRUE;
				continue;
			}
			/* option --greetings  (UNDOCUMENTED) */
			/* this feature is intentionally undocumented 
			 * I use it to generate the README file */
			else if (!strcasecmp(argv[ac], "--greetings")) {
				printLogo();
				printExistence();
				printVersion();
				fprintf(stdout,WELCOME);
				printHelp(TRUE);
				fprintf(stdout,CRS);
				printHelpBanner();
				fprintf(stdout,GREET);
				printWarranty();
				printConditions();
				fprintf(stdout,"It's GPL. Enjoy!\n\n");
				exit(EXIT_SUCCESS);
			}
			/* Ignore unrecognized options */
			else if (!strncmp(argv[ac],"--",2)) {
				fprintf(stderr,"(%s%s)\n",argv[ac],error[8]);
				continue;
			}
			
			/* PARSE SINGLE-DASH OPTIONS */
			while (*++argv[ac]) {
				c=*argv[ac];
				/* option -b 
				 * add root block */
				if (c=='b') {
					addMainRootBlock=TRUE;
				}
				/* option -c 
				 * list program  */
				else if (c=='c') {
					isCDC=TRUE;
				}
				/* option -d 
				 * set normal Debug mode */
				else if (c=='d') {
					isDebug=TRUE;
					stopKey=TRUE;
				}
				/* option -e 
				 * set verbose Debug mode (with pre-parsing phase) */
				else if (c=='e') {
					isDebug=TRUE;
					isVerboseDebug=TRUE;
					stopKey=TRUE;
				}
				/* option -h 
				 * print help*/
				else if (c=='h') {
					printLogo();
					printExistence();
					printHelp(TRUE); 
					exit(EXIT_SUCCESS); 
				}
				/* option -l 
				 * list program  */
				else if (c=='l') {
					isList=TRUE;
				}
				/* option -L 
				 * print program in meta-text and exit 
				 * (UNDOCUMENTED) */
				else if (c=='L') {
					isMetaList=TRUE;
				}
				/* option -S 
				 * print number of source-code line in error messages 
				 * (UNDOCUMENTED) */
				else if (c=='S') {
					isPrintSourceCode=TRUE;
				}
				/* option -t 
				 * set timing */
				else if (c=='t') {
					isTiming=TRUE;
				}
				/* option -v 
				 * print version */
				else if (c=='v') {
					printLogo();
					printExistence();
					printVersion();
					exit(EXIT_SUCCESS); 
				}
				/* option -V 
				 * print only version numbers (version-number-release) 
				 * (UNDOCUMENTED) */
				else if (c=='V') {
					fprintf(stdout,"%s",VERSION);
					exit(EXIT_SUCCESS); 
				}
				/* option -X 
				 * exit after preprocessing in extended debug mode 
				 * (UNDOCUMENTED) */
				else if (c=='X') {
					isNoExecute=TRUE;
					isVerbose=TRUE;
					isVerboseDebug=TRUE;
				}
				/* 
				 * ignore unrecognized options */
				else fprintf(stderr,"\n(-%c%s)\n",c,error[8]);
			}
		}
		else break;
	}

	/* Widen resources for recursion (*rlim are defined into taxi.h)*/
	getrlimit(RLIMIT_STACK,&origrlim);
	setrlimit(RLIMIT_STACK,&taxirlim);

	/* OK; now if, after any option, no file name was given
	 * (or if taxi was launched with no parameters), issue an error */
	if ((ac==argc)||(1==argc)) {
		pe(1,-1); 
	}

	/* get input file name (discard following arguments) 
	 * and setup name, attaching extension if not present */ 
	strncpy_(filename, argv[ac], COMPSCR);
	setupName(filename);

	signal(SIGABRT, sigquit);
	signal(SIGSEGV, sigquit);
	signal(SIGBUS, sigquit);
	/* Trap interrupts */
	signal(SIGHUP, sigint);
	signal(SIGINT, sigint);

#ifndef NOUNIX
	/* Store tcgetattr values */
	tcgetattr(0, &modes);
	savemodes = modes;
#endif

	/* OLD (implicit!)
	 * store program in memory */
	OLD();

	/* -L option (undocumented)
	 * list copy of tokenized program and exit*/
	if (isMetaList) {
		int i,j=1,ch=0;
		for (i=0; i<=endCore; i++) {
			if (m[i] && *m[i]) {
				fprintf(stdout,"  [%5d]  %s\n",i+incrLin[i],m[i]);
				j++;
				ch+=strlen(m[i]);
			}
		}
		fprintf(stdout,"%s","Program data:\n");
		fprintf(stdout,"\n%d total lines\n",i);
		fprintf(stdout,"%d condensed operative lines\n",j);
		fprintf(stdout,"%d total operative characters\n",ch);
		fprintf(stdout,"%d total listing characters\n",totchars);
		END(EXIT_SUCCESS);
	} // end MetaList

	/* --list option
	 * list the ALGOL program and exit */
	if (isList) {
		int i,j, off=0;
		int indent=TRUE;
		char *w;
		FILE *fd;
		fd=fopen(filename,"r");
		for (j=1;j<=endCore;j++) {
			int tempoff=FALSE, realoff;
			getNextLine(B,COMPSTR,fd);
			w=B; 
			if (w) while (isblank(*w)) w++;
			/* in case of indenting, begin calculations... */
			if (!mb[j] || !m[j]) fputc('\n',stdout);
			else if (indent) {
				/* decrement offset in case of END*/
				if (sEND(m[j])&& !strcasestr_oq(m[j],"BEGIN")){
					off-=tabDef;
					if (m[j][strlen(m[j])-1]=='.')
						indent=FALSE;
				}
				else if (sEND(m[j]) && strcasestr_oq(m[j],"BEGIN")) {
					if (m[j][strlen(m[j])-1]=='.')
						indent=FALSE;
				}
				/* calculate labels*/
				if ((w=strcasestr_oq(mb[j],":"))) {
					char *wl,*wr;
					wl=strcasestr_oq(mb[j],"[");
					wr=strcasestr_oq(mb[j],"]");
					if (!(wl<w && w<wr)) {
						while (isblank(*w)) 
							w++;
						if (*(w+1)!='=') {
							tempoff=TRUE;
							realoff=off-(w-mb[j])-1;
						}
					}
				}
				/* in case of regular lines, print current
				 * offset */
				if (!tempoff) {
					for (i=0;i<off;i++) fputc(' ',stdout);
					fprintf(stdout,"%s\n",mb[j]);
				}
				/* in case of label lines, print label, and
				 * then an aligned offset */
				else {
					char *wk=mb[j];
					while (wk<=w) fputc(*wk,stdout),wk++;
					for (i=0;i<realoff;i++) 
						fputc(' ',stdout);
					while(isblank(*wk)) 
						wk++;
					fprintf(stdout,"%s\n",wk);
				}
				tempoff=FALSE;
				/* increment offset in case of BEGIN */
				if (!sEND(m[j]) && strcasestr_oq(m[j],"BEGIN")) 
					off+=tabDef;
				/* after END, print a Carriage Return */
				if (sEND(m[j]) && !strcasestr_oq(m[j],"BEGIN")) {
					fputc('\n',stdout);
				}
			}
			/* ...otherwise print lines as-is */
			else if (mb[j] && *w) fprintf(stdout,"%s\n",mb[j]);
		}
		END(EXIT_SUCCESS);
	} // end list

	/* --purge option
	 * purge the ALGOL program from ticks, spit it to stdout and exit */
	if (isPurge) {
		FILE *fd;
		char *w;
		int isComment=FALSE, isString=FALSE;
		fd=fopen(filename,"r");
		while(fgets(B,COMPSTR,fd)) {
			if (B[(int)strlen(B)-1]==13) B[(int)strlen(B)-1]='\0';
			if (B[(int)strlen(B)-1]==10) B[(int)strlen(B)-1]='\0';
			if (B[(int)strlen(B)-1]==13) B[(int)strlen(B)-1]='\0';
			w=B;
			while (*w) {
				/* try to avoid comments and strings
				 * from purging the ticks */
				char op[COMPSTR];
				if (!strncasecmp_(w,"COMMENT",7) && !isComment && !isString){
					isComment= TRUE;
					sprintw(op,8,"%s",w);
					w+=7;
					if (*w=='\'') w++;
					fprintf(stdout,"%s",op);
				}
				else if (*w==';' && isComment && !isString) {
					isComment= FALSE;
				}
				else if (*w=='\"' && !isString && !isComment) isString=TRUE;
				else if (*w=='\"' && isString && ! isComment) isString=FALSE;
				else if (*w=='\'' && *(w+1)=='\'' && !isString && !isComment)
				       isString=TRUE;
				else if (*w=='\'' && *(w+1)=='\'' && isString && !isComment) 
					isString=FALSE;

				/* put character: always, in case of comments
				 * or strings, or only if not a tick in all other cases */
				if (isComment || isString) putc(*w,stdout);
				else {
					if (*w!='\'') putc(*w,stdout);
				}
				w++;

			}
			putc('\n',stdout);
		}
		END(EXIT_SUCCESS);
	} // end purge

	/* RUN (finally!)
	 * Note: RUN records also starting timer */
	RUN();
    	
	/* END (safety measure, it should never happen here) */
	END(EXIT_SUCCESS);

	/* this is needed for C */
	return EXIT_SUCCESS; 

} /* end main */

/* end of taxi.c */

