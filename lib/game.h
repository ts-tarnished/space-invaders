#include "ts_sw_graphics.h"

#define GAME_WIDTH 800
#define GAME_HEIGHT 600
#define GAME_PADDING 10
#define GAME_ENTITY_CAPACITY 2056
#define GAME_SHIP_PROJECTILE_CAPACITY 1028
#define GAME_ENEMY_CAPACITY 1028
#define GAME_ENEMY_WIDTH 40
#define GAME_ENEMY_HEIGHT 40

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

typedef struct GameEntity {
    int id;
    enum GameEntType entType;
    bool active;
    GameCollider collider;
    int directionX;
    int directionY;
    int projectileOffset;
    float velocity;
    void (*onUpdate)(struct GameEntity *e, int);
} GameEntity;

typedef struct GameLinkedEnt {
    struct GameEntity* entity;
    struct GameLinkedEnt* next;
} GameLinkedEnt;

typedef struct {
    GameEntity *ship;
    uint32_t framebuffer[GAME_WIDTH * GAME_HEIGHT];
    TSG_PixelArray pixels;
    GameEntity entities[GAME_ENTITY_CAPACITY];
    GameLinkedEnt entityPool[GAME_ENTITY_CAPACITY];
    GameLinkedEnt *enemies;
    GameLinkedEnt *projectiles;
    enum GamePlayingState playingState;
    int projectileTimer;
    int enemyVelocity;
    float enemyRowPosTracker;
} GameContext;

GameContext *Game_Init();
void Game_SteerShip(int directionX, int directionY);
void Game_Update(int dt_ns);
void Game_Free();
