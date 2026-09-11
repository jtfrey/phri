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
    kphri_data_op_mem                       = 0b0010000000000000,   /*!< Memory-based instructions if set */
    
    kphri_data_op_rgstrs_imm4               = 0b0001000000000000,   /*!< If set, register set using 4-bit immediate */
    kphri_data_op_rgstrs_imm4_not           = 0b0000100000000000,   /*!< Bitwise NOT the new value */
    kphri_data_op_rgstrs_imm4_keep          = 0b0000010000000000,   /*!< Retain unchanged bits of dest register value */
    kphri_data_op_rgstrs_imm4_rot_mask      = 0b0000001110000000,   /*!< Right-rotate the 4-bit immediate 2x this many bits */
    kphri_data_op_rgstrs_imm4_rot_shift     = 7,                    /*!< Shift the masked rotate size */
    kphri_data_op_rgstrs_imm4_const_mask    = 0b0000000001111000,   /*!< The 4-bit constant to introduce */
    kphri_data_op_rgstrs_imm4_const_shift   = 3,                    /*!< Shift the masked constant */
    kphri_data_op_rgstrs_dsti_mask          = 0b0000000000000111,   /*!< The destination register index */
    
    kphri_data_op_rgstrs_imm8               = 0b0000100000000000,   /*!< If set, register set using 8-bit immediate value */
    kphri_data_op_rgstrs_imm8_const_mask    = 0b0000011111111000,   /*!< The 8-bit constant to use */
    kphri_data_op_rgstrs_imm8_const_shift   = 3,                    /*!< Shift the masked 8-bit constant */
    kphri_data_op_rgstrs_gp_to_ssr          = 0b0000000000100000,   /*!< If set, the SSR is modified using the low nibble of
                                                                         a register */
    kphri_data_op_rgstrs_imm4_to_ssr        = 0b0000000000010000,   /*!< If set, the DSEG of the SSR is set using a 4-bit
                                                                         immediate value */
    kphri_data_op_rgstrs_imm4_to_ssr_mask   = 0b0000000000001111,   /*!< Mask for the 4-bit DSEG constant */
    kphri_data_op_rgstrs_ssr_to_gp          = 0b0000000000001000,   /*!< If set, g.p. register set to SSR */
    kphri_data_op_rgstrs_pc_to_gp_mask      = 0b1111111111111000,   /*!< If all bits save the lowest 3 are unset, this is
                                                                         a copy the PC to a g.p. register; this is never
                                                                         directly tested, it's just the default case if
                                                                         all other patterns are not matched */
    
    /* Memory mode components: */
    kphri_data_op_mem_cond                  = 0b0000010000000000,   /*!< Operation is conditional */
    kphri_data_op_mem_16b_mask              = 0b0001111110000000,   /*!< Data width is 16-bit if non-zero */
    kphri_data_op_mem_8b_hi                 = 0b0000000001000000,   /*!< Data width is 8-bit and high byte */
    kphri_data_op_mem_autoinc               = 0b0000100000000000,   /*!< Automatically alter the address
                                                                         register */
    kphri_data_op_mem_offset_ry             = 0b0000001000000000,   /*!< Target addr offset by Ry */
    kphri_data_op_mem_autoinc_ry            = 0b0000010000000000,   /*!< Target addr offset by Ry not constant */
    kphri_data_op_mem_autoinc_const_mask    = 0b0000001111000000,   /*!< Auto-increment 4-bit constant mask */
    kphri_data_op_mem_autoinc_const_shift   = 6,                    /*!< AUto-increment 4-bit consant shift */
    kphri_data_op_mem_store                 = 0b0001000000000000,   /*!< If set, STR; if clear, LDR */
    kphri_data_op_mem_rd_mask               = 0b0000000000000111,   /*!< Mask for Rd register index */
    kphri_data_op_mem_rx_mask               = 0b0000000000111000,   /*!< Mask for Rx register index */
    kphri_data_op_mem_rx_shift              = 3,                    /*!< Shift for Rx register index */
    kphri_data_op_mem_ry_mask               = 0b0000000111000000,   /*!< Mask for Ry register index */
    kphri_data_op_mem_ry_shift              = 6,                    /*!< Shift for Ry register index */
    kphri_data_op_mem_cc_mask               = 0b0000000111000000,   /*!< Mask for condition code value */
    kphri_data_op_mem_cc_shift              = 6                    /*!< Shift for condition code value */
};

/**
 * PHRI Data Load Unit
 * This structure encompasses the input and output state of the DLU.  Fields are set
 * with input values then the \ref phri_dlu_exec() function is called to compute the
 * result.
 *
 * The DLU takes two words and combines them to produce a value that can be written
 * to a register.  The operations are:
 *
 *     result = {~}((((arg1 ROR shift) & mask) | arg2) ROL shift)
 *
 * If arg1=0x0123, arg2=0x0004, shift=4, and mask is 0xFFF0:
 *
 *     result = ((((0x0123 ROR 12) & 0xFFF0) | 0x0004) ROL 12)
 *            = (((0x1230 & 0xFFF0) | 0x0004) ROL 12)
 *            = ((0x1230 | 0x0004) ROL 12)
 *            = (0x1234 ROL 12)
 *            = 0x4123
 */
typedef struct __attribute__((packed)) {
    phri_word_t     arg1;       /*!< IN:        argument 1 is the starting word state */
    phri_word_t     arg2;       /*!< IN:        argument 2 is the change to be made to that state */
    phri_word_t     mask;       /*!< IN:        significant bits of argument 1 */
    phri_byte_t     shift;      /*!< IN:        the masked-in portion of arg1 is rotated right by this amount, and the
                                                value once augmented by arg2 is rotated left by this amount */
    phri_bit_t      not;        /*!< IN:        bitwise-NOT the result */
    phri_word_t     result;     /*!< OUT:       result of the operation */
} phri_dlu_t;

/**
 * Initialize an DLU structure to all defaults
 * Sets all fields of \p DLU to their defaults.
 */
#define phri_dlu_init(DLU) \
            ((DLU).arg1 = 0, (DLU).arg2 = 0, (DLU).not = kphri_bit_off, (DLU).mask = 0xFFFF, (DLU).shift = 0)

static inline
void
phri_dlu_exec(
    phri_dlu_t  *dlu
)
{
    phri_word_t     result;

    /* Rotate the bits to be modifed into the lowest nibble, then OR-in the new bits:
     *
     *         ((arg1 ROR shift) & mask) | arg2
     */
    result = (((dlu->arg1 >> (16 - dlu->shift)) | (dlu->arg1 << dlu->shift)) & dlu->mask) | dlu->arg2;
    
    /* Rotate the bits back into their original positions, shifting the modified bit
     * to the appropriate position:
     *
     *         (result ROL shift)
     */
    result = (result << (16 - dlu->shift)) | (result >> dlu->shift);
    
    /* Set the result, doing a bitwise-NOT if necessary: */
    dlu->result = dlu->not ? ~result : result;
}

#endif /* __PHRI_DATA_H__ */

