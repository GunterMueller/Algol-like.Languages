include "menulib.s"

let m = @1 of string [ "one", "two", "three"]

let m1= make.text.menu(m,16+3)


write menu.pop.up(m1,40,10)

?
