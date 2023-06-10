#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor
{
private:
    int black_value;

    bool isBlack(short sensor_value, int offset);

public:
    int8_t error, pre_error, sum_error;
    // Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
    bool s0, s1, s2, s3, s4, s5;
    // Sensor Tracking Position -> { a5  a6  a7  a8  a9 }
    bool s6, s7, s8, s9, s10;

    void begin();
    void read();
    void findError();
    void log();
    bool isCenter();
    bool isLeftCross();
    bool isRightCross();
};

#endif
