# PHRI Data Sheet

The PHRI (PHRI is a Hypothetical RISC ISA) is an invented 16-bit ISA.  It features a uniform 16-bit data bus and 20-bit address bus; memory is addressed in *segments* of 64 KiB.


## Register set

The PHRI possesses twelve 16-bit registers.

There are eight general-purpose registers labelled R0 through R7; R0 is a read-only register that always contains a value of zero.  The remaining registers are usable in any capacity across the instructions, but the assembly mnemonics include aliases for each that convey a possible purpose:

| Index | Alias | Implied usage                                          |
| :---- | :---- | :----------------------------------------------------- |
| `R0`  | `Z`   | The zero register; read-only, always contains `0x0000` |  
| `R1`  | `I`   | Integer indexes, bitmasks, etc.                        |
| `R2`  | `J`   | Integer indexes, bitmasks, etc.                        |
| `R3`  | `A`   | Accumulator, used for arithmetic/logic computation     |
| `R4`  | `B`   | Accumulator, used for arithmetic/logic computation     |
| `R5`  | `M`   | Memory addresses, for load/store of data               |
| `R6`  | `L`   | Link register, used for subroutine call return address |
| `R7`  | `SP`  | Stack pointer                                          |

This is merely a suggested convention:  user software can task each of the registers (except `R0`/`Z` and in some cases `R6`/`L`) to whatever purpose is useful.

A program counter (`PC`) register holds the 16-bit address of the instruction that will be fetched next.  At boot, the `PC` is initialized to `$00000` — program code is expected to originate at this address.  The leading nibble comes from the *program counter segment* or `PSEG`, which comes from the high nibble of the `MSEG` register.  When the program counter is updated using a relative offset — as well as instruction fetch — the `PSEG` will wrap to the subsequent/previous segment automatically when crossing a 64 KiB boundary.

Instructions that load/store data must also work with memory addresses.  A separate *data segment* or `DSEG` comes from the low nibble of the `MSEG` register.  Like the `PSEG`, the `DSEG` is initialized to zero.  It can be directly modified by user code to alter the target address of subsequent load/store instructions.  If there were a single segment value, user code would effectively switch itself out of scope when altering a singular segment register; keeping the `PSEG` and `DSEG` separate keeps the `PC` functional.  The programmer should bear in mind, though, that the `DSEG` is **not** automatically adjusted:  user code must handle the `DSEG` adjustments as necessary to access data held in other segments.  This has a direct effect on PHRI stacks:  a stack can never exceed 64 KiB in size and it is up to the user code to ensure the correct `DSEG` is selected when pushing/popping data.

The `F` register is an 8-bit set of condition (status) flags.  Only five bits are currently utilized:

- `H`: the halt flag, set to indicate the processor should stop executing code
- `M`: the minus flag, set to indicate a value has bit 15 set
- `C`: the carry flag, set to indicate an arithmetic operation produced a carry out of bit 15
    - this flag is also used to indicate the bit value to initially shift into a register on bit shift operations (`LSL`, `LSR`)
    - this flag is also set to indicate the captured bit index value on bit shift operations
- `V`: the overflow flag, set to indicate a signed arithmetic operation produced a carry out of bit 14
- `Z`: the zero flag, set to indicate a value was zero, clear if non-zero

The `MSEG` and `F` registers collectively form the 16-bit Segment and Status Register, `SSR`.  The `MSEG` is its LSB and the `SSR` is its MSB.  Instructions are present that allow for transfer and manipulation of the `SSR` as a whole.

The instruction register (`INSTR`) latches the instruction word when one is fetched.

| 16-bit register | Additional info                   |
| :-------------- | :-------------------------------- |
| `R0` (`Z`)      |                                   |
| `R1` (`I`)      |                                   |
| `R2` (`J`)      |                                   |
| `R3` (`A`)      |                                   |
| `R4` (`B`)      |                                   |
| `R5` (`M`)      |                                   |
| `R6` (`L`)      |                                   |
| `R7` (`SP`)     |                                   |
| `PC`            | Program counter                   |
| `INSTR`         | Last-fetched instruction          |
| `MOFF`          | Low word of address on memory bus |
| `SSR`           |    \|   F    :   MSEG  \|         |
|                 | 15 \|        :PSEG:DSEG\| 0       |
|                 |    \|▪︎▪︎▪︎▪︎▪︎▪︎▪︎▪︎:▪︎▪︎▪︎▪︎:▪︎▪︎▪︎▪︎\|         |

### Condition codes

Some instructions include *condition code* suffixes that determine whether the instruction is executed or ignored; this obviates the need for explicit conditional instructions and branches.  The condition codes are tests of the processor condition flags:

| Mnemonic | Value | Description           |
| :------- | :---: | :-------------------- |
| `EQ`     |  `0`  | The `Z` flag is set   |
| `VS`     |  `1`  | The `V` flag is set   |
| `CS`     |  `2`  | The `C` flag is set   |
| `MI`     |  `3`  | The `M` flag is set   |
| `NE`     |  `4`  | The `Z` flag is clear |
| `VC`     |  `5`  | The `V` flag is clear |
| `CC`     |  `6`  | The `C` flag is clear |
| `PL`     |  `7`  | The `M` flag is clear |

