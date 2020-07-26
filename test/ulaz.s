.global labela
.extern nekiSimbol, opetNekiSimbol,  josSimbola  ,df

.text
loop:
jmp *labela(%pc)
mov 5(%r6), $10
push labela(%r7)



.data
.word labela, 0x15, 15  ,labela   , 100
.skip 1
labela: .byte 255
.end
