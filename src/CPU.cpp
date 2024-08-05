#include <CPU.hpp>

#include <System.hpp>

#include <iostream>

CPU::CPU(Bus* memory) {
    this->memory = memory;
}

void CPU::powerOn() {
    programCounter = memory->readWord(0xFFFC);
    accumulator = 0;
    xIndex = 0;
    yIndex = 0;
    stackPointer = 0xFD;
    flags = 0x34;
    cycles = 0;
}

void CPU::reset() {
    programCounter = memory->readWord(0xFFFC);
    stackPointer = stackPointer - 3;
    flags = 0x34;
    cycles = 0;
}

uint8_t CPU::getCarry() {
    return (flags & 0x01) >> 0;
}

void CPU::setCarry(bool value) {
    flags = (flags & 0xFE) | (value << 0);
}

uint8_t CPU::getZero() {
    return (flags & 0x02) >> 1;
}

void CPU::setZero(bool value) {
    flags = (flags & 0xFD) | (value << 1);
}

uint8_t CPU::getInterrupt() {
    return (flags & 0x04) >> 2;
}

void CPU::setInterrupt(bool value) {
    flags = (flags & 0xFB) | (value << 2);
}

uint8_t CPU::getDecimal() {
    return (flags & 0x08) >> 3;
}   

void CPU::setDecimal(bool value) {
    flags = (flags & 0xF7) | (value << 3);
}

uint8_t CPU::getBFlag() {
    return (flags & 0x10) >> 4;
}

void CPU::setBFlag(bool value) {
    flags = (flags & 0xEF) | (value << 4);
}

uint8_t CPU::getIFlag() {
    return (flags & 0x20) >> 5;
}

void CPU::setIFlag(bool value) {
    flags = (flags & 0xDF) | (value << 5);
}

uint8_t CPU::getOverflow() {
    return (flags & 0x40) >> 6;
}

void CPU::setOverflow(bool value) {
    flags = (flags & 0xBF) | (value << 6);
}

uint8_t CPU::getNegative() {
    return (flags & 0x80) >> 7;
}

void CPU::setNegative(bool value) {
    flags = (flags & 0x7F) | (value << 7);
}

void CPU::pushByte(uint8_t data) {
    memory->write(0x100 | stackPointer, data);
    stackPointer--;
}

void CPU::pushWord(uint16_t data) {
    pushByte((data >> 8) & 0xFF);
    pushByte(data & 0xFF);
}

uint8_t CPU::popByte() {
    stackPointer++;
    return memory->read(0x100 | stackPointer);
}

uint16_t CPU::popWord() {
    uint16_t data = popByte();
    data |= popByte() << 8;
    return data;
}

uint8_t CPU::fetch() {
    uint8_t data = memory->read(programCounter++);
    return data;
}

uint16_t CPU::fetchWord() {
    uint16_t data = memory->readWord(programCounter);
    programCounter += 2;
    return data;
}

void CPU::stepTo(uint64_t cycle) {
    while (cycles < cycle && !System::instance->stop) {
        execOnce();
    }
}

void CPU::stepCpu(uint64_t cycle) {
    cycles += cycle;
}

