{
----------------------------------------------------
  Compiler tool box
  Copyright W P Cockshott
  Module to generate ieee floating point numbers
  Date  : 21 - 9 - 88
----------------------------------------------------

}
{
   ieee double precision numbers are 64 bits long with
   1  sign bit           s
   11 exponent bits      e
   52 magnitude bits     f

  value is defined as:
        (-1)^s*2^(e-1023) *1.f
}
UNIT reals;
INTERFACE

type ieee = array [1..8] of 0..255;
procedure realtoieee(r:real; var f:ieee);
IMPLEMENTATION

{---------------------------------}
{      REAL To IEEE               }
{---------------------------------}
procedure realtoieee(r:real; var f:ieee);
const explength = 11;
      fractlength = 52;
      bias = 1023;

var sign:(plus,minus); exponent:integer; fraction:real;
    i:integer;
    procedure Normalise;
    begin
        while fraction>1 do begin
           fraction:= fraction / 2;
           exponent := exponent + 1;
        end;
        while fraction<1 do begin
           fraction:=fraction+fraction;
           exponent := exponent-1;
        end;
        fraction:=fraction-1;
    end;
    procedure setbit(bitnum,value:integer);
    var i:integer;
    begin
       for i:=0 to ((bitnum mod 8) -1) do value:=value+value;
       f[(bitnum div 8)+1]:= f[(bitnum div 8)+1]+value;
    end;

begin
    if r<0 then sign:=minus else sign:=plus;
    if sign = minus then r := - r;
    exponent := bias;
    fraction := r;
    for i:=1 to 8 do f[i]:=0;
    if r<>0 then begin
       Normalise;
       setbit(63,ord(sign));
       for i:=1 to explength do begin
           setbit(fractlength+i-1,exponent mod 2 );
           exponent := exponent div 2;
       end;
       for i:=51  downto 0 do begin
           fraction :=fraction *2;
           if fraction >=1 then begin
             setbit(i,1);
             fraction:=fraction -1;
           end else setbit(i,0);
       end;
    end;
end;
BEGIN


END.