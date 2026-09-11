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
 * Bitmasks that are used in decoding ALU instructions.
 */
enum __attribute__((packed)) {
    kphri_alu_op_splmnt_mask        = 0b0111110000000000,   /*!< If these bits are set… */
    kphri_alu_op_splmnt_sr          = 0b0111010000000000,   /*!< …it's an SR instruction */
    kphri_alu_op_splmnt_sr_op_mask  = 0b0000001100000000,   /*!< 2a. The bits in these positions… */
    kphri_alu_op_splmnt_sr_op_shift = 8,                    /*!< …indicate the µop to be used */
    kphri_alu_op_splmnt_sr_const    = 0b0000000010001111,   /*!< Bitmask for the SR_ constant */

    kphri_alu_op_uop_mask           = 0b0111000000000000,   /*!< Bitmask for the ALU operation */
    kphri_alu_op_uop_shift          = 12,                   /*!< Shift for the ALU operation */
    
    kphri_alu_op_bitshiftmode_mask  = 0b0001100000000000,   /*!< Bitmask for the ALU bitshift mode */
    kphri_alu_op_bitshiftmode_shift = 11,                   /*!< Shift for the ALU bitshift mode */
    
    kphri_alu_op_splmnt             = 0b0111000000000000,   /*!< Supplementary instructions */
    kphri_alu_op_splmnt_cmpn_mask   = 0b0000100000000000,   /*!< 1. If this bit is set… */
    kphri_alu_op_splmnt_cmpn        = 0b0000100000000000,   /*!< …it's CMP/CMN */
    
    kphri_alu_op_splmnt_uop_mask    = 0b0000110000000000,   /*!< Bitmask (& and ^) for the ALU supplemental uop */
    kphri_alu_op_splmnt_uop_shift   = 10,                   /*!< Shift for the ALU supplemental uop */
    
    kphri_alu_op_class_mask         = 0b0110000000000000,   /*!< Bitmask for the instruction class */
    kphri_alu_op_class_shift        = 13,                   /*!< Shift for the instruction class */
    
    kphri_alu_op_add                = 0b0000000000000000,   /*!< ADD{S} */
    kphri_alu_op_sub                = 0b0001000000000000,   /*!< SUB{S} */
    kphri_alu_op_logical_shift      = 0b0010000000000000,   /*!< LSL/LSR */
    kphri_alu_op_arith_and_rotate   = 0b0011000000000000,   /*!< ASR/ROR */
    kphri_alu_op_and                = 0b0100000000000000,   /*!< AND{S} */
    kphri_alu_op_or                 = 0b0101000000000000,   /*!< OR{S} */
    kphri_alu_op_xor                = 0b0110000000000000,   /*!< XOR{S} */
    
    kphri_alu_op_scond              = 0b0000100000000000,   /*!< Set if the arithmetic/bitwise logic should set flags */
    kphri_alu_op_imm7               = 0b0000010000000000,   /*!< Set if a single 7-bit immediate operand is present */
    kphri_alu_op_imm7_mask          = 0b0000001111111000,   /*!< Bitmask for the 7-bit immediate operand */
    kphri_alu_op_imm7_shift         = 3,                    /*!< Shift of the 7-bit immediate operand */
    kphri_alu_op_rx_mask            = 0b0000001110000000,   /*!< Bitmask for the Rx register index */
    kphri_alu_op_rx_shift           = 7,                    /*!< Shift of the Rx register index */
    kphri_alu_op_imm3               = 0b0000000001000000,   /*!< Set if the second operand is a 3-bit constant */
    kphri_alu_op_imm3_mask          = 0b0000000000111000,   /*!< Bitmask for the 3-bit immediate operand or Ry register index */
    kphri_alu_op_imm3_shift         = 3,                    /*!< Shift of the 3-bit immediate operand or Ry register index */
    kphri_alu_op_rd_mask            = 0b0000000000000111    /*!< Bitmask for the Rd register index */
};



enum __attribute__((packed)) {
    kphri_alu_path_arith       = 0b00,
    kphri_alu_path_bitshift    = 0b01,
    kphri_alu_path_logic       = 0b10,
    kphri_alu_path_splmnt      = 0b11
};

