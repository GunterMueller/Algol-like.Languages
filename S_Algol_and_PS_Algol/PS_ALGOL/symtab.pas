
{$b+
-----------------------------------------------------------------
Module      : symtab.cmp
Used in     : Compiler toolbox
Author      : W P Cockshott
Date        : 27 oct 86
Version     : 1
Function    : to maintain a table relating identifiers to type
Copyright (C) WP Cockshott & P Balch
----------------------------------------------------------------
}

UNIT symtab;
INTERFACE
USES
  editdecl,
  errors,

  idtypes,
  dlb,
{$ifdef assembler}  cgendum;
{$endif}
{$ifndef assembler} sagen;
{$endif}
const
       constants:set of lexeme =
                 [cprocedure_sy,cint_sy,cfile_sy,creal_sy,cbool_sy,cpntr_sy,cstring_sy];
function eq(var t1:typerec;  var t2:typerec):boolean;

procedure balance(var t1:typerec; var t2:typerec);

procedure fixupstruct(var t:typerec;
                      var pntrs:integer;
                      var reals:integer;
                      var ints :integer);

function isvariable(t:typerec):boolean;


procedure match(var t1:typerec;var t2:typerec);

procedure coerce(var t1:typerec;var t2:typerec);

function  lookup(id :lextoken):namedesc;

function inblock(n:namedesc):boolean;

procedure appendparam(var proc:typerec; parameter:idref)   ;

procedure fixup_params(var theproc:typerec);

procedure enterlexlevel;

procedure typeof(id:lextoken; var t:typerec);

function classof(id:lextoken):namedesc;

function  newname(i:lextoken; t:typerec; sort:nametype;var ref:idref):namedesc;

function  newdummy(i:lextoken; t:typerec; sort:nametype;var ref:idref):namedesc;

procedure bindlabel(l:labl;v:idref);

procedure setresult(var proc:typerec; res:idref)          ;

procedure enterscope( var oldscope:namedesc);

procedure exitblock( oldscope:namedesc; var t:typerec);

procedure exit_proc(oldscope:namedesc);

procedure exitlexlevel;

procedure initsymtab;

procedure tablecontents(table:typerec;var contents:typerec);
procedure tableof(contents:typerec;var table:typerec);


IMPLEMENTATION
Type
    string255 = string[255];



var dummyname:namedesc;

Var
{        NAMETOP       ALWAYS POINTS ONE ABOVE LATEST DECLARATION }
    nametop       : namedesc;
    scope_level   :  byte ;
    scope_to_level : array[1..127]of byte;
    idlist: idref;

{ ------------------- new id --------------------------------------- }
procedure newid(var id:idref);
begin
   new(id);
   id^.idchain:=idlist;
   idlist:=id;
end;
{ ------------------- Table types ---------------------------------- }

procedure tablecontents(table:typerec;var contents:typerec);
begin
     contents:=table.result^.typeinfo;
end;

procedure tableof(contents:typerec;var table:typerec);
begin
     table:=VOID;
     newid(table.result);
     table.result^.typeinfo:=contents;
     table.typeid:=dollar_sy;
     table.aliasid:=ord(dollar_sy);
end;

{ ------------------------------------------------------------------ }
{         Scope handling                                             }
{ ------------------------------------------------------------------ }
procedure enterscope( var oldscope:namedesc);
{ this is called on entry to a new scope level }
{ that is either a block or a procedure        }
begin
     oldscope:= nametop;
     scope_level:=scope_level+1;
     scope_to_level[scope_level]:=lexical_level;
end;
function inblock(n:namedesc):boolean;
{ return true is name declared in this block }
begin
   inblock:=scope_level = scopelist[n]^.block_level
end;
procedure exitblock( oldscope:namedesc; var t:typerec);
label 99;
begin
    { dispose of objects declared in the block }
  scope_level:=scope_level - 1;
  if (nametop>0) and(scopelist[nametop-1]^.block_level >scope_level) then
    begin
    while (nametop>0)and
    ( scopelist[nametop-1]^.block_level >scope_level )do
    begin

          nametop:=nametop-1;
          if nametop=0 then goto 99;
    end;

    99:
    discard(scopelist[nametop]^,t);
  end
