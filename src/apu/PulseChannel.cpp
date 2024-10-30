#include <apu/PulseChannel.hpp>

PulseChannel::PulseChannel() {
    // Initialize all variables to 0
    dutyCycle = 0;
    lengthTimerHalt = false;
    constantVolume = false;
    volume = 0;
    currentTimer = 0;
    timer = 0;
    lengthCounter = 0;
    divider = 0;
    sweepEnabled = false;
    sweepPeriod = 0;
    sweepNegate = false;
    sweepShift = 0;
}

PulseChannel::~PulseChannel() {
    // Nothing to do here
}

void PulseChannel::init() {
    currentTimer = 0;
    timer = 0;
}

void PulseChannel::writeDuty(uint8_t duty) {
    dutyCycle = (duty & 0xC0) >> 6;
    lengthTimerHalt = duty & 0x20;
    constantVolume = duty & 0x10;
    volume = duty & 0x07;
}

void PulseChannel::writeSweep(uint8_t sweep) {
    sweepEnabled = sweep & 0x80;
    sweepPeriod = (sweep & 0x70) >> 4;
    sweepNegate = sweep & 0x08;
    sweepShift = sweep & 0x07;
}

void PulseChannel::writeTimerLow(uint8_t timerLow) {
    timer |= timerLow;
}

void PulseChannel::writeLength(uint8_t length) {
    timer = (timer & 0xff) | ((length & 0x7) << 8);
    lengthCounter = length >> 3;
    currentTimer = timer;
}

const uint8_t PulseChannel::dutyTable[4][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 1, 1 },
    { 0, 0, 0, 0, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 0, 0 },
};