#include <ppu/MaskRegister.hpp>

MaskRegister::MaskRegister() {
    flags = 0;
}

void MaskRegister::write(uint8_t data) {
    flags = data;
}

bool MaskRegister::greyscale() {
    return (flags & GREYSCALE) ? true : false;
}

bool MaskRegister::showBackgroundLeft() {
    return (flags & SHOW_BACKGROUND_LEFT) ? true : false;
}

bool MaskRegister::showSpritesLeft() {
    return (flags & SHOW_SPRITES_LEFT) ? true : false;
}

bool MaskRegister::showBackground() {
    return (flags & SHOW_BACKGROUND) ? true : false;
}

bool MaskRegister::showSprites() {
    return (flags & SHOW_SPRITES) ? true : false;
}

bool MaskRegister::emphasizeRed() {
    return (flags & EMPHASIZE_RED) ? true : false;
}

bool MaskRegister::emphasizeGreen() {
    return (flags & EMPHASIZE_GREEN) ? true : false;
}

bool MaskRegister::emphasizeBlue() {
    return (flags & EMPHASIZE_BLUE) ? true : false;
}
