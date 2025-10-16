#ifndef MAIN_H
#define MAIN_H

#include <cstdint>

// CHIP-8 "emulator" object
typedef struct {
    uint8_t mem[4096]; // Memory
    bool vmem[2048]; // Video memory
    uint16_t stack[12]; // Call stack
    uint8_t V[16]; // 8-bit general registers V0 - VF
    uint16_t I; // 16-bit index register
    uint16_t PC; // 16-bit program counter - Stores current address
    uint8_t delayTimer; // 8-bit delay timer - Decrements at 60hz when non-zero
    uint8_t soundTimer; // 8-bit sound timer - Also decrements at 60hz but plays tone when non-zero
    bool keypad[16]; // Hexadecimal keypad with layout:      [1|2|3|C]
    char *rom; // .ch8 ROM that is running in the emulator   [4|5|6|D]
} chip8Obj;                                             //   [7|8|9|E]
                                                        //   [A|O|B|F]
#endif
