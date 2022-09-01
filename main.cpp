#include <iostream>
#include <filesystem>
#include <ctime>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "libraw/libraw.h"

struct file {
    std::string name;
    time_t timestamp;
};

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Specify a path." << std::endl << "Usage: file-timeline-viewer <path>" << std::endl;
        return 0;
    }
    if (argc > 2) {
        std::cout << "Specify only one argument, a path." << std::endl << "Usage: file-timeline-viewer <path>" << std::endl;
        return 0;
    }

    std::string path(argv[1]);

    if (!std::filesystem::exists(path)) {
        std::cout << "Specified path does not exist." << std::endl << "Usage: file-timeline-viewer <path>" << std::endl;
        return 0;
    }
    if (!std::filesystem::is_directory(path)) {
        std::cout << "Specified path is not a directory." << std::endl << "Usage: file-timeline-viewer <path>" << std::endl;
        return 0;
    }
    
    std::vector<file> files;
    LibRaw iProcessor;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (!std::filesystem::is_regular_file(entry.path())) continue;
        file f;
        f.name = entry.path().filename();
        // ext fs's don't have creation date, well fuck, its all up to the RAW header to have a Shoot date, so this will be limited by the file and by what libraw can do now

        int res = 0;
        if ((res = iProcessor.open_file(entry.path().c_str())) != LIBRAW_SUCCESS) {
            if (res > 0) std::cout << "Error opening file " << f.name << ": " << strerror(res) << std::endl;
            else std::cout << "LibRaw error in file " << f.name << ": " << LibRaw::strerror(res) << std::endl;
            continue;
        }

        f.timestamp = iProcessor.imgdata.other.timestamp;

        // debug print timestamps
        std::tm tm = *std::localtime(&f.timestamp);
        std::cout << f.name << "\t" << std::put_time(&tm, "%d-%m-%Y %T") << std::endl;

        files.push_back(f);
        iProcessor.recycle();
    }




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