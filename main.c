#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

typedef struct {
    uint32_t width; /* Window width (Example: 640) */
    uint32_t height; /* Window height (Example: 480) */
    uint32_t fgColor; /* Foreground color (RGBA - 8888) */
    uint32_t bgColor; /* Background color (RGBA - 8888) */
    uint32_t scale; /* Scale pixels (Example: 10x 64 = 640) */
} emuConfig;

bool quit = false;
bool paused = false;

/* Initialize SDL */
bool initSdl(sdl_t *sdl, emuConfig config) {
    if (SDL_Init(SDL_INIT_VIDEO) & SDL_INIT_VIDEO) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    sdl->window = SDL_CreateWindow(
        "SunChip v0.0", /* Title */
        config.width * config.scale, config.height * config.scale, /* Resolution */
        SDL_WINDOW_OPENGL /* Renderer */
    );

    if (!sdl->window) {
        SDL_Log("Could not create SDL window: %s\n", SDL_GetError());
        return -1;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, NULL);
    if (!sdl->window) {
        SDL_Log("Could not create SDL renderer: %s\n", SDL_GetError());
        return -1;
    }

    return 1; /* true */
}

/* Setup config */
bool configArgs(emuConfig *config, const int argc, char **argv) {
    /* Set default config */
    *config = (emuConfig){
        .width = 64, /* CHIP-8 X resolution */
        .height = 32, /* CHIP-8 Y resolution */
        .fgColor = 0xFFFFFFFF, /* CHIP-8 Foreground color */
        .bgColor = 0x00000000, /* CHIP-8 Background color */
        .scale = 10 /* Default resolution: 640 x 320 */
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

/* void loadRom(chip8 *chip8 const char romFile[]) {} */

/* Initialize CHIP-8 "emulator" */
bool initEmu(chip8 *chip8, const char romFile[]) {
    const uint32_t romLocation = 0x200; /* Load CHIP-8 roms to 0x200 (512) */

    /* Load font */
    loadFont(chip8);

    /* Load ROM */
    FILE *rom = fopen(romFile, "rb");
    if (rom) {
        fseek(rom, 0, SEEK_END);
        const size_t romSize = ftell(rom);
        const size_t maxSize = sizeof chip8->ram - romLocation;
        rewind(rom);

        if (romSize > maxSize) {
            printf("Rom file %s size is %zu, which is bigger than the max size of %zu\n", romFile, romSize, maxSize);
            return -1;
        }

        /* Load ROM */
        size_t ramDump = fread(&chip8->ram[romLocation], romSize, 1, rom);

        if (ramDump != 1) {
            return -1;
        }

        fclose(rom);
    }
    else {
        printf("Invalid or missing rom file: %s\n", romFile);
        return -1;
    }

    /* Set defaults */
    chip8->PC = romLocation; /* Program counter starts at ROM location */
    chip8->rom = romFile;

    return 1; /* true */
}

void cleanup(const sdl_t *sdl) {
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

/* Clear screen */
void sdlClear(const emuConfig config, const sdl_t sdl) {
    const uint8_t r = (config.bgColor >> 24) & 0xFF;
    const uint8_t g = (config.bgColor >> 16) & 0xFF;
    const uint8_t b = (config.bgColor >> 8) & 0xFF;
    const uint8_t a = (config.bgColor >> 0) & 0xFF;

    SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
    SDL_RenderClear(sdl.renderer);
}

/* Update window */
void updateWin(const sdl_t sdl) {
    SDL_RenderPresent(sdl.renderer);
}

/* Handle event */
void event(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT: /* Close window to quit emulator */
                quit = true;
                return;

            case SDL_EVENT_KEY_DOWN:
                switch (event.key.key) {
                    case SDLK_ESCAPE: /* Escape key quits emulator */
                        quit = true;
                        return;

                    case SDLK_SPACE:
                        /* Space bar */
                        if (!paused) {
                            paused = true; /* Pause */
                            puts("Emulation paused");
                        }
                        else {
                            paused = false; /* Resume/unpause */
                            puts("Emulation resumed");
                        }


                        default:
                            break;
                    }
                    break;

            case SDL_EVENT_KEY_UP:
                break;
        }
    }
}

/* Fetch and execute CHIP-8 instruction */
void execute(chip8 *chip8) {
    /* Fetch next opcode */
    uint8_t b1 = chip8->ram[chip8->PC], /* NN = 8-bit constant */
            b2 = chip8->ram[chip8->PC + 1]; /* KK */

    /* Instructions: */
    uint8_t c = b1 >> 4; /* Decode */
    uint16_t NNN = ((b1 & 0xF) << 8) | b2; /* Address */
    uint8_t N = b2 & 0xF; /* 4-bit constant */
    uint8_t X = b1 & 0xF; /* 4-bit register identifier */
    uint8_t Y = b2 >> 4; /* 4-bit register identifier */

    chip8->PC += 2; /* Move PC to next opcode */

    /* Execute opcode */
    switch (c) {
        case 0x00:
            switch (b2) {

            case 0x00: /* HALT - 0000 */
                chip8->PC -= 2;

            case 0xE0: /* CLS - 00E0 */
                memset(&chip8->vram[0], false, sizeof chip8->vram);
                break;

            case 0xEE: /* RET - 00EE */
                chip8->PC = --chip8->SP;
            }

            case 0x02: /* CALL address -  2NNN */
                chip8->SP = chip8->PC;
                chip8->PC = NNN;
                break;

        default:
            break; /* Illegal opcode */
    }
}

/* Main loop */
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s [.ch8 file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Initialize config */
    emuConfig config = {};
    if (!configArgs(&config, argc, argv)) exit(EXIT_FAILURE);

    /* Initialize SDL */
    sdl_t sdl = {};
    if (!initSdl(&sdl, config)) exit(EXIT_FAILURE);

    chip8 chip8 = {};
    const char *rom = argv[1];
    if (!initEmu(&chip8, rom)) exit(EXIT_FAILURE);

    /* Emulator loop */
    while (!quit) {
        /* Handle event */
        event();

        if (paused) continue;

        /* Emulate instructions */
        execute(&chip8);
        /* Set refresh rate to 60hz (16.67ms) */
        SDL_Delay(16);
        /* Clear screen */
        sdlClear(config, sdl);
        /* Update window */
        updateWin(sdl);
    }

    /* Cleanup */
    cleanup(&sdl);

    /* Test */
    printf("Emulator quit successfully.");
    exit(EXIT_SUCCESS);
}
