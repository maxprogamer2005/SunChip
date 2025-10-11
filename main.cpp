#include <SDL3/SDL_events.h>
#include <cstdlib>
// #include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

typedef struct {
    uint32_t width; // Window width (Example: 640)
    uint32_t height; // Window height (Example: 480)
    uint32_t fgColor; // Foreground color (RGBA - 8888)
    uint32_t bgColor; // Background color (RGBA - 8888)
    uint32_t scale; // Scale pixels (Example: 10x 64 = 640)
} emuConfig;

bool quit = false;
bool paused = false;

// Initialize SDL
bool initSdl(sdl_t *sdl, emuConfig config) {
    if (SDL_Init(SDL_INIT_VIDEO) & SDL_INIT_VIDEO) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    sdl->window = SDL_CreateWindow(
        "SunChip v0.0", // Title
        config.width * config.scale, config.height * config.scale, // Resolution
        SDL_WINDOW_OPENGL // Renderer
    );

    if (!sdl->window) {
        SDL_Log("Could not create SDL window: %s\n", SDL_GetError());
        return -1;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, nullptr);
    if (!sdl->window) {
        SDL_Log("Could not create SDL renderer: %s\n", SDL_GetError());
        return -1;
    }

    return 1; // true
}

// Setup config
bool configArgs(emuConfig *config, const int argc, char **argv) {
    // Set default config
    *config = (emuConfig){
        .width = 64, // CHIP-8 X resolution
        .height = 32, // CHIP-8 Y resolution
        .fgColor = 0xFFFFFFFF, // CHIP-8 Foreground color
        .bgColor = 0x00000000, // CHIP-8 Background color
        .scale = 10 // Default resolution: 640 x 320
    };

    // Override config
    for (int i = 1; i < argc; i++) {
        (void)argv[i]; // Prevent error from unused arguments
    }

    return 1; // true
}

void cleanup(const sdl_t *sdl) {
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

// Clear screen
void sdlClear(const emuConfig config, const sdl_t sdl) {
    const uint8_t r = (config.bgColor >> 24) & 0xFF;
    const uint8_t g = (config.bgColor >> 16) & 0xFF;
    const uint8_t b = (config.bgColor >> 8) & 0xFF;
    const uint8_t a = (config.bgColor >> 0) & 0xFF;

    SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
    SDL_RenderClear(sdl.renderer);
}

// Update window
void updateWin(const sdl_t sdl) {
    SDL_RenderPresent(sdl.renderer);
}

// Handle event
void event(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT: // X Button - Quit emulator
                quit = true;
                return;

            case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key) {
                        case SDLK_ESCAPE: // Escape key - Quit emulator
                            quit = true;
                            return;
                    }
        }
    }
}

// Main loop
int main(int argc, char **argv) {
    // Initialize config
    emuConfig config = {0, 0, 0, 0, 0};
    if (!configArgs(&config, argc, argv)) exit(EXIT_FAILURE);

    // Initialize SDL
    sdl_t sdl = {0, 0};
    if (!initSdl(&sdl, config)) exit(EXIT_FAILURE);

    // Emulator loop
    while (!quit) {
        // Handle input
        event();

        // Emulate instructions

        // Set refresh rate to 60hz (16.67ms)
        SDL_Delay(16);
        // Clear screen
        sdlClear(config, sdl);
        // Update window
        updateWin(sdl);
    }

    // Cleanup
    cleanup(&sdl);

    exit(EXIT_SUCCESS);
}
