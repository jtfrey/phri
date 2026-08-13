
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "phri_bus.h"
phri_word_t     asmbin[] = {
                    0xD007,
                    0x10B7,
                    0x4808,
                    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                    0xD003,
                    0x1A03,
                    0xD004,
                    0x101C,
                    0xD201,
                    0x680C,
                    0xD880,
                    0x5109,
                    0x918B,
                    0x8251,
                    0x680B,
                    0xD880,
                    0x5104,
                    0x818B,
                    0xF40B,
                    0x8C24,
                    0x5FF3,
                    0x4FFF,
                    0xC880,
                    0x5006,
                    0xF40B,
                    0x98C8,
                    0x5903,
                    0xD001,
                    0x1A01,
                    0x0016,
                    0x3BBA,
                    0x3BBB,
                    0x3BBC,
                    0x3BBD,
                    0xD003,
                    0xD002,
                    0x1012,
                    0xD004,
                    0xD005,
                    0x1075,
                    0xA244,
                    0xF40B,
                    0x9980,
                    0x5903,
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
                    0xA402,
                    0xA9C3,
                    0xA142,
                    0x4FEE,
                    0xF40B,
                    0x9980,
                    0x5906,
                    0xA402,
                    0xA9C3,
                    0xA142,
                    0xF40B,
                    0x9908,
                    0x5501,
                    0x840C,
                    0xD201,
                    0x28BD,
                    0x28BC,
                    0x28BB,
                    0x28BA,
                    0x0016 };

phri_word_t
one_over_n(
    int             n
)
{
    phri_word_t     remainder, quotient;
    int             i = 14;
    
    if ( n == 0 ) return 0;
    if ( n == 1 ) return 0x4000;
    remainder = 2, quotient = 0;
    do {
        quotient <<= 1;
        if ( remainder >= n ) {
            remainder -= n;
            quotient |= 0b1;
        }
        remainder <<= 1;
    } while ( --i > 0 );
    if ( remainder >= n ) quotient++;
    return quotient;
}

phri_word_t
fixed_estimate_pi_over_4(void)
{
    phri_word_t     pi_over_4 = 0x4000;
    phri_word_t     factor;
    phri_word_t     n = 3;
    
    printf("0x4000 ");
    while ( !(n & 0b1000000000000000) ) {
        factor = one_over_n(n);
        if ( factor == 0 ) break;
        printf("- 0x%04hX ", factor);
        pi_over_4 -= factor;
        factor = one_over_n(n+2);
        if ( factor == 0 ) break;
        printf("+ 0x%04hX ", factor);
        pi_over_4 += factor;
        n += 4;
    }
    printf("= 0x%04hX (%f, n = %d)\n", pi_over_4, (double)pi_over_4 / 16384.0, n);
    return pi_over_4;
}

phri_word_t
float_estimate_pi_over_4(void)
{
    phri_word_t     pi_over_4 = 0x4000;
    phri_word_t     factor;
    int             n = 3;
    
    printf("0x4000 ");
    while ( 1 ) {
        factor = trunc(((double)1.0 / (double)n) * 16384.0);
        if ( factor == 0 ) break;
        printf("- 0x%04hX ", factor);
        pi_over_4 -= factor;
        n += 2;
        factor = trunc(((double)1.0 / (double)n) * 16384.0);
        if ( factor == 0 ) break;
        printf("+ 0x%04hX ", factor);
        pi_over_4 += factor;
        n += 2;
    }
    printf("= 0x%04hX (%f, n = %d)\n", pi_over_4, (double)pi_over_4 / 16384.0, n);
    return pi_over_4;
}

int
main()
{
    phri_bus_t              B;
    phri_cpu_t              C = phri_cpu_create();
    phri_mem_N_by_4KiB_t    M;
    phri_word_t             last_PC = 0xFFFF;
    unsigned int            i, n = 3;
    int                     sign = -1;
    
    phri_mem_N_by_4KiB_init_with_bitmap(&M, 0b1000000000000001);
    phri_bus_init(&B, &C, &M);
    phri_mem_bulk_copyin(&M, 0x0000, asmbin, sizeof(asmbin));
    
    while ( 1 ) {
        // Process an instruction and show the CPU summary:
        phri_cpu_fetchinstr(&C);
        phri_cpu_execinstr(&C);
        phri_cpu_summary(&C);
        
        // Detect an infinite loop (our end condition):
        if ( C.registers.PC == last_PC ) break;
        
        // Update the saved PC (for infinite loop detection) and do the next pass:
        last_PC = C.registers.PC;
        switch ( last_PC ) {
            case 0x008A:
                n += 2;
                break;
        }
    }
    phri_cpu_summary(&C);
    phri_mem_summary(&M, stdout);
    printf("= 0x%04hX (%f, last term = 1/%u)\n", C.registers.R[3], (double)C.registers.R[3] / 16384.0, n);
    
    printf("Program exited at $%02hhX%04hX\n", C.registers.PSEG, C.registers.PC);
    printf("C estimate fixed = 0x%04hX\n", fixed_estimate_pi_over_4());
    //printf("C estimate float = 0x%04hX\n", float_estimate_pi_over_4());
    return 0;
}
