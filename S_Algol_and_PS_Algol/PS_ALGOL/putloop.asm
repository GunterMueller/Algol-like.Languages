RASTER_DATA	segment word public 'DATA'
	dw	0
RASTER_DATA	ends

RASTER_TEXT	segment	byte public 'CODE'
	assume	cs:RASTER_TEXT,ds:RASTER_DATA
; Line 71
_Ploop	proc	far
	push	ds
	push	si
	push	di
	push	bp
	mov	bp,RASTER_DATA
	mov	ds,bp
	mov	bp,sp
	sub	sp,10
	mov	si,word ptr [bp+22]
; Line 72
; Line 73
; Line 74
; Line 75
; Line 76
; Line 77
; Line 78
	mov	word ptr [bp-6],0
; Line 79
	mov	byte ptr [bp-6],-1
; Line 80
	mov	ax,word ptr [bp-6]
	mov	cx,si
	shl	ax,cl
	xor	ax,-1
	mov	word ptr [bp-6],ax
; Line 81
; Line 82
	mov	di,word ptr [bp+20]
	jmp	short @L13
@L12:
; Line 83
; Line 84
	mov	ax,0
; Line 85
	les	bx,dword ptr [bp+12]
	mov	al,byte ptr es:[bx]
;	mov	byte ptr [bp-8],al
	inc	word ptr [bp+12]
; Line 86
;	mov	ax,word ptr [bp-8]
	mov	cx,si
	shl	ax,cl
	mov	word ptr [bp-8],ax
; Line 87
	les	bx,dword ptr [bp+16]
	mov	ax, es:[bx]
;	mov	byte ptr [bp-4],al
    xchg ah,al      ; new line
; Line 88
;	les	bx,dword ptr [bp+16]
;	mov	al,byte ptr es:[bx]
;	mov	byte ptr [bp-3],al
; Line 89
;	mov	ax,word ptr [bp-4]
	and	ax,word ptr [bp-6]
	or	ax,word ptr [bp-8]
	mov	word ptr [bp-4],ax
; Line 90
;	mov	al,byte ptr [bp-3]
;	les	bx,dword ptr [bp+16]   ; bx already has this
	mov	byte ptr es:[bx],ah    ; used to be al  , write low byte
	inc	word ptr [bp+16]
; Line 91
;	les	bx,dword ptr [bp+16]
    inc bx	      ; substitute for line above
	mov	ah,byte ptr es:[bx]    ; dummy read to load ega latches
;	mov	byte ptr [bp-9],al     ; this was assignment to junk
; Line 92
;	mov	al,byte ptr [bp-4]
;	les	bx,dword ptr [bp+16]  ; bx has this
	mov	byte ptr es:[bx],al    ; write high byte in ram
; Line 93
; Line 94
; Line 95
@L11:
	dec	di
@L13:
	or	di,di
	jne	@L12
@L10:
; Line 96
	cmp	word ptr [bp+26],0
	jne	@L14
	jmp	@L9
; Line 97
; Line 98
; Line 99
@L14:
	mov	word ptr [bp-8],0
	mov	word ptr [bp-2],0
; Line 100
	les	bx,dword ptr [bp+12]
	mov	al,byte ptr es:[bx]
	mov	byte ptr [bp-8],al
; Line 101
	mov	al,byte ptr [bp+24]
	mov	byte ptr [bp-2],al
; Line 102
; Line 103
	mov	ax,word ptr [bp-8]
	mov	cx,si
	shl	ax,cl
	mov	word ptr [bp-8],ax
; Line 104
	mov	ax,word ptr [bp-2]
	mov	cx,si
	shl	ax,cl
	mov	word ptr [bp-2],ax
; Line 105
; Line 106
	les	bx,dword ptr [bp+16]
	mov	al,byte ptr es:[bx+1]
	mov	byte ptr [bp-4],al
; Line 107
	les	bx,dword ptr [bp+16]
	mov	al,byte ptr es:[bx]
	mov	byte ptr [bp-3],al
; Line 108
	mov	ax,word ptr [bp-2]
	not	ax
	and	ax,word ptr [bp-4]
	mov	dx,word ptr [bp-8]
	and	dx,word ptr [bp-2]
	or	ax,dx
	mov	word ptr [bp-4],ax
; Line 109
	mov	al,byte ptr [bp-3]
	les	bx,dword ptr [bp+16]
	mov	byte ptr es:[bx],al
; Line 110
	les	bx,dword ptr [bp+16]
	mov	al,byte ptr es:[bx+1]
	mov	byte ptr [bp-9],al
; Line 111
; Line 112
	mov	al,byte ptr [bp-4]
	les	bx,dword ptr [bp+16]
	mov	byte ptr es:[bx+1],al
; Line 113
; Line 114
@L9:
	mov	sp,bp
	pop	bp
	pop	di
	pop	si
	pop	ds
	ret
_Ploop	endp
; Line 124
_getline	proc	far
	push	ds
	push	si
	push	di
	push	bp
	mov	bp,RASTER_DATA
	mov	ds,bp
	mov	bp,sp
	sub	sp,6
; Line 20
; Line 21
; Line 22
; Line 23
; Line 24
; Line 25
; Line 26
; Line 27
; Line 28
; Line 29
	mov	ax,8
	mov	dx,word ptr [bp+20]
	and	dx,7
	sub	ax,dx
	mov	word ptr [bp-2],ax
; Line 30
; Line 31
	mov	dx,word ptr [bp+22]
	mov	ax,word ptr [bp+20]
	mov	cx,3
	call	far ptr LRSH@
	mov	word ptr [bp-6],ax
; Line 32
	mov	dx,word ptr [bp+26]
	mov	ax,word ptr [bp+24]
	mov	cx,3
	call	far ptr LRSH@
	mov	di,ax
; Line 33
	mov	dx,word ptr [bp+26]
	mov	ax,word ptr [bp+24]
	and	ax,7
	and	dx,0
	or	dx,ax
	je	@2
@7:
	inc	di
; Line 34
@2:
	les	bx,dword ptr [bp+12]
	add	bx,word ptr [bp-6]
	mov	word ptr [bp+14],es
	mov	word ptr [bp+12],bx
; Line 35
	xor	si,si
	mov	cx,word ptr [bp-2]
	jmp	short @6
@5:
; Line 36
; Line 37
	les	bx,dword ptr [bp+12]
;	mov	al,byte ptr es:[bx+1]
;	mov	byte ptr [bp-4],al
; Line 38
;	les	bx,dword ptr [bp+12]
	mov	ax, es:[bx]
;	mov	byte ptr [bp-3],al
	inc	word ptr [bp+12]
; Line 39
;	mov	ax,word ptr [bp-4]
    xchg ah,al		; new line
	sar	ax,cl
	les	bx,dword ptr [bp+16]
	mov	byte ptr es:[bx],al
	inc	word ptr [bp+16]
; Line 40
; Line 41
@4:
	inc	si
@6:
	cmp	si,di
	jl	@5
@3:
; Line 42
; Line 43
; Line 44
@1:
	mov	sp,bp
	pop	bp
	pop	di
	pop	si
	pop	ds
	ret
_getline	endp

RASTER_TEXT	ends
RASTER_TEXT	segment	byte public 'CODE'
	public	_Ploop
	public  _getline
RASTER_TEXT	ends
    include raster.asm
