                        ;
                        ; Calculate 1/2 using a signed 1.14 fixed-precision number
                        ; format.  The ONE_OVER_N subroutine can obviously be reused
                        ; for an entire sequence of numbers (for example).
                        ;
                                            .ORG    $0000
                                            
0001|0000000000000001   __MAIN:             MOV     R1, R0
1011|0001000000010001                       MVL     R1, #2
6801|0110100000000001                       BRL     ONE_OVER_N
                                            
4FFF|0100111111111111   LOCKUP:             BR      LOCKUP              ; Go into an infinite loop
                        
                        
                        
                        
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
C880|1100100010000000   ONE_OVER_N:         ANDS    R0, R1, R0          ; Set flags from R1 & 0xFFFF
5006|0101000000000110                       B.EQ    L                   ; The input was 0, just return as-is
                                            ;
                                            ; Not zero, check for 11:
                                            ;
98C8|1001100011001000                       SUBS    R0, R1, #1          ; R1 == 1?
5903|0101100100000011                       BR.NE   NOT_ONE
0001|0000000000000001   RETURN_ONE:         MOV     R1, R0              ; R1 <= 0x0000
1A01|0001101000000001                       MVH     R1, #0x40           ; R1 <= 0x4000
00F0|0000000011110000                       MOV     PC, L               ; return(0x4000=1.0)
3BBA|0011101110111010   NOT_ONE:            STO     #-2, [R7], R2       ; PUSH R2
3BBB|0011101110111011                       STO     #-2, [R7], R3       ; PUSH R3
3BBC|0011101110111100                       STO     #-2, [R7], R4       ; PUSH R4
3BBD|0011101110111101                       STO     #-2, [R7], R5       ; PUSH R5
                                            
0003|0000000000000011                       MOV     R3, R0              ; Set R3.R2 to the remainder…
0002|0000000000000010                       MOV     R2, R0              ; …of 1…
1012|0001000000010010                       MVL     R2, #2              ; …pre-shifted for the DIV_LOOP

0004|0000000000000100                       MOV     R4, R0              ; Set R4 to the quotient, 0.0
                                            
0005|0000000000000101                       MOV     R5, R0
1075|0001000001110101                       MVL     R5, #14             ; 14 bit positions to process

                        DIV_LOOP:           ;
                                            ; Shift the quotient:
                                            ;
A244|1010001001000100                       SHL     R4, R4, #1          ; R4 <= R4 << 1
                                            ;
                                            ; Test for remainder (R3.R2) >= n (R1).  The upper word of
                                            ; n is implied zero.
                                            ;
F400|1111010000000000                       SR      AND, mcvz           ; Clear all the status bits
9980|1001100110000000                       SUBS    R0, R3, R0          ; Is R3 == 0?
5903|0101100100000011                       BR.NE   ADJUST_REMAINDER
                                            ;
                                            ; Now check if the lower word of the remainder is less than n:
                                            ;
F400|1111010000000000                       SR      AND, mcvz           ; Clear all the status bits
9908|1001100100001000                       SUBS    R0, R2, R1          ; R2 - R1
5504|0101010100000100                       BR.CS   NEXT_ITERATION      ; R2 < R1, skip to the next iteration
                        
                        ADJUST_REMAINDER:   ;
                                            ; Remainder -= n
                                            ;
F400|1111010000000000                       SR      AND, mcvz           ; Clear all the status bits
990A|1001100100001010                       SUBS    R2, R2, R1          ; R2 <= R2 - R1
9983|1001100110000011                       SUBS    R3, R3, R0          ; R3 <= R3 - (0 + [C])
                                            ;
                                            ; Set bit 0 in the quotient:
                                            ;
D40C|1101010000001100                       OR      R4, #0b1            ; R4 <= R4 | 1
                        
                        NEXT_ITERATION:     ;
                                            ; Decrement iteration count:
                                            ;
F400|1111010000000000                       SR      AND, mcvz           ; Clear all the status bits
9ACD|1001101011001101                       SUBS    R5, R5, #1          ; R5 <= R5 - 1
5104|0101000100000100                       BR.EQ   EXIT_LOOP           ; All done!
                                            
                                            ;
                                            ; Copy bit 15 from the low word of the remainder into [C]arry
                                            ; then shift the low word:
                                            ;
A402|1010010000000010                       SHL     R2, #0, #0b000      ; [C] <= bit 15 of R2 (low word)
A9C3|1010100111000011                       SHCL    R3, R3, #1          ; R3 <= R3 << 1, [C]arry goes to bit 0 of R3
A142|1010000101000010                       SHL     R2, R2, #1          ; R2 <= R2 << 1, no carry-in
4FEE|0100111111101110                       BR      DIV_LOOP
                                            
                        EXIT_LOOP:          ;
                                            ; Check if the remainder is >= n/2, round-up if so
                                            ;
F400|1111010000000000                       SR      AND, mcvz           ; Clear all the status bits
9980|1001100110000000                       SUBS    R0, R3, R0          ; R3 == 0?
5906|0101100100000110                       BR.NE   ROUND_UP            ; R3 > 0, round-up
A402|1010010000000010                       SHL     R2, #0, #0b000      ; [C] <= bit 15 of R2 (low word)
A9C3|1010100111000011                       SHCL    R3, R3, #1          ; R3 <= R3 << 1, [C]arry goes to bit 0 of R3
A142|1010000101000010                       SHL     R2, R2, #1          ; R2 <= R2 << 1, no carry-in
F400|1111010000000000                       SR      AND, mcvz           ; Clear all the status bits
9908|1001100100001000                       SUBS    R0, R2, R1          ; R2 < R1?
5501|0101010100000001                       BR.CS   EXIT_FN             ; No round-up necessary
                        
840C|1000010000001100   ROUND_UP:           ADD     R4, #1              ; R4 <= R4 + 1
                        
0021|0000000000100001   EXIT_FN:            MOV     R1, R4              ; R1 <= R4 = quotient
28BD|0010100010111101   CLEANUP_FN:         LDR     R5, [R7], #2        ; POP R5
28BC|0010100010111100                       LDR     R4, [R7], #2        ; POP R4
28BB|0010100010111011                       LDR     R3, [R7], #2        ; POP R3
28BA|0010100010111010                       LDR     R2, [R7], #2        ; POP R2
00F0|0000000011110000                       MOV     PC, L               ; return(R1)
