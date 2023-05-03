#pragma once
#include <SDL.h>
class Box;
#include "Box.h"

//
class Tile 
{
public:

    // Constructor that sets x y cords of the tile in worldspace.
    Tile(SDL_Renderer* renderer, SDL_Texture* texture, float x, float y, bool finish = false);
    ~Tile();

    // Render this Tile relative to the camera's position.
    void render(int xcam, int ycam);

    // Given (x,y) of Box, returns if it is colliding
    // with this Tile.
    bool isColliding(Box* box);
    bool isColliding(float x, float y, int width, int height);

    static const int TILE_WIDTH = 75;
    static const int TILE_HEIGHT = 75;

    SDL_Texture* texture;

    float x, y;

    SDL_Rect rect;
    SDL_Renderer* renderer;

    bool finish;
};