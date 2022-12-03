.MODEL huge,c

linkseg segment para public 'CODE'
assume cs:linkseg
       db 'linker V1',0
extrn  _link:far
_linker proc far
public _linker
      push ds
	  push es
	  call _link
	  pop es
	  pop ds
	  pop ax      ; go back to restart the interrupt instruction
				  ; which will now have been converted to a call
	  sub ax,2
	  push ax
	  iret
_linker endp
_psabort proc far
public _psabort
      push ss      ; make sure data set set up ok
	  pop ds
      mov ax, 4c01h
	  int 21h
_psabort endp

linkseg ends

end
