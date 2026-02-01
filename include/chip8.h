/* SPDX-License-Identifier: (Unlicense OR CC0-1.0 OR WTFPL OR MIT-0 OR 0BSD) */
#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define maxRam 65536
#define pcStartDefault 0x200
#define spStartDefault 0xA0
#define audioSize 16
#define audioStartDefault 256
#define fontStartDefault 0x0
#define bigFontStartDefault 0x50
#define defaultSpeed 1000
#define earthSecond 1000000
#define displayWidth 64 /* CHIP-8 X resolution */
#define displayHeight 32 /* CHIP-8 Y resolution */
#define defaultFgColor 0x00131A00 /* CHIP-8 Foreground color */
#define defaultBgColor 0xF9FFB3FF /* CHIP-8 Background color */
#define defaultScale 10 /* Default resolution: 640 x 320 */
#define defaultKeys 16
#define defaultQuirks 10
#define defaultPitch 64

/* Key (or button) states */
typedef enum {
    keyUp,
    keyDown,
    keyReleased
} EMUKEYS;

typedef enum {
    bmNone,
    bm1,
    bm2,
    bmBoth
} EMUBM;

/* CHIP-8 "emulator" object  */
typedef struct {
    uint8_t ram[maxRam]; /* Memory */
    bool vram[displayWidth * displayHeight]; /* Video memory */
    bool vram2[displayWidth * displayHeight]; /* Second video memory */
    EMUBM bitMask; /* Display bitmask */
    uint16_t stack[12]; /* Call stack */
    uint8_t V[16]; /* 8-bit general registers V0 - VF */
    uint16_t I; /* 16-bit index register */
    uint16_t PC; /* 16-bit program counter - Stores current address */
    uint16_t SP; /* 16-bit stack pointer - Points to call stack */
    uint8_t delayTimer; /* 8-bit delay timer - Decrements at 60hz when non-zero */
    uint8_t soundTimer; /* 8-bit sound timer - Also decrements at 60hz but plays tone when non-zero */
    uint8_t pitch; /* 8-bit audio pitch register */
    EMUKEYS keypad[defaultKeys]; /* Hexadecimal keypad with layout:            [1|2|3|C] */
    /* [4|5|6|D] */
    /* Speed and timers                                                        [7|8|9|E] */
    unsigned long cpuHz;                                                    /* [A|0|B|F] */
    unsigned long timerHz;
    unsigned long refreshHz;
    long timerMaxCycles;
    long cpuMaxCycles;
    long cpuCycles;
    long soundCycles;
    long delayCycles;
    long refreshMaxCycles;
    long refreshCycles;
    long cycleTime;

    const char *rom; /* .ch8 ROM that is running in the emulator */
    uint16_t pcStart; /* Load CHIP-8 roms to 0x200 (512) */
    bool quirks[defaultQuirks];
    bool beep; /* Produce sound */
    bool fakeLcd; /* Simulate LCD */
    bool exit; /* Exit the interpreter */
} chip8;

#endif
