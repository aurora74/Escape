#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <vector>
#include <fstream>

#include "Box.h"
#include "Text.h"
#include "Tile.h"

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;

const int CAMERA_PADDING = 275;

const int EMPTY_TILE = 0;
const int GROUND_TILE = 1;
const int END_TILE = 2;
const int SPAWN_TILE = 3;

const SDL_Color SDL_COLOR_BLACK = { 0, 0, 0, 255 }; 
const SDL_Color SDL_COLOR_WHITE = { 255, 255, 255, 255 };
const SDL_Color SDL_COLOR_GREEN = { 72, 255, 39, 255 };

short int fpsCap = 60;

SDL_Window* mainWindow;
SDL_Renderer* renderer;


SDL_Texture* bgTexture;
int bgTextureHeight;
int bgTextureWidth;


SDL_Texture* boxTexture;
SDL_Texture* tileTexture;
SDL_Texture* endTileTexture;
TTF_Font* globalFont;
TTF_Font* timerFont;
TTF_Font* sofachrome;


const int NUMBER_OF_LEVELS = 5; 
int currentLevelIndex = 0;
std::vector<std::string> levelNames;
std::vector<Tile>* currentLevelTiles; 
std::vector<Tile> levelTilesets[NUMBER_OF_LEVELS]; 
std::vector<std::pair<int, int>> levelSpawnPoints;
int playerStartX, playerStartY;


Uint32 timeOfStartEntireGameRTA = 0; // for entire run of whole game
bool completedRTA = false;
Uint32 timeOfStart = 0; // start of timer in level
int timeOfFinish = -1; // time when the user finished this level

bool gameComplete = false;

bool debug = false;

Mix_Music* music = NULL;
Mix_Chunk* hit1 = NULL;
Mix_Chunk* hit2 = NULL;
Mix_Chunk* hit3 = NULL;
Mix_Chunk* hitsounds[3];

bool started = false;

void close() 
{
    SDL_DestroyTexture(boxTexture);
    SDL_DestroyTexture(tileTexture);
    SDL_DestroyTexture(endTileTexture);
    SDL_DestroyTexture(bgTexture);

    TTF_CloseFont(globalFont);
    TTF_CloseFont(timerFont);
    TTF_CloseFont(sofachrome);
    TTF_Quit();

    Mix_FreeChunk(hit1);
    Mix_FreeChunk(hit2);
    Mix_FreeChunk(hit3);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    Mix_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(mainWindow);

    boxTexture = NULL;
    tileTexture = NULL;
    endTileTexture = NULL;
    bgTexture = NULL;

    currentLevelTiles = NULL;

    globalFont = NULL;
    timerFont = NULL;

    music = NULL;
    hit1 = NULL;

    renderer = NULL;
    mainWindow = NULL;

}

// Loading all assets into SDL_Texture.
bool loadAssets() 
{
    bool success = true;

    boxTexture = IMG_LoadTexture(renderer, "assets/img/box_final5.png");
    if (boxTexture == NULL) 
    {
        std::cout << "Couldn't load box texture. " << IMG_GetError();
        success = false;
    }

    tileTexture = IMG_LoadTexture(renderer, "assets/img/tile_final.png");
    if (tileTexture == NULL) 
    {
       std::cout << "Couldn't load tile texture. " << IMG_GetError();
        success = false;
    }

    endTileTexture = IMG_LoadTexture(renderer, "assets/img/end2.png");
    if (tileTexture == NULL) 
    {
        std::cout << "Couldn't load tile texture. " << IMG_GetError();
        success = false;
    }

    bgTexture = IMG_LoadTexture(renderer, "assets/img/warehouse_big.png");
    if (bgTexture == NULL) 
    {
        std::cout << "Couldn't load box texture. " << IMG_GetError();
    }

    // Store its width/height for scrolling purposes.
    SDL_QueryTexture(bgTexture, NULL, NULL, &bgTextureWidth, &bgTextureHeight);


    globalFont = TTF_OpenFont("assets/fonts/Lato-Black.ttf", 26);
    if (globalFont == NULL) 
    {
        std::cout << "Failed to load lato black: " << TTF_GetError() << "\n";
        return false;
    }

    timerFont = TTF_OpenFont("assets/fonts/Lato-Black.ttf", 30);
    if (timerFont == NULL) 
    {
        std::cout << "Failed to load lato black: " << TTF_GetError() << "\n";
        return false;
    }

    sofachrome = TTF_OpenFont("assets/fonts/sofachrome.otf", 46);
    if (sofachrome == NULL) 
    {
        std::cout << "Failed to load sofachrome font: " << TTF_GetError() << "\n";
        return false;
    }

    music = Mix_LoadMUS("assets/sounds/phoon.wav");
    if (music == NULL) 
    {
        std::cout << "Failed to load music. " << Mix_GetError() << "\n";
        return false;
    }

    hit1 = Mix_LoadWAV("assets/sounds/hit1.wav");
    if (hit1 == NULL) 
    {
        std::cout << "Failed to load hit1. " << Mix_GetError() << "\n";
        return false;
    }

    hit2 = Mix_LoadWAV("assets/sounds/hit2.wav");
    if (hit1 == NULL) 
    {
        std::cout << "Failed to load hit2. " << Mix_GetError() << "\n";
        return false;
    }

    hit3 = Mix_LoadWAV("assets/sounds/hit3.wav");
    if (hit1 == NULL) 
    {
        std::cout << "Failed to load hit3. " << Mix_GetError() << "\n";
        return false;
    }

    return success;
}

