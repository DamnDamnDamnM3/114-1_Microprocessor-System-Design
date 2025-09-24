        AREA    |.text|, CODE, READONLY
        THUMB
        EXPORT  __main
        EXPORT  main
        ENTRY

;------------------------------------------------------
; Lab 1.3 (Thumb-1 / Cortex-M0 safe, only R0–R7 used)
; Reverse product:
;   result = a1*bn + a2*b(n-1) + ... + an*b1
; Final result -> R4
; Multiply done by repeated addition
;------------------------------------------------------

__main
main
        ; r0 = &A_Array
        LDR     r0, =A_Array

        ; r7 = n = size
        LDR     r7, =size
        LDR     r7, [r7]

        ; r1 = &B_Array + (n-1)*4
        LDR     r1, =B_Array
        MOVS    r2, r7
        SUBS    r2, #1
        LSLS    r2, r2, #2
        ADDS    r1, r2           ; r1 -> last element of B

        ; r3 = accumulator (sum)
        MOVS    r3, #0

loop
        ; load one pair
        LDR     r4, [r0]         ; r4 = a_i
        LDR     r5, [r1]         ; r5 = b_(n+1-i)

        ; advance pointers
        ADDS    r0, #4
        SUBS    r1, #4

        ; ---------- multiply r4 * r5 by repeated add ----------
        ; r6 = multiplicand (a_i)
        ; r5 = multiplier (b)
        ; r2 = product accumulator (clear)
        MOV     r6, r4
        MOVS    r2, #0

        ; if multiplier == 0 skip multiply loop
        CMP     r5, #0
        BEQ     mul_done

mul_loop
        ADDS    r2, r6           ; r2 += r6
        SUBS    r5, #1
        BNE     mul_loop

mul_done
        ADDS    r3, r2           ; sum += product

        ; countdown main loop
        SUBS    r7, #1
        BNE     loop

        ; move final sum to R4 (as required)
        MOV     r4, r3

        BKPT    #0               ; stop in debugger
halt    B       halt

;----------------------- DATA -------------------------
        AREA    myData, DATA, READONLY
        ALIGN
size    DCD     8
A_Array DCD     1, 2, 3, 4, 5, 6, 7, 8
B_Array DCD     9,10,11,12,13,14,15,16
        END
