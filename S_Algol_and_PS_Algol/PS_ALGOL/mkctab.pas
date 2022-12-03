{
-----------------------------------------------------------------
Program     : Mkctab
Used in     : Compiler toolbox
Author      : W P Cockshott
Date        : 3 Oct 1986
Version     : 1
Function    : to build a class table include file for the
              lexical analyser
Copyright (C) WP Cockshott & P Balch
----------------------------------------------------------------
}
{ p100}
const opsy:set of char=[':',  '-', '=', '+', '@','&','^','%','`',  '_','$',
                  '?', '/', '>', '<','~','|'] ;

      sepsy:set of char = [';'];
      bracket: set of char = [')', '(', '{', '}', ']', '[','*',','  ]           ;
      alpha:set of char = ['a'..'z','A'..'Z'];
      digits:set of char = ['0'..'9','.'];
      alphanum:set of char = ['a'..'z','A'..'Z','.','0'..'9','#'];
      spacechars:set of char =[' '];
var c:char;
begin
     writeln('type fsmstate =(startstate,opstate,idstate,numstate,          ');
     writeln('                expstate,commentstate,stringstate,escapestate,');
     writeln('                lastquotestate,sepstate,brackstate);');
     writeln('     charclass=(operator,bracket,alpha,digits,exponent,dquote,');
     writeln('                quote,shriek,separator,whitespace);'           );
     writeln('const classtab:array[0..127] of charclass =(');
     for c:=chr(0) to chr(127) do begin
           if (ord(c) mod 4) = 0 then writeln;
           write(' { ');
           if c < chr(32) then write('^',chr(64+ord(c)),'} ')
           else if c='}' then write('closing bracket}')
           else if c=chr(127) then write('del}')
           else write(c,'} ');

           if c in opsy then write('operator,') else
           if c in ['E','e']  then write('exponent,') else
           if c in alpha then write('alpha,') else
           if c in digits then write('digits,') else
           if c in bracket then write('bracket,') else
           if c = '!' then write ('shriek,') else
           if (c=chr(13)) or (c in sepsy)  then write('separator,') else
           if c = ''''  then write('quote,') else
           if c = '"'   then write('dquote,') else
           begin
              write ('whitespace');
              if ord(c)<>127 then write(',') else writeln;
           end;
    end;
    writeln(');           { end of classtab }');

end.