The values of the latter four codes negate the first four, and differ only in bit 3's being set, e.g. `MI` is `0b011` and `PL` is `0b111`.


## Instruction set

Instructions consist of a 16-bit word with no additional operand bytes.  Any operands are embedded in the instruction word.

Instructions are classifed into three categories:

- Arithmetic and logic unit
- Data movement
- Branching

All instructions are assigned a mnemonic for the sake of assembly programming.

It should be noted that there are no explicit no-operation (NOP) instructions present; there are a number of instructions that effectively function as such, though:

| Mnemonic                | Notes
| :---------------------- | :--------------------------------- |
| `<OPCODE> R0, R0, R0`   | OPCODE = ADD, SUB, AND, OR, XOR    |
| `SR       AND, #0b1111` | Status bits retain existing values |


### Arithmetic and logic

All instructions with bit 15 set are handled by the ALU.

This category includes instructions to perform two's-complement addition/subtraction, shift and rotate bits, and perform standard bitwise logic like AND, OR, and XOR.  This category also includes instructions to alter the condition flags in the `F` register (e.g. clear carry).  For operations with an `S` mode suffix, the processor's status flags will be updated after the ALU operation; bit shift/rotate instructions always update the [C]arry bit (and no others).  The ADD and SUB instructions ignore the [C]arry bit unless the `C` mode suffix is included.

| Mnemonic                                  | Bit pattern        | Description                                       |
| :---------------------------------------- | :----------------- | :------------------------------------------------ |
| `ADD{C}    Rd, Rx, Ry`                    | `1000C0XXX0YYYDDD` | Rd <= Rx + Ry, {C}=include carry              [1] |
| `ADD{C}    Rd, Rx, #<IMM3>`               | `1000C0XXX1cccDDD` | Rd <= Rx + IMM3, {C}=include carry            [1] |
| `ADD{C}    Rd, #<IMM7>`                   | `1000C1cccccccDDD` | Rd <= Rd + IMM7, {C}=include carry            [1] |
| `SUB{C}    Rd, Rx, Ry`                    | `1001C0XXX0YYYDDD` | Rd <= Rx - Ry, {C}=include carry              [1] |
| `SUB{C}    Rd, Rx, #<IMM3>`               | `1001C0XXX1cccDDD` | Rd <= Rx - IMM3, {C}=include carry            [1] |
| `SUB{C}    Rd, #<IMM7>`                   | `1001C1cccccccDDD` | Rd <= Rd - IMM7, {C}=include carry            [1] |
| `LSL       Rd, Rx, Ry`                    | `101000XXX0YYYDDD` | Rd <= Rx << (Ry & 0xF)                        [2] |
| `LSR       Rd, Rx, Ry`                    | `101010XXX0YYYDDD` | Rd <= Rx >> (Ry & 0xF)                        [2] |
| `ASR       Rd, Rx, Ry`                    | `101100XXX0YYYDDD` | Rd <= Rx ASR (Ry & 0xF)                       [2] |
| `ROR       Rd, Rx, Ry`                    | `101110XXX0YYYDDD` | Rd <= Rx ROR (Ry & 0xF)                       [2] |
| `LSL       Rd, Rx, #<IMM3>`               | `101000XXX1CCCDDD` | Rd <= Rx << (2 * IMM3)                        [2] |
| `LSL       Rd, Rx, #1`                    | `101000XXX1000DDD` | Rd <= Rx << 1                               [2,3] |
| `LSR       Rd, Rx, #<IMM3>`               | `101010XXX1CCCDDD` | Rd <= Rx >> (2 * IMM3)                        [2] |
| `LSR       Rd, Rx, #1`                    | `101010XXX1000DDD` | Rd <= Rx >> 1                               [2,3] |
| `ASR       Rd, Rx, #<IMM3>`               | `101100XXX1CCCDDD` | Rd <= Rx ASR (2 * IMM3)                       [2] |
| `ASR       Rd, Rx, #1`                    | `101100XXX1000DDD` | Rd <= Rx ASR 1                              [2,3] |
| `ROR       Rd, Rx, #<IMM3>`               | `101110XXX1CCCDDD` | Rd <= Rx ROR (2 * IMM3)                       [2] |
| `ROR       Rd, Rx, #1`                    | `101110XXX1000DDD` | Rd <= Rx ROR 1                              [2,3] |
| `LSL       Rd, #<IMM7>`                   | `101001CCCCCCCDDD` | Rd <= Rd << (IMM7 & 0xF)                      [2] |
| `LSR       Rd, #<IMM7>`                   | `101011CCCCCCCDDD` | Rd <= Rd >> (IMM7 & 0xF)                      [2] |
| `ASR       Rd, #<IMM7>`                   | `101101CCCCCCCDDD` | Rd <= Rd ASR (IMM7 & 0xF)                     [2] |
| `ROR       Rd, #<IMM7>`                   | `101111CCCCCCCDDD` | Rd <= Rd ROR (IMM7 & 0xF)                     [2] |
| `AND{S}    Rd, Rx, Ry`                    | `1100S0XXX0YYYDDD` | Rd <= Rx & Ry, {S}=set status bits          [4,5] |
| `AND{S}    Rd, Rx, #<IMM3>`               | `1100S0XXX1CCCDDD` | Rd <= Rx & IMM3, {S}=set status bits          [4] |
| `AND{S}    Rd, #<IMM7>`                   | `1100S1CCCCCCCDDD` | Rd <= Rd & IMM7, {S}=set status bits          [4] |
| `OR{S}     Rd, Rx, Ry`                    | `1101S0XXX0YYYDDD` | Rd <= Rx \| Ry, {S}=set status bits           [4] |
| `OR{S}     Rd, Rx, #<IMM3>`               | `1101S0XXX1CCCDDD` | Rd <= Rx \| IMM3, {S}=set status bits         [4] |
| `OR{S}     Rd, #<IMM7>`                   | `1101S1CCCCCCCDDD` | Rd <= Rd \| IMM7, {S}=set status bits         [4] |
| `XOR{S}    Rd, Rx, Ry`                    | `1110S0XXX0YYYDDD` | Rd <= Rx ^ Ry, {S}=set status bits          [4,5] |
| `XOR{S}    Rd, Rx, #<IMM3>`               | `1110S0XXX1CCCDDD` | Rd <= Rx ^ IMM3, {S}=set status bits          [4] |
| `XOR{S}    Rd, #<IMM7>`                   | `1110S1CCCCCCCDDD` | Rd <= Rd ^ IMM7, {S}=set status bits          [4] |
| `CMP       Rd, #<IMM7>`                   | `111110CCCCCCCDDD` | Z <= Rd - IMM7, set status bits               [6] |
| `CMN       Rd, #<IMM7>`                   | `111111CCCCCCCDDD` | Z <= Rd + IMM7, set status bits               [6] |
| `SR        OP \| #<IMM2>, SYM \| #<IMM8>` | `111101IIH000MCVZ` | Alter F using OP and 8-bit constant         [7,8] |

