" Vim syntax file
" Language:	ALGOL - for tonibin's DEC ALGOL taxi
" Creator:	Antonio Maschio <ing dot antonio dot maschio at gmail dot com>
" Maintainer:	Antonio Maschio <ing dot antonio dot maschio at gmail dot com>
" Last updated: February 2020

" Based on Allan Kelly's BASIC vim file
" With the active help of Mike Soyka, Brett Stahlman and Christian Brabandt

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" specials
syn case ignore
syn sync fromstart

" end else 
"syn keyword taxiStatement end else
syn match  taxiStatement  "else"
syn match  taxiStatement  "end"
syn match  taxiStatement  "end\."

" functions
syn keyword taxiFunction abs cos exp int sin tan sign length size pi
syn keyword taxiFunction ln sqrt lsin lcos larctan lsqrt lexp lln lb ub
syn keyword taxiFunction arccos array maxreal minreal maxint epsilon inf
syn keyword taxiFunction arcsin arctan cosh dim imin imax rmin rmax lmin lmax
syn keyword taxiFunction arcsinh arccosh arctanh cotan arccotan cotanh
syn keyword taxiFunction arccotanh sec sech arcsec arcsech cosec arccosec
syn keyword taxiFunction cosech arccosech integral
syn keyword taxiFunction entier sinh tanh bool infinity gamma erf
syn keyword taxiFunction true false inchan outchan iochan
syn match   taxiFunction "<"
syn match   taxiFunction ">"
syn match   taxiFunction "<>"
syn match   taxiFunction "><"
syn match   taxiFunction "!="
syn match   taxiFunction "\~="
syn match   taxiFunction ":="
syn match   taxiFunction "<-"
syn match   taxiFunction "\*\*"
syn match   taxiFunction "\^"
syn match   taxiFunction "\->"
syn match   taxiFunction ">>"
syn match   taxiFunction "=="
syn match   taxiFunction "<->"
syn match   taxiFunction "<="
syn match   taxiFunction "=<"
syn match   taxiFunction ">="
syn match   taxiFunction "=>"
syn match   taxiFunction "="
syn match   taxiFunction "\/\\"
syn match   taxiFunction "\\\/"
syn keyword taxiFunction and or not eqv imp div rem mod
syn keyword taxiFunction eql notequal neq impl eqiv leq lss gtr geq
syn keyword taxiStrFunction concat copy newstring convert
syn keyword taxiStrFunction newline space tab take drop head tail 
syn keyword taxiDebug ontrace offtrace dump

" facilities
syn keyword taxiTodo contained	TODO

" statements always without arguments or connective elements
syn keyword taxiType      integer longreal long real boolean string label value
syn keyword taxiStatement procedure outarray inarray if input insymbol call
syn keyword taxiStatement begin breakoutput checkoff checon createfile fault
syn keyword taxiStatement closefile delete do external for forward pause
syn keyword taxiStatement nextsymbol openfile output outsymbol own
syn keyword taxiStatement print println printoctal read readoctal release
syn keyword taxiStatement selectinput selectoutput skipsymbol stop step
syn keyword taxiStatement switch then until while write writeln transfile
syn keyword taxiStatement inchar outchar ininteger outinteger instring
syn keyword taxiStatement outstring inreal outreal inlongreal outlongreal
syn keyword taxiStatement outboolean incharacter outcharacter include
syn keyword taxiStatement clock page delete outterminator

" strings 
syn region taxiString	start="\""  end="\""
syn region taxiString	start="\'\'"  end="\'\'"

" Integer number
syn match  taxiNumber		"\<\d\+\>"
" Octal number 
syn match  taxiNumber		"%\+[01234567]\+"
" ASCII number 
syn match  taxiNumber		"\$\(.\).\{,4}\1"

" Real and long real with optional exponent
syn match  taxiReal		"\<\d\+\(\.\d\+\)\=\([ED@&]\+[+-]\=\d\+\)\=\>"
syn match  taxiReal		"\<\d\+\(·\d\+\)\=\([ED@&]\+[+-]\=\d\+\)\=\>"
" Real and long real starting with a '.', optional exponent
syn match  taxiReal		"\.\d\+\([ED@&]\+[+-]\=\d\+\)\=\>"
syn match  taxiReal		"·\d\+\([ED@&]\+[+-]\=\d\+\)\=\>"
" Real starting with a '&'
syn match  taxiReal		"[@&]\+[+-]\=\d\+"

" Label ending with colon (catch only labels attached to colon)
" If a standard name is used as label (e.g. step:), the chosen highlight
" will be the word colour, and not the label color.
syn region taxiLabel start="go\s*to\s\+"hs=e+1 end="[0-9A-Za-z\.]\+" contains=taxiStatement
syn match  taxiLabel "[0-9A-Za-z\.]\+:$"
syn match  taxiLabel "[0-9A-Za-z\.]\+:\s\+"
syn match  taxiStatement "go\s*to" contained

" comments
" default comments with comment and !
syn region  taxiComment start="comment" end=";"me=e-1,he=e-1 contains=taxiTodo
syn region  taxiComment start="![^=]" end=";"me=e-1,he=e-1 contains=taxiTodo
" text between a ) and the next :( is a comment
syn match  taxiComment ").*:\s*("ms=s+1,me=e-1
" taxi comment with a starting dash until end of line
syn region  taxiComment start="^-"      end="$" contains=taxiTodo
" text following the last end with dot is a comment 
syn region  taxiBottomComment start="end.*\."hs=e+1 contains=taxiLastComment,taxiStatement end="%$"
" text between end and either a semicolon or a dot is also a comment
" thanks to Haakon Riiser <hakonrk@fys.uio.no> for his simula.vim file
" from which I grabbed the following line, adapting it to taxi
syn region  taxiLastComment start="end"lc=3 matchgroup=taxiStatement end="[\.;]"

" special cases of semicolon and end-else continuous, taken as statements
syn match   taxiStatement "end\s*else"
syn match   taxiStatement ";"

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_taxi_syntax_inits")
  if version < 508
    let did_taxi_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink taxiStatement		Statement
  HiLink taxiDebug		Error
  HiLink taxiFile		Special
  HiLink taxiString		String
  HiLink taxiLabel		String
  HiLink taxiComment		Comment
  HiLink taxiLastComment	Comment
  HiLink taxiBottomComment	Comment
  HiLink taxiTodo		Todo
  HiLink taxiType		Function
  HiLink taxiFunction		Function
  HiLink taxiStrFunction	String
  HiLink taxiLoops 		PreProc
  HiLink taxiNumber		Number
  HiLink taxiReal		Float

  delcommand HiLink
endif

let b:current_syntax = "taxi"
set ts=3
set nocin
set nosi
set columns=80

" vim: ts=8
