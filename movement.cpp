#include "movement.h"

// elapsedMillis PrintValue;

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

void move(bool runTest, byte speed[], int direction)
{
    reset();

    // run Test Wheel
    while (runTest)
    {
        analogWrite(frontLeft[2], speed[0]);
        analogWrite(frontRight[2], speed[1]);
        analogWrite(backLeft[2], speed[2]);
        analogWrite(backRight[2], speed[3]);

        digitalWrite(frontLeft[1], 0);
        digitalWrite(frontRight[0], 0);
        digitalWrite(backLeft[1], 0);
        digitalWrite(backRight[0], 0);

        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[1], 1);
        delay(1000);

        digitalWrite(frontLeft[0], 0);
        digitalWrite(frontRight[1], 0);
        digitalWrite(backLeft[0], 0);
        digitalWrite(backRight[1], 0);

        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[0], 1);
        delay(1000);
    }

    analogWrite(frontLeft[2], speed[0]);
    analogWrite(frontRight[2], speed[1]);
    analogWrite(backLeft[2], speed[2]);
    analogWrite(backRight[2], speed[3]);

    switch (direction)
    {
    case 1:
        // moveFront
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[1], 1);
        break;
    case 2:
        // moveLeft
        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[1], 1);
        break;
    case 3:
        // moveRight
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[0], 1);
        break;
    case 4:
        // moveBack
        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[0], 1);
        break;
    case 101:
        analogWrite(frontLeft[2], 0);
        analogWrite(frontRight[2], 0);
        analogWrite(backLeft[2], 0);
        analogWrite(backRight[2], 0);
        break;
    default:
        break;
    }

    delay(100);
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