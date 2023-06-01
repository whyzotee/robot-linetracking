#include "movement.h"

// Array Position { intA, intB, en }
const byte frontLeft[] = {13, 12, 11};
const byte frontRight[] = {10, 9, 8};
const byte backLeft[] = {7, 6, 5};
const byte backRight[] = {4, 3, 2};

void move_setup()
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

void move(byte speed, byte direction)
{
    // Reset Digital Value intA and intB
    reset();

    // Set Enable Pin Speed
    analogWrite(frontLeft[2], speed);
    analogWrite(frontRight[2], speed - 5);
    analogWrite(backLeft[2], speed);
    analogWrite(backRight[2], speed - 5);

    /*
    Direction Position 0 = Break Move, 1 = Move Front,
    2 = Move Left, 3 = Move Right, 4 = Move Back,
    200 = Test Run
    */
    if (direction == 0)
    {
        // Break Move
        analogWrite(frontLeft[2], 0);
        analogWrite(frontRight[2], 0);
        analogWrite(backLeft[2], 0);
        analogWrite(backRight[2], 0);
    }
    else if (direction == 1)
    {
        // Move Front
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[1], 1);
    }
    else if (direction == 2)
    {
        // Move Left
        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[0], 1);
    }
    else if (direction == 3)
    {
        // Move Right

        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[1], 1);
    }
    else if (direction == 4)
    {
        // Move Front
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[1], 1);
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

    delay(50);
}