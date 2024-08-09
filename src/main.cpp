#include <iostream>
#include <chrono>

#include <PPU.hpp>
#include <System.hpp>

#include <SDL2/SDL.h>

#include <vector>

const std::vector<SDL_Color> nesPalette = {
 { 0x80, 0x80, 0x80, 0xff }, { 0x00, 0x3D, 0xA6, 0xff }, { 0x00, 0x12, 0xB0, 0xff }, { 0x44, 0x00, 0x96, 0xff },
 { 0xA1, 0x00, 0x5E, 0xff }, { 0xC7, 0x00, 0x28, 0xff }, { 0xBA, 0x06, 0x00, 0xff }, { 0x8C, 0x17, 0x00, 0xff },
 { 0x5C, 0x2F, 0x00, 0xff }, { 0x10, 0x45, 0x00, 0xff }, { 0x05, 0x4A, 0x00, 0xff }, { 0x00, 0x47, 0x2E, 0xff },
 { 0x00, 0x41, 0x66, 0xff }, { 0x00, 0x00, 0x00, 0xff }, { 0x05, 0x05, 0x05, 0xff }, { 0x05, 0x05, 0x05, 0xff },
 { 0xC7, 0xC7, 0xC7, 0xff }, { 0x00, 0x77, 0xFF, 0xff }, { 0x21, 0x55, 0xFF, 0xff }, { 0x82, 0x37, 0xFA, 0xff },
 { 0xEB, 0x2F, 0xB5, 0xff }, { 0xFF, 0x29, 0x50, 0xff }, { 0xFF, 0x22, 0x00, 0xff }, { 0xD6, 0x32, 0x00, 0xff },
 { 0xC4, 0x62, 0x00, 0xff }, { 0x35, 0x80, 0x00, 0xff }, { 0x05, 0x8F, 0x00, 0xff }, { 0x00, 0x8A, 0x55, 0xff },
 { 0x00, 0x99, 0xCC, 0xff }, { 0x21, 0x21, 0x21, 0xff }, { 0x09, 0x09, 0x09, 0xff }, { 0x09, 0x09, 0x09, 0xff },
 { 0xFF, 0xFF, 0xFF, 0xff }, { 0x0F, 0xD7, 0xFF, 0xff }, { 0x69, 0xA2, 0xFF, 0xff }, { 0xD4, 0x80, 0xFF, 0xff },
 { 0xFF, 0x45, 0xF3, 0xff }, { 0xFF, 0x61, 0x8B, 0xff }, { 0xFF, 0x88, 0x33, 0xff }, { 0xFF, 0x9C, 0x12, 0xff },
 { 0xFA, 0xBC, 0x20, 0xff }, { 0x9F, 0xE3, 0x0E, 0xff }, { 0x2B, 0xF0, 0x35, 0xff }, { 0x0C, 0xF0, 0xA4, 0xff },
 { 0x05, 0xFB, 0xFF, 0xff }, { 0x5E, 0x5E, 0x5E, 0xff }, { 0x0D, 0x0D, 0x0D, 0xff }, { 0x0D, 0x0D, 0x0D, 0xff },
 { 0xFF, 0xFF, 0xFF, 0xff }, { 0xA6, 0xFC, 0xFF, 0xff }, { 0xB3, 0xEC, 0xFF, 0xff }, { 0xDA, 0xAB, 0xEB, 0xff },
 { 0xFF, 0xA8, 0xF9, 0xff }, { 0xFF, 0xAB, 0xB3, 0xff }, { 0xFF, 0xD2, 0xB0, 0xff }, { 0xFF, 0xEF, 0xA6, 0xff },
 { 0xFF, 0xF7, 0x9C, 0xff }, { 0xD7, 0xE8, 0x95, 0xff }, { 0xA6, 0xED, 0xAF, 0xff }, { 0xA2, 0xF2, 0xDA, 0xff },
 { 0x99, 0xFF, 0xFC, 0xff }, { 0xDD, 0xDD, 0xDD, 0xff }, { 0x11, 0x11, 0x11, 0xff }, { 0x11, 0x11, 0x11, 0xff },
};

