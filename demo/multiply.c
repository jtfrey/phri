
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phri_bus.h"
phri_word_t     asmbin[] = {
                    0xD001,
                    0x1051,
                    0xD002,
                    0x166A,
                    0x6801,
                    0x4FFF,
                    0x3BB9,
                    0x3BBA,
                    0xD003,
                    0xC900,
                    0x5D01,
                    0x818B,
                    0xA0C1,
                    0xB40A,
                    0x55FC,
                    0xF40B,
                    0x9900,
                    0x59FA,
                    0x28BA,
                    0x28B9,
                    0x0016 };

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
        phri_cpu_execinstr(&C);
        phri_cpu_summary(&C);
        
        // Detect an infinite loop (our end condition):
        if ( C.registers.PC == last_PC ) break;
        
        // Update the saved PC (for infinite loop detection) and do the next pass:
        last_PC = C.registers.PC;
    }
    printf("Program exited at $%02hhX%04hX\n", C.registers.PSEG, C.registers.PC);
    return 0;
}
