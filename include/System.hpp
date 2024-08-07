#pragma once

#include <cstdint>

#include <CPU.hpp>
#include <Memory.hpp>

class System {
public:
    static System* instance;
    bool stop;

    CPU* cpu;

    Memory* bus;

    uint64_t masterCycles;
    System();

    void run();

    void step();
};
