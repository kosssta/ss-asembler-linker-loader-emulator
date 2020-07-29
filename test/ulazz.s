.global labela
.extern nekiSimbol, opetNekiSimbol,  josSimbola  ,df

.text
loop:
jmp *%pc
mov 5(%r6), $10
push labela
push C
.equ C, A + 4
.end
