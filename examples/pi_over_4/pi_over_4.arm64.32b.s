;
; Calculate π/4 using a signed 1.62 fixed-precision number
; format and the Maclaurin series for the inverse tangent:
;
;     π/4 ≈ 1 - (1/3) + (1/5) - (1/7) + (1/9) …
;

.global _main

_main:              MOVZ    W3, #0x4000, LSL#16 ; W3 <= 0x40000000 = 1.0
                    MOVZ    W4, #3              ; W4 <= 0x0003 = n

pi_loop:            MOV     W1, W4              ; W1 <= W4 (n)
                    BL      one_over_n          ; W1 <= 1 / n
                    CMP     W1, WZR             ; W1 (1 / n) == 0?
                    B.EQ    exit                ; Nothing left to introduce into the sum
                    SUB     W3, W3, W1          ; W3 <= W3 - W1 = sum - (1 / n)
                    ADD     W1, W4, #2          ; W1 <= W4 + 2
                    BL      one_over_n          ; W1 <= 1 / (n+2)
                    CMP     W1, #0              ; W1 (1 / (n+2)) == 0?
                    B.EQ    exit                ; Nothing left to introduce into the sum
                    ADD     W3, W3, W1          ; W3 <= W3 + W1 = sum + (1 / (n+2))
                    ADD     W4, W4, #4          ; W4 <= W4 + 4
                    CMP     W4, #0x100000
next_cycle:         B.LT    pi_loop             ; So long as we are positive, go again
                    
exit:               ADRP    X0, fmtStr@PAGE
                	ADD     X0, X0, fmtStr@PAGEOFF
                	SUB     SP, SP, #16
                    STR     W3, [SP]
                    BL	    _printf
                    ADD     SP, SP, #16

                    MOV     W0, #0		        ; Use 0 return code
                    MOVZ    W16, #1		        ; System call number 1 terminates this program
                    SVC     #0x80		        ; Call kernel to terminate the program

.data
fmtStr:             .asciz	"Estimate of pi/4 = 0x%08X\n"
.align 4
.text


;
; one_over_n
;
; Subroutine that calculates the fraction (1/n) where n is a positive
; integer coming from W1.
;
; Registers W2…W5 are utilized by the computation, so for non-trivial
; cases all four registers are pushed to the stack associated with W7/SP
; and retrieved before return.
;
; The result is returned in W1.
;
one_over_n:         CMP     W1, #0              ; W1 == 0?
                    B.NE    check_one
                    RET                         ; The input was 0, just return as-is
                    ;
                    ; Not zero, check for 1:
                    ;
check_one:          CMP     W1, #1
                    B.NE    not_one
return_one:         MOVZ    W1, #0x4000, LSL#16 ; W1 <= 0x40000000 = 1.0
                    RET                         ; return(1.0)
not_one:            STP     X2, X3, [SP, #-16]! ; Save scratch registers to the stack
                    STP     X4, X5, [SP, #-16]!
                    
                    MOVZ    W3, #2              ; Set remainder to pre-shifted
                                                ; numerator
                    MOVZ    W4, #0              ; Set W4 to the quotient, 0.
                    MOVZ    W5, #30             ; 30 bit positions to process

div_loop:           ;
                    ; Shift the quotient:
                    ;
                    LSL     W4, W4, #1          ; W4 <= W4 << 1
                    ;
                    ; Test for remainder (W3) >= n (W1).
                    ;
                    CMP     W3, W1              ; Is W3 >= 0?
                    B.LT    next_iteration

adjust_remainder:   ;
                    ; Remainder -= n
                    ;
                    SUB     W3, W3, W1          ; W3 <= W3 - W1
                    ;
                    ; Set bit 0 in the quotient:
                    ;
                    ORR     W4, W4, #0b1        ; W4 <= W4 | 1

next_iteration:     ;
                    ; Decrement iteration count:
                    ;
                    SUBS    W5, W5, #1          ; W5 <= W5 - 1
                    B.EQ    exit_loop           ; All done!
                    
                    LSL     W3, W3, #1          ; W3 <= W3 << 1
                    B       div_loop
                    
exit_loop:          ;
                    ; Check if the remainder W3 is >= n/2, round-up if so
                    ;
                    LSL     W3, W3, #1          ; Compare 2*remainder to n
                    CMP     W3, W1
                    B.LT    exit_fn

round_up:           ADD     W4, W4, #1          ; W4 <= W4 + 1

exit_fn:            MOV     W1, W4              ; W1 <= W4 = quotient
                    LDP     X4, X5, [SP], #16   ; Pop saved registers off the stack
                    LDP     X2, X3, [SP], #16
                    RET                         ; return(W1)