**[1]** For `ADD{C}` and `SUB{C}`, the value of the [C]arry flag will be added into the sum/difference only when the "C" bit is set in the instruction.  The `SUB{C}` instruction adds the 2's complement of the subtrahend and in the "C" form also adds the inverted [C]arry flag (as 0/1).  This makes e.g. `SUB Rd, Rx, Ry` the sum `Rx + ~Ry +(~0) = Rx - Ry`; for `SUBC` with the [C]arry flag clear, the same formula holds, while for [C]arry set `Rx + ~Ry + (~1) = Rx - Ry - 1`.  The status bits are _always_ set after these instructions.  The ALU µop is transferred from bits 12…13 (`00`=`ADD`, `01`=`SUB`) and the ALU class from bits 13…14 is `00` (arithmetic class).

**[2]** The processor's [C]arry flag will be the first bit shifted into the argument by `LSL`/`LSR`; `ASR` will repeatedly shift the MSb and `ROR` will shift the LSb into the MSb.  After these instructions, the [C]arry status flag will hold the last bit to be moved.  A shift of zero moves no bits, therefore the carry flag will be clear.  The ALU µop is transferred from bits 11…12 (`00`=`LSL`, `01`=`LSR`, `10`=`ASR`, and `11`=`ROR`).  The ALU class from bits 13…14 is `01` (bitshift class).

**[3]** The 3-bit constant second operand mode multiplies the immediate value by two to get shifts of {0, 2, 4, 6, 8, 10, 12, 14} bit positions.  A zero-distance shift is a no-op, so that 3-bit value instead yields a single-bit shift.  In other words, `LSR Rd, Rx, #0` is not a valid instruction because the bit pattern triggers the CPU to do a `LSR Rd, Rx, #1`.

**[4]** For `ANDS`, `ORS`, `XORS`, after the operation the [M]inus, o[V]erflow, and [C]arry flags in the status register are set to reflect the value of bits 15, 14, and 0, respectively.  The [Z]ero flag is set to reflect the value's being zero or not.  The ALU µop is transferred from bits 12…13 (`00`=`AND`, `01`=`OR`, `10`=XOR) and the ALU class from bits 13…14 is `10` (logic class).

**[5]** For `AND` and `XOR` with two source register operands, if `Ry` is `R0` the value of that register is inverted to `0xFFFF`, since that pattern is far more useful in bitwise and/exclusive or operations.

**[6]** Normally the `ADD` and `SUB` instructions with `R0`/`Z` as the destination register suffice for comparisons, but that restricts non-destructive comparison with a 3-bit immediate value.  Explicit 7-bit compare/compare-negated instructions are present to address that deficiency.  The ALU µop is selected from bits 10…11; when bitwise-NOT is applied, the `10` from `CMP` becomes `01` (the µop for subtract) and the `11` from `CMN` becomes `00` (the µop for add).

**[7]** The cluster of SR* instructions reuse the bit pattern associated with the AND/OR/XOR instructions, implying that the same gating of the ALU could be reused by the status register alteration logic.  The status register is 8-bit and the instruction has room for an 8-bit constant, so were the ISA to be extended in the future this instruction could include them, as well.  In assembly the OPeration to be performed can be specified symbolically using AND, OR, or XOR (case insensitive):  `SR   OR, #0b1010` would reproduce bit pattern `0b1`**`101`**`…` from `OR{S}` in the `…1II…` component of the `SR` opcode (`AND`=`00`, `OR`=`01`, `XOR`=`10`).

