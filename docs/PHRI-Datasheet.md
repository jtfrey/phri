# PHRI Data Sheet

The PHRI (PHRI is a Hypothetical RISC ISA) is an invented 16-bit ISA.  It features a uniform 16-bit data bus and 20-bit address bus; memory is addressed in *segments* of 64 KiB.


## Register set

The PHRI possesses twelve 16-bit registers.  There are eight general-purpose registers labelled R0 through R7; R0 is a read-only register that always contains a value of zero.  The remaining registers are usable in any capacity across the instructions, but the assembly mnemonics include aliases for each that convey a possible purpose:

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

A program counter (`PC`) register holds the 16-bit address of the instruction that will be fetched next.  At boot, the `PC` is initialized to `$00000` — program code is expected to originate at this address.  The leading nibble comes from the *program counter segment* or `PSEG`, which comes from the high nibble of the `MSEG` register.  When the program counter is updated using a relative offset, the `PSEG` will wrap to the subsequent/previous segment automatically when crossing a `$0000` address.

Instructions that load/store data must also work with memory addresses.  A separate *data segment* or `DSEG` comes from the low nibble of the `MSEG` register.  Like the `PSEG`, the `DSEG` is initialized to zero.  It can be directly modified by user code to alter the target address of subsequent load/store instructions.  If there were a single segment value, user code would effectively switch itself out of scope when altering a singular segment register; keeping the `PSEG` and `DSEG` separate keeps the `PC` functional.  The programmer should bear in mind, though, that the `DSEG` is **not** automatically adjusted:  user code must handle the `DSEG` adjustments as necessary to access data held in other segments.  This has a direct effect on PHRI stacks:  a stack can never exceed 64 KiB in size and it is up to the user code to ensure the correct `DSEG` is selected when pushing/popping data.

The instruction register (`INSTR`) latches the instruction word when one is fetched.

The final register `F` is an 8-bit set of condition (status) flags.  Only four bits are currently utilized:

- `M`: the minus flag, set to indicate a value has bit 15 set
- `C`: the carry flag, set to indicate an arithmetic operation produced a carry out of bit 15
    - this flag is also used to indicate the bit value to shift into a register on bit shift operations
    - this flag is also set to indicate the captured bit index value on bit shift operations
- `V`: the overflow flag, set to indicate a signed arithmetic operation produced a carry out of bit 14
- `Z`: the zero flag, set to indicate a value was zero, clear if non-zero

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
| `MOV      R0, Rx`       | R0 as dest = discard value         |
| `MOV      Rx, Rx`       | Move a register to itself          |
| `<OPCODE> R0, R0, R0`   | OPCODE = ADD, SUB, AND, OR, XOR    |
| `SR       AND, #0b1111` | Status bits retain existing values |


### Arithmetic and logic

All instructions with bit 15 set are handled by the ALU.

This category includes instructions to perform two's-complement addition/subtraction, shift and test bits, and perform standard bitwise logic like AND, OR, and XOR.  This category also includes instructions to alter the condition flags in the `F` register (e.g. clear carry).