end;
procedure exitproc(oldscope:namedesc);
begin
     { exit from the scope associated with procedure parameters }
     scope_level := scope_level - 1;
     nametop:=oldscope;
end;
procedure enterlexlevel;
begin
     lexical_level:=succ(lexical_level);
end;
procedure exitlexlevel;
begin
     lexical_level:=pred(lexical_level);
end;











{ ------------------------------------------------------------------ }
{      declare types                                                 }
{ ------------------------------------------------------------------ }

procedure declare_types;

begin
     dect(VOID,UNDEFINED);
     dect(int_type,int_sy);decc(cint_type,int_type,ord(cint_sy));
     dect(bool_type,bool_sy);decc(cbool_type,bool_type,ord(cbool_sy));
     dect(file_type,file_sy);decc(cfile_type,file_type,ord(cfile_sy));
     dect(real_type,real_sy);decc(creal_type,real_type,ord(creal_sy));
     dect(pntr_type,pntr_sy);decc(cpntr_type,pntr_type,ord(cpntr_sy));
     dect(string_type,string_sy);decc(cstring_type,string_type,ord(cstring_sy));
     dect(structure_type,structure_sy);

     dect(condition_type,condition_sy);
end;

{ ------------------------------------------------------------------ }
{       SETRESULT  updates the result field of a procedure id        }
{ ------------------------------------------------------------------ }
procedure setresult(var proc:typerec; res:idref)          ;
begin
     proc.result :=res
end;
{ ------------------------------------------------------------------ }
{       APPENDPARAM  appends a new parameter to a procedure id       }
{ ------------------------------------------------------------------ }
procedure appendparam(var proc:typerec; parameter:idref)   ;
  procedure append(var p:paramref);
  begin

     if p=nil then
     begin
        new(p);p^.next:=nil;
        p^.paramname:=parameter
     end
     else append(p^.next)
  end;

begin
  append(proc.params);
end;

{ ------------------------------------------------------------------ }
{        VARIABLE  - if an item is a var                             }
{ ------------------------------------------------------------------ }
function isvariable(t:typerec):boolean;
begin
     isvariable:=(not t.constant) and
                 (t.typeid in [ int_sy ,bool_sy , real_sy ,file_sy,
                              procedure_sy,
                              string_sy , pntr_sy, dollar_sy ]);


end;
{ ------------------------------------------------------------------ }
{        EQ : compare two types                                      }
{             EQ - name equality                                     }
{             EQ2 - representation equality                          }
{ ------------------------------------------------------------------ }

function eq(var t1:typerec;  var t2:typerec):boolean;
     function eqid(n1,n2:paramref):boolean;
     begin
          if n1=n2 then eqid:=true else
          if n1=nil then eqid := false else
          if n2=nil then eqid := false else
          eqid:=eq(n1^.paramname^.typeinfo,n2^.paramname^.typeinfo) and
                eqid(n1^.next,n2^.next);
     end;
begin
     eq:=false;
     if (t1.dimensions=t2.dimensions) and( t1.field=t2.field)
     then
         if(t1.typeid =procedure_sy) then
            eq:= (eqid(t1.params,t2.params) and
            eq(t1.result^.typeinfo,t2.result^.typeinfo) )
         else eq:=((t1.aliasid=ord(t2.typeid))and t2.constant) or

                  (t1.aliasid=t2.aliasid) or
                  ((t2.aliasid= ord(t1.typeid))and t1.constant) ;
end;

function eq2(var t1:typerec;  var t2:typerec):boolean;
{ coerce t1 to a t2}
var r1,r2:typerec;
begin
      if eq(t1,t2) then eq2:=true
      else begin
          representation(t1,r1);
          eq2:=eq(r1,t2);
      end;
end;
{ ------------------------------------------------------------------ }
{      MATCH                                                         }
{ ------------------------------------------------------------------ }
procedure match(var t1:typerec;var t2:typerec);
var em:textline;
begin
     if not eq(t1,t2) then begin
        em:= ptype(t1)+' not compatible with ' +ptype(t2);
        error(em );
     end;
