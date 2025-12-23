/* SPDX-License-Identifier: (MIT OR BSD-2-Clause OR ISC) */
#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define maxRam 65536
#define pcStartDefault 0x200
#define defaultKeys 16
#define defaultQuirks 10

/* Key (or button) states */
typedef enum {
    keyUp,
    keyDown,
    keyReleased
} EMUKEYS;

/* CHIP-8 "emulator" object  */
typedef struct {
    uint8_t ram[maxRam]; /* Memory */
    bool vram[2048]; /* Video memory */
    uint16_t stack[12]; /* Call stack */
    uint8_t V[16]; /* 8-bit general registers V0 - VF */
    uint16_t I; /* 16-bit index register */
    uint16_t PC; /* 16-bit program counter - Stores current address */
    uint16_t SP; /* 16-bit stack pointer - Points to call stack */
    uint8_t delayTimer; /* 8-bit delay timer - Decrements at 60hz when non-zero */
    uint8_t soundTimer; /* 8-bit sound timer - Also decrements at 60hz but plays tone when non-zero */
    EMUKEYS keypad[defaultKeys]; /* Hexadecimal keypad with layout:            [1|2|3|C]                        */
    const char *rom; /* .ch8 ROM that is running in the emulator   [4|5|6|D]                        */
    uint16_t pcStart; /* Load CHIP-8 roms to 0x200 (512)           [7|8|9|E]                        */
    bool quirks[defaultQuirks];/*                                  [A|O|B|F]                        */
    bool beep; /* Produce sound */
    bool exit; /* Exit the interpreter */
} chip8;

#endif
