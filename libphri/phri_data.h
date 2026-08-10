/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares the constants used to decode data movement instructions.
 *
 */

#ifndef __PHRI_DATA_H__
#define __PHRI_DATA_H__

#include "phri.h"

/**
 * Data movement instruction decode
 * Bitmasks used to decode a data movement instruction.
 */
enum __attribute__((packed)) {
    kphri_data_op_mem                   = 0b0010000000000000,   /*!< Memory-based instructions */
    kphri_data_op_mov_8b_mask           = 0b0001000000000000,   /*!< If this bit is set, it's an
                                                                     8-bit immediate value */
    kphri_data_op_mov_8b_lohi_mask      = 0b0000100000000000,   /*!< Lo/hi byte dest, 0=lo, 1=hi */
    kphri_data_op_mov_8b_imm8_mask      = 0b0000011111111000,   /*!< Mask for 8-bit immediate */
    kphri_data_op_mov_8b_imm8_shift     = 3,                    /*!< Shift of 8-bit immediate */
    
    kphri_data_op_mov_shreg_mask        = 0b0000100000000000,   /*!< If this bit is set, it's a
                                                                     shifted-register move */
    kphri_data_op_mov_shreg_lr_mask     = 0b0000010000000000,   /*!< Shift direction, 1=R, 0=L */
    kphri_data_op_mov_shreg_shift_mask  = 0b0000001111000000,   /*!< Immediate shift size */
    kphri_data_op_mov_shreg_shift_shift = 6,                    /*!< Shift of shift size */
    kphri_data_op_mov_shreg_srci_mask   = 0b0000000000111000,   /*!< Shifted source register */
    kphri_data_op_mov_shreg_srci_shift  = 3,                    /*!< Shift source index */
    
    kphri_data_op_mov_4b_mask           = 0b0000010000000000,   /*!< If this bit is set, it's a
                                                                     4-bit shifted immediate */
    kphri_data_op_mov_4b_zero_mask      = 0b0000001000000000,   /*!< 1=zero other bits, 0=keep
                                                                     other bits */
    kphri_data_op_mov_4b_shift_mask     = 0b0000000110000000,   /*!< Shift size, 00=0, 01=4,
                                                                     10=8, 11=12 */
    kphri_data_op_mov_4b_shift_shift    = 7,                    /*!< Shift for shift size */
    kphri_data_op_mov_4b_imm4_mask      = 0b0000000001111000,   /*!< Mask for 4-bit immediate */
    kphri_data_op_mov_4b_imm4_shift     = 3,                    /*!< Shift of 4-bit immediate */
    
    kphri_data_op_movn_mask             = 0b0000001000000000,   /*!< If this bit is set, it's a
                                                                     negated 4-bit immediate */
    /* The operands for the 4-bit immediate shifted MOV are reused by MVN */
    
    kphri_data_op_bswp_mask             = 0b0000000001000000,   /*!< If this bit is set, it's a
                                                                     register byte-swap */
    kphri_data_op_bswp_srci_mask        = 0b0000000000111000,   /*!< The source index */
    kphri_data_op_bswp_srci_shift       = 3,                    /*!< Shift of source index */
    
    kphri_data_op_dseg_mask             = 0b0000000000100000,   /*!< If this bit is set, it's a
                                                                     DSEG operation */
    kphri_data_op_dseg_4b_mask          = 0b0000000000010000,   /*!< If this bit is set, it's a
                                                                     4-bit immediate */
    kphri_data_op_dseg_4b_imm4          = 0b0000000000001111,   /*!< Mask for 4-bit immediate */
    
    kphri_data_op_pc_mask               = 0b0000000000010000,   /*!< If this bit is set, it's a
                                                                     PC operation */
    kphri_data_op_pcdseg_rd_mask        = 0b0000000000001000,   /*!< For register-based, if this is
                                                                     set it's Rx, DSEG|PC */
    /* The arbitrary register index = dsti */
    
    kphri_data_op_mov_dsti              = 0b0000000000000111,   /*!< Destination register index */
    
    /* Memory mode components: */
    kphri_data_op_cond                  = 0b0000010000000000,   /*!< Operation is conditional */
    kphri_data_op_16b_mask              = 0b0001111110000000,   /*!< Data width is 16-bit if non-zero */
    kphri_data_op_8b_hi                 = 0b0000000001000000,   /*!< Data width is 8-bit and high byte */
    kphri_data_op_autoinc               = 0b0000100000000000,   /*!< Automatically alter the address
                                                                 register */
    kphri_data_op_offset_ry             = 0b0000001000000000,   /*!< Target addr offset by Ry */
    kphri_data_op_autoinc_ry            = 0b0000010000000000,   /*!< Target addr offset by Ry not constant */
    kphri_data_op_autoinc_const_mask    = 0b0000001111000000,   /*!< Auto-increment 4-bit constant mask */
    kphri_data_op_autoinc_const_shift   = 6,                    /*!< AUto-increment 4-bit consant shift */
    kphri_data_op_store                 = 0b0001000000000000,   /*!< If set, STR; if clear, LDR */
    kphri_data_op_rd_mask               = 0b0000000000000111,   /*!< Mask for Rd register index */
    kphri_data_op_rx_mask               = 0b0000000000111000,   /*!< Mask for Rx register index */
    kphri_data_op_rx_shift              = 3,                    /*!< Shift for Rx register index */
    kphri_data_op_ry_mask               = 0b0000000111000000,   /*!< Mask for Ry register index */
    kphri_data_op_ry_shift              = 6,                    /*!< Shift for Ry register index */
    kphri_data_op_cc_mask               = 0b0000000111000000,   /*!< Mask for condition code value */
    kphri_data_op_cc_shift              = 6                    /*!< Shift for condition code value */
};

#endif /* __PHRI_DATA_H__ */

