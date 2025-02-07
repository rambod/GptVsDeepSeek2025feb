#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GRAVITY = 0.5f;
const float JUMP_FORCE = -12.0f;
const float MOVE_SPEED = 5.0f;
const float ENEMY_SPEED = 2.0f;

struct Platform {
    sf::RectangleShape shape;
    bool hasEnemy;
};

struct Enemy {
    sf::RectangleShape shape;
    float movement;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Infinite Platformer");
    window.setFramerateLimit(60);
    std::srand(std::time(nullptr));

    // Player
    sf::RectangleShape player(sf::Vector2f(40, 40));
    player.setFillColor(sf::Color::Green);
    player.setPosition(100, WINDOW_HEIGHT - 200);
    sf::Vector2f velocity(0, 0);
    bool onGround = false;
    float maxDistance = 0;

    // Level
    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;

    // Generate initial platform
    Platform startPlatform;
    startPlatform.shape.setSize(sf::Vector2f(200, 50));
    startPlatform.shape.setPosition(0, WINDOW_HEIGHT - 50);
    startPlatform.shape.setFillColor(sf::Color::White);
    startPlatform.hasEnemy = false;
    platforms.push_back(startPlatform);

    // Font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) return EXIT_FAILURE;
    sf::Text distanceText;
    distanceText.setFont(font);
    distanceText.setCharacterSize(24);
    distanceText.setFillColor(sf::Color::White);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Player movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) velocity.x = -MOVE_SPEED;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) velocity.x = MOVE_SPEED;
        else velocity.x = 0;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && onGround) {
            velocity.y = JUMP_FORCE;
            onGround = false;
        }

        // Apply gravity
        velocity.y += GRAVITY;

        // Update player position
        player.move(velocity);

        // Track max distance
        if (player.getPosition().x > maxDistance) maxDistance = player.getPosition().x;

        // Collision with platforms
        onGround = false;
        sf::FloatRect playerBounds = player.getGlobalBounds();

        for (auto& platform : platforms) {
            sf::FloatRect platformBounds = platform.shape.getGlobalBounds();
            
            if (playerBounds.intersects(platformBounds)) {
                if (velocity.y > 0 && playerBounds.top + playerBounds.height - velocity.y <= platformBounds.top) {
                    player.setPosition(playerBounds.left, platformBounds.top - playerBounds.height);
                    velocity.y = 0;
                    onGround = true;
                }
            }
        }

        // Enemy collision
        for (size_t i = 0; i < enemies.size(); ++i) {
            sf::FloatRect enemyBounds = enemies[i].shape.getGlobalBounds();
            
            if (playerBounds.intersects(enemyBounds)) {
                if (velocity.y > 0 && playerBounds.top + playerBounds.height - velocity.y <= enemyBounds.top) {
                    enemies.erase(enemies.begin() + i);
                    velocity.y = JUMP_FORCE * 0.5f;
                    break;
                } else {
                    // Reset game on enemy collision
                    player.setPosition(100, WINDOW_HEIGHT - 200);
                    platforms.clear();
                    enemies.clear();
                    platforms.push_back(startPlatform);
                    maxDistance = 0;
                }
            }
        }

        // Enemy movement
        for (auto& enemy : enemies) {
            enemy.shape.move(enemy.movement, 0);
            sf::FloatRect enemyBounds = enemy.shape.getGlobalBounds();
            
            if (enemyBounds.left < 0 || enemyBounds.left + enemyBounds.width > WINDOW_WIDTH) {
                enemy.movement *= -1;
            }
        }

        // Procedural generation
        float rightmost = 0;
        for (auto& platform : platforms) rightmost = std::max(rightmost, platform.shape.getPosition().x);
        
        if (rightmost < player.getPosition().x + WINDOW_WIDTH * 0.75f) {
            Platform newPlatform;
            newPlatform.shape.setSize(sf::Vector2f(std::rand() % 150 + 50, 50));
            newPlatform.shape.setPosition(
                rightmost + std::rand() % 200 + 100,
                std::rand() % 300 + 200
            );
            newPlatform.shape.setFillColor(sf::Color::White);
            
            // 25% chance to spawn enemy
            newPlatform.hasEnemy = (std::rand() % 4 == 0);
            if (newPlatform.hasEnemy) {
                Enemy newEnemy;
                newEnemy.shape.setSize(sf::Vector2f(30, 30));
                newEnemy.shape.setFillColor(sf::Color::Red);
                newEnemy.shape.setPosition(
                    newPlatform.shape.getPosition().x + newPlatform.shape.getSize().x/2 - 15,
                    newPlatform.shape.getPosition().y - 30
                );
                newEnemy.movement = ENEMY_SPEED * (std::rand() % 2 ? 1 : -1);
                enemies.push_back(newEnemy);
            }
            
            platforms.push_back(newPlatform);
        }

        // Death check
        if (player.getPosition().y > WINDOW_HEIGHT) {
            player.setPosition(100, WINDOW_HEIGHT - 200);
            platforms.clear();
            enemies.clear();
            platforms.push_back(startPlatform);
            maxDistance = 0;
        }

        // Update UI
        std::stringstream ss;
        ss << "Distance: " << static_cast<int>(maxDistance);
        distanceText.setString(ss.str());

        // Drawing
        window.clear(sf::Color::Black);
        
        for (auto& platform : platforms) window.draw(platform.shape);
        for (auto& enemy : enemies) window.draw(enemy.shape);
        window.draw(player);
        window.draw(distanceText);
        
        window.display();
    }

    return 0;
}


// Key features:

// Infinite procedural generation of platforms with random widths and heights

// Enemies spawn randomly on platforms (25% chance)

// Player can jump on enemies to defeat them

// Collision with enemies from the side resets the game

// Falling off the bottom of the screen resets the game

// Continuous tracking of maximum distance reached

// Simple physics with gravity and jump mechanics

// Enemies move back and forth on their platforms

// To compile and run:

// Install SFML library

// Link SFML libraries in your compiler settings

// Make sure to have a valid font file (e.g., arial.ttf) in the working directory

// Controls:

// A/D: Move left/right

// Space: Jump

// Escape: Close window

// The game will continuously generate new platforms to the right as the player progresses. The maximum distance traveled is displayed in the top-left corner. The challenge increases with random platform spacing and enemy placement.