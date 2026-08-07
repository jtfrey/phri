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
    kphri_data_op_ir_byte               = 0b0001000000000000,   /*!< In interregister modes, act on a
                                                                 single byte */
    kphri_data_op_ir_byte_hi            = 0b0000100000000000,   /*!< When acting on single bytes, affect
                                                                 high byte on 1, low byte on 0 */
    kphri_data_op_ir_select             = 0b0000001000000000,   /*!< Conditional selection */
    kphri_data_op_ir_mseg               = 0b0000000100000000,   /*!< MSEG register */
    kphri_data_op_ir_mseg_imm4          = 0b0000000110000000,   /*!< MSEG, set with 4-bit immed */
    kphri_data_op_ir_mseg_rx            = 0b0000000101000000,   /*!< MSEG, set with register */
    kphri_data_op_ir_pc                 = 0b0000000010000000,   /*!< PC register */
    kphri_data_op_ir_pc_set             = 0b0000000011000000,   /*!< General register to PC */
    kphri_data_op_ir_pcrel              = 0b0000000010000000,   /*!< PC-relative move */
    kphri_data_op_ir_negate             = 0b0000000001000000,   /*!< negate value from source register */
    
    kphri_data_op_rd_mask               = 0b0000000000000111,   /*!< Mask for Rd register index */
    kphri_data_op_rx_mask               = 0b0000000000111000,   /*!< Mask for Rx register index */
    kphri_data_op_rx_shift              = 3,                    /*!< Shift for Rx register index */
    kphri_data_op_ry_mask               = 0b0000000111000000,   /*!< Mask for Ry register index */
    kphri_data_op_ry_shift              = 6,                    /*!< Shift for Ry register index */
    kphri_data_op_cc_mask               = 0b0000000111000000,   /*!< Mask for condition code value */
    kphri_data_op_cc_shift              = 6                    /*!< Shift for condition code value */
};

#endif /* __PHRI_DATA_H__ */

