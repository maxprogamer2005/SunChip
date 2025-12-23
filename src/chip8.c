/* SPDX-License-Identifier: (MIT OR BSD-2-Clause OR ISC)
***************
*```(`UN``````*
*   ,)CHIP    *
*~~~~~~~~~~~~~*
*   Chip-8    *
* Interpreter *
* Made in C89 *
*~~~~~~~~~~~~~*
*   By max    *
* Version 1.0 *
*_____________*
**************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "../include/chip8.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    uint32_t width; /* Window width (Example: 640) */
    uint32_t height; /* Window height (Example: 480) */
    uint32_t fgColor; /* Foreground color (RGBA - 8888) */
    uint32_t bgColor; /* Background color (RGBA - 8888) */
    uint32_t scale; /* Scale pixels (Example: 10x 64 = 640) */
    bool fakeLcd; /* Simulate LCD */
} emuConfig;

bool quit = false;
bool paused = false;
bool quirks[defaultQuirks] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

/* Setup config */
bool configArgs(emuConfig *config, const int argc, char **argv) {
    /* Set default config */
    *config = (emuConfig) {
        .width = 64, /* CHIP-8 X resolution */
        .height = 32, /* CHIP-8 Y resolution */
        .fgColor = 0xFFFFFFFF, /* CHIP-8 Foreground color */
        .bgColor = 0x00000000, /* CHIP-8 Background color */
        .scale = 10, /* Default resolution: 640 x 320 */
        .fakeLcd = false /* Simulate LCD */
    };

    /* Override config */
    int i = 1;
    for (i = 1; i < argc; i++) {
        (void)argv[i]; /* Prevent error from unused arguments */
    }

    return 1; /* true */
}

void loadFont(chip8 *chip8) {
    const uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
        0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
        0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
        0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
        0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
        0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
        0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
        0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
        0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
        0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
        0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
        0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
        0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
        0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
        0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
        0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
    };
    memcpy(&chip8->ram[0], font, sizeof(font));
};

void draw(chip8 *chip8, emuConfig *config, uint8_t x, uint8_t y, uint8_t N) {
    const uint8_t xStart = x; /* Original X */

    chip8->V[0x0F] = 0; /* Initialize carry flag to 0 */

    uint8_t i; int8_t j; /* Please clean this up. */

    for (i = 0; i < N; i++) {
        const uint8_t spriteData = chip8->ram[chip8->I + i];
        x = xStart; /* Reset X for next row */

        for (j = 7; j >= 0; j--) {
            /* If sprite pixel and display pixel are on, set carry flag */
            bool *pixel = &chip8->vram[y * config->width + x];
            const bool spriteBit = (spriteData & (1 << j));

            if (spriteBit && *pixel) {
                chip8->V[0x0F] = 1;
            }

            /* XOR display sprite pixel on or off */
            *pixel ^= spriteBit;

            /* Stop drawing if the screen edge is hit*/
            if (++x >= config->width) break;
        }

        /* Stop drawing screen if the bottom edge is hit */
        if (++y >= config->height) break;
    }
}

void loadRom(chip8 *chip8, const char romFile[]) {
    /* Load ROM */
    FILE *rom = fopen(romFile, "rb");
    if (rom) {
        fseek(rom, 0, SEEK_END);
        const size_t romSize = ftell(rom);
        const size_t maxSize = sizeof chip8->ram - pcStartDefault;
        rewind(rom);

        if (romSize > maxSize) {
            printf("Rom file '%s' is %zu bytes too large!\n", romFile, romSize - maxSize);
            return (void)-1;
        }

        /* Load ROM */
        size_t ramDump = fread(&chip8->ram[pcStartDefault], romSize, 1, rom);

        if (ramDump != 1) {
            return (void)-1;
        }

        fclose(rom);
    }
    else {
        printf("Invalid or missing rom file: %s\n", romFile);
    }

    /* Set defaults */
    chip8->rom = romFile;
}

void keyWait (chip8 *chip8, uint8_t x) {
    bool releaseKey = false;

    int i = 0;

    for (i = 0; i < defaultKeys; i++) {
        if (chip8->keypad[i] == keyReleased) {
            /* Release key */
            chip8->V[x] = i;
            releaseKey = true;
            break;
        }
    }

    if (!releaseKey) {
        chip8-> PC -= 2;
    }
}

void resetKeypad(chip8 *chip8) {
    int k;
    for (k = 0; k < defaultKeys; k++) {
        chip8->keypad[k] = keyUp;
    }
}

