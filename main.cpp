#define _CRT_SECURE_NO_WARNINGS
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <ctime>
#include <cstdlib>
#include <cmath> 
#include <iostream>
#include <fstream>
#include <string>
#include "constants.h"
#include "bullet.h"
#include "player.h"
#include "enemy.h"
#include "filehandler.h"
#include "menu.h"
#include "game.h"
// Game state: 0 menu, 1 play, 2 pause, 3 gameover, 4 settings, 5 load_prompt
int gameState = 0;
// Score variables
int scoreVal = 0;
// High Scores Array (top 3 scores save karne)
int highScores[3] = { 0, 0, 0 };
// Difficulty & Settings
int difficultyLevel = 1; // 0 easy, 1 med, 2 hard
int masterVolume = 50;
// Lives & Respawn 
int livesLeft = 3;
bool playerDead = false;
float respawnTimer = 0.0f;
// Invincibility (to be used in respawning so u dont die immediately after respawning)
bool isInvincible = false;
float invincibilityTimer = 0.0f;
// Player position 
float playerX = PLAYER_START_X;
float playerY = PLAYER_START_Y;
// Player bullets 
float pbX[MAX_PB];
float pbY[MAX_PB];
bool pbActive[MAX_PB];
// Enemy Data (enemies are stored in a 2D grid)
float enemyX[ENEMY_ROWS][ENEMY_COLS];
float enemyY[ENEMY_ROWS][ENEMY_COLS];
bool enemyAlive[ENEMY_ROWS][ENEMY_COLS];
int enemyType[ENEMY_ROWS][ENEMY_COLS]; // 0,1,2 for different types
// STATES: 0=Grid, 1=Direct Dive, 2=ZigZag, 3=BoxSpiral
int enemyState[ENEMY_ROWS][ENEMY_COLS];
// enemy movement
float enemyVX[ENEMY_ROWS][ENEMY_COLS];
float enemyVY[ENEMY_ROWS][ENEMY_COLS];
float enemyTimer[ENEMY_ROWS][ENEMY_COLS]; // Pattern timing ke liye
int enemyAux[ENEMY_ROWS][ENEMY_COLS];     // Extra variable for zig-zag direction
float enemyDir = 1.0f;
float enemySpeed = 1.0f;
float diveSpeed = 200.0f;
int levelVal = 1;
int diveChance = 2; // Kitne percent chance hai ke enemy neeche aye ga
// Enemy bullets 
float ebX[MAX_EB];
float ebY[MAX_EB];
bool ebActive[MAX_EB];
// Explosions 
const int MAX_EXPL = 20;
float explX[MAX_EXPL];
float explY[MAX_EXPL];
float explTimer[MAX_EXPL];
bool explActive[MAX_EXPL];
// Textures & Sprites declarations, not using namespace sf because i had to use std for testing and debugging
sf::Texture texFallback; // very important, kyuke 2F vectors allowed hi ni we will use ts to solve that problem
sf::Texture texPlayer;
sf::Sprite spritePlayer;
sf::Texture texEnemy1, texEnemy2, texEnemy3;
sf::Sprite spriteEnemy;
sf::Texture texBullet;
sf::Sprite spritePB;
sf::Texture texEBullet;
sf::Sprite spriteEB;
sf::Texture texLife;
sf::Sprite spriteLife;
sf::Texture texExplosion;
sf::Sprite spriteExplosion;
sf::Font gameFont;
bool fontLoaded = false; // check for safe font loading
// Menu & Backgrounds
sf::Texture texMenuBg;
sf::Sprite spriteMenuBg;
sf::Texture texGameBg;
sf::Sprite spriteGameBg;
sf::Texture texLogo;
sf::Sprite spriteLogo;
// Audio
sf::SoundBuffer sbShoot;
sf::SoundBuffer sbExplode;
// Sound POOL (to prevent overlapping audio)
sf::Sound soundShootPool[5];
int shootPoolIndex = 0;
sf::Sound soundExplode;
sf::Music musicMenu;
sf::Music musicGame;
// Timing (used for enemy firing wagheira and their pattern descent)
sf::Clock frameClock;
float enemyFireTimer = 0.0f;
float enemyFireInterval = 2.0f;
// Forward declarations (cuz the functions use each other and aren't ordered yet so we need to declare them to avoid loss of data errors)
void safeLoadTexture(sf::Texture& t, const char path[]);
void safeLoadSound(sf::SoundBuffer& b, const char path[]);
void spawnExplosion(float x, float y);
void updateExplosions(float dt);
void drawExplosions(sf::RenderWindow& window);
bool checkSaveFileExists();
void showLoadPrompt(sf::RenderWindow& window, sf::Font& font);
void updateHighScores(int newScore);
void loadHighScores();
void saveHighScores();
// Helpers to handle edge cases, if TA feels funky and tries to delete our assets, the code should still work
void safeLoadTexture(sf::Texture& t, const char path[])
{
    if (!t.loadFromFile(path)) {
        // if no file found then no problem, do not crash the program pls
    }
}
void safeLoadSound(sf::SoundBuffer& b, const char path[])
{
    if (!b.loadFromFile(path)) {
        // same thing as texture, no sound found, no problem
    }
}
// Explosion System 
void spawnExplosion(float x, float y)
{
    // Check karte hain koi free slot hai array mein
    for (int i = 0; i < MAX_EXPL; ++i)
    {
        if (!explActive[i])
        {
            explActive[i] = true;
            explX[i] = x;
            explY[i] = y;
            explTimer[i] = 0.0f;
            break; // Ek mil gaya bas kafi hai
        }
    }
}
void updateExplosions(float dt)
{
    for (int i = 0; i < MAX_EXPL; ++i)
    {
        if (explActive[i])
        {
            explTimer[i] += dt;
            if (explTimer[i] > 0.5f) { // explosion disappears after 0.5 seconds
                explActive[i] = false;
            }
        }
    }
}
void drawExplosions(sf::RenderWindow& window)
{
    if (texExplosion.getSize().x == 0) return;

    for (int i = 0; i < MAX_EXPL; ++i)
    {
        if (explActive[i])
        {
            spriteExplosion.setPosition(explX[i], explY[i]);

            // the explosion fades out and increases in size, if we don't do this it will look weird asf
            float life = explTimer[i] / 0.5f;
            float scale = 1.0f + life; // increasing in size
            int alpha = (int)(255 * (1.0f - life)); // fading out in opacity

            spriteExplosion.setScale(scale, scale);
            spriteExplosion.setColor(sf::Color(255, 255, 255, alpha));

            window.draw(spriteExplosion);
        }
    }
}
// bullet.h implementation
void initPlayerBullets()
{
    for (int i = 0; i < MAX_PB; ++i) pbActive[i] = false;
}
void playerShoot(float px)
{
    // Find first inactive bullet in array and fire it
    for (int i = 0; i < MAX_PB; ++i)
    {
        if (!pbActive[i])
        {
            pbActive[i] = true;
            pbX[i] = px + PLAYER_W / 2.0f - PB_W / 2.0f;
            pbY[i] = playerY;

            // looping through 5 sounds, in case the TA has fast fingers and spams space too much, a maximum of 5 sounds will be played at once
            if (sbShoot.getSampleCount() > 0) {
                soundShootPool[shootPoolIndex].setBuffer(sbShoot);
                soundShootPool[shootPoolIndex].setVolume((float)masterVolume);
                soundShootPool[shootPoolIndex].play();
                // Cycle index 0->1->2->3->4->0
                shootPoolIndex = (shootPoolIndex + 1) % 5;
            }
            break;
        }
    }
}
void updatePlayerBullets(float dt)
{
    for (int i = 0; i < MAX_PB; ++i)
    {
        if (!pbActive[i]) continue;
        pbY[i] -= PB_SPEED * dt;
        if (pbY[i] < -50) pbActive[i] = false; // if the bullet goes out of the screen, deactivate it
    }
}

