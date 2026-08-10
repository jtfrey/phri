__MAIN:             MV0L    R1, #10         ; Multiplicand
                    MV0L    R2, #0xCD       ; Multiplier
                    BRL     MULTIPLY        ; Call the MULTIPLY subroutine
ALL_DONE:           BR      ALL_DONE        ; Infinite loop
                    
MULTIPLY:           PUSH    R1, R2          ; Save R1, R2 so we don't clobber them
                    MOV     R3, R0          ; R3 <= 0
                    BITS    R2              ; Get bit 0 of multiplier in [C]arry
                    BR.CC   NO_PART_SUM
NEXT_PART_SUM:      ADD     R3, R3, R1
NO_PART_SUM:        SHL     R1, R1, #1      ; Multiply multiplicand by 2
                    SHR     R2, #1, #0      ; Divide multiplier by two, copying bit 0 to [C]arry
                    BR.CS   NEXT_PART_SUM   ; Bit 0 of multiplier is set, do the next partial sum
                    CMP     R2, R0          ; Has multiplier gone to zero?
                    BR.NE   NO_PART_SUM     ; Multiplier is still non-zero, but no partial sum
                                            ; on this iteration
                    POP     R2, R1          ; Restore R1, R2
                    RET