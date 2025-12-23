/* SPDX-License-Identifier: (MIT OR BSD-2-Clause OR ISC) */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "chip8.c"
#include "../include/chip8.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;


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
void updateScr(const sdl_t sdl, const emuConfig config, const chip8 chip8) {
    SDL_FRect rect = {.x = 0, .y = 0, .w = config.scale, .h = config.scale};

    /* Grab colors */

    const uint8_t fg_r = (config.fgColor >> 24) & 0xFF;
    const uint8_t fg_g = (config.fgColor >> 16) & 0xFF;
    const uint8_t fg_b = (config.fgColor >> 8) & 0xFF;
    const uint8_t fg_a = (config.fgColor >> 0) & 0xFF;
    /* Foreground */

    const uint8_t bg_r = (config.bgColor >> 24) & 0xFF;
    const uint8_t bg_g = (config.bgColor >> 16) & 0xFF;
    const uint8_t bg_b = (config.bgColor >> 8) & 0xFF;
    const uint8_t bg_a = (config.bgColor >> 0) & 0xFF;
    /* Background */

    /* Loop through pixels and draw a rectangle per pixel
     *    This may not be the best approach but it will be used until SDL is no longer needed */
    uint32_t i;
    for (i = 0; i < sizeof chip8.vram; i++) {
        /* Convert i value into XY coords */
        rect.x = (i % config.width) * config.scale;
        rect.y = (i / config.width) * config.scale;

        if (chip8.vram[i]) {
            /* Pixel is on = Draw FG */
            SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);

            if (config.fakeLcd) {
                /* Draw fake "scanlines" */
                SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
                SDL_RenderRect(sdl.renderer, &rect);
            }

        } else {
            /* Pixel is on = Draw FG */
            SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        }
    }
    SDL_RenderPresent(sdl.renderer);
}

/* Convert SDL_Keycode to emulator key */
unsigned char sdlHex(SDL_Keycode key) {
    switch (key) {
        case SDLK_1:
            /* Key 1 */
            return 0x01;
            break;

        case SDLK_2:
            /* Key 2 */
            return 0x02;
            break;

        case SDLK_3:
            /* Key 3 */
            return 0x03;
            break;

        case SDLK_4:
            /* Key C */
            return 0x0C;
            break;

        case SDLK_Q:
            /* Key 4 */
            return 0x04;
            break;

        case SDLK_W:
            /* Key 5 */
            return 0x05;
            break;

        case SDLK_E:
            /* Key 6 */
            return 0x06;
            break;

        case SDLK_R:
            /* Key D */
            return 0x0D;
            break;

        case SDLK_A:
            /* Key 7 */
            return 0x07;
            break;

        case SDLK_S:
            /* Key 8 */
            return 0x08;
            break;

        case SDLK_D:
            /* Key 9 */
            return 0x09;
            break;

        case SDLK_F:
            /* Key E */
            return 0x0E;
            break;

        case SDLK_Z:
            /* Key A */
            return 0x0A;
            break;

        case SDLK_X:
            /* Key 0 */
            return 0x00;
            break;

        case SDLK_C:
            /* Key B */
            return 0x0B;
            break;

        case SDLK_V:
            /* Key F */
            return 0x0F;
            break;

        default:
            /* Invalid */
            return 0x10;
            break;
    }
}

/* Handle event */
void event(chip8 *chip8) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        unsigned char keyhex;

        switch (event.type) {
            case SDL_EVENT_QUIT: /* Close window to quit emulator */
                quit = true;
                return;

            case SDL_EVENT_KEY_DOWN:
                switch (event.key.key) {
                    case SDLK_ESCAPE: /* Escape key quits emulator */
                        quit = true;
                        break;

                    case SDLK_SPACE:
                        /* Space bar */
                        if (!paused) {
                            paused = true; /* Pause */
                            puts("Emulation paused");
                            break;
                        }
                        else {
                            paused = false; /* Resume/unpause */
                            puts("Emulation resumed");
                            break;
                        }

                    keyhex = sdlHex(event.key.key);
                    if (keyhex != 0x10) {
                        chip8->keypad[keyhex] = keyDown;
                    }

                    break;

                    default:
                        break;
                }
                break;

            case SDL_EVENT_KEY_UP:
                keyhex = sdlHex(event.key.key);
                if (keyhex != 0x10) {
                    chip8->keypad[keyhex] = keyReleased;
                }
                break;
        }
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
        event(&chip8);

        if (paused) continue;

        /* Emulate instructions */
        execute(&chip8, &config);
        /* Set refresh rate to 60hz (16.67ms) */
        SDL_Delay(16);
        /* Clear screen */
        sdlClear(config, sdl);
        /* Update window */
        updateScr(sdl, config, chip8);
    }

    /* Cleanup */
    cleanup(&sdl);

    /* Test */
    printf("Emulator quit successfully.");
    exit(EXIT_SUCCESS);
}