void resetReleased(chip8 *chip8) {
    int k;
    for (k = 0; k < defaultKeys; k++) {
        if (chip8->keypad[k] == keyReleased) {
            chip8->keypad[k] = keyUp;
        }
    }
}

void reset(chip8 *chip8){
    chip8->PC = pcStartDefault;

    resetKeypad(chip8);
}

void updateTimers(chip8 *chip8) {

    /* Delay timer */
    if (chip8->delayTimer > 0) {
        chip8->delayTimer--; /* Add accuracy features later */
    }
    /* Sound timer */
    if (chip8->soundTimer > 0) {
        chip8->beep = true;
        chip8->soundTimer--;
    }
    else {
        chip8->beep = false;
    }
}


void skipInstr(chip8 *chip8) {
    if (chip8->ram[chip8->PC] == 0xF0 && chip8->ram[chip8->PC + 1 == 0x00]) {
        chip8->PC += 4;
    }
    else {
        chip8->PC += 2;
    }
}

/* Initialize CHIP-8 "emulator" */
bool initEmu(chip8 *chip8, const char romFile[]) {

    /* Reset emulator */
    reset(chip8);

    /* Load font */
    loadFont(chip8);

    /* Load rom */
    loadRom(chip8, romFile);

    /* Emulate instructions */
    return 1; /* true */
}

