.global A, C,D
.extern B

.equ C, A - B + 0x10
.equ A, 0x20 + s + x - x2
.equ D, x

.data
.skip 32
x : .word D
x2: .word C

.section .sekcija r,w
.skip 64
s:
.skip 5
.end
