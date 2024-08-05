#pragma once

#include <cstdint>
#include <Bus.hpp>

class CPU {
public:
    uint16_t programCounter;
    uint8_t accumulator;
    uint8_t xIndex;
    uint8_t yIndex;
    uint8_t stackPointer;
    uint8_t flags;

    uint64_t cycles;

    Bus* memory;

    CPU(Bus* memory);

    void powerOn();
    void reset();

    uint8_t getCarry();
    void setCarry(bool value);

    uint8_t getZero();
    void setZero(bool value);

    uint8_t getInterrupt();
    void setInterrupt(bool value);

    uint8_t getDecimal();
    void setDecimal(bool value);

    uint8_t getBFlag();
    void setBFlag(bool value);

    uint8_t getIFlag();
    void setIFlag(bool value);

    uint8_t getOverflow();
    void setOverflow(bool value);

    uint8_t getNegative();
    void setNegative(bool value);

    void pushByte(uint8_t data);
    void pushWord(uint16_t data);

    uint8_t popByte();
    uint16_t popWord();

    void stepCpu(uint64_t cycle);

    void stepTo(uint64_t cycle);

    void execOnce();
private:
    uint8_t fetch();
    uint16_t fetchWord();
};