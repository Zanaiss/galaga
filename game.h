#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>

void initAll();
void updateGame(float dt);
void drawGame(sf::RenderWindow& window);
void startNewGame();
void togglePause();

#endif
