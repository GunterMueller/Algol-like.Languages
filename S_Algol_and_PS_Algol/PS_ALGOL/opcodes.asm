_samain:  
 dw jlstart
 dw jlestart
 dw jgstart
 dw jgestart
 dw jestart
 dw jnzstart
 dw jumpstart
 dw jumptstart
 dw jumpttstart
 dw cjumpstart
 dw forteststart
 dw forstepstart
 dw outbytestart
 dw shrinkstart
 dw enterframestart
 dw exitframestart
 dw prologstart
 dw dbstart
 dw mov_ax_spstart
 dw mov_bx_spstart
 dw mov_bp_spstart
 dw push_axstart
 dw pop_axstart
 dw push_bpstart
 dw prolog86start
 dw pushdispstart
 dw push_bxstart
 dw pop_bxstart
 dw cslea_axstart
 dw add_axstart
 dw lea_axstart
 dw mov_axstart
 dw push_ssstart
 dw dwstart
 dw epilogstart
 dw formclosstart
 dw call_staticstart
 dw start_paramsistart
 dw start_paramsvstart
 dw start_paramsrstart
 dw start_paramspstart
 dw call_globstart
 dw call_locstart
 dw call_start
 dw llintstart
 dw int18start
 dw int21start
 dw pushrealstart
 dw pushpstart
 dw heappushrealstart
 dw heappushpstart
 dw poprealstart
 dw poppstart
 dw heappoprealstart
 dw heappoppstart
 dw movpstart
 dw movrstart
 dw add_spstart
 dw cmp_axstart
 dw ldssistart
 dw pushfarpntrstart
 dw sspushfarpntrstart
 dw llreal_opstart
 dw globalistart
 dw globaladdrstart
 dw globalpstart
 dw globalprocstart
 dw globalrstart
 dw localistart
 dw localaddrstart
 dw localpstart
 dw localprocstart
 dw localrstart
 dw getlevstart
 dw loadistart
 dw loadaddrstart
 dw loadrstart
 dw loadpstart
 dw loadprocstart
 dw assistart
 dw assrstart
 dw asspstart
 dw globalassistart
 dw globalassrstart
 dw globalasspstart
 dw localassistart
 dw localassrstart
 dw localasspstart
 dw localassprocstart
 dw localmovistart
 dw localmovrstart
 dw localmovpstart
 dw discard_globalistart
 dw neg_opstart
 dw bandstart
 dw borstart
 dw shiftrstart
 dw shiftlstart
 dw plusopstart
 dw not_opstart
 dw minusopstart
 dw multopstart
 dw divopstart
 dw remopstart
 dw cmpopstart
 dw mov_di_axstart
 dw mov_si_axstart
 dw mov_es_dxstart
 dw pop_es_distart
 dw push_es_sistart
 dw add_distart
 dw add_sistart
 dw forprepstart
 dw minforteststart
 dw minforstepstart
 dw start_writestart
 dw end_writestart
 dw calldstart
 dw callcstart
 dw writeintstart
 dw writestringstart
 dw writepntrstart
 dw writeboolstart
 dw writerealstart
 dw makevec_opstart
 dw subv_opistart
 dw subvass_opistart
 dw subv_oprstart
 dw subvass_oprstart
 dw subv_oppstart
 dw subvass_oppstart
 dw cmpopsstart
 dw cmoppntrstart
 dw substr_opstart
 dw concat_opstart
 dw fin
