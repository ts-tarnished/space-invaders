#include "ts_sw_graphics.h"

#define GAME_WIDTH 800
#define GAME_HEIGHT 600
#define GAME_ENTITY_CAPACITY 1028

enum GameEntType {
    GAME_ENT_SHIP,
    GAME_ENT_ENEMY,
    GAME_ENT_PROJECTILE
};

enum GamePlayingState {
    GAME_PLAYING_STATE_PLAYING,
    GAME_PLAYING_STATE_PAUSED,
    GAME_PLAYING_STATE_MENU
};

typedef struct {
    float x;
    float y;
    float width;
    float height;
} GameCollider;

typedef struct {
    int id;
    enum GameEntType entType;
    bool active;
    GameCollider collider;
    int directionX;
    int directionY;
    int projectileOffset;
    float velocity;
} GameEntity;

typedef struct {
    GameEntity *ship;
    uint32_t framebuffer[GAME_WIDTH * GAME_HEIGHT];
    TSG_PixelArray pixels;
    GameEntity entities[GAME_ENTITY_CAPACITY];
    enum GamePlayingState playingState;
} GameContext;

GameContext *Game_Init();
void Game_SteerShip(int directionX, int directionY);
void Game_Update(int dt_ns);
void Game_Free();
