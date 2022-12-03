{
-----------------------------------------------------------------
Module      : FSM.cmp
Used in     : Compiler toolbox
Author      : W P Cockshott
Date        : 3 Oct 1986
Version     : 1
Function    : Finite State Transducer to perform first level lexical
              analysis.
              Splits up text in a buffer into lexemes
Copyright (C) WP Cockshott & P Balch
----------------------------------------------------------------
}
UNIT fsm;
INTERFACE  USES
  editdecl;

{$i classtab.cmp }
const
      includedepth=2;
type  textbuffer = record
              thetext: pCharSeq;
              start,finish,textlen: word;
              linenumber :integer;
              {point at the start and finish of lexeme and give length of buffer }
             end;
      string80= string[80];
var
       the_buffer:textbuffer;
       stopline  :integer;
{-------------------------------------------------------------- }
{   THE_LINE                                                    }
{   return the current line number                              }
{-------------------------------------------------------------- }
function the_line:integer;
{-------------------------------------------------------------- }
{    REWIND                                                     }
{    moves the finite state recogniser back to the start        }
{    of the text buffer                                         }
{-------------------------------------------------------------- }
procedure rewind;
{-------------------------------------------------------------- }
{      Push and Pop buffers                                     }
{      this is used to implement include files                  }
{      push_buffer saves old text buffer                        }
{      pop_buffer restores old text buffer                      }
{---------------------------------------------------------------}
function push_buffer:boolean;
function pop_buffer:boolean;
{-------------------------------------------------------------- }
{  NEWLEXEME                                                    }
{  skips start and finish to point at new lexeme             }
{  returns type of lexeme                                       }
{ ------------------------------------------------------------- }
function newlexeme(var B:textbuffer):fsmstate;

IMPLEMENTATION {---------------------------------------------------}

var
       include_sp :integer ;
            NEWSTATE:fsmstate;
       buffstack : array[1..includedepth] of textbuffer;

procedure rewind;
begin
     NEWSTATE:=startstate;
     the_buffer.start:=textbuflo;
     the_buffer.finish:=textbuflo;
     include_sp:=0;
end;

function the_line:integer;
begin
     the_line:=the_buffer.linenumber;
end;
function push_buffer:boolean;
begin
     if include_sp<includedepth then begin
        push_buffer:=true;
        include_sp:=include_sp+1;
        buffstack[include_sp]:=the_buffer;
     end else push_buffer:=false;
end;
function pop_buffer:boolean;
begin
     if include_sp>0 then begin
        pop_buffer:=true;
        with the_buffer do freemem(thetext,textlen);
        the_buffer:=buffstack[include_sp];
        include_sp:=include_sp-1;
     end else pop_buffer:=false;