void drawPlayerBullets(sf::RenderWindow& window)
{
    if (texBullet.getSize().x > 0) {
        for (int i = 0; i < MAX_PB; ++i) {
            if (!pbActive[i]) continue;
            spritePB.setPosition(pbX[i], pbY[i]);
            window.draw(spritePB);
        }
    }
    else {
        // fallback section, very important, instead of using vector2f, we use a simple 1x1 pixel texture, and scale it according to whatever we want
        // would use rectangles here but since vector2f isn't allowed, we do the fallback method mentioned above
        sf::Sprite r;
        r.setTexture(texFallback);
        r.setColor(sf::Color::White);
        r.setScale((float)PB_W, (float)PB_H);

        for (int i = 0; i < MAX_PB; ++i) {
            if (!pbActive[i]) continue;
            r.setPosition(pbX[i], pbY[i]);
            window.draw(r);
        }
    }
}

// Enemy bullets
void initEnemyBullets()
{
    for (int i = 0; i < MAX_EB; ++i) ebActive[i] = false;
}

void spawnEnemyBullet(float x, float y)
{
    for (int i = 0; i < MAX_EB; ++i)
    {
        if (!ebActive[i])
        {
            ebActive[i] = true;
            ebX[i] = x + ENEMY_W / 2.0f - EB_W / 2.0f;
            ebY[i] = y + ENEMY_H;
            break;
        }
    }
}

void updateEnemyBullets(float dt)
{
    for (int i = 0; i < MAX_EB; ++i)
    {
        if (!ebActive[i]) continue;
        ebY[i] += EB_SPEED * dt; // the bullet will descend downwards 
        if (ebY[i] > WINDOW_H + 50) ebActive[i] = false;
    }
}

void drawEnemyBullets(sf::RenderWindow& window)
{
    if (texEBullet.getSize().x > 0) {
        for (int i = 0; i < MAX_EB; ++i) {
            if (!ebActive[i]) continue;
            spriteEB.setPosition(ebX[i], ebY[i]);
            window.draw(spriteEB);
        }
    }
    else {
        // peeche samjhaya hua, same logic
        sf::Sprite r;
        r.setTexture(texFallback);
        r.setColor(sf::Color(255, 150, 0)); // orange color
        r.setScale((float)EB_W, (float)EB_H);

        for (int i = 0; i < MAX_EB; ++i) {
            if (!ebActive[i]) continue;
            r.setPosition(ebX[i], ebY[i]);
            window.draw(r);
        }
    }
}

// player.h implementation
void initPlayer()
{
    // reset player to his starting position, defined in the constants in header files. this is basically where we the ship spawns in the beginning
    playerX = PLAYER_START_X;
    playerY = PLAYER_START_Y;
    playerDead = false;
    livesLeft = 3;
    respawnTimer = 0.0f;
    isInvincible = false;
    invincibilityTimer = 0.0f;

    if (texPlayer.getSize().x > 0) {
        spritePlayer.setTexture(texPlayer);
        spritePlayer.setScale((float)PLAYER_W / texPlayer.getSize().x,
            (float)PLAYER_H / texPlayer.getSize().y);
    }
}

void updatePlayer(float dt)
{
    if (gameState != 1) return;
    if (playerDead) return; // if the player is dead then he can't do anything so just end the function here

    // invincibilty logic, (basically agar aap mar jaen aur respawn hou, tou yei na hou kei spawn hote hi foran mar jaen, 2 seconds ke liye aap safe rahein gei
    if (isInvincible) {
        invincibilityTimer += dt;
        if (invincibilityTimer > 2.0f) { // 2 seconds safety
            isInvincible = false;
            spritePlayer.setColor(sf::Color::White); // Reset color
        }
        else {
            // Blinking Effect 
            int blink = (int)(invincibilityTimer * 10); // Fast blink
            if (blink % 2 == 0) spritePlayer.setColor(sf::Color(255, 255, 255, 100)); // Transparent
            else spritePlayer.setColor(sf::Color::White);
        }
    }
    else {
        spritePlayer.setColor(sf::Color::White); // Ensure full visibility
    }

    float move = PLAYER_SPEED * dt;
    // A D ya Arrows dono chalenge, press either left and right arrow key, or the A and D keys to move
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        playerX -= move;
        if (playerX < 0) playerX = 0;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        playerX += move;
        if (playerX > WINDOW_W - PLAYER_W) playerX = WINDOW_W - PLAYER_W;
    }
}

void drawPlayer(sf::RenderWindow& window)
{
    if (playerDead) return;

    if (texPlayer.getSize().x > 0) {
        spritePlayer.setPosition(playerX, playerY);
        window.draw(spritePlayer);
    }
    else {
        sf::Sprite r;
        r.setTexture(texFallback);
        r.setColor(sf::Color::Cyan);
        r.setScale((float)PLAYER_W, (float)PLAYER_H);
        r.setPosition(playerX, playerY);
        window.draw(r);
    }
}