std::vector<uint8_t> bgPalette(PPU* ppu, int tileColumn, int tileRow) {
    int attributeTableIdx = tileRow / 4 * 8 + tileColumn / 4;
    uint8_t attributeByte = ppu->vram[0x3C0 + attributeTableIdx];

    uint8_t pallet_idx;
    switch ((tileColumn % 4) / 2 * 2 + (tileRow % 4) / 2) {
        case 0: pallet_idx = attributeByte & 0b11; break;
        case 1: pallet_idx = (attributeByte >> 2) & 0b11; break;
        case 2: pallet_idx = (attributeByte >> 4) & 0b11; break;
        case 3: pallet_idx = (attributeByte >> 6) & 0b11; break;
        default: throw std::runtime_error("should not happen");
    }

    return {
        ppu->palette[0],
        ppu->palette[pallet_idx * 4 + 1],
        ppu->palette[pallet_idx * 4 + 2],
        ppu->palette[pallet_idx * 4 + 3]
    };
}

std::vector<uint8_t> spritePalette(PPU* ppu, int spriteIdx) {
    uint8_t start = 0x11 + spriteIdx * 4;
    return {
        0,
        ppu->palette[start],
        ppu->palette[start + 1],
        ppu->palette[start + 2]
    };
}

uint8_t* showTile(PPU* ppu, int bank, int tileN, std::vector<uint8_t> palette) {
    uint8_t* frame = new uint8_t[8 * 8 * 4];
    // zero out the frame
    for (int i = 0; i < 8 * 8 * 4; i++) {
        frame[i] = 0;
    }
    int bankOffset = bank * 0x1000;
    int tileStart = bankOffset + tileN * 16;
    int tileEnd = tileStart + 16;
    std::vector<uint8_t> tile(ppu->chrRom.begin() + tileStart, ppu->chrRom.begin() + tileEnd);
    for (int y = 0; y < 8; y++) {
        uint8_t upper = tile[y];
        uint8_t lower = tile[y + 8];

        for (int x = 7; x >= 0; x--) {
            int value = (1 & lower) << 1 | (1 & upper);
            upper >>= 1;
            lower >>= 1;
    
            SDL_Color color;
            switch (value) {
                case 0: color = nesPalette[ppu->palette[0]]; break;
                case 1: color = nesPalette[palette[1]]; break;
                case 2: color = nesPalette[palette[2]]; break;
                case 3: color = nesPalette[palette[3]]; break;
            }

            frame[(y * 8 + x) * 4] = color.r;
            frame[(y * 8 + x) * 4 + 1] = color.g;
            frame[(y * 8 + x) * 4 + 2] = color.b;
            frame[(y * 8 + x) * 4 + 3] = color.a;
        }
    }

    return frame;
}

uint8_t* showSprite(PPU* ppu, int bank, int tileN, std::vector<uint8_t> palette, bool flipVertical, bool flipHorizontal) {
    uint8_t* frame = new uint8_t[8 * 8 * 4];
    // zero out the frame
    for (int i = 0; i < 8 * 8 * 4; i++) {
        frame[i] = 0;
    }
    int bankOffset = bank;
    int tileStart = bankOffset + tileN * 16;
    int tileEnd = tileStart + 16;
    std::vector<uint8_t> tile(ppu->chrRom.begin() + tileStart, ppu->chrRom.begin() + tileEnd);
    for (int y = 0; y < 8; y++) {
        uint8_t upper = tile[y];
        uint8_t lower = tile[y + 8];

        for (int x = 7; x >= 0; x--) {
            int value = (1 & lower) << 1 | (1 & upper);
            upper >>= 1;
            lower >>= 1;
    
            SDL_Color color;
            switch (value) {
                case 0: continue;
                case 1: color = nesPalette[palette[1]]; break;
                case 2: color = nesPalette[palette[2]]; break;
                case 3: color = nesPalette[palette[3]]; break;
            }

            if (flipHorizontal) {
                if (flipVertical) {
                    frame[((7 - y) * 8 + (7 - x)) * 4] = color.r;
                    frame[((7 - y) * 8 + (7 - x)) * 4 + 1] = color.g;
                    frame[((7 - y) * 8 + (7 - x)) * 4 + 2] = color.b;
                    frame[((7 - y) * 8 + (7 - x)) * 4 + 3] = color.a;
                } else {
                    frame[(y * 8 + (7 - x)) * 4] = color.r;
                    frame[(y * 8 + (7 - x)) * 4 + 1] = color.g;
                    frame[(y * 8 + (7 - x)) * 4 + 2] = color.b;
                    frame[(y * 8 + (7 - x)) * 4 + 3] = color.a;
                }
            } else {
                if (flipVertical) {
                    frame[((7 - y) * 8 + x) * 4] = color.r;
                    frame[((7 - y) * 8 + x) * 4 + 1] = color.g;
                    frame[((7 - y) * 8 + x) * 4 + 2] = color.b;
                    frame[((7 - y) * 8 + x) * 4 + 3] = color.a;
                } else {
                    frame[(y * 8 + x) * 4] = color.r;
                    frame[(y * 8 + x) * 4 + 1] = color.g;
                    frame[(y * 8 + x) * 4 + 2] = color.b;
                    frame[(y * 8 + x) * 4 + 3] = color.a;
                }
            }
        }
    }

    return frame;
}