/* Fetch and execute CHIP-8 instruction */
void execute(chip8 *chip8, emuConfig *config) {
    /* Fetch next opcode */
    uint8_t b1 = chip8->ram[chip8->PC], /* NN = 8-bit constant */
            b2 = chip8->ram[chip8->PC + 1]; /* NN */

    /* Instructions: */
    uint8_t c = b1 >> 4; /* Decode - first 8 bits of instruction */
    uint16_t NNN = ((b1 & 0xF) << 8) | b2; /* Address */
    uint8_t N = b2 & 0xF; /* 4-bit constant */
    uint8_t x = b1 & 0xF; /* 4-bit register identifier */
    uint8_t y = b2 >> 4; /* 4-bit register identifier */
    uint8_t NN = b2; /* Last 8 bits of instruction */

    skipInstr(chip8); /* Move PC to next opcode */

    /* Execute opcode */
    switch (c) {
        case 0x00:
            switch (b2) {

            case 0x00: /* HALT the emulator - 0000 */
                chip8->PC -= 2;
                break;

            case 0xE0: /* Clear the display - 00E0 */
                memset(&chip8->vram[0], false, sizeof chip8->vram);
                break;

            case 0xEE: /* RET (urn) from address - 00EE */
                chip8->PC = (chip8->ram[chip8->SP] << 8);
                chip8->PC |= chip8->ram[chip8->SP + 1];
                chip8->SP -= 2;
                break;

            case 0xFD: /* EXIT - 00FD: S-CHIP only */
                quit = true;
                break;
            }
            break;

        case 0x01: /* JP address - 1NNN */
            chip8->PC = NNN;
            break;

        case 0x02: /* CALL address -  2NNN */
            chip8->SP += 2;
            chip8->ram[chip8->SP] = chip8->PC >> 8;
            chip8->ram[chip8->SP + 1] = chip8->PC & 0x00FF;
            chip8->PC = NNN;
            break;

        case 0x03: /* If Vx == NN, skip the next instruction - 3NNN */
            if (chip8->V[x] == NN) {
                skipInstr(chip8);
            }
            break;

        case 0x04: /* If Vx != NN, skip the next instruction - 4NNN */
            if (chip8->V[x] != NN) {
                skipInstr(chip8);
            }
            break;

        case 0x05:
            switch (N) {
                case 0x00: /* If Vx != NN, skip the next instruction - 5xy0 */
                    if (chip8->V[x] == chip8->V[y]) {
                        skipInstr(chip8);
                        break;
                    }
                }
            break;

        case 0x06: /* Set index register Vx to address NN - 6xNN */
            chip8->V[x] = NN;
            break;

        case 0x07: /* Adds index register Vx to address NN - 7xNN */
            chip8->V[x] += NN;
            break;

        case 0x08:
            switch (N) {
                case 0x00: /* Set index register Vx = Vy - 8xy0 */
                    chip8->V[x] = chip8->V[y];
                    break;

                case 0x01: /* Set index register Vx to Vx or Vy - 8xy1 */
                    chip8->V[x] |= chip8->V[y];
                    break;

                case 0x02: /* Set index register Vx and Vy - 8xy2 */
                    chip8->V[x] &= chip8->V[y];
                    break;

                case 0x03: /* Set index register Vx xor Vy - 8xy3 */
                    chip8->V[x] ^= chip8->V[y];
                    break;

                case 0x04: { /* Adds index register Vx from Vy and sets VF to 1 if carry - 8xy4 */
                    bool carry = ((chip8->V[x] + chip8->V[y]) > 0xFF);
                    chip8->V[x] += chip8->V[y];
                    chip8->V[0x0F] = carry;
                    break;
                }

                case 0x05: { /* Subtracts index register Vx from Vy and sets VF to 1 if there is no borrow - 8xy5 */
                    bool noBorrow = (chip8->V[x] >= chip8->V[y]);
                    chip8->V[x] = chip8->V[x] - chip8->V[y];
                    chip8->V[0x0F] = noBorrow;
                    break;
                }

                case 0x06: { /* Set index register Vx xor Vy - 8xy6 */
                    int carry = chip8->V[x] & 0x01;
                    chip8->V[x] >>= 1;
                    chip8->V[0x0F] = carry;
                    break;
                }

                case 0x07: { /* Adds index register Vx to Vy - 8xy7 */
                    bool noBorrow = (chip8->V[y] >= chip8->V[x]);
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    chip8->V[0x0F] = noBorrow;
                    break;
                }

                case 0x0E: { /* Subtracts index register Vx from Vy - 8xyE */
                    int carry = (chip8->V[x] & 0x80) >> 7;
                    chip8->V[x] <<= 1;
                    chip8->V[0x0F] = carry;
                    break;
                }
            }
            break;

        case 0x09: /* If Vx != Vy, skip the next instruction - 9xy0 */
            if (chip8->V[x] != chip8->V[y]) {
                skipInstr(chip8);
                break;
            }
            break;

        case 0x0A: /* Set index register I to address NNN - ANNN */
            chip8->I = NNN;
            break;

        case 0x0B: /* C8: Jump to address NNN + V0, SC: Jump to address NNN + Vx */
            chip8->PC = NNN + chip8->V[0];
            break;

        case 0x0C: /* Set Vx to random byte and NN - CxNN  */
            chip8->V[x] = (rand() % 0x1000) & NN;
            break;

        case 0x0D: /* Display N-byte sprite at coordinates (Vx, Vy), set VF = collision - DxyN */
            draw(chip8, config, chip8->V[x] % config->width, chip8->V[y] % config->height, N);
            break;

        case 0x0E:
            switch (b2) {
                case 0x9E:
                    if (chip8->keypad[chip8->V[x]] == keyDown) {
                        skipInstr(chip8);
                    }
                    break;

                case 0xA1:
                    if (chip8->keypad[chip8->V[x]] == keyUp) {
                        skipInstr(chip8);
                    }
                    break;
            }
            break;

        case 0x0F:
            switch (b2) {
                case 0x07: /* Set Vx = delay timer */
                    chip8->V[x] = chip8->delayTimer;
                    break;

                case 0x0A:
                    keyWait(chip8, x);
                    break;

                case 0x15:
                    chip8->delayTimer = chip8->V[x];
                    break;

                case 0x18:
                    chip8->soundTimer = chip8->V[x];
                    break;

                case 0x1E:
                    chip8->I += chip8->V[x];
                    break;

                case 0x33: /* Store Vx in locations I, I + 1, and I + 2 - Fx33 */
                    chip8->ram[chip8->I]     = (chip8->V[x] / 100) % 10;
                    chip8->ram[chip8->I + 1] = (chip8->V[x] / 10) % 10;
                    chip8->ram[chip8->I + 2] =  chip8->V[x] % 10;
                    break;

                case 0x55: { /* Store registers V0 through Vx in memory starting at location I - Fx55 */
                    int r = 0;
                    for (r = 0; r <=x; r++) {
                        chip8->ram[chip8->I + r] = chip8->V[r];
                    }
                    break;
                }

                case 0x65: { /* Read registers V0 through Vx from memory starting at location I - Fx65 */
                    int r = 0;
                    for (r = 0; r <=x; r++) {
                        chip8->V[r] = chip8->ram[chip8->I + r];
                    }
                    break;
                }
            }
            break;

        /* Reset keys that were released in the previous frame */
        resetKeypad(chip8);
        break;

        default: /* Illegal opcode */
            printf(" Illegal opcode: %X", b1);
            printf(" Illegal opcode: %X", b2);
            puts(""); /* Prevent duplicate printing */
            break;
    }
    printf(" 0x%02X", b1);
    fflush(stdout);
    printf(" 0x%02X\n", b2);
}
