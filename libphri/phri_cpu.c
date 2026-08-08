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
        phri_word_t     opcode = (cpu->registers.INSTR & kphri_instr_mask_alu_opcd);
        phri_byte_t     set_cond = (cpu->registers.INSTR & kphri_instr_mask_alu_scond) ? 1 : 0;
        phri_word_t     Rd = (cpu->registers.INSTR & kphri_instr_mask_alu_dst);
        phri_word_t     Arg1 = 0x0000, Arg2 = 0x0000, Arg3 = 0x0000, Accum;
        
        if ( (opcode == kphri_alu_op_misc) && ((cpu->registers.INSTR & kphri_alu_op_misc_cmpn_mask) != kphri_alu_op_misc_cmpn) ) {
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
        } else {
            phri_byte_t     set_dest = 1;
            
            if ( cpu->registers.INSTR & kphri_instr_mask_alu_src1t ) {
                // 7-bit const, sign-extend or bias if necessary
                Arg1 = phri_cpu_rdr(cpu, Rd);
                Arg2 = (cpu->registers.INSTR & kphri_instr_mask_alu_src1c) >> kphri_instr_mask_alu_src2_shift;
                switch ( opcode ) {
                    case kphri_alu_op_shr:
                        // Carry bit will be selected from this position:
                        Arg3 = (Arg2 & kphri_instr_mask_alu_src1_shCbit) >> kphri_instr_mask_alu_src1_shCbit_shift;
                        // Shift distance from the rest of the 7-bit constant:
                        Arg2 &= kphri_instr_mask_alu_src1_shdist;
                        break;
                    case kphri_alu_op_shl:
                        // Carry bit will be selected from this position:
                        Arg3 = 8 + (((Arg2 & kphri_instr_mask_alu_src1_shCbit) >> kphri_instr_mask_alu_src1_shCbit_shift) ^ 0b111);
                        // Shift distance from the rest of the 7-bit constant:
                        Arg2 &= kphri_instr_mask_alu_src1_shdist;
                        break;
                    case kphri_alu_op_and:
                    case kphri_alu_op_or:
                    case kphri_alu_op_xor:
                        break;
                    default:
                        // Sign-extend the 7-bit constant to 16-bit:
                        if ( Arg2 & 0b1000000 ) Arg2 |= 0b1111111110000000;
                        break;
                }
            } else {
                Arg1 = cpu->registers.R[(cpu->registers.INSTR & kphri_instr_mask_alu_src1i) >> 7];
                if ( cpu->registers.INSTR & kphri_instr_mask_alu_src2t ) {
                    // 3-bit const, sign-extend or bias if necessary
                    Arg2 = (cpu->registers.INSTR & kphri_instr_mask_alu_src2c) >> 3;
                    switch ( opcode ) {
                        case kphri_alu_op_shr:
                        case kphri_alu_op_shl:
                            // The 3-bit constant for SHR/SLH gets incremented by one so the range is [1, 8]:
                            Arg2++;
                            break;
                        case kphri_alu_op_and:
                        case kphri_alu_op_or:
                        case kphri_alu_op_xor:
                            break;
                        default:
                            // Sign-extend the 3-bit constant to 16-bit:
                            if ( Arg2 & 0b100 ) Arg2 |= 0b1111111111111000;
                            break;
                    }
                } else {
                    Arg2 = phri_cpu_rdr(cpu, (cpu->registers.INSTR & kphri_instr_mask_alu_src2i) >> 3);
                    if ( ((cpu->registers.INSTR & kphri_instr_mask_alu_src2i) == 0) &&
                           (opcode == kphri_alu_op_and || opcode == kphri_alu_op_xor) ) Arg2 = ~Arg2;
                }
            }
            
            // Operands have been decoded
            switch ( opcode ) {
                case kphri_alu_op_add:
                    Accum = phri_word_add(
                                Arg1,
                                Arg2 + ((set_cond && (cpu->registers.F & kphri_sb_c)) ? 1 : 0),
                                set_cond ? &cpu->registers.F : NULL,
                                0);
                    break;
                case kphri_alu_op_sub:
                    Accum = phri_word_add(
                                Arg1,
                                ~(Arg2 + ((set_cond && (cpu->registers.F & kphri_sb_c)) ? 1 : 0)) + 1,
                                set_cond ? &cpu->registers.F : NULL,
                                1);
                    break;
                case kphri_alu_op_shl:
                    Accum = Arg1 << Arg2;
                    // If SHCL and the C flag is set, set the shifted-in bits to 1:
                    if ( set_cond && (cpu->registers.F & kphri_sb_c) ) Accum |= (0b1111111111111111 >> (16 - Arg2));
                    // Set the C flag to the value of the desired bit index:
                    if ( Accum & (1 << Arg3) )
                        cpu->registers.F |= kphri_sb_c;
                    else
                        cpu->registers.F &= ~kphri_sb_c;
                    break;
                case kphri_alu_op_shr:
                    Accum = Arg1 >> Arg2;
                    // If SHCR and the C flag is set, set the shifted-in bits to 1:
                    if ( set_cond && (cpu->registers.F & kphri_sb_c) ) Accum |= (0b1111111111111111 << (16 - Arg2));
                    // Set the C flag to the value of the desired bit index:
                    if ( Accum & (1 << Arg3) )
                        cpu->registers.F |= kphri_sb_c;
                    else
                        cpu->registers.F &= ~kphri_sb_c;
                    break;
                case kphri_alu_op_and:
                    Accum = Arg1 & Arg2;
                    // If ANDS, copy specific bit-index values from the result to the status register:
                    if ( set_cond ) cpu->registers.F = ((Accum & 0b1000000000000000) ? kphri_sb_m : 0) |
                                                       ((Accum & 0b0100000000000000) ? kphri_sb_v : 0) |
                                                       ((Accum & 0b0000000000000001) ? kphri_sb_c : 0) |
                                                        (Accum ? 0 : kphri_sb_z);
                    break;
                case kphri_alu_op_or:
                    Accum = Arg1 | Arg2;
                    // If ORS, copy specific bit-index values from the result to the status register:
                    if ( set_cond ) cpu->registers.F = ((Accum & 0b1000000000000000) ? kphri_sb_m : 0) |
                                                       ((Accum & 0b0100000000000000) ? kphri_sb_v : 0) |
                                                       ((Accum & 0b0000000000000001) ? kphri_sb_c : 0) |
                                                        (Accum ? 0 : kphri_sb_z);
                    break;
                case kphri_alu_op_xor:
                    Accum = Arg1 ^ Arg2;
                    // If XORS, copy specific bit-index values from the result to the status register:
                    if ( set_cond ) cpu->registers.F = ((Accum & 0b1000000000000000) ? kphri_sb_m : 0) |
                                                       ((Accum & 0b0100000000000000) ? kphri_sb_v : 0) |
                                                       ((Accum & 0b0000000000000001) ? kphri_sb_c : 0) |
                                                        (Accum ? 0 : kphri_sb_z);
                    break;
                case kphri_alu_op_misc: {
                    // There is no writeback to any registers for these instructions:
                    set_dest = 0;
                    if ( (cpu->registers.INSTR & kphri_alu_op_misc_cmpn_mask) == kphri_alu_op_misc_cmpn ) {
                        if ( (cpu->registers.INSTR & kphri_alu_op_misc_cmp_mask) == kphri_alu_op_misc_cmn ) {
                            // CMN is an addition with the result discarded:
                            phri_word_add(Arg1, Arg2, &cpu->registers.F, 0);
                        } else {
                            // CMP is a subtraction with the result discarded:
                            phri_word_add(Arg1, ~Arg2 + 1, &cpu->registers.F, 1);
                        }
                    }
                    break;
                }
            }
            // Write result to Rd if the instruction calls for that:
            if ( set_dest ) phri_cpu_wrr(cpu, Rd, Accum);
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
        // Data movement; the destination register is a common component across
        // many of the opcodes, so decode that now:
        unsigned int    Rd = cpu->registers.INSTR & kphri_data_op_rd_mask;
        // The data movement unit has it's own status register for address computation
        phri_byte_t     S;
        
        if ( cpu->registers.INSTR & kphri_data_op_mem ) {
            // Memory-based instructions
            phri_word_t Rx = (cpu->registers.INSTR & kphri_data_op_rx_mask) >> kphri_data_op_rx_shift;
            phri_word_t is_cond = 1;
            phri_word_t dRx = 0;            
            phri_word_t mask = 0x0000, set=0xFFFF, is_writeback=0;
            
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
                    mask = 0x00FF, set = 0xFF00, is_writeback = 1;
                } else {
                    // 8-bit, LSB
                    mask = 0xFF00, set = 0x00FF, is_writeback = 1;
                }
            }
            
            if ( is_cond ) {
                if ( cpu->registers.INSTR & kphri_data_op_store ) {
                    // Store
                    // Pre-adjust register value if auto-increment was enabled:
                    cpu->registers.R[Rx] += dRx;
                    
                    // Set address offset register:                    
                    cpu->registers.MOFF = phri_cpu_rdr(cpu, Rx);
                    
                    // Set the address pins on the bus:
                    phri_cpu_bus_setaddr(cpu, false);
                    
                    // If we're doing a half-word store, load the existing value
                    if ( is_writeback ) phri_cpu_bus_rd(cpu);
                    
                    // Set the data bus:
                    cpu->bus->data = (cpu->bus->data & mask) | (cpu->registers.R[Rd] & set);
                    
                    // Write the data:
                    phri_cpu_bus_wr(cpu);
                } else {
                    // Load
                    // Set address registers:
                    cpu->registers.MOFF = cpu->registers.R[Rx];
                    
                    // Read the data
                    phri_cpu_fetchdata(cpu);
                    
                    // Set register
                    phri_cpu_wrr(cpu, Rd, (cpu->registers.R[Rd] & mask) | (cpu->bus->data & set));
                    
                    // Post-adjust register value if auto-increment was enabled:
                    cpu->registers.R[Rx] += dRx;
                }
            }
        } else {
            // Interregister instructions
            if ( cpu->registers.INSTR & kphri_data_op_ir_byte ) {
                // Byte-based LDR/STR:
                if ( cpu->registers.INSTR & kphri_data_op_ir_byte_hi ) {
                    phri_cpu_wrr(cpu, Rd, (cpu->registers.R[Rd] & 0x00FF) | ((cpu->registers.INSTR << 5) & 0xFF00));
                } else {
                    phri_cpu_wrr(cpu, Rd, (cpu->registers.R[Rd] & 0xFF00) | ((cpu->registers.INSTR >> 3) & 0x00FF));
                }
            }
            else {
                uint16_t    Rx = (cpu->registers.INSTR >> 3) & 0b111;
                
                if ( cpu->registers.INSTR & kphri_data_op_ir_select ) {
                    // Conditional selection
                    uint16_t    Ry = (cpu->registers.INSTR >> 6) & 0b111;
                    
                    phri_cpu_wrr(cpu, Rd, cpu->registers.R[Rd] ? cpu->registers.R[Rx] : cpu->registers.R[Ry]);
                }
                else if ( cpu->registers.INSTR & kphri_data_op_ir_mseg ) {
                    // MSEG register
                    if ( cpu->registers.INSTR & kphri_data_op_ir_mseg_imm4 ) {
                        cpu->registers.DSEG = (cpu->registers.INSTR >> 3) & 0b11;
                    }
                    else if ( (cpu->registers.INSTR & kphri_data_op_ir_mseg_rx) == kphri_data_op_ir_mseg_rx ) {
                        cpu->registers.DSEG = cpu->registers.R[(cpu->registers.INSTR >> 3) & 0b111] & 0xF;
                    }
                    else {
                        phri_cpu_wrr(cpu, cpu->registers.INSTR & 0b111, cpu->registers.MSEG);
                    }
                }
                else if ( cpu->registers.INSTR & kphri_data_op_ir_pc ) {
                    // PC register
                    if ( (cpu->registers.INSTR & kphri_data_op_ir_pc_set) == kphri_data_op_ir_pc_set ) {
                        cpu->registers.PC = cpu->registers.R[Rx];
                    } else {
                        phri_cpu_wrr(cpu, Rd, cpu->registers.PC + cpu->registers.R[Rx]);
                    }
                }
                else {
                    phri_cpu_wrr(cpu, Rd, (cpu->registers.INSTR & kphri_data_op_ir_negate) ? ~cpu->registers.R[Rx] : cpu->registers.R[Rx]);
                }
            }
        }
    }
}
