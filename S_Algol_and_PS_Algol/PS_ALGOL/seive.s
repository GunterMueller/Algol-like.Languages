

! Program to find prime numbers
let rep =10
write "This program finds prime numbers",
      "'ntype in a number less than 3000 and greater than 3 >"

let n:=readi()
while n<=3 or n>3000 do begin
    write "Outside of bounds'ntry again:"
    n:=readi()
end

let seive := vector 1::n of true
for j= 1 to rep do {
  for i= 1 to n do seive(i):=true
for i=2 to n do
  if seive (i)   do {
   for k=i*2 to n by i do seive(k):=false
  }

}
write "finished'n" ,rep," repetitions'n"
for i= 1 to n do if seive (i) do   write i,"'n"
?
