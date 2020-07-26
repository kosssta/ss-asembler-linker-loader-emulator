.extern puts@PLT
	.text
	.section	.rodata
.LC0:
	.text
	.global	main
main:
.LFB0:
	push	%sp
	mov		%r1, %sp
	mov	.LC0(%pc), %r3
	call	puts@PLT
	movb	%r0, $0
	pop	%sp
	ret
.LFE0:
	.section	.note.GNU-stack
	.section	.note.gnu.property
	.word	 0x11
	.word	 -0x4
	.word	 5
0:
	.skip	 4
1:
	.word	 0xc002
	.word	 3
2:
	.word	 0x3
3:
	.skip 8
4:
.end