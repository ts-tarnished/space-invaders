#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "game.h"
#include "ts_sw_graphics.h"

static GameContext game;
static int projectileTimer = 0;
static int spawnTimer = 0;
static void drawBackground() { TSG_Clear(&game.pixels, 0xFFFF0000); }

static TSG_Rectangle gameColliderToTSGRect(GameCollider c) {
    return (TSG_Rectangle) {
        .x = c.x,
        .y = c.y,
        .width = c.width,
        .height = c.height
    };
}

static GameEntity* addEntity(GameEntity ent, enum GameEntType entType) {
    for(int i = 0; i < GAME_ENTITY_CAPACITY; i++) {
        GameEntity *e = &game.entities[i];
        if (e->active)
            continue;
        e->id = i;
        e->active = true;
        e->collider = ent.collider;
        e->directionX = ent.directionX;
        e->directionY = ent.directionY;
        e->velocity = ent.velocity;
        e->projectileOffset = ent.projectileOffset;
        e->entType = entType;
        return e;
    }

    fprintf(stderr, "Error: Entity Capacity exceeded.\n");
    exit(1);
}

static void shootProjectile(GameEntity *entity) {
    int projectileWidth = 10, projectileHeight = 10;
    addEntity((GameEntity) {
        .velocity = 100,
        .directionX = 0,
        .directionY = -1,
        .collider = {
            .x = (float)entity->collider.x + entity->collider.width / 2 - (float)projectileWidth / 2,
            .y = entity->collider.y - projectileHeight - entity->projectileOffset,
            .width = projectileWidth,
            .height = projectileHeight
        }
    }, GAME_ENT_PROJECTILE);
}

static void drawProjectile(GameEntity *ent) {
    TSG_DrawRectangleRec(&game.pixels,
        gameColliderToTSGRect(ent->collider),
        0);
}

static void drawEnemy(GameEntity *ent) {
    TSG_DrawRectangleRec(&game.pixels,
        gameColliderToTSGRect(ent->collider),
        0);
}

static void drawShip(GameEntity *ent) {
    int overlap = 10;
    TSG_DrawTriangle(&game.pixels,
        ent->collider.x + ent->collider.width / 2, ent->collider.y - overlap,
        ent->collider.x - overlap, ent->collider.y + ent->collider.height,
        ent->collider.x + ent->collider.width + overlap, ent->collider.y + ent->collider.height,
        0);
}

static void updateEnt(GameEntity *e, int dt) {
    if (!e->active)
        return;
    e->collider.y += e->directionY * dt * pow(10,-9) * e->velocity;
    e->collider.x += e->directionX * dt * pow(10,-9) * e->velocity;
}

static void spawnEnemy() {
    int width = 60, height = 10;
    addEntity((GameEntity) {
        .directionX = 0,
        .directionY = 1,
        .velocity = 10,
        .collider = (GameCollider) {
            .x = rand() % (int)(GAME_WIDTH - width),
            .y = 0,
            .width = width,
            .height = height
        }
    }, GAME_ENT_ENEMY);
}

static void updateGamePlaying(int dt) {
    srand(time(NULL));
    for (int i = 0; i < GAME_ENTITY_CAPACITY; i++) {
        GameEntity *e = &game.entities[i];
        if (!e->active)
            continue;

        updateEnt(e, dt);
    }

    projectileTimer -= dt;
    spawnTimer -= dt;
    if (projectileTimer <= 0) {
        shootProjectile(game.ship);
        projectileTimer = 400 * pow(10, 6);
    }

    if (spawnTimer <= 0) {
        spawnEnemy();
        spawnTimer = 1000 * pow(10, 6);
    }

    drawBackground();

    for (int i = 0; i < GAME_ENTITY_CAPACITY; i ++) {
        GameEntity *e = &game.entities[i];
        if (!e->active)
            continue;
        switch (e->entType) {
                case GAME_ENT_SHIP:
                    drawShip(e);
                    break;
                case GAME_ENT_ENEMY:
                    drawEnemy(e);
                    break;
                case GAME_ENT_PROJECTILE:
                    drawProjectile(e);
                    break;
        }
    }
}

static void updateGameMenu(int dt) {
    char text[] = "Press SPACE to start.";
    TSG_Clear(&game.pixels, 0xFFFF0000);
    TSG_DrawText(&game.pixels, GAME_WIDTH / 2 - strlen(text)*8 / 2, GAME_HEIGHT / 2 - 8, text, 0);
}

static void updateGamePaused(int dt) {
    char text[] = "Press SPACE to continue.";
    char text2[] = "Press R to restart.";
    TSG_Clear(&game.pixels, 0xFFFF0000);
    TSG_DrawText(&game.pixels, GAME_WIDTH / 2 - strlen(text)*8 / 2, GAME_HEIGHT / 2 - 16, text, 0);
    TSG_DrawText(&game.pixels, GAME_WIDTH / 2 - strlen(text2)*8 / 2, GAME_HEIGHT / 2 + 16, text2, 0);
}

void Game_SteerShip(int directionX, int directionY) {
    game.ship->directionX = directionX;
    game.ship->directionY = directionY;
}

GameContext *Game_Init() {
    for (int i = 0; i < GAME_ENTITY_CAPACITY; i++) {
        GameEntity *e = &game.entities[i];
        e->active = false;
        e->directionX = 0;
        e->directionY = 0;
        e->velocity = 0;
    }

    game.pixels = (TSG_PixelArray){
        .width = GAME_WIDTH, .height = GAME_HEIGHT, .data = game.framebuffer};

    int shipWidth = 30;
    int shipHeight = 30;
    game.ship = addEntity((GameEntity) {
        .velocity = 200,
        .directionX = 0,
        .directionY = 0,
        .projectileOffset = 15,
        .collider = (GameCollider) {
            .x = (float)GAME_WIDTH / 2 - (float)shipWidth / 2,
            .y = GAME_HEIGHT - (float)shipHeight - 10,
            .width = shipWidth,
            .height = shipHeight,
        },
    }, GAME_ENT_SHIP);
    game.playingState = GAME_PLAYING_STATE_MENU;
    return &game;
}

void Game_Update(int dt) {
    switch(game.playingState) {
        case GAME_PLAYING_STATE_PLAYING:
            updateGamePlaying(dt);
            break;
        case GAME_PLAYING_STATE_MENU:
            updateGameMenu(dt);
            break;
        case GAME_PLAYING_STATE_PAUSED:
            updateGamePaused(dt);
            break;
    }
}

void Game_Free() {
}
