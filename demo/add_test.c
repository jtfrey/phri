
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phri_bus.h"

/*

0x187B|0001.1000.0111.1011    __MAIN:         MOVZN   R3, #0xF
0x1034|0001.0000.0011.0100                    MOVZ    R4, #6
0xD002|1101.0000.0000.0010                    OR      R2, R0, R0
0x81A3|1000.0001.1010.0011                    ADD     R3, R3, R4
0x8C02|1000.1100.0000.0010                    ADDC    R2, #0
0x81A3|1000.0001.1010.0011                    ADD     R3, R3, R4
0x8C02|1000.1100.0000.0010                    ADDC    R2, #0
0x81A3|1000.0001.1010.0011                    ADD     R3, R3, R4
0x8C02|1000.1100.0000.0010                    ADDC    R2, #0
0x81A3|1000.0001.1010.0011                    ADD     R3, R3, R4
0x8C02|1000.1100.0000.0010                    ADDC    R2, #0
0x81A3|1000.0001.1010.0011                    ADD     R3, R3, R4
0x8C02|1000.1100.0000.0010                    ADDC    R2, #0
0x81A3|1000.0001.1010.0011                    ADD     R3, R3, R4
0x8C02|1000.1100.0000.0010                    ADDC    R2, #0
0xF680|1111.0110.1000.0000                    HALT

*/

phri_word_t     asmbin[] = {
                    0x187B,
                    0x1034,
                    0x81A3,
                    0x8C02,
                    0x81A3,
                    0x8C02,
                    0x81A3,
                    0x8C02,
                    0x81A3,
                    0x8C02,
                    0x81A3,
                    0x8C02,
                    0x81A3,
                    0x8C02,
                    0xF680 };

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
        // Process an instruction and show the CPU summary:
        phri_cpu_fetchinstr(&C);
        if ( phri_cpu_execinstr(&C) ) {
            phri_cpu_summary(&C);
        } else {
            break;
        }
    }
    printf("Program exited at $%02hhX%04hX\n", C.registers.PSEG, C.registers.PC);
                            
    return 0;
}