enum __attribute__((packed)) {
    kphri_alu_uop_arith_add     = 0b00,
    kphri_alu_uop_arith_sub     = 0b01
};

enum __attribute__((packed)) {
    kphri_alu_uop_logic_and     = 0b00,
    kphri_alu_uop_logic_or      = 0b01,
    kphri_alu_uop_logic_xor     = 0b10
};

enum __attribute__((packed)) {
    kphri_alu_uop_bitshift_lsl  = 0b00,
    kphri_alu_uop_bitshift_lsr  = 0b01,
    kphri_alu_uop_bitshift_asr  = 0b10,
    kphri_alu_uop_bitshift_ror  = 0b11
};

typedef struct __attribute__((packed)) {
    struct {
        phri_byte_t     path : 2;
        phri_byte_t     uop : 2;
        phri_byte_t     carry_in : 1;
    };
    phri_word_t         arg1;
    phri_word_t         arg2;
    phri_word_t         result;
    phri_byte_t         flags;
    phri_byte_t         flags_mask;
} phri_alu_t;

#define phri_alu_init(ALU)  \
    ((ALU).path = 0, (ALU).uop = 0, (ALU).carry_in = 0, (ALU).arg1 = 0, \
     (ALU).arg2 = 0, (ALU).result = 0, (ALU).flags = 0)

