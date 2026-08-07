/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares the data structure and callback functions that
 * implement the common address/data lines that makeup the system
 * bus.
 *
 */

#ifndef __PHRI_BUSLINK_H__
#define __PHRI_BUSLINK_H__

#include "phri.h"

/**
 * Type of a function that services bus read data.
 */
typedef void (*phri_buslink_rd_t)(void *buslink);

/**
 * Type of a function that services bus write data.
 */
typedef void (*phri_buslink_wr_t)(void *buslink);

/**
 * System bus linkage
 * A buslink is a singular data structure shared by the CPU and
 * system memory objects used to mimic the address and data lines.
 */
typedef struct {
    phri_addr_t         addr;   /*!< address lines */
    phri_word_t         data;   /*!< data lines */
    phri_buslink_rd_t   rd;     /*!< pointer to the function that requests read of \p addr
                                     into \p data */
    phri_buslink_wr_t   wr;     /*!< pointer to the function that requests write of \p data
                                     to \p addr */
} phri_buslink_t;

#endif /* __PHRI_BUSLINK_H__ */