void CPU::execOnce() {
    if (System::instance->stop) {
        return;
    }
    uint8_t opcode = fetch();
    // print registers
    // std::cout << "PC: " << std::hex << (int)programCounter;
    // std::cout << " A: " << std::hex << (int)accumulator;
    // std::cout << " X: " << std::hex << (int)xIndex;
    // std::cout << " Y: " << std::hex << (int)yIndex;
    // std::cout << " SP: " << std::hex << (int)stackPointer;
    // std::cout << " Flags: " << std::hex << (int)flags << std::endl;
    // std::cout << "Opcode: " << std::hex << (int)opcode << std::dec << std::endl;
    switch (opcode) {
        case 0x00: // BRK
            break;
        case 0x01:
            accumulator |= memory->read(memory->readWord(fetch() + xIndex));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x02: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x03: // Illegal SLO Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator |= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x04: // Illegal DOP Zero Page
            fetch();
            break;

        case 0x05: // ORA Zero Page
            accumulator |= memory->read(fetch());
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;
        
        case 0x06: // ASL Zero Page
            {
                uint8_t data = memory->read(fetch());
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(fetch(), data);
            }
            break;

        case 0x07: // Illegal SLO Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator |= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;  

        case 0x08: // PHP
            pushByte(flags);
            break;

        case 0x09: // ORA Immediate
            accumulator |= fetch();
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x0A: // ASL Accumulator
            setCarry(accumulator & 0x80);
            accumulator <<= 1;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x0B: // Illegal ANC Immediate
        case 0x2B: // Illegal ANC Immediate aagain
            accumulator &= fetch();
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            setCarry(accumulator & 0x80);
            break;

        case 0x0C: // Illegal TOP Absolute
            fetchWord();
            break;

        case 0x0D: // ORA Absolute
            accumulator |= memory->read(fetchWord());
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x0E: // ASL Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x0F: // Illegal SLO Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator |= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x10: // BPL
            if (!getNegative()) {
                int8_t offset = (int8_t)fetch();
                programCounter += offset;
                stepCpu(1);
            }
            break;

        case 0x11: // ORA Indirect, Indexed
            accumulator |= memory->read(memory->readWord(fetch()) + yIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x12: // Illegal KIL
            System::instance->stop = true;
            break;
            
        case 0x13: // Illegal SLO Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator |= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }

        case 0x14: // Illegal DOP Zero Page, X
            fetch();
            break;

        case 0x15: // ORA Zero Page, X
            accumulator |= memory->read(fetch() + xIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x16: // ASL Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x17: // Illegal SLO Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator |= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x18: // CLC
            setCarry(false);
            break;

        case 0x19: // ORA Absolute, Y
            accumulator |= memory->read(fetchWord() + yIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x1A: // Illegal NOP
            break;

        case 0x1B: // Illegal SLO Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator |= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x1C: // Illegal TOP Absolute, X
            fetchWord();
            break;

        case 0x1D: // ORA Absolute, X
            accumulator |= memory->read(fetchWord() + xIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x1E: // ASL Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x1F: // Illegal SLO Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator |= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x20: // JSR
            pushWord(programCounter + 1);
            programCounter = fetchWord();
            break;

        case 0x22: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x23: // Illegal RLA Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch()) + yIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x24: // BIT Zero Page
            {
                uint8_t data = memory->read(fetch());
                setZero((accumulator & data) == 0);
                setOverflow(data & 0x40);
                setNegative(data & 0x80);
            }
            break;

        case 0x25: // AND Zero Page
            accumulator &= memory->read(fetch());
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x26: // ROL Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x27: // Illegal RLA Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x29: // AND Immediate
            accumulator &= fetch();
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x2A: // ROL Accumulator
            {
                uint8_t carry = getCarry();
                setCarry(accumulator & 0x80);
                accumulator <<= 1;
                accumulator |= carry;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x2C: // BIT Absolute
            {
                uint8_t data = memory->read(fetchWord());
                setZero((accumulator & data) == 0);
                setOverflow(data & 0x40);
                setNegative(data & 0x80);
            }
            break;

        case 0x2D: // AND Absolute
            accumulator &= memory->read(fetchWord());
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x2E: // ROL Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x2F: // Illegal RLA Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x30: // BMI
            if (getNegative()) {
                int8_t offset = (int8_t)fetch();
                programCounter += offset;
                stepCpu(1);
            } else {
                fetch();
            }
            break;

        case 0x31: // AND Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch());
                uint8_t data = memory->read(address) + yIndex;
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x32: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x33: // Illegal RLA Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x34: // Illegal DOP Zero Page, X
            fetch();
            break;

        case 0x35: // AND Zero Page, X
            accumulator &= memory->read(fetch() + xIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x36: // ROL Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x37: // Illegal RLA Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;  

        case 0x38: // SEC
            setCarry(true);
            break;

        case 0x39: // AND Absolute, Y
            accumulator &= memory->read(fetchWord() + yIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x3A: // Illegal NOP
            break;

        case 0x3B: // Illegal RLA Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x3C: // Illegal TOP Absolute, X
            fetchWord();
            break;

        case 0x3D: // AND Absolute, X
            accumulator &= memory->read(fetchWord() + xIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x3E: // ROL Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x3F: // Illegal RLA Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x40: // RTI
            flags = popByte();
            programCounter = popWord();
            break;

        case 0x41: // EOR Indexed, Indirect
            accumulator ^= memory->read(memory->readWord(fetch() + xIndex));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x42: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x43: // Illegal SRE Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x44: // Illegal DOP Zero Page
            fetch();
            break;

        case 0x45: // EOR Zero Page
            accumulator ^= memory->read(fetch());
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x46: // LSR Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x47: // Illegal SRE Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x48: // PHA
            pushByte(accumulator);
            break;

        case 0x49: // EOR Immediate
            accumulator ^= fetch();
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x4A: // LSR Accumulator
            setCarry(accumulator & 0x01);
            accumulator >>= 1;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x4B: // Illegal ASR Immediate
            {
                uint8_t data = fetch();
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
                setCarry(accumulator & 0x01);
                accumulator >>= 1;
            }
            break;

        case 0x4C: // JMP Absolute
            programCounter = fetchWord();
            break;

        case 0x4D: // EOR Absolute
            accumulator ^= memory->read(fetchWord());
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x4E: // LSR Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x4F: // Illegal SRE Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x50:
            if (!getOverflow()) {
                int8_t offset = (int8_t)fetch();
                programCounter += offset;
                stepCpu(1);
            } else {
                fetch();
            }
            break;

        case 0x51: // EOR Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch());
                accumulator ^= memory->read(address) + yIndex;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x52: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x53: // Illegal SRE Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch()) + yIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x54: // Illegal DOP Zero Page, X
            fetch();
            break;

        case 0x55: // EOR Zero Page, X
            accumulator ^= memory->read(fetch() + xIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x56: // LSR Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x57: // Illegal SRE Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x58: // CLI
            setIFlag(false);
            break;

        case 0x59: // EOR Absolute, Y
            accumulator ^= memory->read(fetchWord() + yIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x5A: // Illegal NOP
            break;

        case 0x5B: // Illegal SRE Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x5C: // Illegal TOP Absolute, X
            fetchWord();
            break;

        case 0x5D: // EOR Absolute, X
            accumulator ^= memory->read(fetchWord() + xIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x5E: // LSR Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x5F: // Illegal SRE Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x60: // RTS
            programCounter = popWord() + 1;
            break;

        case 0x61: // ADC Indexed Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x62: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x63: // Illegal RRA Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x64: // Illegal DOP Zero Page
            fetch();
            break;

        case 0x65: // ADC Zero Page
            {
                uint8_t data = memory->read(fetch());
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x66: // ROR Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x67: // Illegal RRA Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x68: // PLA
            accumulator = popByte();
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x69: // ADC Immediate
            {
                uint8_t data = fetch();
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x6A: // ROR Accumulator
            {
                uint8_t carry = getCarry();
                setCarry(accumulator & 0x01);
                accumulator >>= 1;
                accumulator |= carry << 7;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x6B: // Illegal ARR Immediate
            {
                uint8_t data = fetch();
                accumulator &= data;
                setCarry(accumulator & 0x80);
                accumulator >>= 1;
                accumulator |= getCarry() << 7;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
                setOverflow((accumulator & 0x40) != 0);
            }
            break;

        case 0x6C: // JMP Indirect
            {
                uint16_t address = memory->readWord(fetchWord());
                programCounter = address;
            }
            break;

        case 0x6D: // ADC Absolute
            {
                uint8_t data = memory->read(fetchWord());
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x6E: // ROR Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x6F: // Illegal RRA Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x70: // BVS
            if (getOverflow()) {
                int8_t offset = (int8_t)fetch();
                programCounter += offset;
                stepCpu(1);
            } else {
                fetch();
            }
            break;

        case 0x71: // ADC Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch());
                uint8_t data = memory->read(address) + yIndex;
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x72: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x73: // Illegal RRA Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch()) + yIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x74: // Illegal DOP Zero Page, X
            fetch();
            break;

        case 0x76: // ROR Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x77: // Illegal RRA Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x78: // SEI
            setIFlag(true);
            break;

        case 0x79: // ADC Absolute, Y
            {
                uint8_t data = memory->read(fetchWord() + yIndex);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x7A: // Illegal NOP
            break;

        case 0x7B: // Illegal RRA Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x7C: // Illegal TOP Absolute, X
            fetchWord();
            break;

        case 0x7D: // ADC Absolute, X
            {
                uint8_t data = memory->read(fetchWord() + xIndex);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x7E: // ROR Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0x7F: // Illegal RRA Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x80: // Illegal DOP Immediate
            fetch();
            break;

        case 0x81: // STA Indexed, Indirect
            {
                uint16_t addr = memory->readWord(fetch() + xIndex);
                memory->write(addr, accumulator);
            }
            break;

        case 0x82: // Illegal DOP Immediate
            fetch();
            break;

        case 0x83: // Illegal SAX Zero Page, Y
            {
                uint8_t data = accumulator & xIndex;
                memory->write(fetch() + yIndex, data);
            }
            break;

        case 0x84: // STY Zero Page
            memory->write(fetch(), yIndex);
            break;

        case 0x85: // STA Zero Page
            memory->write(fetch(), accumulator);
            break;

        case 0x86: // STX Zero Page
            memory->write(fetch(), xIndex);
            break;

        case 0x87: // Illegal SAX Zero Page.
            {
                uint8_t data = accumulator & xIndex;
                memory->write(fetch(), data);
            }
            break;

        case 0x88: // DEY
            yIndex--;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0x89: // Illegal DOP Immediate
            fetch();
            break;

        case 0x8A: // TXA
            accumulator = xIndex;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x8B: // Illegal XAA Immediate. no idea if this is right. (it isnt this opperation isnt fully understood)
            {
                uint8_t data = fetch();
                accumulator &= xIndex & data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x8C: // STY Absolute
            memory->write(fetchWord(), yIndex);
            break;

        case 0x8D: // STA Absolute
            memory->write(fetchWord(), accumulator);
            break;

        case 0x8E: // STX Absolute
            memory->write(fetchWord(), xIndex);
            break;

        case 0x8F: // Illegal SAX Absolute
            {
                uint8_t data = accumulator & xIndex;
                memory->write(fetchWord(), data);
            }
            break;

        case 0x90: // BCC
            if (!getCarry()) {
                int8_t offset = (int8_t)fetch();
                programCounter += offset;
                stepCpu(1);
            } else {
                fetch();
            }
            break;

        case 0x91: // STA Indirect, Indexed}
            {
                uint8_t base = fetch();
                uint16_t addr = memory->readWord(base) + yIndex;
                memory->write(addr, accumulator);
            }
            break;

        case 0x92: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x93: // Illegal SHA Indirect, Indexed
            {
                uint8_t base = fetch();
                uint16_t addr = memory->readWord(base) + yIndex;
                uint8_t data = accumulator & xIndex & 0x7;
                memory->write(addr, data);
            }
            break;

        case 0x94: // STY Zero Page, X
            memory->write(fetch() + xIndex, yIndex);
            break;

        case 0x95: // STA Zero Page, X
            memory->write(fetch() + xIndex, accumulator);
            break;

        case 0x96: // STX Zero Page, Y
            memory->write(fetch() + yIndex, xIndex);
            break;

        case 0x97: // Illegal Indirect, Indexed
            {
                uint8_t base = fetch();
                uint16_t addr = memory->readWord(base + xIndex); // yeah i dont even know :sob:
                uint8_t data = accumulator & xIndex;
                memory->write(addr, data);
            }
            break;

        case 0x98: // TYA
            accumulator = yIndex;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x99: // STA Absolute, Y
            memory->write(fetchWord() + yIndex, accumulator);
            break;

        case 0x9A: // TXS
            stackPointer = xIndex;
            break;

        case 0x9C: // Illegal SHY Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = xIndex & accumulator;
                stackPointer = data;
                data &= (address >> 8) + 1;
                memory->write(address, data);
            }
            break;

        case 0x9B: // Illegal SHS Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = accumulator & stackPointer;
                memory->write(address, data);
                stackPointer = accumulator;
            }
            break;

        case 0x9D: // STA Absolute, X
            memory->write(fetchWord() + xIndex, accumulator);
            break;

        case 0x9E: // Illegal SHX Absolute, Y
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = xIndex & memory->read(address + 1);
                memory->write(address, data);
            }
            break;

        case 0x9F: // Illegal SHA Absolute, Y. freaky operation :skull:
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = accumulator & xIndex & 0x7;
                memory->write(address, data);
            }
            break;

        case 0xA0: // LDY Immediate
            yIndex = fetch();
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xA1: // LDA Indexed, Indirect
            accumulator = memory->read(memory->readWord(fetch() + xIndex));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xA2: // LDX Immediate
            xIndex = fetch();
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xA3: // Illegal LAX Indexed, Indirect
            {
                uint8_t data = memory->read(memory->readWord(fetch() + xIndex));
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }

        case 0xA4: // LDY Zero Page
            yIndex = memory->read(fetch());
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xA5: // LDA Zero Page
            accumulator = memory->read(fetch());
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xA6: // LDX Zero Page
            xIndex = memory->read(fetch());
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xA7: // Illegal LAX Zero Page
            {
                uint8_t data = memory->read(fetch());
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xA8: // TAY
            yIndex = accumulator;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xA9: // LDA Immediate
            accumulator = fetch();
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xAA: // TAX
            xIndex = accumulator;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xAB: // Illegal LXA Immediate
            {
                uint8_t data = fetch();
                accumulator &= data;
                xIndex = accumulator;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0xAC: // LDY Absolute
            yIndex = memory->read(fetchWord());
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;
        
        case 0xAD: // LDA Absolute
            accumulator = memory->read(fetchWord());
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xAE: // LDX Absolute
            xIndex = memory->read(fetchWord());
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xAF: // Illegal LAX Absolute
            {
                uint8_t data = memory->read(fetchWord());
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xB0: // BCS
            if (getCarry()) {
                int8_t offset = (int8_t)fetch();
                programCounter += offset;
                stepCpu(1);
            } else {
                fetch();
            }
            break;

        case 0xB1: // LDA Indirect, Indexed
            accumulator = memory->read(memory->readWord(fetch()) + yIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xB2: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0xB3: // Illegal LAX Indirect, Indexed
            {
                uint8_t data = memory->read(memory->readWord(fetch()) + yIndex);
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xB4: // LDY Zero Page, X
            yIndex = memory->read(fetch() + xIndex);
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xB5: // LDA Zero Page, X
            accumulator = memory->read(fetch() + xIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xB7: // Illegal LAX Zero Page, Y
            {
                uint8_t data = memory->read(fetch() + yIndex);
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xB6: // LDX Zero Page, Y
            xIndex = memory->read(fetch() + yIndex);
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xB8: // CLV
            setOverflow(false);
            break;

        case 0xB9: // LDA Absolute, Y
            accumulator = memory->read(fetchWord() + yIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xBA: // TSX
            xIndex = stackPointer;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xBB: // Illegal LAS Absolute
            {
                uint8_t data = memory->read(fetchWord() + yIndex);
                accumulator &= data;
                xIndex = accumulator;
                stackPointer = accumulator;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0xBC: // LDY Absolute, X
            yIndex = memory->read(fetchWord() + xIndex);
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xBD: // LDA Absolute, X
            accumulator = memory->read(fetchWord() + xIndex);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xBE: // LDX Absolute, Y
            xIndex = memory->read(fetchWord() + yIndex);
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xBF: // Illegal LAX Absolute, Y
            {
                uint8_t data = memory->read(fetchWord() + yIndex);
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xC0: // CPY Immediate
            {
                uint8_t data = fetch();
                uint16_t result = yIndex - data;
                setCarry(yIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xC1: // CMP Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xC2: // Illegal DOP Immediate
            fetch();
            break;

        case 0xC3: // Illegal DCP Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                data--;
                setCarry(accumulator >= data);
            }
            break;

        case 0xC4: // CPY Zero Page
            {
                uint8_t data = memory->read(fetch());
                uint16_t result = yIndex - data;
                setCarry(yIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xC5: // CMP Zero Page
            {
                uint8_t data = memory->read(fetch());
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xC6: // DEC Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                data--;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0xC7: // Illegal DCP Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                data--;
                setCarry(accumulator >= data);
            }
            break;

        case 0xC8: // INY
            yIndex++;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xC9: // CMP Immediate
            {
                uint8_t data = fetch();
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xCA: // DEX
            xIndex--;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xCB: // Illegal SBX Immediate
            {
                uint8_t data = fetch();
                xIndex &= accumulator;
                xIndex -= data;
                setZero(xIndex == 0);
                setNegative(xIndex & 0x80);
                setCarry(xIndex >= 0);
            }
            break;
        
        case 0xCC: // CPY Absolute
            {
                uint8_t data = memory->read(fetchWord());
                uint16_t result = yIndex - data;
                setCarry(yIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xCD: // CMP Absolute
            {
                uint8_t data = memory->read(fetchWord());
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xCE: // DEC Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                data--;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0xCF: // Illegal DCP Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                data--;
                setCarry(accumulator >= data);
            }
            break;

        case 0xD0: // BNE
            if (!getZero()) {
                int8_t offset = (int8_t)fetch();
                programCounter += offset;
                stepCpu(1);
            } else {
                fetch();
            }
            break;

        case 0xD1: // CMP Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch());
                uint8_t data = memory->read(address) + yIndex;
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xD2: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0xD3: // Illegal DCP Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch());
                uint8_t data = memory->read(address) + yIndex;
                data--;
                setCarry(accumulator >= data);
            }
            break;

        case 0xD4: // Illegal DOP Zero Page, X
            fetch();
            break;

        case 0xD5: // CMP Zero Page, X
            {
                uint8_t data = memory->read(fetch() + xIndex);
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xD6: // DEC Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                data--;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0xD7: // Illegal DCP Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                data--;
                setCarry(accumulator >= data);
            }
            break;

        case 0xD8: // CLD
            setDecimal(false);
            break;

        case 0xD9: // CMP Absolute, Y
            {
                uint8_t data = memory->read(fetchWord() + yIndex);
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xDA: // Illegal NOP
            break;

        case 0xDB: // Illegal DCP Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = memory->read(address);
                data--;
                setCarry(accumulator >= data);
            }
            break;

        case 0xDC: // Illegal TOP Absolute, X
            fetchWord();
            break;

        case 0xDD: // CMP Absolute, X
            {
                uint8_t data = memory->read(fetchWord() + xIndex);
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xDE: // DEC Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                data--;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0xE0: // CPX Immediate
            {
                uint8_t data = fetch();
                uint16_t result = xIndex - data;
                setCarry(xIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xE1: // SBC Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xE2: // Illegal DOP Immediate
            fetch();
            break;

        case 0xE3: // Illegal ISB Indexed, Indirect
            {
                uint16_t address = memory->readWord(fetch() + xIndex);
                uint8_t data = memory->read(address);
                data++;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
                memory->write(address, data);
            }
            break;

        case 0xE4: // CPX Zero Page
            {
                uint8_t data = memory->read(fetch());
                uint16_t result = xIndex - data;
                setCarry(xIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xE5: // SBC Zero Page
            {
                uint8_t data = memory->read(fetch());
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xE6: // INC Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                data++;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0xE7: // Illegal ISB Zero Page
            {
                uint16_t address = fetch();
                uint8_t data = memory->read(address);
                data++;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
                memory->write(address, data);
            }
            break;

        case 0xE8: // INX
            xIndex++;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xE9: // SBC Immediate
            {
                uint8_t data = fetch();
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xEA: // NOP
            break;

        case 0xEB: // Illegal SBC Immediate
            {
                uint8_t data = fetch();
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xEC: // CPX Absolute
            {
                uint8_t data = memory->read(fetchWord());
                uint16_t result = xIndex - data;
                setCarry(xIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xED: // SBC Absolute
            {
                uint8_t data = memory->read(fetchWord());
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xEE: // INC Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                data++;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0xEF: // Illegal ISB Absolute
            {
                uint16_t address = fetchWord();
                uint8_t data = memory->read(address);
                data++;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
                memory->write(address, data);
            }
            break;

        case 0xF0: // BEQ
            if (getZero()) {
                int8_t offset = (int8_t)fetch();
                programCounter += offset;
                stepCpu(1);
            } else {
                fetch();
            }
            break;

        case 0xF1: // SBC Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch());
                uint8_t data = memory->read(address) + yIndex;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xF2: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0xF3: // Illegal ISB Indirect, Indexed
            {
                uint16_t address = memory->readWord(fetch());
                uint8_t data = memory->read(address) + yIndex;
                data++;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
                memory->write(address, data);
            }
            break;

        case 0xF4: // Illegal DOP Zero Page, X
            fetch();
            break;

        case 0xF5: // SBC Zero Page, X
            {
                uint8_t data = memory->read(fetch() + xIndex);
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xF6: // INC Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                data++;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0xF7: // Illegal ISB Zero Page, X
            {
                uint16_t address = fetch() + xIndex;
                uint8_t data = memory->read(address);
                data++;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
                memory->write(address, data);
            }
            break;

        case 0xF8: // SED
            setDecimal(true);
            break;

        case 0xF9: // SBC Absolute, Y
            {
                uint8_t data = memory->read(fetchWord() + yIndex);
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xFA: // Illegal NOP
            break;

        case 0xFB: // Illegal ISB Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = memory->read(address);
                data++;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
                memory->write(address, data);
            }
            break;

        case 0xFC: // Illegal TOP Absolute, X
            fetchWord();
            break;
            
        case 0xFD: // SBC Absolute, X
            {
                uint8_t data = memory->read(fetchWord() + xIndex);
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xFE: // INC Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                data++;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            break;

        case 0xFF: // Illegal ISB Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                data++;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
                memory->write(address, data);
            }
            break;
        
        default:
            std::cout << "Opcode not implemented: " << std::hex << (int)opcode << std::endl;
            System::instance->stop = true;
            break;
    }

    stepCpu(1);
}