**[8]** The constant bit pattern can be specified SYMbolically, using the status bit symbols HMCVZ in uppercase symbol for a `1` or lowercase `0`; omission of a symbol implies `0`.  An 8-bit numerical constant is permissible (though the programmer must ensure the correct ordering of the status bits).  E.g. the symbolic form `MZvc` equates with the numerical constant `0b1001` or `0x9` as well as the symbolic form `MZ`.


#### Pseudo-instructions

While there are no explicit `CMP` or `CMN` modes for multiple registers or short (3-bit) constants, the assembler accepts pseudo-instructions that are translated to `SUB` and `ADD` instructions:

| Mnemonic             | Actual code                  | Description                                                    |
| :------------------- | :--------------------------- | :------------------------------------------------------------- |
| `CMP.R Rx, Ry`       | `SUB R0, Rx, Ry`             | Subtract Ry from Rx, set flags, discard result                 |
| `CMP.I Rx, #<IMM3>`  | `SUB R0, Rx, #<IMM3>`        | Subtract the 3-bit constant from Rx, set flags, discard result |
| `CMN.R Rx, Ry`       | `ADD R0, Rx, Ry`             |                                                                |
| `CMN.I Rx, #<IMM3>`  | `ADD R0, Rx, #<IMM3>`        |                                                                |

Inverting the bits in a word is a bitwise NOT; this can be effected using an exclusive or with `0xFFFF`.  In other assembly languages a bit-test instruction sets status flags to the values at specific bit indices, which in this ISA can be accompished with an `ANDS` that discards its result and has `R0`/`Z` as it's `Ry` operand.  For bit indices not captured by `ANDS`, bit shift instructions with `R0` as the destination register can be used to non-destructively test an arbitrary bit index.

| Mnemonic             | Actual code                | Description                                                    |
| :------------------- | :------------------------- | :------------------------------------------------------------- |
| `NOT  Rd, Rx`        | `XOR Rd, Rx, R0`           | When Ry=R0, R0 is flipped to `0xFFFF` and Rd <= Rx ^ 0xFFFF    |
| `NOTS Rd, Rx`        | `XORS Rd, Rx, R0`          | Same as `NOT` but status bits are set from the result          |
| `BITS Rx`            | `ANDS R0, Rx, R0`          | When Ry=R0, R0 is flipped to `0xFFFF` and status bits are set  |
|                      |                            | from the discarded result (Rx & 0xFFFF)                        |
|                      |                            | ([M]=bit 15, [V]=bit 14, [C]=bit 0)                            |
| `BITC Rx, #0`        | `LSR R0, Rx, #1`           | [C]arry will be set to bit 0 of Rx.                            |
| `BITC Rx, #{1…13/2}` | `LSR R0, Rx, #(N+1)`       | [C]arry will be set to the given odd-index bit of Rx; e.g.     |
|                      |                            | since `LSR` sets [C] to the bit last moved, bit 1 requires 2   |
|                      |                            | shifts to move it into [C] while bit 13 needs 14 shifts        |
| `BITC Rx, #{2…14/2}` | `LSL R0, Rx, #(16-N)`      | [C]arry will be set to the given even-index bit of Rx; e.g.    |
|                      |                            | since `LSL` sets [C] to the bit last moved, bit 14 requires 2  |
|                      |                            | shifts to move it into [C] while bit 2 needs 14 shifts         |
| `BITC Rx, #15`       | `LSL R0, Rx, #1`           | [C]arry will be set to bit 15 of Rx                            |

Shortcut mnemonics for alterations to the status register are also provided:

| Mnemonic              | Actual code               | Description         |
| :-------------------- | :------------------------ | :------------------ |
| `SRA  SYM \| #<IMM4>` | `SR  AND, SYM \| #<IMM8>` | F <= F & IMM8       |
| `SRO  SYM \| #<IMM4>` | `SR  OR, SYM \| #<IMM8>`  | F <= F \| IMM8      |
| `SRX  SYM \| #<IMM4>` | `SR  XOR, SYM \| #<IMM8>` | F <= F ^ IMM8       |
| `SRCC`                | `SR  AND, McVZ`           | Clear carry         |
| `SRSC`                | `SR  OR, mCvz`            | Set carry           |
| `HALT`                | `SR  XOR, H`              | Halt the CPU        |

Since the `HALT` instruction toggles the H status bit, one might be tempted to call the instruction `PAUSE` instead — until one considers how a CPU no longer processing instructions would ever clear the H status bit.  In theory the CPU package would have a pin that can influence it.

### Data movement

All instructions with bits 14 and 15 clear are data movement instructions.

Data movement encompasses instructions that move bits into registers; move bits to special-purpose registers in the processor; and move data between registers and the system memory.


#### Inter-register

Introducing values into registers is a fundamental behavior of a processor.  The ISA includes instructions that transfer values from general-purpose registers to special-purpose registers — the `SSR` and `PC` registers — as well as instructions with embedded constants and bit-shift schemes.

