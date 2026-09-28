#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game.h"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static GameContext* game;
static Uint64 lastFrame;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Bullet Heaven Demo", "0.1", "bullet-heaven-demo");
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60");
    lastFrame = SDL_GetTicksNS();
    game = Game_Init();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Bullet Heaven Demo", game->pixels.width, game->pixels.height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, game->pixels.width, game->pixels.height);
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

static void iterateGamePlaying(const bool *keyboardState) {
    if (keyboardState[SDL_SCANCODE_ESCAPE]) {
        game->playingState = GAME_PLAYING_STATE_PAUSED;
        return;
    }

    int directionX = 0, directionY = 0;
    if (keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A]) {
        directionX = -1;
    };
    if (keyboardState[SDL_SCANCODE_RIGHT] || keyboardState[SDL_SCANCODE_D]) {
        directionX = 1;
    };

    Game_SteerShip(directionX, directionY);
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    const Uint64 now_ns = SDL_GetTicksNS();

    const bool *keyboardState = SDL_GetKeyboardState(NULL);

    switch (game->playingState) {
        case GAME_PLAYING_STATE_PAUSED:
        case GAME_PLAYING_STATE_MENU:
            if (keyboardState[SDL_SCANCODE_SPACE])
                game->playingState = GAME_PLAYING_STATE_PLAYING;
           break;
        case GAME_PLAYING_STATE_PLAYING:
            iterateGamePlaying(keyboardState);
            break;
    }


    Game_Update(now_ns - lastFrame);
    lastFrame = now_ns;
    /* clear the window to the draw color. */
    SDL_UpdateTexture(
        texture,
        NULL,
        game->pixels.data,
        game->pixels.width * sizeof(uint32_t)
    );

    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    Game_Free();
    /* SDL will clean up the window/renderer for us. */
}
