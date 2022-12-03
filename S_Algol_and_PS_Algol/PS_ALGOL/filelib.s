! library of file io primitives used by PS-algol
! Copyright W P Cockshott
let rmode =0; let wmode=1; let rwmode=2
let open      = proc( string filename; int mode -> file );alien "_OPEN"
let create    = proc( string filename -> file );alien "_CREATE"
let close     = proc( file f);alien "_CLOSE"
let seek      = proc( file f; int offset, key ->int);alien "_SEEK"

