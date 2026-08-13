;
; Calculate π/4 using a signed 1.14 fixed-precision number
; format and the Maclaurin series for the inverse tangent:
;
;     π/4 ≈ 1 - (1/3) + (1/5) - (1/7) + (1/9) …
;
                    .ORG    $0000
                    
__MAIN:             MV0L    R7, __STACK@LSB     ; Set SP to our built-in stack
                    BR      OVER_STACK          ; Skip over the stack
__STACK_BOTTOM:     DS      16

__STACK_TOP:
OVER_STACK:         MVH0    R3, #0x40           ; R3 <= 0x4000 = 1.0
                    MV0L    R4, #3              ; R4 <= 0x0003 = n

PI_LOOP:            MOV     R1, R4              ; R1 <= R4 (n)
                    BRL     ONE_OVER_N          ; R1 <= 1 / n
                    ORS     R0, R1, R0          ; R1 (1 / n) == 0?
                    BR.EQ   LOCKUP              ; Nothing left to introduce into the sum
                    SUB     R3, R3, R1          ; R3 <= R3 - R1 = sum - (1 / n)
                    ADD     R1, R4, #2          ; R1 <= R4 + 2
                    BRL     ONE_OVER_N          ; R1 <= 1 / (n+2)
                    ORS     R0, R1, R0          ; R1 (1 / (n+2)) == 0?
                    BR.EQ   LOCKUP              ; Nothing left to introduce into the sum
                    ADD     R3, R3, R1          ; R3 <= R3 + R1 = sum + (1 / (n+2))
                    SRCC                        ; Clear carry
                    ADDS    R4, #4              ; R4 <= R4 + 4
                    BR.PL   PI_LOOP             ; So long as we are < 32767, go again
                    
LOCKUP:             BR      LOCKUP              ; Go into an infinite loop


;
; ONE_OVER_N
;
; Subroutine that calculates the fraction (1/n) where n is a positive
; integer coming from R1.
;
; Registers R2…R5 are utilized by the computation, so for non-trivial
; cases all four registers are pushed to the stack associated with R7/SP
; and retrieved before return.
;
; The result is returned in R1.
;
ONE_OVER_N:         BITS    R1                  ; Set flags from R1 & 0xFFFF
                    B.EQ    L                   ; The input was 0, just return as-is
                    ;
                    ; Not zero, check for 1:
                    ;
                    CMP     R1, #1
                    BR.NE   NOT_ONE
RETURN_ONE:         MV0H    R1, #0x40           ; R1 <= 0x4000
                    RET                         ; return(0x4000=1.0)
NOT_ONE:            PUSH    R2, R3, R4, R5      ; Save registers we clobber
                    
                    MV0     R3                  ; Set R3.R2 to the remainder…
                    MV0L    R2, #2              ; …pre-shifted for the DIV_LOOP

                    MV0     R4                  ; Set R4 to the quotient, 0.0
                    
                    MV0L    R5, #14             ; 14 bit positions to process

DIV_LOOP:           ;
                    ; Shift the quotient:
                    ;
                    SHL     R4, R4, #1          ; R4 <= R4 << 1
                    ;
                    ; Test for remainder (R3.R2) >= n (R1).  The upper word of
                    ; n is implied zero.
                    ;
                    CMP     R3, R0              ; Is R3 == 0?
                    BR.NE   ADJUST_REMAINDER
                    ;
                    ; Now check if the lower word of the remainder is less than n:
                    ;
                    CMP     R2, R1              ; R2 == R1 ?
                    BR.CS   NEXT_ITERATION      ; R2 < R1, skip to the next iteration

ADJUST_REMAINDER:   ;
                    ; Remainder -= n
                    ;
                    SRCC                        ; Clear carry
                    SUBS    R2, R2, R1          ; R2 <= R2 - R1
                    SUBS    R3, R3, R0          ; R3 <= R3 - (0 + [C])
                    ;
                    ; Set bit 0 in the quotient:
                    ;
                    OR      R4, #0b1            ; R4 <= R4 | 1

NEXT_ITERATION:     ;
                    ; Decrement iteration count:
                    ;
                    SRCC                        ; Clear carry
                    SUBS    R5, R5, #1          ; R5 <= R5 - 1
                    BR.EQ   EXIT_LOOP           ; All done!
                    
                    ;
                    ; Copy bit 15 from the low word of the remainder into [C]arry
                    ; then shift the low word:
                    ;
                    BIT     R2, #15             ; [C] <= bit 15 of R2 (low word)
                    SHCL    R3, R3, #1          ; R3 <= R3 << 1, [C]arry goes to bit 0 of R3
                    SHL     R2, R2, #1          ; R2 <= R2 << 1, no carry-in
                    BR      DIV_LOOP
                    
EXIT_LOOP:          ;
                    ; Check if the remainder is >= n/2, round-up if so
                    ;
                    CMP     R3, R0              ; R3 == 0?
                    BR.NE   ROUND_UP            ; R3 > 0, round-up
                    BIT     R2, #15             ; [C] <= bit 15 of R2 (low word)
                    SHCL    R3, R3, #1          ; R3 <= R3 << 1, [C]arry goes to bit 0 of R3
                    SHL     R2, R2, #1          ; R2 <= R2 << 1, no carry-in
                    CMP     R2, R1              ; R2 < R1?
                    BR.CS   EXIT_FN             ; No round-up necessary

ROUND_UP:           ADD     R4, #1              ; R4 <= R4 + 1

EXIT_FN:            MOV     R1, R4              ; R1 <= R4 = quotient
CLEANUP_FN:         POP     R5, R4, R3, R2      ; Restore saved registers
                    RET                         ; return(R1)
