begin
	comment 
		The Whetstone Benchmark.

		See:
		A Synthetic Benchmark, 
		H.J. Curnow and B.A. Whichmann, 
		The Computing Journal, Volume 19  Number 1
					Feb 1976, P. 43-49

		Some results
		(in thousands of Whetstone Instructions per second):

		IBM 3090:  		(Delft ?) Algol Compiler	5000
		Sun 3/60: 		Nase Algol Interpreter		10
							Nase Algol2C					400
		Sparc 2: 		Nase Algol Interpreter		63
							Nase Algol2C					4200
		Pyramid ??: 	Nase Algol Interpreter		20

		``Nase'' indicates NASE A60.

		This software is part of Erik Schoenfelder's NASE A60 package
		<schoenfr at gaertner dot de>.
	;

	comment
	This program is part of the NASE A60 files for UNIX, converted 
	to reserved format (without ticks). It works painless in taxi,
	so I added the credits and joined it to the distribution package.
	
	If Erik Schoenfelder or other NASE A60 developers should think
	it's unfair, I will remove the file from the package. Write to
	Antonio Maschio <ing dot antonio dot maschio at gmail dot com>;

	real x1, x2, x3, x4, x, y, z, t, t1, t2;
	array e1[1:4];
	integer i, j, k, l, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10,n11;

	procedure pa(e);
	array e;
	begin
		integer j;
		
		j := 0;
	lab:
		e[1] := (e[1] + e[2] + e[3] - e[4]) * t;
		e[2] := (e[1] + e[2] - e[3] + e[4]) * t;
		e[3] := (e[1] - e[2] + e[3] + e[4]) * t;
		e[4] := (- e[1] + e[2] + e[3] + e[4]) / t2;
		j := j + 1;
		if j < 6 then
			goto lab;
	end;

	procedure p0;
	begin
		e1[j] := e1[k];
		e1[k] := e1[l];
		e1[l] := e1[j];
	end;

	procedure p3(x, y, z);
	value x, y;
	real x, y, z;
	begin
		x := t * (x + y);
		y := t * (x + y);
		z := (x + y) / t2;
	end;

	procedure pout (n, j, k, x1, x2, x3, x4);
	value n, j, k, x1, x2, x3, x4;
	integer n, j, k;
	real x1, x2, x3, x4;
	begin
		outreal (1, n);
		outreal (1, j);
		outreal (1, k);
		outreal (1, x1);
		outreal (1, x2);
		outreal (1, x3);
		outreal (1, x4);
		outstring (1, "\n");
	end;

	comment initialize constants ;

	t := 0.499975;
	t1 := 0.50025;
	t2 := 2.0;

	comment read the value of i, controlling total wight: if i = 10 the
		total whight is one million Whetstone instructions ;

	comment inreal (2, i) ;

	i := 10;

	n1 := 0;		n2 := 12 * i;		n3 := 14 * i;
	n4 := 345 * i;		n5 := 0;		n6 := 210 * i;
	n7 := 32 * i;		n8 := 899 * i;		n9 := 616 * i;
	n10 := 0;		n11 := 93 * i;

	comment module 1: simple identifiers ;

	x1 := 1.0;
	x2 := x3 := x4 := - 1.0;

	for i := 1 step 1 until n1 do
	begin
		x1 := (x1 + x2 + x3 - x4) * t;
		x2 := (x1 + x2 - x3 + x4) * t;
		x3 := (x1 - x2 + x3 + x4) * t;
		x4 := (- x1 + x2 + x3 + x4) * t;
	end;

	pout (n1, n1, n1, x1, x2, x3, x4);

	comment module 2: array elements ;

	e1[1] := 1.0;
	e1[2] := e1[3] := e1[4] := -1.0;

	for i := 1 step 1 until n2 do
	begin
		e1[1] := (e1[1] + e1[2] + e1[3] - e1[4]) * t;
		e1[2] := (e1[1] + e1[2] - e1[3] + e1[4]) * t;
		e1[3] := (e1[1] - e1[2] + e1[3] + e1[4]) * t;
		e1[4] := (- e1[1] + e1[2] + e1[3] + e1[4]) * t;
	end;

	 pout (n2, n3, n2, e1[1], e1[2], e1[3], e1[4]);
	
	comment module 3: as array parameter ;

	for i:= 1 step 1 until n3 do
		pa(e1);

	pout (n3, n2, n2, e1[1], e1[2], e1[3], e1[4]);

	comment module 4: conditional jumps ;

	j := 1;
	for i := 1 step 1 until n4 do
	begin
		if j = 1 then
			j := 2
		else
			j := 3;

		if j > 2 then
			j := 0
		else
			j := 1;

		if j < 1 then
			j := 1
		else
			j := 0;
	end;

	pout (n4, j, j, x1, x2, x3, x4);

	comment module 5: omitted ;

	comment module 6: integer arithmetik ;

	j := 1;	k := 2;	l := 3;

	for i := 1 step 1 until n6 do
	begin
		j := j * (k - j) * (l - k);
		k := l * k - (l - j) * k;
		l := (l - k) * (k + j) ;	 comment typo in TCJ ;
		e1[l-1] := j + k + l;
		e1[k-1] := j * k * l;
	end;

	pout (n6, j, k, e1[1], e1[2], e1[3], e1[4]);

	comment module 7: trig functions ;

	x := y := 0.5;

	for i := 1 step 1 until n7 do
	begin
		x := t * arctan (t2 * sin (x) * cos (x) /
			(cos (x + y) + cos (x - y) - 1.0));
		y := t * arctan (t2 * sin (y) * cos (y) /
			(cos (x + y) + cos (x - y) - 1.0));
	end;

	pout (n7, j, k, x, x, y, y);

	comment module 8: procedure calls ;

	x := y := z := 1.0;

	for i := 1 step 1 until n8 do
		p3 (x, y, z);

	pout (n8, j, k, x, y, z, z);

	comment module 9: array references ;

	j := 1;  k := 2;  l := 3;
	e1[1] := 1.0;  e1[2] := 2.0;  e1[3] := 3.0;

	for i := 1 step 1 until n9 do
		p0;

	pout (n9, j, k, e1[1], e1[2], e1[3], e1[4]);

	comment module 10: integer arithmetik ;

	j := 2;  k := 3;

	for i := 1 step 1 until n10 do
	begin
		j := j + k;
		k := j + k;
		j := k - j;
		k := k -j -j;
	end;

	pout (n10, j, k, x1, x2, x3, x4);

	comment module 11: standard functions ;

	x := 0.75;
	for i := 1 step 1 until n11 do
		x := sqrt (exp (ln (x) / t1));
	
	pout (n11, j, k, x, x, x, x);
end.
