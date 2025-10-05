#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    SDL_Window *window;
} sdlWin;

typedef struct {
    uint32_t width; // Window width (Example: 640)
    uint32_t height; // Window height (Example: 480)
} emuConfig;

// Initialize SDL
bool initSdl(sdlWin *sdl, emuConfig config) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    sdl->window = SDL_CreateWindow(
        "SunChip v0.0", // Title
        config.width, config.height, // Resolution
        SDL_WINDOW_OPENGL // Renderer
    );

    if (!sdl->window) {
        SDL_Log("Could not create SDL window: %s\n", SDL_GetError());
        return -1;
    }

    return 0; // true
}

// Create window and renderer
// SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

// Setup config
bool configArgs(emuConfig *config, const int argc, char **argv) {
    // Set default config
    *config = (emuConfig){
        .width = 64, // CHIP-8 X resolution
        .height = 32, // CHIP-8 Y resolution
    };

    // Override config
    for (int i = 1; i < argc; i++) {
        (void)argv[i]; // Prevent error from unused arguments
    }

    return 1; // true
}

void cleanup(const sdlWin *sdl) {
    // SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}


// Main loop
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // Initialize config
    emuConfig config = {0, 0};
    if (!configArgs(&config, argc, argv)) exit(EXIT_FAILURE);

    // Initialize SDL
    sdlWin sdl = {0};
    if (!initSdl(&sdl, config)) exit(EXIT_FAILURE);

    printf("SunChip v0.0");

    // Cleanup
    cleanup(&sdl);

    exit(EXIT_SUCCESS);
}
