#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        std::cout << "Error initializing SDL2: " << SDL_GetError() << std::endl;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if ((window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 600, window_flags)) == NULL)
        std::cout << "Error creating window: " << SDL_GetError() << std::endl;

    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL)
        std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;

    bool stop = false;
    while (!stop) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    stop = SDL_TRUE;
                break;
            }
        }

        SDL_RenderClear(renderer);



        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}