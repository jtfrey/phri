/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file implements the concrete memory pseudo-subclasses.
 *
 */

#include "phri_mem.h"


void
phri_mem_bulk_copyin(
    const void      *M,
    phri_addr_t     addr,
    const void      *p,
    size_t          plen
)
{
    phri_mem_t      *MEM = (phri_mem_t*)M;
    phri_word_t     *P = (phri_word_t*)p;
    unsigned int    nwords = plen / sizeof(phri_word_t);
    unsigned int    extra_byte = plen % sizeof(phri_word_t);
    unsigned int    i;
    
    while ( nwords-- ) {
        MEM->bus->data = *P++;
        MEM->bus->addr = addr;
        phri_mem_wr(MEM);
        addr += 2;
    }
    if ( extra_byte ) {
        phri_byte_t *B = (phri_byte_t*)P;
        
        MEM->bus->addr = addr;
        phri_mem_rd(MEM);
        // Overwrite low byte:
        MEM->bus->data = (MEM->bus->data & 0xFF00) | *B;
        phri_mem_wr(MEM);
    }
}

//

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

static
void
phri_mem_64k_summary(
    void        *M,
    FILE        *stream
)
{
    phri_mem_64k_t  *M64k = (phri_mem_64k_t*)M;
    fprintf(stream,
        "phri_mem_64k@%p\n",
        M64k);
}

void
phri_mem_64k_init(
    phri_mem_64k_t  *M
)
{
    M->base.rd = phri_mem_64k_rd;
    M->base.wr = phri_mem_64k_wr;
    M->base.summary = phri_mem_64k_summary;
}

//
////
//

#define PHRI_MEM_N_4KIB_DECODE_ADDR(ADDR, AIDX, BIDX) \
        ((BIDX) = ((ADDR) & 0x0FFF), (AIDX) = (((ADDR) >> 12) & 0xF))

static
void
phri_mem_N_by_4KiB_rd(
    void            *M
)
{
    phri_mem_N_by_4KiB_t    *M4_KiB = (phri_mem_N_by_4KiB_t*)M;
    phri_addr_t             aidx, bidx;
    
    PHRI_MEM_N_4KIB_DECODE_ADDR(M4_KiB->base.bus->addr, aidx, bidx);
    if ( M4_KiB->arrays[aidx] ) {
        M4_KiB->base.bus->data = (M4_KiB->arrays[aidx]->chip[0][bidx] << 0) |
                                 (M4_KiB->arrays[aidx]->chip[1][bidx] << 1) |
                                 (M4_KiB->arrays[aidx]->chip[2][bidx] << 2) |
                                 (M4_KiB->arrays[aidx]->chip[3][bidx] << 3) |
                                 (M4_KiB->arrays[aidx]->chip[4][bidx] << 4) |
                                 (M4_KiB->arrays[aidx]->chip[5][bidx] << 5) |
                                 (M4_KiB->arrays[aidx]->chip[6][bidx] << 6) |
                                 (M4_KiB->arrays[aidx]->chip[7][bidx] << 7);
        bidx++;
        aidx += (bidx & 0xF000) ? 1 : 0;
        bidx &= 0x0FFF;
        M4_KiB->base.bus->data |=(M4_KiB->arrays[aidx]->chip[0][bidx] << 8) |
                                 (M4_KiB->arrays[aidx]->chip[1][bidx] << 9) |
                                 (M4_KiB->arrays[aidx]->chip[2][bidx] << 10) |
                                 (M4_KiB->arrays[aidx]->chip[3][bidx] << 11) |
                                 (M4_KiB->arrays[aidx]->chip[4][bidx] << 12) |
                                 (M4_KiB->arrays[aidx]->chip[5][bidx] << 13) |
                                 (M4_KiB->arrays[aidx]->chip[6][bidx] << 14) |
                                 (M4_KiB->arrays[aidx]->chip[7][bidx] << 15);
        
        if ( M4_KiB->base.bus->addr < M4_KiB->arrays[aidx]->raddr.lo )
            M4_KiB->arrays[aidx]->raddr.lo = M4_KiB->base.bus->addr;
        if ( M4_KiB->base.bus->addr + 1 > M4_KiB->arrays[aidx]->raddr.hi )
            M4_KiB->arrays[aidx]->raddr.hi = M4_KiB->base.bus->addr + 1;
    }
}

