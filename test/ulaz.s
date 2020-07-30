.extern nekiSimbol, opetNekiSimbol,  josSimbola  ,df, B
.global labela, C


.text
loop:
jmp *(%pc)
mov 5(%r6), $10
push labela
push C
.equ C, 5 + labela

.data
.word labela, 0x15, C  ,labela   , 100
.skip 1
labela: .byte 255
.end