end;
function newlexeme(var B:textbuffer):fsmstate;
         const transtable:array [fsmstate,charclass] of fsmstate =
         (
         {startstate}
            {operator bracket    alpha    digits   exponen}
            (opstate, brackstate, idstate, numstate,idstate,
            {  "          '          !             ;          whitespace}
             stringstate, startstate,commentstate, sepstate, startstate),

         {opstate}
            {operator bracket    alpha    digits   exponen}
            (opstate, brackstate, idstate,numstate,idstate,
            {  "          '          !             ;          whitespace}
             stringstate, startstate,commentstate, sepstate, startstate),

         {idstate}
            {operator   bracket    alpha    digits   exponen}
            (opstate, brackstate, idstate, idstate, idstate,
            {  "          '          !             ;          whitespace}
             stringstate, startstate,commentstate, sepstate, startstate),

         {numstate}
            {operator   bracket    alpha      digits   exponen}
            (opstate,   brackstate,idstate, numstate, expstate,
            {  "          '            !             ;          whitespace}
             stringstate, startstate,  commentstate, sepstate, startstate),

         {expstate}
            {operator   bracket    alpha      digits     exponen}
            (numstate,  brackstate, idstate, numstate, idstate,
            {  "          '          !             ;          whitespace}
             stringstate, startstate,  commentstate, sepstate, startstate),

         {commentstate}
            {operator      bracket       alpha         digits       exponen}
            (commentstate, commentstate, commentstate, commentstate,commentstate,
            {  "           '            !             ;          whitespace}
             commentstate, commentstate,commentstate, sepstate, commentstate),


         {stringstate}
            {operator      bracket       alpha         digits       exponen}
            (stringstate, stringstate, stringstate, stringstate,stringstate,
            {  "             '              !             ;          whitespace}
             lastquotestate, escapestate,stringstate, stringstate, stringstate),


         {escapestate}
            {operator      bracket       alpha         digits       exponen}
            (stringstate, stringstate, stringstate, stringstate,stringstate,
            {  "             '              !             ;          whitespace}
              stringstate, stringstate,stringstate, stringstate, stringstate),

         {lastquotestate}
            {operator      bracket    alpha    digits       exponen}
            (opstate, brackstate,   idstate,  numstate,   idstate,
            {  "             '              !             ;          whitespace}
              stringstate, startstate,commentstate, sepstate, startstate),

         {sepstate}
            {operator bracket    alpha    digits   exponen}
            (opstate, brackstate, idstate, numstate,idstate,
            {  "          '          !             ;     whitespace}
             stringstate, startstate,commentstate, sepstate, startstate),

         {brackstate}
            {operator bracket    alpha    digits   exponen}
            (opstate, brackstate, idstate, numstate,idstate,
            {  "          '          !             ;     whitespace}
             stringstate, startstate,commentstate, sepstate, startstate)
         );
         type action =(last,skip,include);
         const emit:array [fsmstate,charclass] of action =
         (
         {startstate}
            {operator bracket    alpha    digits   exponen}
            (skip,   skip,     skip,   skip,   skip,
            {  "          '          !          ;          whitespace}
             skip,   skip,      skip,           skip,     skip),

         {opstate}
            {operator bracket    alpha    digits   exponen}
            (include,   last,     last,   last,   last,
            {  "          '          !     ;          whitespace}
             last,   last,      last,     last,     last),

         {idstate}
            {operator bracket    alpha    digits   exponen}
            (last ,   last,      include,    include,    include,
            {  "          '          !  ;          whitespace}
             last,   last,      last,     last,     last),

         {numstate}
            {operator bracket    alpha    digits   exponen}
            (last,   last,      last,      include,    include,
            {  "          '          !  ;          whitespace}
             last,   last,      last,   last,     last),

         {expstate}
            {operator bracket    alpha    digits   exponen}
            (include,   last,      last,      include,    last,
            {  "          '          !  ;          whitespace}
             last,   last,      last,   last,     last),

         {commentstate}
            {operator bracket    alpha    digits   exponen}
            (skip,   skip,     skip,   skip,   skip,
            {  "          '      !     ;          whitespace}
             skip,   skip,      skip,  last,     skip),

         {stringstate}
            {operator bracket    alpha    digits   exponen}
            (include,   include,     include,   include,   include,
            {  "          '          !     ;          whitespace}
             include,   include,      include,   include,     include),


         {escapestate}
            {operator bracket    alpha    digits   exponen}
            (include,   include,     include,   include,   include,
            {  "          '          !     ;          whitespace}
             include,   include,      include,   include,     include),

         {lastquotestate}
            {operator bracket    alpha    digits   exponen}
            (last,   last,      last,      last,    last,
            {  "          '          !  ;          whitespace}
             last,   last,      last,   last,     last),

         {sepstate}
            {operator bracket    alpha    digits   exponen}
            (last,   last,      last,      last,    last,
            {  "          '          !  ;          whitespace}
             last,   last,      last,   last,     last),

         {brackstate}
            {operator bracket    alpha    digits   exponen}
            (last,   last,      last,      last,    last,
            {  "          '          !   ;          whitespace}
             last,   last,      last,    last,     last)
         );
         label 1,99;
         var
            S:fsmstate;
            C:charclass;
            A:action;
            T:textbuffer absolute the_buffer;
            I:integer;
         begin
              1:

              t.start:=t.finish;
              if listprog then
              { put the condition outside the loop to prevent things
                being slowed down too much }
              repeat
                 S:=NEWSTATE;
                 t.finish := t.finish+1;
                 I:=ord(T.thetext^[t.finish] );
                 write(listfile,chr(i));
                 if I= 10 then begin
                     t.linenumber:=t.linenumber+1;

                 end ;

                 C:=classtab[I and 127];
                 NEWSTATE:= transtable[S,C];
                 A:=emit[S,C];
                 if A= skip then t.start:=t.finish ;   { mark start of lexeme }
                 if t.finish >= t.textlen then begin
                 { HANDLE RUNNING OUT OF THE BUFFER }
                    if pop_buffer then goto 1 ;
                    a:=last;
                 end;
              until( A=last)
              else
              repeat
                 S:=NEWSTATE;
                 t.finish := t.finish+1;
                 I:=ord(T.thetext^[t.finish] );
                 if I= 10 then begin
                     t.linenumber:=t.linenumber+1;
                 end ;

                 C:=classtab[I and 127];
                 NEWSTATE:= transtable[S,C];
                 A:=emit[S,C];
                 if A= skip then t.start:=t.finish ;   { mark start of lexeme }
                 if t.finish >= t.textlen then begin
                 { HANDLE RUNNING OUT OF THE BUFFER }
                    if pop_buffer then goto 1 ;
                    a:=last;
                 end;
              until( A=last) ;
              99:

              newlexeme:=S;
         end;

var i:integer;

begin
       NEWSTATE:=startstate;
       listprog:=false;
       include_sp := 0;
end.