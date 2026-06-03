GALAGA CLONE

A retro arcade shooter built in C++ using the Simple and Fast Multimedia Library (SFML).

This project was built under strict academic constraints: Procedural Programming only. No classes, no structs, no std::vector, and no dynamic memory allocation (new/delete). The game logic is entirely driven by parallel arrays and custom object pooling.

 Features

Classic Arcade Action: Defend against grids of descending alien ships.

Dynamic AI Patterns: Enemies don't just move side-to-side; they roll a chance to "dive" at the player using distinct movement algorithms:

Direct Dive: Calculates a normalized vector to kamikaze directly at the player.

Zig-Zag Dive: Sweeps left and right while descending.

Box Spiral: Moves in a hardcoded rectangular spiral pattern.

Frame-Rate Independence: Fully utilizes Delta Time (dt) to ensure game physics and speeds are identical on 30 FPS laptops and 240 FPS gaming rigs.

State Persistence (Save/Load): Save your game mid-battle. The system stores all global variables, enemy positions, and individual AI states into a .txt file to resume later.

Leaderboard: Persistent Top 3 High Scores saved via file handling.

Robust Edge-Case Handling: * Auto-Pause: Game pauses automatically if the window loses focus (Alt-Tab).

Asset Fallbacks: Generates 1x1 pixel scalable blocks if sprite images are deleted/missing.

Sound Pooling: Utilizes an array of sf::Sound objects to prevent audio cutoff when spamming the shoot button.

ontrols

In-Game

A / D or Left / Right Arrows: Move Ship

Spacebar: Shoot

P: Pause Game

U: Save Game (mid-game)

Esc: Return to Main Menu

Menus

Enter: Select / Start

Y / N: Accept/Decline Load prompts

Up / Down Arrows: Adjust Master Volume

1 / 2 / 3: Change Difficulty (Easy, Medium, Hard)

Technical Architecture & Constraints

To adhere to the "No OOP / No Structs / No Vectors" rule, this project uses:

Parallel Arrays: Instead of an Enemy class, we use interconnected global arrays (e.g., enemyX[r][c], enemyY[r][c], enemyState[r][c]). The index links the data together.

Object Pooling: Bullets and explosions are pre-allocated in fixed-size arrays (e.g., pbActive[20]). When firing, the game finds the first inactive index, activates it, and recycles it when it goes off-screen.

Pure C++ File Handling: No C-style FILE* pointers. Uses std::ifstream and std::ofstream for secure, pointer-free sequential file reading and writing.

Installation & Setup

Prerequisites: You must have a C++ compiler (GCC/MinGW or MSVC) and SFML 2.5+ installed.

Assets: Ensure the assets/ folder is in the same directory as the executable. It should contain:

.png files (player.png, enemy1.png, etc.)

.wav and .mp3 files (shoot.wav, music_game.mp3, etc.)

.ttf font file (font.ttf)

Compile: Link against the required SFML modules: sfml-graphics, sfml-window, sfml-system, sfml-audio.

Note: If an asset is missing, the game will not crash. It will utilize standard geometric colored blocks as fallbacks.
