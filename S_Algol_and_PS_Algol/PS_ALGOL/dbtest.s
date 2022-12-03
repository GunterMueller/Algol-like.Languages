include "filelib.s"
structure db($string names)
let t := tab string of
        "Paul":"Cockshott"
        default : " I do not know about them "
write "'n Database demo'n'n1. Open a db'n2. Create new db'n"
 let reply =readi()
write reply
case reply of
1 : {
     write "Database file name:"
     let fn= read.a.line()
     let f= open( fn , rmode )
     if f=nullfile then {write "Could not open it'n'n'n"}
     else {
       let p=readp(f)
         t:=p(names)
     }
 }
2 : write "Using new database 'n"
default : { }

let q:= true
while q do
begin
    write "'n1. Add a name pair'n'n2. Lookup a surname'n'n3. Quit'n'n>"
    let j=readi()
    case j of
    1 : {
         write "Forename:"
         let f=read.a.line()
         write "Surname:"
         t (f):=read.a.line()
         }
    2 : {
         write "Forename :"
         write t(read.a.line()),"'n"
        }
    default: q:=false


end

write "'n'n'n1. Save Database'n'n2. Quit 'n"
let jj=readi()
write jj
case jj of
1: {
    write "Dbname:"

    let n=read.a.line()
    let f= create(n)
    if f=nullfile then {
       write " can not open ",n,"'n"
    }
    else
    {
       output f, db(t)
    }
  }
default : write "bye"

?
