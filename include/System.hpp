#pragma once

#include <cstdint>

#include <CPU.hpp>
#include <Bus.hpp>

class System {
public:
    static System* instance;
    bool stop;

    CPU* cpu;

    Bus* bus;

    uint64_t masterCycles;
    System();

    void run();

    void step();
};
