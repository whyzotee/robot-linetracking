#include "sensor.h"

void Sensor::begin()
{
    this->black_value = 300;
}

void Sensor::read()
{
    this->s0 = isBlack(analogRead(64), 0);
    this->s1 = isBlack(analogRead(55), 0);
    this->s2 = isBlack(analogRead(56), 0);
    this->s3 = isBlack(analogRead(57), 0);
    this->s4 = isBlack(analogRead(58), 450);
    this->s5 = isBlack(analogRead(63), 0);
    this->s6 = isBlack(analogRead(59), 0);
    this->s7 = isBlack(analogRead(61), 0);
    this->s8 = isBlack(analogRead(62), 0);
    this->s9 = isBlack(analogRead(60), 0);
}

void Sensor::findError()
{
    if (!this->s0 && !this->s1 && !this->s2 && !this->s3 && this->s4)
        error = 4;
    else if (!this->s0 && !this->s1 && !this->s2 && this->s3 && this->s4)
        error = 3;
    else if (!this->s0 && !this->s1 && !this->s2 && this->s3 && !this->s4)
        error = 2;
    else if (!this->s0 && !this->s1 && this->s2 && this->s3 && !this->s4)
        error = 1;
    else if (!this->s0 && !this->s1 && this->s2 && !this->s3 && !this->s4)
        error = 0;
    else if (!this->s0 && this->s1 && this->s2 && !this->s3 && !this->s4)
        error = -1;
    else if (!this->s0 && this->s1 && !this->s2 && !this->s3 && !this->s4)
        error = -2;
    else if (this->s0 && this->s1 && !this->s2 && !this->s3 && !this->s4)
        error = -3;
    else if (this->s0 && !this->s1 && !this->s2 && !this->s3 && !this->s4)
        error = -4;
}

void Sensor::log()
{
    Serial.print("A0: ");
    Serial.print(analogRead(64));
    Serial.print(" A1: ");
    Serial.print(analogRead(55));
    Serial.print(" A2: ");
    Serial.print(analogRead(56));
    Serial.print(" A3: ");
    Serial.print(analogRead(57));
    Serial.print(" A4: ");
    Serial.print(analogRead(58));
    Serial.println("");
    Serial.print("A5: ");
    Serial.print(analogRead(63));
    Serial.print(" A6: ");
    Serial.print(analogRead(59));
    Serial.print(" A7: ");
    Serial.print(analogRead(61));
    Serial.print(" A8: ");
    Serial.print(analogRead(62));
    Serial.print(" A9: ");
    Serial.print(analogRead(60));
    Serial.println("");
    Serial.println("");
}

bool Sensor::isBlack(short sensor_value, int offset)
{
    return sensor_value > this->black_value + offset;
}

bool Sensor::isCenter()
{
    return this->s0 && this->s1 && this->s2 && this->s3 && this->s4;
}

bool Sensor::isLeftCross()
{
    return this->s0 && this->s1 && this->s2 && !this->s3 && !this->s4;
}

bool Sensor::isRightCross()
{
    return !this->s0 && !this->s1 && this->s2 && this->s3 && this->s4;
}