#pragma once

#include <cstdint>

namespace interrupt {
    enum class InterruptType {
        NMI,
    };

    struct Interrupt {
        InterruptType itype;
        uint16_t vector_addr;
        uint8_t b_flag_mask;
        uint8_t cpu_cycles;

        bool operator==(const Interrupt& other) const {
            return itype == other.itype &&
                   vector_addr == other.vector_addr &&
                   b_flag_mask == other.b_flag_mask &&
                   cpu_cycles == other.cpu_cycles;
        }
    };

    const Interrupt NMI = {
        InterruptType::NMI,
        0xfffA,
        0b00100000,
        2
    };
}
