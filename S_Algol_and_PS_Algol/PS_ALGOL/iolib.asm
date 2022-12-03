; io lib .asm
    .MODEL HUGE,c
    .CODE
    PUBLIC ASMWRITE,ASMREAD,ASMOPEN
ASMWRITE PROC uses ds dx cx bx,handle,buf:ptr,len
    mov bx,handle
    mov cx,len
    mov ah,40h
    IF @DataSize

	lds dx,buf
    ELSE
	mov dx,buf
    ENDIF
    int 21h
    jnc endwrite
    neg ax
endwrite:
    ret
ASMWRITE endp

ASMREAD PROC uses ds dx cx bx,handle,buf:ptr,len
    mov bx,handle
    mov cx,len
    mov ah,3fh
    IF @DataSize

	lds dx,buf
    ELSE
	mov dx,buf
    ENDIF
    int 21h
    jnc endread
    neg ax
endread:
    ret
ASMREAD endp

ASMOPEN PROC uses ds dx cx bx,buf:ptr,mode
    mov ax,mode
    mov ah,3dh

    IF @DataSize

	lds dx,buf
    ELSE
	mov dx,buf
    ENDIF
    int 21h
    jnc endopen
    neg ax
endopen:
    ret
ASMOPEN endp




 end
