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
#include "phri_statusbits.h"

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
    kphri_instr_mask_alu_uop_mask   = 0b0111000000000000,   /*!< bitmask for the µop */
    kphri_instr_mask_alu_uop_shift  = 12,                   /*!< bit shift for the µop */
    kphri_instr_mask_alu_scond      = 0b0000100000000000,   /*!< bitmask for set condition flag */
    kphri_instr_mask_alu_src1t      = 0b0000010000000000,   /*!< bitmask for source 1 type */
    kphri_instr_mask_alu_src1i      = 0b0000001110000000,   /*!< bitmask for source index 1 */
    kphri_instr_mask_alu_src1c      = 0b0000001111111000,   /*!< bitmask for source const 1 */
    kphri_instr_mask_alu_src1i_shift = 7,                   /*!< bit shift for source index 1 */
    kphri_instr_mask_alu_src1c_shift = 3,                   /*!< bit shift for source const 1 */
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

/**
 * ALU µ-operations
 * The ALU only has four operations it performs:  binary addition and bitwise
 * and, or, and xor.
 */
enum {
    kphri_alu_uop_add   = 0,        /*!< ALU binary addition */
    kphri_alu_uop_and   = 1,        /*!< ALU bitwise and */
    kphri_alu_uop_or    = 2,        /*!< ALU bitwise or */
    kphri_alu_uop_xor   = 3         /*!< ALU bitwise xor */
};

/**
 * PHRI Arithmetic and Logic Unit
 * This structure encompasses the input and output state of the ALU.  Fields are set
 * with input values then the \ref phri_alu_exec() function is called to compute the
 * result.  The flags are updated in the process.
 */
typedef struct __attribute__((packed)) {
    phri_byte_t     uop;        /*!< IN:        µ-operation to perform */
    phri_word_t     arg1;       /*!< IN:        argument 1 will have mask1 and shift1 (in that order) applied to it */
    phri_word_t     arg2;       /*!< IN:        argument 2 will have shift2 and negation (in that order) applied to it */
    phri_bit_t      carry_in;   /*!< IN:        carry-in bit */
    phri_word_t     mask1;      /*!< IN:        significant bits of argument 1 */
    phri_byte_t     shift1;     /*!< IN:        shift to be applied to significant bits of argument 1; bit 7 indicates
                                                direction (1=shift right, 0=shift left) */
    phri_bit_t      not2;       /*!< IN:        1=bitwise NOT arg2 */
    phri_byte_t     shift2;     /*!< IN:        shift to be applied to argument 2; bit 7 indicates direction (1=shift
                                                right, 0=shift left) */
    phri_word_t     result;     /*!< OUT:       result of the operation */
    phri_byte_t     flags;      /*!< OUT:       status flags produced by the operation */
} phri_alu_t;

/**
 * Initialize an ALU structure to all defaults
 * Sets all fields of \p ALU to their defaults.
 */
#define phri_alu_init(ALU) \
            (cpu->alu.uop = 0, cpu->alu.arg1 = 0, cpu->alu.arg2 = 0, cpu->alu.carry_in = kphri_bit_off, \
             cpu->alu.mask1 = 0xFFFF, cpu->alu.shift1 = 0, cpu->alu.not2 = kphri_bit_off, cpu->alu.shift2 = 0)
/**
 * Initialize an ALU structure for binary addition
 * Sets all fields of \p ALU to add two integers.
 * @param ALU           the ALU to set
 * @param CARRY_IN      carry bit coming into the operation
 * @param ARG1          addend 1
 * @param ARG2          addend 2
 * @param SHIFT2        ignored for this operation, just pass 0
 */
#define phri_alu_init_add(ALU, CARRY_IN, ARG1, ARG2, SHIFT2) \
            (ALU).uop = kphri_alu_uop_add, (ALU).carry_in = (CARRY_IN) ? kphri_bit_on : kphri_bit_off, (ALU).arg1 = (ARG1), (ALU).arg2 = (ARG2), \
            (ALU).mask1 = 0xFFFF, (ALU).shift1 = 0, (ALU).not2 = kphri_bit_off, (ALU).shift2 = 0, (ALU).result = 0
/**
 * Initialize an ALU structure for binary subtraction
 * Sets all fields of \p ALU to subtract one integer from another.
 * @param ALU           the ALU to set
 * @param CARRY_IN      carry bit coming into the operation
 * @param ARG1          subtrahend 1
 * @param ARG2          subtrahend 2
 * @param SHIFT2        ignored for this operation, just pass 0
 */            
