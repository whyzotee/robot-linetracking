#include "movement.h"

// Array Position { intA, intB, en }
const byte frontLeft[] = {13, 12, 11};
const byte frontRight[] = {10, 9, 8};
const byte backLeft[] = {7, 6, 5};
const byte backRight[] = {4, 3, 2};

String old_direction = "";

void motor_setup()
{
    for (int i = 0; i < 3; i++)
    {
        pinMode(frontLeft[i], OUTPUT);
        pinMode(frontRight[i], OUTPUT);
        pinMode(backLeft[i], OUTPUT);
        pinMode(backRight[i], OUTPUT);
    }
}

void reset()
{
    digitalWrite(frontLeft[0], 0);
    digitalWrite(frontRight[0], 0);
    digitalWrite(backLeft[0], 0);
    digitalWrite(backRight[0], 0);
    digitalWrite(frontLeft[1], 0);
    digitalWrite(frontRight[1], 0);
    digitalWrite(backLeft[1], 0);
    digitalWrite(backRight[1], 0);
}

void move(byte speed, String direction)
{
    // // Reset Digital Value intA and intB
    if (old_direction != direction)
    {
        reset();
        old_direction = direction;
    }

    // Set Enable Pin Speed
    if (direction == "sleft")
    {
        analogWrite(frontLeft[2], speed);
        analogWrite(frontRight[2], speed - 160);
        analogWrite(backLeft[2], speed);
        analogWrite(backRight[2], speed - 160);
    }
    else if (direction == "sright")
    {
        analogWrite(frontLeft[2], speed);
        analogWrite(frontRight[2], speed - 175);
        analogWrite(backLeft[2], speed);
        analogWrite(backRight[2], speed);
    }
    else
    {
        analogWrite(frontLeft[2], speed);
        analogWrite(frontRight[2], speed);
        analogWrite(backLeft[2], speed);
        analogWrite(backRight[2], speed);
    }

    /*
    Direction Position 0 = Break Move, 1 = Move Front,
    2 = Move Left, 3 = Move Right, 4 = Move Back,
    200 = Test Run
    */
    if (direction == "stop")
    {
        // Break Move
        analogWrite(frontLeft[2], 0);
        analogWrite(frontRight[2], 0);
        analogWrite(backLeft[2], 0);
        analogWrite(backRight[2], 0);
    }
    else if (direction == "front")
    {
        // Move Front
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[1], 1);
    }
    else if (direction == "left")
    {
        // Move Left
        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[1], 1);
    }
    else if (direction == "sleft")
    {
        // Move Slide Left
        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[0], 1);
    }
    else if (direction == "right")
    {
        // Move Right
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[0], 1);
    }
    else if (direction == "sright")
    {
        // Move Slide Right
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[1], 1);
    }
    else if (direction == "back")
    {
        // Move Back
        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[0], 1);
    }
    else
    {
        // Test Run
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[1], 1);
        delay(1000);

        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[0], 1);
        delay(1000);
    }
}