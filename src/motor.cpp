#include "motor.h"

void Motor::begin()
{
    this->old_direction = "";

    for (byte i = 0; i < 3; i++)
    {
        pinMode(this->frontLeft[i], OUTPUT);
        pinMode(this->frontRight[i], OUTPUT);
        pinMode(this->backLeft[i], OUTPUT);
        pinMode(this->backRight[i], OUTPUT);
    }
}

void Motor::reset()
{
    for (byte i = 0; i < 3; i++)
    {
        digitalWrite(this->frontLeft[i], LOW);
        digitalWrite(this->frontRight[i], LOW);
        digitalWrite(this->backLeft[i], LOW);
        digitalWrite(this->backRight[i], LOW);
    }
}

void Motor::move(int speed, String direction)
{
    // Reset Digital Value intA and intB
    if (this->old_direction != direction)
    {
        this->reset();
        this->old_direction = direction;
    }

    // Set Enable Pin Speed
    analogWrite(this->frontLeft[2], speed);
    analogWrite(this->frontRight[2], speed);
    analogWrite(this->backLeft[2], speed);
    analogWrite(this->backRight[2], speed);

    if (direction == "stop")
    {
        // Break Move
        analogWrite(this->frontLeft[2], 0);
        analogWrite(this->frontRight[2], 0);
        analogWrite(this->backLeft[2], 0);
        analogWrite(this->backRight[2], 0);
    }
    else if (direction == "front")
    {
        // Move Front
        digitalWrite(this->frontLeft[0], 1);
        digitalWrite(this->frontRight[1], 1);
        digitalWrite(this->backLeft[0], 1);
        digitalWrite(this->backRight[1], 1);
    }
    else if (direction == "left")
    {
        // Move Left
        digitalWrite(this->frontLeft[1], 1);
        digitalWrite(this->frontRight[1], 1);
        digitalWrite(this->backLeft[1], 1);
        digitalWrite(this->backRight[1], 1);
    }
    else if (direction == "sleft")
    {
        // Move Slide Left
        digitalWrite(this->frontLeft[1], 1);
        digitalWrite(this->frontRight[1], 1);
        digitalWrite(this->backLeft[0], 1);
        digitalWrite(this->backRight[0], 1);
    }
    else if (direction == "right")
    {
        // Move Right
        digitalWrite(this->frontLeft[0], 1);
        digitalWrite(this->frontRight[0], 1);
        digitalWrite(this->backLeft[0], 1);
        digitalWrite(this->backRight[0], 1);
    }
    else if (direction == "sright")
    {
        // Move Slide Right
        digitalWrite(this->frontLeft[0], 1);
        digitalWrite(this->frontRight[0], 1);
        digitalWrite(this->backLeft[1], 1);
        digitalWrite(this->backRight[1], 1);
    }
    else if (direction == "back")
    {
        // Move Back
        digitalWrite(this->frontLeft[1], 1);
        digitalWrite(this->frontRight[0], 1);
        digitalWrite(this->backLeft[1], 1);
        digitalWrite(this->backRight[0], 1);
    }
}
