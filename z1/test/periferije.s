.global main
.extern data_out, data_in, timer_cfg

.text
main:
cmp %r1, $1
jne *main(%pc)
halt

.data
x: .word 48

.section .int.timer r
mov x, %r0
mov %r0, data_out
add $1, %r0
cmp $58, %r0
jgt *exit(%pc)
mov $48, %r0

exit: mov %r0, x 
iret

.section .int.term r
mov data_in, %r0

cmp %r0, $0x71
jne *continue(%pc)
mov $1, %r1
jmp exit2

continue:
cmp %r0, $0x66
jne *continue2(%pc)
cmp timer_cfg, $0
jeq *exit2(%pc)
sub $1, timer_cfg
jmp exit2

continue2:
cmp %r0, $0x73
jne *exit2(%pc)
cmp timer_cfg, $7
jeq *exit2(%pc)
add $1, timer_cfg

exit2:
iret

.end
