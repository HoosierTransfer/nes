#pragma once

#include <cstdint>

enum PPUMaskFlags {
    GREYSCALE = 0x01,
    SHOW_BACKGROUND_LEFT = 0x02,
    SHOW_SPRITES_LEFT = 0x04,
    SHOW_BACKGROUND = 0x08,
    SHOW_SPRITES = 0x10,
    EMPHASIZE_RED = 0x20,
    EMPHASIZE_GREEN = 0x40,
    EMPHASIZE_BLUE = 0x80
};

class MaskRegister {
public:
    MaskRegister();

    void write(uint8_t data);

    bool greyscale();

    bool showBackgroundLeft();

    bool showSpritesLeft();

    bool showBackground();

    bool showSprites();

    bool emphasizeRed();

    bool emphasizeGreen();

    bool emphasizeBlue();

private:
    uint8_t flags;
};