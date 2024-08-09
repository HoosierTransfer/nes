#include <iostream>
#include <chrono>

#include <CPU.hpp>
#include <Rom.hpp>
#include <Bus.hpp>
#include <PPU.hpp>
#include <System.hpp>

#include <SDL2/SDL.h>

#include <vector>

static SDL_Color make_argb(uint8_t r, uint8_t g, uint8_t b) {
    return SDL_Color{r, g, b, 255};
}

static std::vector<SDL_Color> palette = {
    make_argb(84,  84,  84),    make_argb(0,  30, 116),    make_argb(8,  16, 144),    make_argb(48,   0, 136),   make_argb(68,   0, 100),   make_argb(92,   0,  48),   make_argb(84,   4,   0),   make_argb(60,  24,   0),   make_argb(32,  42,   0),   make_argb(8,  58,   0),   make_argb(0,  64,   0),    make_argb(0,  60,   0),    make_argb(0,  50,  60),    make_argb(0,   0,   0),   make_argb(0, 0, 0), make_argb(0, 0, 0),
        make_argb(152, 150, 152),   make_argb(8,  76, 196),    make_argb(48,  50, 236),   make_argb(92,  30, 228),   make_argb(136,  20, 176),  make_argb(160,  20, 100),  make_argb(152,  34,  32),  make_argb(120,  60,   0),  make_argb(84,  90,   0),   make_argb(40, 114,   0),  make_argb(8, 124,   0),    make_argb(0, 118,  40),    make_argb(0, 102, 120),    make_argb(0,   0,   0),   make_argb(0, 0, 0), make_argb(0, 0, 0),
        make_argb(236, 238, 236),   make_argb(76, 154, 236),   make_argb(120, 124, 236),  make_argb(176,  98, 236),  make_argb(228,  84, 236),  make_argb(236,  88, 180),  make_argb(236, 106, 100),  make_argb(212, 136,  32),  make_argb(160, 170,   0),  make_argb(116, 196,   0), make_argb(76, 208,  32),   make_argb(56, 204, 108),   make_argb(56, 180, 204),   make_argb(60,  60,  60),  make_argb(0, 0, 0), make_argb(0, 0, 0),
        make_argb(236, 238, 236),   make_argb(168, 204, 236),  make_argb(188, 188, 236),  make_argb(212, 178, 236),  make_argb(236, 174, 236),  make_argb(236, 174, 212),  make_argb(236, 180, 176),  make_argb(228, 196, 144),  make_argb(204, 210, 120),  make_argb(180, 222, 120), make_argb(168, 226, 144),  make_argb(152, 226, 180),  make_argb(160, 214, 228),  make_argb(160, 162, 160), make_argb(0, 0, 0), make_argb(0, 0, 0)
};

uint8_t* showTile(PPU* ppu, int bank, int tile) {
    uint16_t addr = (bank * 0x1000) + (tile * 16);
    uint8_t* data = new uint8_t[8 * 8 * 4];

    for (int i = 0; i < 8; i++) {
        uint8_t low = ppu->chrRom[addr + i];
        uint8_t high = ppu->chrRom[addr + i + 8];

        for (int j = 0; j < 8; j++) {
            uint8_t bit = 7 - j;
            uint8_t color = ((low >> bit) & 1) | (((high >> bit) & 1) << 1);

            SDL_Color rgb;
            switch (color) {
                case 0: rgb = palette[0x01]; break;
                case 1: rgb = palette[0x23]; break;
                case 2: rgb = palette[0x27]; break;
                case 3: rgb = palette[0x30]; break;
                default: throw std::runtime_error("Unexpected value");
            }

            int pixelIndex = (i * 8 + j) * 4;

            data[pixelIndex] = rgb.r;
            data[pixelIndex + 1] = rgb.g;
            data[pixelIndex + 2] = rgb.b;
            data[pixelIndex + 3] = 255;
        }
    }

    return data;
}


uint8_t* showTileBank(PPU* ppu, int bank) {
    uint8_t* data = new uint8_t[256 * 240 * 4];
    int tileX = 0;
    int tileY = 0;
    size_t bank_offset = bank * 0x1000;

    // zero the data
    for (int i = 0; i < 256 * 240 * 4; i++) {
        data[i] = 0;
    }

    for (int tile = 0; tile < 255; tile++) {
        if (tile != 0 && tile % 20 == 0) {
            tileX = 0;
            tileY += 1;
        }

        uint8_t* tileData = showTile(ppu, bank, tile);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int pixelIndex = ((tileY * 10 + i) * 256 + tileX * 10 + j) * 4;
                int tileIndex = (i * 8 + j) * 4;

                data[pixelIndex] = tileData[tileIndex];
                data[pixelIndex + 1] = tileData[tileIndex + 1];
                data[pixelIndex + 2] = tileData[tileIndex + 2];
                data[pixelIndex + 3] = tileData[tileIndex + 3];
            }
        }

        tileX++;

    }

    return data;
}

uint8_t* render(PPU* ppu) {
    int bank = ppu->controlRegister->backgroundPatternTableAddress();
    uint8_t* data = new uint8_t[256 * 240 * 4];
    for (int i = 0; i < 0x03c0; i++) {
        uint16_t tile = ppu->vram[i];
        int tileX = i % 32;
        int tileY = i / 32;
        uint8_t* tileData = showTile(ppu, bank, tile);

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int pixelIndex = ((tileY * 8 + i) * 256 + tileX * 8 + j) * 4;
                int tileIndex = (i * 8 + j) * 4;

                data[pixelIndex] = tileData[tileIndex];
                data[pixelIndex + 1] = tileData[tileIndex + 1];
                data[pixelIndex + 2] = tileData[tileIndex + 2];
                data[pixelIndex + 3] = tileData[tileIndex + 3];
            }
        }
    }

    return data;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256 * 3, 240 * 3, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Unable to create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Unable to create renderer: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_RenderSetScale(renderer, 3, 3);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

    System* system = new System("Alter_Ego.nes");

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        for (int i = 0; i < 10; i++) {
            system->step();
        }
        uint8_t* data = render(system->ppu);
        SDL_UpdateTexture(texture, nullptr, data, 256 * 4);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