bool checkPlayerHitByEnemyOrBullet()
{
    if (isInvincible) return false; // if the player is invincible (yeni abhi abhi spawn hua) then enemy or enemy bullet shouldn't hit him 

    // checking enemy bullets
    for (int i = 0; i < MAX_EB; ++i)
    {
        if (!ebActive[i]) continue;
        // Simple AABB collision detection, ill explain this concept in the recorded video but basically eik region hota hei around everybody which is called the hitbox, if the position of the hitbox and a bullet matches, then they're colliding
        if (ebX[i] + EB_W > playerX && ebX[i] < playerX + PLAYER_W &&
            ebY[i] + EB_H > playerY && ebY[i] < playerY + PLAYER_H)
        {
            ebActive[i] = false;
            return true; // if hit, then just return
        }
    }

    // checking enemy collision itself, this is the kamikaze enemy variant, the enemies diving down. similar to the bullet if the hitbox of the player and enemy matches, collision detected
    for (int r = 0; r < ENEMY_ROWS; ++r)
    {
        for (int c = 0; c < ENEMY_COLS; ++c)
        {
            if (!enemyAlive[r][c]) continue;
            float ex = enemyX[r][c], ey = enemyY[r][c];
            if (playerX + PLAYER_W > ex && playerX < ex + ENEMY_W &&
                playerY + PLAYER_H > ey && playerY < ey + ENEMY_H)
            {
                enemyAlive[r][c] = false; // kill the enemy as well if they collide with the player
                spawnExplosion(ex, ey);

                if (sbExplode.getSampleCount() > 0) {
                    soundExplode.setBuffer(sbExplode);
                    soundExplode.setVolume((float)masterVolume);
                    soundExplode.play();
                }
                return true;
            }
        }
    }
    return false;
}

void handleRespawn(float dt)
{
    if (!playerDead) return;
    respawnTimer += dt;
    // basically just wait a little bit before respawning the player
    if (respawnTimer >= RESPAWN_DELAY)
    {
        playerDead = false;
        respawnTimer = 0.0f;
        playerX = PLAYER_START_X;
        playerY = PLAYER_START_Y;

        // enable the invincibility when we respawn
        isInvincible = true;
        invincibilityTimer = 0.0f;
    }
}

// enemy.h implementation
void setupEnemies()
{
    // making the grid of the enemies, basically the 2D array
    float sx = ENEMY_INIT_STARTX;
    float sy = ENEMY_INIT_STARTY;
    for (int r = 0; r < ENEMY_ROWS; ++r)
    {
        for (int c = 0; c < ENEMY_COLS; ++c)
        {
            enemyAlive[r][c] = true;
            enemyX[r][c] = sx + c * ENEMY_H_SPACING;
            enemyY[r][c] = sy + r * ENEMY_V_SPACING;

            // we have 3 sprite types of enemies, we're giving every enemy a random sprite out of the 3 taake different lagein sare
            enemyType[r][c] = rand() % 3;

            enemyState[r][c] = 0; // Sab grid mein start honge, different enemy states hein like diving and whatnot
            enemyTimer[r][c] = 0.0f;
            enemyAux[r][c] = 0;

            enemyVX[r][c] = 0.0f;
            enemyVY[r][c] = 0.0f;
        }
    }
    enemyDir = 1.0f;

    // this is very important, this is the adjustment of the enemy fire rate based on the difficulty level which can be selected through the settings. this is just a basic formula which increases the rate on different difficulties
    enemyFireInterval = 1.0f / (1.0f + 0.5f * difficultyLevel);
}


// VERY IMPORTANT SECTION 
// also enemyAux array basically isliye use kiya hei to know kei enemy left move kar rah kei right, whenever we need to switch it in patterns, we just change its value from 0 to 1 or viceversa
// enemyVX aur enemyVY array is the diving SPEED
// enemyX and enemyY arrays are obviously the XY positions of the enemies
// enemyState array is the array with the different enemy states like normal movement, diving, box spiral, and zigzag. 
void updateEnemies(float dt)
{
    bool touchEdge = false;

    for (int r = 0; r < ENEMY_ROWS; ++r)
    {
        for (int c = 0; c < ENEMY_COLS; ++c)
        {
            if (!enemyAlive[r][c]) continue;

            // STATE 0: GRID MOVEMENT (Normal movement) 
            if (enemyState[r][c] == 0)
            {
                enemyX[r][c] += enemyDir * enemySpeed * dt;

                // Agar screen ke end pe pohanch gya tou touchedge = true
                if (enemyX[r][c] < 0 || enemyX[r][c] > WINDOW_W - ENEMY_W)
                    touchEdge = true;

                // Random chance to dive, basically wou player ki taraf direct descend karna shuru hojae ga
                if (rand() % 1000 < diveChance)
                {
                    // Randomly decide pattern: 0=Direct, 1=ZigZag, 2=BoxSpiral
                    // if they do decide to dive we further randomize their dive pattern as well
                    // direct wala is just straight linear diving
                    // zigzag is self explanatory
                    // boxspiral mei wou like left aur right hote hue neeche ate 
                    int diveType = rand() % 3;

                    if (diveType == 0) // direct dive
                    {
                        // descending DIRECTLY towards the player
                        enemyState[r][c] = 1;
                        float dx = playerX - enemyX[r][c];
                        float dy = playerY - enemyY[r][c];
                        float len = sqrt(dx * dx + dy * dy);
                        if (len > 0) {
                            enemyVX[r][c] = (dx / len) * diveSpeed;
                            enemyVY[r][c] = (dy / len) * diveSpeed;
                        }
                    }
                    else if (diveType == 1)
                    {
                        // zigzag movement
                        enemyState[r][c] = 2;
                        enemyTimer[r][c] = 0.0f;
                        enemyAux[r][c] = 0; // 0 = Left, 1 = Right
                    }
                    else
                    {
                        // box spiral movement
                        enemyState[r][c] = 3;
                        enemyTimer[r][c] = 0.0f;
                    }
                }
            }
            // now if the enemy is already in the diving state, in the next frame, this is where we descend him downwards
            // this is normal diving so straight forward neeche karni position
            else if (enemyState[r][c] == 1)
            {
                enemyX[r][c] += enemyVX[r][c] * dt;
                enemyY[r][c] += enemyVY[r][c] * dt;
            }
            // state 2 is the zigzag dive, iske andar zra formulas use hue hein for the dive
            else if (enemyState[r][c] == 2)
            {
                enemyY[r][c] += diveSpeed * dt; // descend the Y position obviously
                enemyTimer[r][c] += dt;

                if (enemyAux[r][c] == 0) {
                    enemyX[r][c] -= diveSpeed * dt; // moving left
                    if (enemyTimer[r][c] > 0.3f) {
                        enemyAux[r][c] = 1; // switching rightwards after a certain time to ensure the zigzag pattern
                        enemyTimer[r][c] = 0.0f;
                    }
                }
                else {
                    enemyX[r][c] += diveSpeed * dt; // moving it right
                    if (enemyTimer[r][c] > 0.3f) {
                        enemyAux[r][c] = 0; // switching leftwards like explained above
                        enemyTimer[r][c] = 0.0f;
                    }
                }
            }
            // state 3: box spiral
            else if (enemyState[r][c] == 3)
            {
                enemyTimer[r][c] += dt;
                // divided the movement into steps, basically hardcoded the box movement, timing kei mutabiq kiya hei like is time kei baad is direction mei jae phir ismei
                if (enemyTimer[r][c] < 0.2f) {
                    enemyX[r][c] += diveSpeed * dt; // Right
                }
                else if (enemyTimer[r][c] < 0.4f) {
                    enemyY[r][c] += diveSpeed * dt; // Down
                }
                else if (enemyTimer[r][c] < 0.6f) {
                    enemyX[r][c] -= diveSpeed * dt; // Left
                }
                else if (enemyTimer[r][c] < 0.8f) {
                    enemyY[r][c] += diveSpeed * dt; // Down
                }
                else {
                    enemyTimer[r][c] = 0.0f; // Reset loop
                }
            }

            // Agar screen se neeche nikal gaya to disable/qatal kardo 
            if (enemyState[r][c] != 0 && enemyY[r][c] > WINDOW_H)
            {
                enemyAlive[r][c] = false;
            }
        }
    }

    // moving the grid downwards, basically if the edge of the screen is touched when they're moving left, reverse their direction so they dont move out of the screen
    if (touchEdge)
    {
        enemyDir = -enemyDir;
        for (int r = 0; r < ENEMY_ROWS; ++r) {
            for (int c = 0; c < ENEMY_COLS; ++c) {
                if (enemyAlive[r][c] && enemyState[r][c] == 0) {
                    enemyY[r][c] += 18.0f;
                }
            }
        }
    }
}

