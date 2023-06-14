#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor
{
private:
    uint16_t black_value;

    bool isBlack(uint16_t sensor_value, int offset);

public:
    // Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
    bool s0, s1, s2, s3, s4;
    // Sensor Tracking Position -> { a5  a6  a7  a8  a9 }
    bool s5, s6, s7, s8, s9;

    void begin();
    void read();
    void log();
    bool isCenter();
    bool isLeftCross();
    bool isRightCross();
    bool isSomeBlack();
};

#endif