end;
{ ------------------------------------------------------------------ }
{     COERCE - verify representaional equality                       }
{ ------------------------------------------------------------------ }
procedure coerce(var t1:typerec;var t2:typerec);
var em:textline;
begin
     if not eq2(t1,t2) then begin
        em:= ptype(t1)+' not compatible with ' +ptype(t2);
        error(em );
     end;
end;
{ ------------------------------------------------------------------ }
{      BALANCE                                                       }
{ ------------------------------------------------------------------ }
procedure balance(var t1:typerec; var t2:typerec);
begin
     if Not eq(t1,t2) then begin
        if eq(t1,int_type) then begin
           match(t2,real_type);
           float2op;
           t1:=real_type;
        end else
        if eq(t1,real_type) then begin
           match(t2,int_type);
           floatop;
           t2:=real_type;
        end;
     end;
end;

{ ------------------------------------------------------------------ }
{        Lookup an Identifier : return zero if does not exist        }
{ ------------------------------------------------------------------ }
function  lookup(id :lextoken):namedesc;
var i:integer; label 99;
begin
   i:= pred(nametop);
   if i>0 then while  (scopelist[i]^.identifier<>id) do
   begin     i:=pred(i); if i<=0 then goto 99 end;
   99:
   if i<0 then lookup:=0 else
           lookup:=i;

end;
function lookup_by_name(t:textline):namedesc;
begin
     lookup_by_name:=lookup(tokenof(t));
end;

{ ------------------------------------------------------------------ }
{        Typeof an Identifier                                        }
{ ------------------------------------------------------------------ }
procedure typeof(id:lextoken; var t:typerec);
begin
     t:=scopelist[lookup(id)]^.typeinfo;
end;
{ ------------------------------------------------------------------ }
{        LEXLEVEL  - return the lexical lvevel of a var              }
{ ------------------------------------------------------------------ }
function lexlevel(s:lextoken):integer;
begin
    lexlevel:=scopelist[lookup(s)]^.lex_level;

end;

{ ---------------------------------------------------------------- }
{       MAKE_ID                                                    }
{ ---------------------------------------------------------------- }

function make_id(var ref:idref):namedesc;
begin

     newid(ref);scopelist[nametop]:=ref;
     dect(ref^.typeinfo,UNDEFINED);
     with ref ^ do begin
          lex_level:=lexical_level;
          block_level:=scope_level;
     end;
     make_id:=nametop;
     nametop:=nametop+1;
end;
{ ------------------------------------------------------------------- }
{ BINDLABEL                                                           }
{ ------------------------------------------------------------------- }
procedure bindlabel(l:labl;v:idref);
begin
   v^.offset:=ord(l);
end;

{------------------------------------------------------------------- }
{   Fixup params                                                     }
{------------------------------------------------------------------- }
procedure fixup_params(var theproc:typerec);
{ call the code generator to assign offsets to the parameters }
{ proceed in reverse order to declaration }
var ptotal:byte;
   procedure listchase(i:paramref);
   begin
        if i<> nil then
        with i^ do
        begin
             listchase(next);
             ptotal:=ptotal+fixup_param(paramname^);
        end;
   end;
   procedure listmodify(i:paramref);
   begin
        if i<> nil then
        with i^ do
        begin
             listmodify(next);
             correct_param(paramname^,theproc.paramspace,theproc.resultspace);
        end;
   end;
begin
    ptotal:=0;
    with theproc do begin
     listchase(params);
     paramspace:=ptotal;
     if result <>nil then resultspace:=fixup_result(result^,paramspace)
     else resultspace:=0;
     listmodify(params);
    end;