// basic sfml texture drawing

void drawEnemies(sf::RenderWindow& window)
{
    if (texEnemy1.getSize().x > 0)
    {
        for (int r = 0; r < ENEMY_ROWS; ++r)
        {
            for (int c = 0; c < ENEMY_COLS; ++c)
            {
                if (enemyAlive[r][c])
                {
                    // set the sprite texture based on the enemy type, as explained above 3 types hein enemy textures kei
                    if (enemyType[r][c] == 0) spriteEnemy.setTexture(texEnemy1);
                    else if (enemyType[r][c] == 1) spriteEnemy.setTexture(texEnemy2);
                    else spriteEnemy.setTexture(texEnemy3);

                    spriteEnemy.setPosition(enemyX[r][c], enemyY[r][c]);

                    float tsx = (float)texEnemy1.getSize().x;
                    float tsy = (float)texEnemy1.getSize().y;

                    spriteEnemy.setScale((float)ENEMY_W / tsx, (float)ENEMY_H / tsy);
                    window.draw(spriteEnemy);
                }
            }
        }
    }
    else
    {
        // woi fallback method
        sf::Sprite re;
        re.setTexture(texFallback);
        re.setColor(sf::Color::Red);
        re.setScale((float)ENEMY_W, (float)ENEMY_H);

        for (int r = 0; r < ENEMY_ROWS; ++r)
            for (int c = 0; c < ENEMY_COLS; ++c)
                if (enemyAlive[r][c]) {
                    re.setPosition(enemyX[r][c], enemyY[r][c]);
                    window.draw(re);
                }
    }
}

bool allEnemiesDead()
{
    // Check karo koi bhi zinda hai
    for (int r = 0; r < ENEMY_ROWS; ++r)
        for (int c = 0; c < ENEMY_COLS; ++c)
            if (enemyAlive[r][c]) return false;
    return true; // this means all dead
}

void resetEnemiesForNextWave()
{
    // level up now, this happens when all enemies are dead. now the difficulty will increase 
    levelVal++;
    setupEnemies();

    // this is where we increase the difficulty, based on the level value which increases we put that in formulas here and the different enemy values keep increasing along with levels
    // here's the speed formulae
    enemySpeed = 70.0f + (levelVal * 15.0f);
    diveSpeed = 200.0f + (levelVal * 10.0f);

    // decreasing fire interval formulae
    float newInterval = 2.0f - (levelVal * 0.15f);
    if (newInterval < 0.3f) newInterval = 0.3f;
    enemyFireInterval = newInterval;

    // increase the dive chance (ab zyada chance hou kei enemies neeche player ki taraf ayein)
    diveChance = 2 + levelVal;
}

// high score logic, file handling stuff
void updateHighScores(int newScore)
{
    if (newScore == highScores[0] || newScore == highScores[1] || newScore == highScores[2]) {
        return;
    }

    // Check karo score leaderboard mein kahan fit hota hai, top 3 scores only
    if (newScore > highScores[0]) {
        highScores[2] = highScores[1];
        highScores[1] = highScores[0];
        highScores[0] = newScore;
    }
    else if (newScore > highScores[1]) {
        highScores[2] = highScores[1];
        highScores[1] = newScore;
    }
    else if (newScore > highScores[2]) {
        highScores[2] = newScore;
    }
    // immediately save the scores
    saveHighScores();
}

void saveHighScores()
{
    std::ofstream outFile(HIGH_FILE);
    if (outFile.is_open()) {
        outFile << highScores[0] << " " << highScores[1] << " " << highScores[2] << std::endl;
        outFile.close();
    }
}

void loadHighScores()
{
    std::ifstream inFile(HIGH_FILE);
    if (inFile.is_open()) {
        inFile >> highScores[0] >> highScores[1] >> highScores[2];
        inFile.close();
    }
    else {
        highScores[0] = 0; highScores[1] = 0; highScores[2] = 0;
    }
}

//filehandler.h implementations 
void saveGameState()
{
    std::ofstream outFile(SAVE_FILE);
    if (!outFile.is_open()) return;

    // Writing variables separated by spaces
    outFile << scoreVal << " " << highScores[0] << " " << highScores[1] << " " << highScores[2] << " "
        << difficultyLevel << " " << livesLeft << " " << playerX << " " << playerY << " "
        << 1 << " " << levelVal << std::endl;

    for (int r = 0; r < ENEMY_ROWS; ++r)
    {
        for (int c = 0; c < ENEMY_COLS; ++c)
        {
            int isAlive = enemyAlive[r][c] ? 1 : 0;
            outFile << isAlive << " " << enemyX[r][c] << " " << enemyY[r][c] << " "
                << enemyState[r][c] << " " << enemyVX[r][c] << " " << enemyVY[r][c] << " "
                << enemyType[r][c] << " " << enemyTimer[r][c] << " " << enemyAux[r][c] << std::endl;
        }
    }
    outFile.close();
}