/*
    List of things to do:
    - SDL_Init (no vsync, I do it manually)
    - Create sdl window, renderer, etc.
    - Call loadAssets
    - Add level names to 'levelNames' array
*/

int init() 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        return -1;
    }

    mainWindow = SDL_CreateWindow("Warehouse Escape", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (mainWindow == NULL) 
    {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
        return -1;
    }

    // To do: create renderer

    renderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) 
    {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
        return -1;
    }

    // To do: initialize sdl_image

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) 
    { 
        // To do: look back at lazyfoo to see wtf this is

        std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << "\n";
        return -1;
    }

    if (TTF_Init() == -1) 
    {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << "\n";
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) 
    {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << "\n";
        return -1;
    }

    // Load all textures and fonts

    if (!loadAssets()) 
    {
        return -1;
    }

    levelNames.push_back("01_21_10");
    levelNames.push_back("02_21_19");
    levelNames.push_back("03_42_12");
    levelNames.push_back("04_33_25");
    levelNames.push_back("05_15_35");

    return 0;
}

// Loads level from levels directory by filename (no file extension)

std::vector<Tile> loadLevel(std::string filename) 
{
    std::vector<Tile> tiles;
    int x = 0, y = 0; // tile offsets

    std::ifstream map("levels/" + filename + ".level");

    int width = stoi(filename.substr(3, 2));
    int length = stoi(filename.substr(6, 2));

    if (map.fail()) 
    {
        std::cout << "Unable to load map 1.";
    }
    else 
    {
        for (int i = 0; i < (width * length); i++) 
        {
            int type = -1;

            map >> type;

            if (map.fail()) 
            {
                std:: cout << "Unable to load map 2.";
            }

            if (type == EMPTY_TILE) 
            {
                // Do nothing
            }
            else if (type == GROUND_TILE) 
            {
                tiles.push_back(Tile(renderer, tileTexture, x, y));
            }
            else if (type == END_TILE) 
            {
                tiles.push_back(Tile(renderer, endTileTexture, x, y, true));
            }
            else if (type == SPAWN_TILE) 
            {
                // If there's more than one then this breaks

                std::pair<int, int> pos = { x, y };
                levelSpawnPoints.push_back(pos);
            }

            x += 75;

            // Move to next row if at end of current row (down)

            if (x / 75 > (width - 1)) 
            {
                x = 0;
                y += 75;
            }
        }
    }

    return tiles;
}

// Return current time since the given time param in ticks in timer format

std::string getTimeFormatted(Uint32 time) 
{
    Uint32 ms = (SDL_GetTicks() - time) / 10;
    Uint32 seconds = (SDL_GetTicks() - time) / 1000;
    Uint16 minutes = seconds / 60;
    seconds -= (minutes * 60);

    std::string secondsStr;
    if (seconds > 9) 
    {
        secondsStr = std::to_string(seconds);
    }
    else 
    {
        secondsStr = "0" + std::to_string(seconds);
    }

    std::string msString = std::to_string(ms);

    if (msString.size() > 2) 
    {
        msString = msString.substr(msString.size() - 2);
    }
    else 
    {
        msString = "0" + msString.substr(msString.size() - 1);
    }

    return std::to_string(minutes) + ":" + secondsStr + ":" + msString;
}

