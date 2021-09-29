#include "comic-ranz.h"

#include "SDL.h"
#include <stdbool.h>
#include <stdio.h>

#include "palette_sdl.h"

int main(int argc, char **argv) {
  const int WIDTH = 1280;
  const int HEIGHT = 1024;

  SDL_Window *window; // Declare a pointer

  SDL_Init(SDL_INIT_VIDEO); // Initialize SDL2

  Uint32 window_flags = 0;
  // Create an application window with the following settings:
  window = SDL_CreateWindow("Comic Ranz",            // window title
                            SDL_WINDOWPOS_UNDEFINED, // initial x position
                            SDL_WINDOWPOS_UNDEFINED, // initial y position
                            WIDTH,                   // width, in pixels
                            HEIGHT,                  // height, in pixels
                            window_flags             // flags - see below
  );

  // Check that the window was successfully created
  if (window == NULL) {
    // In the case that the window could not be made...
    printf("Could not create window: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    printf("Failed to create renderer\n");
    return 1;
  }
  SDL_Surface *surface =
      SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 256, 8, 0, 0, 0, 0);

  SDL_Color colors[64];
  uint8_t c;
  for (int i = 0; i < 32; i++) {
    c = (uint8_t)(palette[i] >> 8) & 0xf;
    colors[i].r = c | (c << 4);
    c = (uint8_t)(palette[i] >> 4) & 0xf;
    colors[i].g = c | (c << 4);
    c = (uint8_t)palette[i] & 0xf;
    colors[i].b = c | (c << 4);
  }
  // simulate EHB
  for (int i = 0; i < 32; i++) {
    c = (uint8_t)((palette[i] >> 8) & 0xf) >> 1;
    colors[i + 32].r = c | (c << 4);
    c = (uint8_t)((palette[i] >> 4) & 0xf) >> 1;
    colors[i + 32].g = c | (c << 4);
    c = (uint8_t)(palette[i] & 0xf) >> 1;
    colors[i + 32].b = c | (c << 4);
  }
  SDL_SetPaletteColors(surface->format->palette, colors, 0, 64);
  create_ranz(surface->pixels);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event) != 0) {
      switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          running = false;
        }
        break;
      case SDL_QUIT:
        running = false;
        break;
      }
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);

  // Close and destroy the window
  SDL_DestroyWindow(window);

  // Clean up
  SDL_Quit();
  return 0;
}