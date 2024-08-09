#include <iostream>
#include <PPU.hpp>

#define GREEN "\x1b[32m"
#define RED "\x1b[31m"
#define RESET "\x1b[0m"

void runPPUTests() {
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
            std::cout << GREEN << "PPU vram status latch test passed" << RESET << std::endl;
        } else {
            std::cout << RED << "PPU vram status latch test failed" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "PPU vram status latch test failed" << RESET << std::endl;
    }

    ppu = new PPU();

    ppu->writeToControlRegister(0x00);
    ppu->vram[0x0305] = 0x66;

    ppu->writeToAddrRegister(0x63); //0x6305 -> 0x2305
    ppu->writeToAddrRegister(0x05);

    ppu->readFromDataRegister(); // Load into buffer
    if (ppu->readFromDataRegister() == 0x66) {
        std::cout << GREEN << "PPU VRAM mirroring test passed" << RESET << std::endl;
    } else {
        std::cout << RED << "PPU VRAM mirroring test failed" << RESET << std::endl;
    }

    ppu = new PPU();
    
    ppu->statusRegister->set_vblank_status(true);

    uint8_t status = ppu->readFromStatusRegister();

    if (status == 0x80) {
        if (ppu->statusRegister->snapshot() != 0x80) {
            std::cout << GREEN << "PPU status register test passed" << RESET << std::endl;
        } else {
            std::cout << RED << "PPU status register test failed" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "PPU status register test failed" << RESET << std::endl;
    }

    ppu = new PPU();

    ppu->writeToOamAddress(0x10);
    ppu->writeToOamData(0x66);
    ppu->writeToOamData(0x77);

    ppu->writeToOamAddress(0x10);
    if (ppu->readFromOamData() == 0x66) {
        ppu->writeToOamAddress(0x11);
        if (ppu->readFromOamData() == 0x77) {
            std::cout << GREEN << "PPU OAM read/write test passed" << RESET << std::endl;
        } else {
            std::cout << RED << "PPU OAM read/write test failed" << RESET << std::endl;
        }
    } else {
        std::cout << RED << "PPU OAM read/write test failed" << RESET << std::endl;
    }

    delete ppu;
}

// int main() {
//     runPPUTests();
//     return 0;
// }
