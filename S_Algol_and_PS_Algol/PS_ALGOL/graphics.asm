	name	graphics
GRAPHICS_TEXT	segment	byte public 'CODE'
	assume	cs:GRAPHICS_TEXT,ds:GRAPHICS_DATA
GRAPHICS_TEXT	ends
GRAPHICS_DATA	segment word public 'DATA'
_d@	label	byte
_b@	label	byte
_mode	label	word
	dw	1
GRAPHICS_DATA	ends
GRAPHICS_TEXT	segment	byte public 'CODE'
; Line 7
_clearscreen	proc	far
	push	ds
	push	si
	push	bp
	mov	bp,GRAPHICS_DATA
	mov	ds,bp
	mov	bp,sp
	sub	sp,8
; Line 8
; Line 9
	mov	word ptr [bp-2],-20480
	mov	word ptr [bp-4],0
; Line 10
; Line 11
	cmp	word ptr _mode,1
	jne	@2
	call	far ptr _clrscr
; Line 12
	jmp	short @3
@2:
; Line 13
	les	bx,dword ptr [bp-4]
	mov	word ptr [bp-6],es
	mov	word ptr [bp-8],bx
; Line 14
	xor	si,si
	jmp	short @7
@6:
	les	bx,dword ptr [bp-8]
	mov	byte ptr es:[bx],0
	inc	word ptr [bp-8]
@5:
	inc	si
@7:
	cmp	si,32767
	jl	@6
@4:
; Line 15
; Line 16
@3:
; Line 17
@1:
	mov	sp,bp
	pop	bp
	pop	si
	pop	ds
	ret	
_clearscreen	endp
; Line 21
_Line	proc	far
	push	ds
	push	si
	push	di
	push	bp
	mov	bp,GRAPHICS_DATA
	mov	ds,bp
	mov	bp,sp
	sub	sp,8
; Line 22
; Line 23
; Line 24
; Line 25
; Line 26
; Line 27
	mov	ax,word ptr [bp+16]
	cmp	ax,word ptr [bp+12]
	jne	@9
; Line 28
; Line 29
	mov	ax,word ptr [bp+14]
	cmp	ax,word ptr [bp+18]
	jle	@10
; Line 30
	push	ss
	lea	ax,word ptr [bp+18]
	push	ax
	push	ss
	lea	ax,word ptr [bp+14]
	push	ax
	call	far ptr _Swap
	add	sp,8
; Line 31
@10:
	mov	si,word ptr [bp+14]
	jmp	short @14
@13:
; Line 32
	push	word ptr [bp+20]
	push	si
	push	word ptr [bp+12]
	call	far ptr _SetPixel
	add	sp,6
@12:
	inc	si
@14:
	cmp	si,word ptr [bp+18]
	jle	@13
@11:
; Line 33
	jmp	@8
; Line 34
; Line 35
@9:
	mov	ax,word ptr [bp+12]
	cmp	ax,word ptr [bp+16]
	jle	@15
; Line 36
; Line 37
	push	ss
	lea	ax,word ptr [bp+16]
	push	ax
	push	ss
	lea	ax,word ptr [bp+12]
	push	ax
	call	far ptr _Swap
	add	sp,8
; Line 38
	push	ss
	lea	ax,word ptr [bp+18]
	push	ax
	push	ss
	lea	ax,word ptr [bp+14]
	push	ax
	call	far ptr _Swap
	add	sp,8
; Line 39
; Line 40
@15:
	mov	ax,word ptr [bp+18]
	sub	ax,word ptr [bp+14]
	cwd	
	push	dx
	push	ax
	FILD	dword ptr [bp-12]
	FWAIT	
	add	sp,4
	mov	ax,word ptr [bp+16]
	sub	ax,word ptr [bp+12]
	cwd	
	push	dx
	push	ax
	FILD	dword ptr [bp-12]
	FWAIT	
	add	sp,4
	FDIV	
	FSTP	dword ptr [bp-8]
	FWAIT	
