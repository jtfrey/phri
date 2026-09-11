
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phri_bus.h"

phri_word_t     asmbin[] = {
                        0x1011,
                        0x6801,
                        0xF680,
                        0xC880,
                        0x5006,
                        0xF40B,
                        0x98C8,
                        0x5902,
                        0x1121,
                        0x4006,
                        0x3BBA,
                        0x3BBB,
                        0x3BBC,
                        0x3BBD,
                        0xD003,
                        0x1012,
                        0xD004,
                        0x1075,
                        0xF40B,
                        0xA244,
                        0xF40B,
                        0x9980,
                        0x5904,
                        0xF40B,
                        0x9908,
                        0x5504,
                        0xF40B,
                        0x990A,
                        0x9983,
                        0xD40C,
                        0xF40B,
                        0x9ACD,
                        0x5104,
                        0xF40B,
                        0xA142,
                        0xA1C3,
                        0x4FED,
                        0xF40B,
                        0x9980,
                        0x5906,
                        0xF40B,
                        0xA142,
                        0xA1C3,
                        0xF40B,
                        0x9908,
                        0x5501,
                        0x840C,
                        0xD221,
                        0x28BD,
                        0x28BC,
                        0x28BB,
                        0x28BA,
                        0x4006 };

int
main()
{
    phri_bus_t          B;
    phri_cpu_t          C = phri_cpu_create();
    phri_mem_64k_t      M;
    phri_word_t         last_PC = 0xFFFF;
    unsigned int        i;
    
    phri_mem_64k_init(&M);
    memcpy(M.ram, asmbin, sizeof(asmbin));
    phri_bus_init(&B, &C, &M);
    
    while ( 1 ) {
        // Process an instruction and show the CPU summary:
        phri_cpu_fetchinstr(&C);
        if ( phri_cpu_execinstr(&C) ) {
            phri_cpu_summary(&C);
            
            // Detect an infinite loop (our end condition):
            if ( C.registers.PC == last_PC ) break;
            
            // Update the saved PC (for infinite loop detection) and do the next pass:
            last_PC = C.registers.PC;
        } else {
            break;
        }
    }
    printf("Program exited at $%02hhX%04hX\n", C.registers.PSEG, C.registers.PC);
    return 0;
}
