{
-----------------------------------------------------------------
Module      : IdTypes
Used in     : Compiler toolbox
Author      : W P Cockshott
Date        : 8 Feb 87
Version     : 2
Changes     : converted to unit 21 - 9 - 88
Function    : Declare the types and variables needed for the symbol table
Copyright (C) WP Cockshott & P Balch
----------------------------------------------------------------
}
UNIT idtypes;
INTERFACE

USES
  editdecl,
  errors,
  dlb;
const  maxid = 350;
       nulid = 0;
       global_level = 0;

type
    idref     = ^idrec;
    paramref  = ^parameter;
    nametype = (global,local,param,static);
    typerec  = record
               typeid     :lexeme;
               aliasid    :lextoken;
               aliasnum,              { type num of the representation }
               paramspace,            { number of bytes of parameters for proc }
               resultspace,           { number of bytes of result }
               dimensions : byte;     { number of vector dimensions }
               field,constant: boolean;
               params: paramref;
               result:idref;
               end;
    idrec    = record
               name_type  : nametype;
               identifier : lextoken;
               offset     : integer ;
               block_level,lex_level  : byte;
               typeinfo   : typerec;
               idchain    : idref;
               end;
     parameter=record
               paramname:idref;
               next:paramref;
               end;
    namedesc = nulid..maxid;
var
     VOID ,
     int_type ,
     cint_type ,
     bool_type ,
     cbool_type ,
     file_type ,
     cfile_type ,
     string_type ,
     cstring_type ,
     real_type ,
     creal_type ,
     pntr_type ,
     cpntr_type ,
     condition_type,
     structure_type : typerec ;
     scopelist : array[namedesc] of idref;
     lexical_level :  byte ;

{ -------- PROCEDURES -------------- }

function ptype(var t1:typerec):textline;

procedure writevar( v:namedesc;var  asmfile:text);

function lex_level_of(n:namedesc):integer;
procedure mkalias(var t,rep:typerec;synonym:lextoken;isconst:boolean);

function typecode( var t:typerec):char;

function ispntr(var t:typerec):boolean;

function istype(i:lextoken):boolean;

procedure representation(var alias:typerec; var rep:typerec);

{ declare a base type                                                  }
procedure dect(var t:typerec;i:lexeme);
{ declare a constant                                                   }
procedure decc(var t:typerec;var r:typerec;i:lextoken);
procedure alias(var t:typerec;i:lexeme;alias:lextoken;isconst:boolean;alt:byte);
procedure findtype(i:lextoken; var t:typerec);

procedure cleartypes;
IMPLEMENTATION

const maxtype =100;
type  typerange = 1..maxtype;
var typestack:array[typerange]of typerec;
    toptype:typerange;
procedure cleartypes;
begin toptype:=1; end;

procedure scant(i:lextoken; var j:typerange;var already:boolean);
     begin
          j:=toptype;
          already:=false;
          while (j>1) and not already do begin
             j:=j-1;
             already:= typestack[j].aliasid = i;
          end;


     end;
procedure alias(var t:typerec;i:lexeme;alias:lextoken;isconst:boolean;alt:byte);
     var j:typerange; already:boolean;
     begin

          with t do begin
              aliasid:=alias;
              typeid:=i;
              resultspace:=0;
              paramspace:=0;
              constant := isconst;
              aliasnum:=alt;
              dimensions:=0;
              field:=false;
              params:=nil;result:=nil;
          end;
          scant(alias,j,already);
          if already then typestack[j]:=t else begin
            if toptype=maxtype then error('too many types');
            typestack[toptype]:=t;
            toptype:=toptype+1;
          end;


     end;


{ -------------------------------------------------------------------- }
{ MKALIAS                                                              }
{ -------------------------------------------------------------------- }
procedure mkalias(var t,rep:typerec;synonym:lextoken;isconst:boolean);
var j:typerange;
begin
            if toptype=maxtype then error('too many types');
            typestack[toptype]:=rep;
            j:=toptype;
            toptype:=toptype+1;
     alias(t,rep.typeid,synonym,isconst,j);

end;

function istype(i:lextoken):boolean;
     var j:typerange; already:boolean;
     begin
        scant(i,j,already);
        istype:=already;
     end;

{ -------------------------------------------------------------------- }
{ FINDTYPE                                                             }
{       looks for a base type ( not a vector                           }
{ -------------------------------------------------------------------- }
procedure findtype(i:lextoken; var t:typerec);

     var j:typerange; already:boolean;
     begin
        scant(i,j,already);
        if not already then error('no such type');
        t:=typestack[j];

        t.dimensions:=0;
        if t.constant then begin representation(t,t);
                                 t.constant:=true;
                           end;
     end;

{ ------------------------------------------------------------------ }
{  REPRESENTATION                                                    }
{          returns the ultimate represetnation of a type             }
{ ------------------------------------------------------------------ }
procedure representation(var alias:typerec; var rep:typerec);
begin
     if alias.aliasid = ord(alias.typeid) then
     { this is a primitive }
        rep:=alias
     else begin
        representation(typestack[alias.aliasnum],rep);
        rep.dimensions:=rep.dimensions+alias.dimensions;
     end;
end;

function typecode( var t:typerec):char;
  var s:char;
      t2:typerec;
begin
     representation(t,t2);
     case t2.typeid of
     file_sy,cfile_sy,int_sy,cint_sy,bool_sy,cbool_sy:s:='i';
     UNDEFINED:s:='v';
     real_sy,creal_sy:s:='r';
     procedure_sy:s:='c';
     else s:='p';
     end;
     if t2.dimensions > 0 then s:='p';
     typecode:=s;
end;
procedure writevar( v:namedesc;var asmfile:text);
begin
     with scopelist[v] ^ do begin
     writeln(asmfile,typecode(typeinfo),' ',offset)
     end;
end;
function lex_level_of(n:namedesc):integer;
begin
     lex_level_of:=scopelist[n]^.lex_level;
end;
{ ------------------------------------------------------------------ }
{      ptype                                                         }
{ ------------------------------------------------------------------ }
function ptype(var t1:typerec):textline;
var i:integer;s:textline;trav:paramref;
begin
       with t1 do begin
            s:='';
            if field then s:='field ';
            i:=dimensions;
            if constant then s:=s+'c';
            if i < 6 then while i>0 do begin s:=s+'*'; i:=pred(i) end;
            s:=s+(psym(aliasid)) ;
            if typeid=procedure_sy then begin
               s:=s+'(';
               trav:=params;
               while trav<> nil do
               begin
                     s:=s+ptype(trav^.paramname^.typeinfo) ;
                     trav:= trav^.next;
               end;
               if result <> nil then
                 s:=s+'->'+ptype(result^.typeinfo);
               s:=s+(')');
            end else
            if result <> nil then
                 s:=s+ptype(result^.typeinfo);
       end;
       ptype:=s;
end;

function ispntr(var t:typerec):boolean;
begin
    ispntr:= typecode(t)='p';
end;






{ declare a base type                                                  }
procedure dect(var t:typerec;i:lexeme);
   begin
     alias(t,i,ord(i),false,1);
   end;
{ declare a constant                                                   }
procedure decc(var t:typerec;var r:typerec;i:lextoken);
begin
   mkalias(t,r,i,true);
end;


BEGIN
END.