| Mnemonic                                  | Bit pattern        | Description                                       |
| :---------------------------------------- | :----------------- | :------------------------------------------------ |
| `ADD{S}    Rd, Rx, Ry`                    | `1000S0XXX0YYYDDD` | Rd <= Rx + Ry, {S}=set status bits            [1] |
| `ADD{S}    Rd, Rx, #<IMM3>`               | `1000S0XXX1CCCDDD` | Rd <= Rx + IMM3, {S}=set status bits          [1] |
| `ADD{S}    Rd, #<IMM7>`                   | `1000S1CCCCCCCDDD` | Rd <= Rd + IMM7, {S}=set status bits          [1] |
| `SUB{S}    Rd, Rx, Ry`                    | `1001S0XXX0YYYDDD` | Rd <= Rx - Ry, {S}=set status bits            [1] |
| `SUB{S}    Rd, Rx, #<IMM3>`               | `1001S0XXX1CCCDDD` | Rd <= Rx - IMM3, {S}=set status bits          [1] |
| `SUB{S}    Rd, #<IMM7>`                   | `1001S1CCCCCCCDDD` | Rd <= Rd - IMM7, {S}=set status bits          [1] |
| `SH{C}L    Rd, Rx, Ry`                    | `1010C0XXX0YYYDDD` | Rd <= Rx << (Ry&0xF)  {C}=C flag fill       [2,3] |
| `SH{C}L    Rd, Rx, #<IMM3>`               | `1010C0XXX1cccDDD` | Rd <= Rx << (IMM3+1), {C}=C flag fill       [2,4] |
| `SH{C}L    Rd, #<IMM4>, #<IMM3>`          | `1010C1BBBccccDDD` | Rd <= Rd << IMM4, {C}=C flag fill           [2,5] |
| `SH{C}R    Rd, Rx, Ry`                    | `1011C0XXX0YYYDDD` | Rd <= Rx >> (Ry&0xF), {C}=C flag fill       [2,6] |
| `SH{C}R    Rd, Rx, #<IMM3>`               | `1011C0XXX1cccDDD` | Rd <= Rx >> (IMM3+1), {C}=C flag fill       [2,7] |
| `SH{C}R    Rd, #<IMM4>, #<IMM3>`          | `1011C1BBBccccDDD` | Rd <= Rd >> IMM4, {C}=C flag fill           [2,8] |
| `AND{S}    Rd, Rx, Ry`                    | `1100S0XXX0YYYDDD` | Rd <= Rx & Ry, {S}=set status bits         [9,10] |
| `AND{S}    Rd, Rx, #<IMM3>`               | `1100S0XXX1CCCDDD` | Rd <= Rx & IMM3, {S}=set status bits          [9] |
| `AND{S}    Rd, #<IMM7>`                   | `1100S1CCCCCCCDDD` | Rd <= Rd & IMM7, {S}=set status bits          [9] |
| `OR{S}     Rd, Rx, Ry`                    | `1101S0XXX0YYYDDD` | Rd <= Rx | Ry, {S}=set status bits            [9] |
| `OR{S}     Rd, Rx, #<IMM3>`               | `1101S0XXX1CCCDDD` | Rd <= Rx | IMM3, {S}=set status bits          [9] |
| `OR{S}     Rd, #<IMM7>`                   | `1101S1CCCCCCCDDD` | Rd <= Rd | IMM7, {S}=set status bits          [9] |
| `XOR{S}    Rd, Rx, Ry`                    | `1110S0XXX0YYYDDD` | Rd <= Rx ^ Ry, {S}=set status bits         [9,10] |
| `XOR{S}    Rd, Rx, #<IMM3>`               | `1110S0XXX1CCCDDD` | Rd <= Rx ^ IMM3, {S}=set status bits          [9] |
| `XOR{S}    Rd, #<IMM7>`                   | `1110S1CCCCCCCDDD` | Rd <= Rd ^ IMM7, {S}=set status bits          [9] |
| `CMP       Rd, #<IMM7>`                   | `111110CCCCCCCDDD` | Z <= Rd - IMM7, set status bits              [11] |
| `CMN       Rd, #<IMM7>`                   | `111111CCCCCCCDDD` | Z <= Rd + IMM7, set status bits              [11] |
| `SR        OP \| #<IMM2>, SYM \| #<IMM4>` | `111101II0000MCVZ` | Alter F using OP and 4-bit constant       [12,13] |

**[1]** For `ADDS` and `SUBS`, the value of the [C]arry flag will be added into the sum/difference and modified during execution to reflect carry/borrow.

**[2]** For bit shift instructions with a `C` in the mnemonic, the value of the [C]arry flag coming into execution is the bit value shifted into the register.  E.g. if `R1` contains `0b1010101010101010` and [C]arry is set, `SHCL R1, R1, #3` will leave `0b0101010101010111` in `R1`.

**[3]** The lowest nibble of the `Ry` register is the number of bit positions to shift (0 through 15).  The next-highest three bits of `Ry` indicate the bit position whose value after the shift should be copied to the [C]arry flag in the status register.  The index is calculated from the three bits as `(8 | ~BBB)`, so the bit pattern `000` is bit 15, and `111` is bit 8.

**[4]** The 3-bit immediate value is biased by one so that the instruction provides for shifts of 1 to 8 bit positions.  The bit at index 15 after the shift is copied to the [C]arry flag in the status register.

**[5]** The IMM3 value from the instruction is used to compute `(8 | ~IMM3)` as the bit position to copy to the [C]arry flag in the status register.

