#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor
{
private:
    byte speed;
    int black_value;
    // Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
    bool sensor0;
    bool sensor1;
    bool sensor2;
    bool sensor3;
    bool sensor4;
    // Sensor Tracking Position -> { a5  a6  a7  a8  a9 }
    bool sensor5;
    bool sensor6;
    bool sensor7;
    bool sensor8;
    bool sensor9;

    void isBlack(short sensor_value, int offset);

public:
    Sensor(byte speed, int black_value);
    void sensor_read();
};

#endif
