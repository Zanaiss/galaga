#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>

// Enemy functions
void setupEnemies();
void updateEnemies(float dt);
void drawEnemies(sf::RenderWindow& window);
bool allEnemiesDead();
void resetEnemiesForNextWave();

#endif
