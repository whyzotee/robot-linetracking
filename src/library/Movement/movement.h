#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <Arduino.h>
#include "sensor.h"
#include "motor.h"

class Movement
{
private:
    Motor motor;
    String direction;

public:
    Movement();
    Sensor sensor;
    void balance_move(bool isFront);
};

#endif