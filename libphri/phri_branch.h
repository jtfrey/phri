/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares the constants used to decode branch instructions.
 *
 */

#ifndef __PHRI_BRANCH_H__
#define __PHRI_BRANCH_H__

#include "phri.h"

/**
 * Branching instruction decode masks
 * Bitmasks that are used to decode branching instructions.
 */
enum __attribute__((packed)) {
    kphri_branch_op_link            = 0b0010000000000000,   /*!< link -- copy PC to a register */
    kphri_branch_op_cond            = 0b0001000000000000,   /*!< instruction is conditional */
    kphri_branch_op_cond_mask       = 0b0000111000000000,   /*!< condition code bitmask */
    kphri_branch_op_cond_shift      = 9,                    /*!< condition code must be shifted
                                                                 down 9 indices */
    kphri_branch_op_const           = 0b0000100000000000,   /*!< 11-bit constant */
    kphri_branch_op_const_mask      = 0b0000011111111111,   /*!< 11-bit constant */
    kphri_branch_op_rel             = 0b0000000000001000,   /*!< register treated as relative? */
    kphri_branch_op_cond_const      = 0b0000000100000000,   /*!< 8-bit constant */
    kphri_branch_op_cond_const_mask = 0b0000000011111111,   /*!< 8-bit constant */
    kphri_branch_op_ry_mask         = 0b0000000001110000,   /*!< bitmask for register index y */
    kphri_branch_op_ry_shift        = 4,                    /*!< Ry index must be shifted down
                                                                 4 indices */
    kphri_branch_op_rx_mask         = 0b0000000000000111    /*!< bitmask for register index x */
};

#endif /* __PHRI_BRANCH_H__ */

