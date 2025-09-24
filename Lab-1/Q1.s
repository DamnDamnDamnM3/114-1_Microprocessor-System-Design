        AREA    |.text|, CODE, READONLY
        THUMB
        EXPORT  __main
        EXPORT  main
        ENTRY

__main
main
        LDR     r0, =array
        MOVS    r1, #10
        MOVS    r3, #0

loop
        LDR     r2, [r0]
        ADDS    r0, #4
        MULS    r2, r2
        ADDS    r3, r2
        SUBS    r1, #1
        BNE     loop

        MOV     r4, r3

stop    B       stop

        AREA    myData, DATA, READONLY
        ALIGN
array   DCD     2, 4, 7, 3, 1, 2, 10, 11, 5, 13
        END
