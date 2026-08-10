![PHRI Logo](docs/phri-logo.png)

For a fun side project, what if I decided on a 16-bit data and address bus feeding a RISC-like ISA that uses a 16-bit fixed instruction size?  I'd come up with:

**PHRI:  PHRI is a Hypothetical RISC ISA**

That is pronounced like "fry" — you know, my last name.  

The [processor data sheet](docs/PHRI-Datasheet.md) contains an extensive description of the ISA.  Some of those initial design ideas were relaxed:  the address bus eventually got another 4 bits, making a whole 1 MiB of memory addressable.  Granted, in a rather *creative* way.

Instructions are constructed with decode logic starting at the MSb and moving downwards:  if bit 15 is set the instruction targets the ALU and the remaining 15 bits are available, otherwise bit 14 selects data movement versus branching classes and there are 14 bits available.  The ALU class uses bits 14…12 as an opcode field (8 operators) and bit 11 selects whether or not to enable carry-in and status flag updates after the operation.  For 7 of the operators (0…6) the remaining 11 bits encode three addressing modes:

```
 F E D C B A 9 8 7 6 5 4 3 2 1 0
 --------------------------------
|1|  OP |S|0|  X  |0|  Y  |  D  |   <OP>{S}     Rd, Rx, Ry
|1|  OP |S|0|  X  |1| IMM3|  D  |   <OP>{S}     Rd, Rx, #<IMM3>
|1|  OP |S|1|    IMM7     |  D  |   <OP>{S}     Rd, #<IMM7>
```

Operator 7 signals a sub-division of the lower 11 bits into additional classes: compare/negated-compare with a 7-bit immediate value and operations to affect the status register (e.g set/clear/toggle bits).

Data movement instructions lead with two zeroes, followed by bit 13's indicating memory-oriented (1) or CPU oriented (0) instructions.  For memory-oriented instructions, bit 12 denotes directionality (1 = store, 0=load).

```
 F E D C B A 9 8 7 6 5 4 3 2 1 0
 --------------------------------
|0 0|0|  …0…    …1…   OPERANDS   |  MOV et. al
|0 0|1|0|   …0…  …1…  OPERANDS   |  LDR et. al
|0 0|1|1|   …0…  …1…  OPERANDS   |  STO et. al
```

Finally, the branching instructions lead with a zero and one, and bit 13 indicates if the branch, when taken, should set a linking register (1) or not (0).  Linking moves the PC to a register so that the branch can "return" to that point — meaning a branch with linking is effectively a subroutine call.  Branches can be conditional (based on a condition code) or unconditional.  Branches can also be relative (PC += OPERAND) or absolute (PC = OPERAND), but absolute can only use the value of a register as its operand (since we can't embed a 16-bit immediate value in the instruction).

```
 F E D C B A 9 8 7 6 5 4 3 2 1 0
 --------------------------------
|0 1|0|0|0 0 0 0 0 0 0 0|R|  X  |       B{R}        Rx
|0 1|1|0|0 0 0 0 0|  Y  |R|  X  |       B{R}L       Rx, Ry
|0 1|0|1| CC  |0 0 0 0 0|R|  X  |       B{R}.<CC>   Rx
|0 1|1|1| CC  |0 0|  Y  |R|  X  |       B{R}L.<CC>  Rx, Ry
|0 1|0|0|1|        IMM11        |       BR          #<IMM11>
|0 1|1|0|1|        IMM11        |       BRL         #<IMM11>
|0 1|0|1| CC  |1|     IMM8      |       BR.<CC>     #<IMM8>
|0 1|1|1| CC  |1|     IMM8      |       BRL.<CC>    #<IMM8>
```

Since the ISA is homogeneous 16-bit instructions, the IMM11 and IMM8 values are **shifted left one bit** before being added to the PC; code **must** be 16-bit aligned.  For the immediate operand linking branches, register `R6` is implied (hence its conventional name of `L`).


# First pass issues

After a first pass at the ISA and implementing it, there are a few issues.  First and foremost, there's a lot of instruction overlap.  I went back and reread a book on ARM64 and it dawned on me that the baseline `MOV` instructions in PHRI are redundant:

| MOV form      | Alternative       |                                      |
| :------------ | :---------------- | :----------------------------------- |
| `MOV  Rd, Rx` | `OR   Rd, Rx, R0` | Rd <= Rx | 0x0000 = Rx               |
| `MVN  Rd, Rx` | `XOR  Rd, Rx, R0` | Rd <= Rx ^ 0xFFFF = ~Rx              |

The instructions working with the PC and DSEG registers were allocated in the middle of the block, leaving a large gap of unused opcodes above the `MOV Rd, Rx` that only used the 6 lowest bits in the word.  The 8-bit immediate instructions were naturally present at the end of the series, since they used the greatest number of bits.  The order didn't work, and there was no need for register-to-register movement since ALU logic instructions using R0 accomplish the same.

The first alteration was to move the special-case instructions first:  for the PC and DSEG instructions, one of the registers is always implied and there are no options so fewer operand bits are needed.  Instructions use from 5 to 6 bits.

The next-largest instruction loads a 4-bit immediate value (possibly shifted 0, 4, 8, or 12 bit positions) into a register — sign-extending as appropriate.  The shifts are not arbitrary, they are nibble-aligned:  0, 4, 8, 12, using just 2 bits in the instruction.  That's a 3-bit register index, 4-bit immediate value, 2-bit shift, and a single decode bit: 10 bits.

The jump from 6 to 10 bits leaves some room for other instructions in between.
