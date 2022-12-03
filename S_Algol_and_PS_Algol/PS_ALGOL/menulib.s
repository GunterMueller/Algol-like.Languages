! Menulib.s depends upon file menuseg.out in PSDIR
! Works in IBM text mode with microsoft mouse
! Copyright Paul Cockshott
let menuseg="menuseg"
let make.text.menu = import("make.text.menu")(*string,int -> * *int)
                segment menuseg
! first parameter is a vector of strings
! second parameter is the attributes to be used
! low order 4 bits specify foreground
! bits 4 to 6 specify background colour
! returns a 2 d aray of integers made up of the
! each integer contains a char in lower 8 bits and
! attributes in the top 8


let swap.block = import("swap.block")(* *int , int,int->* *int)
                segment menuseg
! swap a 2d array with an area of screen ram
! the old contents of the screen ram are returned
! first param is a 2 d array of integers
! next 2 give x,y position of top left corner

let menu.pop.up = import("menu.pop.up")(* *int,int,int->int)
                  segment menuseg
! put up a menu return the entry selected
! -1 if no entry selected
