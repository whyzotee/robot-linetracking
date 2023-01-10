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

void move(byte speed, char direction[])
{
    // Reset Digital Value IntA IntB
    reset();

    // Set Enable Pin Speed
    analogWrite(frontLeft[2], speed);
    analogWrite(frontRight[2], speed - 5);
    analogWrite(backLeft[2], speed);
    analogWrite(backRight[2], speed - 5);

    switch (direction)
    {
    case "front":
        // Move Front
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[1], 1);
        break;
    case "left":
        // Move Left
        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[1], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[1], 1);
        break;
    case "right":
        // Move Right
        digitalWrite(frontLeft[0], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[0], 1);
        digitalWrite(backRight[0], 1);
        break;
    case "back":
        // Move Back
        digitalWrite(frontLeft[1], 1);
        digitalWrite(frontRight[0], 1);
        digitalWrite(backLeft[1], 1);
        digitalWrite(backRight[0], 1);
        break;
    case "break":
        // Break Move
        analogWrite(frontLeft[2], 0);
        analogWrite(frontRight[2], 0);
        analogWrite(backLeft[2], 0);
        analogWrite(backRight[2], 0);
        break;
    case 'test':
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
        break;
    }

    delay(50);
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