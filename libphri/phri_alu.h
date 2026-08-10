/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares the constants used to decode ALU instructions.
 *
 */

#ifndef __PHRI_ALU_H__
#define __PHRI_ALU_H__

#include "phri.h"

/**
 * ALU opcode decode masks
 * Bitmasks that are used in decoding ALU instructions.  The values from
 * \ref kphri_alu_op_add through \ref kphri_alu_op_misc are matched to the
 * bits isolated from the instruction word using \ref kphri_instr_mask_alu_opcd.
 * The miscellaneous instructions are decoded in a second stage by isolating
 * the \ref kphri_alu_op_misc_mask bits and comparing against the remaining
 * constants.
 */
enum __attribute__((packed)) {
    kphri_alu_op_add                = 0b1000000000000000,   /*!< ADD{S} */
    kphri_alu_op_sub                = 0b1001000000000000,   /*!< SUB{S} */
    kphri_alu_op_shl                = 0b1010000000000000,   /*!< SHL{S} */
    kphri_alu_op_shr                = 0b1011000000000000,   /*!< SHR{S} */
    kphri_alu_op_and                = 0b1100000000000000,   /*!< AND{S} */
    kphri_alu_op_or                 = 0b1101000000000000,   /*!< OR{S} */
    kphri_alu_op_xor                = 0b1110000000000000,   /*!< XOR{S} */
    kphri_alu_op_misc               = 0b1111000000000000,   /*!< Miscellaneous */
    kphri_alu_op_misc_cmpn_mask     = 0b0000100000000000,   /*!< 1. Misc: if this bit is set… */
    kphri_alu_op_misc_cmpn          = 0b0000100000000000,   /*!< …it's CMP/CMN */
    kphri_alu_op_misc_cmp_mask      = 0b0000010000000000,   /*!< 1a. CMP/CMN: if this bit is set… */
    kphri_alu_op_misc_cmn           = 0b0000010000000000,   /*!< …it's CMN */
    kphri_alu_op_misc_sr_mask       = 0b0000010000000000,   /*!< 2. Misc: if this bit is set… */
    kphri_alu_op_misc_sr            = 0b0000010000000000,   /*!< …it's an SR instruction */
    kphri_alu_op_misc_sr_op_mask    = 0b0000001100000000,   /*!< 2a. The bits in these positions… */
    kphri_alu_op_misc_sr_op_and     = 0b0000000000000000,   /*!< …select AND… */
    kphri_alu_op_misc_sr_op_or      = 0b0000000100000000,   /*!< …select OR… */
    kphri_alu_op_misc_sr_op_xor     = 0b0000001000000000,   /*!< …select XOR… */
    kphri_alu_op_misc_sr_op_set     = 0b0000001100000000,   /*!< …or SET. */
};

/**
 * ALU instruction decode masks
 * Bitmasks that are used to isolate components of an ALU instruction.
 */
enum __attribute__((packed)) {
    kphri_instr_mask_alu_opcd       = 0b1111000000000000,   /*!< bitmask for the opcode */
    kphri_instr_mask_alu_scond      = 0b0000100000000000,   /*!< bitmask for set condition flag */
    kphri_instr_mask_alu_src1t      = 0b0000010000000000,   /*!< bitmask for source 1 type */
    kphri_instr_mask_alu_src1i      = 0b0000001110000000,   /*!< bitmask for source index 1 */
    kphri_instr_mask_alu_src1c      = 0b0000001111111000,   /*!< bitmask for source const 1 */
    kphri_instr_mask_alu_src1_shCbit        = 0b1110000,
    kphri_instr_mask_alu_src1_shCbit_shift  = 4,
    kphri_instr_mask_alu_src1_shdist        = 0b0001111,
    kphri_instr_mask_alu_src2t      = 0b0000000001000000,   /*!< bitmask for source 2 type */
    kphri_instr_mask_alu_src2i      = 0b0000000000111000,   /*!< bitmask for source index 2 */
    kphri_instr_mask_alu_src2c      = 0b0000000000111000,   /*!< bitmask for source const 2 */
    kphri_instr_mask_alu_src2_shift = 3,                    /*!< bit shift for source 2 */
    kphri_instr_mask_alu_dst        = 0b0000000000000111,   /*!< bitmask for dest index */
    kphri_instr_mask_misc_sr_const  = 0b0000000000001111  /*!< bitmask for the SR_ constant */
};

#endif /* __PHRI_ALU_H__ */

