#include <APU.hpp>

APU::APU() : status(0) {}

APU::~APU() {}

void APU::writeStatus(uint8_t value) {
    status = value;
}

uint8_t APU::readStatus() {
    return status;
}