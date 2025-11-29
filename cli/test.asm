
; test.asm
; This program prints "Hello, DirtVM!" and then exits using direct bytecode operations.

; Manually place "Hello, DirtVM!\n" into global memory starting at address 0
; 'H' at address 0
pushd8 'H'
pushd16 0
gstore
; 'e' at address 1
pushd8 'e'
pushd16 1
gstore
; 'l' at address 2
pushd8 'l'
pushd16 2
gstore
; 'l' at address 3
pushd8 'l'
pushd16 3
gstore
; 'o' at address 4
pushd8 'o'
pushd16 4
gstore
; ',' at address 5
pushd8 ','
pushd16 5
gstore
; ' ' at address 6
pushd8 ' '
pushd16 6
gstore
; 'D' at address 7
pushd8 'D'
pushd16 7
gstore
; 'i' at address 8
pushd8 'i'
pushd16 8
gstore
; 'r' at address 9
pushd8 'r'
pushd16 9
gstore
; 't' at address 10
pushd8 't'
pushd16 10
gstore
; 'V' at address 11
pushd8 'V'
pushd16 11
gstore
; 'M' at address 12
pushd8 'M'
pushd16 12
gstore
; '!' at address 13
pushd8 '!'
pushd16 13
gstore
; '\n' at address 14
pushd8 '\n'
pushd16 14
gstore
; Null terminator for good measure (though not strictly needed by SYS_write with explicit count)
pushd8 0
pushd16 15
gstore


; Setup stack for write(1, &"Hello, DirtVM!\n", 15) - 15 is length without null terminator
; Push length of "Hello, DirtVM!\n" (15 chars + 1 for newline = 16)
pushd8 16

; Push starting address of string (address 0)
pushd128 0x00000000000000000000000000000000

; Push file descriptor (stdout = 1)
pushd8 1

; Call SYS_write (syscall number 1)
syscall 1

; Push exit status (0)
pushd8 0

; Call SYS_exit (syscall number 60)
syscall 60