| Mnemonic                                 | Bit pattern        | Description                                |
| :--------------------------------------- | :----------------- | :----------------------------------------- |
| `MOV          Rd, PC`                    | `0000000000000DDD` | Rd <= PC                               [1] |
| `MOV          Rd, SSR`                   | `0000000000001DDD` | Rd <= SSR                              [2] |
| `MOV          SSR, Rx`                   | `0000000000100XXX` | SSR <= (Rx & ~0x00F0) | (SSR & 0x00F0) [3] |
| `MOV          SSR, #<IMM4>`              | `000000000011CCCC` | SSR <= (SSR & ~0x000F) | IMM4          [4] |
| `MOV{Z}       Rd, #<IMM8>`               | `00001CCCCCCCCDDD` | Rd <= IMM8                             [5] |
| `MOV{K\|Z}{N} Rd, #<IMM4>, ROR#(2*SSS)`  | `0001NKSSSCCCCDDD` | Rd <= const                            [6] |
| `  MOV{Z}     Rd, #<IMM4>, ROR#(2*SSS)`  | `000100SSSCCCCDDD` | Rd <= IMM4 ROR {0,2,4,6,8,10,12,14}        |
| `  MOVZN      Rd, #<IMM4>, ROR#(2*SSS)`  | `000110SSSCCCCDDD` | Rd <= ~(IMM4 ROR {0,2,4,6,8,10,12,14})     |
| `  MOVK       Rd, #<IMM4>, ROR#(2*SSS)`  | `000101SSSCCCCDDD` | `MOVZ` but with untouched Rd bits retained |
| `  MOVKN      Rd, #<IMM4>, ROR#(2*SSS)`  | `000111SSSCCCCDDD` | `MOVZN` but with untouched Rd bits         |
|                                          |                    | retained prior to bitwise not              |

**[1]** There is no corresponding `MOV PC, Rx` since a `B Rx` instruction accomplishes the same thing.

**[2]** The entire SSR register, consisting of the status register in the top 8 bits; the PSEG in bits 4-7; and the DSEG in bits 0-3 is transferred to Rd.

**[3]** The value in the Rx register overwrites all *except* the PSEG component and unused status bits of the SSR.  This allows for the F (status) register to be restored after being pushed to a stack, for example.  Likewise, when working with data in a different memory segment a subroutine could `MOV M, SSR` then switch segments with `MOV SSR, #4`.  Subsequently, when the subroutine performs cleanup prior to returning to the caller it can restore whatever segment and flags existed using `MOV SSR, M`.

**[4]** With a 4-bit constant, only the lowest four bits of the SSR (the DSEG) are modified, making this effectively a data segment set instruction.

**[5]** The register is set to the 8-bit immediate value with the upper byte zeroed.

**[6]** The `N` bit, when set, applies a bitwise NOT to the value before writing to Rd.  The `K` bit, when set, introduces the value of Rd into the data load unit and preserves all bit positions outside of the 4-bit constant that is introduced; when clear, zero is loaded into the data load unit for the operation.  The 4-bit constant, IMM4, is effectively repositioned using a ROR shift of two-times the 3-bit shift constant:  so `SSS=111` equates with a shift of 14 bits, for example.  See the table below for how the values equate to bit positions in a word.

| ROR# | ROL# |    SSS | Resulting bit pattern |
| ---: | ---: | ------: | :-------------------: |
|  `0` |  `0` | `0b000` |  `0b0000000000001111` |
|  `2` | `14` | `0b001` |  `0b1100000000000011` |
|  `4` | `12` | `0b010` |  `0b1111000000000000` |
|  `6` | `10` | `0b011` |  `0b0011110000000000` |
|  `8` |  `8` | `0b100` |  `0b0000111100000000` |
| `10` |  `6` | `0b101` |  `0b0000001111000000` |
| `12` |  `4` | `0b110` |  `0b0000000011110000` |
| `14` |  `2` | `0b111` |  `0b0000000000111100` |

##### Pseudo-instructions

The list above does not include an instruction to move one register's value to another; basic register-to-register movement is implemented using `OR` and `XOR`:

| Mnemonic        | Actual code       | Description                                |
| :-------------- | :---------------- | :----------------------------------------- |
| `MOV  Rd, Rx`   | `OR   Rd, Rx, R0` | Rd <= Rx \| R0 = Rx \| 0x0000 = Rx         |
| `MOVN Rd, Rx`   | `XOR  Rd, Rx, R0` | Rd <= Rx ^ R0 = Rx ^ 0xFFFF = ~Rx          |
| `MOV  Rd, #0`   | `OR   Rd, R0, R0` | Rd <= R0 \| R0 = 0x0000 \| 0x0000 = 0x0000 |

Moving a 16-bit word into a register can be accomplished using up to 3 instructions, and can be heavily optimized by recognizing bit patterns in the word as well as in its bitwise NOT form.

| Mnemonic             | Actual code                                | Description                              |
| :------------------- | :----------------------------------------- | :--------------------------------------- |
| `MOV   Rd, #0xCA65`  | `MOVZ   Rd, #0x65`                         | Move the low byte into the register,     |
|                      | `MOVK   Rd, #0xC, ROR#4`                   | then the top nibble of the MSB, followed |
|                      | `MOVK   Rd, #0xA, ROR#8`                   | by the bottom nibble of the MSB.  Used   |
|                      |                                            | when the IMM16 bit pattern doesn't allow |
|                      |                                            | for optimizations.                       |
|                      |                                            |                                          |
| `MOV   Rd, #0xD02F`  | `MOVZ   Rd, #0x2F`                         | The bottom nibble of the MSB is zero, so |
|                      | `MOVK   Rd, #0xD, ROR#4`                   | it does not need to be set.              |
|                      |                                            |                                          |
| `MOV   Rd, #0xFD93`  | `MOVZ   Rd, #0x9B`                         | Bitwise NOT of the constant yields 0x26C |
|                      | `LSL    Rd, #2`                            | which shifted right by two bits is 0x9B. |
|                      | `NOT    Rd`                                | Shift back then do a bitwise NOT again.  |
|                      |                                            |                                          |
|                      |             OR                             |                                          |
|                      |                                            |                                          |
| `MOV   Rd, #0xFD93`  | `MOVZ   Rd, #0x6C`                         | The LSB of the NOT'ed value followed by  |
|                      | `MOVKN  Rd, #2, ROR#8`                     | the single bit in the MSB and NOT.       |

