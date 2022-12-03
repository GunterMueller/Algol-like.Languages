begin
	comment
	GENERAL PROBLEM SOLVER
	The use of the GPS procedure, which is rightly described by Knuth and Merner
	as "just ALGOL for ALGOL's sake", nevertheless demonstrates several
	fundamental characteristics of the ALGOL language which are of great 
	practical value. 
	It starts by setting matrices A and B. Then is proceeed by setting
	matrix C to have the value of the multiplication of A*B. 
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
	real K;
	comment m = 4, n = 5, p = 3;
	real array A[1:4, 1:5], B[1:5, 1:3], C[1:4, 1:3];
 
	comment 
	procedure GPS;
   real procedure GPS(I1,N,Z,V); 
	real I1,N,Z,V;
	begin
		for I1 := 1 step 1 until N do Z := V;
		GPS := 1
	end;

	comment
	fill up matrices A and B;
	for I:=1 step 1 until 4 do
		for J:=1 step 1 until 5 do
			A[I,J]:=I*J;
	for I:=1 step 1 until 5 do
		for J:=1 step 1 until 3 do
			B[I,J]:=I*J;

	comment main section
	m = 4, n = 5, p = 3;
	I:=GPS(I, 1.0, C[I,I], 0.0) * GPS(I, (4-1) * GPS(J,(3-1) * GPS(K,5.0, C[I,J],
   C[I,J]+A[I,K]*B[K,J]), C[I,J+1], 0.0), C[I+1,1], 0.0);

	comment output of I and J;
	outreal(1,I);outstring(1,"\n");
	outreal(1,J);outstring(1,"\n");
	outreal(1,K);outstring(1,"\n");

	comment 
	output of the arrays;
	outstring(1,"Array A is\n");
	for I:=1 step 1 until 4 do begin
		for J:=1 step 1 until 5 do begin
			outreal(1,A[I,J]); 
		end;
		outstring(1,"\n");
	end;
	outstring(1,"Array B is\n");
	for I:=1 step 1 until 5 do begin
		for J:=1 step 1 until 3 do begin
			outreal(1,B[I,J]); 
		end;
		outstring(1,"\n");
	end;
	outstring(1,"\nMultiplication array C is\n");
	for I:=1 step 1 until 4 do begin
		for J:=1 step 1 until 3 do begin
			outreal(1,C[I,J]); 
		end;
		outstring(1,"\n");
	end;

	comment
	erasing matrix C;
	for J:=1 step 1 until 4 do
		for K:=1 step 1 until 3 do
			C[J,K]:=0;
	outstring(1,"\nRedoing it with classic FOR cycles\n");
	for K:=1 step 1 until 3 do
		for J:=1 step 1 until 5 do
			for I:=1 step 1 until 4 do
				C[I,K]:=C[I,K]+A[I,J]*B[J,K];
	outstring(1,"\nNew Multiplication array C is\n");
	for I:=1 step 1 until 4 do begin
		for J:=1 step 1 until 3 do begin
			outreal(1,C[I,J]); 
		end;
		outstring(1,"\n");
	end;

end.
