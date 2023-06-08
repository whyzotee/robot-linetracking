#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class Motor
{
private:
    const byte frontLeft[3] = {13, 12, 11};
    const byte frontRight[3] = {10, 9, 8};
    const byte backLeft[3] = {7, 6, 5};
    const byte backRight[3] = {4, 3, 2};

    String old_direction;

    void reset();

public:
    Motor();

    void initMotor();
    void move(byte speed, String direction);
};

#endif