end;
{ ------------------------------------------------------------------ }
{      fixup struct                                                  }
{ ------------------------------------------------------------------ }
{ this allocates addresses to fields within a structure              }
{ in doing so it reorganises the fields into the order               }
{    trademark   pntr* real* int*                                    }
{ integers are assumed to be 4 byte ones for compatibility           }
{ with code that will run on a 386 in future                         }
procedure fixupstruct(var t:typerec;
                      var pntrs:integer;
                      var reals:integer;
                      var ints :integer);
  procedure listchase(i:paramref);
  var locreals,locints,locpntrs:integer;
   begin
        if i<> nil then
        with i^ do
        begin
              locints:=ints;
              locpntrs:=pntrs;
              locreals:=reals;
              if ispntr(paramname^.typeinfo) then  pntrs:= pntrs +1 else
                         case paramname^.typeinfo.typeid of
                         real_sy,creal_sy:reals:=reals+1;
                         else ints:=ints+1;
                         end;

             listchase(next);
             with i^ do
             if ispntr(paramname^.typeinfo) then
             paramname^.offset:= locpntrs*heappntr_size*stride
             else        with paramname^ do
                         case typeinfo.typeid of
                         real_sy,creal_sy:offset:= pntrs*heappntr_size*stride +
                                                   locreals*real_size*stride;
                         else offset:= pntrs*heappntr_size*stride +
                                       reals*real_size*stride +
                                       locints*long_int_size*stride;
                         end;


        end;
   end;
begin
      reals:=0; ints:=0;
      pntrs:=1;
      listchase(t.params);

end;
procedure exit_proc(oldscope:namedesc);
begin
     { exit from the scope associated with procedure parameters }
     scope_level := scope_level - 1;
     nametop:=oldscope;
end;


{ ------------------------------------------------------------------ }
{       already declared here : true if var defined within block     }
{ ------------------------------------------------------------------ }
function already_declared_here( id  :lextoken):boolean;
var n : integer;
begin
     n:=lookup(id);
     if n<=0 then already_declared_here:=false else
     already_declared_here:=scopelist[n]^.block_level = scope_level;
     if scopelist[n]^.block_level = scope_level  then
       writeln(psym(id),' already declared as variable ',n,' in scope ',
       scope_level);
end;

{ ------------------------------------------------------------------ }
{        Define generator functions                                  }
{ ------------------------------------------------------------------ }

function  newname(i:lextoken; t:typerec; sort:nametype;var ref:idref):namedesc;
var temp:namedesc;
begin
     if already_declared_here(i) then begin Error('Already declared here'); exit end;
     newname:=make_id(ref);
     with ref^ do begin
          identifier:=i;
          name_type:=sort;
          typeinfo:=t;
          case sort of
          global : offset:=declare_global(ref^);
          local  : offset:=declare_local(ref^);
          end;
     end;
end;
function  newdummy(i:lextoken; t:typerec; sort:nametype;var ref:idref):namedesc;
var temp:namedesc;
begin
     newdummy:=make_id(ref);
     with ref^ do begin
          identifier:=i;
          name_type:=sort;
          typeinfo:=t;
          case sort of
          global : offset:=declare_global(ref^);
          local  : offset:=declare_local(ref^);
          end;
     end;
end;


{ ------------------------------------------------------------------ }
{       INITSYMTAB initialises the symbol table                      }
{ ------------------------------------------------------------------ }
var firstinit:boolean;
procedure initsymtab;
var i:integer;
    temp:idref;
begin
       cleartypes;
       declare_types;
       nametop:=succ(nulid);
       lexical_level:=global_level;
       scope_level:=global_level;
       for i:=1 to 127 do scope_to_level[i]:=global_level;

       idlist:=nil;
       for i:= nulid to maxid do begin
         newid(scopelist[i]);
         with scopelist[i] ^ do begin
            typeinfo:=void;
            identifier:=0;
            block_level:=127;
            lex_level:=global_level;
         end;
       end;
end;

{ ------------------------------------------------------------------ }
{        Class of a field                                            }
{ ------------------------------------------------------------------ }
function classof(id:lextoken):namedesc;
var i:integer;
begin
   i:= lookup(id);
   while (i<nametop) and (scopelist[i]^.typeinfo.typeid<>structure_sy) do
        i:=succ(i);
        classof:=i;
end;






BEGIN
    firstinit:=true;
    initsymtab;
END.

///////////////////////////////////////////////////////////////

