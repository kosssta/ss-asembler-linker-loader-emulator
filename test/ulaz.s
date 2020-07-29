.global labela
.extern nekiSimbol, opetNekiSimbol,  josSimbola  ,df

.text
loop:
jmp *%pc
mov 5(%r6), $10
push labela
push C
.equ C, A + 4
.equ B, labela - 10

.data
.word labela, 0x15, C  ,labela   , 100, A
.skip 1
labela: .byte 255
.end
