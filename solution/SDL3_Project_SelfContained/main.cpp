#include <iostream>
#include <vector>
#include <SDL3/SDL.h>
#include "rectangle.h"
#include "circle.h"

//define window dimensions
constexpr int WINDOW_WIDTH{ 700 };
constexpr int WINDOW_HEIGHT{ 800 };

// Vertical scrolling grid related variables
constexpr float VERTICAL_SCROLLING_SPEED{ 100 };
constexpr int VERTICAL_SCROLLING_GRID_SQUARE_SIZE{ 20 };
std::vector<Rectangle> scrollingGrid;

// Animation related variables
float DT{ 0.01f };
Uint64 startTime{ 0 };
Uint64 endTime{ 0 };

//define SDL Window related variables
SDL_Window* window{ nullptr };
SDL_Renderer* renderer{ nullptr };
SDL_Event currentEvent;
SDL_Color backgroundColor{ 255, 255, 255, 255 };

bool quit{ false };

float mouseX{ -1.0f }, mouseY{ -1.0f };

float displayScale{ 1.0f };

//green rectangle player
glm::vec2 player_vec = { WINDOW_WIDTH / 2.0f - 25.0f, WINDOW_HEIGHT - 75.0f };
SDL_Color player_color = { 0, 255, 0, 255 };

Rectangle player_rec = Rectangle(50, 50, player_vec, player_color);

//orange rectangle enemies
int enemy1Speed = (50 + std::rand() % 101);
int enemy2Speed = (50 + std::rand() % 101);
int enemy3Speed = (50 + std::rand() % 101);

SDL_Color enemyColor = { 255, 165, 0 , 255 };
glm::vec2 enemy1Pos = { 700.f / 6.f, 0.f };
glm::vec2 enemy2Pos = { 700.f / 3.f + 700.f / 6.f, 0.f };
glm::vec2 enemy3Pos = { 2 * (700.f / 3.f) + 700.f / 6.f, 0.f };

Rectangle enemy1Rec = { 50.f, 50.f, enemy1Pos, enemyColor };
Rectangle enemy2Rec = { 50.f, 50.f, enemy2Pos, enemyColor };
Rectangle enemy3Rec = { 50.f, 50.f, enemy3Pos, enemyColor };

//magenta circle projectile
SDL_Color bulletColor = { 255, 0, 0, 255 };

Circle bullet = { 10.f, enemy1Pos, bulletColor };

void initScollingRectangles() {
    int numGridRectangles = (WINDOW_HEIGHT / VERTICAL_SCROLLING_GRID_SQUARE_SIZE) / 2 + 2;

    for (int i = 0; i < numGridRectangles; i++) {
        Rectangle newGridRectangleLeft, newGridRectangleRight;

        newGridRectangleLeft.width = newGridRectangleLeft.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleLeft.pos.x = newGridRectangleLeft.width / 2.0f;
        newGridRectangleLeft.pos.y = WINDOW_HEIGHT - newGridRectangleLeft.height / 2.0f - 2.0f * i * newGridRectangleLeft.height;
        newGridRectangleLeft.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleLeft.color = SDL_Color{ 235, 235, 235, 255 };

        newGridRectangleRight.width = newGridRectangleRight.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleRight.pos.x = WINDOW_WIDTH - newGridRectangleRight.width / 2.0f;
        newGridRectangleRight.pos.y = WINDOW_HEIGHT - newGridRectangleRight.height / 2.0f - 2.0f * i * newGridRectangleRight.height;
        newGridRectangleRight.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleRight.color = SDL_Color{ 235, 235, 235, 255 };

        scrollingGrid.push_back(newGridRectangleLeft);
        scrollingGrid.push_back(newGridRectangleRight);
    }
}

