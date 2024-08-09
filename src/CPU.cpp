#include <CPU.hpp>

#include <System.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

CPU::CPU(Bus* memory) {
    this->memory = memory;
}

void CPU::powerOn() {
    programCounter = memory->readWord(0xFFFC);
    // programCounter = 0xC000;
    oldPC = programCounter;
    accumulator = 0;
    xIndex = 0;
    yIndex = 0;
    stackPointer = 0xFD;
    flags = 0x24;
    oldAccumulator = accumulator;
    oldXIndex = xIndex;
    oldYIndex = yIndex;
    oldFlags = flags;
    oldStackPointer = stackPointer;
    branchLocation = 0;
    cycles = 4;
    miscValue = 0;
    stepCountAfterFatalError = 0;
    pageCrossed = false;
}

void CPU::reset() {
    programCounter = memory->readWord(0xFFFC);
    stackPointer = stackPointer - 3;
    flags = 0x24;
    cycles = 4;
    pageCrossed = false;
}

size_t CPU::getCycles() {
    return cycles;
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

uint8_t CPU::getInterruptDisable() {
    return (flags & 0x04) >> 2;
}

void CPU::setInterruptDisable(bool value) {
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
    fetchLogs.push_back(data);
    return data;
}

uint16_t CPU::fetchWord() {
    uint16_t data = memory->readWord(programCounter);
    programCounter += 2;
    fetchLogs.push_back(data & 0xFF);
    fetchLogs.push_back(data >> 8);
    return data;
}

uint8_t CPU::fetchNoAdvance() {
    return memory->read(programCounter);
}

uint16_t CPU::fetchWordNoAdvance() {
    return memory->readWord(programCounter);
}

void CPU::stepTo(uint64_t cycle) {
    while (cycles < cycle && !System::instance->stop) {
        execOnce();
    }
}

void CPU::stepCpu(uint64_t cycle) {
    cycles += cycle;
}

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

std::string CPU::log() {
    std::stringstream ss;
    ss << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << (int)oldPC;
    ss << " ";
    for (uint8_t data : fetchLogs) {
        ss << " " << std::hex << std::setw(2) << std::setfill('0') << (int)data;
    }
    std::string instruction = getInstruction();

    int length = ss.str().length();
    for (int i = 0; i < 16 - length - startsWith(instruction, "*"); i++) {
        ss << " ";
    }
    ss << instruction;

    // pad to 48 characters
    length = ss.str().length();
    for (int i = 0; i < 48 - length; i++) {
        ss << " ";
    }

    ss << "A:" << std::hex << std::setw(2) << std::setfill('0') << (int)oldAccumulator;
    ss << " X:" << std::hex << std::setw(2) << std::setfill('0') << (int)oldXIndex;
    ss << " Y:" << std::hex << std::setw(2) << std::setfill('0') << (int)oldYIndex;
    ss << " P:" << std::hex << std::setw(2) << std::setfill('0') << (int)oldFlags;
    ss << " SP:" << std::hex << std::setw(2) << std::setfill('0') << (int)oldStackPointer;
    // ss << " PPU:  0, 21";
    // ss << " CYC:" << std::dec << cycles;
    oldOldAccumulator = oldAccumulator;
    oldOldXIndex = oldXIndex;
    oldOldYIndex = oldYIndex;
    oldOldFlags = oldFlags;
    oldOldStackPointer = oldStackPointer;

    oldPC = programCounter;
    oldAccumulator = accumulator;
    oldXIndex = xIndex;
    oldYIndex = yIndex;
    oldFlags = flags;
    oldStackPointer = stackPointer;
    return ss.str();
}

uint8_t wrapingAdd(uint8_t a, uint8_t b) {
    uint16_t result = a + b;
    return result & 0xFF;
}

// i messed up so instead of imm returning the program counter it returns the value at the program counter
uint16_t CPU::getAddress(AddressingMode mode) {
    switch (mode) {
        case AddressingMode::ACC:
            stepCpu(2);
            return 0;
        case AddressingMode::IMP:
            stepCpu(2);
            return 0;
        case AddressingMode::IMM:
            stepCpu(2);
            return fetch();
        case AddressingMode::ZP0:
            stepCpu(3);
            return fetch();
        case AddressingMode::ZPX:
            stepCpu(4);
            return (fetch() + xIndex) & 0xFF;
        case AddressingMode::ZPY:
            stepCpu(4);
            return (fetch() + yIndex) & 0xFF;
        case AddressingMode::REL:
            return programCounter + (int8_t)fetch();
        case AddressingMode::ABS:
            stepCpu(4);
            return fetchWord();
        case AddressingMode::ABX:
            {
                uint16_t addr = fetchWord();
                if (mode == AddressingMode::ABX && (addr & 0xFF00) != ((addr + xIndex) & 0xFF00)) {
                    stepCpu(1);
                }
                stepCpu(4);
                return addr + xIndex;
            }
        case AddressingMode::ABY: 
            {
                uint16_t addr = fetchWord();
                if (mode == AddressingMode::ABY && (addr & 0xFF00) != ((addr + yIndex) & 0xFF00)) {
                    stepCpu(1);
                }
                stepCpu(4);
                return addr + yIndex;
            }
        case AddressingMode::IND:
            return memory->read(getAddress(AddressingMode::ABS));
        case AddressingMode::IZX:
            {
                stepCpu(6);
                uint8_t addr = fetch();
                return memory->read((addr + xIndex) & 0xFF) + (uint16_t(memory->read((addr + xIndex + 1) & 0xFF)) << 8);
            }
        case AddressingMode::IZY:
            {
                uint8_t base = fetch();
                uint8_t lo = memory->read(base);
                uint8_t hi = memory->read((base + 1) & 0xFF);

                uint16_t deref_base = ((uint16_t)lo) | ((uint16_t)hi << 8);
                uint16_t deref = deref_base + yIndex;
                if ((deref_base & 0xFF00) != (deref & 0xFF00)) {
                    stepCpu(1);
                }
                stepCpu(5);
                return deref;
            }
    }
    return 0;
}

std::string CPU::getAddressWithMode(AddressingMode mode) {
    std::stringstream ss;
    ss << std::uppercase;
    switch (mode) {
        case AddressingMode::IMP:
            break;
        case AddressingMode::ACC:
            ss << "A";
            break;
        case AddressingMode::IMM:
            ss << "#$" << std::hex << std::setw(2) << std::setfill('0') << (int)fetchLogs[1];
            break;
        case AddressingMode::ZP0:
            ss << "$" << std::hex << std::setw(2) << std::setfill('0') << (int)fetchLogs[1];
            break;
        case AddressingMode::ZPX:
            ss << "$" << std::hex << std::setw(2) << std::setfill('0') << (int)fetchLogs[1] << ",X";
            break;
        case AddressingMode::ZPY:
            ss << "$" << std::hex << std::setw(2) << std::setfill('0') << (int)fetchLogs[1] << ",Y";
            break;
        case AddressingMode::REL:
            ss << "$" << std::hex << std::setw(4) << std::setfill('0') << (int)branchLocation;
            break;
        case AddressingMode::ABS:
            ss << "$" << std::hex << std::setw(4) << std::setfill('0') << (int)fetchLogs[1] + (fetchLogs[2] << 8);
            break;
        case AddressingMode::ABX:
            ss << "$" << std::hex << std::setw(4) << std::setfill('0') << (int)fetchLogs[1] + (fetchLogs[2] << 8) << ",X";
            break;
        case AddressingMode::ABY:
            ss << "$" << std::hex << std::setw(4) << std::setfill('0') << (int)fetchLogs[1] + (fetchLogs[2] << 8) << ",Y";
            break;
        case AddressingMode::IND:
            ss << "($" << std::hex << std::setw(4) << std::setfill('0') << (int)fetchLogs[1] + (fetchLogs[2] << 8) << ")";
            break;
        case AddressingMode::IZX:
            ss << "($" << std::hex << std::setw(2) << std::setfill('0') << (int)fetchLogs[1] << ",X)";
            break;
        case AddressingMode::IZY:
            ss << "($" << std::hex << std::setw(2) << std::setfill('0') << (int)fetchLogs[1] << "),Y";
            break;
        case AddressingMode::NOP:
            break;
    }

    return ss.str();
}

std::string CPU::getInstruction() {
    uint8_t opcode = fetchLogs[0];

    std::string opcodeName = "NOP";
    AddressingMode mode = AddressingMode::NOP;

    switch (opcode) {
        case 0x00:
            opcodeName = "BRK";
            mode = AddressingMode::IMP;
            break;

        case 0x01:
            opcodeName = "ORA";
            mode = AddressingMode::IZX;
            break;

        case 0x02:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x03:
            opcodeName = "*SLO";
            mode = AddressingMode::IZX;
            break;

        case 0x04:
            opcodeName = "*NOP";
            mode = AddressingMode::ZP0;
            break;

        case 0x05:
            opcodeName = "ORA";
            mode = AddressingMode::ZP0;
            break;

        case 0x06:
            opcodeName = "ASL";
            mode = AddressingMode::ZP0;
            break;

        case 0x07:
            opcodeName = "*SLO";
            mode = AddressingMode::ZP0;
            break;

        case 0x08:
            opcodeName = "PHP";
            mode = AddressingMode::IMP;
            break;

        case 0x09:
            opcodeName = "ORA";
            mode = AddressingMode::IMM;
            break;

        case 0x0A:
            opcodeName = "ASL";
            mode = AddressingMode::ACC;
            break;

        case 0x0B:
            opcodeName = "AAC";
            mode = AddressingMode::IMM;
            break;
        
        case 0x0C:
            opcodeName = "*NOP";
            mode = AddressingMode::ABS;
            break;

        case 0x0D:
            opcodeName = "ORA";
            mode = AddressingMode::ABS;
            break;

        case 0x0E:
            opcodeName = "ASL";
            mode = AddressingMode::ABS;
            break;

        case 0x0F:
            opcodeName = "*SLO";
            mode = AddressingMode::ABS;
            break;

        case 0x10:
            opcodeName = "BPL";
            mode = AddressingMode::REL;
            break;
        
        case 0x11:
            opcodeName = "ORA";
            mode = AddressingMode::IZY;
            break;
        
        case 0x12:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x13:
            opcodeName = "*SLO";
            mode = AddressingMode::IZY;
            break;

        case 0x14:
            opcodeName = "*NOP";
            mode = AddressingMode::ZPX;
            break;

        case 0x15:
            opcodeName = "ORA";
            mode = AddressingMode::ZPX;
            break;

        case 0x16:
            opcodeName = "ASL";
            mode = AddressingMode::ZPX;
            break;

        case 0x17:
            opcodeName = "*SLO";
            mode = AddressingMode::ZPX;
            break;

        case 0x18:
            opcodeName = "CLC";
            mode = AddressingMode::IMP;
            break;

        case 0x19:
            opcodeName = "ORA";
            mode = AddressingMode::ABY;
            break;
        
        case 0x1A:
            opcodeName = "*NOP";
            mode = AddressingMode::IMP;
            break;

        case 0x1B:
            opcodeName = "*SLO";
            mode = AddressingMode::ABY;
            break;

        case 0x1C:
            opcodeName = "*NOP";
            mode = AddressingMode::ABX;
            break;

        case 0x1D:
            opcodeName = "ORA";
            mode = AddressingMode::ABX;
            break;

        case 0x1E:
            opcodeName = "ASL";
            mode = AddressingMode::ABX;
            break;

        case 0x1F:
            opcodeName = "*SLO";
            mode = AddressingMode::ABX;
            break;

        case 0x20:
            opcodeName = "JSR";
            mode = AddressingMode::ABS;
            break;
        
        case 0x21:
            opcodeName = "AND";
            mode = AddressingMode::IZX;
            break;

        case 0x22:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x23:
            opcodeName = "*RLA";
            mode = AddressingMode::IZX;
            break;

        case 0x24:
            opcodeName = "BIT";
            mode = AddressingMode::ZP0;
            break;

        case 0x25:
            opcodeName = "AND";
            mode = AddressingMode::ZP0;
            break;

        case 0x26:
            opcodeName = "ROL";
            mode = AddressingMode::ZP0;
            break;

        case 0x27:
            opcodeName = "*RLA";
            mode = AddressingMode::ZP0;
            break;

        case 0x28:
            opcodeName = "PLP";
            mode = AddressingMode::IMP;
            break;

        case 0x29:
            opcodeName = "AND";
            mode = AddressingMode::IMM;
            break;

        case 0x2A:
            opcodeName = "ROL";
            mode = AddressingMode::ACC;
            break;

        case 0x2B:
            opcodeName = "ANC";
            mode = AddressingMode::IMM;
            break;

        case 0x2C:
            opcodeName = "BIT";
            mode = AddressingMode::ABS;
            break;

        case 0x2D:
            opcodeName = "AND";
            mode = AddressingMode::ABS;
            break;

        case 0x2E:
            opcodeName = "ROL";
            mode = AddressingMode::ABS;
            break;

        case 0x2F:
            opcodeName = "*RLA";
            mode = AddressingMode::ABS;
            break;

        case 0x30:
            opcodeName = "BMI";
            mode = AddressingMode::REL;
            break;

        case 0x31:
            opcodeName = "AND";
            mode = AddressingMode::IZY;
            break;

        case 0x32:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x33:
            opcodeName = "*RLA";
            mode = AddressingMode::IZY;
            break;

        case 0x34:
            opcodeName = "*NOP";
            mode = AddressingMode::ZPX;
            break;

        case 0x35:
            opcodeName = "AND";
            mode = AddressingMode::ZPX;
            break;

        case 0x36:
            opcodeName = "ROL";
            mode = AddressingMode::ZPX;
            break;

        case 0x37:
            opcodeName = "*RLA";
            mode = AddressingMode::ZPX;
            break;

        case 0x38:
            opcodeName = "SEC";
            mode = AddressingMode::IMP;
            break;

        case 0x39:
            opcodeName = "AND";
            mode = AddressingMode::ABY;
            break;

        case 0x3A:
            opcodeName = "*NOP";
            mode = AddressingMode::IMP;
            break;

        case 0x3B:
            opcodeName = "*RLA";
            mode = AddressingMode::ABY;
            break;

        case 0x3C:
            opcodeName = "*NOP";
            mode = AddressingMode::ABX;
            break;

        case 0x3D:
            opcodeName = "AND";
            mode = AddressingMode::ABX;
            break;

        case 0x3E:
            opcodeName = "ROL";
            mode = AddressingMode::ABX;
            break;

        case 0x3F:
            opcodeName = "*RLA";
            mode = AddressingMode::ABX;
            break;

        case 0x40:
            opcodeName = "RTI";
            mode = AddressingMode::IMP;
            break;

        case 0x41:
            opcodeName = "EOR";
            mode = AddressingMode::IZX;
            break;

        case 0x42:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x43:
            opcodeName = "*SRE";
            mode = AddressingMode::IZX;
            break;

        case 0x44:
            opcodeName = "*NOP";
            mode = AddressingMode::ZP0;
            break;

        case 0x45:
            opcodeName = "EOR";
            mode = AddressingMode::ZP0;
            break;

        case 0x46:
            opcodeName = "LSR";
            mode = AddressingMode::ZP0;
            break;

        case 0x47:
            opcodeName = "*SRE";
            mode = AddressingMode::ZP0;
            break;

        case 0x48:
            opcodeName = "PHA";
            mode = AddressingMode::IMP;
            break;

        case 0x49:
            opcodeName = "EOR";
            mode = AddressingMode::IMM;
            break;

        case 0x4A:
            opcodeName = "LSR";
            mode = AddressingMode::ACC;
            break;

        case 0x4B:
            opcodeName = "ASR";
            mode = AddressingMode::IMM;
            break;

        case 0x4C:
            opcodeName = "JMP";
            mode = AddressingMode::ABS;
            break;

        case 0x4D:
            opcodeName = "EOR";
            mode = AddressingMode::ABS;
            break;

        case 0x4E:
            opcodeName = "LSR";
            mode = AddressingMode::ABS;
            break;

        case 0x4F:
            opcodeName = "*SRE";
            mode = AddressingMode::ABS;
            break;

        case 0x50:
            opcodeName = "BVC";
            mode = AddressingMode::REL;
            break;

        case 0x51:
            opcodeName = "EOR";
            mode = AddressingMode::IZY;
            break;

        case 0x52:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x53:
            opcodeName = "*SRE";
            mode = AddressingMode::IZY;
            break;
        
        case 0x54:
            opcodeName = "*NOP";
            mode = AddressingMode::ZPX;
            break;

        case 0x55:
            opcodeName = "EOR";
            mode = AddressingMode::ZPX;
            break;

        case 0x56:
            opcodeName = "LSR";
            mode = AddressingMode::ZPX;
            break;

        case 0x57:
            opcodeName = "*SRE";
            mode = AddressingMode::ZPX;
            break;

        case 0x58:
            opcodeName = "CLI";
            mode = AddressingMode::IMP;
            break;

        case 0x59:
            opcodeName = "EOR";
            mode = AddressingMode::ABY;
            break;

        case 0x5A:
            opcodeName = "*NOP";
            mode = AddressingMode::IMP;
            break;

        case 0x5B:
            opcodeName = "*SRE";
            mode = AddressingMode::ABY;
            break;

        case 0x5C:
            opcodeName = "*NOP";
            mode = AddressingMode::ABX;
            break;

        case 0x5D:
            opcodeName = "EOR";
            mode = AddressingMode::ABX;
            break;

        case 0x5E:
            opcodeName = "LSR";
            mode = AddressingMode::ABX;
            break;

        case 0x5F:
            opcodeName = "*SRE";
            mode = AddressingMode::ABX;
            break;

        case 0x60:
            opcodeName = "RTS";
            mode = AddressingMode::IMP;
            break;

        case 0x61:
            opcodeName = "ADC";
            mode = AddressingMode::IZX;
            break;

        case 0x62:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x63:
            opcodeName = "*RRA";
            mode = AddressingMode::IZX;
            break;

        case 0x64:
            opcodeName = "*NOP";
            mode = AddressingMode::ZP0;
            break;

        case 0x65:
            opcodeName = "ADC";
            mode = AddressingMode::ZP0;
            break;

        case 0x66:
            opcodeName = "ROR";
            mode = AddressingMode::ZP0;
            break;

        case 0x67:
            opcodeName = "*RRA";
            mode = AddressingMode::ZP0;
            break;

        case 0x68:
            opcodeName = "PLA";
            mode = AddressingMode::IMP;
            break;

        case 0x69:
            opcodeName = "ADC";
            mode = AddressingMode::IMM;
            break;

        case 0x6A:
            opcodeName = "ROR";
            mode = AddressingMode::ACC;
            break;

        case 0x6B:
            opcodeName = "ARR";
            mode = AddressingMode::IMM;
            break;

        case 0x6C:
            opcodeName = "JMP";
            mode = AddressingMode::IND;
            break;

        case 0x6D:
            opcodeName = "ADC";
            mode = AddressingMode::ABS;
            break;

        case 0x6E:
            opcodeName = "ROR";
            mode = AddressingMode::ABS;
            break;

        case 0x6F:
            opcodeName = "*RRA";
            mode = AddressingMode::ABS;
            break;

        case 0x70:
            opcodeName = "BVS";
            mode = AddressingMode::REL;
            break;

        case 0x71:
            opcodeName = "ADC";
            mode = AddressingMode::IZY;
            break;

        case 0x72:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x73:
            opcodeName = "*RRA";
            mode = AddressingMode::IZY;
            break;

        case 0x74:
            opcodeName = "*NOP";
            mode = AddressingMode::ZPX;
            break;

        case 0x75:
            opcodeName = "ADC";
            mode = AddressingMode::ZPX;
            break;

        case 0x76:
            opcodeName = "ROR";
            mode = AddressingMode::ZPX;
            break;

        case 0x77:
            opcodeName = "*RRA";
            mode = AddressingMode::ZPX;
            break;

        case 0x78:
            opcodeName = "SEI";
            mode = AddressingMode::IMP;
            break;

        case 0x79:
            opcodeName = "ADC";
            mode = AddressingMode::ABY;
            break;

        case 0x7A:
            opcodeName = "*NOP";
            mode = AddressingMode::IMP;
            break;

        case 0x7B:
            opcodeName = "*RRA";
            mode = AddressingMode::ABY;
            break;

        case 0x7C:
            opcodeName = "*NOP";
            mode = AddressingMode::ABX;
            break;

        case 0x7D:
            opcodeName = "ADC";
            mode = AddressingMode::ABX;
            break;

        case 0x7E:
            opcodeName = "ROR";
            mode = AddressingMode::ABX;
            break;

        case 0x7F:
            opcodeName = "*RRA";
            mode = AddressingMode::ABX;
            break;

        case 0x80:
            opcodeName = "*NOP";
            mode = AddressingMode::IMM;
            break;

        case 0x81:
            opcodeName = "STA";
            mode = AddressingMode::IZX;
            break;

        case 0x82:
            opcodeName = "NOP";
            mode = AddressingMode::IMM;
            break;

        case 0x83:
            opcodeName = "*SAX";
            mode = AddressingMode::IZX;
            break;

        case 0x84:
            opcodeName = "STY";
            mode = AddressingMode::ZP0;
            break;

        case 0x85:
            opcodeName = "STA";
            mode = AddressingMode::ZP0;
            break;

        case 0x86:
            opcodeName = "STX";
            mode = AddressingMode::ZP0;
            break;

        case 0x87:
            opcodeName = "*SAX";
            mode = AddressingMode::ZP0;
            break;

        case 0x88:
            opcodeName = "DEY";
            mode = AddressingMode::IMP;
            break;

        case 0x89:
            opcodeName = "NOP";
            mode = AddressingMode::IMM;
            break;

        case 0x8A:
            opcodeName = "TXA";
            mode = AddressingMode::IMP;
            break;

        case 0x8B:
            opcodeName = "XAA";
            mode = AddressingMode::IMM;
            break;

        case 0x8C:
            opcodeName = "STY";
            mode = AddressingMode::ABS;
            break;

        case 0x8D:
            opcodeName = "STA";
            mode = AddressingMode::ABS;
            break;

        case 0x8E:
            opcodeName = "STX";
            mode = AddressingMode::ABS;
            break;

        case 0x8F:
            opcodeName = "*SAX";
            mode = AddressingMode::ABS;
            break;

        case 0x90:
            opcodeName = "BCC";
            mode = AddressingMode::REL;
            break;

        case 0x91:
            opcodeName = "STA";
            mode = AddressingMode::IZY;
            break;

        case 0x92:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0x93:
            opcodeName = "AHX";
            mode = AddressingMode::IZY;
            break;

        case 0x94:
            opcodeName = "STY";
            mode = AddressingMode::ZPX;
            break;

        case 0x95:
            opcodeName = "STA";
            mode = AddressingMode::ZPX;
            break;

        case 0x96:
            opcodeName = "STX";
            mode = AddressingMode::ZPY;
            break;

        case 0x97:
            opcodeName = "*SAX";
            mode = AddressingMode::ZPY;
            break;

        case 0x98:
            opcodeName = "TYA";
            mode = AddressingMode::IMP;
            break;

        case 0x99:
            opcodeName = "STA";
            mode = AddressingMode::ABY;
            break;

        case 0x9A:
            opcodeName = "TXS";
            mode = AddressingMode::IMP;
            break;

        case 0x9B:
            opcodeName = "TAS";
            mode = AddressingMode::ABY;
            break;

        case 0x9C:
            opcodeName = "SHY";
            mode = AddressingMode::ABX;
            break;

        case 0x9D:
            opcodeName = "STA";
            mode = AddressingMode::ABX;
            break;

        case 0x9E:
            opcodeName = "SHX";
            mode = AddressingMode::ABY;
            break;

        case 0x9F:
            opcodeName = "AHX";
            mode = AddressingMode::ABY;
            break;

        case 0xA0:
            opcodeName = "LDY";
            mode = AddressingMode::IMM;
            break;

        case 0xA1:
            opcodeName = "LDA";
            mode = AddressingMode::IZX;
            break;

        case 0xA2:
            opcodeName = "LDX";
            mode = AddressingMode::IMM;
            break;

        case 0xA3:
            opcodeName = "*LAX";
            mode = AddressingMode::IZX;
            break;

        case 0xA4:
            opcodeName = "LDY";
            mode = AddressingMode::ZP0;
            break;

        case 0xA5:
            opcodeName = "LDA";
            mode = AddressingMode::ZP0;
            break;

        case 0xA6:
            opcodeName = "LDX";
            mode = AddressingMode::ZP0;
            break;

        case 0xA7:
            opcodeName = "*LAX";
            mode = AddressingMode::ZP0;
            break;

        case 0xA8:
            opcodeName = "TAY";
            mode = AddressingMode::IMP;
            break;

        case 0xA9:
            opcodeName = "LDA";
            mode = AddressingMode::IMM;
            break;

        case 0xAA:
            opcodeName = "TAX";
            mode = AddressingMode::IMP;
            break;

        case 0xAB:
            opcodeName = "LAX";
            mode = AddressingMode::IMM;
            break;

        case 0xAC:
            opcodeName = "LDY";
            mode = AddressingMode::ABS;
            break;

        case 0xAD:
            opcodeName = "LDA";
            mode = AddressingMode::ABS;
            break;

        case 0xAE:
            opcodeName = "LDX";
            mode = AddressingMode::ABS;
            break;

        case 0xAF:
            opcodeName = "*LAX";
            mode = AddressingMode::ABS;
            break;

        case 0xB0:
            opcodeName = "BCS";
            mode = AddressingMode::REL;
            break;

        case 0xB1:
            opcodeName = "LDA";
            mode = AddressingMode::IZY;
            break;

        case 0xB2:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0xB3:
            opcodeName = "*LAX";
            mode = AddressingMode::IZY;
            break;

        case 0xB4:
            opcodeName = "LDY";
            mode = AddressingMode::ZPX;
            break;

        case 0xB5:
            opcodeName = "LDA";
            mode = AddressingMode::ZPX;
            break;

        case 0xB6:
            opcodeName = "LDX";
            mode = AddressingMode::ZPY;
            break;
            
        case 0xB7:
            opcodeName = "*LAX";
            mode = AddressingMode::ZPY;
            break;

        case 0xB8:
            opcodeName = "CLV";
            mode = AddressingMode::IMP;
            break;

        case 0xB9:
            opcodeName = "LDA";
            mode = AddressingMode::ABY;
            break;

        case 0xBA:
            opcodeName = "TSX";
            mode = AddressingMode::IMP;
            break;

        case 0xBB:
            opcodeName = "LAS";
            mode = AddressingMode::ABY;
            break;

        case 0xBC:
            opcodeName = "LDY";
            mode = AddressingMode::ABX;
            break;

        case 0xBD:
            opcodeName = "LDA";
            mode = AddressingMode::ABX;
            break;

        case 0xBE:
            opcodeName = "LDX";
            mode = AddressingMode::ABY;
            break;

        case 0xBF:
            opcodeName = "*LAX";
            mode = AddressingMode::ABY;
            break;

        case 0xC0:
            opcodeName = "CPY";
            mode = AddressingMode::IMM;
            break;

        case 0xC1:
            opcodeName = "CMP";
            mode = AddressingMode::IZX;
            break;

        case 0xC2:
            opcodeName = "NOP";
            mode = AddressingMode::IMM;
            break;

        case 0xC3:
            opcodeName = "*DCP";
            mode = AddressingMode::IZX;
            break;

        case 0xC4:
            opcodeName = "CPY";
            mode = AddressingMode::ZP0;
            break;

        case 0xC5:
            opcodeName = "CMP";
            mode = AddressingMode::ZP0;
            break;

        case 0xC6:
            opcodeName = "DEC";
            mode = AddressingMode::ZP0;
            break;

        case 0xC7:
            opcodeName = "*DCP";
            mode = AddressingMode::ZP0;
            break;

        case 0xC8:
            opcodeName = "INY";
            mode = AddressingMode::IMP;
            break;

        case 0xC9:
            opcodeName = "CMP";
            mode = AddressingMode::IMM;
            break;

        case 0xCA:
            opcodeName = "DEX";
            mode = AddressingMode::IMP;
            break;

        case 0xCB:
            opcodeName = "AXS";
            mode = AddressingMode::IMM;
            break;

        case 0xCC:
            opcodeName = "CPY";
            mode = AddressingMode::ABS;
            break;

        case 0xCD:
            opcodeName = "CMP";
            mode = AddressingMode::ABS;
            break;

        case 0xCE:
            opcodeName = "DEC";
            mode = AddressingMode::ABS;
            break;

        case 0xCF:
            opcodeName = "*DCP";
            mode = AddressingMode::ABS;
            break;

        case 0xD0:
            opcodeName = "BNE";
            mode = AddressingMode::REL;
            break;

        case 0xD1:
            opcodeName = "CMP";
            mode = AddressingMode::IZY;
            break;

        case 0xD2:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0xD3:
            opcodeName = "*DCP";
            mode = AddressingMode::IZY;
            break;

        case 0xD4:
            opcodeName = "*NOP";
            mode = AddressingMode::ZPX;
            break;

        case 0xD5:
            opcodeName = "CMP";
            mode = AddressingMode::ZPX;
            break;

        case 0xD6:
            opcodeName = "DEC";
            mode = AddressingMode::ZPX;
            break;

        case 0xD7:
            opcodeName = "*DCP";
            mode = AddressingMode::ZPX;
            break;

        case 0xD8:
            opcodeName = "CLD";
            mode = AddressingMode::IMP;
            break;

        case 0xD9:
            opcodeName = "CMP";
            mode = AddressingMode::ABY;
            break;

        case 0xDA:
            opcodeName = "*NOP";
            mode = AddressingMode::IMP;
            break;

        case 0xDB:
            opcodeName = "*DCP";
            mode = AddressingMode::ABY;
            break;

        case 0xDC:
            opcodeName = "*NOP";
            mode = AddressingMode::ABX;
            break;

        case 0xDD:
            opcodeName = "CMP";
            mode = AddressingMode::ABX;
            break;

        case 0xDE:
            opcodeName = "DEC";
            mode = AddressingMode::ABX;
            break;

        case 0xDF:
            opcodeName = "*DCP";
            mode = AddressingMode::ABX;
            break;

        case 0xE0:
            opcodeName = "CPX";
            mode = AddressingMode::IMM;
            break;

        case 0xE1:
            opcodeName = "SBC";
            mode = AddressingMode::IZX;
            break;

        case 0xE2:
            opcodeName = "NOP";
            mode = AddressingMode::IMM;
            break;

        case 0xE3:
            opcodeName = "*ISB";
            mode = AddressingMode::IZX;
            break;

        case 0xE4:
            opcodeName = "CPX";
            mode = AddressingMode::ZP0;
            break;

        case 0xE5:
            opcodeName = "SBC";
            mode = AddressingMode::ZP0;
            break;

        case 0xE6:
            opcodeName = "INC";
            mode = AddressingMode::ZP0;
            break;

        case 0xE7:
            opcodeName = "*ISB";
            mode = AddressingMode::ZP0;
            break;

        case 0xE8:
            opcodeName = "INX";
            mode = AddressingMode::IMP;
            break;

        case 0xE9:
            opcodeName = "SBC";
            mode = AddressingMode::IMM;
            break;

        case 0xEA:
            opcodeName = "NOP";
            mode = AddressingMode::IMP;
            break;

        case 0xEB:
            opcodeName = "*SBC";
            mode = AddressingMode::IMM;
            break;

        case 0xEC:
            opcodeName = "CPX";
            mode = AddressingMode::ABS;
            break;

        case 0xED:
            opcodeName = "SBC";
            mode = AddressingMode::ABS;
            break;

        case 0xEE:
            opcodeName = "INC";
            mode = AddressingMode::ABS;
            break;

        case 0xEF:
            opcodeName = "*ISB";
            mode = AddressingMode::ABS;
            break;

        case 0xF0:
            opcodeName = "BEQ";
            mode = AddressingMode::REL;
            break;

        case 0xF1:
            opcodeName = "SBC";
            mode = AddressingMode::IZY;
            break;

        case 0xF2:
            opcodeName = "KIL";
            mode = AddressingMode::NOP;
            break;

        case 0xF3:
            opcodeName = "*ISB";
            mode = AddressingMode::IZY;
            break;

        case 0xF4:
            opcodeName = "*NOP";
            mode = AddressingMode::ZPX;
            break;

        case 0xF5:
            opcodeName = "SBC";
            mode = AddressingMode::ZPX;
            break;

        case 0xF6:
            opcodeName = "INC";
            mode = AddressingMode::ZPX;
            break;

        case 0xF7:
            opcodeName = "*ISB";
            mode = AddressingMode::ZPX;
            break;

        case 0xF8:
            opcodeName = "SED";
            mode = AddressingMode::IMP;
            break;

        case 0xF9:
            opcodeName = "SBC";
            mode = AddressingMode::ABY;
            break;

        case 0xFA:
            opcodeName = "*NOP";
            mode = AddressingMode::IMP;
            break;

        case 0xFB:
            opcodeName = "*ISB";
            mode = AddressingMode::ABY;
            break;

        case 0xFC:
            opcodeName = "*NOP";
            mode = AddressingMode::ABX;
            break;

        case 0xFD:
            opcodeName = "SBC";
            mode = AddressingMode::ABX;
            break;

        case 0xFE:
            opcodeName = "INC";
            mode = AddressingMode::ABX;
            break;

        case 0xFF:
            opcodeName = "*ISB";
            mode = AddressingMode::ABX;
            break;
    }

    std::stringstream ss;

    ss << opcodeName << " ";
    ss << getAddressWithMode(mode);
    // if its an sta, stx, or sty we need to print what is being stored
    // if its a bit we need to print the bit
    ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0');
    if ((opcodeName == "STA" || opcodeName == "STX" || opcodeName == "STY") && mode != AddressingMode::IZX) {
        ss << " = " << std::setw(2) << (int)miscValue;
    } else if ((opcodeName == "LDX" || opcodeName == "LDY" || opcodeName == "LDA" || opcodeName == "ORA" || opcodeName == "BIT") && mode != AddressingMode::IMM && mode != AddressingMode::IZX) {
        ss << " = " << std::setw(2) << (int)miscValue;
    } else if ((opcodeName == "LDX" || opcodeName == "LDY" || opcodeName == "LDA" || opcodeName == "STA" || opcodeName == "STX" || opcodeName == "STY" || opcodeName == "ORA" || opcodeName == "EOR" || opcodeName == "ADC" || opcodeName == "CMP" || opcodeName == "SBC") && mode == AddressingMode::IZX) {
        ss << " @ " << std::setw(2) << (int)indirectAddr1 << " = " << std::setw(4) << (int)indirectAddr2 << " = " << std::setw(2) << (int)miscValue;
    } else if ((opcodeName == "*DCP") && mode == AddressingMode::IZY) {
        ss << " = " << std::setw(2) << (int)indirectAddr1 << " @ " << std::setw(4) << (int)indirectAddr2 << " = " << std::setw(2) << (int)miscValue;
    }

    return ss.str();
}

void CPU::interrupt(const interrupt::Interrupt& interrupt) {
    pushWord(programCounter);
    uint8_t flag = this->flags;
    setBFlag((interrupt.b_flag_mask & 0b010000) == 0b010000);
    setIFlag((interrupt.b_flag_mask & 0b100000) == 0b100000);
    pushByte(flag);
    setInterruptDisable(true);
    System::instance->stepThisAndPPU(interrupt.cpu_cycles);
    programCounter = memory->readWord(interrupt.vector_addr);
}

void CPU::execOnce() {
    fetchLogs.clear();
    if (System::instance->stop) {
        return;
    }

    if (bool nmiStatus = memory->pollNmiStatus(); nmiStatus) {
        interrupt(interrupt::NMI);
    }

    uint8_t opcode = fetch();

    switch (opcode) {
        case 0x00: // BRK
            programCounter++;
            pushWord(programCounter);
            pushByte(flags);
            setInterruptDisable(true);
            programCounter = memory->readWord(0xFFFE);
            stepCpu(7);
            break;
        case 0x01: // ORA Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
                accumulator |= memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x02: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x03: // Illegal *SLO Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
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
            stepCpu(2);
            break;

        case 0x04: // Illegal *NOP Zero Page
            fetch();
            stepCpu(3);
            break;

        case 0x05: // ORA Zero Page
            accumulator |= memory->read(getAddress(AddressingMode::ZP0));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;
        
        case 0x06: // ASL Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x07: // Illegal *SLO Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
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
            stepCpu(2);
            break;  

        case 0x08: // PHP
            pushByte(flags);
            stepCpu(3);
            break;

        case 0x09: // ORA Immediate
            accumulator |= getAddress(AddressingMode::IMM);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x0A: // ASL Accumulator
            setCarry(accumulator & 0x80);
            accumulator <<= 1;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            stepCpu(2); 
            break;

        case 0x0B: // Illegal ANC Immediate
        case 0x2B: // Illegal ANC Immediate aagain
            accumulator &= getAddress(AddressingMode::IMM);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            setCarry(accumulator & 0x80);
            break;

        case 0x0C: // Illegal TOP Absolute
            getAddress(AddressingMode::ABS);
            break;

        case 0x0D: // ORA Absolute
            accumulator |= memory->read(getAddress(AddressingMode::ABS));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x0E: // ASL Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x0F: // Illegal *SLO Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
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
            stepCpu(2);
            break;

        case 0x10: // BPL
            if (!getNegative()) {
                int8_t offset = (int8_t)fetch();
                uint16_t newAddress = programCounter + offset;
                branchLocation = newAddress;
                if ((newAddress & 0xFF00) != (programCounter & 0xFF00)) {
                    stepCpu(1);
                }
                programCounter = newAddress;
                stepCpu(1);
            } else {
                branchLocation = programCounter + (int8_t)fetch() + 1;
            }
            stepCpu(2);
            break;

        case 0x11: // ORA Indirect, Indexed
            {
                uint8_t data = memory->read(getAddress(AddressingMode::IZY));
                accumulator |= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x12: // Illegal KIL
            System::instance->stop = true;
            break;
            
        case 0x13: // Illegal *SLO (Indirect), Y
            {
                uint8_t base = fetch();
                uint16_t address = ((uint16_t(memory->read((base + 1) & 0xff)) << 8) | memory->read(base)) + yIndex;
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
            stepCpu(8);
            break;

        case 0x14: // Illegal *NOP Zero Page, X
            fetch();
            stepCpu(4);
            break;

        case 0x15: // ORA Zero Page, X
            accumulator |= memory->read(getAddress(AddressingMode::ZPX));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x16: // ASL Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
                uint8_t data = memory->read(address);
                setCarry(data & 0x80);
                data <<= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x17: // Illegal *SLO Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
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
            stepCpu(2);
            break;

        case 0x18: // CLC
            setCarry(false);
            stepCpu(2);
            break;

        case 0x19: // ORA Absolute, Y
            accumulator |= memory->read(getAddress(AddressingMode::ABY));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x1A: // Illegal NOP
            stepCpu(2);
            break;

        case 0x1B: // Illegal *SLO Absolute, Y
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
            stepCpu(7);
            break;

        case 0x1C: // Illegal TOP Absolute, X
            getAddress(AddressingMode::ABX);
            break;

        case 0x1D: // ORA Absolute, X
            {
                accumulator |= memory->read(getAddress(AddressingMode::ABX));
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
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
            stepCpu(7);
            break;

        case 0x1F: // Illegal *SLO Absolute, X
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
            stepCpu(7);
            break;

        case 0x20: // JSR
            pushWord(programCounter + 1);
            programCounter = fetchWord();
            stepCpu(6);
            break;

        case 0x21: // AND Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
                accumulator &= memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x22: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x23: // Illegal *RLA (Indirect,X)
            {
                uint16_t address = getAddress(AddressingMode::IZX);
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
            stepCpu(2);
            break;

        case 0x24: // BIT Zero Page
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZP0));
                miscValue = data;
                setZero((accumulator & data) == 0);
                setOverflow(data & 0x40);
                setNegative(data & 0x80);
            }
            break;

        case 0x25: // AND Zero Page
            accumulator &= memory->read(getAddress(AddressingMode::ZP0));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x26: // ROL Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x27: // Illegal *RLA Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
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
            stepCpu(2);
            break;

        case 0x28: // PLP
            // Nintendulator actually always sets bit 5, not sure which one is correct
            flags = (popByte() & 0xEF) | (flags & 0x10) | 0x20;
            stepCpu(4);
            break;

        case 0x29: // AND Immediate
            accumulator &= getAddress(AddressingMode::IMM);
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
            stepCpu(2);
            break;

        case 0x2C: // BIT Absolute
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABS));
                setZero((accumulator & data) == 0);
                setOverflow(data & 0x40);
                setNegative(data & 0x80);
            }
            break;

        case 0x2D: // AND Absolute
            accumulator &= memory->read(getAddress(AddressingMode::ABS));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x2E: // ROL Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x2F: // Illegal *RLA Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
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
            stepCpu(2);
            break;

        case 0x30: // BMI
            if (getNegative()) {
                int8_t offset = (int8_t)fetch();
                uint16_t newAddress = programCounter + offset;
                branchLocation = newAddress;
                if ((newAddress & 0xFF00) != (programCounter & 0xFF00)) {
                    stepCpu(1);
                }
                programCounter = newAddress;
                stepCpu(1);
            } else {
                branchLocation = programCounter + (int8_t)fetch() + 1;
            }
            stepCpu(2);
            break;

        case 0x31: // AND Indirect, Indexed
            {
                uint16_t address = getAddress(AddressingMode::IZY);
                uint8_t data = memory->read(address);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x32: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x33: // Illegal *RLA (Indirect),Y
            {
                uint8_t base = fetch();
                uint16_t address = ((uint16_t(memory->read((base + 1) & 0xff)) << 8) | memory->read(base)) + yIndex;
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
            stepCpu(8);
            break;

        case 0x34: // Illegal *NOP Zero Page, X
            fetch();
            stepCpu(4);
            break;

        case 0x35: // AND Zero Page, X
            accumulator &= memory->read(getAddress(AddressingMode::ZPX));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x36: // ROL Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x80);
                data <<= 1;
                data |= carry;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x37: // Illegal *RLA Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
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
            stepCpu(2);
            break;  

        case 0x38: // SEC
            setCarry(true);
            stepCpu(2);
            break;

        case 0x39: // AND Absolute, Y
            {
                uint16_t address = getAddress(AddressingMode::ABY);
                accumulator &= memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x3A: // Illegal NOP
            stepCpu(2);
            break;

        case 0x3B: // Illegal *RLA Absolute, Y
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
            stepCpu(7);
            break;

        case 0x3C: // Illegal TOP Absolute, X
            getAddress(AddressingMode::ABX);
            break;

        case 0x3D:  // AND Absolute, X
            {
                uint16_t address = getAddress(AddressingMode::ABX);
                accumulator &= memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
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
            stepCpu(7);
            break;

        case 0x3F: // Illegal *RLA Absolute, X
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
            stepCpu(7);
            break;

        case 0x40: // RTI
            flags = (popByte() & 0xEF) | (flags & 0x10) | 0x20;
            programCounter = popWord();
            stepCpu(6);
            break;

        case 0x41: // EOR Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
                accumulator ^= memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x42: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x43: // Illegal *SRE Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
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
            stepCpu(2);
            break;

        case 0x44: // Illegal *NOP Zero Page
            fetch();
            stepCpu(3);
            break;

        case 0x45: // EOR Zero Page
            accumulator ^= memory->read(getAddress(AddressingMode::ZP0));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x46: // LSR Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x47: // Illegal *SRE Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
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
            stepCpu(2);
            break;

        case 0x48: // PHA
            pushByte(accumulator);
            stepCpu(3);
            break;

        case 0x49: // EOR Immediate
            accumulator ^= getAddress(AddressingMode::IMM);
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x4A: // LSR Accumulator
            setCarry(accumulator & 0x01);
            accumulator >>= 1;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            stepCpu(2);
            break;

        case 0x4B: // Illegal ASR Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
                accumulator &= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
                setCarry(accumulator & 0x01);
                accumulator >>= 1;
            }
            break;

        case 0x4C: // JMP Absolute
            programCounter = fetchWord();
            stepCpu(3);
            break;

        case 0x4D: // EOR Absolute
            accumulator ^= memory->read(getAddress(AddressingMode::ABS));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x4E: // LSR Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x4F: // Illegal *SRE Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
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
            stepCpu(2);
            break;

        case 0x50:
            if (!getOverflow()) {
                int8_t offset = (int8_t)fetch();
                uint16_t newAddress = programCounter + offset;
                branchLocation = newAddress;
                if ((newAddress & 0xFF00) != (programCounter & 0xFF00)) {
                    stepCpu(1);
                }
                programCounter = newAddress;
                stepCpu(1);
            } else {
                branchLocation = programCounter + (int8_t)fetch() + 1;
            }
            stepCpu(2);
            break;

        case 0x51: // EOR Indirect, Indexed
            {
                uint16_t address = getAddress(AddressingMode::IZY);
                uint8_t data = memory->read(address);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x52: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x53: // Illegal *SRE Indirect, Indexed
            {
                uint8_t base = fetch();
                uint16_t deref_base = (uint16_t(memory->read((base + 1) & 0xff)) << 8) | memory->read(base);
                uint8_t data = memory->read(deref_base + yIndex);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(deref_base + yIndex, data);
                accumulator ^= data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            stepCpu(8);
            break;

        case 0x54: // Illegal *NOP Zero Page, X
            fetch();
            stepCpu(4);
            break;

        case 0x55: // EOR Zero Page, X
            accumulator ^= memory->read(getAddress(AddressingMode::ZPX));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0x56: // LSR Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
                uint8_t data = memory->read(address);
                setCarry(data & 0x01);
                data >>= 1;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x57: // Illegal *SRE Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
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
            stepCpu(2);
            break;

        case 0x58: // CLI
            setInterruptDisable(false);
            stepCpu(2);
            break;

        case 0x59: // EOR Absolute, Y
            {
                uint16_t address = getAddress(AddressingMode::ABY);
                accumulator ^= memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x5A: // Illegal NOP
            stepCpu(2);
            break;

        case 0x5B: // Illegal *SRE Absolute, Y
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
            stepCpu(7);
            break;

        case 0x5C: // Illegal TOP Absolute, X
            getAddress(AddressingMode::ABX);
            break;

        case 0x5D: // EOR Absolute, X
            {
                uint16_t address = getAddress(AddressingMode::ABX);
                accumulator ^= memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
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
            stepCpu(7);
            break;

        case 0x5F: // Illegal *SRE Absolute, X
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
            stepCpu(7);
            break;

        case 0x60: // RTS
            programCounter = popWord() + 1;
            stepCpu(6);
            break;

        case 0x61: // ADC Indexed Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            stepCpu(6);
            break;

        case 0x62: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x63: // Illegal *RRA Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
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
            stepCpu(2);
            break;

        case 0x64: // Illegal *NOP Zero Page
            fetch();
            stepCpu(3);
            break;

        case 0x65: // ADC Zero Page
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZP0));
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
                uint16_t address = getAddress(AddressingMode::ZP0);
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x67: // Illegal *RRA Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
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
            stepCpu(2);
            break;

        case 0x68: // PLA
            accumulator = popByte();
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            stepCpu(4);
            break;

        case 0x69: // ADC Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
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
            stepCpu(2);
            break;

        case 0x6B: // Illegal ARR Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
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
                uint16_t address = fetchWord();
                programCounter = memory->read(address) | (memory->read((address & 0xFF00) | ((address + 1) & 0xFF)) << 8);
            }
            stepCpu(5);
            break;

        case 0x6D: // ADC Absolute
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABS));
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
                uint16_t address = getAddress(AddressingMode::ABS);
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x6F: // Illegal *RRA Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
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
            stepCpu(2);
            break;

        case 0x70: // BVS
            if (getOverflow()) {
                int8_t offset = (int8_t)fetch();
                uint16_t newAddress = programCounter + offset;
                branchLocation = newAddress;
                if ((newAddress & 0xFF00) != (programCounter & 0xFF00)) {
                    stepCpu(1);
                }
                programCounter = newAddress;
                stepCpu(1);
            } else {
                branchLocation = programCounter + (int8_t)fetch() + 1;
            }
            stepCpu(2);
            break;

        case 0x71: // ADC Indirect, Indexed
            {
                uint16_t address = getAddress(AddressingMode::IZY);
                uint8_t data = memory->read(address);
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

        case 0x73: // Illegal *RRA Indirect, Indexed
            {
                uint8_t base = fetch();
                uint16_t deref_base = (uint16_t(memory->read((base + 1) & 0xff)) << 8) | memory->read(base);
                uint8_t data = memory->read(deref_base + yIndex);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(deref_base + yIndex, data);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            stepCpu(8);
            break;

        case 0x74: // Illegal *NOP Zero Page, X
            fetch();
            stepCpu(4);
            break;
        
        case 0x75: // ADC Zero Page, X
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZPX));
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0x76: // ROR Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
                uint8_t data = memory->read(address);
                uint8_t carry = getCarry();
                setCarry(data & 0x01);
                data >>= 1;
                data |= carry << 7;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0x77: // Illegal *RRA Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
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
            stepCpu(2);
            break;

        case 0x78: // SEI
            setInterruptDisable(true);
            stepCpu(2);
            break;

        case 0x79: // ADC Absolute, Y
            {
                uint16_t address = getAddress(AddressingMode::ABY);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator + data + getCarry();
                setCarry(result > 0xFF);
                setZero((result & 0xFF) == 0);
                setOverflow((~(accumulator ^ data) & (accumulator ^ result) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            stepCpu(4);
            break;

        case 0x7A: // Illegal NOP
            stepCpu(2);
            break;

        case 0x7B: // Illegal *RRA Absolute, Y
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
            stepCpu(7);
            break;

        case 0x7C: // Illegal TOP Absolute, X
            getAddress(AddressingMode::ABX);
            break;

        case 0x7D: // ADC Absolute, X
            {
                uint16_t address = getAddress(AddressingMode::ABX);
                uint8_t data = memory->read(address);
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
            stepCpu(7);
            break;

        case 0x7F: // Illegal *RRA Absolute, X
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
            stepCpu(7);
            break;

        case 0x80: // Illegal *NOP Immediate
            fetch();
            stepCpu(2);
            break;

        case 0x81: // STA Indexed, Indirect
            memory->write(getAddress(AddressingMode::IZX), accumulator);
            break;

        case 0x82: // Illegal *NOP Immediate
            fetch();
            stepCpu(2);
            break;

        case 0x83: // Illegal *SAX indirect x
            {
                uint16_t address = getAddress(AddressingMode::IZX);
                uint8_t data = accumulator & xIndex;
                memory->write(address, data);
            }
            break;

        case 0x84: // STY Zero Page
            memory->write(getAddress(AddressingMode::ZP0), yIndex);
            break;

        case 0x85: // STA Zero Page
            memory->write(getAddress(AddressingMode::ZP0), accumulator);
            break;

        case 0x86: // STX Zero Page
            memory->write(getAddress(AddressingMode::ZP0), xIndex);
            break;

        case 0x87: // Illegal *SAX Zero Page.
            {
                uint8_t data = accumulator & xIndex;
                memory->write(getAddress(AddressingMode::ZP0), data);
            }
            break;

        case 0x88: // DEY
            yIndex--;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            stepCpu(2);
            break;

        case 0x89: // Illegal *NOP Immediate
            fetch();
            stepCpu(2);
            break;

        case 0x8A: // TXA
            accumulator = xIndex;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            stepCpu(2);
            break;

        case 0x8B: // Illegal XAA Immediate. no idea if this is right. (it isnt this opperation isnt fully understood)
            {
                uint8_t data = getAddress(AddressingMode::IMM);
                accumulator &= xIndex & data;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0x8C: // STY Absolute
            memory->write(getAddress(AddressingMode::ABS), yIndex);
            break;

        case 0x8D: // STA Absolute
            memory->write(getAddress(AddressingMode::ABS), accumulator);
            break;

        case 0x8E: // STX Absolute
            memory->write(getAddress(AddressingMode::ABS), xIndex);
            break;

        case 0x8F: // Illegal *SAX Absolute
            {
                uint8_t data = accumulator & xIndex;
                memory->write(getAddress(AddressingMode::ABS), data);
            }
            break;

        case 0x90: // BCC
            if (!getCarry()) {
                int8_t offset = (int8_t)fetch();
                uint16_t newAddress = programCounter + offset;
                branchLocation = newAddress;
                if ((newAddress & 0xFF00) != (programCounter & 0xFF00)) {
                    stepCpu(1);
                }
                programCounter = newAddress;
                stepCpu(1);
            } else {
                branchLocation = programCounter + (int8_t)fetch() + 1;
            }
            stepCpu(2);
            break;

        case 0x91: // STA Indirect, Indexed
            {
                uint8_t base = fetch();
                uint8_t lo = memory->read(base);
                uint8_t hi = memory->read((base + 1) & 0xFF);
                uint16_t deref_base = ((uint16_t)lo) | ((uint16_t)hi << 8);
                uint16_t deref = deref_base + yIndex;
                memory->write(deref, accumulator);
            }
            stepCpu(6);
            break;

        case 0x92: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0x93: // Illegal SHA Indirect, Indexed
            {
                uint8_t base = fetch();
                uint16_t deref_base = (uint16_t(memory->read((base + 1) & 0xff)) << 8) | memory->read(base);
                uint8_t data = accumulator & xIndex & 0x7;
                memory->write(deref_base + yIndex, data);
            }
            stepCpu(6);
            break;

        case 0x94: // STY Zero Page, X
            memory->write(getAddress(AddressingMode::ZPX), yIndex);
            break;

        case 0x95: // STA Zero Page, X
            memory->write(getAddress(AddressingMode::ZPX), accumulator);
            break;

        case 0x96: // STX Zero Page, Y
            memory->write(getAddress(AddressingMode::ZPY), xIndex);
            break;

        case 0x97: // Illegal *SAX Zero Page, Y
            {
                uint8_t data = accumulator & xIndex;
                memory->write(getAddress(AddressingMode::ZPY), data);
            }
            break;

        case 0x98: // TYA
            accumulator = yIndex;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            stepCpu(2);
            break;

        case 0x99: // STA Absolute, Y
            memory->write(getAddress(AddressingMode::ABY), accumulator);
            stepCpu(5);
            break;

        case 0x9A: // TXS
            stackPointer = xIndex;
            stepCpu(2);
            break;

        case 0x9C: // Illegal SHY Absolute, Y.
            {
                uint16_t address = fetchWord() + yIndex + 1;
                uint8_t data = yIndex & (address >> 8);
                memory->write(address, data);
            }
            break;

        case 0x9B: // Illegal SHS Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                stackPointer = accumulator & xIndex;
                stackPointer &= (address >> 8) + 1;
                memory->write(address, stackPointer);
            }
            stepCpu(5);
            break;

        case 0x9D: // STA Absolute, X
            memory->write(fetchWord() + xIndex, accumulator);
            stepCpu(5);
            break;

        case 0x9E: // Illegal SHX Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex + 1;
                uint8_t data = xIndex & (address >> 8);
                memory->write(address, data);
            }
            stepCpu(5);
            break;

        case 0x9F: // Illegal SHA Absolute, Y. freaky operation :skull:
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = accumulator & xIndex & 0x7;
                memory->write(address, data);
            }
            stepCpu(5);
            break;

        case 0xA0: // LDY Immediate
            yIndex = getAddress(AddressingMode::IMM);
            miscValue = yIndex;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xA1: // LDA Indexed, Indirect
            {   
                uint16_t address = getAddress(AddressingMode::IZX);
                accumulator = memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;
            
            // indirectAddr1 = fetchNoAdvance() + xIndex;
            // indirectAddr2 = memory->readWord(indirectAddr1);
            // std::cout << "Low: " << std::hex << (int) (memory->read(fetchNoAdvance() + xIndex)) << std::endl;
            // std::cout << "High: " << std::hex << (int) (memory->read(fetchNoAdvance() + xIndex + 1)) << std::endl;
            // accumulator = memory->read(memory->readWord(getAddress(AddressingMode::ZPX)));
            // miscValue = accumulator;
            // setZero(accumulator == 0);
            // setNegative(accumulator & 0x80);
            // stepCpu(6);
            // break;

        case 0xA2: // LDX Immediate
            xIndex = getAddress(AddressingMode::IMM);
            miscValue = xIndex;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xA3: // Illegal LAX Indexed, Indirect
            {
                uint8_t data = memory->read(getAddress(AddressingMode::IZX));
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xA4: // LDY Zero Page
            yIndex = memory->read(getAddress(AddressingMode::ZP0));
            miscValue = yIndex;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xA5: // LDA Zero Page
            accumulator = memory->read(getAddress(AddressingMode::ZP0));
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xA6: // LDX Zero Page
            xIndex = memory->read(getAddress(AddressingMode::ZP0));
            miscValue = xIndex;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xA7: // Illegal LAX Zero Page
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZP0));
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
            stepCpu(2);
            break;

        case 0xA9: // LDA Immediate
            accumulator = getAddress(AddressingMode::IMM);
            miscValue = accumulator;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xAA: // TAX
            xIndex = accumulator;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            stepCpu(2);
            break;

        case 0xAB: // Illegal LXA Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
                accumulator &= data;
                xIndex = accumulator;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0xAC: // LDY Absolute
            yIndex = memory->read(getAddress(AddressingMode::ABS));
            miscValue = yIndex;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;
        
        case 0xAD: // LDA Absolute
            accumulator = memory->read(getAddress(AddressingMode::ABS));
            miscValue = accumulator;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xAE: // LDX Absolute
            xIndex = memory->read(getAddress(AddressingMode::ABS));
            miscValue = xIndex;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xAF: // Illegal LAX Absolute
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABS));
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xB0: // BCS
            if (getCarry()) {
                int8_t offset = (int8_t)fetch();
                uint16_t newAddress = programCounter + offset;
                branchLocation = newAddress;
                if ((newAddress & 0xFF00) != (programCounter & 0xFF00)) {
                    stepCpu(1);
                }
                programCounter = newAddress;
                stepCpu(1);
            } else {
                branchLocation = programCounter + (int8_t)fetch() + 1;
            }
            stepCpu(2);
            break;

        case 0xB1: // LDA Indirect, Indexed
            {
                uint16_t address = getAddress(AddressingMode::IZY);
                accumulator = memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0xB2: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0xB3: // Illegal LAX Indirect, Indexed
            {
                uint8_t data = memory->read(getAddress(AddressingMode::IZY));
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xB4: // LDY Zero Page, X
            yIndex = memory->read(getAddress(AddressingMode::ZPX));
            miscValue = yIndex;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            break;

        case 0xB5: // LDA Zero Page, X
            accumulator = memory->read(getAddress(AddressingMode::ZPX));
            miscValue = accumulator;
            setZero(accumulator == 0);
            setNegative(accumulator & 0x80);
            break;

        case 0xB7: // Illegal LAX Zero Page, Y
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZPY));
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xB6: // LDX Zero Page, Y
            xIndex = memory->read(getAddress(AddressingMode::ZPY));
            miscValue = xIndex;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            break;

        case 0xB8: // CLV
            setOverflow(false);
            stepCpu(2);
            break;

        case 0xB9: // LDA Absolute, Y
            {
                uint16_t address = getAddress(AddressingMode::ABY);
                accumulator = memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0xBA: // TSX
            xIndex = stackPointer;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            stepCpu(2);
            break;

        case 0xBB: // Illegal LAS Absolute
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABY));
                accumulator &= data;
                xIndex = accumulator;
                stackPointer = accumulator;
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0xBC: // LDY Absolute, X
            {
                yIndex = memory->read(getAddress(AddressingMode::ABX));
                setZero(yIndex == 0);
                setNegative(yIndex & 0x80);
            }
            stepCpu(4);
            break;

        case 0xBD: // LDA Absolute, X
            {
                uint16_t address = getAddress(AddressingMode::ABX);
                accumulator = memory->read(address);
                setZero(accumulator == 0);
                setNegative(accumulator & 0x80);
            }
            break;

        case 0xBE: // LDX Absolute, Y
            {
                uint16_t address = getAddress(AddressingMode::ABY);
                xIndex = memory->read(address);
                setZero(xIndex == 0);
                setNegative(xIndex & 0x80);
            }
            break;

        case 0xBF: // Illegal LAX Absolute, Y
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABY));
                accumulator = data;
                xIndex = data;
                setZero(data == 0);
                setNegative(data & 0x80);
            }
            break;

        case 0xC0: // CPY Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
                uint16_t result = yIndex - data;
                setCarry(yIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xC1: // CMP Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xC2: // Illegal *NOP Immediate
            fetch();
            stepCpu(2);
            break;

        case 0xC3: // Illegal *DCP indirect x
            {   
                uint16_t address = getAddress(AddressingMode::IZX);
                uint8_t data = memory->read(address);
                data--;
                memory->write(address, data);
                setCarry(accumulator >= data);
                setZero((accumulator - data) == 0);
                setNegative((accumulator - data) & 0x80);
            }
            stepCpu(2);
            break;

        case 0xC4: // CPY Zero Page
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZP0));
                uint16_t result = yIndex - data;
                setCarry(yIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            stepCpu(3);
            break;

        case 0xC5: // CMP Zero Page
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZP0));
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xC6: // DEC Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
                uint8_t data = memory->read(address);
                data--;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0xC7: // Illegal *DCP Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
                uint8_t data = memory->read(address);
                data--;
                memory->write(address, data);
                setCarry(accumulator >= data);
                setZero((accumulator - data) == 0);
                setNegative((accumulator - data) & 0x80);
            }
            stepCpu(2);
            break;

        case 0xC8: // INY
            yIndex++;
            setZero(yIndex == 0);
            setNegative(yIndex & 0x80);
            stepCpu(2);
            break;

        case 0xC9: // CMP Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
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
            stepCpu(2);
            break;

        case 0xCB: // Illegal SBX Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
                xIndex &= accumulator;
                xIndex -= data;
                setZero(xIndex == 0);
                setNegative(xIndex & 0x80);
                setCarry(xIndex >= 0);
            }
            break;
        
        case 0xCC: // CPY Absolute
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABS));
                uint16_t result = yIndex - data;
                setCarry(yIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            stepCpu(4);
            break;

        case 0xCD: // CMP Absolute
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABS));
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            stepCpu(4);
            break;

        case 0xCE: // DEC Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
                uint8_t data = memory->read(address);
                data--;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0xCF: // Illegal *DCP Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
                uint8_t data = memory->read(address);
                data--;
                memory->write(address, data);
                setCarry(accumulator >= data);
                setZero((accumulator - data) == 0);
                setNegative((accumulator - data) & 0x80);
            }
            stepCpu(2);
            break;

        case 0xD0: // BNE
            if (!getZero()) {
                int8_t offset = (int8_t)fetch();
                uint16_t newAddress = programCounter + offset;
                branchLocation = newAddress;
                if ((newAddress & 0xFF00) != (programCounter & 0xFF00)) {
                    stepCpu(1);
                }
                programCounter = newAddress;
                stepCpu(1);
            } else {
                branchLocation = programCounter + (int8_t)fetch() + 1;
            }
            stepCpu(2); 
            break;

        case 0xD1: // CMP Indirect, Indexed
            {
                uint16_t address = getAddress(AddressingMode::IZY);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xD2: // Illegal KIL
            System::instance->stop = true;
            break;

        case 0xD3: // Illegal *DCP Indirect, Indexed
            {
                uint8_t base = fetch();
                uint16_t deref_base = (uint16_t(memory->read((base + 1) & 0xff)) << 8) | memory->read(base);
                uint16_t address = deref_base + yIndex;
                uint8_t data = memory->read(address);
                data--;
                memory->write(address, data);
                setCarry(accumulator >= data);
                setZero((accumulator - data) == 0);
                setNegative((accumulator - data) & 0x80);
            }
            stepCpu(8);
            break;

        case 0xD4: // Illegal *NOP Zero Page, X
            fetch();
            stepCpu(4);
            break;

        case 0xD5: // CMP Zero Page, X
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZPX));
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xD6: // DEC Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
                uint8_t data = memory->read(address);
                data--;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0xD7: // Illegal *DCP Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
                uint8_t data = memory->read(address);
                data--;
                memory->write(address, data);
                setCarry(accumulator >= data);
                setZero((accumulator - data) == 0);
                setNegative((accumulator - data) & 0x80);
            }
            stepCpu(2);
            break;

        case 0xD8: // CLD
            setDecimal(false);
            stepCpu(2);
            break;

        case 0xD9: // CMP Absolute, Y
            {
                uint16_t address = getAddress(AddressingMode::ABY);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator - data;
                setCarry(accumulator >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xDA: // Illegal NOP
            stepCpu(2);
            break;

        case 0xDB: // Illegal *DCP Absolute, Y
            {
                uint16_t address = fetchWord() + yIndex;
                uint8_t data = memory->read(address);
                data--;
                memory->write(address, data);
                setCarry(accumulator >= data);
                setZero((accumulator - data) == 0);
                setNegative((accumulator - data) & 0x80);
            }
            stepCpu(7);
            break;

        case 0xDC: // Illegal TOP Absolute, X
            getAddress(AddressingMode::ABX);
            break;

        case 0xDD: // CMP Absolute, X
            {
                uint16_t address = getAddress(AddressingMode::ABX);
                uint8_t data = memory->read(address);
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
            stepCpu(7);
            break;

        case 0xDF: // Illegal *DCP Absolute, X
            {
                uint16_t address = fetchWord() + xIndex;
                uint8_t data = memory->read(address);
                data--;
                memory->write(address, data);
                setCarry(accumulator >= data);
                setZero((accumulator - data) == 0);
                setNegative((accumulator - data) & 0x80);
            }
            stepCpu(7);
            break;

        case 0xE0: // CPX Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
                uint16_t result = xIndex - data;
                setCarry(xIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xE1: // SBC Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
                uint8_t data = memory->read(address);
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xE2: // Illegal *NOP Immediate
            fetch();
            stepCpu(2);
            break;

        case 0xE3: // Illegal *ISB Indexed, Indirect
            {
                uint16_t address = getAddress(AddressingMode::IZX);
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
            stepCpu(2);
            break;

        case 0xE4: // CPX Zero Page
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZP0));
                uint16_t result = xIndex - data;
                setCarry(xIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xE5: // SBC Zero Page
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZP0));
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
                uint16_t address = getAddress(AddressingMode::ZP0);
                uint8_t data = memory->read(address);
                data++;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0xE7: // Illegal *ISB Zero Page
            {
                uint16_t address = getAddress(AddressingMode::ZP0);
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
            stepCpu(2);
            break;

        case 0xE8: // INX
            xIndex++;
            setZero(xIndex == 0);
            setNegative(xIndex & 0x80);
            stepCpu(2);
            break;

        case 0xE9: // SBC Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xEA: // NOP
            stepCpu(2);
            break;

        case 0xEB: // Illegal SBC Immediate
            {
                uint8_t data = getAddress(AddressingMode::IMM);
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
                uint8_t data = memory->read(getAddress(AddressingMode::ABS));
                uint16_t result = xIndex - data;
                setCarry(xIndex >= data);
                setZero((result & 0xFF) == 0);
                setNegative(result & 0x80);
            }
            break;

        case 0xED: // SBC Absolute
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABS));
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
                uint16_t address = getAddress(AddressingMode::ABS);
                uint8_t data = memory->read(address);
                data++;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0xEF: // Illegal *ISB Absolute
            {
                uint16_t address = getAddress(AddressingMode::ABS);
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
            stepCpu(2);
            break;
            

        case 0xF0: // BEQ
            if (getZero()) {
                int8_t offset = (int8_t)fetch();
                uint16_t newAddress = programCounter + offset;
                branchLocation = newAddress;
                if ((newAddress & 0xFF00) != (programCounter & 0xFF00)) {
                    stepCpu(1);
                }
                programCounter = newAddress;
                stepCpu(1);
            } else {
                branchLocation = programCounter + (int8_t)fetch() + 1;
            }
            stepCpu(2);
            break;

        case 0xF1: // SBC Indirect, Indexed
            {
                uint8_t data = memory->read(getAddress(AddressingMode::IZY));
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

        case 0xF3: // Illegal *ISB Indirect, Indexed
            {
                uint8_t base = fetch();
                uint16_t deref_base = (uint16_t(memory->read((base + 1) & 0xff)) << 8) | memory->read(base);
                uint8_t data = memory->read(deref_base + yIndex);
                data++;
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
                memory->write(deref_base + yIndex, data);
            }
            stepCpu(8);
            break;

        case 0xF4: // Illegal *NOP Zero Page, X
            fetch();
            stepCpu(4);
            break;

        case 0xF5: // SBC Zero Page, X
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ZPX));
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
                uint16_t address = getAddress(AddressingMode::ZPX);
                uint8_t data = memory->read(address);
                data++;
                setZero(data == 0);
                setNegative(data & 0x80);
                memory->write(address, data);
            }
            stepCpu(2);
            break;

        case 0xF7: // Illegal *ISB Zero Page, X
            {
                uint16_t address = getAddress(AddressingMode::ZPX);
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
            stepCpu(2);
            break;

        case 0xF8: // SED
            setDecimal(true);
            stepCpu(2);
            break;

        case 0xF9: // SBC Absolute, Y
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABY));
                uint16_t result = accumulator - data - (1 - getCarry());
                setCarry(result < 0x100);
                setZero((result & 0xFF) == 0);
                setOverflow(((accumulator ^ result) & (accumulator ^ data) & 0x80) != 0);
                setNegative(result & 0x80);
                accumulator = result & 0xFF;
            }
            break;

        case 0xFA: // Illegal NOP
            stepCpu(2);
            break;

        case 0xFB: // Illegal *ISB Absolute, Y
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
            stepCpu(7);
            break;

        case 0xFC: // Illegal TOP Absolute, X
            getAddress(AddressingMode::ABX);
            break;
            
        case 0xFD: // SBC Absolute, X
            {
                uint8_t data = memory->read(getAddress(AddressingMode::ABX));
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
            stepCpu(7);
            break;

        case 0xFF: // Illegal *ISB Absolute, X
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
            stepCpu(7);
            break;
        
        default:
            std::cout << "Opcode not implemented: " << std::hex << (int)opcode << std::endl;
            System::instance->stop = true;
            break;
    }
}