#define phri_alu_init_sub(ALU, CARRY_IN, ARG1, ARG2, SHIFT2) \
            (ALU).uop = kphri_alu_uop_add, (ALU).carry_in = (CARRY_IN) ? kphri_bit_on : kphri_bit_off, \
            (ALU).arg1 = (ARG1), (ALU).arg2 = (ARG2), (ALU).mask1 = 0xFFFF, (ALU).shift1 = 0, \
            (ALU).not2 = kphri_bit_on, (ALU).shift2 = 0, (ALU).result = 0
/**
 * Initialize an ALU structure for bitwise AND
 * Sets all fields of \p ALU to compute \p ARG1 & \p ARG2
 * @param ALU           the ALU to set
 * @param CARRY_IN      ignored for this operation, just pass 0
 * @param ARG1          value 1
 * @param ARG2          value 2
 * @param SHIFT2        ignored for this operation, just pass 0
 */ 
#define phri_alu_init_and(ALU, CARRY_IN, ARG1, ARG2, SHIFT2) \
            (ALU).uop = kphri_alu_uop_and, (ALU).flags = 0, (ALU).arg1 = (ARG1), (ALU).arg2 = (ARG2), \
            (ALU).mask1 = 0xFFFF, (ALU).shift1 = 0, (ALU).not2 = kphri_bit_off, (ALU).shift2 = 0, (ALU).result = 0
/**
 * Initialize an ALU structure for bitwise OR
 * Sets all fields of \p ALU to compute \p ARG1 | \p ARG2
 * @param ALU           the ALU to set
 * @param CARRY_IN      ignored for this operation, just pass 0
 * @param ARG1          value 1
 * @param ARG2          value 2
 * @param SHIFT2        ignored for this operation, just pass 0
 */ 
#define phri_alu_init_or(ALU, CARRY_IN, ARG1, ARG2, SHIFT2) \
            (ALU).uop = kphri_alu_uop_or, (ALU).flags = 0, (ALU).arg1 = (ARG1), (ALU).arg2 = (ARG2), \
            (ALU).mask1 = 0xFFFF, (ALU).shift1 = 0, (ALU).not2 = kphri_bit_off, (ALU).shift2 = 0, (ALU).result = 0
/**
 * Initialize an ALU structure for bitwise XOR
 * Sets all fields of \p ALU to compute \p ARG1 ^ \p ARG2
 * @param ALU           the ALU to set
 * @param CARRY_IN      ignored for this operation, just pass 0
 * @param ARG1          value 1
 * @param ARG2          value 2
 * @param SHIFT2        ignored for this operation, just pass 0
 */ 
#define phri_alu_init_xor(ALU, CARRY_IN, ARG1, ARG2, SHIFT2) \
            (ALU).uop = kphri_alu_uop_xor, (ALU).flags = 0, (ALU).arg1 = (ARG1), (ALU).arg2 = (ARG2), \
            (ALU).mask1 = 0xFFFF, (ALU).shift1 = 0, (ALU).not2 = kphri_bit_off, (ALU).shift2 = 0, (ALU).result = 0
/**
 * Initialize an ALU structure for bitwise left shift
 * Sets all fields of \p ALU to compute \p ARG2 << \p SHIFT2
 * @param ALU           the ALU to set
 * @param CARRY_IN      if set, all newly-introduced bits will be 1; otherwise, 0
 * @param ARG1          value 1
 * @param ARG2          value 2
 * @param SHIFT2        the left-shift distance
 */ 
#define phri_alu_init_shl(ALU, CARRY_IN, ARG1, ARG2, SHIFT2) \
            (ALU).uop = kphri_alu_uop_or, (ALU).flags = 0, (ALU).arg1 = (CARRY_IN) ? 0xFFFF : 0x0000, (ALU).arg2 = (ARG2), \
            (ALU).mask1 = 0xFFFF, (ALU).shift1 = 0b10000000 | (16 - (SHIFT2)), (ALU).not2 = kphri_bit_off, \
            (ALU).shift2 = (SHIFT2), (ALU).result = 0
/**
 * Initialize an ALU structure for bitwise right shift
 * Sets all fields of \p ALU to compute \p ARG2 >> \p SHIFT2
 * @param ALU           the ALU to set
 * @param CARRY_IN      if set, all newly-introduced bits will be 1; otherwise, 0
 * @param ARG1          value 1
 * @param ARG2          value 2
 * @param SHIFT2        the right-shift distance
 */ 
