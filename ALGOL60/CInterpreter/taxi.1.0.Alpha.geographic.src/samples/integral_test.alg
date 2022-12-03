begin
	comment integral procedure that uses integral() in the short version;
	real procedure integ(F) from:(a) to:(b) integration var:(x);
	string F; value a,b; real a,b,x;
	begin
		integ:=integral(F,a,b,x,7);
	end;

	comment main section;
	real a1,b1,x1;
	string s;
	comment integration limits;
	a1:=-1;
	b1:=4;
	comment integration function;
	s:="x^2-4"; ! look! the variable is x, not x1;

	outstring(1,"\nResult = ");
	outlongreal(1,integ(s,a1,b1,x1));
	outstring(1,"\nlast x = ");
	outreal(1,x1);
	outstring(1,"\n");
end.
