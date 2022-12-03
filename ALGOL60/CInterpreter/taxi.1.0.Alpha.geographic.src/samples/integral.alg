begin
	real a,b,x;
	
	a:=0;
	b:=2;
	
	!outreal(1,integral("x^2-4") from:(a) to:(b) var:(x) depth:(6));
	outreal(1,integral("x^2-4",a,b,x,6));
end.