bool initWindow() {
    bool success{ true };

    //Try to initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
        success = false;
    }
    else {
        //Try to create the window and renderer
        displayScale = SDL_GetDisplayContentScale(1);

        if (!SDL_CreateWindowAndRenderer(
            "SDL Hello World Example",
            static_cast<int>(displayScale * WINDOW_WIDTH),
            static_cast<int>(displayScale * WINDOW_HEIGHT),
            0,
            &window, &renderer)) {
            SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
            success = false;
        }
        else {
            //Apply global display scaling to renderer
            SDL_SetRenderScale(renderer, displayScale, displayScale);

            //Set background color
            SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b,
                backgroundColor.a);

            //Apply background color
            SDL_RenderClear(renderer);
        }
    }

    return success;
}

void processEvents() {
    //Check for events in queue
    SDL_HideCursor();
    while (SDL_PollEvent(&currentEvent)) {
        //User requests quit
        if (currentEvent.type == SDL_EVENT_QUIT) {
            quit = true;
        }

        //Mouse event -> pressed button
        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT) {
                SDL_GetMouseState(&mouseX, &mouseY);
                mouseX /= displayScale;
                mouseY /= displayScale;
                std::cout << "Left mouse click => " << "x: " << mouseX << ", y: " << mouseY << std::endl;
            }
        }

        //Mouse event -> mouse movement
        if (currentEvent.type == SDL_EVENT_MOUSE_MOTION) {
           
            player_vec.x = currentEvent.motion.x - 25.0f;
	        player_vec.y = currentEvent.motion.y - 25.0f;

			player_rec.pos = player_vec;                   
       
        }

        //Keyboard event
        if (currentEvent.type == SDL_EVENT_KEY_DOWN) {
            switch (currentEvent.key.key) {
            case SDLK_UP:
                std::cout << "UP arrow key was pressed" << std::endl;
                break;
            case SDLK_DOWN:
                std::cout << "DOWN arrow key was pressed" << std::endl;
                break;
            case SDLK_LEFT:
                std::cout << "LEFT arrow key was pressed" << std::endl;
                break;
            case SDLK_RIGHT:
                std::cout << "RIGHT arrow key was pressed" << std::endl;
                break;
            case SDLK_R:
                std::cout << "R key was pressed" << std::endl;
                break;
            case SDLK_S:
                std::cout << "S key was pressed" << std::endl;
                break;
            case SDLK_ESCAPE:
                quit = true;
                break;
            default:
                break;
            }
        }
    }
}

void animate() {

    // Scrolling grid
    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);

        gridRectangle.pos += gridRectangle.vel * DT;

        if (gridRectangle.pos.y > (WINDOW_HEIGHT + gridRectangle.height / 2.0f)) {
            gridRectangle.pos.y -= gridRectangle.height * scrollingGrid.size();
        }
    }
}

void drawFrame() {
    //Clear the background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    //Draw scene

    //Draw scrolling grid

    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);
        gridRectangle.draw(renderer);
    }

    //Draw player
    player_rec.draw(renderer);

	//Draw enemy
    enemy1Rec.pos.y = (enemy1Rec.pos.y > 800) ? enemy1Pos[1] : enemy1Rec.pos.y += DT * enemy1Speed;
    enemy1Rec.draw(renderer);

    enemy2Rec.pos.y = (enemy2Rec.pos.y > 800) ? enemy2Pos[1] : enemy2Rec.pos.y += DT * enemy2Speed;
    enemy2Rec.draw(renderer);

    enemy3Rec.pos.y = (enemy3Rec.pos.y > 800) ? enemy3Pos[1] : enemy3Rec.pos.y += DT * enemy3Speed;
    enemy3Rec.draw(renderer);

    //Draw projectile
    bullet.draw(renderer);

    //Update window
    SDL_RenderPresent(renderer);
}

void cleanup() {
    //Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    //Destroy window
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    //Quit SDL
    SDL_Quit();
}

int main() {
    //Initialize window
    if (!initWindow()) {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }

    SDL_zero(currentEvent);

    initScollingRectangles();


    //Game loop
    while (!quit) {
        startTime = SDL_GetTicks();

        processEvents();
        animate();
        drawFrame();

        endTime = SDL_GetTicks();
        DT = (endTime - startTime) / 1000.0f;
    }

    cleanup();
    return 0;
}