/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file implements the CPU execution of instructions.
 *
 */

#include "phri_cpu.h"
#include "phri_instr.h"
#include "phri_alu.h"
#include "phri_data.h"
#include "phri_branch.h"

static int phri_uop_map[] = {
                kphri_alu_uop_add,      /*  + = 000 */
                kphri_alu_uop_add,      /*  - = 001 */
                kphri_alu_uop_or,       /* << = 010 */
                kphri_alu_uop_or,       /* >> = 011 */
                kphri_alu_uop_and,      /*  & = 100 */
                kphri_alu_uop_or,       /*  | = 101 */
                kphri_alu_uop_xor };    /*  ^ = 110 */
/* Using the indices above, mark an ALU operation for not2 behavior */
static phri_byte_t phri_not2_map = 0b00000010;  

void
phri_cpu_execinstr(
    phri_cpu_t  *cpu
)
{
    // One cycle for decode, additional cycles added as necessary (memory requests, etc.)
    cpu->cycles++;
    
    if ( cpu->registers.INSTR & kphri_instr_kind_alu_mask ) {
        // ALU:
        phri_word_t     Rd_idx = (cpu->registers.INSTR & kphri_instr_mask_alu_dst);
        phri_word_t     opcode = (cpu->registers.INSTR & kphri_instr_mask_alu_opcd);
        
        phri_alu_init(cpu->alu);
        if ( opcode == kphri_alu_op_misc ) {
            // Miscellaneous
            if ( (cpu->registers.INSTR & kphri_alu_op_misc_cmpn_mask) != kphri_alu_op_misc_cmpn ) {
                // SR[AOXS] instruction -- note that there is room here for additional instruction groups if 
                // the kphri_alu_op_misc_sr_mask bit were NOT set, but that's for future expansion (haha)
                switch ( cpu->registers.INSTR & kphri_alu_op_misc_sr_op_mask ) {
                    case kphri_alu_op_misc_sr_op_and:
                        cpu->registers.F &= (cpu->registers.INSTR & kphri_instr_mask_misc_sr_const);
                        break;
                    case kphri_alu_op_misc_sr_op_or:
                        cpu->registers.F |= (cpu->registers.INSTR & kphri_instr_mask_misc_sr_const);
                        break;
                    case kphri_alu_op_misc_sr_op_xor:
                        cpu->registers.F ^= (cpu->registers.INSTR & kphri_instr_mask_misc_sr_const);
                        break;
                    case kphri_alu_op_misc_sr_op_set:
                        cpu->registers.F = (cpu->registers.INSTR & kphri_instr_mask_misc_sr_const);
                        break;
                }
            }
            else if ( (cpu->registers.INSTR & kphri_alu_op_misc_cmpn_mask) == kphri_alu_op_misc_cmpn ) {
                cpu->alu.arg1 = phri_cpu_rdr(cpu, Rd_idx);
                cpu->alu.arg2 = (cpu->registers.INSTR & kphri_instr_mask_alu_src2c) >> kphri_instr_mask_alu_src2_shift;
                // Sign-extend the 7-bit constant to 16-bit:
                if ( cpu->alu.arg2 & 0b1000000 ) cpu->alu.arg2 |= 0b1111111110000000;
                cpu->alu.not2 = ((cpu->registers.INSTR & kphri_alu_op_misc_cmp_mask) == kphri_alu_op_misc_cmn) ? kphri_bit_off : kphri_bit_on;
                phri_alu_exec(&cpu->alu);
                cpu->registers.F = cpu->alu.flags;
            }
        } else {
            // Standard arithmetic/bitwise logic operations
            phri_bit_t      set_cond = (cpu->registers.INSTR & kphri_instr_mask_alu_scond) ? kphri_bit_on : kphri_bit_off;
            phri_byte_t     op_idx = (cpu->registers.INSTR & kphri_instr_mask_alu_uop_mask) >> kphri_instr_mask_alu_uop_shift;
            phri_word_t     postshift_bitmask = 0b0000000000000000;
            
            cpu->alu.uop = phri_uop_map[op_idx];
            cpu->alu.carry_in = (set_cond && (cpu->registers.F & kphri_sb_c)) ? kphri_bit_on : kphri_bit_off;
            cpu->alu.not2 = (phri_not2_map & (0b1 << op_idx)) ? kphri_bit_on : kphri_bit_off;
            
            if ( cpu->registers.INSTR & kphri_instr_mask_alu_src1t ) {
                // 7-bit const:
                switch ( opcode ) {
                    case kphri_alu_op_shr:
                    case kphri_alu_op_shl: {
                        phri_byte_t     postshift_bit_idx;
                        
                        cpu->alu.arg1 = cpu->alu.carry_in ? 0xFFFF : 0x0000;
                        cpu->alu.arg2 = phri_cpu_rdr(cpu, Rd_idx);
                        cpu->alu.carry_in = kphri_bit_off;
                        cpu->alu.shift2 = ((opcode == kphri_alu_op_shl) ? 0b00000000: 0b10000000 ) | 
                                (((cpu->registers.INSTR & kphri_instr_mask_alu_src1c) >> kphri_instr_mask_alu_src1c_shift) &
                                    kphri_instr_mask_alu_src1_shdist);
                        cpu->alu.shift1 = ((opcode == kphri_alu_op_shl) ? 0b10000000: 0b00000000 ) | (16 - 
                                (((cpu->registers.INSTR & kphri_instr_mask_alu_src1c) >> kphri_instr_mask_alu_src1c_shift) &
                                    kphri_instr_mask_alu_src1_shdist));
                        // Carry-out this bit:
                        postshift_bit_idx = (((cpu->registers.INSTR & kphri_instr_mask_alu_src1c) >> kphri_instr_mask_alu_src1c_shift) & kphri_instr_mask_alu_src1_shCbit) >> kphri_instr_mask_alu_src1_shCbit_shift;
                        postshift_bitmask = (opcode == kphri_alu_op_shl) ?
                                                0b1000000000000000 >> postshift_bit_idx :
                                                0b0000000000000001 << postshift_bit_idx;
                        set_cond = kphri_bit_on;
                        break;
                    }
                    default:
                        cpu->alu.arg1 = phri_cpu_rdr(cpu, Rd_idx);
                        cpu->alu.arg2 = (cpu->registers.INSTR & kphri_instr_mask_alu_src1c) >> kphri_instr_mask_alu_src1c_shift;
                        // Sign-extend the 7-bit constant to 16-bit for +/-:
                        if ( ! (op_idx & 0b110) && (cpu->alu.arg2 & 0b1000000) ) cpu->alu.arg2 |= 0b1111111110000000;
                        break;
                }
            } else {
                if ( cpu->registers.INSTR & kphri_instr_mask_alu_src2t ) {
                    // 2 register + 3-bit const
                    switch ( opcode ) {
                        case kphri_alu_op_shr:
                        case kphri_alu_op_shl:
                            cpu->alu.arg1 = cpu->alu.carry_in ? 0xFFFF : 0x0000;
                            cpu->alu.arg2 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_instr_mask_alu_src1i) >> kphri_instr_mask_alu_src1i_shift);
                            cpu->alu.carry_in = kphri_bit_off;
                            // The 3-bit constant for SHR/SLH gets incremented by one so the range is [1, 8]:
                            cpu->alu.shift2 = 1 + ((cpu->registers.INSTR & kphri_instr_mask_alu_src2c) >> kphri_instr_mask_alu_src2_shift);
                            postshift_bitmask = (opcode == kphri_alu_op_shl) ? 0b1000000000000000 : 0b0000000000000001;
                            set_cond = kphri_bit_on;
                            break;
                        default:
                            cpu->alu.arg1 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_instr_mask_alu_src1i) >> kphri_instr_mask_alu_src1i_shift), cpu->alu.mask1 = 0xFFFF;
                            cpu->alu.arg2 = (cpu->registers.INSTR & kphri_instr_mask_alu_src2c) >> kphri_instr_mask_alu_src2_shift;
                            // Sign-extend the 3-bit constant to 16-bit:
                            if ( cpu->alu.arg2 & 0b100 ) cpu->alu.arg2 |= 0b1111111111111000;
                            break;
                    }
                } else {
                    phri_byte_t     Rx_idx = (cpu->registers.INSTR & kphri_instr_mask_alu_src1i) >> kphri_instr_mask_alu_src1i_shift;
                    phri_byte_t     Ry_idx = (cpu->registers.INSTR & kphri_instr_mask_alu_src2i) >> kphri_instr_mask_alu_src2_shift;
                    
                    cpu->alu.arg1 = phri_cpu_rdr(cpu, Rx_idx);
                    cpu->alu.arg2 = phri_cpu_rdr(cpu, Ry_idx) ^ \
                                        ((!(op_idx & 0b100) || (op_idx & 0b001) || Ry_idx) ? 0x0000 : 0xFFFF);
                    switch ( opcode ) {
                        case kphri_alu_op_shr:
                        case kphri_alu_op_shl:
                            cpu->alu.carry_in = kphri_bit_off;
                            postshift_bitmask = (opcode == kphri_alu_op_shl) ? 0b1000000000000000 : 0b0000000000000001;
                            set_cond = kphri_bit_on;
                            break;
                        default:
                            break;
                    }
                }
            }
            
            phri_alu_exec(&cpu->alu);
            phri_cpu_wrr(cpu, Rd_idx, cpu->alu.result);
            if ( set_cond ) cpu->registers.F = cpu->alu.flags;
            if ( postshift_bitmask ) cpu->registers.F = (cpu->alu.result & postshift_bitmask) ?
                                                            cpu->registers.F | kphri_sb_c :
                                                            cpu->registers.F & ~kphri_sb_c;
        }
    }
    else if ( (cpu->registers.INSTR & kphri_instr_kind_not_alu_mask) == kphri_instr_kind_branch ) {
        // Branching:
        phri_word_t     is_cond = 1;
        phri_word_t     is_link = (cpu->registers.INSTR & kphri_branch_op_link) ? 1 : 0;
        phri_word_t     is_rel = 1;
        unsigned int    Ry = 6;         // Implied link register is 6/L
        phri_word_t     offset = 0x0000;
        
        if ( cpu->registers.INSTR & kphri_branch_op_cond ) {
            phri_word_t cond = (cpu->registers.INSTR & kphri_branch_op_cond_mask) >> kphri_branch_op_cond_shift;
            
            // Check the condition code and decode the operands:
            is_cond = phri_cpu_check_cc(cpu, cond);
            if ( cpu->registers.INSTR & kphri_branch_op_cond_const ) {
                // The constant is multiplied by 2 since the ISA is 16-bit aligned:
                offset = (cpu->registers.INSTR & kphri_branch_op_cond_const_mask) << 1;
                // Sign-extend the now 9-bit constant to 16-bit (constants are always relative branching):
                if ( offset & 0b100000000 ) offset |= 0b1111111000000000;
            } else {
                is_rel = (cpu->registers.INSTR & kphri_branch_op_rel) ? 1 : 0;
                offset = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_branch_op_rx_mask));
                Ry = (cpu->registers.INSTR & kphri_branch_op_ry_mask) >> 4;
            }
        } else {
            // Decode the operands:
            if ( cpu->registers.INSTR & kphri_branch_op_const ) {
                // The constant is multiplied by 2 since the ISA is 16-bit aligned:
                offset = (cpu->registers.INSTR & kphri_branch_op_const_mask) << 1;
                // Sign-extend the now 12-bit constant to 16-bit (constants are always relative branching):
                if ( offset & 0b100000000000 ) offset |= 0b1111000000000000;
            } else {
                is_rel = (cpu->registers.INSTR & kphri_branch_op_rel) ? 1 : 0;
                // Offset (or absolute address) comes from the indicated register:
                offset = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_branch_op_rx_mask));
                Ry = (cpu->registers.INSTR & kphri_branch_op_ry_mask) >> kphri_branch_op_ry_shift;
            }
        }
        if ( is_cond ) {
            // If this is a linking branch, copy the PC to the target register:
            if ( is_link ) phri_cpu_wrr(cpu, Ry, cpu->registers.PC);
            if ( is_rel )
                // For a relative branch, adjust the PC by offset:
                phri_cpu_pc_adjust(cpu, offset);
            else
                // For a non-relative branch, set the PC to offset:
                cpu->registers.PC = offset;
        }
    }
    else {
        if ( cpu->registers.INSTR & kphri_data_op_mem ) {
            // Memory-based instructions
            // The data movement unit has it's own status register for address computation
            phri_byte_t     S, Rd = cpu->registers.INSTR & kphri_data_op_rd_mask;
            phri_byte_t     Rx = (cpu->registers.INSTR & kphri_data_op_rx_mask) >> kphri_data_op_rx_shift;
            phri_word_t     is_cond = 1;
            phri_word_t     dRx = 0;            
            phri_word_t     mask = 0x0000, set=0xFFFF;
            phri_bit_t      is_mem_read = (!(cpu->registers.INSTR & kphri_data_op_store)) ? kphri_bit_on : kphri_bit_off;
            
            if ( cpu->registers.INSTR & kphri_data_op_autoinc ) {
                // Auto-increment instruction:
                if ( cpu->registers.INSTR & kphri_data_op_autoinc_ry ) {
                    // Auto-increment comes from Ry
                    dRx = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_ry_mask) >> kphri_data_op_ry_shift);
                } else {
                    // Auto-increment comes from a 4-bit constant
                    dRx = (phri_word_t)(cpu->registers.INSTR & kphri_data_op_autoinc_const_mask) >> kphri_data_op_autoinc_const_shift;
                    // Sign-extend from 4-bit to 16-bit if necessary:
                    if ( dRx & 0b1000 ) dRx |= 0b1111111111110000;
                }
            } else if ( cpu->registers.INSTR & kphri_data_op_cond ) {
                // Conditional
                phri_word_t  cond = (cpu->registers.INSTR & kphri_data_op_cc_mask) >> kphri_data_op_cc_shift;
                is_cond = phri_cpu_check_cc(cpu, cond);
            } else if ( cpu->registers.INSTR & kphri_data_op_offset_ry ) {
                // Rx offset by Ry
                cpu->registers.MOFF += phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_ry_mask) >> kphri_data_op_ry_shift);
            } else {
                // 8-bit options
                if ( cpu->registers.INSTR & kphri_data_op_8b_hi ) {
                    // 8-bit, MSB
                    mask = 0x00FF, set = 0xFF00, is_mem_read = kphri_bit_on;
                } else {
                    // 8-bit, LSB
                    mask = 0xFF00, set = 0x00FF, is_mem_read = kphri_bit_on;
                }
            }
            
            if ( is_cond ) {
                if ( cpu->registers.INSTR & kphri_data_op_store ) {
                    // Store
                    // Pre-adjust register value if auto-increment was enabled:
                    cpu->registers.R[Rx] += dRx;
                }
                // Set address offset register:                    
                cpu->registers.MOFF = phri_cpu_rdr(cpu, Rx);
                
                // Set the address pins on the bus:
                phri_cpu_bus_setaddr(cpu, false);
                
                // Load existing value from memory
                if ( is_mem_read ) phri_cpu_bus_rd(cpu);
                
                if ( cpu->registers.INSTR & kphri_data_op_store ) {
                    // Set the data bus:
                    cpu->bus->data = (cpu->bus->data & mask) | (cpu->registers.R[Rd] & set);
                    
                    // Write the data:
                    phri_cpu_bus_wr(cpu);
                } else {
                    // Set register
                    phri_cpu_wrr(cpu, Rd, (cpu->registers.R[Rd] & mask) | (cpu->bus->data & set));
                    
                    // Post-adjust register value if auto-increment was enabled:
                    cpu->registers.R[Rx] += dRx;
                }
            }
        } else {
            phri_byte_t     dsti, shift2;
            phri_word_t     arg1, arg2, mask1;
            
            if ( cpu->registers.INSTR & kphri_data_op_mov_8b_mask ) {
                // 8-bit immediate mode
                arg1 = phri_cpu_rdr(cpu, dsti);
                arg2 = (cpu->registers.INSTR & kphri_data_op_mov_8b_imm8_mask);
                dsti = (cpu->registers.INSTR & kphri_data_op_mov_dsti);
                if ( cpu->registers.INSTR & kphri_data_op_mov_8b_lohi_mask ) {
                    shift2 = (8 - kphri_data_op_mov_8b_imm8_shift);
                    mask1 = 0x00FF;
                } else {
                    shift2 = 0x80 | kphri_data_op_mov_8b_imm8_shift;
                    mask1 = 0xFF00;
                }
            } else if ( cpu->registers.INSTR & kphri_data_op_mov_shreg_mask ) {
                // Shifted register
                dsti = (cpu->registers.INSTR & kphri_data_op_mov_dsti);
                arg1 = mask1 = 0x0000;
                arg2 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_mov_shreg_srci_mask) >> kphri_data_op_mov_shreg_srci_shift);
                shift2 = ((cpu->registers.INSTR & kphri_data_op_mov_shreg_shift_mask) >> kphri_data_op_mov_shreg_shift_shift) |
                        ((cpu->registers.INSTR & kphri_data_op_mov_shreg_lr_mask) ? 0x80 : 0x00);
            } else if ( cpu->registers.INSTR & kphri_data_op_mov_4b_mask ) {
                // 4-bit immediate mode
                dsti = (cpu->registers.INSTR & kphri_data_op_mov_dsti);
                shift2 = (cpu->registers.INSTR & kphri_data_op_mov_4b_shift_mask) >> (kphri_data_op_mov_4b_shift_shift - 2);
                arg2 = ((cpu->registers.INSTR & kphri_data_op_mov_4b_imm4_mask) >> kphri_data_op_mov_4b_imm4_shift) << shift2;
                if ( cpu->registers.INSTR & kphri_data_op_mov_4b_zero_mask ) {
                    arg1 = mask1 = 0x0000;
                } else {
                    arg1 = phri_cpu_rdr(cpu, dsti);
                    mask1 = ~(0x000F << shift2);
                }
            } else if ( cpu->registers.INSTR & kphri_data_op_movn_mask ) {
                // 4-bit negated immediate mode
                dsti = (cpu->registers.INSTR & kphri_data_op_mov_dsti);
                shift2 = (cpu->registers.INSTR & kphri_data_op_mov_4b_shift_mask) >> (kphri_data_op_mov_4b_shift_shift - 2);
                arg2 = ((cpu->registers.INSTR & kphri_data_op_mov_4b_imm4_mask) >> kphri_data_op_mov_4b_imm4_shift) ^ 0x000F;
                arg1 = 0xFFFF;
                mask1 = ~(0x000F << shift2);
            } else if ( cpu->registers.INSTR & kphri_data_op_bswp_mask ) {
                // Byte swap mode
                dsti = (cpu->registers.INSTR & kphri_data_op_mov_dsti);
                arg1 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_bswp_srci_mask) >> kphri_data_op_bswp_srci_shift) >> 8;
                mask1 = 0x00FF;
                arg2 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_bswp_srci_mask) >> kphri_data_op_bswp_srci_shift);
                shift2 = 8;
            } else if ( cpu->registers.INSTR & kphri_data_op_dseg_mask ) {
                if ( cpu->registers.INSTR & kphri_data_op_dseg_4b_mask ) {
                    // DSEG <- 4-bit immediate
                    dsti = kphri_register_index_MSEG;
                    arg1 = phri_cpu_rdr(cpu, kphri_register_index_MSEG);
                    mask1 = 0x00F0;
                    arg2 = (cpu->registers.INSTR & kphri_data_op_dseg_4b_imm4);
                    shift2 = 0;
                } else {
                    // Register <-> DSEG
                    if ( cpu->registers.INSTR & kphri_data_op_pcdseg_rd_mask ) {
                        dsti = (cpu->registers.INSTR & kphri_data_op_mov_dsti);
                        arg1 = phri_cpu_rdr(cpu, kphri_register_index_MSEG);
                        mask1 = 0x000F;
                        arg2 = 0x0000;
                        shift2 = 0;
                    } else {
                        dsti = kphri_register_index_MSEG;
                        arg1 = phri_cpu_rdr(cpu, kphri_register_index_MSEG);
                        mask1 = 0x00F0;
                        arg2 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_mov_dsti)) & 0xF;
                        shift2 = 0;
                    }
                }
            } else if ( cpu->registers.INSTR & kphri_data_op_pc_mask ) {
                // Register <-> PC
                if ( cpu->registers.INSTR & kphri_data_op_pcdseg_rd_mask ) {
                    dsti = (cpu->registers.INSTR & kphri_data_op_mov_dsti);
                    arg1 = mask1 = 0x0000;
                    arg2 = phri_cpu_rdr(cpu, kphri_register_index_PC);
                    shift2 = 0;
                } else {
                    dsti = kphri_register_index_PC;
                    arg1 = mask1 = 0x0000;
                    arg2 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_mov_dsti));
                    shift2 = 0;
                }
            }
            phri_cpu_wrr(cpu, dsti, (arg1 & mask1) | ((shift2 & 0x80) ? (arg2 >> (0x80 ^ shift2)) : (arg2 << shift2)));
        }
    }
}