**[6]** The lowest nibble of the `Ry` register is the number of bit positions to shift (0 through 15).  The next-highest three bits of `Ry` indicate the bit position whose value after the shift should be copied to the [C]arry flag in the status register.  The index is the pattern itself, so `000` is bit 0, and `111` is bit 7.

**[7]** The 3-bit immediate value is biased by one so that the instruction provides for shifts of 1 to 8 bit positions.  The bit at index 0 after the shift is copied to the [C]arry flag in the status register.

**[8]** The IMM3 value from the instruction is the bit position to copy to the [C]arry flag in the status register.

**[9]** For `ANDS`, `ORS`, `XORS`, after the operation the [M]inus, o[V]erflow, and [C]arry flags in the status register are set to reflect the value of bits 15, 14, and 0, respectively.  The [Z]ero flag is set to reflect the value's being zero or not.

**[10]** For `AND` and `XOR` with two source register operands, if `Ry` is `R0` the value of that register is inverted to `0xFFFF`, since that pattern is far more typically-used in bitwise and and exclusive or operations.

**[11]** Normally the `ADDS` and `SUBS` instructions with `R0`/`Z` as the destination register suffice for comparisons, but that restricts non-destructive comparison with a 3-bit immediate value.  Explicit 7-bit compare/compare-negated instructions are present to address that deficiency.

**[12]** The cluster of SR_ instructions reuse the bit pattern associated with the AND/OR/XOR instructions, implying that the same gating of the ALU could be reused by the status register alteration logic.  The status register is 8-bit and the instruction has room for an 8-bit constant, so were the ISA to be extended in the future this instruction could include them, as well.  In assembly the OPeration to be performed can be specified symbolically using AND, OR, XOR, or SET (case insensitive):  `SR   OR, #0b1010` would reproduce bit pattern `0b1`**`101`**`…` from `OR{S}` in the `…1II…` component of the `SR` opcode.

**[13]** The constant bit pattern can be specified SYMbolically, using the four status bit symbols MCVZ in uppercase symbol for a `1` or lowercase `0`; omission of a symbol implies `0`.  A 4-bit numerical constant is permissible (though the programmer must ensure the correct ordering of the status bits).  E.g. the symbolic form `MZvc` equates with the numerical constant `0b1001` or `0x9` as well as the symbolic form `MZ`.


#### Pseudo-instructions

While there are no explicit `CMP` or `CMN` modes for multiple registers or short (3-bit) constants, the assembler accepts pseudo-instructions that are translated to `SUBS` and `ADDS` instructions:

| Mnemonic           | Actual code            | Description                                                    |
| :----------------- | :--------------------- | :------------------------------------------------------------- |
| `CMP  Rx, Ry`      | `SUBS R0, Rx, Ry`      | Subtract Ry from Rx, set flags, discard result                 |
| `CMP  Rx, #<IMM3>` | `SUBS R0, Rx, #<IMM3>` | Subtract the 3-bit constant from Rx, set flags, discard result |
| `CMN  Rx, Ry`      | `ADDS R0, Rx, Ry`      | Add Ry to Rx, set flags, discard result                        |
| `CMN  Rx, #<IMM3>` | `ADDS R0, Rx, #<IMM3>` | Add the 3-bit constant to Rx, set flags, discard result        |

Inverting the bits in a word is a bitwise NOT; this can be effected using an exclusive or with `0xFFFF`.  In other assembly languages a bit-test instruction sets status flags to the values at specific bit indices, which in this ISA can be accompished with an ANDS that discards its result and has `R0`/`Z` as it's `Ry` operand.  For bit indices not captured by ANDS, the `SHR` and `SHL` instructions can be used with a shift of zero and a bit index.

| Mnemonic           | Actual code               | Description                                                    |
| :----------------- | :------------------------ | :------------------------------------------------------------- |
| `NOT  Rd, Rx`      | `XOR Rd, Rx, R0`          | When Ry=R0, R0 is flipped to `0xFFFF` and Rd <= Rx ^ 0xFFFF    |
| `NOTS Rd, Rx`      | `XORS Rd, Rx, R0`         | Same as `NOT` but status bits are set from the result          |
| `BITS Rx`          | `ANDS R0, Rx, R0`         | When Ry=R0, R0 is flipped to `0xFFFF` and status bits are set  |
|                    |                           | from the result of (Rx & 0xFFFF), which is discarded           |
| `BIT  Rx, #<IMM4>` | `SH[R|L] Rx, #0, #<IMM3>` | `SHR` if 0 <= IMM4 < 8; `SHL` 8 <= IMM4 <= 15                  |

