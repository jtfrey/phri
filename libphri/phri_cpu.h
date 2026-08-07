/**
 * PHRI - PHRI is a Hypothetical RISC ISA
 *
 * A fake CPU created just for fun.
 *
 * This file declares the CPU data structure, including the registers, bus
 * linkage, and cycle counter.  A variety of inline functions are defined
 * that assist in effecting execution of programs on the CPU.
 *
 */

#ifndef __PHRI_CPU_H__
#define __PHRI_CPU_H__

#include "phri_buslink.h"
#include "phri_math.h"

typedef struct __attribute__((packed)) {
    phri_buslink_t           *bus;
    union __attribute__((packed)) {
        phri_word_t          R[12];  /*!< the registers by index */
        struct {
            phri_word_t      Z;      /*!< 0:   register Z, "zero register" */
            phri_word_t      I;      /*!< 1:   register X, "index 0" */
            phri_word_t      J;      /*!< 2:   register Y, "index 1" */
            phri_word_t      A;      /*!< 3:   register A, "accumulator 0" */
            phri_word_t      B;      /*!< 4:   register B, "accumulator 1" */
            phri_word_t      M;      /*!< 5:   register M, "memory address" */
            phri_word_t      L;      /*!< 6:   register L, "link register" */
            phri_word_t      S;      /*!< 7:   register S, "stack pointer" */
            phri_word_t      PC;     /*!< 8:   program counter */
            phri_word_t      INSTR;  /*!< 9:   instruction register */
            phri_word_t      MOFF;   /*!< 10:  memory address offset register */
            union {
                phri_byte_t  MSEG;   /*!< 11L: memory segment register */
                struct {
#ifdef BITS_BIG_ENDIAN
                    phri_byte_t  PSEG : 4;
                    phri_byte_t  DSEG : 4;
#else
                    phri_byte_t  DSEG : 4;   /*!< 11L.L:  memory segment, data xfer */
                    phri_byte_t  PSEG : 4;   /*!< 11L.H:  memory segment, instruction xfer */
#endif
                };
            };
            phri_byte_t      F;      /*!< 11H: status register */
        };
    } registers;
    uint64_t                cycles;
} phri_cpu_t;



static inline
phri_cpu_t
phri_cpu_create(void)
{
    phri_cpu_t       C = { .registers.R = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                                           0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                                           0x0000 },
                          .bus = NULL,
                          .cycles = 0ULL };
    return C;
}

static inline
void
phri_cpu_summary(
    phri_cpu_t       *cpu
)
{
    char            status[9];
    
    status[0] = '-', status[1] = '-', status[2] = '-', status[3] = '-', 
    status[4] = (cpu->registers.F & kphri_sb_m) ? 'M' : 'm',
    status[5] = (cpu->registers.F & kphri_sb_c) ? 'C' : 'c',
    status[6] = (cpu->registers.F & kphri_sb_v) ? 'V' : 'v',
    status[7] = (cpu->registers.F & kphri_sb_z) ? 'Z' : 'z',
    status[8] = '\0';
    
    printf("[PHRI00]_____________________________________________________________\n"
           "|                                                                   |\n"
           "|  R0[Z] = 0x%1$04hX [%1$6hd|%1$-6hu]   R1[I] = 0x%2$04hX [%2$6hd|%2$-6hu]  |\n"
           "|  R2[J] = 0x%3$04hX [%3$6hd|%3$-6hu]   R3[A] = 0x%4$04hX [%4$6hd|%4$-6hu]  |\n"
           "|  R4[B] = 0x%5$04hX [%5$6hd|%5$-6hu]   R5[M] = 0x%6$04hX [%6$6hd|%6$-6hu]  |\n"
           "|  R6[L] = 0x%7$04hX [%7$6hd|%7$-6hu]  R7[SP] = 0x%8$04hX [%8$6hd|%8$-6hu]  |\n"
           "|  PC    = 0x%9$04hX [%9$6hd|%9$-6hu]       F =   0x%10$02hhX [   %11$s  ]  |\n"
           "|  INSTR = 0x%12$04hX [%12$6hd|%12$-6hu]     MEM = $[P:%13$1hhX|D:%14$1hhX]%15$04hX          |\n"
           "|                                                                   |\n"
           "|  CYCLE = 0x%16$016llX [%16$20llu]                |\n"
           "|                                                                   |\n"
           "| ---- B U S --------- B U S --------- B U S --------- B U S ------ |\n"
           "|                                                                   |\n"
           "|  DATA  = 0x%17$04hX [%17$6hd|%17$-6hu]    ADDR = $%18$05hX                  |\n"
           "|___________________________________________________________________|\n"
           "\n",
           cpu->registers.R[0], cpu->registers.R[1], cpu->registers.R[2], cpu->registers.R[3],
           cpu->registers.R[4], cpu->registers.R[5], cpu->registers.R[6], cpu->registers.R[7],
           cpu->registers.PC, cpu->registers.F, status,
           cpu->registers.INSTR, cpu->registers.PSEG, cpu->registers.DSEG, cpu->registers.MOFF,
           cpu->cycles,
           cpu->bus->data, cpu->bus->addr);
}

