Unit Dir;

{\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\\                                                                   \\
\\   ANALOGUE EDITOR TOOLBOX Version 2.00A                           \\
\\   File Name:                                                      \\
\\                                                                   \\
\\   Editor:                                                         \\
\\     Directory Services                                            \\
\\                                                                   \\
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\}

Interface

uses
  Crt,
  Dos,
  Screen,
  Menu,
  editdecl;

function DeleteFile(name: string): integer;
function show_dir(var pattern: string; var name: string): boolean;
function get_file_name(cx,cy: integer; var Fname: string; msg: string):
    boolean;

Implementation

Const
  Nul = #0;

procedure del_blank(var s: string);
  var i: integer;
begin
  repeat
    i:=pos(' ',s);
    s:=copy(s,1,i-1)+copy(s,i+1,255);
  until i=0;
end;

function upstring(str: string): string;
var  i: word;
begin
  for i:=1 to length(str) do
    str[i]:=upcase(str[i]);
  upstring:=str;
end;

{=========================================================================
=========================================================================}
procedure split_name(var dir,name: string);
  var i: integer;
begin
      {$I-}
      dir:=name;
      name:='';

      if pos(':\',dir) <> 0 then chdir(copy(dir,1,3));
      if copy(dir,1,1) = '\' then chdir('\');
      i:=IOresult;

      chdir(dir);
      i:=IOresult;
      if i <> 0 then
      begin
        if (pos('\',dir) or pos(':',dir)) <> 0 then
        begin
          while (dir[length(dir)] <> '\') and (dir[length(dir)] <> ':') do
          begin
            name:=dir[length(dir)]+name;
            dec(dir[0]);
          end;
          if dir[length(dir)] = '\' then dec(dir[0]);
        end else
        begin
          name:=dir;
          dir:='';
        end;
        chdir(dir);
        if IOresult <> 0 then exit;
      end;
      getdir(0,dir);
      if name = '' then name:='*';
      if pos('.',name) = 0 then name:=name+'.'+extension;
      {$I+}
end;

{=========================================================================
=========================================================================}
procedure del_slash(var s: string);
  var i: integer;
begin
  repeat
    i:=pos('\\',s);
    s:=copy(s,1,i-1)+copy(s,i+1,255);
  until i=0;
end;

{=========================================================================
=========================================================================}
function get_file_name(cx,cy: integer; var Fname: string; msg: string):
    boolean;
  var name,pattern,olddir: string;
begin
  AutoHelp:=false;
  if string_box(cx,cy,30,Fname,msg) then
  begin
    getdir(0,olddir);
    get_file_name:=true;
    del_blank(Fname);
    if (Fname = '') or (pos('?',Fname) <> 0) or
       (pos('*',Fname) <> 0) or (Fname[ord(Fname[0])] = ':') or
       (Fname[ord(Fname[0])] = '\') then
    begin
      pattern:=Fname;
      repeat
        if not show_dir(pattern,name) then
        begin
          get_file_name:=false;
          chdir(olddir);
          AutoHelp:=true;
          exit;
        end;
        if name[1] = '\' then chdir(copy(name,2,255));
      until name[1] <> '\';
      Fname:=name;
    end else
    begin
      split_name(pattern,Fname);
      Fname:=pattern+'\'+Fname;
      del_slash(Fname);
    end;
    chdir(olddir);
  end else
    get_file_name:=false;
  AutoHelp:=true;
end;

{=========================================================================
=========================================================================}
function DeleteFile(name: string): integer;
{ deletes a file }
  var regs: Registers;
      fname: string;
begin
  with regs do
  begin
    fname:=name+Nul;

    AX:=$4100;
    DX:=ofs(fname[1]);
    DS:=seg(fname[1]);
    MsDos(Dos.Registers(regs));
    if (Flags and 1)<>0 then DeleteFile:=AX else DeleteFile:=0;
  end;
end;

{=========================================================================
=========================================================================}
function show_dir(var pattern: string; var name: string): boolean;
  var n: integer;
      thisdir: string;
      cx,cy,am: integer;
      menu: menu_header;
  procedure make_list(pat: string; dirs: boolean; choice: integer);
    type
      DTArecord  = record
        case boolean of
          true:  ( dosreserved: array[1..21] of byte;
                   attribute: byte;
                   filetime,filedate,Sizelow,Sizehigh: integer;
                   foundname: array [1..13] of char );
          false: ( dir: array [1..64] of char );
      end;
    var regs: Registers;
        i,j: integer;
        nomatch,lastone: boolean;
        DTArec  : DTArecord;
        st: string;
  begin
    with regs,DTArec do
    begin
      split_name(thisdir,pat);

      if dirs then pat:='*.*' else
      begin
        pattern:=pat;
        if pattern[1] = '\' then pattern:=copy(pattern,2,255);
      end;
      pat:=thisdir+'\'+pat;
      del_slash(pat);

      if not dirs then menu.title:=' '+pat+' ';

      ax:=$1A00;
      ds:=seg(DTArec);
      dx:=ofs(DTArec);
      msdos(Dos.Registers(regs));

      pat:=copy(pat,1,254)+Nul;
      nomatch:=true;
      ds:=seg(pat[1]);
      dx:=ofs(pat[1]);
      cx:=$10;

      ax:=$4E00;

      repeat
        msdos(Dos.Registers(regs));
        lastone:=false;
        if (Flags and 1) > 0 then
        begin
          lastone:=(ax=2) or (ax=18);
          if not lastone then exit;
        end else
        if dirs = ((attribute and $10) > 0) then
        with DTArec do
        begin
          move(foundname[1],st[1],14);
          st[0]:=char(sizeof(st)-1);
          st[0]:=char(pos(#0,st)-1);
          if dirs then st:='\'+st;
          if st <> '\.' then
          begin
            inc(n);
            if choice = 10000 then menu_item(st,n,menu) else
            if n = choice then name:=st;
          end;
          nomatch:=false;
        end;
        ax:=$4F00;
      until lastone or (n >= choice);
    end;
  end;
begin
  AutoHelp:=false;
  name:=pattern;

  cx:=1; cy:=1;
  start_menu(cx,cy,menu);
  with menu do
  begin
    menu_kind:=big_menu;
    x2:=x1+14;
  end;

  n:=0;
  make_list(pattern,false,10000);
  make_list('*.*',true,10000);

  if menu.choicelist = nil then
    menu_item(' ',-1,menu);

  am:=0;
  exec_menu(cx,cy,am,menu);

  if am > 0 then
  begin
    n:=0;
    make_list(pattern,false,am);
    make_list('*.*',true,am);
    if name[1] = '\' then name:='\'+thisdir+'\'+name else
      name:=thisdir+'\'+name;
    del_slash(name);
    show_dir:=true;
  end else
    show_dir:=false;
  hide_menu(menu);
  AutoHelp:=true;
end;

end.

Ä ³ ¿ À Ú Ù Â Á ´ Ã Å
Í º » È É ¼ Ë Ê ¹ Ì Î
µ ¶ · ¸ ½ ¾ Æ Ç Ï Ð Ñ Ò Ó Ô Õ Ö × Ø

Unit structur;

{============================================================================
This unit defines the data structure for the neural net interface.
It contains functions and procedures for adding and deleting both
units and connections.

============================================================================}

{============================================================================
               Design of the unit and connection data structures
               =================================================
Dynamic variables are used for storing unit and connection information
for efficiency of storage and reference.
Units are represented by multi-field records and are stored in a linked
list with a root-pointer to the first item in the list.  In this way,
procedures which map across all units can be processed by moving down the
linked list.

Connections between units are separate records.  These are pointed at by
the unit records which the connection record connects, and the connection
record points, in turn, to the unit records which it connects and to
other connections which join one of the connected units.

                          <--------------------------
                     UnitA---<-->-----              ^
                       |             |              |
                       v        Connection1--->----||
                       |             |             ||
                     UnitB---<-->----|             ||
                       |                           ||
                       v                      Connection2
                       |                           |
                     UnitC------<-------------->---|

eg UnitA connects with UnitB and also with UnitC:
   Units A, B and C are linked together in a sequential linked list;
   UnitA and UnitB are connected by Connection1;
   UnitA and UnitB point to Connection1 and in turn are pointed at by it;
   UnitA and UnitC are connected by Connection2;
   UnitA and UnitC point to Connection2 and in turn are pointed at by it;
   Connection1 points at Connection2 as also connecting to UnitA

In this way, netinput and netoutput can be calculated from a
complicated network structure.

External input is represented as originating from a sensor, which is
a type of unit-record.  It is connected to other units through
a connection record.


============================================================================}


{>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<}
                                interface
{>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<}
uses
  crt,
  lex,
  filer;

const
  MAXPARAM = 10;
  maxconnect = 2500;
  current_act   =  1;                                {current activation level}
  c_weight      =  1;                                {connection weight}

{=============================================================================}
type
  act_ptr = ^act_rec;

  act_rec = record
    cycle: longint;
    active: real;
    next: act_ptr;
  end;

{=============================================================================}
type  initial_constraint =
  (fixed,                          {fixed floating-point value: initialisation}
   random_positive,       {can take on a positive random value: initialisation}
   random_negative);      {can take on a negative random value: initialisation}

{=============================================================================}

type
  procedure_ptr = (no_procedure,iac,cs,bp);
                                {type of update algorithm specified for a unit}
       {this will be exchanged for a different pointer-type in later revisions}

{=============================================================================
conn_field                                                      *  MODIFY *
an enumerated type
listing connection fields with pointers relevant to the compiler
=============================================================================}
type conn_field =
  (unit_to,
   unit_from);
{=============================================================================}


{=============================================================================}
{=============================================================================}
{ these declarations describe the model types }

type
  param_index = 1..MAXPARAM;                                         {subrange}
  param_name_array = array [param_index] of idstring;
  param_array =      array [param_index] of real;
  model_ptr = ^model_rec;
  unit_ptr = ^unit_rec;
  connect_ptr = ^connection_rec;
  unit_proc = procedure (u: unit_ptr);     { pointer to unit update procedure }
  pass_kind = (once,every,randm);

  code_rec = record
    obj_size: word;                                   {size of the object code}
    obj_code: unit_proc;              {pointer to the start of the object code}
    source_size: word;                                     {size of the source}
    source:   pointer;                     {pointer to the start of the source}
    kind: pass_kind;                                  {type of processing pass}
  end;

  pass_ptr = ^pass_rec;
  pass_rec = record
    code: code_rec;
    next: pass_ptr;
  end;


{=============================================================================
unit_rec
record defining a single unit;
the record has fields for arrays of global and unit-level parameters
and an array of dependencies which flags which unit-level parameters are
governed by the global setting;
there are pointers to the procedure which governs the updating of parameters,
to the next unit in the linked list, and to connection records which define
the network links which affect this unit;
an enumerated type field indicates the class of unit, as input/visible/hidden
=============================================================================}
unit_rec = record
  name:             idstring;                                       {unit name}
  model:            model_ptr;                               {pointer to model}
  unit_params:      param_array;                 {parameters set at unit level}
  connect_from:     connect_ptr;               {pointer to connection received}
  connect_to:       connect_ptr;                   {pointer to connection sent}
  next_unit:        unit_ptr;             {pointer to next unit in linked list}
end;

model_rec = record
  model_name: idstring;                                    {name of the model}
  init_newstart:  code_rec;                          {called by newstart menu}
  pass_update: pass_ptr;                 {called when doing sequential passes}
  model_param: param_array;       {global parameters applicable to this model}
  model_param_name: param_name_array;             {names of global parameters}
  unit_param_name: param_name_array;            {names of the unit parameters}
  conn_param_name: param_name_array;             {names of connect parameters}
  unit_param_def:  param_array;                 {defaults for unit parameters}
  conn_param_def:  param_array;              {defaults for connect parameters}
  library_model: boolean;                          {is this a built-in model?}
  done_once: boolean;                              {flag used by calc_updates}
  input_name: string[30];                                    {input file name}
  input_file: filevar;                          {file used for input patterns}
  file_is_open: boolean;                            {is the input_file open ?}
  next_model: model_ptr;                        {pointer to next model record}
end;


{=============================================================================
connect_rec
a record which represents a connecction between units;
=============================================================================}
connection_rec= record
  conn_params: param_array;                     {connection-related parameters}
  unit_to:          unit_ptr;                   {pointer to unit connecting to}
  unit_from:        unit_ptr;                 {pointer to unit connecting from}
  and_to_connect:   connect_ptr;             {pointer to subsequent connection}
  and_from_connect: connect_ptr;               {pointer to previous connection}
end;

var
  CYCLENO, NCYCLES: longint;

const
  extension = 'NET';                                   {suffix for file names}

{=============================================================================}
{ these variables are used by the compiled units }

type
  local_rec = record
      case local_kind: lexeme of
        real_sym: (l_r: real);
        conn_sym: (l_p: pointer);
      end;
var
  locals:  array[param_index] of local_rec;
  local_names: array[param_index] of idstring;
  local_max: 0..MAXPARAM;
  this_unit: unit_ptr;  { at run-time, points at the current unit }

{=============================================================================}

procedure initialise_structur;
function  ROOT_UNIT: unit_ptr;
function  ROOT_MODEL: model_ptr;

function  find_model(id: idstring): model_ptr;
function  add_model(id: idstring): boolean;
procedure delete_model(mptr: model_ptr);

function  find_unit(sname: idstring): unit_ptr;
function  add_unit(unitname: idstring): boolean;
procedure delete_unit(uname: unit_ptr);
function  is_sensor(uptr: unit_ptr): boolean;

function  add_connection(from_unit,to_unit: unit_ptr): connect_ptr;
procedure delete_connection(conn: connect_ptr);

function  get_pass(model: model_ptr; n: integer): pass_ptr;
procedure add_pass(model: model_ptr);
function  count_passes(model: model_ptr): integer;
function  pointr(p: unit_proc): pointer;

function  upstring(str: string): string;
procedure del_blank(var s: string);
procedure beep;

var
  HEAD_UNIT: unit_ptr;                        {globally available as ROOT_UNIT}

{>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<}
                             implementation
{>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<}

{==============================================================================
==============================================================================}
procedure beep;                     {produces a short sound; used for warning}
begin
  sound(1000); delay(100); nosound;
end;

{==============================================================================
==============================================================================}
{==============================================================================
==============================================================================}
function pointr(p: unit_proc): pointer;
  var pp: pointer absolute p;
begin
  pointr:=pp;
end;

var
  HEAD_MODEL: model_ptr;                     {globally available as ROOT_MODEL}

{*****************************************************************************}
{ function upstring
{ returns a string converted to uppercase
{
{*****************************************************************************}
{*****************************************************************************}
{ function is_sensor
{ is the unit a sensor ?
{
{*****************************************************************************}
function is_sensor(uptr: unit_ptr): boolean;
begin
  is_sensor:=uptr^.model^.model_name[1] = '*';
end;

{=============================================================================
function ROOT_UNIT
arguments: none
returns: the value of HEAD_UNIT unit-pointer
used to make the root pointer globally available without it being modifiable
=============================================================================}
function ROOT_UNIT: unit_ptr;
begin
  ROOT_UNIT:=HEAD_UNIT;
end;

{=============================================================================
function ROOT_MODEL
arguments: none
returns: the value of HEAD_MODEL unit-pointer
used to make the root pointer globally available without it being modifiable
=============================================================================}
function ROOT_MODEL: model_ptr;
begin
  ROOT_MODEL:=HEAD_MODEL;
end;

{=============================================================================
function find_unit
argument: string to be searched for in the unit's .name field
returns:  pointer to the unit searched for or nil when search fails
=============================================================================}
function find_unit(sname: idstring): unit_ptr;
  var temp: unit_ptr;
begin
  temp:=HEAD_UNIT;
  while (temp <> nil) and (temp^.name <> sname) do
    temp:=temp^.next_unit;
  find_unit:=temp;
end;

{=============================================================================
function add_unit
arguments: the name of the new unit, as a string
returns: true if the unit_record has been successfully added, false otherwise
The latest item added to the FRONT of the list.  HEAD_UNIT points at it
=============================================================================}
function add_unit(unitname: idstring): boolean;
  var temp: unit_ptr;
      i:  param_index;
begin
  add_unit:=false;
  if unitname = '' then exit;
  if find_unit(unitname) <> nil then exit;  { if duplicate name, return from procedure }

  new(temp);
  with temp^ do
  begin
    model:=nil;
    connect_from:=     nil;
    connect_to:=       nil;
    for i:=1 to MAXPARAM do
      unit_params[i]:= 0;           {unit parameters set to zero}
    name:=unitname;
    next_unit:=HEAD_UNIT;
  end;
  HEAD_UNIT:=temp;
  add_unit:=true;
end;

{=============================================================================
procedure delete_connection(conn: connect_ptr);
arguments: a pointer to the connection to be deleted
There are two passes through the linked list: firstly the connections_to
other connections are uncoupled and the remaining connections relinked past
them, then the connections_from are uncoupled, with necessary relinking,
and the record disposed of
=============================================================================}
procedure delete_connection(conn: connect_ptr);

    {==========================================================================
    procedure uncouple_connections_to
    arguments: name of unit to be deleted
               root connection pointer
    this removes the first connection to other connect_rec down the linked list from
    the unit_rec to be destroyed and re-makes connections between remaining units
    ==========================================================================}
    procedure uncouple_connections_to(uname: connect_ptr; var head: connect_ptr);
    begin
      if head <> nil then
      begin
        if head = uname then
          head:=head^.and_to_connect
        else
          uncouple_connections_to(uname,head^.and_to_connect);
      end;
    end;
    {==========================================================================
    procedure delete_connections_from
    arguments: name of unit to be deleted
               root connection pointer
    this removes the first connections from other connect_rec down the linked list from
    the unit_rec to be destroyed and re-makes connections between other units
    and disposes of the connection rec
    ==========================================================================}
    procedure delete_connections_from(uname: connect_ptr; var head: connect_ptr);
      var temp: connect_ptr;
    begin
      if head <> nil then
      begin
        if head = uname then
        begin
          temp:=head;
          head:=head^.and_from_connect;
          dispose(temp);
        end else
          delete_connections_from(uname,head^.and_from_connect);
      end;
    end;
    {=========================================================================}

begin
  if conn = nil then exit;
  uncouple_connections_to(conn,conn^.unit_from^.connect_to);
  delete_connections_from(conn,conn^.unit_to^.connect_from);
end;

{=============================================================================
procedure delete_unit
arguments: a pointer to the unit to be destroyed
removes a unit_rec from the linked list
connections to and from the unit are disposed of prior to the unit being destroyed
=============================================================================}
procedure delete_unit(uname: unit_ptr);

  {===========================================================================
  procedure delete(u: unit_ptr; var head: unit_ptr);
  arguments: pointer to the unit_rec to be deleted
             pointer to the unit_rec at the head of the list
  disposes of a specified unit_rec
  ===========================================================================}
  procedure delete(u: unit_ptr; var head: unit_ptr);
    var temp: unit_ptr;
  begin
    if head <> nil then
    begin
      if u = head then
      begin
        temp:=head;
        head:=head^.next_unit;
        dispose(temp);
      end else
        delete(u,head^.next_unit);
    end;
  end;
  {==========================================================================}

begin
  if uname = nil then exit;

  while uname^.connect_to <> nil do
    delete_connection(uname^.connect_to);

  while uname^.connect_from <> nil do
    delete_connection(uname^.connect_from);

  delete(uname,HEAD_UNIT);
end;

{=============================================================================
function add_connection
arguments: pointer to unit_rec connected from
           pointer to unit_rec connected to
returns:   pointer to the new connection_rec
eg unit A is connected to unit B (which is already connected to another unit)

 --------------
| name       A |<--------------------------------------------------|
| connect_to   |--------------->                                   |
| connect_from |                 -------------------               |
| next_unit    |---             | unit_to           |------        |
 --------------   |             | unit_from         |-----|--------|
 --------------   |             | and_to_connect    |     |
| name       B |<--             | and_from_connect  |------------|
| connect_to   |                 -------------------      |      |
| connect_from |----------|                               |      |
| next_unit    | <--------|--------------------------------      |
 --------------           |                                      |
                          |      -------------------             |
                          |---->| unit_to           |  <---------|
                                | unit_from         |
                                | and_to_connect    |
                                | and_from_connect  |
                                 -------------------
=============================================================================}
function add_connection(from_unit,to_unit: unit_ptr): connect_ptr;
  var connection: connect_ptr;
      i:  param_index;
begin
  add_connection:=nil;
  if (from_unit = nil) or (to_unit = nil) then exit;

  new(connection);
  add_connection:=connection;
  with connection^ do
  begin
    for i:=1 to MAXPARAM do
      conn_params[i]:= 0;           {conn parameters set to zero}
    and_from_connect:=to_unit^.connect_from;
    to_unit^.connect_from:=connection;
    and_to_connect:=from_unit^.connect_to;
    from_unit^.connect_to:=connection;
    unit_to:=to_unit;
    unit_from:=from_unit;
  end;
end;

{==============================================================================
function get_pass(model: model_ptr; n: integer);
returns a pointer to the n'th pass record for the model
if neccessary creates one or more records
==============================================================================}
function get_pass(model: model_ptr; n: integer): pass_ptr;

  {============================================================================
  procedure get(var p: pass_ptr; n: integer);
  arguments: a variable to return the pointer to the nth code record
             the pass number as an integer
  If there is no code_rec for that pass (or any earlier pass) then one is
  created.  Called recursively to move down linked list of code_rec
  ============================================================================}
  procedure get(var p: pass_ptr; n: integer);
    const nill: pointer = nil;
    var   nilp: unit_proc absolute nill;
  begin
    if n < 1 then exit;
    if p = nil then                 {if there is no code_record for this pass}
    begin
      new(p);                                           {create a code record}
      with p^ do
      begin
        code.obj_size:=0;
        code.obj_code:=nilp;
        code.source_size:=0;
        code.source:=nil;
        code.kind:=every;
        next:=nil;
      end;
    end; {if p = nil}
    get_pass:=p;
    get(p^.next,n-1);                     {called recursively on next code_rec}
  end;
  {===========================================================================}

begin
  get(model^.pass_update,n);
end;

{==============================================================================
function count_passes(model: model_ptr): integer;
returns the number of passes of the model
==============================================================================}
function count_passes(model: model_ptr): integer;
  var n: integer;
      c: pass_ptr;
begin
  n:=0;
  c:=model^.pass_update;
  while c <> nil do
  begin
    inc(n);
    c:=c^.next;
  end;
  count_passes:=n;
end;

{==============================================================================
function add_pass(model: model_ptr; n: integer);
add a new pass record to the model
==============================================================================}
procedure add_pass(model: model_ptr);
begin
  if get_pass(model,count_passes(model)+1) = nil then ;
end;

{==============================================================================
function find_model(id: idstring): model_ptr;
argument: the idstring (name) of the model to be searched for
returns:  a pointer to the model_record found or a nil pointer if none found
==============================================================================}
function find_model(id: idstring): model_ptr;
var mptr: model_ptr;
begin
  find_model:=nil;
  mptr:=HEAD_MODEL;
  while mptr <> nil do
  begin
    if  (mptr^.model_name = id) then
      find_model:=mptr;
    mptr:=mptr^.next_model;
  end;
end;
{==============================================================================
function add_model(id: idstring): boolean;
argument: idstring (model_rec name field) for new model_record
returns:  true if the model_record has been added to the list
model_record is added to the front of the list and the HEAD_MODEL pointer is
changed to point at this new record
==============================================================================}
function add_model(id: idstring): boolean;
  procedure init_code(var r: code_rec);
    const nill: pointer = nil;
    var   nilp: unit_proc absolute nill;
  begin
    with r do
    begin
      obj_size:=0;
      obj_code:=nilp;
      source_size:=0;
      source:=nil;
      kind:=every;
    end;
  end;

  var p: model_ptr;
      i: integer;
begin
  if (id = '') or (find_model(id) <> nil) then
    add_model:=false else
  begin {add_model = true}
    new(p);
    with p^ do
    begin {with p^}
      model_name:=id;
      init_code(init_newstart);

      pass_update:=nil;

      with p^ do
      begin
        for i:=1 to MAXPARAM do
        begin
          model_param[i]:=0;
          unit_param_def[i]:=0;
          conn_param_def[i]:=0;
          model_param_name[i]:='';
          unit_param_name[i]:='';
          conn_param_name[i]:='';
        end;
        unit_param_name[current_act]:=  'activation';
        conn_param_name[c_weight]:=  'weight';
        library_model:=false;
        input_name:='';
        file_is_open:=false;
        next_model:=HEAD_MODEL;
      end;
      HEAD_MODEL:=p;
    end; {with p^}
    add_model:=true;
  end;  {add_model = true}
end;

{=============================================================================
procedure delete_model(mptr: model_ptr);
argument: a pointer to the model_rec to be deleted
relinks records past the record-to-be-deleted and disposes of bypassed record
=============================================================================}
procedure delete_model(mptr: model_ptr);
  procedure del(var list,mptr: model_ptr);
  begin
    if list = nil then exit;
    if mptr = list then
    begin
      list:=list^.next_model;
      dispose(mptr);
    end else
      del(list^.next_model,mptr);
  end;
begin
  del(HEAD_MODEL,mptr);
end;

{=============================================================================
procedure initialise_structur
initialises structur unit
=============================================================================}
procedure initialise_structur;
  var i_type: param_index;
begin
  HEAD_UNIT:=nil;
  HEAD_MODEL:=nil;
end;

end.

/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\