Shortcut mnemonics for alterations to the status register are also provided:

| Mnemonic              | Actual code              | Description      |
| :-------------------- | :------------------ ---- | :--------------- |
| `SRA  SYM \| #<IMM4>` | `SR  AND, SYM | #<IMM4>` | F <= F & IMM4    |
| `SRO  SYM \| #<IMM4>` | `SR  OR, SYM | #<IMM4>`  | F <= F | IMM4    |
| `SRX  SYM \| #<IMM4>` | `SR  XOR, SYM | #<IMM4>` | F <= F ^ IMM4    |
| `SRS  SYM \| #<IMM4>` | `SR  SET, SYM | #<IMM4>` | F <= IMM4        |


### Data movement

All instructions with bits 14 and 15 clear are data movement instructions.

Data movement encompasses instructions that move words between registers in the processor as well as instructions that move data between registers and the system memory.


#### Inter-register

Introducing values into registers is a fundamental behavior of a processor.  The ISA includes instructions that transfer values from one register to another — including the `MSEG` and `PC` registers — as well as instructions with embedded 8-bit constants to set the LSB or MSB of a register.

| Mnemonic                  | Bit pattern        | Description                           |
| :------------------------ | :----------------- | :------------------------------------ |
| `MOV       Rd, Rx`        | `0000000000XXXDDD` | Rd <= Rx                              |
| `MVN       Rd, Rx`        | `0000000001XXXDDD` | Rd <= ~Rx                             |
| `MOV       Rd, PC, Rx`    | `0000000010XXXDDD` | Rd <= PC + Rx                         |
| `MOV       PC, Rx`        | `0000000011XXX000` | PC <= Rx                              |
| `MOV       Rd, MSEG`      | `0000000100000DDD` | Rd <= MSEG                            |
| `MOV       MSEG, Rx`      | `0000000101XXX000` | MSEG <= Rx                            |
| `MOV       MSEG, #<IMM4>` | `00000001100CC000` | MSEG <= IMM4               (2b const) |
| `SEL       Rd, Rx, Ry`    | `0000001YYYXXXDDD` | Rd <= (Rd == 0) ? Rx : Ry             |
| `MVL       Rd, #<IMM8>`   | `00010CCCCCCCCDDD` | Rd.LSB <= IMM8             (8b const) |
| `MVH       Rd, #<IMM8>`   | `00011CCCCCCCCDDD` | Rd.MSB <= IMM8             (8b const) |

##### Pseudo-instructions

Zeroing a register is a common action; the presence of the `R0`/`Z` register makes this easy, so a pseudo-instruction is provided.  The `MVL` and `MVH` instructions only affect the LSB/MSB of the register; it is often necessay to zero the opposing byte.

| Mnemonic               | Actual code                    | Description            |
| :--------------------- | :----------------------------- | :--------------------- |
| `ZERO     Rd`          | `MOV  Rd, R0`                  | Rx <= `$0000`          |
| `MV0L     Rd, #<IMM8>` | `MOV  Rd, R0; MVL Rd, #<IMM8>` | Rx <= `$00XX`          |
| `MVH0     Rd, #<IMM8>` | `MOV  Rd, R0; MVH Rd, #<IMM8>` | Rx <= `$XX00`          |

Setting a register to a memory address or label happens with great frequency; a pseudo-instruction is included to improve code readability.

| Mnemonic               | Actual code                            | Description              |
| :--------------------- | :------------------------------------- | :----------------------- |
| `ADR      Rd, <LABEL>` | `MVL Rd, LABEL.LSB; MVH Rd, LABEL.MSB` | Rd <= address of label   |
| `ADR      Rd, $SHHLL`  | `MVL Rd, #$LL; MVH Rd, #$HH`           | Rd <= `$HHLL`            |
| `ADRS     <LABEL>`     | `MOV MSEG, (LABEL & #0xF000) >> #16`   | DSEG <= segment of label |
| `ADRS     Rd, $SHHLL`  | `MOV MSEG, (LABEL & #$SHHLL) >> #16`   | DSEG <= segment of addr  |