void loadGameState()
{
    std::ifstream inFile(SAVE_FILE);
    if (!inFile.is_open()) {
        printf("Error: No save file found!\n");
        return;
    }

    int s = 0, h1 = 0, h2 = 0, h3 = 0, diff = 1, lives = 3, dummyEndless = 1, lvl = 1;
    float px = PLAYER_START_X, py = PLAYER_START_Y;
    int dH1 = 0, dH2 = 0, dH3 = 0;

    levelVal = 1;
    diveChance = 2;
    enemySpeed = 70.0f;
    diveSpeed = 200.0f;
    enemyFireInterval = 1.8f;

    // Reading from stream
    if (inFile >> s >> dH1 >> dH2 >> dH3 >> diff >> lives >> px >> py >> dummyEndless >> lvl)
    {
        scoreVal = s;
        difficultyLevel = diff;
        livesLeft = lives;
        playerX = px;
        playerY = py;
        levelVal = lvl;

        if (levelVal < 1) levelVal = 1;
        if (livesLeft < 1) livesLeft = 1;
        if (playerX < 0) playerX = 0;
        if (playerX > WINDOW_W - PLAYER_W) playerX = WINDOW_W - PLAYER_W;

        diveChance = 2 + levelVal;
        enemySpeed = 70.0f + (levelVal * 15.0f);
        diveSpeed = 200.0f + (levelVal * 10.0f);
        float newInterval = 2.0f - (levelVal * 0.15f);
        if (newInterval < 0.3f) newInterval = 0.3f;
        enemyFireInterval = newInterval;
    }

    // Skip remainder of line 
    inFile.ignore(10000, '\n');

    for (int r = 0; r < ENEMY_ROWS; ++r)
    {
        for (int c = 0; c < ENEMY_COLS; ++c)
        {
            int isAlive = 0;
            float ex = 0, ey = 0, evx = 0, evy = 0;
            int state = 0;
            int type = 0;
            float timer = 0.0f;
            int aux = 0;

            if (inFile >> isAlive >> ex >> ey >> state >> evx >> evy >> type >> timer >> aux)
            {
                enemyAlive[r][c] = (isAlive == 1);
                enemyX[r][c] = ex;
                enemyY[r][c] = ey;
                enemyState[r][c] = state;
                enemyVX[r][c] = evx;
                enemyVY[r][c] = evy;
                enemyType[r][c] = type;
                enemyTimer[r][c] = timer;
                enemyAux[r][c] = aux;
            }
        }
    }
    inFile.close();

    initPlayerBullets();
    initEnemyBullets();

    // delete any old explosions playing 
    for (int i = 0; i < MAX_EXPL; ++i) explActive[i] = false;
    // ensuring that Game music plays and Menu music stops
    musicMenu.stop();
    // ensuring that volume is set to current masterVolume
    musicGame.setVolume((float)masterVolume);
    // Only play if not already playing
    if (musicGame.getStatus() != sf::Music::Playing) {
        musicGame.play();
    }

    // kill invincibilty as explained before
    isInvincible = true;
    invincibilityTimer = 0.0f;

    gameState = 1;
    frameClock.restart(); // clock restart for prevention of the load bug
    // printing just for testing and confirmation
    printf("Game Loaded! Level: %d\n", levelVal);
}

bool checkSaveFileExists()
{
    std::ifstream inFile(SAVE_FILE);
    return inFile.good(); // .good() means file exists and opened successfully, agr mil gayi tou it will return true and nai mili tou it will return false
}

//menu.h implementation
//yaar yei ez portion hei sfml wala hei ai sei keh dou samjha dega i aint writing many comments here
// bas eik important cheez jaha bhi WINDOW_W aur WINDOW_H use horaha hei, this is an edge case being handled. like if the TA decides to change our window size or go full screen, the sprites would not work properly, but we're scaling them with respect to the window size itself so changing the window size doesn't matter since our sprite sizes depend on the window size itself

void showStartMenu(sf::RenderWindow& window, sf::Font& font)
{
    if (texMenuBg.getSize().x > 0) window.draw(spriteMenuBg);

    if (texLogo.getSize().x > 0) {
        spriteLogo.setPosition(WINDOW_W / 2.0f, 30);
        window.draw(spriteLogo);
    }
    else {
        // font check, edge case here if the TA deletes our font it shouldn't crash and still work
        if (fontLoaded) {
            sf::Text title("GALAGA", font, 50);
            title.setFillColor(sf::Color::Cyan);
            sf::FloatRect textRect = title.getLocalBounds();
            title.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            title.setPosition(WINDOW_W / 2.0f, 80);
            window.draw(title);
        }
    }

    if (fontLoaded) {
        sf::Text tStart("PRESS ENTER TO START", font, 30);
        tStart.setFillColor(sf::Color::Green);
        sf::FloatRect r1 = tStart.getLocalBounds();
        tStart.setOrigin(r1.left + r1.width / 2.0f, r1.top + r1.height / 2.0f);
        tStart.setPosition(WINDOW_W / 2.0f, 420);

        // Shadow effect on text, black karke zra peeche karke dala hei for that effect
        sf::Text tStartShadow = tStart;
        tStartShadow.setFillColor(sf::Color::Black);
        tStartShadow.setPosition(WINDOW_W / 2.0f + 2, 420 + 2);

        window.draw(tStartShadow);
        window.draw(tStart);

        sf::Text tOpt("S - SETTINGS    Q - QUIT", font, 18);
        tOpt.setFillColor(sf::Color::White);
        sf::FloatRect r2 = tOpt.getLocalBounds();
        tOpt.setOrigin(r2.left + r2.width / 2.0f, r2.top + r2.height / 2.0f);
        tOpt.setPosition(WINDOW_W / 2.0f, 500);
        window.draw(tOpt);

        // Show TOP score only here
        char buf[64]; sprintf(buf, "HIGH SCORE: %d", highScores[0]);
        sf::Text th(buf, font, 20);
        th.setFillColor(sf::Color::Yellow);
        sf::FloatRect r3 = th.getLocalBounds();
        th.setOrigin(r3.left + r3.width / 2.0f, r3.top + r3.height / 2.0f);
        th.setPosition(WINDOW_W / 2.0f, 560);
        window.draw(th);

        // Controls Section in the bottom right
        sf::Text tControls("CONTROLS:\nMove: WASD / Arrows\nShoot: Space", font, 16);
        tControls.setFillColor(sf::Color::Cyan);
        sf::FloatRect rc = tControls.getLocalBounds();
        tControls.setOrigin(rc.width, rc.height);
        tControls.setPosition(WINDOW_W - 20, WINDOW_H - 20);
        window.draw(tControls);
    }
}

