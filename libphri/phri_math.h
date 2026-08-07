/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file defines some basic ALU math functions.
 *
 */

#ifndef __PHRI_MATH_H__
#define __PHRI_MATH_H__

#include "phri_statusbits.h"

/**
 * Add two words, returning sum and setting status bits
 * The "adder" circuit that sums two 16-bit values.  If \p S is not
 * NULL then the status register flags in the byte it points to will
 * be updated to match the result.
 * @param w1    addend word 1
 * @param w2    addend word 2
 * @param S     pointer to a status byte to update, or NULL for no
 *              update
 * @return      the sum of w1 and w2
 */
static inline
phri_word_t
phri_word_add(
    phri_word_t w1,
    phri_word_t w2,
    phri_byte_t *S
)
{
    uint32_t    s = w1 + w2;
    uint16_t    m = (w1 ^ w2) & 0b1000000000000000;
    
    // m = 1 when the signs on w1, w2 differ
    if ( S ) {
        *S = (s ? 0 : kphri_sb_z) \
                | ((s & 0b1000000000000000) ? kphri_sb_m : 0) \
                | ((s & 0xFFFF0000) ? kphri_sb_c : 0) \
                | ((!m && ((w1 & 0b1000000000000000) ^ (s & 0b1000000000000000))) ? kphri_sb_v : 0);
    }
    return (s & 0xFFFF);
}

#endif /* __PHRI_MATH_H__ */

