#include <ppu/StatusRegister.hpp>

StatusRegister::StatusRegister() {
    flags = 0;
}

void StatusRegister::setVBlank(bool vblank) {
    if (vblank) {
        flags |= VBLANK;
    } else {
        flags &= ~VBLANK;
    }
}

void StatusRegister::setSpriteZeroHit(bool hit) {
    if (hit) {
        flags |= SPRITE_ZERO_HIT;
    } else {
        flags &= ~SPRITE_ZERO_HIT;
    }
}

void StatusRegister::setSpriteOverflow(bool overflow) {
    if (overflow) {
        flags |= SPRITE_OVERFLOW;
    } else {
        flags &= ~SPRITE_OVERFLOW;
    }
}

void StatusRegister::resetVBlank() {
    flags &= ~VBLANK;
}

bool StatusRegister::isVBlank() {
    return flags & VBLANK;
}

uint8_t StatusRegister::read() {
    return flags;
}
