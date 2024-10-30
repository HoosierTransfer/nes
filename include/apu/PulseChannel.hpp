#pragma once

#include <cstdint>

class PulseChannel {
    PulseChannel();
    ~PulseChannel();

    void init();
    void writeDuty(uint8_t duty);
    void writeSweep(uint8_t sweep); 
    void writeTimerLow(uint8_t timerLow);
    void writeLength(uint8_t length);

    uint8_t dutyCycle;
    bool lengthTimerHalt;
    bool constantVolume;
    uint8_t volume;

    uint16_t currentTimer;
    uint16_t timer;
    uint8_t lengthCounter;
    uint8_t divider;

    bool sweepEnabled;
    uint8_t sweepPeriod;
    bool sweepNegate;
    uint8_t sweepShift;

    static const uint8_t dutyTable[4][8];
};