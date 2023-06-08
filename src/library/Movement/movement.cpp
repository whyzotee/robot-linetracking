#include "movement.h"

void Movement::Movement();

void Movement::balance_move(byte speed, bool isFront)
{
    if (isFront)
    {
        if (this->sensor2)
            this->motor.move(speed, "front");
        else if (this->sensor0 || this->sensor1)
            this->motor.move(speed, "left");
        else if (this->sensor3 || this->sensor4)
            this->motor.move(speed, "right");
        else
            this->motor.move(0, "stop");
    }
    else
    {
        if (this->sensor2)
            this->motor.move(speed, "back");
        else if (this->sensor1 || this->sensor4)
            this->motor.move(speed, "back");
        else if (this->sensor0 || this->sensor3)
            this->motor.move(speed, "back");
        else
            this->motor.move(0, "stop");
    }
}