static inline
phri_word_t
phri_cpu_rdr(
    phri_cpu_t       *cpu,
    unsigned int    Ri
)
{
    return cpu->registers.R[Ri];
}

static inline
void
phri_cpu_wrr(
    phri_cpu_t       *cpu,
    unsigned int    Ri,
    phri_word_t      w
)
{
    if ( Ri ) cpu->registers.R[Ri] = w;
}

static inline
phri_word_t
phri_cpu_check_cc(
    phri_cpu_t       *cpu,
    phri_cc_t        cc
)
{
    return ((cpu->registers.F & (1 << (cc & 0b11))) ? 1 : 0) ^ ((cc & 0b100) >> 2);
}

static inline
phri_addr_t
phri_cpu_rdaddr(
    phri_cpu_t       *cpu,
    bool            is_instr
)
{
    return ((phri_addr_t)(is_instr ? cpu->registers.PSEG : cpu->registers.DSEG) << 16) | cpu->registers.MOFF;
}

static inline
void
phri_cpu_wraddr(
    phri_cpu_t       *cpu,
    bool            is_instr,
    phri_addr_t      addr
)
{
    cpu->registers.MOFF = addr & 0xFFFF;
    if ( is_instr )
        cpu->registers.PSEG = (addr >> 16) & 0xF;
    else
        cpu->registers.DSEG = (addr >> 16) & 0xF;
}

static inline
void
phri_cpu_bus_setaddr(
    phri_cpu_t       *cpu,
    bool            is_instr
)
{
    cpu->bus->addr = phri_cpu_rdaddr(cpu, is_instr);
}

static inline
void
phri_cpu_bus_rd(
    phri_cpu_t       *cpu
)
{
    cpu->bus->rd(cpu->bus);
    cpu->cycles++;
}

static inline
void
phri_cpu_bus_wr(
    phri_cpu_t       *cpu
)
{
    cpu->bus->wr(cpu->bus);
    cpu->cycles++;
}

static inline
void
phri_cpu_fetchdata(
    phri_cpu_t       *cpu
)
{
    // Set the address pins on the bus:
    phri_cpu_bus_setaddr(cpu, false);
    
    // [1 cycle]
    // Wait on memory to present data on the bus:
    phri_cpu_bus_rd(cpu);
}

static inline
void
phri_cpu_pc_adjust(
    phri_cpu_t       *cpu,
    phri_word_t      dPC
)
{
    phri_byte_t      S;
    
    cpu->registers.PC = phri_word_add(cpu->registers.PC, dPC, &S);
    if ( S & kphri_sb_c ) {
        cpu->cycles++;
        if ( dPC & 0b1000000000000000 )
            cpu->registers.PSEG--;
        else
            cpu->registers.PSEG++;
    }
}

static inline
void
phri_cpu_fetchinstr(
    phri_cpu_t       *cpu
)
{
    // Load the PC into the MOFF register; instructions MUST be
    // 16-bit aligned, otherwise instruction fetch could cross a
    // segment boundary and wrap!
    cpu->registers.MOFF = cpu->registers.PC;
    
    // Set the address pins on the bus:
    phri_cpu_bus_setaddr(cpu, true);
    
    // [1 or 2 cycles]
    // Increment the PC while we wait on memory to present data
    // on the bus; note that crossing a segment boundary adds
    // one extra cycle:
    phri_cpu_pc_adjust(cpu, 2);
    phri_cpu_bus_rd(cpu);
    
    // Move data from the bus into the INSTR register:
    cpu->registers.INSTR = cpu->bus->data;
}

static inline
void
phri_cpu_senddata(
    phri_cpu_t       *cpu
)
{
    // Set the address pins on the bus:
    phri_cpu_bus_setaddr(cpu, false);
    
    // [1 cycle]
    // Wait on memory to present data on the bus:
    phri_cpu_bus_wr(cpu);
}

void phri_cpu_execinstr(phri_cpu_t *cpu);

#endif /* __PHRI_CPU_H__ */

