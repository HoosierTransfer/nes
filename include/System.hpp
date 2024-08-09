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
    System(std::string romPath);

    void run();

    void step();

    void stepThisAndPPU(uint8_t cycles);

    bool needsDraw();

private:
    bool draw;
};
