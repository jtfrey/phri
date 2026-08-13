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
 * Type of a funtion that summarizes a memory system
 * The stdio \p stream is used to print information about the memory
 * system, \p M.
 */
typedef void (*phri_mem_summary_t)(void *M, FILE *stream);

/**
 * Abstract base structure for system memory
 * Every concrete memory architecture must extend this data structure
 * with its own fields.
 */
typedef struct {
    phri_buslink_t      *bus;       /*!< pointer to the shared buslink object */
    phri_mem_rd_t       rd;         /*!< function to read a word from memory */
    phri_mem_wr_t       wr;         /*!< function to write a word to memory */
    phri_mem_summary_t  summary;    /*!< function to summarize the memory system */
} phri_mem_t;

void phri_mem_bulk_copyin(const void *M, phri_addr_t addr, const void *p, size_t plen);

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
 * Trigger word write to memory
 * Function-like macro that invokes the phri_mem_t object's
 * write function.
 */
#define phri_mem_summary(_M_, _STRM_) (((phri_mem_t*)(_M_))->summary((_M_), (_STRM_)))

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
 * function pointers and all other fields.
 */
void phri_mem_64k_init(phri_mem_64k_t *M);


/**
 * A 4 KiB memory array
 * A set of (8) 4 Kbit chips is used to store 4 Kbytes.
 */
typedef struct __attribute__((packed)) {
    struct {
        phri_addr_t lo;
        phri_addr_t hi;
    } raddr;
    struct {
        phri_addr_t lo;
        phri_addr_t hi;
    } waddr;
    phri_bit_t      chip[8][4096];
} phri_mem_ram4KiB_t;

/**
 * Memory system with a fixed number of 4 KiB chips
 * The memory in this object is stored in modules consisting of
 * 8 x 4 Kbit RAM chips.  Bits 15…12 in the address act as a
 * select on the chip array to be accessed.
 */
typedef struct {
    phri_mem_t              base;
    phri_byte_t             N_4KiB;
    phri_mem_ram4KiB_t*     arrays[16];
} phri_mem_N_by_4KiB_t;

/**
 * Init 4KiB chip system memory object, first N
 * Given a pointer to a phri_mem_N_by_4KiB_t, initialize its read/write
 * function pointers and all other fields.
 *
 * The first \p N_4KiB banks of 4 KiB are mapped into the address space.
 */
void phri_mem_N_by_4KiB_init(phri_mem_N_by_4KiB_t *M, unsigned int N_4KiB);

/**
 * Init 4KiB chip system memory object, arbitrary
 * Given a pointer to a phri_mem_N_by_4KiB_t, initialize its read/write
 * function pointers and all other fields.
 *
 * The bitmask \p are_mapped indicates which of the 16 x 4 KiB chips are
 * to be present in the memory space.  The LSb represents the 4 KiB at
 * $0000, the MSb represents the 4 KiB at $F000.
 */
void phri_mem_N_by_4KiB_init_with_bitmap(phri_mem_N_by_4KiB_t *M, phri_word_t are_mapped);

#endif /* __PHRI_MEM_H__ */