jlstart: jl $+99
jlestart: jle $+99
jgstart: jg $+99
jgestart: jge $+99
jestart: je $+99
jnzstart: jnz $+99
jumpstart: jump 12345
jumptstart: jumpt 12345
jumpttstart: jumptt 12345
cjumpstart: cjump 12345
forteststart: fortest 12345
forstepstart: forstep 12345
outbytestart: outbyte 
shrinkstart: shrink 
enterframestart: enterframe 
exitframestart: exitframe 
prologstart: prolog 199
dbstart: db 199
mov_ax_spstart: mov_ax_sp 
mov_bx_spstart: mov_bx_sp 
mov_bp_spstart: mov_bp_sp 
push_axstart: push_ax 
pop_axstart: pop_ax 
push_bpstart: push_bp 
prolog86start: prolog86 
pushdispstart: pushdisp 
push_bxstart: push_bx 
pop_bxstart: pop_bx 
cslea_axstart: cslea_ax 12345
add_axstart: add_ax 12345
lea_axstart: lea_ax 12345
mov_axstart: mov_ax 12345
push_ssstart: push_ss 
dwstart: dw 12345
epilogstart: epilog 12345
formclosstart: formclos 12345
call_staticstart: call_static 12345
start_paramsistart: start_paramsi 
start_paramsvstart: start_paramsv 
start_paramsrstart: start_paramsr 
start_paramspstart: start_paramsp 
call_globstart: call_glob 12345
call_locstart: call_loc 12345
call_start: call_ 12345, 23456
llintstart: llint 12345
int18start: int18 
int21start: int21 
pushrealstart: pushreal 
pushpstart: pushp 
heappushrealstart: heappushreal 
heappushpstart: heappushp 
poprealstart: popreal 
poppstart: popp 
heappoprealstart: heappopreal 
heappoppstart: heappopp 
movpstart: movp 
movrstart: movr 
add_spstart: add_sp 12345
cmp_axstart: cmp_ax 12345
ldssistart: ldssi 12345
pushfarpntrstart: pushfarpntr 12345
sspushfarpntrstart: sspushfarpntr 12345
llreal_opstart: llreal_op 12345
globalistart: globali 12345
globaladdrstart: globaladdr 12345
globalpstart: globalp 12345
globalprocstart: globalproc 12345
globalrstart: globalr 12345
localistart: locali 12345
localaddrstart: localaddr 12345
localpstart: localp 12345
localprocstart: localproc 12345
localrstart: localr 12345
getlevstart: getlev 12345
loadistart: loadi 12345, 23456
loadaddrstart: loadaddr 12345, 23456
loadrstart: loadr 12345, 23456
loadpstart: loadp 12345, 23456
loadprocstart: loadproc 12345, 23456
assistart: assi 12345, 23456
assrstart: assr 12345, 23456
asspstart: assp 12345, 23456
globalassistart: globalassi 12345
globalassrstart: globalassr 12345
globalasspstart: globalassp 12345
localassistart: localassi 12345
localassrstart: localassr 12345
localasspstart: localassp 12345
localassprocstart: localassproc 12345
localmovistart: localmovi 12345
localmovrstart: localmovr 12345
localmovpstart: localmovp 12345
discard_globalistart: discard_globali 12345
neg_opstart: neg_op 
bandstart: band 
borstart: bor 
shiftrstart: shiftr 
shiftlstart: shiftl 
plusopstart: plusop 
not_opstart: not_op 
minusopstart: minusop 
multopstart: multop 
divopstart: divop 
remopstart: remop 
cmpopstart: cmpop 
mov_di_axstart: mov_di_ax 
mov_si_axstart: mov_si_ax 
mov_es_dxstart: mov_es_dx 
pop_es_distart: pop_es_di 12345
push_es_sistart: push_es_si 12345
add_distart: add_di 12345
add_sistart: add_si 12345
forprepstart: forprep 
minforteststart: minfortest 12345
minforstepstart: minforstep 12345
start_writestart: start_write 
end_writestart: end_write 
calldstart: calld 12345
callcstart: callc abcd
writeintstart: writeint 
writestringstart: writestring 
writepntrstart: writepntr 
writeboolstart: writebool 
writerealstart: writereal 
makevec_opstart: makevec_op 
subv_opistart: subv_opi 
subvass_opistart: subvass_opi 
subv_oprstart: subv_opr 
subvass_oprstart: subvass_opr 
subv_oppstart: subv_opp 
subvass_oppstart: subvass_opp 
cmpopsstart: cmpops 
cmoppntrstart: cmoppntr 
substr_opstart: substr_op 
concat_opstart: concat_op 
fin: db 0
 end 
