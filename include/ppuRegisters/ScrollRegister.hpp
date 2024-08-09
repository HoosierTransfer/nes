#pragma once

#include <cstdint>

class ScrollRegister {
public:
    ScrollRegister();

    void write(uint8_t data);
    void reset_latch();

    uint8_t get_scroll_x() const { return scroll_x; }
    uint8_t get_scroll_y() const { return scroll_y; }

private:
    uint8_t scroll_x;
    uint8_t scroll_y;
    bool latch;
};