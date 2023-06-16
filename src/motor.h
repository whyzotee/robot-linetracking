#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "sensor.h"

class Motor
{
private:
    String old_direction;
    void reset();

public:
    const uint8_t frontLeft[3] = {11, 13, 17};
    const uint8_t frontRight[3] = {5, 6, 16};
    const uint8_t backLeft[3] = {10, 9, 15};
    const uint8_t backRight[3] = {2, 4, 14};

    void begin();
    void move(int speed, String direction);
};

#endif
