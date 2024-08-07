#pragma once

#include <cstdint>

#include <CPU.hpp>
#include <Bus.hpp>
#include <PPU.hpp>

class System {
public:
    static System* instance;
    bool stop;

    CPU* cpu;
    Bus* bus;
    PPU* ppu;

    uint64_t masterCycles;
    System();

    void run();

    void step();
};
