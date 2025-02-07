#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iostream>
#include <algorithm>

// --- Game Constants ---
const int WINDOW_WIDTH   = 800;
const int WINDOW_HEIGHT  = 600;

const float GRAVITY      = 1000.f;
const float PLAYER_SPEED = 300.f;
const float JUMP_SPEED   = -500.f;

const float PLAYER_SIZE      = 30.f;
const float ENEMY_SIZE       = 30.f;
const float PLATFORM_HEIGHT  = 20.f;
const float INITIAL_PLATFORM_Y = 500.f;  // baseline Y for initial platform

// --- Structures ---
struct Platform {
    sf::RectangleShape shape;
};

struct Enemy {
    sf::RectangleShape shape;
    bool alive;
};

struct Player {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    bool canJump;
};

// --- Game Reset Function ---
// Resets the game state: player position/velocity, clears platforms/enemies, and resets generation.
void resetGame(Player &player,
               std::vector<Platform> &platforms,
               std::vector<Enemy> &enemies,
               float &lastPlatformEndX,
               float &maxDistance)
{
    // Reset player
    player.shape.setPosition(100.f, 300.f);
    player.velocity = sf::Vector2f(0.f, 0.f);
    player.canJump = false;
    
    // Clear old platforms and enemies
    platforms.clear();
    enemies.clear();
    
    // Reset platform generation state:
    lastPlatformEndX = 150.f; // start with an initial platform covering starting area
    maxDistance = 100.f;
    
    // Create the initial platform from x=0 to x=lastPlatformEndX
    Platform initialPlat;
    initialPlat.shape.setSize(sf::Vector2f(lastPlatformEndX, PLATFORM_HEIGHT));
    initialPlat.shape.setFillColor(sf::Color::Green);
    initialPlat.shape.setPosition(0.f, INITIAL_PLATFORM_Y);
    platforms.push_back(initialPlat);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D Platformer");
    window.setFramerateLimit(60);
    
    // Seed random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    // --- Initialize Player ---
    Player player;
    player.shape.setSize(sf::Vector2f(PLAYER_SIZE, PLAYER_SIZE));
    player.shape.setFillColor(sf::Color::Blue);
    player.shape.setPosition(100.f, 300.f);
    player.velocity = sf::Vector2f(0.f, 0.f);
    player.canJump = false;
    
    // --- Containers for Platforms and Enemies ---
    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;
    
    float lastPlatformEndX = 150.f;  // right end of the last generated platform
    float maxDistance = 100.f;       // track the farthest x reached
    
    // Create an initial platform so the player has something to stand on.
    Platform initialPlat;
    initialPlat.shape.setSize(sf::Vector2f(lastPlatformEndX, PLATFORM_HEIGHT));
    initialPlat.shape.setFillColor(sf::Color::Green);
    initialPlat.shape.setPosition(0.f, INITIAL_PLATFORM_Y);
    platforms.push_back(initialPlat);
    
    // --- Setup the View (Camera) ---
    sf::View view(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
    
    // --- For Timing ---
    sf::Clock clock;
    
    // --- Distance Text (requires a font file "arial.ttf") ---
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        std::cout << "Failed to load font (arial.ttf). Make sure the file exists in your working directory." << std::endl;
        // The game will run without text if the font isn’t loaded.
    }
    sf::Text distanceText;
    distanceText.setFont(font);
    distanceText.setCharacterSize(20);
    distanceText.setFillColor(sf::Color::White);
    
    // --- Main Game Loop ---
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        
        // --- Event Handling ---
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        // --- Input Handling ---
        player.velocity.x = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            player.velocity.x = -PLAYER_SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            player.velocity.x = PLAYER_SPEED;
        // Allow jumping if the player is on a platform
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && player.canJump)
        {
            player.velocity.y = JUMP_SPEED;
            player.canJump = false;
        }
        
        // --- Physics: Apply Gravity ---
        player.velocity.y += GRAVITY * dt;
        
        // --- Update Player Position ---
        player.shape.move(player.velocity * dt);
        sf::FloatRect playerBounds = player.shape.getGlobalBounds();
        
        // --- Collision with Platforms ---
        // Reset jump flag; we will set it to true if a landing occurs.
        player.canJump = false;
        for (auto &plat : platforms)
        {
            sf::FloatRect platBounds = plat.shape.getGlobalBounds();
            if (playerBounds.intersects(platBounds))
            {
                // Check if collision is coming from above.
                float playerBottom = playerBounds.top + playerBounds.height;
                float platTop = platBounds.top;
                // A tolerance of 5 pixels (and using previous frame position estimate)
                if (player.velocity.y > 0 && (playerBottom - player.velocity.y * dt) <= platTop + 5.f)
                {
                    // Snap the player to the top of the platform and cancel downward velocity.
                    player.shape.setPosition(player.shape.getPosition().x, platTop - PLAYER_SIZE);
                    player.velocity.y = 0;
                    player.canJump = true;
                    playerBounds = player.shape.getGlobalBounds();
                }
            }
        }
        
        // --- Collision with Enemies ---
        for (auto &enemy : enemies)
        {
            if (!enemy.alive)
                continue;
            if (playerBounds.intersects(enemy.shape.getGlobalBounds()))
            {
                sf::FloatRect enemyBounds = enemy.shape.getGlobalBounds();
                float playerBottom = playerBounds.top + playerBounds.height;
                // If the player is falling and hits near the top of the enemy, defeat the enemy.
                if (player.velocity.y > 0 && (playerBottom - player.velocity.y * dt) <= enemyBounds.top + 10.f)
                {
                    enemy.alive = false;
                    player.velocity.y = JUMP_SPEED / 2;  // Bounce upward slightly.
                }
                else
                {
                    // Otherwise, treat it as a fatal collision – reset the game.
                    resetGame(player, platforms, enemies, lastPlatformEndX, maxDistance);
                    break;
                }
            }
        }
        
        // --- Update Maximum Distance ---
        if (player.shape.getPosition().x > maxDistance)
            maxDistance = player.shape.getPosition().x;
        
        // --- Procedural Generation of Platforms and Enemies ---
        // Generate ahead of the view (with a margin of 200 pixels).
        float viewRight = view.getCenter().x + WINDOW_WIDTH / 2;
        while (lastPlatformEndX < viewRight + 200.f)
        {
            // Random gap: between 50 and 150 pixels.
            float gap = 50.f + std::rand() % 101;
            // Random width: between 100 and 300 pixels.
            float width = 100.f + std::rand() % 201;
            float newX = lastPlatformEndX + gap;
            // Vary the platform’s vertical position relative to the last one.
            float prevY = platforms.back().shape.getPosition().y;
            float newY = prevY + (std::rand() % 61 - 30); // variation from -30 to +30
            newY = std::max(300.f, std::min(newY, 550.f)); // clamp Y value
            Platform newPlat;
            newPlat.shape.setSize(sf::Vector2f(width, PLATFORM_HEIGHT));
            newPlat.shape.setFillColor(sf::Color::Green);
            newPlat.shape.setPosition(newX, newY);
            platforms.push_back(newPlat);
            
            // With about a 30% chance, add an enemy on the platform if the width is sufficient.
            if (width > 150 && (std::rand() % 100) < 30)
            {
                Enemy enemy;
                enemy.alive = true;
                enemy.shape.setSize(sf::Vector2f(ENEMY_SIZE, ENEMY_SIZE));
                enemy.shape.setFillColor(sf::Color::Red);
                // Place the enemy at a random horizontal position on the platform.
                float enemyX = newX + std::rand() % static_cast<int>(width - ENEMY_SIZE);
                float enemyY = newY - ENEMY_SIZE;
                enemy.shape.setPosition(enemyX, enemyY);
                enemies.push_back(enemy);
            }
            
            lastPlatformEndX = newX + width;
        }
        
        // --- Remove Off-screen Platforms and Enemies ---
        float viewLeft = view.getCenter().x - WINDOW_WIDTH / 2 - 200;
        platforms.erase(std::remove_if(platforms.begin(), platforms.end(),
            [viewLeft](const Platform &p) {
                return (p.shape.getPosition().x + p.shape.getSize().x < viewLeft);
            }),
            platforms.end());
        
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
            [viewLeft](const Enemy &e) {
                return ((e.shape.getPosition().x + e.shape.getSize().x < viewLeft) || (!e.alive));
            }),
            enemies.end());
        
        // --- Check if the Player Falls Off-Screen ---
        if (player.shape.getPosition().y > view.getCenter().y + WINDOW_HEIGHT/2)
            resetGame(player, platforms, enemies, lastPlatformEndX, maxDistance);
        
        // --- Update the View to Follow the Player Horizontally ---
        sf::Vector2f viewCenter = view.getCenter();
        // Offset the view so the player is more to the left of the screen.
        viewCenter.x = player.shape.getPosition().x + WINDOW_WIDTH / 4;
        view.setCenter(viewCenter);
        
        // --- Update Distance Text ---
        std::stringstream ss;
        ss << "Max Distance: " << static_cast<int>(maxDistance);
        distanceText.setString(ss.str());
        // Position the text in the top-left of the view.
        distanceText.setPosition(view.getCenter().x - WINDOW_WIDTH/2 + 10,
                                   view.getCenter().y - WINDOW_HEIGHT/2 + 10);
        
        // --- Draw Everything ---
        window.clear(sf::Color::Black);
        window.setView(view);
        
        // Draw platforms
        for (auto &plat : platforms)
            window.draw(plat.shape);
        
        // Draw enemies
        for (auto &enemy : enemies)
            if (enemy.alive)
                window.draw(enemy.shape);
        
        // Draw the player
        window.draw(player.shape);
        
        // Draw the distance counter
        window.draw(distanceText);
        
        window.display();
    }
    
    return 0;
}


// How It Works
// Player Movement & Physics:
// The player’s velocity is updated each frame with gravity. Horizontal movement is controlled by the left/right arrow keys, and a jump is triggered with the space bar (only if the player “lands” on a platform).

// Platform Collision:
// A simple collision check puts the player on top of any platform it lands on and resets vertical velocity. This is only done if the player is falling and hits near the top of the platform.

// Enemies:
// Enemies are placed on platforms with roughly a 30% chance. When the player collides with an enemy, the code checks if the collision is from above. If so, the enemy is “defeated” (marked dead) and the player bounces. Otherwise, the game resets.

// Procedural Generation:
// New platforms (and possibly enemies) are generated ahead of the view as the player moves right. Old platforms/enemies that have scrolled off-screen are removed.

// Maximum Distance:
// The game records and displays the furthest horizontal distance reached by the player.

