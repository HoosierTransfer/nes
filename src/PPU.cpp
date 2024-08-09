#include <PPU.hpp>

#include <iostream>

PPU::PPU() {
    vram = new uint8_t[0x4000];
    oam = new uint8_t[0x100];
    palette = new uint8_t[0x20];
    chrRom = std::vector<uint8_t>(0x2000);
    oamAddr = 0;
    mirroring = Mirroring::HORIZONTAL;

    controlRegister = new ControlRegister();
    maskRegister = new MaskRegister();
    statusRegister = new StatusRegister();
    scrollRegister = new ScrollRegister();
    addrRegister = new AddrRegister();

    dataBuffer = 0;
}

PPU::~PPU() {
    delete[] vram;
    delete[] oam;
    delete[] palette;
    delete controlRegister;
    delete maskRegister;
    delete statusRegister;
    delete scrollRegister;
    delete addrRegister;
}

void PPU::setChrRom(uint8_t* chrRom, size_t size) {
    if (this->chrRom.size() != size) {
        this->chrRom.resize(size);
    }
    
    for (size_t i = 0; i < size; i++) {
        this->chrRom[i] = chrRom[i];
    }
}

void PPU::setMirroring(Mirroring mirroring) {
    this->mirroring = mirroring;
}

uint16_t PPU::mirrorVramAddress(uint16_t address) {
    uint16_t mirroredVram = address & 0x2FFFF;
    mirroredVram -= 0x2000;
    uint16_t nametable = mirroredVram / 0x400;
    switch (mirroring) {
        case Mirroring::HORIZONTAL:
            if (nametable == 1 || nametable == 2) {
                mirroredVram -= 0x400;
            } else if (nametable == 3) {
                mirroredVram -= 0x800;
            }
            break;
        case Mirroring::VERTICAL:
            if (nametable == 2 || nametable == 3) {
                mirroredVram -= 0x800;
            }
            break;
        case Mirroring::FOUR_SCREEN:
            break;
    }

    return mirroredVram;
}

void PPU::incrementVramAddress() {
    this->addrRegister->increment(this->controlRegister->vram_addr_increment());
}

void PPU::writeToControlRegister(uint8_t data) {
    bool beforeNmi = this->controlRegister->generate_vblank_nmi();
    this->controlRegister->update(data);
}

void PPU::writeToMaskRegister(uint8_t data) {
    this->maskRegister->update(data);
}

void PPU::writeToOamAddress(uint8_t data) {
    this->oamAddr = data;
}

void PPU::writeToOamData(uint8_t data) {
    this->oam[this->oamAddr] = data;
    this->oamAddr = (this->oamAddr + 1) & 0xFF;
}

void PPU::writeToScrollRegister(uint8_t data) {
    this->scrollRegister->write(data);
}

void PPU::writeToAddrRegister(uint8_t data) {
    this->addrRegister->update(data);
}

void PPU::writeToDataRegister(uint8_t data) {
    uint16_t address = this->addrRegister->get();
    if (address < 0x2000) {
        std::cerr << "Attempted to write to CHR ROM" << std::endl;
    } else if (address < 0x3000) {
        this->vram[this->mirrorVramAddress(address)] = data;
    } else if (address < 0x3F00) {
        std::cerr << "This address should not be written to" << std::endl;
    } else if (address == 0x3F10 || address == 0x3F14 || address == 0x3F18 || address == 0x3F1C) {
        uint16_t mirroredAddress = address - 0x10;
        this->palette[mirroredAddress - 0x3F00] = data;
    } else if (address >= 0x3F00 && address < 0x4000) {
        this->palette[address - 0x3F00] = data;
    } else {
        std::cerr << "Invalid PPU write address: " << std::hex << address << std::endl;
    }
    incrementVramAddress();
}

void PPU::writeToOamDma(uint8_t* data) {
    for (int i = 0; i < 256; i++) {
        this->oam[this->oamAddr] = data[i];
        this->oamAddr = (this->oamAddr + 1) & 0xFF;
    }
}

uint8_t PPU::readFromStatusRegister() {
    uint8_t status = this->statusRegister->snapshot();
    this->statusRegister->reset_vblank_status();
    this->addrRegister->reset_latch();
    this->scrollRegister->reset_latch();
    return status;
}

uint8_t PPU::readFromOamData() {
    return this->oam[this->oamAddr];
}

uint8_t PPU::readFromDataRegister() {
    uint16_t address = this->addrRegister->get();
    incrementVramAddress();

    if (address < 0x2000) {
        uint8_t data = this->dataBuffer;
        this->dataBuffer = this->chrRom[address];
        return data;
    } else if (address < 0x3000) {
        uint8_t data = this->dataBuffer;
        this->dataBuffer = this->vram[this->mirrorVramAddress(address)];
        return data;
    } else if (address < 0x3F00) {
        std::cerr << "This address should not be read from" << std::endl;
        return 0;
    } else if (address == 0x3F10 || address == 0x3F14 || address == 0x3F18 || address == 0x3F1C) {
        uint16_t mirroredAddress = address - 0x10;
        return this->palette[mirroredAddress - 0x3F00];
    } else if (address >= 0x3F00 && address < 0x4000) {
        return this->palette[address - 0x3F00];
    } else {
        std::cerr << "Invalid PPU read address: " << std::hex << address << std::endl;
        return 0;
    }
}


void PPU::tick(int cycles) {
    // Do nothing for now
}
