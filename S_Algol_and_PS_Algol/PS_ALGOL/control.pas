{$R+,B-,S+,I-,N-,L-,V-}

unit control;

interface

uses
  Crt,
  Dos,
  Screen,
  EditDecl,
  ExecT,
  Menu, mouse,
  Dir;

procedure show_menu_bar;

procedure menu_cmd(var Cmd: char;
                   Text: TextStack;
                   var cursor: word;
                   var ErrMsg: TextLine;
                   var CompileChangeFlag: boolean;
                   var Searchstr,SubsStr,Fname: string;
                   var IgnCase,Ask,Global: boolean
                   );

Implementation

const CtrlZ = #26;

var
  menu_bar: menu_header;

procedure show_menu_bar;
  var cx,cy: integer;
begin
  show_menu(cx,cy,0,menu_bar);
  gotoxy(1,1);
  write(memavail div 1024,'K ');
end;

procedure menu_cmd(var Cmd: char;
                   Text: TextStack;
                   var cursor: word;
                   var ErrMsg: TextLine;
                   var CompileChangeFlag: boolean;
                   var Searchstr,SubsStr,Fname: string;
                   var IgnCase,Ask,Global: boolean
                   );

  var cmd_out: char;

procedure del_blank(var s: string);
  var i: integer;
begin
  repeat
    i:=pos(' ',s);
    s:=copy(s,1,i-1)+copy(s,i+1,255);
  until i=0;
end;

function get_file_name(cx,cy: integer; var Fname: string; msg: string):
    boolean;
  var name,pattern,olddir: string;
begin
  Push_Help_Page(254);
  if string_box(cx,cy,20,Fname,msg) then
  begin
    get_file_name:=true;
    del_blank(Fname);
    if (Fname = '') or (pos('?',Fname) <> 0) or
       (pos('*',Fname) <> 0) then
    begin
      getdir(0,olddir);
      pattern:=Fname;
      repeat
        if show_dir(pattern,name) then
          if name[1] = '\' then chdir(copy(name,2,255));
      until name[1] <> '\';
      if name = Esc then
        get_file_name:=false else
        Fname:=name;
      chdir(olddir);
    end;
  end else
    get_file_name:=false;
  Pop_Help_Page;
end;

procedure file_menu(var cx,cy,am: integer);
  var menu: menu_header;
      olddir,s,name: string;
  const pattern: string = '';
