.global main
.extern data_in, data_out, foo

.data
flag: .word 0

.bss
n: .skip 2

.text
main:
loop:
cmp $0, flag
jeq *loop(%pc)

push n
call *foo(%pc)
pop %r1

mov $0, %r5
write:
mov %r0, %r3
and $0xF, %r3
shr %r0, $4

cmp %r3, $9
jgt *letter(%pc)
add $0x30, %r3
jmp *continue(%pc)

letter:
add $0x41, %r3
sub $10, %r3

continue:
mov %r3, result(%r5)
add $2, %r5

cmp $0, %r0
jne *write(%pc)

sub $2, %r5
write2:
cmp $0, %r5
jgt *exit(%pc)
mov result(%r5), %r1
mov %r1, data_out
sub $2, %r5
jmp *write2(%pc)

exit:
halt

.section .int.term r,x,p
mov data_in, %r3
sub $0x30, %r3
mov %r3, n
mov $1, flag
iret

.bss
result: .skip 10
.end
