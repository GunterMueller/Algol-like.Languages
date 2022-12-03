begin
	! setup;
	integer n0, m0, j;	
	real eps1, eps2, eps3, eta0,limit;
	real array x[0:10];
	integer array sol[0:10];
	n0:=8;
	m0:=100;
	eps1:=1&-8;
	eps2:=1&-9;
	eps3:=1&-9;
	eta0:=1&-10;
	limit:=1&-18;
	for j:=0 until n0 do x[j]:=0;

	comment the mathematical function;
	procedure func(x,f);
	real x,f; value x;	
	f:=x^8-24*x^7+218*x^6-880*x^5+1089*x^4+2584*x^3-8028*x^2+5040*x;

	procedure zeros(n,C,FUNCTION,m,ep1,ep2,ep3,eta);
	integer n,m; real ep1,ep2,ep3,eta; array C; procedure FUNCTION;
	comment This procedure finds the real zeros of an arbitrary
	function using Muller's method and is adapted from a
	FORTRAN code by Frank. Each iteration determines a zero
	of the quadratic passing through the last three function
	values. Parameters include the number of roots desired n.
	If C[i] is zero, starting values are -1, 1, 0 respectively.
	If C[i]=B then the starting values are .9B, 1.1B, B. The
	procedure FUNCTION(rt,frt) must be supplied to evaluate
	the function value frt, given the argument rt. m is the
	maximum number of iterations permitted. ep1 is the relative
	convergence criterion on successive iterates. ep2 is the
	absolute convergence criterion on the function value. eta
	is the spread for multiple roots, that is, if |rt-C[i]|<ep3
	where C[i] is a previously found root, then rt is replaced
	by rt+eta;
	begin
		integer L,jk,i,mm; real p,p1,p2,x0,x1,x2,rt,
					frt,fprt,d,dd,di,h,bi,den,dn,dm,tem;
		switch S:= S1, S2, S3, S4;
		for L:=1 step 1 until n do begin
			jk:=0; if C[L]=0 then goto initial 
			else goto assign;
initial:	p:=-1;  p1:=1;  p2:=0; goto start;
assign:	p:=.9*C[L];  p1:=1.1*C[L];  p2:=C[L];
start:	rt:=p; goto fn;
enter:	goto S[if jk<4 then jk else 4];
S1:		rt:=p1;  x0:=fprt;  goto fn;
S2:		rt:=p2;  x1:=fprt;  goto fn;
S3:		x2:=fprt;  h:=if C[L]=0 then -1 else -.1*C[L];
			d:=-.5;
loop:		dd:=1+d;  bi:=x0*d^2-x1*dd^2+x2*(dd+d);
			den:=bi^2-4*x2*d*dd*(x0*d-(x1*dd)+x2);
			if den <= 0 then den:= 0 else den:=sqrt(den);
			dn:=bi+den;  dm:=bi-den;
			if abs(dn)<=abs(dm) then den:=dm else den:=dn;
			if den=0 then den:=1;
			di:=-2*x2*dd/den;  h:=di*h;  rt:=rt+h;
			goto if abs(h/rt)<ep1 then funccall else fn;
S4:		if abs(fprt) < abs(x2*10) then begin
				x0:=x1; x1:=x2; x2:=fprt; d:=di;
				goto loop;
			end
			else begin
				di:= di*.5; h:=h*.5; rt:=rt-h;
				goto fn;
			end;
fn:		jk:=jk+1; if jk<m then mm:=1 else mm:=0;
funccall:		
			FUNCTION(rt,frt); 
			if mm=1 then goto compute
			else goto root;
compute:	fprt:=frt;
			!ontrace;
			for i:=2 step 1 until L do begin
				tem:=rt-C[i-1];
				if abs(tem)<ep3 then goto change
				else fprt:=fprt/tem
			end;
			!offtrace;
test:		if abs(frt)<ep2 /\ abs(fprt)<ep2 then goto root
			else goto enter;
change:	rt:=rt+eta;  jk:=jk-1; goto fn;
root:		C[L]:=rt; 
		end L;
	end ZEROS;

	! main;

	! call zeros;
	zeros(n0,x,func,m0,eps1,eps2,eps3,eta0);

	! print zeros;
	write("Solutions for the desired function[n]");
	for j:=1 until n0 do begin
		write("x[[");print(j,1,0);write("]]=");
		println(x[j],4,8);
	end;
	write("[2n]Proof[n]");
	for j:=1 until n0 do begin
		real y;
		func(x[j],y);
		write("zero of x[[");print(j,1,0);write("]]=");
		println(y);
	end;
end.
			
