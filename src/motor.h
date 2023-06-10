#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "sensor.h"

class Motor
{
private:
    Sensor sensor;
    String old_direction;

    const uint8_t frontLeft[3] = {13, 12, 11};
    const uint8_t frontRight[3] = {10, 9, 8};
    const uint8_t backLeft[3] = {7, 6, 5};
    const uint8_t backRight[3] = {4, 3, 2};

    double Kp, Kd, Ki;
    int motorSpeed, leftSpeed, rightSpeed;
    int baseSpeed, maxSpeed, minSpeed;

    void reset();

public:
    void begin(int baseSpeed, int maxSpeed, int minSpeed);
    void move(int speed, String direction);
    void balance_move(bool isFront);
    void turn(char direction);
    void slide(char direction);
    void controllPID();
};

#endif
