begin
	comment
	GENERAL PROBLEM SOLVER
	The use of the GPS procedure, which is rightly described by Knuth and Merner
	as "just ALGOL for ALGOL's sake", nevertheless demonstrates several
	fundamental characteristics of the ALGOL language which are of great 
	practical value. 
	It starts by setting matrix A, in which each A[i, j] has the value i + j. 
	In addition, the complete representation of the object program of the 
	assignment instruction, which uses GPS to perform the multiplication of the 
	matrix, is given.
	Although the implementation method has not been proven to be somehow 
	optimal, the example shows standard taxi mechanisms for the management 
	of recursive procedures. 
	Furthermore, parameters called by name can easily deal with the apparent 
	complications of the General Problem Solver.
	B. Randell and L.J. Russell, "Algol 60 Implementation", 1964, p. 253;

	real I;
	real J;
	real array A[1:4, 1:5];
 
	comment 
	procedure GPS;
   real procedure GPS(I1,N,Z,V); 
	real I1,N,Z,V;
	begin
		for I1 := 1 step 1 until N do Z := V;
		GPS := 1
	end;

	comment main section. 
	Note: there's no need to instantiate I and J.
	Warning: don't invert I and J, or the program will start an infinite cycle;
	I := GPS(J, 5.0, I, GPS(I, 4.0, A[I,J], I + J));

	comment
	output of final I and J;
	outreal(1,I);outstring(1,"\n");
	outreal(1,J);outstring(1,"\n");

	comment 
	output of the array;
	for I:=1 step 1 until 4 do begin
		for J:=1 step 1 until 5 do begin
			outreal(1,A[I,J]); 
		end;
		outstring(1,"\n");
	end;
end.