begin
  start_menu(cx,cy+1,menu);
  menu_item('Load    F3',11,menu);
{  menu_item('Pick',      12,menu); {}
  menu_item('New',       13,menu);
  menu_item('Save    F2',14,menu);
  menu_item('Write to',  15,menu);
  menu_item('Directory', 16,menu);
  menu_item('Change dir',17,menu);
  menu_item('OS shell',  18,menu);
  menu_item('Quit',      19,menu);
  repeat
    exec_menu(cx,cy,am,menu);
    case am of
      11: if get_file_name(cx,cy+1,Fname,' Load File ') then
           cmd_out:='e' else cmd_out:=' ';
      13: cmd_out:='a';
      14: cmd_out:='b';
      15: if get_file_name(cx,cy+1,Fname,' Write File ') then
           cmd_out:='c' else cmd_out:=' ';
      16: if string_box(9,9,40,pattern,' Dir Mask ') then
        begin
          getdir(0,olddir);
          repeat
            del_blank(pattern);
            if show_dir(pattern,name) then
              if name[1] = '\' then chdir(copy(name,2,255));
          until name[1] <> '\';
          chdir(olddir);
        end;
      17: begin
          getdir(0,olddir);
          s:=' ';
          if show_dir(s,name) then
            if name[1] = '\' then
              chdir(copy(name,2,255)) else
              chdir(olddir);
        end;
      18: cmd_out:='y';
      19: cmd_out:='d';
    end;
  until (am <> 16) and (am <> 17) and (cmd <> Esc) and (cmd_out <> Esc);
  hide_menu(menu);
end;

procedure edit_menu(var cx,cy,am: integer);
  var menu: menu_header;
  function find_options(cx,cy: integer): boolean;
    const opt: string = '';
    var i: integer;
  begin
    if string_box(cx,cy,11,opt,' Options ') then
    begin
      find_options:=true;
      Global:=false;
      IgnCase:=false;
      Ask:=true;
      for i:=1 to length(opt) do
        case upcase(opt[i]) of
          'G': Global:=true;
          'U': IgnCase:=true;
          'N': Ask:=false;
        end;
    end else
      find_options:=false;
  end;
begin
  start_menu(cx,cy+1,menu);
  menu_item('Find',          21,menu);
  menu_item('Replace',       22,menu);
  menu_item('Again',         23,menu);
  menu_item('Undelete',      24,menu);
  menu_item('AutoIndent    ',25,menu);
  menu_item('Insert        ',26,menu);
  menu_item('Tab on        ',27,menu);
  repeat
    OnOff_menu_item(AutoIndent,25,menu);
    OnOff_menu_item(InsertMode,26,menu);
    if Tab_8 then change_menu_item('Tab on   Eight',27,menu) else
                  change_menu_item('Tab on   Blank',27,menu);

    exec_menu(cx,cy,am,menu);
    case am of
      21: if string_box(cx,cy+1,20,SearchStr,' Find ') then
            if find_options(cx,cy+1) then
              cmd_out:='F';
      22: if string_box(cx,cy+1,20,SearchStr,' Find ') then
            if string_box(cx,cy+1,20,SubsStr,' Replace ') then
              if find_options(cx,cy+1) then
                cmd_out:='S';
      23: cmd_out:='?';
      24: cmd_out:='U';
      25: AutoIndent:=not AutoIndent;
      26: InsertMode:=not InsertMode;
      27: Tab_8:=not Tab_8;
    end;
  until (am <> 25) and (am <> 26) and (am <> 27);
  hide_menu(menu);
end;

procedure block_menu(var cx,cy,ab: integer);
  var menu: menu_header;
begin
  start_menu(cx,cy+1,menu);
  menu_item('Begin block', 31,menu);
  menu_item('End block',   32,menu);
  menu_item('Copy block',  33,menu);
  menu_item('Move block',  34,menu);
  menu_item('Delete block',35,menu);
  menu_item('Read block',  38,menu);
  menu_item('Write block', 36,menu);
  menu_item('Print block', 37,menu);
  exec_menu(cx,cy,ab,menu);
  case ab of
    31: cmd_out:='w'; { begin block }
    32: cmd_out:='x'; { end block }
    33: cmd_out:='X'; { copy block }
    34: cmd_out:='Y'; { move block }
    35: cmd_out:='W'; { delete block }
    36: if get_file_name(cx,cy+1,Fname,' Write Block ') then
         cmd_out:='l' else cmd_out:=' '; { write block }
    37: begin         { print block }
         fname:='PRN';
         cmd_out:='l';
       end;
    38: if get_file_name(cx,cy+1,Fname,' Read Block ') then
         cmd_out:='R' else cmd_out:=' '; { read block }
  end;
  hide_menu(menu);
end;

procedure options_menu(var cx,cy,am: integer);
  var menu: menu_header;
begin
  start_menu(cx,cy+1,menu);
  menu_item('Parameters',    41,menu);
  menu_item('Help File',     42,menu);
  menu_item('Extension',     43,menu);
  menu_item('AutoSave     ', 44,menu);
  menu_item('*.ASM file',    45,menu);
  menu_item('Keyword file',  46,menu);
  menu_item('Macro library', 47,menu);
  menu_item('Assembler',     48,menu);
  menu_item('Salgol path',   49,menu);

  repeat
    OnOff_menu_item(AutoSave,44,menu);
    exec_menu(cx,cy,am,menu);
    case am of
      41: if string_box(cx,cy+1,40,Parameters,' Parameters ') then ;
      42: if string_box(cx,cy+1,40,HelpFile,' Help File ') then ;
      43: if string_box(cx,cy+1,10,extension,' Extension ') then ;
      44: AutoSave:=not AutoSave;
      45: if string_box(cx,cy+1,40,ASM,' *.ASM File ') then ;
      46: if string_box(cx,cy+1,40,lexemefile,' Keyword File ') then ;
      47: if string_box(cx,cy+1,40,MACROLIB,' Macro Library ') then ;
      48: if string_box(cx,cy+1,40,PASS2PROG,' Assembler Program ') then ;
      49: if string_box(cx,cy+1,40,PSDIR,' Salgol path ') then
          begin
            if PSDIR[length(PSDIR)] <> '\' then
              PSDIR:=PSDIR+'\';
          end;
    end;
  until (am <> 44);
  hide_menu(menu);
end;

procedure main_menu;
  var cx,cy: integer;
      ok: boolean;
  const ab:  integer = 1;
        amb: integer = 1;
        amf: integer = 1;
        ame: integer = 1;
        amo: integer = 1;
begin
  repeat
    cmd_out:=' ';
    exec_menu(cx,cy,ab,menu_bar);
    ok:=true;
    case ab of
      1: begin file_menu(cx,cy,amf); ok:=amf <> 0; end;
      2: begin edit_menu(cx,cy,ame); ok:=ame <> 0; end;
      3: cmd_out:='v'; { run }
      4: cmd_out:='u'; { compile }
      5: begin options_menu(cx,cy,amo); ok:=amo <> 0; end;
      6: begin block_menu(cx,cy,amb); ok:=amb <> 0; end;
      7: begin
           Push_Help_Page(7);
           help_screen;
           pop_help_page;
         end;
   end;
  until ok;
  show_menu_bar;
end;

begin { procedure menu_cmd }
  cmd_out:='@';

  if cmd < Esc then cmd:=char(ord(cmd)+64);
  case cmd of
    'B':      cmd_out:='#'; { general  test cmd            }
    Alt_F,Alt_E,Alt_B,Alt_R,Alt_C,Alt_O: begin { Alt-F E B R C O }
        stuffkey(cmd);
        main_menu;
      end;
    F1:     cmd_out:='@'; { F1     help                  }
    F2:     cmd_out:='b'; { F2     Save                  }
    F3:     begin         { F3     load a new file       }
                stuffkey(Alt_F+Alt_L);
                main_menu;
              end;
    F7:     cmd_out:='w'; { F7     begin block           }
    F8:     cmd_out:='x'; { F8     end block             }
    F9:     cmd_out:='u'; { F9     compile               }
    F10:     main_menu;    { F10    Top Line Menu         }
    Alt_F7:     cmd_out:='}'; { Alt-F7 Indent line           }
    Alt_F8:     cmd_out:='{'; { Alt-F8 Outdent line          }
    Alt_X:     cmd_out:='d'; { Alt-X  Exit to DOS.          }
    Esc:     cmd_out:=Esc; { Esc    nothing               }
    Home:     cmd_out:='['; { Home   Beginning of line     }
    End_ch:     cmd_out:=']'; { End    End of  line          }
    Up,'E': cmd_out:='-'; { Up     Line up               }
    Down,'X': cmd_out:='+'; { Dn     Line down             }
    Left,'S': cmd_out:='<'; { <-     Char left             }
    Right,'D': cmd_out:='>'; { ->     Char right            }
    Page_Up,'R': cmd_out:='^'; { Pg Up  Page up               }
    Page_dn,'C': cmd_out:='V'; { Pg Dn  Page down             }
    Del,'G': cmd_out:='.'; { Del    Delete right          }
    'H':      cmd_out:='B'; { BS     Delete left           }
    'I':      cmd_out:='t'; { Tab    Tab                   }
    'M':      cmd_out:='N'; { Ret    Newline               }
    Ctrl_Home:     cmd_out:='G'; { Ctrl-home  Top of Window     }
    Ctrl_End:     cmd_out:='H'; { Ctrl-end   Bot of Window     }
    Ctrl_PgDn:     cmd_out:='T'; { Ctrl-PgUp  Top of File       }
    Ctrl_PgUp:     cmd_out:='E'; { Ctrl-PgDn  Bottom of file    }
    Ctrl_Right,'F': cmd_out:='L'; { Ctrl- ->   right word        }
    Ctrl_Left,'A': cmd_out:='M'; { Ctrl- <-   left word         }
    'N':      cmd_out:='g'; { Ctrl-N     Insert Line       }
    'Y':      cmd_out:='D'; { Ctrl-Y     Delete line       }
    'L':      cmd_out:='?'; { Ctrl-L     Find again        }
    'U':      cmd_out:='U'; { Ctrl-V     undo delete       }
    'T':      cmd_out:='O'; { Ctrl-T     delete word       }
    Ins,'V': begin         { Ins,Ctrl-V Insert Mode       }
                InsertMode:=not InsertMode;
                cmd_out:=' ';
              end;
    'O','Q':  begin         { Ctrl-Q                       }
                cmd:=upcase(GetBigKey);
                if cmd < ' ' then cmd:=char(ord(cmd)+64);
                case cmd of
                  'F': begin  { Find }
                         stuffkey(Alt_E+Alt_F);
                         main_menu;
                       end;
                  'A': begin  { Replace }
                         stuffkey(Alt_E+Alt_R);
                         main_menu;
                       end;
                  'B': cmd_out:='J'; { move to begin block }
                  'K': cmd_out:='K'; { move to end block }
                  'Y': cmd_out:='Z'; { Delete to eol  }
                  'E': cmd_out:='G'; { Top of Window  }
                  'X': cmd_out:='H'; { Bot of Window  }
                  'R': cmd_out:='T'; { Top of File    }
                  'C': cmd_out:='E'; { Bottom of file }
                  'P': cmd_out:='I'; { prev cursor posn }
                  'I': begin AutoIndent:=not AutoIndent; cmd_out:=' '; end;
                  'T': begin Tab_8:=not Tab_8; cmd_out:=' '; end;
                  'S': cmd_out:='['; { Beginning of line }
                  'D': cmd_out:=']'; { End of line }
                end;
              end;
    'K':      begin         { Ctrl-K                  }
                cmd:=upcase(GetBigKey);
                if cmd < ' ' then cmd:=char(ord(cmd)+64);
                case cmd of
                  'S': cmd_out:='b'; { Save }
                  'D','Q': cmd_out:='d'; { Exit }
                  'B': cmd_out:='w'; { begin block }
                  'K': cmd_out:='x'; { end block }
                  'C': cmd_out:='X'; { copy block }
                  'V': cmd_out:='Y'; { move block }
                  'Y': cmd_out:='W'; { delete block }
                  'W': begin  { write block }
                         stuffkey(Alt_B+Alt_W);
                         main_menu;
                       end;
                  'R': begin  { read block }
                         stuffkey(Alt_B+Alt_R);
                         main_menu;
                       end;
                  'P': begin          { print block }
                         fname:='PRN';
                         cmd_out:='l';
                       end;
                end;
              end;
    (*
    Mouse_Up:     cmd_out:='-'; { mouse Up      Line up        }
    Mouse_Down:     cmd_out:='+'; { mouse Dn      Line down      }
    Mouse_Left:     cmd_out:='<'; { mouse <-      Char left      }
    Mouse_Right:     cmd_out:='>'; { mouse ->      Char right     }
    Mouse_1:     cmd_out:='w'; { button 1 down begin block    }
    Mouse_2:     main_menu;    { button 2 down main menu      }
    Mouse_0:     cmd_out:='x'; { button up     end block      }
      *)
  end;

  cmd:=cmd_out;
end;

var
  cx,cy,ab:        integer;

begin
  (*
  if DisplayType <> HercMono then
  begin            { color screen }
    in_box   :=14+5*16; { yellow on magenta }
    entry    :=13+2*16; { yellow on green }
    select   := 3+1*16; { cyan on blue }
    frame    :=15;      { white on black }
  end;
  *)
  clrscr;
  start_menu_bar(1,menu_bar);
  menu_item('File',       1,menu_bar);
  menu_item('Edit',       2,menu_bar);
  menu_item('Block',      6,menu_bar);
  menu_item('Run',        3,menu_bar);
  menu_item('Compile-F9', 4,menu_bar);
  menu_item('Options',    5,menu_bar);
  menu_item('Help-F1',    7,menu_bar);
end.



F5         Zooms and "un-zooms" window
F6         Switches window
Alt-F1     Last help screen
Alt-F3     Pick file
Alt-F5     Swap screen
Alt-F9     Compile
Alt-F10    Version
Ctrl-W                Scroll up
Ctrl-Z                Scroll down
Ctrl-QW               Last error position
Ctrl-KT               Mark single word
Ctrl-KH               Hide/display block
ctrl-QL               Restore line
ctrl-Kn               Set place marker
ctrl-Qn               Find place marker
ctrl-P                Control character prefix
ctrl-U                Abort operation
ctrl-QW               Restore error message
ctrl-Q[               Pair brackets forward
ctrl-Q]               Pair brackets backward
B         Searches backward
n         n = any number; searches n times
W         Searches for whole words only
L         Searches locally


