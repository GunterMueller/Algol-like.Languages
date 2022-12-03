begin 
	integer n2;
	n2:=3;
	array a2,b2,c2[1:n2,1:n2];

	PROCEDURE MATRIXMULT(A,B,C,N);
	VALUE N; ARRAY A,B,C; INTEGER N;
	BEGIN   
		INTEGER I,J,K; REAL X;

      COMMENT THIS PROCEDURE PERFORMS THE MATRIX
		MULTIPLICATION OF 'B' AND 'C' AND STORES THE RESULT
		IN 'A'. THE ARRAYS ARE ASSUMED TO BE SQUARE
		AND OF BOUNDS 1:N,1:N;
	
		FOR I := 1 UNTIL N DO
			FOR J := 1 UNTIL N DO BEGIN 
				X := 0;
				FOR K := 1 UNTIL N DO X := X + B [I,K]*C[K,J];
 				A[I,J] := X
  	   
  	   	END
	END;

	a2[1,1]:=a2[2,2]:=a2[3,3]:=1;
	a2[1,2]:=a2[1,3]:=4;
	a2[2,1]:=a2[2,3]:=0;
	a2[3,1]:=a2[3,2]:=0;

	b2[1,1]:=b2[2,2]:=b2[3,3]:=4;
	b2[1,2]:=b2[1,3]:=5;
	b2[2,1]:=b2[2,3]:=-6;
	b2[3,1]:=b2[3,2]:=9;

	writeln("Matrices are");
	
	outarray(1,a2);
	outarray(1,b2);

	MATRIXMULT(c2,a2,b2,n2);

	writeln("[2n]Result is");
	outarray(1,c2);
end.
