#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>

void showStartMenu(sf::RenderWindow& window, sf::Font& font);
void showPauseOverlay(sf::RenderWindow& window, sf::Font& font);
void showGameOverScreen(sf::RenderWindow& window, sf::Font& font, int score, int highScore);
void showSettingsScreen(sf::RenderWindow& window, sf::Font& font, int difficulty, int endlessMode);

#endif