static inline
void
phri_alu_exec(
    phri_alu_t  *alu
)
{
    phri_word_t result, arg1, arg2;
    phri_bit_t  minus, overflow, carry, zero;
    
    arg1 = alu->arg1, arg2 = alu->arg2, carry = alu->carry_in, result = 0x0000;
    minus = overflow = zero = kphri_bit_off;
    alu->flags_mask = kphri_sb_z | kphri_sb_c | kphri_sb_v | kphri_sb_m;
    
    switch ( alu->path ) {
        case kphri_alu_path_arith:
        case kphri_alu_path_splmnt: {
#define A               (arg1 & bitmask)
#define B               (arg2 & bitmask)
#define A_XOR_B         (A_xor_B = A ^ B)
#define SUM             (result |= (bit = A_xor_B ^ carry_out))
#define ZERO_CHECK      (zero = (zero & ! bit) ? kphri_bit_on : kphri_bit_off)
#define CARRY_OUT       (carry_out = ((A & B) ^ ((A_xor_B) & carry_out)) << 1), (bitmask <<= 1)
#define OVERFLOW_CHECK1 (overflow = carry_out ? kphri_bit_on : kphri_bit_off)
#define MINUS_CHECK     (minus = bit ? kphri_bit_on : kphri_bit_off)
#define OVERFLOW_CHECK2 (overflow = overflow ^ (carry_out ? kphri_bit_on : kphri_bit_off))
            phri_word_t bitmask = 0b0000000000000001, A_xor_B, bit;
            phri_bit_t  inverted = alu->uop;    /* add = 0b0, sub=0b1 */
            uint32_t    carry_out;
            
            zero = kphri_bit_on;
            carry_out = carry;
            if ( inverted ) arg2 = ~arg2, carry_out ^= kphri_bit_on;
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
            carry = (carry_out ? kphri_bit_on : kphri_bit_off) ^ inverted;
#undef OVERFLOW_CHECK2
#undef MINUS_CHECK
#undef OVERFLOW_CHECK1
#undef CARRY_OUT
#undef ZERO_CHECK
#undef SUM
#undef A_XOR_B
#undef B
#undef A
            
            break;
        }
        
        case kphri_alu_path_bitshift: {
            phri_byte_t     shift = alu->arg2;
            phri_word_t     bit_out;
            
            alu->flags_mask = kphri_sb_c;
            switch ( alu->uop ) {
            
                case kphri_alu_uop_bitshift_lsl: {
#define SHIFT (((bit_out = arg1 & 0b1000000000000000) ? kphri_bit_on : kphri_bit_off), (arg1 <<= 1))
#define CARRY ((arg1 |= carry), (carry = kphri_bit_off))
                    while ( shift-- ) SHIFT, CARRY;
#undef CARRY
#undef SHIFT
                    break;
                }
                case kphri_alu_uop_bitshift_lsr: {
#define SHIFT (((bit_out = arg1 & 0b1) ? kphri_bit_on : kphri_bit_off), (arg1 >>= 1))
#define CARRY ((arg1 |= (carry << 15)), (carry = kphri_bit_off))
                    while ( shift-- ) SHIFT, CARRY;
#undef CARRY
#undef SHIFT
                    break;
                }
                case kphri_alu_uop_bitshift_asr: {
#define SHIFT (((bit_out = arg1 & 0b1) ? kphri_bit_on : kphri_bit_off), (arg1 >>= 1))
#define CARRY (arg1 |= (carry << 15))
                    carry = (arg1 & 0b1000000000000000) ? kphri_bit_on : kphri_bit_off;
                    while ( shift-- ) SHIFT, CARRY;
#undef CARRY
#undef SHIFT
                    break;
                }
                case kphri_alu_uop_bitshift_ror: {
#define SHIFT (((bit_out = arg1 & 0b1) ? kphri_bit_on : kphri_bit_off), (arg1 >>= 1))
#define CARRY (arg1 |= (bit_out << 15))
                    while ( shift-- ) SHIFT, CARRY;
#undef CARRY
#undef SHIFT
                    break;
                }
            }
            result = arg1;
            carry = bit_out ? kphri_bit_on : kphri_bit_off;
            break;
        }
        
        default: {
#define ZERO_CHECK      (zero = (zero & ! bit) ? kphri_bit_on : kphri_bit_off)
#define BIT0_CHECK      (carry = bit ? kphri_bit_on : kphri_bit_off)
#define BIT14_CHECK     (overflow = bit ? kphri_bit_on : kphri_bit_off)
#define BIT15_CHECK     (minus = bit ? kphri_bit_on : kphri_bit_off)
            phri_word_t bitmask = 0b0000000000000001,
                        bit;
            
            zero = kphri_bit_on;
            switch ( alu->uop ) {
                case kphri_alu_uop_logic_and:
#define BIT (result |= (bit = ((arg1 & bitmask) & (arg2 & bitmask))), (bitmask <<= 1), ZERO_CHECK)
                    /*    0 */ BIT, BIT0_CHECK;
                    /* 1…13 */ BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT,
                               BIT, BIT, BIT;
                    /*   14 */ BIT, BIT14_CHECK;
                    /*   15 */ BIT, BIT15_CHECK;
#undef BIT
                    break;
                    
                case kphri_alu_uop_logic_or:
#define BIT (result |= (bit = ((arg1 & bitmask) | (arg2 & bitmask))), (bitmask <<= 1), ZERO_CHECK)
                    /*    0 */ BIT, BIT0_CHECK;
                    /* 1…13 */ BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT,
                               BIT, BIT, BIT;
                    /*   14 */ BIT, BIT14_CHECK;
                    /*   15 */ BIT, BIT15_CHECK;
#undef BIT
                    break;
                    
                case kphri_alu_uop_logic_xor:
#define BIT (result |= (bit = ((arg1 & bitmask) ^ (arg2 & bitmask))), (bitmask <<= 1), ZERO_CHECK)
                    /*    0 */ BIT, BIT0_CHECK;
                    /* 1…13 */ BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT, BIT,
                               BIT, BIT, BIT;
                    /*   14 */ BIT, BIT14_CHECK;
                    /*   15 */ BIT, BIT15_CHECK;
#undef BIT
                    break;
            }
#undef BIT15_CHECK
#undef BIT14_CHECK
#undef BIT0_CHECK
            break;
        }
    }
    alu->result = result;
    alu->flags = (zero ? kphri_sb_z : 0) |
                 (carry ? kphri_sb_c : 0) |
                 (overflow ? kphri_sb_v : 0) |
                 (minus ? kphri_sb_m : 0);
}

#endif /* __PHRI_ALU_H__ */

