.global main
.extern len, arr, data_out

.text
main:
cmp $0, len
jeq *end(%pc)
jgt *end(%pc)

mov len, %r6
mul $2, %r6
mov $0, %r0
outer_loop:
cmp %r0, %r6
jeq *prep_write(%pc)

mov %r0, %r1

inner_loop:
add $2, %r1
cmp %r6, %r1
jeq *continue(%pc)

mov arr(%r0), %r2
mov arr(%r1), %r3
cmp %r3, %r2
jgt *inner_loop(%pc)
xchg %r2, %r3
mov %r2, arr(%r0)
mov %r3, arr(%r1)
jmp *inner_loop(%pc)

continue:
add $2, %r0
jmp *outer_loop(%pc)


prep_write:
mov $0, %r5
mov $0, %r4
or $0, %r6


mov arr(%r5), %r0
write:
mov %r0, %r3
and $0xF, %r3
shr %r0, $4
and $0x0fff, %r0

cmp %r3, $9
jgt *letter(%pc)
add $0x30, %r3
jmp *continue_write(%pc)

letter:
add $0x41, %r3
sub $10, %r3

continue_write:
mov %r3, result(%r4)
add $2, %r4

cmp $0, %r0
jne *write(%pc)

sub $2, %r4
write2:
mov result(%r4), %r1
mov %r1, data_out
sub $2, %r4
cmp %r4, $0
jgt *write2(%pc)
jeq *write2(%pc)

add $2, %r5
mov arr(%r5), %r0
mov $0, %r4
mov $0x20, data_out
cmp %r5, %r6
jne *write(%pc)

end:
halt

.bss
result: .skip 10
.end