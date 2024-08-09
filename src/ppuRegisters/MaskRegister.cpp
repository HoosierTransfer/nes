#include <ppuRegisters/MaskRegister.hpp>

MaskRegister::MaskRegister() : bits(0b00000000) {}

bool MaskRegister::contains(uint8_t flag) const {
    return (bits & flag) != 0;
}

bool MaskRegister::is_grayscale() const {
    return contains(GREYSCALE);
}

bool MaskRegister::leftmost_8pxl_background() const {
    return contains(LEFTMOST_8PXL_BACKGROUND);
}

bool MaskRegister::leftmost_8pxl_sprite() const {
    return contains(LEFTMOST_8PXL_SPRITE);
}

bool MaskRegister::show_background() const {
    return contains(SHOW_BACKGROUND);
}

bool MaskRegister::show_sprites() const {
    return contains(SHOW_SPRITES);
}

bool MaskRegister::emphasise_red() const {
    return contains(EMPHASISE_RED);
}

bool MaskRegister::emphasise_green() const {
    return contains(EMPHASISE_GREEN);
}

bool MaskRegister::emphasise_blue() const {
    return contains(EMPHASISE_BLUE);
}

void MaskRegister::update(uint8_t data) {
    bits = data;
}