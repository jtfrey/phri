
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phri_alu.h"

const char ops[] = { '+', '&', '|', '^' };

void
print_alu_in(
    phri_alu_t      *alu
)
{
    printf("[flags=0x%02hhX] (0x%04hX & 0x%04hX) %c (%c0x%04hX << 0x%02hhX) + %d",
        alu->flags,
        alu->arg1, alu->mask1,
        ops[alu->uop],
        alu->not2 ? '-' : ' ', alu->arg2, alu->shift2,
        alu->carry_in ? 1 : 0);
}

void
print_alu_out(
    phri_alu_t      *alu
)
{
    char            flags[5] = {
                        (alu->flags & kphri_sb_m) ? 'M' : 'm',
                        (alu->flags & kphri_sb_c) ? 'C' : 'c',
                        (alu->flags & kphri_sb_v) ? 'V' : 'v',
                        (alu->flags & kphri_sb_z) ? 'Z' : 'z',
                        '\0' };
    printf(" = 0x%04hX [flags=%s]\n",
        alu->result, flags);
}

int
main()
{
    phri_alu_t      alu;
    phri_word_t     arg1 = 0xFFF0;
    phri_word_t     arg2 = 0x00CD;
    
    phri_alu_init_add(alu, 0, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_add(alu, 1, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_sub(alu, 0, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_sub(alu, 1, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_and(alu, 0, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_and(alu, 1, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_or(alu, 0, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_or(alu, 1, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_xor(alu, 0, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_xor(alu, 1, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    arg2 = 0b1010101001010101;
    phri_alu_init_shl(alu, 0, arg1, arg2, 3);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_shl(alu, 1, arg1, arg2, 3);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_shr(alu, 0, arg1, arg2, 5);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_shr(alu, 1, arg1, arg2, 5);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    arg1 = arg2 = 0x0100;
    
    phri_alu_init_add(alu, 0, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    phri_alu_init_sub(alu, 0, arg1, arg2, 0);
    print_alu_in(&alu);
    phri_alu_exec(&alu);
    print_alu_out(&alu);
    
    return 0;
}