Many negative constants can be loaded in fewer instructions through creative application of the bitwise-NOT and bit shift, e.g. `MOVZN  Rd, #0b0001, ROR#0` would load Rd with `0xFFFE` or `-2`.  Loading 12- or 8-bit constants behaves in similar fashion, but the two special cases of replacing the high- or low-byte of a register are included:

| Mnemonic                | Actual code                               | Description                              |
| :---------------------- | :---------------------------------------- | :--------------------------------------- |
| `MOVK.L    Rd, #<IMM8>` | `MOVK   Rd, #<IMM8> & 0xF, ROR#0`         | Move each nibble of the 8-bit byte into  |
|                         | `MOVK   Rd, #(<IMM8> >> 4) & 0xF, ROR#12` | into the low-byte of Rd in sequence.     |
| `MOVK.H    Rd, #<IMM8>` | `MOVK   Rd, #<IMM8> & 0xF, ROR#8`         | Move each nibble of the 8-bit byte into  |
|                         | `MOVK   Rd, #(<IMM8> >> 4) & 0xF, ROR#4`  | into the high-byte of Rd in sequence.    |

It may be more natural for the programmer to think of the bit shifts in terms of an ROL; the assembler can accept `ROL#<shift>` syntax and reinterpret it as `ROR#(16 - <shift>)%16`.

Loading addresses into a register, either from an immediate value or from a label, is given its own pseudo-instructions:

| Mnemonic                 | Actual code                            | Description                         |
| :----------------------- | :------------------------------------- | :---------------------------------- |
| `ADR      Rd, <LABEL>`   | `MOVZ    Rd, LABEL & 0xFF`             | Rd <= LABEL & 0x0FFFF               |
|                          | `MOVK.H  Rd, (LABEL >> 8) & 0xFF`      |                                     |
| `ADR      Rd, #${S}HHLL` | `MOVZ    Rd, #0x{S}HHLL & 0xFF`        | Rd <= $HHLL                         |
|                          | `MOVK.H  Rd, #(0x{S}HHLL >> 8) & 0xFF` |                                     |
| `ADRS     Rd, <LABEL>`   | `MOVZ    Rd, LABEL >> 16, ROR#0`       | Rd <= (LABEL & 0xF0000) >> 16       |
| `ADRS     Rd, #${S}HHLL` | `MOVZ    Rd, #0x{S}HHLL >> 16, ROR#0`  | Rd <= ${S}                          |
| `ADRS     DSEG, <LABEL>` | `MOV     SSR, #LABEL >> 16`            | SSR.DSEG <= (LABEL & 0xF0000) >> 16 |


#### Memory-based

The ISA includes a number of instructions which move data between system memory and the general-purpose registers.  The first subset of modes use memory addresses taken directly from a register, optionally indexed by the value of a second register.  A conditional form is also present that only performs the memory operation if the indicated condition code (CC) is satisfied by the current state of the CPU.

| Mnemonic                      | Bit pattern        | Description                           |
| :---------------------------- | :----------------- | :------------------------------------ |
| `LDR       Rd, [Rx+Ry]`       | `0010001YYYXXXDDD` | Rd <= [Rx + Ry]                       |
| `LDL       Rd, [Rx]`          | `0010000000XXXDDD` | Rd.LSB <= [Rx].LSB                    |
| `LDH       Rd, [Rx]`          | `0010000001XXXDDD` | Rd.MSB <= [Rx].MSB                    |
| `LDR.<CC>  Rd, [Rx]`          | `0010010cccXXXDDD` | If CC: Rd <= [Rx]                     |
| `STO       [Rx+Ry], Rd`       | `0011001YYYXXXDDD` | [Rx + Ry] <= Rd (no offset, Ry=Z)     |
| `STL       [Rx], Rd`          | `0011000000XXXDDD` | [Rx].LSB <= Rd.LSB                    |
| `STH       [Rx], Rd`          | `0011000001XXXDDD` | [Rx].MSB <= Rd.MSB                    |
| `STO.<CC>  [Rx], Rd`          | `0011010cccXXXDDD` | If CC: [Rx] <= Rd                     |

The second subset are auto-modification modes.  For the load instructions, the value at the location in the address register is loaded into the destination register, and then the address register is incremented/decremented by a signed 4-bit constant or the value of a third register.  For the store instructions, the address register is first incremented/decremented then the source register's value is written to that location.

