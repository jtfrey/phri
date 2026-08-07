
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phri_bus.h"

/*
__MAIN:             MV0L    R1, #10         ; Multiplicand
                    MV0L    R2, #0xCD       ; Multiplier
                    BRL     MULTIPLY        ; Call the MULTIPLY subroutine
ALL_DONE:           B       ALL_DONE        ; Infinite loop
                    
MULTIPLY:           PUSH    R1              ; Save R1, R2 so we don't clobber them
                    PUSH    R2
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
                    POP     R2
                    POP     R1              ; Restore R1, R2
                    RET

---------------------------------------------------------------------------------------------------

__MAIN:             MOV     R1, R0              0000.0000.0000.0001     0x0001
                    MVL     R1, #10             0001.0000.0101.0001     0x1051
                    MOV     R2, R0              0000.0000.0000.0010     0x0002
                    MVL     R2, #CD             0001.0110.0110.1010     0x166A
                    BRL     MULTIPLY            0110.1000.0000.0001     0x6801
ALL_DONE:           BR      ALL_DONE            0100.1111.1111.1111     0x4FFF
                    
MULTIPLY:           STO     #-2, [SP], R1       0011.1011.1011.1001     0x3BB9
                    STO     #-2, [SP], R2       0011.1011.1011.1010     0x3BBA
                    MOV     R3, R0              0000.0000.0000.0011     0x0003
                    ANDS    R0, R2, R0          1100.1001.0000.0000     0xC900
NEXT_PART_SUM:      ADD     R3, R3, R1          1000.0001.1000.1011     0x818B
NO_PART_SUM:        SHL     R1, R1, #1          1010.0000.1100.0001     0xA0C1
                    SHR     R2, #1, #0          1011.0100.0000.1010     0xB40A
                    BR.CS   NEXT_PART_SUM       0101.0101.1111.1100     0x55FC
                    SUBS    R0, R2, R0          1001.1001.0000.0000     0x9900
                    BR.NE   NO_PART_SUM         0101.1001.1111.1011     0x59FB
                    LDR     R2, [SP], #+2       0010.1000.1011.1010     0x28BA
                    LDR     R1, [SP], #+2       0010.1000.1011.1001     0x28B9
                    MOV     PC, L               0000.0000.1111.0000     0x00F0

 */
phri_word_t     asmbin[] = {
                    0x0001,
                    0x1051,
                    0x0002,
                    0x166A,
                    0x6801,
                    0x4FFF,
                    
                    0x3BB9,
                    0x3BBA,
                    0x0003,
                    0xC900,
                    0x818B,
                    0xA0C1,
                    0xB40A,
                    0x55FC,
                    0x9900,
                    0x59FB,
                    0x28BA,
                    0x28B9,
                    0x00F0 };

int
main()
{
    phri_bus_t          B;
    phri_cpu_t          C = phri_cpu_create();
    phri_mem_64k_t      M;
    unsigned int        i;
    
    phri_mem_64k_init(&M);
    memcpy(M.ram, asmbin, sizeof(asmbin));
    phri_bus_init(&B, &C, &M);
    
    while ( 1 ) {
        phri_cpu_fetchinstr(&C);
        phri_cpu_execinstr(&C);
        phri_cpu_summary(&C);
        if ( C.registers.INSTR == 0x4FFF ) break;
    }
    return 0;
}