bool mainMenuLoop() 
{
    SDL_Event e; 

    // Show title screen

    SDL_Rect menuBg = { 0, 0, bgTextureWidth, bgTextureHeight };
    Text startingText = Text(renderer, sofachrome, SDL_COLOR_WHITE);
    startingText.changeText("Box Escape");
    Text startingText2 = Text(renderer, globalFont, SDL_COLOR_WHITE);
    startingText2.changeText("Press 'Enter' to start");

    while (!started) 
    {
        // Go through event queue
        while (SDL_PollEvent(&e) != 0) 
        {
            switch (e.type) 
            {
            case SDL_QUIT:
                return true;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_RETURN)
                {
                    started = true;
                }
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
        SDL_Rect bg1 = { 0, 0, bgTextureWidth, bgTextureHeight };
        SDL_RenderCopyEx(renderer, bgTexture, NULL, &menuBg, 0, NULL, SDL_FLIP_NONE);
        startingText.render((WINDOW_WIDTH - startingText.getWidth()) / 2, ((WINDOW_HEIGHT - startingText.getHeight()) / 2) - 35);
        startingText2.render((WINDOW_WIDTH - startingText2.getWidth()) / 2, ((WINDOW_HEIGHT - startingText2.getHeight()) / 2) + 35);
        SDL_RenderPresent(renderer);
    }

    startingText.~Text();
    return false;
}

int main(int argc, char* argv[])
{

    int initValue = init();

    if (initValue != 0) 
    {
        std::cout << "Problem when trying to initialize for main loop. error #" << initValue;
        return initValue;
    }

    // Main loop starts
    bool quit = false;

    SDL_Event e; 

    // FPS Stuff
    Uint32 frameStart = 0;

    // How long it took to complete the last frame.
    int frameTimeToComplete = 0;

    // "Delta time"
    // Used to apply physics in terms of time instead of frames.
    float dt = 0.0;

    Uint32 lastPhysicsUpdate = 0;

    float avgFPS = 0.0;

    int countedFrames = 0; // total frames rendered while application running (for avg fps calculation)
    std::stringstream avgFpsStr;

    // UI text to show.

    Text timerText = Text(renderer, timerFont, SDL_COLOR_GREEN);
    Text timerTextRTA = Text(renderer, timerFont, SDL_COLOR_GREEN);

    Text levelBeatenText = Text(renderer, sofachrome, SDL_COLOR_WHITE);
    Text gameDoneText = Text(renderer, globalFont, SDL_COLOR_WHITE);
    Text gameDoneText2 = Text(renderer, globalFont, SDL_COLOR_WHITE);
    Text gameDoneText3 = Text(renderer, globalFont, SDL_COLOR_WHITE);
    Text gameDoneText4 = Text(renderer, globalFont, SDL_COLOR_WHITE);
    Text gameQuit = Text(renderer, globalFont, SDL_COLOR_WHITE);

    levelBeatenText.changeText("That's all the levels");
    gameDoneText.changeText("Thank you for playing");
    gameDoneText4.changeText("Duong Quoc Khanh - CACLC2");
    gameDoneText2.changeText("(Press 'p' key to see debug info for fun)");
    gameDoneText3.changeText("Press 'r' to start a new run");
    gameQuit.changeText("Press 'e' to quit game");

    // Debug text

    Text avgFpsText = Text(renderer, globalFont, SDL_COLOR_BLACK);
    Text msText = Text(renderer, globalFont, SDL_COLOR_BLACK);
    Text boxText = Text(renderer, globalFont, SDL_COLOR_BLACK);
    Text cameraText = Text(renderer, globalFont, SDL_COLOR_BLACK);
    Text velocityText = Text(renderer, globalFont, SDL_COLOR_BLACK);
    Text offsetText = Text(renderer, globalFont, SDL_COLOR_BLACK);


    // Load all level files and put into vector

    for (int i = 0; i < levelNames.size(); i++) 
    {
        levelTilesets[i] = loadLevel(levelNames[i]);
    }
    currentLevelTiles = &levelTilesets[0]; // set current level to first level address
    playerStartX = levelSpawnPoints[0].first;
    playerStartY = levelSpawnPoints[0].second;

    // Camera

    SDL_Rect camera = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };

    int offsetY = 0; // for offsetting the background textures.
    int offsetX = 0;
    int newStartY = 0; // indicates where scrolling should begin again for bg.
    int newStartX = 0;

    // Music

    hitsounds[0] = hit1;
    hitsounds[1] = hit2;
    hitsounds[2] = hit3;
    Mix_VolumeMusic(5);
    Mix_PlayMusic(music, -1);
    Mix_VolumeChunk(hit1, 3);
    Mix_VolumeChunk(hit2, 3);
    Mix_VolumeChunk(hit3, 3);

    bool exited = mainMenuLoop();
    if (exited) 
    {
        quit = true;
    }

    Box box = Box(renderer, boxTexture, playerStartX, playerStartY);
    std::cout<< "Box created";
    timeOfStartEntireGameRTA = SDL_GetTicks();
    timeOfStart = SDL_GetTicks();
    lastPhysicsUpdate = SDL_GetTicks();

    // Start main game loop
    while (!quit) 
    {
        frameStart = SDL_GetTicks(); // mark time(in m/s) at start of this frame

        // Go through event queue
        while (SDL_PollEvent(&e) != 0) 
        {
            switch (e.type) 
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_p) 
                {
                    debug = !debug;
                }
                else if (e.key.keysym.sym == SDLK_e)
                {
                    quit = true;
                }
                else if (e.key.keysym.sym == SDLK_r)
                {
                    // Restart run back to level 1.
                    // Reset timers.
                    currentLevelIndex = 0;
                    currentLevelTiles = &levelTilesets[0];
                    playerStartX = levelSpawnPoints[0].first;
                    playerStartY = levelSpawnPoints[0].second;

                    box.xvelocity = 0;
                    box.yvelocity = 0;
                    box.x = playerStartX;
                    box.y = playerStartY;
                    box.completedLevel = false;
                    gameComplete = false;
                    completedRTA = false;
                    timeOfFinish = -1;

                    timeOfStart = SDL_GetTicks();
                    timeOfStartEntireGameRTA = SDL_GetTicks();
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                // If box is clicked, apply force to it.
                int xmousepos = e.button.x;
                int ymousepos = e.button.y;

                int xbox = box.getX() - camera.x;
                int ybox = box.getY() - camera.y;

                // If click was inside the box
                if (xmousepos > xbox && xmousepos < xbox + box.getWidth() &&
                    ymousepos > ybox && ymousepos < ybox + box.getHeight())
                {
                    int numb = rand() % 3;
                    Mix_PlayChannel(-1, hitsounds[numb], 0);

                    // Apply vertical
                    int scaling = 20;

                    float yCenter = ybox + (box.getHeight() / static_cast<float>(2));
                    float yForce = 0.0;

                    if (ymousepos > yCenter) 
                    {
                        // Move up (negative)
                        yForce = -fabs(ymousepos - yCenter);
                    }
                    else if (ymousepos < yCenter) 
                    {
                        // Move down (positive)
                        yForce = fabs(yCenter - ymousepos);
                    }

                    // Apply horizontal
                    float center = xbox + (box.getWidth() / static_cast<float>(2));

                    // Figure out how much force to apply to xvelocity
                    // (Further from the center, the more force)
                    float xForce = 0.0;

                    if (xmousepos < center) 
                    {
                        xForce = ((box.getWidth() / static_cast<float>(2)) + xbox) - xmousepos;
                    }
                    else if (xmousepos > center) 
                    {
                        xForce = -(xmousepos - ((box.getWidth() / static_cast<float>(2)) + xbox));
                    }
                    else 
                    {
                        // Exactly center (should never happen)
                    }

                    box.applyXYVelocity(xForce * scaling, yForce * scaling);
                }

                break;
            }
        }

       
        // (now - last physics update) / 1000 (put into seconds)

        dt = (SDL_GetTicks() - lastPhysicsUpdate) / 1000.0f;
        box.simulatePhysics(dt, *currentLevelTiles);
        lastPhysicsUpdate = SDL_GetTicks();

        if (box.completedLevel) 
        {
            // Stop RTA timer if totally done.

            if (currentLevelIndex + 1 >= NUMBER_OF_LEVELS) 
            {
                completedRTA = true;
            }

            if (timeOfFinish == -1) 
            {
                timeOfFinish = SDL_GetTicks();
                camera.y -= 100;
            }
            else 
            {
                if (timeOfFinish + 2000 < SDL_GetTicks()) 
                {

                    if (!(currentLevelIndex + 1 >= NUMBER_OF_LEVELS)) 
                    {
                        currentLevelIndex++; // go to next level
                        currentLevelTiles = &levelTilesets[currentLevelIndex];
                        box.xvelocity = 0;
                        box.yvelocity = 0;
                        box.x = levelSpawnPoints[currentLevelIndex].first;
                        box.y = levelSpawnPoints[currentLevelIndex].second;
                        box.completedLevel = false;
                        timeOfStart = SDL_GetTicks();
                        timeOfFinish = -1;
                    }
                    else 
                    {
                        // Game completed, show text.
                        box.x = -500;
                        box.y = -500;
                        box.xvelocity = 0;
                        box.yvelocity = 0;

                        gameComplete = true;
                    }
                }
            }

        }
        else 
        {
            // Adjust camera position depending on player pos

            if (box.getY() < camera.y + CAMERA_PADDING) 
            {
                int oldCamY = camera.y;
                camera.y = box.getY() - CAMERA_PADDING;
                offsetY += fabs(camera.y - oldCamY);
            }
            else if (box.y + box.BOX_HEIGHT > camera.y + camera.h - CAMERA_PADDING) 
            {
                int oldCamY = camera.y;
                int newBottom = (box.y + box.BOX_HEIGHT) + CAMERA_PADDING;
                camera.y = newBottom - camera.h;
                offsetY -= fabs(oldCamY - camera.y);
            }

            if (box.x + box.BOX_WIDTH > (camera.x + camera.w) - CAMERA_PADDING) 
            { 
                // Right side
                int oldCamX = camera.x;
                int newRight = (box.x + box.BOX_WIDTH) + CAMERA_PADDING;
                camera.x = newRight - camera.w;
                offsetX += fabs(camera.x - oldCamX);
            }
            else if (box.x < (camera.x + CAMERA_PADDING)) 
            { 
                // Left side
                int oldCamX = camera.x;
                camera.x = box.x - CAMERA_PADDING;
                offsetX -= fabs(oldCamX - camera.x);
            }
        }

        // Calculate avg fps
        avgFPS = countedFrames / (SDL_GetTicks() / 1000.f);
        if (avgFPS > 2000000)
        {
            avgFPS = 0;
        }

        
        // Rendering starts here
        

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        if (offsetY > bgTextureHeight) 
        {
            // When the first bg goes offscreen (offset > bgTextureHeight), 
            // the offset is restarted and the "newStart" (which indicates the new top of bg1) 
            // is set to the camera's y position.

            offsetY = 0;
            newStartY = camera.y;
        }
        else if (offsetY < -bgTextureHeight) 
        {
            offsetY = 0;
            newStartY = camera.y;
        }

        // Do the same in the x-axis
        if (offsetX > bgTextureWidth) 
        {
            offsetX = 0;
            newStartX = camera.x;
        }
        else if (offsetX < -bgTextureWidth) 
        {
            offsetX = 0;
            newStartX = camera.x;
        }


        /*
         * Bg1-4 are four duplicates of the background image.
         * Depending on the direction the player is moving, bg2/4 will
         * display on the left/right of bg1/3.
         * Currently works strictly with camera width/height, which could be changed
         * to just be the w/h of the image loaded.
         */


        SDL_Rect bg1 = { camera.x - (offsetX * 2) - newStartX,
            camera.y + (offsetY * 2) - newStartY,
            bgTextureWidth,
            bgTextureHeight };
        SDL_Rect bg2 = { camera.x - (offsetX * 2) - newStartX, 0, bgTextureWidth, bgTextureHeight };
        SDL_Rect bg3 = { 0, bg1.y, bgTextureWidth, bgTextureHeight };

        if (offsetX >= 0) 
        {
            bg3.x = bg1.x + bgTextureWidth;
        }
        else 
        {
            bg3.x = bg1.x - bgTextureWidth;
        }

        SDL_Rect bg4 = { bg3.x, bg3.y, bgTextureWidth, bgTextureHeight };

        if (offsetY >= 0) 
        {
            bg2.y = bg1.y - bgTextureHeight;
            bg4.y = bg1.y - bgTextureHeight;
        }
        else 
        {
            bg2.y = bg1.y + bgTextureHeight;
            bg4.y = bg1.y + bgTextureHeight;
        }

        SDL_RenderCopyEx(renderer, bgTexture, NULL, &bg1, 0, NULL, SDL_FLIP_NONE);
        SDL_RenderCopyEx(renderer, bgTexture, NULL, &bg2, 0, NULL, SDL_FLIP_NONE);
        SDL_RenderCopyEx(renderer, bgTexture, NULL, &bg3, 0, NULL, SDL_FLIP_NONE);
        SDL_RenderCopyEx(renderer, bgTexture, NULL, &bg4, 0, NULL, SDL_FLIP_NONE);

        // Render level

        for (int i = 0; i < currentLevelTiles->size(); i++) 
        {
            currentLevelTiles->at(i).render(camera.x, camera.y);
        }

        box.render(camera.x, camera.y);

        if (debug) 
        {
            // Update avg fps text

            avgFpsStr.str("");
            avgFpsStr << "Avg FPS: " << avgFPS;
            avgFpsText.changeText(avgFpsStr.str().c_str());
            avgFpsText.render(0, 0);
            msText.changeText("ms render frame: " + std::to_string(SDL_GetTicks() - frameStart));
            msText.render(0, avgFpsText.getHeight());

            std::ostringstream oss;
            oss << "x:" << box.getX() << ", y:" << box.getY();
            boxText.changeText(oss.str());
            boxText.render(WINDOW_WIDTH - boxText.getWidth(), 0);
            std::ostringstream oss2;
            oss2 << "cam | x:" << camera.x << ", y:" << camera.y;
            cameraText.changeText(oss2.str());
            cameraText.render(WINDOW_WIDTH - cameraText.getWidth(), boxText.getHeight());
            std::ostringstream oss3;
            oss3 << "velocity: (" << box.getXVelocity() << ", " << box.getYVelocity() << ")";
            velocityText.changeText(oss3.str());
            velocityText.render(WINDOW_WIDTH - velocityText.getWidth(), boxText.getHeight() + cameraText.getHeight());
            std::ostringstream oss4;
            oss4 << "offsetY:" << offsetY << ", offsetX:" << offsetX;
            offsetText.changeText(oss4.str());
            offsetText.render(WINDOW_WIDTH - offsetText.getWidth(), boxText.getHeight() + cameraText.getHeight() + velocityText.getHeight());
        }

        // Update the timers 

        if (!box.completedLevel) 
        {
            timerText.changeText(getTimeFormatted(timeOfStart));
        }

        if (!completedRTA) 
        {
            timerTextRTA.changeText(getTimeFormatted(timeOfStartEntireGameRTA));
        }

        timerTextRTA.render((WINDOW_WIDTH - timerText.getWidth()) / 2, 10);
        timerText.render((WINDOW_WIDTH - timerText.getWidth()) / 2, timerTextRTA.getHeight() + 10);

        //test git
        if (gameComplete) 
        {
            levelBeatenText.render((WINDOW_WIDTH - levelBeatenText.getWidth()) / 2, ((WINDOW_HEIGHT - levelBeatenText.getHeight()) / 2) - 200);
            gameDoneText.render((WINDOW_WIDTH - gameDoneText.getWidth()) / 2, (WINDOW_HEIGHT + gameDoneText.getHeight() + levelBeatenText.getHeight()) / 2);
            gameDoneText2.render((WINDOW_WIDTH - gameDoneText2.getWidth()) / 2, (WINDOW_HEIGHT - gameDoneText2.getHeight() - 50));
            gameDoneText3.render((WINDOW_WIDTH - gameDoneText3.getWidth()) / 2, (WINDOW_HEIGHT - gameDoneText3.getHeight()) - 100);
            gameDoneText4.render((WINDOW_WIDTH - gameDoneText4.getWidth()) / 2, (WINDOW_HEIGHT - gameDoneText4.getHeight()) - 250);
            gameQuit.render((WINDOW_WIDTH - gameQuit.getWidth()) / 2, (WINDOW_HEIGHT - gameQuit.getHeight()) - 150);
        }
        else if (box.completedLevel) 
        {
            levelBeatenText.changeText("Escaped level " + std::to_string(currentLevelIndex + 1));
            levelBeatenText.render((WINDOW_WIDTH - levelBeatenText.getWidth()) / 2, ((WINDOW_HEIGHT - levelBeatenText.getHeight()) / 2) - 200);
        }


        SDL_RenderPresent(renderer);
        countedFrames++;

        frameTimeToComplete = SDL_GetTicks() - frameStart;
        if (1000 / fpsCap > frameTimeToComplete) 
        {
            SDL_Delay((1000 / fpsCap) - frameTimeToComplete);
        }

    }

    // Clean everything before closing application.

    avgFpsText.~Text();
    msText.~Text();
    boxText.~Text();
    cameraText.~Text();
    velocityText.~Text();
    offsetText.~Text();
    close();

    return 0;

}