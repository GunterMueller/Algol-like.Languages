!  salgol maths interface library
!  this provides headers to a set of C procedures
!  the C procedures are in Salib.lib
!  Copyright W P Cockshott
let epsilon    = 2.2204449908339731E-16
let pi         = 3.141592653589793
let maxreal    = 1.701411733192643E+18
let sqrt = proc (real x -> real ) ; alien "_SQRT"
let exp  = proc (real x -> real ) ; alien "_EXP"
let ln   = proc (real x -> real ) ; alien "_LN"
let sin  = proc (real x -> real ) ; alien "_SIN"
let cos  = proc (real x -> real ) ; alien "_COS"
let tan  = proc (real x -> real ) ; alien "_TAN"
let atan = proc (real x -> real ) ; alien "_ATAN"
let truncate = proc (real x -> int ) ; alien "_TRUNCATE"
let rabs = proc (real x -> real )  ; if x>0 then x else -x
let abs  = proc (int  x -> int  )  ; if x>0 then x else -x
let randSeEd :=7
let random = proc (-> int ) ;abs( { randSeEd:= (randSeEd+13)*29; randSeEd})
