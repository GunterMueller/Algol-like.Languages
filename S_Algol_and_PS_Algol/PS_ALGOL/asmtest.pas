program asmtest(input,output);
uses opcodes,assemble;
var cfile:codefile;
    i:integer;
begin
     initassem;
     readln;
     for i:=1 to 1000 do begin
     assem(llint,1,0);
     assem(jump,i,0);
     assem(dw,10,0);
     assem(plant_label,i,0);
     assem(plusop,0,0);
     end;
     assign(cfile,'s.com');
     rewrite(cfile,1);
     pass2(cfile);
     close(cfile);
end.