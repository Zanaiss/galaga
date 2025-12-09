#ifndef BULLET_H
#define BULLET_H
#include <SFML/Graphics.hpp>
void initPlayerBullets();
void playerShoot(float px);
void updatePlayerBullets(float dt);
void drawPlayerBullets(sf::RenderWindow& window);
void initEnemyBullets();
void spawnEnemyBullet(float x, float y);
void updateEnemyBullets(float dt);
void drawEnemyBullets(sf::RenderWindow& window);
#endif
