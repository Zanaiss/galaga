#ifndef CONSTANTS_H
#define CONSTANTS_H

// Window
#define WINDOW_W 800
#define WINDOW_H 600

// Player
#define PLAYER_W 50
#define PLAYER_H 32
#define PLAYER_START_X (WINDOW_W/2 - PLAYER_W/2)
#define PLAYER_START_Y (WINDOW_H - 80)
#define PLAYER_SPEED 240.0f   // pixels per second

// Player bullets
#define MAX_PB 16
#define PB_W 8
#define PB_H 12
#define PB_SPEED 420.0f       // pixels per second

// Enemies (grid)
#define ENEMY_ROWS 3
#define ENEMY_COLS 6
#define ENEMY_W 40
#define ENEMY_H 30
#define ENEMY_INIT_STARTX 80
#define ENEMY_INIT_STARTY 50
#define ENEMY_H_SPACING 70
#define ENEMY_V_SPACING 48

// Enemy bullets
#define MAX_EB 24
#define EB_W 6
#define EB_H 10
#define EB_SPEED 200.0f

// Respawn
#define RESPAWN_DELAY 1.0f

// Files
#define SAVE_FILE "save_galaga.txt"
#define HIGH_FILE "highscore.txt"

#endif