; Line 41
	FLD	dword ptr [bp-8]
	mov	ax,word ptr [bp+12]
	cwd	
	push	dx
	push	ax
	FILD	dword ptr [bp-12]
	FWAIT	
	add	sp,4
	FMUL	
	mov	ax,word ptr [bp+14]
	cwd	
	push	dx
	push	ax
	FILD	dword ptr [bp-12]
	FWAIT	
	add	sp,4
	FSUBR	
	FSTP	dword ptr [bp-4]
	FWAIT	
; Line 42
	mov	di,word ptr [bp+12]
	jmp	short @19
@18:
; Line 43
; Line 44
	FLD	dword ptr [bp-8]
	mov	ax,di
	cwd	
	push	dx
	push	ax
	FILD	dword ptr [bp-12]
	FWAIT	
	add	sp,4
	FMUL	
	FLD	dword ptr [bp-4]
	FADD	
	call	far ptr FTOL@
	mov	si,ax
; Line 45
	push	word ptr [bp+20]
	push	si
	push	di
	call	far ptr _SetPixel
	add	sp,6
; Line 46
@17:
	inc	di
@19:
	cmp	di,word ptr [bp+16]
	jle	@18
@16:
; Line 47
@8:
	mov	sp,bp
	pop	bp
	pop	di
	pop	si
	pop	ds
	ret	
_Line	endp
; Line 48
_Swap	proc	far
	push	ds
	push	si
	push	bp
	mov	bp,GRAPHICS_DATA
	mov	ds,bp
	mov	bp,sp
; Line 49
; Line 50
; Line 51
	les	bx,dword ptr [bp+10]
	mov	si,word ptr es:[bx]
; Line 52
	les	bx,dword ptr [bp+14]
	mov	ax,word ptr es:[bx]
	les	bx,dword ptr [bp+10]
	mov	word ptr es:[bx],ax
; Line 53
	les	bx,dword ptr [bp+14]
	mov	word ptr es:[bx],si
; Line 54
@20:
	pop	bp
	pop	si
	pop	ds
	ret	
_Swap	endp
; Line 55
_SetPixel	proc	far
	push	ds
	push	bp
	mov	bp,GRAPHICS_DATA
	mov	ds,bp
	mov	bp,sp
; Line 56
; Line 57
	cmp	word ptr _mode,2
	jne	@22
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	word ptr [bp+8]
	call	far ptr _SETPIXELHGC
	mov	sp,bp
	jmp	short @21
; Line 58
@22:
	mov	ax,25
	sub	ax,word ptr [bp+10]
	push	ax
	push	word ptr [bp+8]
	call	far ptr _gotoxy
	mov	sp,bp
; Line 59
	push	word ptr [bp+12]
	call	far ptr _putch
	pop	cx
; Line 60
; Line 61
@21:
	pop	bp
	pop	ds
	ret	
_SetPixel	endp
; Line 64
_ginit	proc	far
	push	ds
	push	bp
	mov	bp,GRAPHICS_DATA
	mov	ds,bp
; Line 65
	call	far ptr _GMODE
; Line 66
	mov	word ptr _mode,2
; Line 67
@23:
	pop	bp
	pop	ds
	ret	
_ginit	endp
; Line 69
_textmode	proc	far
	push	ds
	push	bp
	mov	bp,GRAPHICS_DATA
	mov	ds,bp
; Line 70
	call	far ptr _TMODE
; Line 71
	mov	word ptr _mode,1
; Line 72
@24:
	pop	bp
	pop	ds
	ret	
_textmode	endp
GRAPHICS_TEXT	ends
GRAPHICS_DATA	segment word public 'DATA'
_s@	label	byte
GRAPHICS_DATA	ends
GRAPHICS_TEXT	segment	byte public 'CODE'
	public	_clearscreen
	public	_textmode
	public	_SetPixel
	public	_ginit
	public	_Swap
	public	_Line
GRAPHICS_TEXT	ends
	extrn	_GMODE:far
	extrn	_TMODE:far
	extrn	_SETPIXELHGC:far
	extrn	_clrscr:far
	extrn	_gotoxy:far
	extrn	FTOL@:far
	extrn	_putch:far
	end
