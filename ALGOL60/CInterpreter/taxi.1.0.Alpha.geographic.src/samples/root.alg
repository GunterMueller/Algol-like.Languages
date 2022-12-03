comment : Rootfinder (1958)
	 J. Wegstein
    National Bureau of Standards, Washington 25, D.C.
	 converted for Algol60 and taxi by Antonio Maschio - March 2019;

comment 	by Antonio Maschio: only to underline the fact this is not
    		a standard root finder, computing x for
 				f(x)=0
	 		but finds the solution of the equation 
				f(x)=x
	 		which is a different subject. To find the desired result for
 			f(x)=0, one must set f = f(x)+x, and the algorithm will find
			where f(X) is null. My 2ç (March 2019);
comment  by Antonio Maschio: this algorithm works badly with functions
         with imaginary solutions and with sinusoidal functions;
	
begin
	comment This is the mathematical function to operate upon. 
	Change it at will;
		real procedure func(x);
		value x; real x;
		begin
			func:= (x-5)*(x-6)*(4-x)*(x-2)+x	;  ! line to be changed;
		end;

	comment Rootfinder, J. Wegstein, 1960
		This procedure computes a value of g=x satisfying 
		the equation x=f(x). The procedure calling statement 
		gives the function, an initial approximation a<>0 to 
		the root, and a tolerance parameter epsi for determining 
		the number of significant figures in the solution. 
		This accelerated iteration or secant method is described
		by the author in Communications, June 1958.;
      real procedure root(f) estimation:(a) precision:(eps);
		value eps; real procedure f;
		begin real b,c,d,e,g;
Root:		b:=a;  c:=f(b);  g:=c;
			if (c=a) then go to Quit;
			d:=a;  b:=c;  e:=c;
Hob: 		c:=f(b); 
			g:=(d*c-b*e)/(c-e-b+d+epsilon);
			if abs((g-b)/(g+epsilon))<=eps then go to Quit;
			e:=c;  d:=b;  b:=g; go to Hob;
Quit: 	root:=g;
		end of root;

		comment of the main section:
		app  is the first approximated estimation: you can set
           it to any desired value and if it converges
           the solution is given in x.
      epsi is the desired precision. I must warn that taxi is
           quite precise even with low epsi values, because the 
           intermediate results are maintained, not trimmed;
		real app, epsi, x;
		epsi:=1&-8;
		app:=5;
		x:=root(func,app,epsi);
		write("Root = "); println(x,3,10);
		write("Proof= "); println(func(x),3,10);
end.

Notes:
The algorithm was slightly modified:
- the procedure Root() yields a return value which is the root value. The
  original algorithm used global variables with g as the root.
- the original division check was 
		if (abs((g-b)/g)<=eps) then goto quit;
  but this leads to errors if g=0 or if the algorithm is given an initial
  value a=0; the version here contained uses
		if (abs((g-b)/(g+epsilon))<=eps) then goto quit;
  to ensure that the solution g=0 is accepted and found and that the initial 
  value can be given also as a=0 (Antonio Maschio, March 2019)

CERTIFICATION
The program was tested using a subset of the same functions of the 
certification by Henry C. Thacher:

(1)	f(x) = (x+1)^(1/3)
(2) 	f(x) = tan x
(3.a) f(x) = 2 pi a + arctan(x)			(a = 1,2,3,4)

Results were as follows:

f(x)		a		epsi				x						notes
(1)				1&-12				 1.3247179572		correct result
(2)				1&-12				
(3.1)		1		1&-12				 7.7252518369		correct result
(3.2)		2		1&-12				14.0661939184		correct result
(3.3)		3		1&-12				20.3713029592		correct result
(3.4)		4		1&-12				26.6660542588		correct result
	

