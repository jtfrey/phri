/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares the condtion code and status bit values associated with
 * the ISA and F register.
 *
 */

#ifndef __PHRI_STATUSBITS_H__
#define __PHRI_STATUSBITS_H__

#include "phri.h"

/**
 * Condition codes (CC)
 * Values in this enumeration indicate a condition under which some
 * instructions will be executed or ignored.
 */
typedef enum __attribute__((packed)) {
    kphri_cc_eq     = 0,        /*!< Z flag set */
    kphri_cc_vs     = 1,        /*!< V flag set */
    kphri_cc_cs     = 2,        /*!< C flag set */
    kphri_cc_mi     = 3,        /*!< M flag set */
    //
    kphri_cc_not    = 4,        /*!< MSb negates the condition */
    //
    kphri_cc_ne     = 4,        /*!< Z flag clear */
    kphri_cc_vc     = 5,        /*!< V flag clear */
    kphri_cc_cc     = 6,        /*!< C flag clear */
    kphri_cc_pl     = 7,        /*!< M flag clear */
} phri_cc_t;

/**
 * Status bits in the F register
 * Bitmasks in this enumeration match a bit index
 * with a status flag.
 */
enum __attribute__((packed)) {
    kphri_sb_z      = 0b00000001,   /*!< The [Z]ero flag bit position */
    kphri_sb_v      = 0b00000010,   /*!< The o[V]erflow flag bit position */
    kphri_sb_c      = 0b00000100,   /*!< The [C]arry flag bit position */
    kphri_sb_m      = 0b00001000,   /*!< The [M]inus flag bit position */
    kphri_sb_mask   = 0b00001111    /*!< Mask of bits used in the F register */
};

#endif /* __PHRI_STATUSBITS_H__ */