| Mnemonic                      | Bit pattern        | Description                           |
| :---------------------------- | :----------------- | :------------------------------------ |
| `LDR       Rd, [Rx], #<IMM4>` | `001010CCCCXXXDDD` | Rd <= [Rx], Rx += IMM4            [1] |
| `LDR       Rd, [Rx], Ry`      | `0010110YYYXXXDDD` | Rd <= [Rx], Rx += Ry                  |
| `STO       #<IMM4>, [Rx], Rd` | `001110CCCCXXXDDD` | Rx += IMM4, [Rx] <= Rd            [1] |
| `STO       Ry, [Rx], Rd`      | `0011110YYYXXXDDD` | Rx += Ry, [Rx] <= Rd                  |

**[1]** The signed 4-bit immediate has the range [-8,+7].

The most common use pattern for the auto-modification modes is pre-decrement `STO` with post-increment `LDR`:  a stack.

##### Stacks

Stacks are implemented on the PHRI by choosing a general-purpose register that will act as the *stack pointer*.  The convention is that `R7` is used, hence its alias of `SP`.

Pushing a register to the stack is accomplished with the `STO` instruction, bearing in mind that registers are two bytes in size:

```
STO     #-2, [SP], R3
```

The `SP` register is pre-decremented by 2, then the value in `R3` is written at the memory address in `SP`.  To pop that value off the stack into `R5`:

```
LDR     R5, [SP], #+2
```

The value at the memory address in `SP` is read into `R5`, then `SP` is post-incremented by 2.

##### Pseudo-instructions

Because stacks are frequently used, the ISA includes pseudo instructions to simplify their use.

| Mnemonic               | Actual code                      | Description                              |
| :--------------------- | :------------------------------- | :--------------------------------------- |
| `PUSH     Rx{, Ry, …}` | `STO  #-2, [SP], R#` for each R# | Push Rx, Ry, … to the stack in `SP`  [1] |
| `POP      Rx{, Ry, …}` | `LDR  R#, [SP], #+2` for each R# | Pop Rx, Ry, … from the stack in `SP` [2] |

**[1]** The registers are pushed in the *same sequence* as the operand list.  They must be popped in reverse order (see **[2]** for the `POP` pseudo-instruction).

**[2]** The registers are popped in the *reverse sequence* of the operand list.  They should have been pushed in the sequence as written (see **[1]** for the `PUSH` pseudo-instruction).

Loading/storing data to the address in a register without an offset is realized using `R0`/`Z` as the `Ry` operand, but a pseudo-instruction that omits the explicit specification of `R0`/`Z` is provided:

| Mnemonic            | Actual code       | Description                            |
| :------------------ | :---------------- | :------------------------------------- |
| `LDR    Rd, [Rx]`   | `LDR Rd, [Rx+R0]` | Load word at address in Rx (offset 0)  |
| `STO    [Rx], Rd`   | `STO [Rx+R0], Rd` | Store word to address in Rx (offset 0) |


### Branching

All instructions with bit 15 clear and bit 14 set are branching instructions.

Branching is the action of redirecting program execution to a location other than the next subsequent word.  The two typical use cases are:

- Conditionals
- Subroutines

For example, consider the following code:

```
x = 10;
if ( x % 2 ) {
    x++;
}
```

Since `x` is even, the conditional fails and the increment must be skipped by branching past it.  Calling a subroutine is slightly more complicated:  the `PC` must be saved before it is modified, so that the subroutine can restore that saved value to continue program execution in the calling context.  In the PHRI ISA, saving the `PC` before altering it is known as *linking*, and can be enabled on any of the branch instructions.  The value of the `PC` is transferred to one of the general purpose registers.  For the 12-bit constant offset modes, the `R6`/`L` is implied, while the other modes allow an arbitrary general purpose register to be chosen:

```
            MOVZ        A, #18          ; Set A to 0x0012
            BRL         two_a_plus_one  ; copy the PC to R6/L, then offset the PC to reach subroutine two_a_plus_one
              :
            
            ADR         R1, three_a     ; M <= address of subroutine three_a
            BRL         R1, R1          ; set the PC to the address in R1, overwriting with the PC for return
            
two_a_plus_one:
            ADD         A, A, A         ; A <= A + A = 2A
            ADD         A, #1           ; A <= A + 1
            B           L               ; restore saved PC value in R6/L 

three_a:    PUSH        R1              ; Save the return address on the stack
            ADD         R1, A, A        ; R1 <= A + A = 2A
            ADD         A, A, R1        ; A <= A + R1 = A + 2A = 3A
            POP         R1              ; Restore return address from stack
            B           R1              ; restore saved PC value in R1
```


#### Absolute mode

To redirect execution to an explicit address in memory, a 16-bit value is required.  Since instructions are 16-bit themselves, there is no way to embed an address in them.  Absolute branching is only possible using addresses present in general-purpose registers.

| Mnemonic               | Bit pattern        | Description                                     |
| :--------------------- | :----------------- | :---------------------------------------------- |
| `B        Rx`          | `0100000000000XXX` | PC <= Rx                                        |
| `BL       Rx, Ry`      | `011000000YYY0XXX` | Ry <= PC, PC <= Rx           (Rx = Ry is valid) |
| `B.<CC>   Rx`          | `0101ccc000000XXX` | If CC: PC <= Rx                                 |
| `BL.<CC>  Rx, Ry`      | `0111ccc00YYY0XXX` | If CC: Ry <= PC, PC <= Rx    (Rx = Ry is valid) |