uint8_t* render(PPU* ppu) {
    uint8_t* frame = new uint8_t[256 * 240 * 4];

    // zero out the frame
    for (int i = 0; i < 256 * 240 * 4; i++) {
        frame[i] = 0;
    }

    for (int i = 0; i < 0x03c0; i++) {
        uint8_t tile = ppu->vram[i];
        int tileX = i % 32;
        int tileY = i / 32;
        std::vector<uint8_t> palette = bgPalette(ppu, tileX, tileY);

        uint8_t* tileFrame = showTile(ppu, 0, tile, palette);

        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                frame[((tileY * 8 + j) * 256 + tileX * 8 + k) * 4] = tileFrame[(j * 8 + k) * 4];
                frame[((tileY * 8 + j) * 256 + tileX * 8 + k) * 4 + 1] = tileFrame[(j * 8 + k) * 4 + 1];
                frame[((tileY * 8 + j) * 256 + tileX * 8 + k) * 4 + 2] = tileFrame[(j * 8 + k) * 4 + 2];
                frame[((tileY * 8 + j) * 256 + tileX * 8 + k) * 4 + 3] = tileFrame[(j * 8 + k) * 4 + 3];
            }
        }

        delete[] tileFrame;
    }

    // draw sprites
    // draw sprites
    for (int i = 63; i >= 0; i--) {
        int spriteIdx = i * 4;
        uint16_t tileIdx = ppu->oam[spriteIdx + 1];
        int tileX = ppu->oam[spriteIdx + 3];
        int tileY = ppu->oam[spriteIdx];

        bool flipVertical = ppu->oam[spriteIdx + 2] & 0x80;
        bool flipHorizontal = ppu->oam[spriteIdx + 2] & 0x40;
        int paletteIdx = ppu->oam[spriteIdx + 2] & 0x3;
        std::vector<uint8_t> palette = spritePalette(ppu, paletteIdx);

        int bank = ppu->controlRegister->sprt_pattern_addr();
        uint8_t* tileFrame = showSprite(ppu, bank, tileIdx, palette, flipVertical, flipHorizontal);

        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                int px = tileX + k;
                int py = tileY + j;
                if (px >= 0 && px < 256 && py >= 0 && py < 240) {
                    if (tileFrame[(j * 8 + k) * 4] != 0 || tileFrame[(j * 8 + k) * 4 + 1] != 0 || tileFrame[(j * 8 + k) * 4 + 2] != 0) {
                        frame[(py * 256 + px) * 4] = tileFrame[(j * 8 + k) * 4];
                        frame[(py * 256 + px) * 4 + 1] = tileFrame[(j * 8 + k) * 4 + 1];
                        frame[(py * 256 + px) * 4 + 2] = tileFrame[(j * 8 + k) * 4 + 2];
                        frame[(py * 256 + px) * 4 + 3] = tileFrame[(j * 8 + k) * 4 + 3];
                    }
                }
            }
        }

        delete[] tileFrame;
    }
    return frame;
}

int main() {
    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256 * 3, 240 * 3, SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 240);

    if (texture == nullptr) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_RenderSetScale(renderer, 3, 3);

    System system("pacman.nes");

    bool running = true;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    uint8_t* frame = render(system.ppu);
    SDL_UpdateTexture(texture, nullptr, frame, 256 * 4);

    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        system.step();

        if (System::instance->needsDraw()) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            uint8_t* frame = render(system.ppu);
            SDL_UpdateTexture(texture, nullptr, frame, 256 * 4);

            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
