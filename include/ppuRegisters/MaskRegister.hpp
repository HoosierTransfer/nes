#pragma once

#include <cstdint>
#include <vector>

class MaskRegister {
public:
    static constexpr uint8_t GREYSCALE               = 0b00000001;
    static constexpr uint8_t LEFTMOST_8PXL_BACKGROUND = 0b00000010;
    static constexpr uint8_t LEFTMOST_8PXL_SPRITE     = 0b00000100;
    static constexpr uint8_t SHOW_BACKGROUND         = 0b00001000;
    static constexpr uint8_t SHOW_SPRITES            = 0b00010000;
    static constexpr uint8_t EMPHASISE_RED           = 0b00100000;
    static constexpr uint8_t EMPHASISE_GREEN         = 0b01000000;
    static constexpr uint8_t EMPHASISE_BLUE          = 0b10000000;

    MaskRegister();

    bool is_grayscale() const;
    bool leftmost_8pxl_background() const;
    bool leftmost_8pxl_sprite() const;
    bool show_background() const;
    bool show_sprites() const;
    bool emphasise_red() const;
    bool emphasise_green() const;
    bool emphasise_blue() const;
    void update(uint8_t data);

private:
    uint8_t bits;
    bool contains(uint8_t flag) const;
};