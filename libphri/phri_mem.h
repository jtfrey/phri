/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares an abstract base data structure for building memory
 * subsystems for use with the ISA.
 *
 */

#ifndef __PHRI_MEM_H__
#define __PHRI_MEM_H__

#include "phri_buslink.h"

/**
 * Type of a function that reads a word from memory onto the bus
 * The \p addr in the system bus is matched to an address associated
 * with the memory subsystem, and the word at that address is placed
 * on the \p data lines of the system bus.
 */
typedef void (*phri_mem_rd_t)(void *M);

/**
 * Type of a function that writes a word from the bus to memory
 * The \p addr in the system bus is matched to an address associated
 * with the memory subsystem, and the word at that address is placed
 * on the \p data lines of the system bus.
 */
typedef void (*phri_mem_wr_t)(void *M);

/**
 * Abstract base structure for system memory
 * Every concrete memory architecture must extend this data structure
 * with its own fields.
 */
typedef struct {
    phri_buslink_t      *bus;   /*!< pointer to the shared buslink object */
    phri_mem_rd_t       rd;     /*!< function to read a word from memory */
    phri_mem_wr_t       wr;     /*!< function to write a word to memory */
} phri_mem_t;

/**
 * Trigger word read from memory
 * Function-like macro that invokes the phri_mem_t object's
 * read function.
 */
#define phri_mem_rd(_M_) (((phri_mem_t*)(_M_))->rd((_M_)))

/**
 * Trigger word write to memory
 * Function-like macro that invokes the phri_mem_t object's
 * write function.
 */
#define phri_mem_wr(_M_) (((phri_mem_t*)(_M_))->wr((_M_)))

/**
 * 64 KiB system memory object
 * A concreate implementation of a phri_mem_t that includes
 * a static array of 64 KiB.
 *
 * Only address lines 0 through 15 are active.  So the
 * address $1FEED effectively maps to $0FEED.
 */
typedef struct {
    phri_mem_t          base;
    phri_byte_t         ram[0x10000];
} phri_mem_64k_t;

/**
 * Init 64 KiB system memory object
 * Given a pointer to a phri_mem_64k_t, initialize its read/write
 * function pointers and all over fields.
 */
void phri_mem_64k_init(phri_mem_64k_t *M);

#endif /* __PHRI_MEM_H__ */