static
void
phri_mem_N_by_4KiB_wr(
    void            *M
)
{
    phri_mem_N_by_4KiB_t    *M4_KiB = (phri_mem_N_by_4KiB_t*)M;
    phri_addr_t             aidx, bidx;
    
    PHRI_MEM_N_4KIB_DECODE_ADDR(M4_KiB->base.bus->addr, aidx, bidx);
    if ( M4_KiB->arrays[aidx] ) {
        phri_word_t         latch = M4_KiB->base.bus->data;
        
        M4_KiB->arrays[aidx]->chip[0][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[1][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[2][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[3][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[4][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[5][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[6][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[7][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;
        bidx++;
        aidx += (bidx & 0xF000) ? 1 : 0;
        bidx &= 0x0FFF;
        M4_KiB->arrays[aidx]->chip[0][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[1][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[2][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[3][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[4][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[5][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[6][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off, latch >>= 1;  
        M4_KiB->arrays[aidx]->chip[7][bidx] = (latch & 0b1) ? kphri_bit_on : kphri_bit_off;
        
        if ( M4_KiB->base.bus->addr < M4_KiB->arrays[aidx]->waddr.lo )
            M4_KiB->arrays[aidx]->waddr.lo = M4_KiB->base.bus->addr;
        if ( M4_KiB->base.bus->addr + 1 > M4_KiB->arrays[aidx]->waddr.hi )
            M4_KiB->arrays[aidx]->waddr.hi = M4_KiB->base.bus->addr + 1;
    }
}

static
void
phri_mem_N_by_4KiB_summary(
    void        *M,
    FILE        *stream
)
{
    phri_mem_N_by_4KiB_t    *M4_KiB = (phri_mem_N_by_4KiB_t*)M;
    phri_addr_t             addr = 0x0000;
    unsigned int            i = 0;
    
    fprintf(stream,
        "phri_mem_N_by_4KiB@%p[N_4KiB=%hhu] {\n",
        M4_KiB, M4_KiB->N_4KiB);
    while ( i < 16 ) {
        if ( M4_KiB->arrays[i] ) {
            fprintf(stream, "    $%04hX:  phri_mem_ram4KiB@%p[", addr, M4_KiB->arrays[i]);
            if ( M4_KiB->arrays[i]->raddr.hi < M4_KiB->arrays[i]->raddr.lo ) {
                fprintf(stream, "R:UNUSED|");
            } else {
                fprintf(stream, "R:$%04hX…$%04hX|", M4_KiB->arrays[i]->raddr.lo, M4_KiB->arrays[i]->raddr.hi);
            }
            if ( M4_KiB->arrays[i]->waddr.hi < M4_KiB->arrays[i]->waddr.lo ) {
                fprintf(stream, "W:UNUSED]\n");
            } else {
                fprintf(stream, "W:$%04hX…$%04hX]\n", M4_KiB->arrays[i]->raddr.lo, M4_KiB->arrays[i]->raddr.hi);
            }
        } else {
            fprintf(stream, "    $%04hX:  <unmapped>\n", addr);
        }
        i++, addr += 0x1000;
    }
    fprintf(stream, "}\n");
}

void
phri_mem_N_by_4KiB_init(
    phri_mem_N_by_4KiB_t    *M,
    unsigned int            N_4KiB
)
{
    unsigned int            i = 16;
    
    M->N_4KiB = 0;
    while ( i-- > N_4KiB ) M->arrays[i] = NULL;
    while ( N_4KiB-- > 0 ) {
        M->arrays[N_4KiB] = (phri_mem_ram4KiB_t*)malloc(sizeof(phri_mem_ram4KiB_t));
        if ( ! M->arrays[N_4KiB] ) {
            fprintf(stderr, "ERROR:  unable to allocate 4 KiB memory array\n");
            exit(1);
        }
        M->arrays[i]->raddr.lo = 0xFFFF, M->arrays[i]->raddr.hi = 0x0000;
        M->arrays[i]->waddr.lo = 0xFFFF, M->arrays[i]->waddr.hi = 0x0000;
        M->N_4KiB++;
    }
    M->base.rd = phri_mem_N_by_4KiB_rd;
    M->base.wr = phri_mem_N_by_4KiB_wr;
    M->base.summary = phri_mem_N_by_4KiB_summary;
}

void
phri_mem_N_by_4KiB_init_with_bitmap(
    phri_mem_N_by_4KiB_t    *M,
    phri_word_t             are_mapped
)
{
    unsigned int            i = 0;

    M->N_4KiB = 0;
    while ( i < 16 ) {
        if ( are_mapped & 0b1 ) {
            M->arrays[i] = (phri_mem_ram4KiB_t*)malloc(sizeof(phri_mem_ram4KiB_t));
            if ( ! M->arrays[i] ) {
                fprintf(stderr, "ERROR:  unable to allocate 4 KiB memory array\n");
                exit(1);
            }
            M->arrays[i]->raddr.lo = 0xFFFF, M->arrays[i]->raddr.hi = 0x0000;
            M->arrays[i]->waddr.lo = 0xFFFF, M->arrays[i]->waddr.hi = 0x0000;
            M->N_4KiB++;
        } else {
            M->arrays[i] = NULL;
        }
        are_mapped >>= 1, i++;
    }
    M->base.rd = phri_mem_N_by_4KiB_rd;
    M->base.wr = phri_mem_N_by_4KiB_wr;
    M->base.summary = phri_mem_N_by_4KiB_summary;
}