With `R6` being the traditional register used for linking (see the section on Branching instructions), a bare return from a subroutine pseudo-instruction is possible (as is one with a register specified):

| Mnemonic               | Actual code                    | Description            |
| :--------------------- | :----------------------------- | :--------------------- |
| `RET`                  | `MOV PC, L`                    | PC <= R6/L             |
| `RET      Rx`          | `MOV PC, Rx`                   | PC <= Rx               |


#### Memory-based

The ISA includes a number of instructions which move data between system memory and the general-purpose registers.

| Mnemonic                      | Bit pattern        | Description                           |
| :---------------------------- | :----------------- | :------------------------------------ |
| `LDR       Rd, [Rx+Ry]`       | `0010001YYYXXXDDD` | Rd <= [Rx + Ry]                       |
| `LDL       Rd, [Rx]`          | `0010000000XXXDDD` | Rd.LSB <= [Rx].LSB                    |
| `LDH       Rd, [Rx]`          | `0010000001XXXDDD` | Rd.MSB <= [Rx].MSB                    |
| `LDR.<CC>  Rd, [Rx]`          | `0010010SSSXXXDDD` | If CC: Rd <= [Rx]                     |
| `LDR       Rd, [Rx], #<IMM4>` | `001010CCCCXXXDDD` | Rd <= [Rx], Rx += IMM4                |
| `LDR       Rd, [Rx], Ry`      | `0010110YYYXXXDDD` | Rd <= [Rx], Rx += Ry                  |
| `STO       [Rx+Ry], Rd`       | `0011001YYYXXXDDD` | [Rx + Ry] <= Rd (no offset, Ry=Z)     |
| `STL       [Rx], Rd`          | `0011000000XXXDDD` | [Rx].LSB <= Rd.LSB                    |
| `STH       [Rx], Rd`          | `0011000001XXXDDD` | [Rx].MSB <= Rd.MSB                    |
| `STO.<CC>  [Rx], Rd`          | `0011010SSSXXXDDD` | If CC: [Rx] <= Rd                     |
| `STO       #<IMM4>, [Rx], Rd` | `001110CCCCXXXDDD` | [Rx] <= Rd, Rx += IMM4                |
| `STO       Ry, [Rx], Rd`      | `0011110YYYXXXDDD` | Rx += Ry, [Rx] <= Rd                  |

The ISA includes load and store instructions that automatically adjust the address register.  For `LDR` instructions the adjustment is made after the data has been loaded; for `STO` the adjustment is made before the data is loaded.  Load instructions are thus post-increment or post-decrement, while store instructions are pre-increment or pre-decrement.  The most common use pattern is pre-decrement `STO` with post-increment `LDR`:  a stack.

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

| Mnemonic      | Actual code          | Description                   |
| :------------ | :------------------- | :---------------------------- |
| `PUSH     Rx` | `STO  #-2, [SP], Rx` | Push Rx to the stack in `SP`  |
| `POP      Rx` | `LDR  Rx, [SP], #+2` | Pop Rx from the stack in `SP` |

Loading/storing data to the address in a register without an offset is realized using `R0`/`Z` as the `Ry` operand, but a pseudo-instruction that omits the explicit specification of `R0`/`Z` is provided:

| Mnemonic            | Actual code       | Description                   |
| :------------------ | :---------------- | :---------------------------- |
| `LDR      Rd, [Rx]` | `LDR Rd, [Rx+R0]` | Load word at address in Rx    |
| `STO      [Rx], Rd` | `STO [Rx+R0], Rd` | Store word to address in Rx   |


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

Since `x` is even, the conditional fails and the increment must be skipped by branching past it.  Calling a subroutine is slightly more complicated:  the `PC` must be saved before it is modified, so that the subroutine can restore that saved value to continue program execution in the calling context.  In the PHRI ISA, saving the `PC` before altering it is known as `linking`, and can be enabled on any of the branch instructions.  The value of the `PC` is transferred to one of the general purpose registers.  For the 12-bit constant offset modes, the `R6`/`L` is implied, while the other modes allow an arbitrary general purpose register to be chosen:

