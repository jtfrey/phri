/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file implements the concrete memory pseudo-subclasses.
 *
 */

#include "phri_mem.h"

static
void
phri_mem_64k_rd(
    void            *M
)
{
    phri_mem_64k_t  *M64k = (phri_mem_64k_t*)M;
    phri_addr_t     addr = M64k->base.bus->addr & 0xFFFF;
    M64k->base.bus->data = M64k->ram[addr] | ((M64k->ram[addr + 1]) << 8);
}

static
void
phri_mem_64k_wr(
    void            *M
)
{
    phri_mem_64k_t  *M64k = (phri_mem_64k_t*)M;
    phri_addr_t     addr = M64k->base.bus->addr & 0xFFFF;
    
    M64k->ram[addr++] = M64k->base.bus->data & 0x00FF;
    M64k->ram[addr] = (M64k->base.bus->data & 0xFF00) >> 8;
}

void
phri_mem_64k_init(
    phri_mem_64k_t  *M
)
{
    M->base.rd = phri_mem_64k_rd;
    M->base.wr = phri_mem_64k_wr;
}