The `ccc` bits represent the condition code which, if satisfied, will cause the instruction to be executed.  The `XXX` and `YYY` are register indices.

A segmented branch atomically sets the `PC` **and** the `PSEG` to immediately jump to code in a specific 64 KiB segment of memory.  This is the only method for direct modification of the `PSEG` by user code (all other cases involve 16-bit address overflow or underflow as the CPU is modifying the `PC`).

| Mnemonic               | Bit pattern        | Description                                     |
| :--------------------- | :----------------- | :---------------------------------------------- |
| `BSEG     Rx, #<IMM4>` | `01000011CCCC0XXX` | PSEG <= IMM4, PC <= Rx                          |
| `BSEG     Rx, Ry`      | `010000100YYY0XXX` | PSEG <= Ry & 0x000F, PC <= Rx                   |

There are no linking segmented branch instructions since a 16-bit register can only hold the value of the `PC`.


##### Pseudo instructions

Since the `PC` starts at `$0000` in segment `$0` at boot, basic bootstrapping code is expected to be present at that address.  The `R0`/`Z` register allows for a straightforward instruction that reenters that bootstrapping code:

```
BSEG    R0, #0
```

The ISA includes a pseudo-instruction for this:

| Mnemonic  | Actual code     | Description                   |
| :-------- | :-------------- | :---------------------------- |
| `RSTRT`   | `BSEG   R0, #0` | PSEG <= `$0`, PC <= `$0000`   |

A conditional and unconditional `RET` instruction makes use of the absolute `B` instruction and the conventional linking register:

| Mnemonic      | Actual code      | Description            |
| :------------ | :--------------- | :--------------------- |
| `RET`         | `B      L`       | PC <= R6/L             |
| `RET.<CC>`    | `B.<CC> L`       | If CC: PC <= R6/L      |


#### Relative mode

In relative addressing mode, a signed two's complement value is added to the `PC` register.  The `PC` is adjusted after an instruction is fetched, so an offset of zero (0) refers to the location of the instruction immediately following the branch being evaluated.

The *register offset relative* mode sources the offset from a general-purpose register.  In this mode, the value of the register is added to the `PC` to produce the target address; the offset is in units of *bytes*.


| Mnemonic            | Bit pattern        | Description                                        |
| :------------------ | :----------------- | :------------------------------------------------- |
| `BR       Rx`       | `0100000000001XXX` | PC <= PC + Rx                                      |
| `BRL      Rx, Ry`   | `011000000YYY1XXX` | Ry <= PC, PC <= PC + Rx         (Rx = Ry is valid) |
| `BR.<CC>  Rx`       | `0101ccc000001XXX` | If CC: PC <= PC + Rx                               |
| `BRL.<CC> Rx, Ry`   | `0111ccc00YYY1XXX` | If CC: Ry <= PC, PC <= PC + Rx  (Rx = Ry is valid) |


The *immediate offset relative* mode sources the offset from a constant embedded in the instruction.  Since instructions are 16-bit the embedded constant is multiplied by two before being added to the `PC`; the offset is in units of *words*.  Thus, an 11-bit constant is actually a 12-bit signed offset, and an 8-bit constant is a 9-bit signed offset.  

| Mnemonic            | Bit pattern        | Description                                        |
| :------------------ | :----------------- | :------------------------------------------------- |
| `BR       <OFFSET>` | `01001CCCCCCCCCCC` | PC <= PC + (OFFSET * 2)                (11b const) |
| `BRL      <OFFSET>` | `01101CCCCCCCCCCC` | R6 <= PC, PC <= PC + (OFFSET * 2)      (11b const) |
| `BR.<CC>  <OFFSET>` | `0101ccc1CCCCCCCC` | PC <= PC + (OFFSET * 2)                 (8b const) |
| `BRL.<CC> <OFFSET>` | `0111ccc1CCCCCCCC` | R6 <= PC, PC <= PC + (OFFSET * 2)       (8b const) |

## System startup/reset

On hardware power-on or reset, the PHRI hardware initializes all mutable memory and registers to zero.  This places both the `PSEG` and `DSEG` in segment `$0` and places the `PC` at `$0000` in that segment.  Since `R7`/`SP` starts at `$0000` the stack will build downwards from the top of segment `$0` — the first word pushed will reside at `$FFFE`.

Typically a ROM containing system bootstrapping code will be located at `$0000` in segment `$0`.  Since the `PC` starts at that address, at power-on/reset the CPU will begin executing that ROM code.

The data bus is pulled low by default on read; if no hardware responds to the address a zero word will be read.  With the `PC` starting at `$0000`, on a system devoid of any memory devices the CPU will read a `0x0000` instruction.  That decodes to `MOV R0, PC` — transfer the `PC` to register `R0` (discarding the value).  The `PC` will advance to `$0002` and fetch another `0x0000` instruction.  As this instruction stream executes, eventually the `PSEG` will increment to `$1` and the `PC` will wrap back around to `$0000` before continuing in like fashion through that segment of memory.  Given enough time, the `PSEG` will increment from `$F` back to `$0` and repeat the entire sequence again.  In this way, the CPU instruction fetch, decode, and execute can be tested and the march of the `PC` and `PSEG` registers can be observed.
