.global labela
.extern nekiSimbol, opetNekiSimbol,  josSimbola  ,df

.text
loop:
jmp *%pc
mov 5(%r6), $10
push labela
.equ C, +5 - 0x06 -labela


.data
.word labela, 0x15, C  ,labela   , 100
.skip 1
labela: .byte 255
.end