void showLoadPrompt(sf::RenderWindow& window, sf::Font& font)
{
    if (!fontLoaded) return;

    // draw the background
    if (texMenuBg.getSize().x > 0) window.draw(spriteMenuBg);

    sf::Text tTitle("SAVE GAME FOUND!", font, 30);
    tTitle.setFillColor(sf::Color::Yellow);
    sf::FloatRect r1 = tTitle.getLocalBounds();
    tTitle.setOrigin(r1.left + r1.width / 2.0f, r1.top + r1.height / 2.0f);
    tTitle.setPosition(WINDOW_W / 2.0f, 200);

    sf::Text tMsg("Do you want to continue?", font, 20);
    tMsg.setFillColor(sf::Color::White);
    sf::FloatRect r2 = tMsg.getLocalBounds();
    tMsg.setOrigin(r2.left + r2.width / 2.0f, r2.top + r2.height / 2.0f);
    tMsg.setPosition(WINDOW_W / 2.0f, 250);

    sf::Text tYes("Y - Continue (Load Game)", font, 24);
    tYes.setFillColor(sf::Color::Green);
    sf::FloatRect r3 = tYes.getLocalBounds();
    tYes.setOrigin(r3.left + r3.width / 2.0f, r3.top + r3.height / 2.0f);
    tYes.setPosition(WINDOW_W / 2.0f, 350);

    sf::Text tNo("N - New Game (Overwrite)", font, 24);
    tNo.setFillColor(sf::Color::Red);
    sf::FloatRect r4 = tNo.getLocalBounds();
    tNo.setOrigin(r4.left + r4.width / 2.0f, r4.top + r4.height / 2.0f);
    tNo.setPosition(WINDOW_W / 2.0f, 400);

    window.draw(tTitle);
    window.draw(tMsg);
    window.draw(tYes);
    window.draw(tNo);
}

void showPauseOverlay(sf::RenderWindow& window, sf::Font& font)
{
    if (!fontLoaded) return;

    sf::Text t("PAUSED - Press P to resume", font, 24);
    t.setFillColor(sf::Color::Yellow);
    t.setPosition(200, 260);
    window.draw(t);

    sf::Text tSave("Press U to Save Game, Esc to Main Menu", font, 18);
    tSave.setFillColor(sf::Color::White);
    tSave.setPosition(220, 300);
    window.draw(tSave);
}

void showGameOverScreen(sf::RenderWindow& window, sf::Font& font, int score, int highScore)
{
    // Draw background if it exists
    if (texMenuBg.getSize().x > 0) window.draw(spriteMenuBg);

    if (!fontLoaded) return;

    char buf[512];
    sprintf(buf, "GAME OVER\n\nYour Score: %d", score);

    sf::Text t(buf, font, 30);
    t.setFillColor(sf::Color::Red);
    sf::FloatRect r1 = t.getLocalBounds();
    t.setOrigin(r1.left + r1.width / 2.0f, r1.top + r1.height / 2.0f);
    t.setPosition(WINDOW_W / 2.0f, 150);
    window.draw(t);

    // leaderboard display from the highscores array
    sf::Text tLead("LEADERBOARD", font, 24);
    tLead.setFillColor(sf::Color::Yellow);
    sf::FloatRect rL = tLead.getLocalBounds();
    tLead.setOrigin(rL.left + rL.width / 2.0f, rL.top + rL.height / 2.0f);
    tLead.setPosition(WINDOW_W / 2.0f, 250);
    window.draw(tLead);

    char scoresBuf[256];
    sprintf(scoresBuf, "1. %d\n2. %d\n3. %d", highScores[0], highScores[1], highScores[2]);
    sf::Text tScores(scoresBuf, font, 22);
    tScores.setFillColor(sf::Color::White);
    tScores.setLineSpacing(1.5f);
    sf::FloatRect rS = tScores.getLocalBounds();
    tScores.setOrigin(rS.left + rS.width / 2.0f, rS.top + rS.height / 2.0f);
    tScores.setPosition(WINDOW_W / 2.0f, 330);
    window.draw(tScores);

    sf::Text tRestart("Press ENTER to return to Menu", font, 18);
    tRestart.setFillColor(sf::Color::Green);
    sf::FloatRect rR = tRestart.getLocalBounds();
    tRestart.setOrigin(rR.left + rR.width / 2.0f, rR.top + rR.height / 2.0f);
    tRestart.setPosition(WINDOW_W / 2.0f, 450);
    window.draw(tRestart);
}

void showSettingsScreen(sf::RenderWindow& window, sf::Font& font, int difficulty)
{
    if (!fontLoaded) return;
    if (texMenuBg.getSize().x > 0) window.draw(spriteMenuBg);
    char buf[256];
    sprintf(buf, "SETTINGS\n\n1 - Easy  2 - Medium  3 - Hard\n\nUp/Down Arrow - Volume: %d%%\n\nEsc - Back", masterVolume);
    sf::Text t(buf, font, 35);
    t.setFillColor(sf::Color::Cyan);

    sf::FloatRect textRect = t.getLocalBounds();
    t.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    t.setPosition((float)window.getSize().x / 2.0f, (float)window.getSize().y / 2.0f);
    window.draw(t);
}

