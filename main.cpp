#include <iostream>
#include <filesystem>
#include <ctime>
#include <vector>
#include <algorithm>
#include <sstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "libraw/libraw.h"

struct file {
    std::string name;
    time_t timestamp;
};

bool compareFile(const file& a, const file& b) {    // ascending order
    return a.timestamp < b.timestamp;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

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
    
    // ============== loading section ==============

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

        files.push_back(f);
        iProcessor.recycle();
    }

    if (files.size() < 1) {
        std::cout << "No files to show." << std::endl;
        return 0;
    }

    std::sort(files.begin(), files.end(), compareFile);

    // debug print timestamps
    for (const file& f : files) {
        std::tm tm = *std::localtime(&f.timestamp);
        std::cout << f.name << "\t" << std::put_time(&tm, "%d-%m-%Y %T") << std::endl;
    }

    // ============== graphical section ==============

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        std::cout << "Error initializing SDL2: " << SDL_GetError() << std::endl;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if ((window = SDL_CreateWindow("file-timeline-viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 600, window_flags)) == NULL) {
        std::cout << "Error creating window: " << SDL_GetError() << std::endl;
        return 1;
    }

    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
        std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
        return 1;
    }

    float width = 1.0f, height = 1.0f;
    constexpr float margin = 10.0f;
    float time_min = files[0].timestamp;
    float time_max = files[files.size() - 1].timestamp;


    bool stop = false;
    while (!stop) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    stop = SDL_TRUE;
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case 'q': {
                            float center = (time_min + time_max) / 2;
                            float span = (time_max - time_min) * 0.9f;
                            time_min = center - (span / 2);
                            time_max = center + (span / 2);
                        } break;
                        case 'e': {
                            float center = (time_min + time_max) / 2;
                            float span = (time_max - time_min) * 1.1f;
                            time_min = center - (span / 2);
                            time_max = center + (span / 2);
                        } break;
                        case 'a': {
                            float span = time_max - time_min;
                            time_min -= span * 0.1;
                            time_max -= span * 0.1;
                        } break;
                        case 'd': {
                            float span = time_max - time_min;
                            time_min += span * 0.1;
                            time_max += span * 0.1;
                        } break;
                    }
                } break;
            }
        }

        int iwidth = 1, iheight = 1;
        SDL_GetWindowSize(window, &iwidth, &iheight);
        width = iwidth; height = iheight;
        time_t itime_min = time_min; time_t itime_max = time_max;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 2 * margin, margin, 2 * margin, height - margin);

        std::tm tm = *std::localtime(&itime_min);
        std::stringstream ss;
        ss << std::put_time(&tm, "%d-%m-%Y %T");
        stringRGBA(renderer, 3 * margin, margin, ss.str().c_str(), 255, 255, 255, 255);
        ss.str(std::string());

        tm = *std::localtime(&itime_max);
        ss << std::put_time(&tm, "%d-%m-%Y %T");
        stringRGBA(renderer, 3 * margin, height - margin, ss.str().c_str(), 255, 255, 255, 255);
        ss.str(std::string());

        for (const file& f : files) {
            tm = *std::localtime(&f.timestamp);
            ss << f.name << " " << std::put_time(&tm, "%d-%m-%Y %T");
            float y = mapfloat(f.timestamp, time_min, time_max, margin, height - margin);
            SDL_RenderDrawLine(renderer, margin, y, 3 * margin, y);
            stringRGBA(renderer, 4 * margin, y, ss.str().c_str(), 255, 255, 255, 255);
            ss.str(std::string());
        }

        SDL_RenderPresent(renderer);
        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}