#define phri_alu_init_shr(ALU, CARRY_IN, ARG1, ARG2, SHIFT2) \
            (ALU).uop = kphri_alu_uop_or, (ALU).flags = 0, (ALU).arg1 = (CARRY_IN) ? 0xFFFF : 0x0000, (ALU).arg2 = (ARG2), \
            (ALU).mask1 = 0xFFFF, (ALU).shift1 = (16 - (SHIFT2)), (ALU).not2 = kphri_bit_off, \
            (ALU).shift2 = 0b10000000 | (SHIFT2), (ALU).result = 0

/**
 * Execute ALU operation
 * Given the operational parameters configured in the structure at \p alu, execute
 * the ALU operation, setting the flags and result fields in the structure.
 *
 * The ALU is fed by several inputs:  two 16-bit arguments, a 16-bit
 * AND mask applied to the first argument, a ±shift of each argument, a carry-in
 * bit, and an optional bitwise NOT of the second argument.  The computation looks
 * like:
 *
 *          result = ((arg1 & mask1) << shift1) [OP] (([~]arg2 << shift2) + [C])
 *
 * where a shift1/shift2 with bit 7 set changes the direction to >>; the distance
 * is invariably the shift1/shift2 & 0x7F.
 *
 * The [OP] can be +, &, |, ^.
 *
 * Subtraction is accomplished by setting the NOT arg2 flag on
 * entry and choosing the + operation.  The NOT arg2 flag also
 * inverts the carry-in/out bit when binary addition is selected.
 *
 * For SHL/SHR instructions, the ALU is actually setup to do
 * an OR.  The value to be shifted is placed in arg2;
 * the ±distance of the shift in shift2; arg1 receives 0x0000
 * which gets inverted if the SHCL/SHCR was used and the C flag
 * is set; and mask1 gets 0xFFFF shifted by ±16 minus the shift
 * that goes into shift2.
 *
 *      SHCL, C=1, ∆=3:  arg1=0xFFFF, mask1= 0xFFFF, shift1 = 0x80 | (16 - 3),
 *                       arg2=<value>, shift2 = 3
 *                       result = (0xFFFF & 0x0007) | (arg2 << 3)
 *                              = 0bAAAAAAAAAAAAA111
 *
 *      SHR, ∆=7:        arg1=0x0000, mask1= 0xFFFF, shift1 = (16 - 7),
 *                       arg2=<value>, shift2 = 0x80 | 7
 *                       result = (0x0000 & 0xFE00) | (arg2 >> 7)
 *                              = 0b0000000AAAAAAAAA
 */
