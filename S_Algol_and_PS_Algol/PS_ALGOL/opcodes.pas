unit opcodes;
{ Copyright (c) Paul Cockshott}
interface
type opcode = (
 jl,
 jle,
 jg,
 jge,
 je,
 jnz,
 jump,
 jumpt,
 jumptt,
 cjump,
 fortest,
 forstep,
 outbyte,
 shrink,
 enterframe,
 exitframe,
 prolog,
 db,
 mov_ax_sp,
 mov_bx_sp,
 mov_bp_sp,
 push_ax,
 pop_ax,
 push_bp,
 prolog86,
 pushdisp,
 push_bx,
 pop_bx,
 cslea_ax,
 add_ax,
 lea_ax,
 mov_ax,
 push_ss,
 dw,
 epilog,
 formclos,
 call_static,
 start_paramsi,
 start_paramsv,
 start_paramsr,
 start_paramsp,
 call_glob,
 call_loc,
 call_,
 llint,
 int18,
 int21,
 pushreal,
 pushp,
 heappushreal,
 heappushp,
 popreal,
 popp,
 heappopreal,
 heappopp,
 movp,
 movr,
 add_sp,
 cmp_ax,
 ldssi,
 pushfarpntr,
 sspushfarpntr,
 llreal_op,
 globali,
 globaladdr,
 globalp,
 globalproc,
 globalr,
 locali,
 localaddr,
 localp,
 localproc,
 localr,
 getlev,
 loadi,
 loadaddr,
 loadr,
 loadp,
 loadproc,
 assi,
 assr,
 assp,
 globalassi,
 globalassr,
 globalassp,
 localassi,
 localassr,
 localassp,
 localassproc,
 localmovi,
 localmovr,
 localmovp,
 discard_globali,
 neg_op,
 band,
 bor,
 shiftr,
 shiftl,
 plusop,
 not_op,
 minusop,
 multop,
 divop,
 remop,
 cmpop,
 mov_di_ax,
 mov_si_ax,
 mov_es_dx,
 pop_es_di,
 push_es_si,
 add_di,
 add_si,
 forprep,
 minfortest,
 minforstep,
 start_write,
 end_write,
 calld,
 callc,
 writeint,
 writestring,
 writepntr,
 writebool,
 writereal,
 makevec_op,
 subv_opi,
 subvass_opi,
 subv_opr,
 subvass_opr,
 subv_opp,
 subvass_opp,
 cmpops,
 cmoppntr,
 substr_op,
 concat_op,
stringlit,reallit,trademark,plant_label);
opmnemonic=string[17];
opstr = string[50];
optype =(nonadic,monadic,dyadic,stringadic,byteadic,relative,byterel,abslabel);
const codeparams:array[opcode]of optype =(
{jl}byterel,
{jle}byterel,
{jg}byterel,
{jge}byterel,
{je}byterel,
{jnz}byterel,
{jump}relative,
{jumpt}relative,
{jumptt}relative,
{cjump}relative,
{fortest}relative,
{forstep}relative,
{outbyte}nonadic,
{shrink}nonadic,
{enterframe}nonadic,
{exitframe}nonadic,
{prolog}byteadic,
{db}byteadic,
{mov_ax_sp}nonadic,
{mov_bx_sp}nonadic,
{mov_bp_sp}nonadic,
{push_ax}nonadic,
{pop_ax}nonadic,
{push_bp}nonadic,
{prolog86}nonadic,
{pushdisp}nonadic,
{push_bx}nonadic,
{pop_bx}nonadic,
{cslea_ax}abslabel,
{add_ax}monadic,
{lea_ax}monadic,
{mov_ax}monadic,
{push_ss}nonadic,
{dw}monadic,
{epilog}monadic,
{formclos}abslabel,
{call_static}abslabel,
{start_paramsi}nonadic,
{start_paramsv}nonadic,
{start_paramsr}nonadic,
{start_paramsp}nonadic,
{call_glob}monadic,
{call_loc}monadic,
{call_}dyadic,
{llint}monadic,
{int18}nonadic,
{int21}nonadic,
{pushreal}nonadic,
{pushp}nonadic,
{heappushreal}nonadic,
{heappushp}nonadic,
{popreal}nonadic,
{popp}nonadic,
{heappopreal}nonadic,
{heappopp}nonadic,
{movp}nonadic,
{movr}nonadic,
{add_sp}monadic,
{cmp_ax}monadic,
{ldssi}monadic,
{pushfarpntr}abslabel,
{sspushfarpntr}abslabel,
{llreal_op}abslabel,
{globali}monadic,
{globaladdr}monadic,
{globalp}monadic,
{globalproc}monadic,
{globalr}monadic,
{locali}monadic,
{localaddr}monadic,
{localp}monadic,
{localproc}monadic,
{localr}monadic,
{getlev}monadic,
{loadi}dyadic,
{loadaddr}dyadic,
{loadr}dyadic,
{loadp}dyadic,
{loadproc}dyadic,
{assi}dyadic,
{assr}dyadic,
{assp}dyadic,
{globalassi}monadic,
{globalassr}monadic,
{globalassp}monadic,
{localassi}monadic,
{localassr}monadic,
{localassp}monadic,
{localassproc}monadic,
{localmovi}monadic,
{localmovr}monadic,
{localmovp}monadic,
{discard_globali}monadic,
{neg_op}nonadic,
{band}nonadic,
{bor}nonadic,
{shiftr}nonadic,
{shiftl}nonadic,
{plusop}nonadic,
{not_op}nonadic,
{minusop}nonadic,
{multop}nonadic,
{divop}nonadic,
{remop}nonadic,
{cmpop}nonadic,
{mov_di_ax}nonadic,
{mov_si_ax}nonadic,
{mov_es_dx}nonadic,
{pop_es_di}monadic,
{push_es_si}monadic,
{add_di}monadic,
{add_si}monadic,
{forprep}nonadic,
{minfortest}relative,
{minforstep}relative,
{start_write}nonadic,
{end_write}nonadic,
{calld}relative,
{callc}stringadic,
{writeint}nonadic,
{writestring}nonadic,
{writepntr}nonadic,
{writebool}nonadic,
{writereal}nonadic,
{makevec_op}nonadic,
{subv_opi}nonadic,
{subvass_opi}nonadic,
{subv_opr}nonadic,
{subvass_opr}nonadic,
{subv_opp}nonadic,
{subvass_opp}nonadic,
{cmpops}nonadic,
{cmoppntr}nonadic,
{substr_op}nonadic,
{concat_op}nonadic,
nonadic,nonadic,nonadic,nonadic);
const codelits:array[opcode]of opmnemonic=(
'jl',
'jle',
'jg',
'jge',
'je',
'jnz',
'jump',
'jumpt',
'jumptt',
'cjump',
'fortest',
'forstep',
'outbyte',
'shrink',
'enterframe',
'exitframe',
'prolog',
'db',
'mov_ax_sp',
'mov_bx_sp',
'mov_bp_sp',
'push_ax',
'pop_ax',
'push_bp',
'prolog86',
'pushdisp',
'push_bx',
'pop_bx',
'cslea_ax',
'add_ax',
'lea_ax',
'mov_ax',
'push_ss',
'dw',
'epilog',
'formclos',
'call_static',
'start_paramsi',
'start_paramsv',
'start_paramsr',
'start_paramsp',
'call_glob',
'call_loc',
'call_',
'llint',
'int18',
'int21',
'pushreal',
'pushp',
'heappushreal',
'heappushp',
'popreal',
'popp',
'heappopreal',
'heappopp',
'movp',
'movr',
'add_sp',
'cmp_ax',
'ldssi',
'pushfarpntr',
'sspushfarpntr',
'llreal_op',
'globali',
'globaladdr',
'globalp',
'globalproc',
'globalr',
'locali',
'localaddr',
'localp',
'localproc',
'localr',
'getlev',
'loadi',
'loadaddr',
'loadr',
'loadp',
'loadproc',
'assi',
'assr',
'assp',
'globalassi',
'globalassr',
'globalassp',
'localassi',
'localassr',
'localassp',
'localassproc',
'localmovi',
'localmovr',
'localmovp',
'discard_globali',
'neg_op',
'band',
'bor',
'shiftr',
'shiftl',
'plusop',
'not_op',
'minusop',
'multop',
'divop',
'remop',
'cmpop',
'mov_di_ax',
'mov_si_ax',
'mov_es_dx',
'pop_es_di',
'push_es_si',
'add_di',
'add_si',
'forprep',
'minfortest',
'minforstep',
'start_write',
'end_write',
'calld',
'callc',
'writeint',
'writestring',
'writepntr',
'writebool',
'writereal',
'makevec_op',
'subv_opi',
'subvass_opi',
'subv_opr',
'subvass_opr',
'subv_opp',
'subvass_opp',
'cmpops',
'cmoppntr',
'substr_op',
'concat_op',
'stringlit','reallit','trademark', 'plant_label');
codelen:array[opcode] of byte=(
2,
2,
2,
2,
2,
2,
3,
8,
10,
11,
24,
11,
19,
7,
2,
2,
4,
1,
2,
2,
2,
1,
1,
1,
5,
6,
1,
1,
3,
3,
3,
3,
1,
2,
6,
5,
6,
1,
0,
3,
3,
5,
4,
9,
4,
2,
2,
11,
14,
18,
18,
11,
12,
17,
12,
17,
17,
4,
3,
7,
5,
5,
20,
5,
6,
18,
9,
15,
4,
6,
18,
9,
15,
4,
9,
10,
19,
22,
13,
8,
19,
20,
5,
15,
16,
4,
15,
16,
9,
6,
21,
21,
4,
4,
5,
5,
5,
5,
5,
5,
5,
5,
6,
6,
4,
2,
2,
2,
5,
5,
4,
4,
8,
7,
7,
4,
1,
5,
9,
12,
12,
12,
12,
12,
13,
12,
15,
11,
14,
11,
14,
13,
14,
14,
15,
0,0,0,0);
codeoffset:array[opcode]of integer=(
{jl}   275,
{jle}   277,
{jg}   279,
{jge}   281,
{je}   283,
{jnz}   285,
{jump}   287,
{jumpt}   290,
{jumptt}   298,
{cjump}   308,
{fortest}   319,
{forstep}   343,
{outbyte}   354,
{shrink}   373,
{enterframe}   380,
{exitframe}   382,
{prolog}   384,
{db}   388,
{mov_ax_sp}   389,
{mov_bx_sp}   391,
{mov_bp_sp}   393,
{push_ax}   395,
{pop_ax}   396,
{push_bp}   397,
{prolog86}   398,
{pushdisp}   403,
{push_bx}   409,
{pop_bx}   410,
{cslea_ax}   411,
{add_ax}   414,
{lea_ax}   417,
{mov_ax}   420,
{push_ss}   423,
{dw}   424,
{epilog}   426,
{formclos}   432,
{call_static}   437,
{start_paramsi}   443,
{start_paramsv}   444,
{start_paramsr}   444,
{start_paramsp}   447,
{call_glob}   450,
{call_loc}   455,
{call_}   459,
{llint}   468,
{int18}   472,
{int21}   474,
{pushreal}   476,
{pushp}   487,
{heappushreal}   501,
{heappushp}   519,
{popreal}   537,
{popp}   548,
{heappopreal}   560,
{heappopp}   577,
{movp}   589,
{movr}   606,
{add_sp}   623,
{cmp_ax}   627,
{ldssi}   630,
{pushfarpntr}   637,
{sspushfarpntr}   642,
{llreal_op}   647,
{globali}   667,
{globaladdr}   672,
{globalp}   678,
{globalproc}   696,
{globalr}   705,
{locali}   720,
{localaddr}   724,
{localp}   730,
{localproc}   748,
{localr}   757,
{getlev}   772,
{loadi}   776,
{loadaddr}   785,
{loadr}   795,
{loadp}   814,
{loadproc}   836,
{assi}   849,
{assr}   857,
{assp}   876,
{globalassi}   896,
{globalassr}   901,
{globalassp}   916,
{localassi}   932,
{localassr}   936,
{localassp}   951,
{localassproc}   967,
{localmovi}   976,
{localmovr}   982,
{localmovp}   1003,
{discard_globali}   1024,
{neg_op}   1028,
{band}   1032,
{bor}   1037,
{shiftr}   1042,
{shiftl}   1047,
{plusop}   1052,
{not_op}   1057,
{minusop}   1062,
{multop}   1067,
{divop}   1072,
{remop}   1078,
{cmpop}   1084,
{mov_di_ax}   1088,
{mov_si_ax}   1090,
{mov_es_dx}   1092,
{pop_es_di}   1094,
{push_es_si}   1099,
{add_di}   1104,
{add_si}   1108,
{forprep}   1112,
{minfortest}   1120,
{minforstep}   1127,
{start_write}   1134,
{end_write}   1138,
{calld}   1139,
{callc}   1144,
{writeint}   1153,
{writestring}   1165,
{writepntr}   1177,
{writebool}   1189,
{writereal}   1201,
{makevec_op}   1213,
{subv_opi}   1226,
{subvass_opi}   1238,
{subv_opr}   1253,
{subvass_opr}   1264,
{subv_opp}   1278,
{subvass_opp}   1289,
{cmpops}   1303,
{cmoppntr}   1316,
{substr_op}   1330,
{concat_op}   1344,
0,0,0,0);
const codelib:array[275..1360]of byte=(


{jl}124,97,

{jle}126,97,

{jg}127,97,

{jge}125,97,

{je}116,97,

{jnz}117,97,

{jump}233,23,46,

{jumpt}88,11,192,116,3,233,15,46,

{jumptt}88,80,
11,192,116,250,233,6,46,88,

{cjump}89,88,80,
59,193,117,4,88,233,250,45,

{fortest}89,90,88,131,249,0,124,7,
57,208,126,9,233,235,45,57,
208,125,2,235,247,80,82,81,

{forstep}89,90,88,
3,193,80,82,81,233,215,45,

{outbyte}88,90,89,
81,83,139,217,185,1,0,80,
1,226,180,64,205,33,88,91,

{shrink}180,74,187,0,16,205,33,

{enterframe}85,83,

{exitframe}91,93,

{prolog}200,0,0,199,

{db}199,

{mov_ax_sp}137,224,

{mov_bx_sp}139,220,

{mov_bp_sp}137,229,

{push_ax}80,

{pop_ax}88,

{push_bp}85,

{prolog86}85,139,253,137,229,

{pushdisp}131,239,2,54,255,53,

{push_bx}83,

{pop_bx}91,

{cslea_ax}184,57,48,

{add_ax}5,57,48,

{lea_ax}184,57,48,

{mov_ax}184,57,48,

{push_ss}22,

{dw}57,48,

{epilog}137,236,93,202,57,48,

{formclos}14,184,57,48,80,

{call_static}14,184,57,48,255,208,

{start_paramsi}80,

{start_paramsv}131,
{start_paramsr}236,4,131,

{start_paramsp}236,8,54,

{call_glob}255,159,57,48,255,

{call_loc}158,57,48,139,

{call_}182,
57,48,54,255,156,160,91,184,

{llint}57,48,80,205,

{int18}24,205,

{int21}33,255,

{pushreal}116,6,255,
116,4,255,116,2,255,52,131,

{pushp}196,240,140,215,142,199,
139,252,185,8,0,243,165,131,

{heappushreal}196,248,
139,252,22,7,185,4,0,140,
216,142,218,243,165,142,216,131,

{heappushp}196,240,
139,252,22,7,185,8,0,140,
216,142,218,243,165,142,216,143,

{popreal}5,143,69,
2,143,69,4,143,69,6,139,

{popp}244,30,7,185,
8,0,243,165,131,196,16,142,

{heappopreal}194,
38,143,5,38,143,69,2,38,
143,69,4,38,143,69,6,139,

{heappopp}244,142,194,185,
8,0,243,165,131,196,16,139,

{movp}244,
131,198,14,131,199,14,253,22,
7,185,8,0,243,165,252,139,

{movr}244,
131,198,6,131,199,6,253,22,
7,185,4,0,243,165,252,129,

{add_sp}196,57,48,61,

{cmp_ax}57,48,140,

{ldssi}200,142,216,190,57,48,14,

{pushfarpntr}184,57,48,80,22,

{sspushfarpntr}184,57,48,80,140,

{llreal_op}200,142,216,190,
57,48,255,116,6,255,116,4,
255,116,2,255,52,22,31,54,

{globali}255,183,57,48,141,

{globaladdr}135,57,48,22,80,141,

{globalp}183,57,
48,131,196,240,140,215,142,199,
139,252,185,8,0,243,165,141,

{globalproc}183,
57,48,255,116,2,255,52,141,

{globalr}183,57,48,255,116,6,255,
116,4,255,116,2,255,52,255,

{locali}182,57,48,141,

{localaddr}134,57,48,22,80,141,

{localp}182,57,
48,131,196,240,140,215,142,199,
139,252,185,8,0,243,165,141,

{localproc}182,
57,48,255,116,2,255,52,141,

{localr}182,57,48,255,116,6,255,
116,4,255,116,2,255,52,139,

{getlev}182,57,48,139,

{loadi}182,
57,48,54,255,180,160,91,139,

{loadaddr}182,57,
48,129,198,160,91,22,86,139,

{loadr}182,57,48,
141,180,160,91,255,116,6,255,
116,4,255,116,2,255,52,139,

{loadp}182,57,48,141,180,160,
91,131,196,240,140,215,142,199,
139,252,185,8,0,243,165,139,

{loadproc}182,57,48,141,180,
160,91,255,116,2,255,52,139,

{assi}182,57,48,143,132,160,91,139,

{assr}182,57,48,
141,188,160,91,143,5,143,69,
2,143,69,4,143,69,6,139,

{assp}182,57,48,141,
188,160,91,139,244,30,7,185,
8,0,243,165,131,196,16,54,

{globalassi}143,135,57,48,141,

{globalassr}191,57,48,143,5,143,69,
2,143,69,4,143,69,6,141,

{globalassp}191,57,48,139,244,30,7,185,
8,0,243,165,131,196,16,143,

{localassi}134,57,48,141,

{localassr}190,57,48,143,5,143,69,
2,143,69,4,143,69,6,141,

{localassp}190,57,48,139,244,30,7,185,
8,0,243,165,131,196,16,141,

{localassproc}190,
57,48,143,5,143,69,2,88,

{localmovi}80,137,134,57,48,141,

{localmovr}190,57,48,139,244,
131,198,6,131,199,6,253,22,
7,185,4,0,243,165,252,141,

{localmovp}190,57,48,139,244,
131,198,14,131,199,14,253,22,
7,185,8,0,243,165,252,129,

{discard_globali}196,57,48,88,

{neg_op}247,216,80,89,

{band}88,35,193,80,89,

{bor}88,11,193,80,89,

{shiftr}88,211,232,80,89,

{shiftl}88,211,224,80,89,

{plusop}88,3,193,80,88,

{not_op}53,1,0,80,89,

{minusop}88,43,193,80,89,

{multop}88,247,233,80,89,

{divop}88,153,247,249,80,89,

{remop}88,153,247,249,82,89,

{cmpop}88,59,193,139,

{mov_di_ax}248,139,

{mov_si_ax}240,142,

{mov_es_dx}194,38,

{pop_es_di}143,133,57,48,38,

{push_es_si}255,180,57,48,129,

{add_di}199,57,48,129,

{add_si}198,57,48,89,

{forprep}88,80,43,200,131,193,2,226,

{minfortest}4,88,233,211,42,81,89,

{minforstep}88,64,80,233,203,42,184,

{start_write}1,0,80,88,

{end_write}83,

{calld}232,194,42,91,83,

{callc}205,
24,97,98,99,100,0,91,83,

{writeint}205,24,95,119,
114,105,116,101,105,0,91,83,

{writestring}205,24,95,119,
114,105,116,101,115,0,91,83,

{writepntr}205,24,95,119,
114,105,116,101,112,0,91,83,

{writebool}205,24,95,119,
114,105,116,101,98,0,91,83,

{writereal}205,24,95,119,
114,105,116,101,114,0,91,83,

{makevec_op}205,24,95,109,97,
107,101,118,101,99,0,91,83,

{subv_opi}205,24,95,115,
117,98,118,105,98,0,91,83,

{subvass_opi}205,24,95,115,117,98,118,
97,115,115,105,98,0,91,83,

{subv_opr}205,24,95,
115,117,98,118,114,0,91,83,

{subvass_opr}205,24,95,115,117,98,
118,97,115,115,114,0,91,83,

{subv_opp}205,24,95,
115,117,98,118,112,0,91,83,

{subvass_opp}205,24,95,115,117,98,
118,97,115,115,112,0,91,83,

{cmpops}205,24,95,99,111,
109,112,115,116,114,0,91,83,

{cmoppntr}205,24,95,99,111,109,
112,112,110,116,114,0,91,83,

{substr_op}205,24,95,115,117,98,
115,116,114,111,112,0,91,83,

{concat_op}205,24,95,99,111,110,99,
97,116,0,91,131,196,16,0,0);
param1:array[opcode] of byte=(
{jl}1,
{jle}1,
{jg}1,
{jge}1,
{je}1,
{jnz}1,
{jump}1,
{jumpt}6,
{jumptt}7,
{cjump}9,
{fortest}13,
{forstep}9,
{outbyte}0,
{shrink}0,
{enterframe}0,
{exitframe}0,
{prolog}3,
{db}0,
{mov_ax_sp}0,
{mov_bx_sp}0,
{mov_bp_sp}0,
{push_ax}0,
{pop_ax}0,
{push_bp}0,
{prolog86}0,
{pushdisp}0,
{push_bx}0,
{pop_bx}0,
{cslea_ax}1,
{add_ax}1,
{lea_ax}1,
{mov_ax}1,
{push_ss}0,
{dw}0,
{epilog}4,
{formclos}2,
{call_static}2,
{start_paramsi}0,
{start_paramsv}0,
{start_paramsr}0,
{start_paramsp}0,
{call_glob}3,
{call_loc}2,
{call_}2,
{llint}1,
{int18}0,
{int21}0,
{pushreal}0,
{pushp}0,
{heappushreal}0,
{heappushp}0,
{popreal}0,
{popp}0,
{heappopreal}0,
{heappopp}0,
{movp}0,
{movr}0,
{add_sp}2,
{cmp_ax}1,
{ldssi}5,
{pushfarpntr}2,
{sspushfarpntr}2,
{llreal_op}5,
{globali}3,
{globaladdr}2,
{globalp}2,
{globalproc}2,
{globalr}2,
{locali}2,
{localaddr}2,
{localp}2,
{localproc}2,
{localr}2,
{getlev}2,
{loadi}2,
{loadaddr}2,
{loadr}2,
{loadp}2,
{loadproc}2,
{assi}2,
{assr}2,
{assp}2,
{globalassi}3,
{globalassr}2,
{globalassp}2,
{localassi}2,
{localassr}2,
{localassp}2,
{localassproc}2,
{localmovi}4,
{localmovr}2,
{localmovp}2,
{discard_globali}2,
{neg_op}0,
{band}0,
{bor}0,
{shiftr}0,
{shiftl}0,
{plusop}0,
{not_op}0,
{minusop}0,
{multop}0,
{divop}0,
{remop}0,
{cmpop}0,
{mov_di_ax}0,
{mov_si_ax}0,
{mov_es_dx}0,
{pop_es_di}3,
{push_es_si}3,
{add_di}2,
{add_si}2,
{forprep}0,
{minfortest}4,
{minforstep}5,
{start_write}0,
{end_write}0,
{calld}2,
{callc}3,
{writeint}0,
{writestring}0,
{writepntr}0,
{writebool}0,
{writereal}0,
{makevec_op}0,
{subv_opi}0,
{subvass_opi}0,
{subv_opr}0,
{subvass_opr}0,
{subv_opp}0,
{subvass_opp}0,
{cmpops}0,
{cmoppntr}0,
{substr_op}0,
{concat_op}0,
0,0,0,0);
param2:array[opcode] of byte=(
{jl}0,
{jle}0,
{jg}0,
{jge}0,
{je}0,
{jnz}0,
{jump}0,
{jumpt}0,
{jumptt}0,
{cjump}0,
{fortest}0,
{forstep}0,
{outbyte}0,
{shrink}0,
{enterframe}0,
{exitframe}0,
{prolog}0,
{db}0,
{mov_ax_sp}0,
{mov_bx_sp}0,
{mov_bp_sp}0,
{push_ax}0,
{pop_ax}0,
{push_bp}0,
{prolog86}0,
{pushdisp}0,
{push_bx}0,
{pop_bx}0,
{cslea_ax}0,
{add_ax}0,
{lea_ax}0,
{mov_ax}0,
{push_ss}0,
{dw}0,
{epilog}0,
{formclos}0,
{call_static}0,
{start_paramsi}0,
{start_paramsv}0,
{start_paramsr}0,
{start_paramsp}0,
{call_glob}0,
{call_loc}0,
{call_}7,
{llint}0,
{int18}0,
{int21}0,
{pushreal}0,
{pushp}0,
{heappushreal}0,
{heappushp}0,
{popreal}0,
{popp}0,
{heappopreal}0,
{heappopp}0,
{movp}0,
{movr}0,
{add_sp}0,
{cmp_ax}0,
{ldssi}0,
{pushfarpntr}0,
{sspushfarpntr}0,
{llreal_op}0,
{globali}0,
{globaladdr}0,
{globalp}0,
{globalproc}0,
{globalr}0,
{locali}0,
{localaddr}0,
{localp}0,
{localproc}0,
{localr}0,
{getlev}0,
{loadi}7,
{loadaddr}6,
{loadr}6,
{loadp}6,
{loadproc}6,
{assi}6,
{assr}6,
{assp}6,
{globalassi}0,
{globalassr}0,
{globalassp}0,
{localassi}0,
{localassr}0,
{localassp}0,
{localassproc}0,
{localmovi}0,
{localmovr}0,
{localmovp}0,
{discard_globali}0,
{neg_op}0,
{band}0,
{bor}0,
{shiftr}0,
{shiftl}0,
{plusop}0,
{not_op}0,
{minusop}0,
{multop}0,
{divop}0,
{remop}0,
{cmpop}0,
{mov_di_ax}0,
{mov_si_ax}0,
{mov_es_dx}0,
{pop_es_di}0,
{push_es_si}0,
{add_di}0,
{add_si}0,
{forprep}0,
{minfortest}0,
{minforstep}0,
{start_write}0,
{end_write}0,
{calld}0,
{callc}0,
{writeint}0,
{writestring}0,
{writepntr}0,
{writebool}0,
{writereal}0,
{makevec_op}0,
{subv_opi}0,
{subvass_opi}0,
{subv_opr}0,
{subvass_opr}0,
{subv_opp}0,
{subvass_opp}0,
{cmpops}0,
{cmoppntr}0,
{substr_op}0,
{concat_op}0,
0,0,0,0);
implementation begin end.
