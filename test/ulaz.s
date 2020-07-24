.text
loop:
jmp *labela(%r4h)
jne *5(%r1)
jmp labela


labela:
.word 0x50

.end

