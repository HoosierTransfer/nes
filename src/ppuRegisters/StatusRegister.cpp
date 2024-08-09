#include <ppuRegisters/StatusRegister.hpp>

// Constructor
StatusRegister::StatusRegister() : bits(0b00000000) {}

void StatusRegister::set(uint8_t flag, bool status) {
    if (status) {
        bits |= flag;
    } else {
        bits &= ~flag;
    }
}

void StatusRegister::remove(uint8_t flag) {
    bits &= ~flag;
}

bool StatusRegister::contains(uint8_t flag) const {
    return (bits & flag) != 0;
}

void StatusRegister::set_vblank_status(bool status) {
    set(VBLANK_STARTED, status);
}

void StatusRegister::set_sprite_zero_hit(bool status) {
    set(SPRITE_ZERO_HIT, status);
}

void StatusRegister::set_sprite_overflow(bool status) {
    set(SPRITE_OVERFLOW, status);
}

void StatusRegister::reset_vblank_status() {
    remove(VBLANK_STARTED);
}

bool StatusRegister::is_in_vblank() const {
    return contains(VBLANK_STARTED);
}

uint8_t StatusRegister::snapshot() const {
    return bits;
}