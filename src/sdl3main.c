/* SPDX-License-Identifier: (Unlicense OR CC0-1.0 OR WTFPL OR MIT-0 OR 0BSD) */
#include "chip8.c"
#include "../include/chip8.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

/* Emulator */
int soundByte = 0;


typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;


/* Initialize SDL */
bool initSdl(sdl_t *sdl) {
    if (SDL_Init(SDL_INIT_VIDEO) & SDL_INIT_VIDEO) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    sdl->window = SDL_CreateWindow(
        "SunChip v0.2", /* Title */
        displayWidth * defaultScale, displayHeight * defaultScale, /* Resolution */
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
void sdlClear(const sdl_t sdl) {
    const uint8_t r = (defaultBgColor >> 24) & 0xFF;
    const uint8_t g = (defaultBgColor >> 16) & 0xFF;
    const uint8_t b = (defaultBgColor >> 8) & 0xFF;
    const uint8_t a = (defaultBgColor >> 0) & 0xFF;

    SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
    SDL_RenderClear(sdl.renderer);
}

/* Update window */
void updateScr(const sdl_t sdl, const chip8 chip8) {
    SDL_FRect rect = {.x = 0, .y = 0, .w = defaultScale, .h = defaultScale};

    /* Grab colors */

    const uint8_t fg_r = (defaultFgColor >> 24) & 0xFF;
    const uint8_t fg_g = (defaultFgColor >> 16) & 0xFF;
    const uint8_t fg_b = (defaultFgColor >> 8) & 0xFF;
    const uint8_t fg_a = (defaultFgColor >> 0) & 0xFF;
    /* Foreground */

    const uint8_t bg_r = (defaultBgColor >> 24) & 0xFF;
    const uint8_t bg_g = (defaultBgColor >> 16) & 0xFF;
    const uint8_t bg_b = (defaultBgColor >> 8) & 0xFF;
    const uint8_t bg_a = (defaultBgColor >> 0) & 0xFF;
    /* Background */

    /* Loop through pixels and draw a rectangle per pixel
     *    This may not be the best approach but it will be used until SDL is no longer needed */
    uint32_t i;
    for (i = 0; i < sizeof chip8.vram; i++) {
        /* Convert i value into XY coords */
        rect.x = (i % displayWidth) * defaultScale;
        rect.y = (i / displayWidth) * defaultScale;

        if (chip8.vram[i]) {
            /* Pixel is on = Draw FG */
            SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);

            if (chip8.fakeLcd) {
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

void squareWaveCallback(void *sendData, SDL_AudioStream *buffer, int bytes, int totalBytes) {
    (void)sendData;

    /* chip8 chip8 = {}; */
    (void)totalBytes;

    bytes /= sizeof (float);
    while (bytes > 0) {
        float samples[2];
        const int total = SDL_min(bytes, SDL_arraysize(samples));
        int i;

        for (i = 0; i < bytes; i++) {
            const int freq = 440;
            const float phase = soundByte * freq / 8000.0f;
            samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
            /* Get byte of buffer from next sample
            buffer[i] = chip8.ram[audioStartDefault + (soundByte / 8)];

            Get sample bit
            buffer[i] <<= (soundByte % 8);
            buffer[i] &= 0x80;

            If sample is 1, set buffer to max volume
            buffer[i] *= 0xFF;

            Keep track of sound buffer */
            soundByte++;
            if (soundByte >= (audioSize * 8)) {
                soundByte = 0;
            }
        }

        soundByte %= 8000;

        /* new data */
        SDL_PutAudioStreamData(buffer, samples, total * sizeof (float));
        bytes -= total; /* subtract stream */
    }
}

void playSound() {
    soundByte = 0;

    SDL_AudioSpec spec = {SDL_AUDIO_U8, 1, 0};
    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, squareWaveCallback, NULL);
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(stream));

    SDL_PauseAudioDevice(1);
}

void stopSound() {
    SDL_PauseAudioDevice(1);
    SDL_CloseAudioDevice(1);
}

void soundEvent() {
    chip8 chip8 = {};
    if (chip8.beep) {
        playSound();
    }
    else if (!chip8.beep) {
        stopSound();
    }
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

                    default:
                        keyhex = sdlHex(event.key.key);
                        if (keyhex != 0x10) {
                            chip8->keypad[keyhex] = keyDown;
                        }

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

    /* Initialize SDL */
    sdl_t sdl = {};
    if (!initSdl(&sdl)) exit(EXIT_FAILURE);

    printf("*```(`UN``````*\n");

    chip8 chip8 = {};
    const char *rom = argv[1];
    if (!initEmu(&chip8, rom)) exit(EXIT_FAILURE);

    printf("*...,)CHIP.v0.2*\n");

    /* Emulator loop */
    while (!quit) {
        /* Handle event */
        event(&chip8);

        if (paused) continue;

        /* Emulate instructions */
        cycle(&chip8);
        /* Set refresh rate to 60hz (16.67ms) */
        SDL_Delay(16);
        /* Clear screen */
        sdlClear(sdl);
        /* Update window */
        updateScr(sdl, chip8);
    }

    /* Cleanup */
    cleanup(&sdl);

    /* Test */
    printf("Emulator quit successfully.");
    exit(EXIT_SUCCESS);
}
