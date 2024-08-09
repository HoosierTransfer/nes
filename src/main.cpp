#include <iostream>
#include <chrono>

#include <PPU.hpp>
#include <System.hpp>

#include <SDL2/SDL.h>

#include <vector>

#define GREEN "\x1b[32m"
#define RED "\x1b[31m"
#define RESET "\x1b[0m"

int main() {
    PPU* ppu = new PPU();
    ppu->writeToAddrRegister(0x23);
    ppu->writeToAddrRegister(0x05);
    ppu->writeToDataRegister(0x66);

    if (ppu->vram[0x0305] == 0x66) {
        std::cout << GREEN << "PPU vram write test passed" << RESET << std::endl;
    } else {
        std::cout << RED << "PPU vram write test failed" << RESET << std::endl;
    }

    ppu = new PPU();
    ppu->writeToControlRegister(0x00);
    ppu->vram[0x0305] = 0x66;

    ppu->writeToAddrRegister(0x23);
    ppu->writeToAddrRegister(0x05);

    ppu->readFromDataRegister();
    if (ppu->addrRegister->get() == 0x2306 && ppu->readFromDataRegister() == 0x66) {
        std::cout << GREEN << "PPU vram read test passed" << RESET << std::endl;
    } else {
        std::cout << RED << "PPU vram read test failed" << RESET << std::endl;
    }

    ppu = new PPU();

    ppu->writeToControlRegister(0x00);
    ppu->vram[0x01ff] = 0x66;
    ppu->vram[0x0200] = 0x77;

    ppu->writeToAddrRegister(0x21);
    ppu->writeToAddrRegister(0xff);

    ppu->readFromDataRegister();
    if (ppu->readFromDataRegister() == 0x66) {
        if (ppu->readFromDataRegister() == 0x77) {
            std::cout << GREEN << "PPU vram page cross test passed" << RESET << std::endl;
        } else {
            std::cout << RED << "PPU vram page cross test failed" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "PPU vram page cross test failed" << RESET << std::endl;
    }
    
    ppu = new PPU();

    ppu->writeToControlRegister(0x04);
    ppu->vram[0x01ff] = 0x66;
    ppu->vram[0x01ff + 32] = 0x77;
    ppu->vram[0x01ff + 64] = 0x88;

    ppu->writeToAddrRegister(0x21);
    ppu->writeToAddrRegister(0xff);

    ppu->readFromDataRegister();

    if (ppu->readFromDataRegister() == 0x66) {
        if (ppu->readFromDataRegister() == 0x77) {
            if (ppu->readFromDataRegister() == 0x88) {
                std::cout << GREEN << "PPU vram step 32 test passed" << RESET << std::endl;
            } else {
                std::cout << RED << "PPU vram step 32 test failed" << RESET << std::endl;
            }
        } else {
            std::cout << RED << "PPU vram step 32 test failed" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "PPU vram step 32 test failed" << RESET << std::endl;
    }

    ppu = new PPU();

    ppu->writeToAddrRegister(0x24);
    ppu->writeToAddrRegister(0x05);
    ppu->writeToDataRegister(0x66);

    ppu->writeToAddrRegister(0x28);
    ppu->writeToAddrRegister(0x05);
    ppu->writeToDataRegister(0x77);

    ppu->writeToAddrRegister(0x20);
    ppu->writeToAddrRegister(0x05);

    ppu->readFromDataRegister();

    if (ppu->readFromDataRegister() == 0x66) {
        ppu->writeToAddrRegister(0x2C);
        ppu->writeToAddrRegister(0x05);
        ppu->readFromDataRegister();
        if (ppu->readFromDataRegister() == 0x77) {
            std::cout << GREEN << "PPU vram horizontal mirror test passed" << RESET << std::endl;
        } else {
            std::cout << RED << "PPU vram horizontal mirror test failed" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "PPU vram horizontal mirror test failed" << RESET << std::endl;
    }

    ppu = new PPU();
    ppu->setMirroring(Mirroring::VERTICAL);

    ppu->writeToAddrRegister(0x20);
    ppu->writeToAddrRegister(0x05);
    ppu->writeToDataRegister(0x66);

    ppu->writeToAddrRegister(0x2C);
    ppu->writeToAddrRegister(0x05);
    ppu->writeToDataRegister(0x77);

    ppu->writeToAddrRegister(0x28);
    ppu->writeToAddrRegister(0x05);

    ppu->readFromDataRegister();

    if (ppu->readFromDataRegister() == 0x66) {
        ppu->writeToAddrRegister(0x24);
        ppu->writeToAddrRegister(0x05);
        ppu->readFromDataRegister();
        if (ppu->readFromDataRegister() == 0x77) {
            std::cout << GREEN << "PPU vram vertical mirror test passed" << RESET << std::endl;
        } else {
            std::cout << RED << "PPU vram vertical mirror test failed" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "PPU vram vertical mirror test failed" << RESET << std::endl;
    }

    ppu = new PPU();
    ppu->vram[0x0305] = 0x66;
    
    ppu->writeToAddrRegister(0x21);
    ppu->writeToAddrRegister(0x23);
    ppu->writeToAddrRegister(0x05);

    ppu->readFromDataRegister();

    if (ppu->readFromDataRegister() != 0x66) {
        ppu->readFromStatusRegister();
        ppu->writeToAddrRegister(0x23);
        ppu->writeToAddrRegister(0x05);

        ppu->readFromDataRegister();
        if (ppu->readFromDataRegister() == 0x66) {
            std::cout << GREEN << "PPU vram address increment test passed" << RESET << std::endl;
        } else {
            std::cout << RED << "PPU vram address increment test failed" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "PPU vram address increment test failed" << RESET << std::endl;
    }



    // System* system = new System("pacman.nes");
    // system->run();

    return 0;
}
