        AREA    |.text|, CODE, READONLY
        THUMB
        EXPORT  __main
        EXPORT  main
        ENTRY

;------------------------------------------------------
; Compare two dates (year -> month -> day)
; If date1 earlier than date2 => R0 = 1
; Otherwise (equal or later)  => R0 = -1
; Cortex-M0 / Thumb-1 safe.
;------------------------------------------------------

__main
main
        ; Default result = -1, using pure Thumb-1 sequence
        MOVS    r0, #0          ; r0 = 0
        SUBS    r0, #1          ; r0 = -1

        ; ---- Compare year ----
        LDR     r1, =date1_year ; r1 = &date1_year
        LDR     r3, [r1]        ; r3 = year1
        LDR     r2, =date2_year ; r2 = &date2_year
        LDR     r4, [r2]        ; r4 = year2
        CMP     r3, r4
        BLT     set_earlier     ; year1 < year2 -> earlier
        BGT     done            ; year1 > year2 -> not earlier

        ; ---- Years equal, compare month ----
        LDR     r1, =date1_month
        LDR     r3, [r1]        ; r3 = month1
        LDR     r2, =date2_month
        LDR     r4, [r2]        ; r4 = month2
        CMP     r3, r4
        BLT     set_earlier
        BGT     done

        ; ---- Months equal, compare day ----
        LDR     r1, =date1_day
        LDR     r3, [r1]        ; r3 = day1
        LDR     r2, =date2_day
        LDR     r4, [r2]        ; r4 = day2
        CMP     r3, r4
        BLT     set_earlier
        B       done            ; >= -> not earlier

set_earlier
        MOVS    r0, #1          ; R0 = 1 (earlier)

done
        BKPT    #0              ; stop in debugger
halt    B       halt            ; safety loop

;---------------------- DATA --------------------------
        AREA    myData, DATA, READONLY
        ALIGN

date1_month DCD 12
date1_day   DCD 31
date1_year  DCD 2014

date2_month DCD 1
date2_day   DCD 20
date2_year  DCD 2013

        END
