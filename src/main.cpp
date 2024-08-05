#include <iostream>
#include <chrono>

#include <CPU.hpp>
#include <Rom.hpp>
#include <System.hpp>

#include <SDL2/SDL.h>

SDL_Color getColor(uint8_t byte) {
    switch (byte) {
        case 0:
            return SDL_Color{0, 0, 0, 255}; // BLACK
        case 1:
            return SDL_Color{255, 255, 255, 255}; // WHITE
        case 2:
        case 9:
            return SDL_Color{128, 128, 128, 255}; // GREY
        case 3:
        case 10:
            return SDL_Color{255, 0, 0, 255}; // RED
        case 4:
        case 11:
            return SDL_Color{0, 255, 0, 255}; // GREEN
        case 5:
        case 12:
            return SDL_Color{0, 0, 255, 255}; // BLUE
        case 6:
        case 13:
            return SDL_Color{255, 0, 255, 255}; // MAGENTA
        case 7:
        case 14:
            return SDL_Color{255, 255, 0, 255}; // YELLOW
        default:
            return SDL_Color{0, 255, 255, 255}; // CYAN
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 320, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Unable to create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Unable to create renderer: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 32, 32);

    System* system = new System();

    SDL_Event e;
    bool quit = false;

    uint8_t frame[0x600 * 3];

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_r:
                        for (int i = 0; i < 0x100; i++) {
                            system->bus->write(rand() % 0xFFFF, rand() % 0xFF);
                        }
                        break;
                }
            }
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // if (update) {
        //     SDL_UpdateTexture(texture, nullptr, frame, 32 * 3);
        //     SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        //     SDL_RenderPresent(renderer);
        // }

        // use deltatime to control the speed of the emulator. we want the clock to run at 1.79 MHz
    
        for (int i = 0; i < 5; i++) {
            system->step();
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
