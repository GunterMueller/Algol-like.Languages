begin
	comment
		calculation of the prime numbers between 2 and 200
	;

	integer NN;

	NN := 2000;

	begin
		comment first algorithm (check division in a loop) ;

		boolean procedure isprime (n);
		value n;
		integer n;
		begin
			boolean procedure even(n);
			value n; integer n;
			even := entier (n / 2) * 2 = n;
			
			integer i;

			isprime := false;
			if even(n) and n notequal 2 then 
				goto ret;

			for i := 3 step 2 until n div 2 do
				if entier (n / i) * i = n then
					goto ret;
			isprime := true;
		    ret:

		end;

		integer ii;

		vprint ("first algorithm:");

		for ii := 2 step 1 until NN do
			if isprime (ii) then
				outinteger (1, ii);
		vprint ("\ndone.")
	end;

	begin
		comment second algorithm (sieve) ;


		boolean array arr [2 : NN];
		integer i, j;

		vprint ("second algorithm:");

		for i := 2 step 1 until NN do
			arr [i] := true;

		for i := 2 step 1 until NN do
			for j := 2 * i step i until NN do
				arr [j] := false;
		
		for i := 2 step 1 until NN do
			if arr [i] then
				outinteger (1, i);
		vprint ("\ndone.")

	end
end.	

