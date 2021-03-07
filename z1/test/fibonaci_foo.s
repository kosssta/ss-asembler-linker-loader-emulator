.global foo

.text
foo:
mov 2(%r6), %r1
mov $1, %r0
cmp $2, %r1
jgt *exit(%pc)

push %r1
sub $1, %r1
push %r1
call *foo(%pc)
pop %r1

pop %r1
push %r0
sub $2, %r1
push %r1
call *foo(%pc)
pop %r1

pop %r4
add %r4, %r0

exit:
ret
.end

0 -> 1
1 -> 1
2 -> 2
3 -> 3
4 -> 5
5 -> 8
6 -> 13 (0xD)
7 -> 21 (0x15)
8 -> 34 (0x22)
9 -> 55 (0x37)
10 (:) -> 89 (0x59)
11 (;) -> 144 (0x90)
