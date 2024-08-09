#pragma once

#include <cstdint>
#include <Bus.hpp>
#include <vector>
#include <string>

#include <fstream>

#include <Interrupt.hpp>

enum class AddressingMode {
    IMP, // Implied
    ACC, // Accumulator
    IMM, // Immediate
    ZP0, // Zero Page
    ZPX, // Zero Page X
    ZPY, // Zero Page Y
    REL, // Relative
    ABS, // Absolute
    ABX, // Absolute X
    ABY, // Absolute Y
    IND, // Indirect
    IZX, // Indexed Indirect X
    IZY, // Indirect Indexed Y
    NOP  // No Operation
};

class CPU {
public:
    uint16_t programCounter;
    uint8_t accumulator;
    uint8_t xIndex;
    uint8_t yIndex;
    uint8_t stackPointer;
    uint8_t flags;

    Bus* memory;

    CPU(Bus* memory);

    void powerOn();
    void reset();

    uint8_t getCarry();
    void setCarry(bool value);

    uint8_t getZero();
    void setZero(bool value);

    uint8_t getInterruptDisable();
    void setInterruptDisable(bool value);

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

    std::string log();

    std::string getInstruction();

    std::string getAddressWithMode(AddressingMode mode);

    uint16_t getAddressWithModeValue(AddressingMode mode);

    uint16_t getAddress(AddressingMode mode);

    size_t getCycles();

    void interrupt(const interrupt::Interrupt& interrupt);

private:
    size_t cycles;
    uint8_t fetch();
    uint16_t fetchWord();
    uint8_t fetchNoAdvance();
    uint16_t fetchWordNoAdvance();
    std::vector<uint8_t> fetchLogs;

    uint16_t oldPC;
    uint16_t branchLocation;

    std::ifstream knownGoodLog;

    uint16_t oldAccumulator;
    uint16_t oldXIndex;
    uint16_t oldYIndex;
    uint16_t oldStackPointer;
    uint16_t oldFlags;

    uint16_t oldOldAccumulator;
    uint16_t oldOldXIndex;
    uint16_t oldOldYIndex;
    uint16_t oldOldStackPointer;
    uint16_t oldOldFlags;

    uint8_t miscValue;

    int stepCountAfterFatalError;

    uint8_t indirectAddr1;
    uint16_t indirectAddr2;

    bool pageCrossed;
};
