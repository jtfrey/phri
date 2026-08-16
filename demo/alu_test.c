
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
    printf("[0x%04hX (%02hhX|%02hhX) 0x%04hX C=%u]",
        alu->arg1, alu->path, alu->uop, alu->arg2, alu->carry_in);
}

void
print_alu_out(
    phri_alu_t      *alu
)
{
    char            flags[5] = {
                        (alu->flags & kphri_sb_m & alu->flags_mask) ? 'M' : 'm',
                        (alu->flags & kphri_sb_c & alu->flags_mask) ? 'C' : 'c',
                        (alu->flags & kphri_sb_v & alu->flags_mask) ? 'V' : 'v',
                        (alu->flags & kphri_sb_z & alu->flags_mask) ? 'Z' : 'z',
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
    
    phri_alu_init(alu);
    alu.arg1 = arg1, alu.arg2 = arg2, alu.carry_in = 0,
        alu.path = kphri_alu_path_arith, alu.uop = kphri_alu_uop_arith_add;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_arith_sub;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.path = kphri_alu_path_logic, alu.uop = kphri_alu_uop_logic_and;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_logic_or;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_logic_xor;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
    
    alu.arg2 = 1, alu.carry_in = 1;
        alu.path = kphri_alu_path_bitshift, alu.uop = kphri_alu_uop_bitshift_lsl;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_bitshift_lsr;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_bitshift_asr;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_bitshift_ror;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
    alu.carry_in = 0;
        alu.uop = kphri_alu_uop_bitshift_lsl;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_bitshift_lsr;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_bitshift_asr;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        alu.uop = kphri_alu_uop_bitshift_ror;
    print_alu_in(&alu); phri_alu_exec(&alu); print_alu_out(&alu);
        
    
    return 0;
}
