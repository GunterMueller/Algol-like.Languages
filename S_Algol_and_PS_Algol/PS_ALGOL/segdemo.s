segment "segdemo"
procedure myproc (int a ->int)
begin

    let j:=a+1
    j:=2
    j
end
procedure  proc2 (int a,b)
begin
   let c=myproc(b)
   b:=c
   write a:b      ,c
end
?