static inline
void
phri_alu_exec(
    phri_alu_t      *alu
)
{
    phri_word_t     arg1 = ((alu->shift1 & 0b10000000) ?
                                (alu->arg1 & alu->mask1) >> (alu->shift1 & 0x7F) :
                                (alu->arg1 & alu->mask1) << (alu->shift1 & 0x7F));
    phri_word_t     arg2 = ((alu->shift2 & 0b10000000) ? 
                                (alu->arg2 >> (alu->shift2 & 0x7F)) : 
                                (alu->arg2 << (alu->shift2 & 0x7F)));
    uint32_t        carry = (uint32_t)alu->carry_in;
    phri_bit_t      overflow = kphri_bit_off, minus, zero = kphri_bit_on;
    phri_word_t     result = 0b0000000000000000;
    
    /* For negated arg2, flip the bits in arg2 and initial carry-in */
    if ( alu->not2 ) arg2 = ~arg2, carry = carry ^ kphri_bit_on;
    
    switch ( alu->uop ) {
        case kphri_alu_uop_add: {
            phri_word_t     bitmask = 0b0000000000000001;
            phri_word_t     A_xor_B;
            phri_word_t     bit;
            
            #define A               (arg1 & bitmask)
            #define B               (arg2 & bitmask)
            #define A_XOR_B         (A_xor_B = A ^ B)
            #define SUM             (result |= (bit = A_xor_B ^ carry))
            #define ZERO_CHECK      (zero = (zero & ! bit) ? kphri_bit_on : kphri_bit_off)
            #define CARRY_OUT       (carry = ((A & B) ^ ((A_xor_B) & carry)) << 1), (bitmask <<= 1)
            #define OVERFLOW_CHECK1 (overflow = carry ? kphri_bit_on : kphri_bit_off)
            #define MINUS_CHECK     (minus = bit ? kphri_bit_on : kphri_bit_off)
            #define OVERFLOW_CHECK2 (overflow = overflow ^ (carry ? kphri_bit_on : kphri_bit_off))
            
            /*  0 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  1 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  2 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  3 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  4 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  5 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  6 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  7 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  8 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /*  9 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /* 10 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /* 11 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /* 12 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /* 13 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT;
            /* 14 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT, OVERFLOW_CHECK1;
            /* 15 */ A_XOR_B, SUM, ZERO_CHECK, CARRY_OUT, MINUS_CHECK, OVERFLOW_CHECK2;
            
            #undef CARRY_OUT
            #undef ZERO_CHECK
            #undef SUM
            #undef A_XOR_B
            #undef B
            #undef A
            break;
        }
        

#define BIT0_CHECK      (carry = bit ? kphri_bit_on : kphri_bit_off)
#define BIT14_CHECK     (overflow = bit ? kphri_bit_on : kphri_bit_off)
#define BIT15_CHECK     (minus = bit ? kphri_bit_on : kphri_bit_off)

        case kphri_alu_uop_and: {
            phri_word_t     bitmask = 0b0000000000000001;
            phri_word_t     bit;
            
            #define BIT             (result |= (bit = (arg1 & bitmask) & (arg2 & bitmask))), (bitmask <<= 1)
            
            /*  0 */ BIT, BIT0_CHECK;
            /*  1 */ BIT;
            /*  2 */ BIT;
            /*  3 */ BIT;
            /*  4 */ BIT;
            /*  5 */ BIT;
            /*  6 */ BIT;
            /*  7 */ BIT;
            /*  8 */ BIT;
            /*  9 */ BIT;
            /* 10 */ BIT;
            /* 11 */ BIT;
            /* 12 */ BIT;
            /* 13 */ BIT;
            /* 14 */ BIT, BIT14_CHECK;
            /* 15 */ BIT, BIT15_CHECK;
            
            #undef BIT
            
            break;
        }
        case kphri_alu_uop_or: {
            phri_word_t     bitmask = 0b0000000000000001;
            phri_word_t     bit;
            
            #define BIT             (result |= (bit = (arg1 & bitmask) | (arg2 & bitmask))), (bitmask <<= 1)
            
            /*  0 */ BIT, BIT0_CHECK;
            /*  1 */ BIT;
            /*  2 */ BIT;
            /*  3 */ BIT;
            /*  4 */ BIT;
            /*  5 */ BIT;
            /*  6 */ BIT;
            /*  7 */ BIT;
            /*  8 */ BIT;
            /*  9 */ BIT;
            /* 10 */ BIT;
            /* 11 */ BIT;
            /* 12 */ BIT;
            /* 13 */ BIT;
            /* 14 */ BIT, BIT14_CHECK;
            /* 15 */ BIT, BIT15_CHECK;
            
            #undef BIT
            
            break;
        }
        case kphri_alu_uop_xor: {
            phri_word_t     bitmask = 0b0000000000000001;
            phri_word_t     bit;
            
            #define BIT             (result |= (bit = (arg1 & bitmask) ^ (arg2 & bitmask))), (bitmask <<= 1)
            
            /*  0 */ BIT, BIT0_CHECK;
            /*  1 */ BIT;
            /*  2 */ BIT;
            /*  3 */ BIT;
            /*  4 */ BIT;
            /*  5 */ BIT;
            /*  6 */ BIT;
            /*  7 */ BIT;
            /*  8 */ BIT;
            /*  9 */ BIT;
            /* 10 */ BIT;
            /* 11 */ BIT;
            /* 12 */ BIT;
            /* 13 */ BIT;
            /* 14 */ BIT, BIT14_CHECK;
            /* 15 */ BIT, BIT15_CHECK;
            
            #undef BIT
            
            break;
        }
        
#undef BIT0_CHECK
#undef BIT14_CHECK
#undef BIT15_CHECK

    }
            
    zero = result ? kphri_bit_off : kphri_bit_on;

    alu->result = result;
    alu->flags = (zero ? kphri_sb_z : 0) |
                 (overflow ? kphri_sb_v : 0) |
                 ((carry ? kphri_sb_c : 0) ^ (alu->not2 ? kphri_sb_c : 0)) |
                 (minus ? kphri_sb_m : 0);
}

#endif /* __PHRI_ALU_H__ */

