/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares the baseline instruction decode constants.
 *
 */

#ifndef __PHRI_INSTR_H__
#define __PHRI_INSTR_H__

#include "phri.h"

/**
 * Instruction category decode constants
 * Bitmasks in this enumeration are used to decode the category an
 * instruction falls under.
 */
typedef enum __attribute__((packed)) {
    kphri_instr_kind_alu_mask       = 0b1000000000000000,   /*!< Mask to check ALU category */
    kphri_instr_kind_alu            = 0b1000000000000000,   /*!< Value to match ALU category */
    kphri_instr_kind_not_alu_mask   = 0b1100000000000000,   /*!< Mask to check non-ALU categories */
    kphri_instr_kind_branch         = 0b0100000000000000,   /*!< Value to match non-ALU branching
                                                                 category */
    kphri_instr_kind_data           = 0b0000000000000000    /*!< Value to match non-ALU data
                                                                 movement category */
} kphri_instr_kind_t;

#endif /* __PHRI_INSTR_H__ */

