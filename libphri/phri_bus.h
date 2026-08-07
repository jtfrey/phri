/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares the system bus — the agglomeration of the bus linkage,
 * CPU, and memory subsystems.
 *
 */

#ifndef __PHRI_BUS_H__
#define __PHRI_BUS_H__

#include "phri_buslink.h"
#include "phri_cpu.h"
#include "phri_mem.h"

/**
 * The system bus and device(s) linked to it
 * Data structure that groups together the system bus lines
 * (phri_buslink_t) with the CPU and memory devices linked
 * to it.
 */
typedef struct {
    phri_buslink_t      lines;  /*!< system bus lines, cpu and mem linked to this */
    phri_cpu_t          *cpu;   /*!< pointer to the CPU object for the system */
    phri_mem_t          *mem;   /*!< pointer to the memory object for the system */
} phri_bus_t;

/**
 * Initialize the bus
 * The \p bus object has its bus lines zeroed and read/write
 * callback functions set.  Pointers to the \p cpu and \p mem
 * are copied, and each have their buslink set to the buslink
 * field associated with \p bus (linking them to the bus).
 *
 * @param bus       the bus object to initialize
 * @param cpu       the phri_cpu_t object to link to \p bus
 * @param cpu       the phri_mem_t object to link to \p bus
 * @return          the bus object pointer
 */
phri_bus_t* phri_bus_init(phri_bus_t *bus, void *cpu, void *mem);

#endif /* __PHRI_BUS_H__ */
