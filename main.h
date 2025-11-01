#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#define romLocation_default = 0x200;

/* CHIP-8 "emulator" object  */
typedef struct {
    uint8_t ram[4096]; /* Memory */
    bool vram[2048]; /* Video memory */
    uint16_t stack[12]; /* Call stack */
    uint8_t V[16]; /* 8-bit general registers V0 - VF */
    uint16_t I; /* 16-bit index register */
    uint16_t PC; /* 16-bit program counter - Stores current address */
    uint16_t SP; /* 16-bit stack pointer - Points to call stack */
    uint8_t delayTimer; /* 8-bit delay timer - Decrements at 60hz when non-zero */
    uint8_t soundTimer; /* 8-bit sound timer - Also decrements at 60hz but plays tone when non-zero */
    bool keypad[16]; /* Hexadecimal keypad with layout:            [1|2|3|C]                        */
    const char *rom; /* .ch8 ROM that is running in the emulator   [4|5|6|D]                        */
    uint32_t romLocation; /* Load CHIP-8 roms to 0x200 (512)                                        */
} chip8;             /*                                            [7|8|9|E]
                                                                   [A|O|B|F]                        */

/* Fetch and execute CHIP-8 instruction */
void execute(chip8 *chip8);

#endif
