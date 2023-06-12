#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "sensor.h"

class Motor
{
private:
    String old_direction;

    const uint8_t frontLeft[3] = {47, 46, 13};
    const uint8_t frontRight[3] = {43, 42, 11};
    const uint8_t backLeft[3] = {48, 49, 9};
    const uint8_t backRight[3] = {45, 44, 10};

    double Kp, Kd, Ki;
    int motorSpeed, leftSpeed, rightSpeed;
    int baseSpeed, maxSpeed, minSpeed;

    void reset();

public:
    Sensor sensor;
    void begin(int baseSpeed, int maxSpeed, int minSpeed);
    void move(int speed, String direction);
    void balance_move(bool isFront);
    void turn(char direction);
    void slide(char direction);
    void controllPID();
};

#endif
