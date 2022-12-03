let code = proc(int n->string)
           alien "_CODE"
let decode = proc(string s->int)
             alien "_DECODE"
let iformat = proc(int i ->string)
begin
        let sign= if i<0 then "-" else ""
        let zero = decode ("0")
        if i<0 do i:= -i
        let s:=code( zero +(i rem 10))
        repeat
        {
                i:= i div 10
        }
        while i >0 do s:= code(zero +( i rem 10 )) ++ s
        sign ++ s
end

let letter = proc ( string s -> bool )
               begin
                       let a=97
					   let z=122
					   let A=65
					   let Z=90
                       let c=decode(s)
                       a<=c and c<=z or A<=c and c<=Z
               end
let digit := proc ( string s -> bool )
               begin
                       let zero=48
					   let nine = 57
                       let c = decode (s)
                       zero<=c and c<=nine
               end

procedure pad(string s
              int w->string)
if length( s ) >= w then s else pad(" "++s,w-1)

procedure eformat(real n
                  int w,d->string)
begin
	! w digits before decimal point
	! d digits after  + exponent
	let truncate = proc(real n ->int)
				   alien "_TRUNCATE"
    if w>4 do w:=4 ! constrained by 16 bit integers
	let exp :=0
	let sign=n>0
	if ~ sign do n:= -n
	while n>10 do { exp:= exp+1
					  n:= n/10}
	while n<1 do {exp:=exp-1
					n:=n *10}
	for i=1 to w do if exp >0 do {exp:=exp-1
					   n:=n*10}
	let tn := truncate(n)
	let lead=pad(iformat(tn),w)
	let zero=decode("0")
	let tail:="."
	for i= 1 to d do{
  	  n:=n - tn
	  n:=n*10
	  tn:=truncate(n)
	  tail:=tail ++ code(zero+tn)
     }
	 (if sign then "" else "-")++lead++tail++"e"++iformat(exp)
  end

procedure fformat(real n
                   int w,d->string)
begin
	! w digits before decimal point
	! d digits after
	let truncate = proc(real n ->int)
				   alien "_TRUNCATE"
    if w>4 do w:=4 ! constrained by 16 bit integers
	let exp :=0
	let sign=n>0
	if ~ sign do n:= -n
	if n<10000 then{
	  ! number in range

	  let tn := truncate(n)
	  let lead=pad(iformat(tn),w)
	  let tail:="."
	  let zero = decode("0")
	  for i= 1 to d do{
  	    n:=n - tn
	    n:=n*10
	    tn:=truncate(n)
	    tail:=tail ++ code(tn+zero)
      }
	  (if sign then " " else "-")++lead++tail
	} else "overflow"
  end

procedure gformat(real n;int w,d ->string)
{
	let absn= (if n>=0 then n else -n)
	if absn<10000 and
	   absn >0.00001
	   then fformat(n,w,d)
	   else eformat(n,2,d)
}

procedure options(-> *string)
{
	let argc=proc(->int)
			 alien "_ARGC"
	let argv=proc(int i->string)
			 alien "_ARGV"
	let count=argc()
    let o:= vector 1::count of ""
	for  i= 1 to count do o(i):=argv(i)
	o
}


