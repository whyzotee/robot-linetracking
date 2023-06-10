#include "sensor.h"

void Sensor::begin()
{
    this->black_value = 300;
}

void Sensor::read()
{
    this->s0 = isBlack(analogRead(A0), 0);
    this->s1 = isBlack(analogRead(A1), 0);
    this->s2 = isBlack(analogRead(A2), 0);
    this->s3 = isBlack(analogRead(A3), 0);
    this->s4 = isBlack(analogRead(A4), 450);
    this->s5 = isBlack(analogRead(A5), 0);
    this->s6 = isBlack(analogRead(A6), 0);
    this->s7 = isBlack(analogRead(A7), 0);
    this->s8 = isBlack(analogRead(A8), 0);
    this->s9 = isBlack(analogRead(A9), 0);
    this->s10 = isBlack(analogRead(A10), 0);
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
    Serial.print(analogRead(A0));
    Serial.print(" A1: ");
    Serial.print(analogRead(A1));
    Serial.print(" A2: ");
    Serial.print(analogRead(A2));
    Serial.print(" A3: ");
    Serial.print(analogRead(A3));
    Serial.print(" A4: ");
    Serial.print(analogRead(A4));
    Serial.print("A5: ");
    Serial.print(analogRead(A5));
    Serial.println("");

    Serial.print(" A6: ");
    Serial.print(analogRead(A6));
    Serial.print(" A7: ");
    Serial.print(analogRead(A7));
    Serial.print(" A8: ");
    Serial.print(analogRead(A8));
    Serial.print(" A9: ");
    Serial.print(analogRead(A9));
    Serial.print(" A10: ");
    Serial.print(analogRead(A10));
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