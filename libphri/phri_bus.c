/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file implements the system bus and its linkage between the
 * CPU and memory subsystems.
 *
 */

#include "phri_bus.h"

static
void
phri_bus_rd(
    void    *buslink
)
{
    phri_bus_t   *bus = (phri_bus_t*)buslink;
    
    phri_mem_rd(bus->mem);
}

static
void
phri_bus_wr(
    void    *buslink
)
{
    phri_bus_t   *bus = (phri_bus_t*)buslink;
    
    phri_mem_wr(bus->mem);
}

phri_bus_t*
phri_bus_init(
    phri_bus_t   *bus,
    void        *cpu,
    void        *mem
)
{
    bus->lines.addr = 0x0000;
    bus->lines.data = 0x0000;
    bus->lines.rd = phri_bus_rd;
    bus->lines.wr = phri_bus_wr;
    
    bus->cpu = (phri_cpu_t*)cpu;
    *((phri_buslink_t**)cpu) = &bus->lines;
    bus->mem = (phri_mem_t*)mem;
    *((phri_buslink_t**)mem) = &bus->lines;
    return bus;
}


