;
; Calculate π/4 using a signed 1.62 fixed-precision number
; format and the Maclaurin series for the inverse tangent:
;
;     π/4 ≈ 1 - (1/3) + (1/5) - (1/7) + (1/9) …
;

.global _main

_main:              MOVZ    X3, #0x4000, LSL#48 ; X3 <= 0x4000000000000000 = 1.0
                    MOVZ    X4, #3              ; X4 <= 0x0003 = n

pi_loop:            MOV     X1, X4              ; X1 <= X4 (n)
                    BL      one_over_n          ; X1 <= 1 / n
                    CMP     X1, XZR             ; X1 (1 / n) == 0?
                    B.EQ    exit                ; Nothing left to introduce into the sum
                    SUB     X3, X3, X1          ; X3 <= X3 - X1 = sum - (1 / n)
                    ADD     X1, X4, #2          ; X1 <= X4 + 2
                    BL      one_over_n          ; X1 <= 1 / (n+2)
                    CMP     X1, #0              ; X1 (1 / (n+2)) == 0?
                    B.EQ    exit                ; Nothing left to introduce into the sum
                    ADD     X3, X3, X1          ; X3 <= X3 + X1 = sum + (1 / (n+2))
                    ADD     X4, X4, #4          ; X4 <= X4 + 4
                    CMP     X4, #0x100000
next_cycle:         B.LT    pi_loop             ; So long as we are positive, go again
                    
exit:               ADRP    X0, fmtStr@PAGE
                	ADD     X0, X0, fmtStr@PAGEOFF
                	SUB     SP, SP, #16
                    STR     X3, [SP]
                    BL	    _printf
                    ADD     SP, SP, #16

                    MOV     X0, #0		        ; Use 0 return code
                    MOVZ    X16, #1		        ; System call number 1 terminates this program
                    SVC     #0x80		        ; Call kernel to terminate the program

.data
fmtStr:             .asciz	"Estimate of pi/4 = 0x%016lX\n"
.align 4
.text


;
; one_over_n
;
; Subroutine that calculates the fraction (1/n) where n is a positive
; integer coming from X1.
;
; Registers X2…X5 are utilized by the computation, so for non-trivial
; cases all four registers are pushed to the stack associated with X7/SP
; and retrieved before return.
;
; The result is returned in X1.
;
one_over_n:         CMP     X1, #0              ; X1 == 0?
                    B.NE    check_one
                    RET                         ; The input was 0, just return as-is
                    ;
                    ; Not zero, check for 1:
                    ;
check_one:          CMP     X1, #1
                    B.NE    not_one
return_one:         MOVZ    X1, #0x4000, LSL#48 ; X1 <= 0x4000000000000000 = 1.0
                    RET                         ; return(1.0)
not_one:            STP     X2, X3, [SP, #-16]! ; Save scratch registers to the stack
                    STP     X4, X5, [SP, #-16]!
                    
                    MOVZ    X3, #2              ; Set remainder to pre-shifted
                                                ; numerator
                    MOVZ    X4, #0              ; Set X4 to the quotient, 0.
                    MOVZ    X5, #62             ; 62 bit positions to process

div_loop:           ;
                    ; Shift the quotient:
                    ;
                    LSL     X4, X4, #1          ; X4 <= X4 << 1
                    ;
                    ; Test for remainder (X3) >= n (X1).
                    ;
                    CMP     X3, X1              ; Is X3 >= 0?
                    B.LT    next_iteration

adjust_remainder:   ;
                    ; Remainder -= n
                    ;
                    SUB     X3, X3, X1          ; X3 <= X3 - X1
                    ;
                    ; Set bit 0 in the quotient:
                    ;
                    ORR     X4, X4, #0b1        ; X4 <= X4 | 1

next_iteration:     ;
                    ; Decrement iteration count:
                    ;
                    SUBS    X5, X5, #1          ; X5 <= X5 - 1
                    B.EQ    exit_loop           ; All done!
                    
                    LSL     X3, X3, #1          ; X3 <= X3 << 1
                    B       div_loop
                    
exit_loop:          ;
                    ; Check if the remainder X3 is >= n/2, round-up if so
                    ;
                    LSL     X3, X3, #1          ; Compare 2*remainder to n
                    CMP     X3, X1
                    B.LT    exit_fn

round_up:           ADD     X4, X4, #1          ; X4 <= X4 + 1

exit_fn:            MOV     X1, X4              ; X1 <= X4 = quotient
                    LDP     X4, X5, [SP], #16   ; Pop saved registers off the stack
                    LDP     X2, X3, [SP], #16
                    RET                         ; return(X1)
