
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phri_bus.h"
phri_word_t     asmbin[] = {
                    0x0851,
                    0x0E6A,
                    0x6801,
                    0xF580,
                    0x3BB9,
                    0x3BBA,
                    0xD803,
                    0xAC0A,
                    0x5D02,
                    0xF40B,
                    0x818B,
                    0xA409,
                    0xF802,
                    0xF40B,
                    0x59F8,
                    0x28BA,
                    0x28B9,
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