// game.h implemnentation very important
void initAll() //when the game starts we will initialize everything like loading textures and displaying the menu
// jo peeche functions banaye hein we're using all of them here
{
    srand((unsigned)time(NULL)); //for random and time

    // THIS IS WHERE WE CREATE THE FALLBACK TEXTURE JISKE BARE MEI MEI BATA RAHA THA, WE CREATE A 1x1 pixel and SCALE EVERYTHING ON THAT
    sf::Image img1x1;
    img1x1.create(1, 1, sf::Color::White);
    texFallback.loadFromImage(img1x1);

    // loading all the textures
    safeLoadTexture(texPlayer, "assets/player.png");
    safeLoadTexture(texEnemy1, "assets/enemy1.png");
    safeLoadTexture(texEnemy2, "assets/enemy2.png");
    safeLoadTexture(texEnemy3, "assets/enemy3.png");
    safeLoadTexture(texBullet, "assets/bullet.png");
    safeLoadTexture(texEBullet, "assets/ebullet.png");
    safeLoadTexture(texLife, "assets/life.png");
    safeLoadTexture(texExplosion, "assets/explosion.png");
    safeLoadTexture(texMenuBg, "assets/menu_bg.png");
    safeLoadTexture(texGameBg, "assets/game_bg.png");
    safeLoadTexture(texLogo, "assets/logo.png");

    // Setup Sprites
    if (texMenuBg.getSize().x > 0) {
        spriteMenuBg.setTexture(texMenuBg);
        spriteMenuBg.setScale(
            (float)WINDOW_W / texMenuBg.getSize().x,  // THIS IS VERY IMPORTANT THIS IS AN EDGE CASE HANDLED, LIKE I EXPLAINED ABOVE
            (float)WINDOW_H / texMenuBg.getSize().y   // basically jo bhi scale hei we're using WINDOW_W and WINDOW_H to scale it according to the window so that if the window size is changed the size of the sprites remains fine since it depends on the window size itself already
        );
    }
    if (texGameBg.getSize().x > 0) {
        spriteGameBg.setTexture(texGameBg);
        spriteGameBg.setScale(
            (float)WINDOW_W / texGameBg.getSize().x,
            (float)WINDOW_H / texGameBg.getSize().y
        );
    }
    if (texLogo.getSize().x > 0) {
        spriteLogo.setTexture(texLogo);
        spriteLogo.setOrigin(texLogo.getSize().x / 2.0f, 0);
        spriteLogo.setPosition(WINDOW_W / 2.0f, 50);
        if (texLogo.getSize().x > 600) spriteLogo.setScale(0.5f, 0.5f);
    }
    if (texExplosion.getSize().x > 0) {
        spriteExplosion.setTexture(texExplosion);
        spriteExplosion.setOrigin(texExplosion.getSize().x / 2.0f, texExplosion.getSize().y / 2.0f);
    }

    // sounds
    safeLoadSound(sbShoot, "assets/shoot.wav");
    safeLoadSound(sbExplode, "assets/explode.wav");

    // music
    if (musicMenu.openFromFile("assets/music_menu.mp3")) {
        musicMenu.setLoop(true);
        musicMenu.setVolume((float)masterVolume); // initiate with respect to master volume
        musicMenu.play();
    }
    if (musicGame.openFromFile("assets/music_game.mp3")) {
        musicGame.setLoop(true);
        musicGame.setVolume((float)masterVolume);
    }

    // assign the textures to the variables and all
    if (texPlayer.getSize().x > 0) spritePlayer.setTexture(texPlayer);
    if (texBullet.getSize().x > 0) spritePB.setTexture(texBullet);
    if (texEBullet.getSize().x > 0) spriteEB.setTexture(texEBullet);
    if (texLife.getSize().x > 0) spriteLife.setTexture(texLife);

    initPlayer();
    initPlayerBullets();
    initEnemyBullets();

    // initiate explosion array
    for (int i = 0; i < MAX_EXPL; ++i) explActive[i] = false;

    setupEnemies();

    scoreVal = 0;
    loadHighScores();

    enemyFireTimer = 0.0f;
    frameClock.restart();
}


// this is where we update the game every frame
// we're using the functions defined beforehand here alot
void updateGame(float dt)
{
    if (gameState != 1) return; // pause hei tou kuch ni karna
    updatePlayer(dt);
    updateExplosions(dt);

    // here we ensure that kei space eik baar dabane sei eik bar hei shoot hou, no spam allowed
    static bool canShoot = true;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        if (canShoot && !playerDead) {
            playerShoot(playerX);//shoot function here
            canShoot = false;
        }
    }
    else
        canShoot = true;

    updatePlayerBullets(dt);
    updateEnemyBullets(dt);
    updateEnemies(dt);
    enemyFireTimer += dt;
    if (enemyFireTimer > enemyFireInterval)
    {
        // try and get an alive enemy to fire a bullet at us, a random enemy out of the grid
        int attempts = 0;
        while (attempts < 200)
        {
            int r = rand() % ENEMY_ROWS;
            int c = rand() % ENEMY_COLS;
            if (enemyAlive[r][c])
            {
                spawnEnemyBullet(enemyX[r][c], enemyY[r][c]);
                break;
            }
            ++attempts;
        }
        enemyFireTimer = 0.0f;
    }

    // Bullet collision logic, explained above already
    for (int i = 0; i < MAX_PB; ++i)
    {
        if (!pbActive[i]) continue;
        for (int r = 0; r < ENEMY_ROWS; ++r)
        {
            for (int c = 0; c < ENEMY_COLS; ++c)
            {
                if (!enemyAlive[r][c]) continue;
                float ex = enemyX[r][c], ey = enemyY[r][c];
                // Agar bullet box ke andar hai
                if (pbX[i] > ex && pbX[i] < ex + ENEMY_W &&
                    pbY[i] > ey && pbY[i] < ey + ENEMY_H)
                {
                    enemyAlive[r][c] = false;
                    pbActive[i] = false;
                    scoreVal += 10;

                    // put the explosion here on the enemy position if they're hit obv
                    spawnExplosion(ex, ey);

                    if (sbExplode.getSampleCount() > 0) {
                        soundExplode.setBuffer(sbExplode);
                        soundExplode.setVolume((float)masterVolume);
                        soundExplode.play();
                    }
                }
            }
        }
    }

    if (!playerDead && checkPlayerHitByEnemyOrBullet())
    {
        playerDead = true;
        livesLeft--;
        if (livesLeft <= 0) {
            // GAME OVER TRIGGER
            gameState = 3;
            updateHighScores(scoreVal);
        }
        else respawnTimer = 0.0f;
    }

    if (playerDead) handleRespawn(dt);

    if (allEnemiesDead())
    {
        resetEnemiesForNextWave(); // Next round start
    }
}

// this is the sfml part of the gameplay itself, we're drawing all the sprites here in this function once again using functions defined in above lines
void drawGame(sf::RenderWindow& window)
{
    // Draw Background First
    if (texGameBg.getSize().x > 0) window.draw(spriteGameBg);
    // Draw Game Objects
    drawPlayer(window);
    drawPlayerBullets(window);
    drawEnemyBullets(window);
    drawEnemies(window);
    // Draw Explosions (if any are active)
    drawExplosions(window);
    // UI (Lives display)
    if (texLife.getSize().x > 0) {
        for (int i = 0; i < livesLeft; ++i) {
            spriteLife.setPosition(10 + i * (texLife.getSize().x + 6), WINDOW_H - texLife.getSize().y - 6);
            window.draw(spriteLife);
        }
    }
    else {
        sf::Sprite r;
        r.setTexture(texFallback);
        r.setColor(sf::Color::Cyan);
        r.setScale(12.0f, 12.0f); // 12x12 set here, this is an edge case, this will happen if the TA deletes our assets
        for (int i = 0; i < livesLeft; ++i) {
            r.setPosition(10 + i * 18, WINDOW_H - 20);
            window.draw(r);
        }
    }
    // text at the bottom right and top is worked with here, simple shi
    if (fontLoaded) {
        char buf[128];
        const char* diffName = "Easy";
        if (difficultyLevel == 1) diffName = "Med";
        else if (difficultyLevel == 2) diffName = "Hard";

        sprintf(buf, "Score: %d   High: %d   Lives: %d   LEVEL: %d   Diff: %s",
            scoreVal, highScores[0], livesLeft, levelVal, diffName);

        sf::Text t(buf, gameFont, 18);
        t.setPosition(10, 10);
        t.setFillColor(sf::Color::Yellow);
        window.draw(t);

        // controls 
        sf::Text tCtrl("WASD/Arrows: Move  Space: Shoot\nU: Save  P: Pause  Esc: Menu", gameFont, 14);
        tCtrl.setFillColor(sf::Color(255, 255, 255, 180)); // Slightly transparent
        sf::FloatRect rCtrl = tCtrl.getLocalBounds();
        tCtrl.setOrigin(rCtrl.width, rCtrl.height);
        tCtrl.setPosition(WINDOW_W - 10, WINDOW_H - 10);
        window.draw(tCtrl);
    }
}

