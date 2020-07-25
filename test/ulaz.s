.global labela

.text
loop:
jmp *labela(%pc)
mov 5(%r6), $10
push labela(%r7)



.data
mov 5, 6
labela:  halt
.end
