#pragma once
#include <SDL.h>
#include "Tile.h"
#include <vector>

class Tile;

class Box 
{
public:

    Box(SDL_Renderer* renderer, SDL_Texture* texture, float startingPosX, float startingPosY);
    ~Box(); 

    // Render this box at the specified x,y location
    // Abstracts all the SDL calls for rendering stuff for us.
    // ( Will use SDL_RenderCopyEx() )
    // Render this Box relative to the camera's position.

    void render(int xcam, int ycam);

    int getWidth();
    int getHeight();
    float getX();
    float getY();
    float getXVelocity();
    float getYVelocity();

    void simulatePhysics(float dt, std::vector<Tile>& tiles);

    void applyXYVelocity(float xForce, float yForce);

    static const int BOX_WIDTH = 75;
    static const int BOX_HEIGHT = 75;

    static constexpr float CLICK_VELOCITY = -500.0;

    static constexpr float GRAVITY = 700.0f;
    static constexpr float X_FRICTION = 700.0f;

    SDL_Renderer* renderer;
    SDL_Texture* texture;

    float x, y;
    float xvelocity, yvelocity;

    SDL_Rect rect;

    bool completedLevel = false;
};