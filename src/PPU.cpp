#include <PPU.hpp>
#include <stdexcept>

#include <iostream>

PPU::PPU() {
    this->chrRom = std::vector<uint8_t>();
    this->mirroring = Mirroring::HORIZONTAL;

    this->paletteTable = new uint8_t[32];
    this->vram = new uint8_t[2048];
    this->oam = new uint8_t[256];
    this->addressRegister = new AddressRegister();
    this->controlRegister = new ControlRegister();
    this->maskRegister = new MaskRegister();
    this->scrollRegister = new ScrollRegister();
    this->statusRegister = new StatusRegister();
    this->dataBuffer = 0;

    this->scanline = 0;
    this->oamAddress = 0;

    this->nmiInterupt = false;

    // zero the data
    for (int i = 0; i < 32; i++) {
        paletteTable[i] = 0;
    }

    for (int i = 0; i < 2048; i++) {
        vram[i] = 0;
    }

    for (int i = 0; i < 256; i++) {
        oam[i] = 0;
    }

    this->cycles = 0;
}

void PPU::writeToAddressRegister(uint8_t data) {
    addressRegister->write(data);
}

void PPU::writeToControlRegister(uint8_t data) {
    bool beforeNMIStatus = controlRegister->generateNMI();
    controlRegister->write(data);
    if (!beforeNMIStatus && controlRegister->generateNMI() && statusRegister->isVBlank()) {
        nmiInterupt = true;
    }
}   

void PPU::writeToMaskRegister(uint8_t data) {
    maskRegister->write(data);
}

void PPU::writeToScrollRegister(uint8_t data) {
    scrollRegister->write(data);
}

void PPU::incrementVramAddress() {
    addressRegister->increment(controlRegister->vramIncrement());
}

uint8_t PPU::read() {
    uint16_t address = addressRegister->get();
    incrementVramAddress();
    
    if (address == 0x3f10 || address == 0x3f14 || address == 0x3f18 || address == 0x3f1c) {
        address -= 0x10;
        return paletteTable[address - 0x3F00];
    }

    if (address < 0x2000) {
        uint8_t result = dataBuffer;
        dataBuffer = chrRom[address];
        return result;
    } else if (address < 0x3000) {
        uint8_t result = dataBuffer;
        dataBuffer = vram[mirrorVramAddress(address)];
        return result;
    } else if (address < 0x3F00) {
        throw std::runtime_error("Invalid PPU read address");
    } else if (address < 0x4000) {
        uint8_t result = dataBuffer;
        dataBuffer = paletteTable[address - 0x3F00];
        return result;
    } else {
        throw std::runtime_error("Invalid PPU read address");
    }
}

void PPU::write(uint8_t data) {
    uint16_t address = addressRegister->get();

    if (address < 0x2000) {
        std::cerr << "Writing to CHR ROM" << std::endl;
    } else if (address < 0x3000) {
        vram[mirrorVramAddress(address)] = data;
    } else if (address < 0x3F00) {
        throw std::runtime_error("Invalid PPU write address");
    } else if (address == 0x3F10 || address == 0x3F14 || address == 0x3F18 || address == 0x3F1C) {
        address -= 0x10;
        paletteTable[address - 0x3F00] = data;
    } else if (address < 0x4000) {
        paletteTable[address - 0x3F00] = data;
    } else {
        throw std::runtime_error("Invalid PPU write address");
    }
    incrementVramAddress();
}

uint16_t PPU::mirrorVramAddress(uint16_t addr) {
    uint16_t mirrored_vram = addr & 0b10111111111111;
    uint16_t vram_index = mirrored_vram - 0x2000;
    uint16_t name_table = vram_index / 0x400;

    switch (mirroring) {
        case Mirroring::VERTICAL:
            if (name_table == 2 || name_table == 3) {
                return vram_index - 0x800;
            }
            break;
        case Mirroring::HORIZONTAL:
            if (name_table == 2 || name_table == 1) {
                return vram_index - 0x400;
            }
            else if (name_table == 3) {
                return vram_index - 0x800;
            }
            break;
    }
    return vram_index;
}

void PPU::writeOAMDMA(uint8_t* data) {
    for (int i = 0; i < 256; i++) {
        oam[oamAddress] = data[i];
        oamAddress = (oamAddress + 1) & 0xFF;
    }
}

uint8_t PPU::readStatusRegister() {
    uint8_t result = statusRegister->read();
    statusRegister->resetVBlank();
    addressRegister->resetLatch();
    scrollRegister->resetLatch();
    return result;
}

uint8_t PPU::readOAMData() {
    return oam[oamAddress];
}

void PPU::writeOAMData(uint8_t data) {
    oam[oamAddress] = data;
    oamAddress = (oamAddress + 1) & 0xFF;
}

void PPU::writeToOAMAddress(uint8_t data) {
    oamAddress = data;
}

void PPU::setChrRom(uint8_t* data, int size) {
    chrRom = std::vector<uint8_t>(data, data + size);
}

void PPU::setMirror(Mirroring mirror) {
    mirroring = mirror;
}

bool PPU::tick(uint8_t cycles) {
    this->cycles += cycles;
    if (this->cycles >= 341) {
        this->cycles -= 341;
        this->scanline++;
        if (this->scanline == 241) {
            statusRegister->setVBlank(true);
            statusRegister->setSpriteZeroHit(false);
            if (controlRegister->generateNMI()) {
                statusRegister->setVBlank(true);
                this->nmiInterupt = true;
            }
        }

        if (this->scanline == 262) {
            this->scanline = 0;
            this->nmiInterupt = false;
            statusRegister->setSpriteZeroHit(false);
            statusRegister->resetVBlank();
            return true;
        }
    }
    return false;
}

bool PPU::isNmiInterupt() {
    return this->nmiInterupt;
}