// function used whenever we start a new game 
void startNewGame()
{
    musicMenu.stop();
    musicGame.stop();
    musicGame.setVolume((float)masterVolume);
    musicGame.play();

    scoreVal = 0;
    livesLeft = 3;
    playerDead = false;

    levelVal = 1;
    diveChance = 2;
    enemySpeed = 70.0f;
    
    setupEnemies();
    initPlayerBullets();
    initEnemyBullets();

    // Reset explosions
    for (int i = 0; i < MAX_EXPL; ++i) explActive[i] = false;

    playerX = PLAYER_START_X;
    playerY = PLAYER_START_Y;
    gameState = 1;
    frameClock.restart();
    enemyFireTimer = 0.0f;

    // Spawn protection
    isInvincible = true;
    invincibilityTimer = 0.0f;

    // whenever we start a new game, remove the old save file and make it so there's no file
    remove(SAVE_FILE);
    //saveGameState()
}

void togglePause()
{
    if (gameState == 1) gameState = 2;
    else if (gameState == 2) gameState = 1;
}

// finally the main function itself where we'll be implementing everything
int main()
{
    srand((unsigned)time(NULL));
    //making the window
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Galaga");
    window.setFramerateLimit(60); // 60 fps limit, no more than 60 warna kharabiyan ajani
    if (gameFont.loadFromFile("assets/font.ttf")) {
        fontLoaded = true;
    }
    initAll();

    frameClock.restart(); // restart clock

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) // basic sfml shi, this is the main game loop where we update everything every single frame
        {
            if (event.type == sf::Event::Closed) { // if we close the window, then we use these functions, the savehighscore one to save the file 
                saveHighScores(); window.close();  musicMenu.stop(); musicGame.stop();

                if (gameState == 1 || gameState == 2) { // if the user was in game while playing then save the game state so it continues from the same side
                    if (!(livesLeft <= 0)) { saveGameState(); }
                }
            }
            // Autopause feature, this is an edge case basically if the TA switches the windows then the game automatically pauses
            if (event.type == sf::Event::LostFocus && gameState == 1) {
                togglePause();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (gameState == 0) // Main Menu
                {
                    if (event.key.code == sf::Keyboard::Enter) {
                        // Check karo agar save file hai to prompt dikhao
                        if (checkSaveFileExists()) {
                            gameState = 5; // Load Prompt
                        }
                        else {
                            startNewGame();
                        }
                    }
                    if (event.key.code == sf::Keyboard::S) gameState = 4;
                    if (event.key.code == sf::Keyboard::Q) { saveHighScores(); window.close(); }
                }
                else if (gameState == 1) // Playing (gameplay state)
                {
                    if (event.key.code == sf::Keyboard::P) { togglePause(); saveGameState(); }
                    if (event.key.code == sf::Keyboard::U) saveGameState();

                    if (event.key.code == sf::Keyboard::Escape)
                    {
                        saveGameState();
                        gameState = 0; // go back to menu 
                        musicGame.stop();
                        musicMenu.setVolume((float)masterVolume);
                        musicMenu.play();
                    }
                }
                else if (gameState == 2) // pause
                {
                    if (event.key.code == sf::Keyboard::P) togglePause();
                    // Pause mein bhi save kar sakte ho cuz ainnoway we gon press U while fighting for our lives against the enemy bullets
                    if (event.key.code == sf::Keyboard::U) saveGameState();
                    if (event.key.code == sf::Keyboard::Escape) {
                        saveGameState();
                        gameState = 0; // go back to menu 
                        musicGame.stop();
                        musicMenu.setVolume((float)masterVolume);
                        musicMenu.play();
                    }
                }
                else if (gameState == 3) // game over
                {
                    if (event.key.code == sf::Keyboard::Enter) {
                        gameState = 0;
                        musicGame.stop();
                        musicMenu.setVolume((float)masterVolume);
                        musicMenu.play();
                    }
                }
                else if (gameState == 4) // setting screen
                {
                    if (event.key.code == sf::Keyboard::Num1) difficultyLevel = 0;
                    if (event.key.code == sf::Keyboard::Num2) difficultyLevel = 1;
                    if (event.key.code == sf::Keyboard::Num3) difficultyLevel = 2;

                    // volume control here, up and down arrow key sei volume adjust kar sakte
                    if (event.key.code == sf::Keyboard::Up) {
                        masterVolume += 10;
                        if (masterVolume > 100) masterVolume = 100;
                        // update it as soon as you get the values
                        musicMenu.setVolume((float)masterVolume);
                        musicGame.setVolume((float)masterVolume);
                    }
                    if (event.key.code == sf::Keyboard::Down) {
                        masterVolume -= 10;
                        if (masterVolume < 0) masterVolume = 0;
                        musicMenu.setVolume((float)masterVolume);
                        musicGame.setVolume((float)masterVolume);
                    }

                    if (event.key.code == sf::Keyboard::Escape) gameState = 0;
                }
                else if (gameState == 5) // this is the load prompt, where we ask whether they want to continue their old saved game or start a new game
                { // Y key sei load hota N key sei new game and ESC sei back to main menu
                    if (event.key.code == sf::Keyboard::Y) {
                        loadGameState();
                    }
                    if (event.key.code == sf::Keyboard::N) {
                        startNewGame();
                    }
                    if (event.key.code == sf::Keyboard::Escape) {
                        gameState = 0;
                    }
                }
            }
        }

        float dt = frameClock.restart().asSeconds();
        // cap the delta time so that the window dragging and resizing doesn't affect us, the delta time is capped at 0.1f
        if (dt > 0.1f) dt = 0.1f;

        if (gameState == 1) updateGame(dt);

        window.clear(sf::Color::Black);

        // here we are doing the sfml stuff regarding game states, using functions with respect to whatever game state is on right now
        if (gameState == 0) showStartMenu(window, gameFont);
        else if (gameState == 1) drawGame(window);
        else if (gameState == 2)
        {
            drawGame(window);
            showPauseOverlay(window, gameFont);
        }
        else if (gameState == 3) showGameOverScreen(window, gameFont, scoreVal, highScores[0]);
        else if (gameState == 4) showSettingsScreen(window, gameFont, difficultyLevel);
        else if (gameState == 5) showLoadPrompt(window, gameFont);

        window.display();
    }
}

// masla hi nai koi itna easy tou hei na


