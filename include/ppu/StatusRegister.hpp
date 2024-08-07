#pragma once

#include <cstdint>

enum PPUStatusFlags {
    SPRITE_OVERFLOW = 0x20,
    SPRITE_ZERO_HIT = 0x40,
    VBLANK = 0x80
};

class StatusRegister {
public:
    StatusRegister();

    void setVBlank(bool vblank);

    void setSpriteZeroHit(bool hit);

    void setSpriteOverflow(bool overflow);

    void resetVBlank();

    bool isVBlank();

    uint8_t read();

private:
    uint8_t flags;
};