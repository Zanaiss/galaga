#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>

// Player functions
void initPlayer();
void updatePlayer(float dt);
void drawPlayer(sf::RenderWindow& window);
bool checkPlayerHitByEnemyOrBullet();
void handleRespawn(float dt);

#endif
