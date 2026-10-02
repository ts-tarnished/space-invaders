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
static void drawBackground() { TSG_Clear(&game.pixels, 0xFFFF0000); }

static TSG_Rectangle gameColliderToTSGRect(GameCollider c) {
    return (TSG_Rectangle) {
        .x = c.x,
        .y = c.y,
        .width = c.width,
        .height = c.height
    };
}


static GameLinkedEnt* requestLinkedEnt() {
    for (int i = 0; i < GAME_ENTITY_CAPACITY; i++) {
        if (game.entityPool[i].entity == NULL) {
            if (game.entityPool[i].next != NULL) {
                fprintf(stderr, "Error: Requested linked entity has next field set.\n");
                exit(1);
            }
            return &game.entityPool[i];
        }
    }

    fprintf(stderr, "Error: Exceeded entity capacity.\n");
}
static GameLinkedEnt* linkEntity(GameLinkedEnt *head, GameEntity *e) {
    if (head == NULL) {
        GameLinkedEnt* linkedEnt = requestLinkedEnt();
        linkedEnt->entity = e;
        return linkedEnt;
    }

    GameLinkedEnt *tail = head;
    while(tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = requestLinkedEnt();
    tail->next->entity = e;
    return head;
}

static GameLinkedEnt* unlinkEntity(GameLinkedEnt *head, GameEntity *e) {
    if (head == NULL) {
        fprintf(stderr, "Error: Cannot unlink from null.\n");
        exit(1);
    }

    e->active = false;
    if (head->entity == e) {
        head->entity = NULL;
        GameLinkedEnt *newHead = head->next;
        head->next = NULL;
        return newHead;
    }

    GameLinkedEnt *prev = NULL;
    GameLinkedEnt *node = head;
    while(node->entity != e) {
        prev = node;
        node = node->next;
        if (node == NULL) {
            fprintf(stderr, "Error: Could not find entity in list.\n");
            exit(1);
        }
    }

    GameLinkedEnt *tail = node->next;
    node->entity = NULL;
    node->next = NULL;
    prev->next = tail;
    return head;
}

static GameEntity* addEntity(GameEntity ent, enum GameEntType entType) {
    for(int i = 0; i < GAME_ENTITY_CAPACITY; i++) {
        GameEntity *e = &game.entities[i];
        if (e->active)
            continue;
        *e = ent;
        e->id = i;
        e->active = true;
        e->entType = entType;
        return e;
    }
    fprintf(stderr, "Error: Entity Capacity exceeded.\n");
    exit(1);
}

static void shootProjectile(GameEntity *entity) {
    int projectileWidth = 10, projectileHeight = 10;
    GameEntity *newP = addEntity((GameEntity) {
        .velocity = 100,
        .directionX = 0,
        .directionY = -1,
        .collider = {
            .x = (float)entity->collider.x + entity->collider.width / 2 - (float)projectileWidth / 2,
            .y = entity->collider.y - projectileHeight - entity->projectileOffset - 30,
            .width = projectileWidth,
            .height = projectileHeight
        }
    }, GAME_ENT_PROJECTILE);

    game.projectiles = linkEntity(game.projectiles, newP);
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

static bool rectCollision(GameCollider c0, GameCollider c1) {
        return c0.x < c1.x + c1.width &&
               c0.x + c0.width > c1.x &&
               c0.y < c1.y + c1.height &&
               c0.y + c0.height > c1.y;
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
    if (e == NULL) {
        fprintf(stderr, "Error: Update failed. Entity must not be null.\n");
        exit(1);
    }

    if (!e->active)
        return;
    e->collider.y += e->directionY * dt * pow(10,-9) * e->velocity;
    e->collider.x += e->directionX * dt * pow(10,-9) * e->velocity;

    if (e->onUpdate != NULL)
        e->onUpdate(e, dt);
}

static void spawnEnemyRow(int row) {
    int width = GAME_ENEMY_WIDTH, height = GAME_ENEMY_HEIGHT;
    for (int c = 0; c < GAME_WIDTH / width; c++) {
        GameEntity *enemy = addEntity((GameEntity) {
            .directionX = 0,
            .directionY = 1,
            .velocity = game.enemyVelocity,
            .collider = (GameCollider) {
                .x = c * width + 1,
                .y = row * height + 1,
                .width = width - 2,
                .height = height - 2
            }
        }, GAME_ENT_ENEMY);
        game.enemies = linkEntity(game.enemies, enemy);
    }
}

static void updateGamePlaying(int dt) {
    srand(time(NULL));
    updateEnt(game.ship, dt);
    GameLinkedEnt *enemy = game.enemies;
    while(enemy != NULL) {
       updateEnt(enemy->entity, dt);
       enemy = enemy->next;
    }

    GameLinkedEnt *projectile = game.projectiles;
    while(projectile != NULL) {
        updateEnt(projectile->entity, dt);
        projectile = projectile->next;
    }

    projectile = game.projectiles;
    while(projectile != NULL) {
        GameLinkedEnt *enemy = game.enemies;
        bool collision = false;
        while(enemy != NULL) {
            collision = rectCollision(enemy->entity->collider, projectile->entity->collider);
           if(collision)
               break;
           enemy = enemy->next;
        }

        if (collision) {
            printf("Collision\n");
            GameLinkedEnt *nextProjectile = projectile->next;
            GameLinkedEnt *nextEnemy = enemy->next;
            game.projectiles = unlinkEntity(game.projectiles, projectile->entity);
            game.enemies = unlinkEntity(game.enemies, enemy->entity);
            projectile = nextProjectile;
            enemy = nextEnemy;
            continue;
        }

        projectile = projectile->next;
    }

    game.projectileTimer -= dt;
    if (game.projectileTimer <= 0) {
        shootProjectile(game.ship);
        game.projectileTimer = 400 * pow(10, 6);
    }

    game.enemyRowPosTracker += game.enemyVelocity * pow(10,-9) * dt;
    if (game.enemyRowPosTracker >= 0) {
        spawnEnemyRow(-1);
        game.enemyRowPosTracker = -1;
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

static void shipOnUpdate(GameEntity *e, int dt) {
    if (e->collider.x  < 0) {
        e->collider.x = 0;
    }
    if (e->collider.x  > GAME_WIDTH - e->collider.width) {
        e->collider.x = GAME_WIDTH - e->collider.width;
    }
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
            .y = GAME_HEIGHT - (float)shipHeight,
            .width = shipWidth,
            .height = shipHeight,
        },
        .onUpdate = shipOnUpdate
    }, GAME_ENT_SHIP);

    game.enemyRowPosTracker = -GAME_ENEMY_HEIGHT;
    game.enemyVelocity = 20;
    game.playingState = GAME_PLAYING_STATE_MENU;

    for (int i = -1; i < 6; i++) {
        spawnEnemyRow(i);
    }
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
