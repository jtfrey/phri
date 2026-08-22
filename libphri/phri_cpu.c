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

void
phri_cpu_execinstr(
    phri_cpu_t  *cpu
)
{
    // One cycle for decode, additional cycles added as necessary (memory requests, etc.)
    cpu->cycles++;
    
    if ( cpu->registers.INSTR & kphri_instr_kind_alu_mask ) {
        // ALU:
        phri_word_t     uop = (cpu->registers.INSTR & kphri_alu_op_uop_mask);
        phri_byte_t     dsti = (cpu->registers.INSTR & kphri_alu_op_rd_mask);
        phri_bit_t      setflags = (cpu->registers.INSTR & kphri_alu_op_scond) ? kphri_bit_on : kphri_bit_off;
        
        phri_alu_init(cpu->alu);
        if ( (uop == kphri_alu_op_splmnt) && (cpu->registers.INSTR & kphri_alu_op_splmnt_cmpn_mask) != kphri_alu_op_splmnt_cmpn ) {
            /* Status register changes */
            dsti = kphri_register_index_SSR;
            cpu->alu.path = kphri_alu_path_logic;
            cpu->alu.uop = (cpu->registers.INSTR & kphri_alu_op_splmnt_sr_op_mask) >> kphri_alu_op_splmnt_sr_op_shift;
            cpu->alu.arg1 = phri_cpu_rdr(cpu, kphri_register_index_SSR);
            cpu->alu.arg2 = (cpu->registers.INSTR & kphri_alu_op_splmnt_sr_const) << 8;
            if ( cpu->alu.uop == kphri_alu_uop_logic_and ) cpu->alu.arg2 |= 0x00FF;
            setflags = kphri_bit_off;
        } else {
            /* Decode operands */
            cpu->alu.path = (cpu->registers.INSTR & kphri_alu_op_class_mask) >> kphri_alu_op_class_shift;
            if ( (cpu->registers.INSTR & kphri_alu_op_imm7) || (cpu->alu.path == kphri_alu_path_splmnt) ) {
                /* Rx, #<IMM7> */
                cpu->alu.arg1 = phri_cpu_rdr(cpu, dsti);
                cpu->alu.arg2 = (cpu->registers.INSTR & kphri_alu_op_imm7_mask) >> kphri_alu_op_imm7_shift;
            }
            else {
                /* Rx, Ry|#<IMM3> */
                cpu->alu.arg1 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_alu_op_rx_mask) >> kphri_alu_op_rx_shift);
                cpu->alu.arg2 = (cpu->registers.INSTR & kphri_alu_op_imm3_mask) >> kphri_alu_op_imm3_shift;
                if ( ! (cpu->registers.INSTR & kphri_alu_op_imm3) ) {
                    /* Special case, R0 */
                    if ( cpu->alu.arg2 ) {
                        cpu->alu.arg2 = phri_cpu_rdr(cpu, cpu->alu.arg2);
                    } else {
                        cpu->alu.arg2 = ( !cpu->alu.arg2 && ((uop == kphri_alu_op_and) || (uop == kphri_alu_op_xor))) ? 0xFFFF : 0x0000;
                    }
                } else if ( cpu->alu.path == kphri_alu_path_bitshift ) {
                    if ( cpu->alu.arg2 == 0 ) cpu->alu.arg2 = 1;
                }
            }
            cpu->alu.carry_in = (cpu->registers.F & kphri_sb_c) ? kphri_bit_on : kphri_bit_off;
            switch ( cpu->alu.path ) {
                case kphri_alu_path_arith:
                case kphri_alu_path_logic:
                    cpu->alu.uop = (uop >> kphri_alu_op_uop_shift) & 0b11;
                    break;
                case kphri_alu_path_bitshift:
                    cpu->alu.uop = (cpu->registers.INSTR & kphri_alu_op_bitshiftmode_mask) >> kphri_alu_op_bitshiftmode_shift;
                    break;
                case kphri_alu_path_splmnt:
                    dsti = 0;
                    cpu->alu.carry_in = kphri_bit_off;
                    cpu->alu.uop = ((cpu->registers.INSTR & kphri_alu_op_splmnt_uop_mask) ^ kphri_alu_op_splmnt_uop_mask) \
                                            >> kphri_alu_op_splmnt_uop_shift;
                    break;
            }
        }
        phri_alu_exec(&cpu->alu);
        phri_cpu_wrr(cpu, dsti, cpu->alu.result);
        if ( setflags ) cpu->registers.F = (cpu->registers.F & ~cpu->alu.flags_mask) | cpu->alu.flags;
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
            phri_byte_t     S, Rd = cpu->registers.INSTR & kphri_data_op_mem_rd_mask;
            phri_byte_t     Rx = (cpu->registers.INSTR & kphri_data_op_mem_rx_mask) >> kphri_data_op_mem_rx_shift;
            phri_word_t     is_cond = 1;
            phri_word_t     dRx = 0;            
            phri_word_t     mask = 0x0000, set=0xFFFF;
            phri_bit_t      is_mem_read = (!(cpu->registers.INSTR & kphri_data_op_mem_store)) ? kphri_bit_on : kphri_bit_off;
            
            if ( cpu->registers.INSTR & kphri_data_op_mem_autoinc ) {
                // Auto-increment instruction:
                if ( cpu->registers.INSTR & kphri_data_op_mem_autoinc_ry ) {
                    // Auto-increment comes from Ry
                    dRx = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_mem_ry_mask) >> kphri_data_op_mem_ry_shift);
                } else {
                    // Auto-increment comes from a 4-bit constant
                    dRx = (phri_word_t)(cpu->registers.INSTR & kphri_data_op_mem_autoinc_const_mask) >> kphri_data_op_mem_autoinc_const_shift;
                    // Sign-extend from 4-bit to 16-bit if necessary:
                    if ( dRx & 0b1000 ) dRx |= 0b1111111111110000;
                }
            } else if ( cpu->registers.INSTR & kphri_data_op_mem_cond ) {
                // Conditional
                phri_word_t  cond = (cpu->registers.INSTR & kphri_data_op_mem_cc_mask) >> kphri_data_op_mem_cc_shift;
                is_cond = phri_cpu_check_cc(cpu, cond);
            } else if ( cpu->registers.INSTR & kphri_data_op_mem_offset_ry ) {
                // Rx offset by Ry
                cpu->registers.MOFF += phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_data_op_mem_ry_mask) >> kphri_data_op_mem_ry_shift);
            } else {
                // 8-bit options
                if ( cpu->registers.INSTR & kphri_data_op_mem_8b_hi ) {
                    // 8-bit, MSB
                    mask = 0x00FF, set = 0xFF00, is_mem_read = kphri_bit_on;
                } else {
                    // 8-bit, LSB
                    mask = 0xFF00, set = 0x00FF, is_mem_read = kphri_bit_on;
                }
            }
            
            if ( is_cond ) {
                if ( cpu->registers.INSTR & kphri_data_op_mem_store ) {
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
                
                if ( cpu->registers.INSTR & kphri_data_op_mem_store ) {
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
            phri_byte_t     dsti, srci;
            
            if ( cpu->registers.INSTR & kphri_data_op_rgstrs_imm4 ) {
                srci = dsti = cpu->registers.INSTR & kphri_data_op_rgstrs_dsti_mask;
                cpu->dlu.arg2 = (cpu->registers.INSTR & kphri_data_op_rgstrs_imm4_const_mask) >> kphri_data_op_rgstrs_imm4_const_shift;
                cpu->dlu.mask = (cpu->registers.INSTR & kphri_data_op_rgstrs_imm4_keep) ? 0xFFF0 : 0x0000;
                cpu->dlu.shift = ((cpu->registers.INSTR & kphri_data_op_rgstrs_imm4_rot_mask) >> kphri_data_op_rgstrs_imm4_rot_shift) << 1;
                cpu->dlu.not = (cpu->registers.INSTR & kphri_data_op_rgstrs_imm4_not) ? kphri_bit_on : kphri_bit_off;
            }
            else if ( cpu->registers.INSTR & kphri_data_op_rgstrs_spcl ) {
                if ( cpu->registers.INSTR & kphri_data_op_rgstrs_spcl_const ) {
                    srci = dsti = kphri_register_index_SSR;
                    cpu->dlu.arg2 = (cpu->registers.INSTR & kphri_data_op_rgstrs_spcl_imm4_mask);
                    cpu->dlu.mask = 0x0000;
                    cpu->dlu.shift = 0;
                    cpu->dlu.not = kphri_bit_off;
                }
                else {
                    if ( cpu->registers.INSTR & kphri_data_op_rgstrs_spcl_from_gp ) {
                        srci = cpu->registers.INSTR & kphri_data_op_rgstrs_spcl_gpi_mask;
                        dsti = (cpu->registers.INSTR & kphri_data_op_rgstrs_spcl_ssr) ? kphri_register_index_SSR : kphri_register_index_PC;
                    } else {
                        srci = (cpu->registers.INSTR & kphri_data_op_rgstrs_spcl_ssr) ? kphri_register_index_SSR : kphri_register_index_PC;
                        dsti = cpu->registers.INSTR & kphri_data_op_rgstrs_spcl_gpi_mask;
                    }                    
                    cpu->dlu.arg2 = 0x0000;
                    cpu->dlu.mask = 0xFFFF;
                    cpu->dlu.shift = 0;
                    cpu->dlu.not = kphri_bit_off;
                }
            }
            cpu->dlu.arg1 = phri_cpu_rdr(cpu, srci);
            phri_dlu_exec(&cpu->dlu);
            phri_cpu_wrr(cpu, dsti, cpu->dlu.result);
        }
    }
}
