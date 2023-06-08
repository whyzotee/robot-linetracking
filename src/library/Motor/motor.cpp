#include "motor.h"

Motor::Motor()
{
    this->old_direction = "";
}

void Motor::initMotor()
{
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
        digitalWrite(this->frontLeft[i], 0);
        digitalWrite(this->frontRight[i], 0);
        digitalWrite(this->backLeft[i], 0);
        digitalWrite(this->backRight[i], 0);
    }
}

void Motor::move(byte speed, String direction)
{
    // Reset Digital Value intA and intB
    if (this->old_direction != direction)
    {
        reset();
        this->old_direction = direction;
    }

    // Set Enable Pin Speed
    if (direction == "sleft")
    {
        analogWrite(this->frontLeft[2], speed);
        analogWrite(this->frontRight[2], speed - 160);
        analogWrite(this->backLeft[2], speed);
        analogWrite(this->backRight[2], speed - 160);
    }
    else if (direction == "sright")
    {
        analogWrite(this->frontLeft[2], speed);
        analogWrite(this->frontRight[2], speed - 175);
        analogWrite(this->backLeft[2], speed);
        analogWrite(this->backRight[2], speed);
    }
    else
    {
        analogWrite(this->frontLeft[2], speed);
        analogWrite(this->frontRight[2], speed);
        analogWrite(this->backLeft[2], speed);
        analogWrite(this->backRight[2], speed);
    }

    /*
      Direction Position 0 = Break Move, 1 = Move Front,
      2 = Move Left, 3 = Move Right, 4 = Move Back,
      200 = Test Run
    */
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
    else
    {
        // Test Run
        digitalWrite(this->frontLeft[0], 1);
        digitalWrite(this->frontRight[1], 1);
        digitalWrite(this->backLeft[0], 1);
        digitalWrite(this->backRight[1], 1);
        delay(1000);

        digitalWrite(this->frontLeft[1], 1);
        digitalWrite(this->frontRight[0], 1);
        digitalWrite(this->backLeft[1], 1);
        digitalWrite(this->backRight[0], 1);
        delay(1000);
    }
}