```
            MV0L        A, #18          ; Set A to 0x0012
            BRL         two_a_plus_one  ; copy the PC to R6/L, then set the PC to the subroutine save_value
              :
            
            ADR         R1, three_a     ; M <= address of subroutine three_a
            BRL         R1, R1          ; set the PC to the address in R1, overwriting with the PC for return
            
two_a_plus_one:
            ADD         A, A, A         ; A <= A + A = 2A
            ADD         A, #1           ; A <= A + 1
            MOV         PC, L           ; restore saved PC value in R6/L 

three_a:    PUSH        R1              ; Save the return address on the stack
            ADD         R1, A, A        ; R1 <= A + A = 2A
            ADD         A, A, R1        ; A <= A + R1 = A + 2A = 3A
            POP         R1              ; Restore return address from stack
            MOV         PC, R1          ; restore saved PC value in R1
```


#### Absolute mode

To redirect execution to an explicit address in memory, a 16-bit value is required.  Since instructions are 16-bit themselves, there is no way to embed an address in them.  Absolute branching is only possible using addresses present in general-purpose registers.

| Mnemonic          | Bit pattern        | Description                                     |
| :---------------- | :----------------- | :---------------------------------------------- |
| `B        Rx`     | `0100000000000XXX` | PC <= Rx                                        |
| `BL       Rx, Ry` | `011000000YYY0XXX` | Ry <= PC, PC <= Rx           (Rx = Ry is valid) |
| `B.<CC>   Rx`     | `0101SSS000000XXX` | If CC: PC <= Rx                                 |
| `BL.<CC>  Rx, Ry` | `0111SSS00YYY0XXX` | If CC: Ry <= PC, PC <= Rx    (Rx = Ry is valid) |

The `SSS` bits represent the condition code which, if satisfied, will cause the instruction to be executed.  The `XXX` and `YYY` are register indices.

##### Pseudo instructions

Since the `PC` starts at `$0000` at boot, basic bootstrapping code is expected to be present at that address.  The `R0`/`Z` register allows for a straightforward instruction that reenters that bootstrapping code:

```
B       R0
```

Since `R0`/`Z` is always zero, the `PC` is set to `$0000` by this instruction.  The ISA includes a pseudo-instruction for this:

| Mnemonic  | Actual code | Description     |
| :-------- | :---------- | :-------------- |
| `RST`     | `B    R0`   | PC <= `$0000`   |

Augmenting the `RET` pseudo-instruction from the Data movement category, a conditional return is available:

| Mnemonic      | Actual code   | Description            |
| :------------ | :------------ | :--------------------- |
| `RET.<CC>`    | `B.<CC> L`    | If CC: PC <= R6/L      |
| `RET.<CC> Rx` | `B.<CC> Rx`   | If CC: PC <= Rx        |


#### Relative mode

In relative addressing mode, a two's complement value is added to the `PC` register.  The value can be sourced from a general-purpose register or using constants embedded in the instruction.

Since instructions are 16-bit and the ISA demands 16-bit alignment for code, the embedded constant is multiplied by two before being added to the `PC`.  Thus, an 11-bit constant is actually a 12-bit offset, and an 8-bit constant is a 9-bit offset.

| Mnemonic            | Bit pattern        | Description                                        |
| :------------------ | :----------------- | :------------------------------------------------- |
| `BR       Rx`       | `0100000000001XXX` | PC <= PC + Rx                                      |
| `BRL      Rx, Ry`   | `011000000YYY1XXX` | Ry <= PC, PC <= PC + Rx         (Rx = Ry is valid) |
| `BR.<CC>  Rx`       | `0101SSS000001XXX` | If CC: PC <= PC + Rx                               |
| `BRL.<CC> Rx, Ry`   | `0111SSS00YYY1XXX` | If CC: Ry <= PC, PC <= PC + Rx  (Rx = Ry is valid) |
| `BR       <OFFSET>` | `01001CCCCCCCCCCC` | PC <= PC + (OFFSET << 1)               (11b const) |
| `BRL      <OFFSET>` | `01101CCCCCCCCCCC` | R6 <= PC, PC <= PC + (OFFSET << 1)     (11b const) |
| `BR.<CC>  <OFFSET>` | `0101SSS1CCCCCCCC` | PC <= PC + (OFFSET << 1)                (8b const) |
| `BRL.<CC> <OFFSET>` | `0111SSS1CCCCCCCC` | R6 <= PC, PC <= PC + (OFFSET << 1)      (8b const) |

