begin
	
	real r;
	long real lr;
	outstring(1,"Here the result of some round-off-prone numbers, known in literature\n\n");
	outstring(1,"1. DEC World.\n");

	outstring(1,"1/7\n");
	r:=1/7;   lr:=1/7;
	print(r); println(lr);

	outstring(1,"ln(2)\n");
	r:=ln(2); lr:=lln(2);	
	print(r); println(lr);

	outstring(1,"sqrt(2)\n");
	r:=sqrt(2); lr:=lsqrt(2);	
	print(r); println(lr);

	outstring(1,"exp(1)=e\n");
	r:=exp(1);  lr:=lexp(1);
	print(r); println(lr);
	
	outstring(1,"greek pi\n");
	r:=toreal(pi);  lr:=pi;
	print(r); println(lr);
	
	outstring(1,"\n2. ISO World.\n");

	outstring(1,"1/7\n");
	r:=1/7;   lr:=1/7;
	outreal(1,r); outlongreal(1,lr);
	outstring(1,"\n");

	outstring(1,"ln(2)\n");
	r:=ln(2); lr:=lln(2);	
	outreal(1,r); outlongreal(1,lr);
	outstring(1,"\n");

	outstring(1,"sqrt(2)\n");
	r:=sqrt(2); lr:=lsqrt(2);	
	outreal(1,r); outlongreal(1,lr);
	outstring(1,"\n");

	outstring(1,"exp(1)=e\n");
	r:=exp(1);  lr:=lexp(1);
	outreal(1,r); outlongreal(1,lr);
	outstring(1,"\n");
	
	outstring(1,"greek pi\n");
	r:=toreal(pi);  lr:=pi;
	outreal(1,r); outlongreal(1,lr);
	outstring(1,"\n");
	
	outstring(1,"\n3. Correct representation\n");

	outstring(1,"1/7\n");
	r:=1/7;   lr:=1/7;
	outstring(1,"0.142857142857142857\n");

	outstring(1,"ln(2)\n");
	r:=ln(2); lr:=lln(2);	
	outstring(1,"0.693147180559945309\n");

	outstring(1,"sqrt(2)\n");
	r:=sqrt(2); lr:=lsqrt(2);	
	outstring(1,"1.414213562373095048\n");

	outstring(1,"exp(1)=e\n");
	r:=exp(1);  lr:=lexp(1);
	outstring(1,"2.718281828459045235\n");
	
	outstring(1,"greek pi\n");
	r:=toreal(pi);  lr:=pi;
	outstring(1,"3.141592653589793238\n");
end.

