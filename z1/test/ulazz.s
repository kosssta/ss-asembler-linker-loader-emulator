.extern data_out
.global main

.text
main:
mov $F, %r0
mov %r0, data_out
halt

.data
.byte 5
x:.word F, x

.equ E, data_out
.equ C, 0x50+x
.equ F, E
.global C
.end
