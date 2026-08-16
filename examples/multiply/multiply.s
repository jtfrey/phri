__MAIN:             MV0L    R1, #10         ; Multiplicand
                    MV0L    R2, #0xCD       ; Multiplier
                    BRL     MULTIPLY        ; Call the MULTIPLY subroutine
ALL_DONE:           BR      ALL_DONE        ; Infinite loop
                    
MULTIPLY:           PUSH    R1, R2          ; Save R1, R2 so we don't clobber them
                    ORS     R3, R0, R0      ; R3 <= 0, get [C]arry cleared
NEXT_PART_SUM:      LSR     R2, #1          ; Drop bit 0 of multiplier into [C]arry
                    BR.CC   NO_PART_SUM
                    SRCC
                    ADD     R3, R3, R1      ; Add the multiplicand into the partial sum
NO_PART_SUM:        LSL     R1, #1          ; Multiplicand *= 2
                    CMP     R2, #0          ; Has the multiplier gone to zero yet?
                    BR.NE   NEXT_PART_SUM   ; Nope, keep doing partial sums
                    POP     R2, R1          ; Restore R1, R2
                    RET