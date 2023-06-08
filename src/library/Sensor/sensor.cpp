#include "sensor.h"

Sensor::Sensor(byte speed, int black_value)
{
    this->speed = speed;
    this->black_value = black_value;
};

void Sensor::sensor_read()
{
    this->sensor0 = isBlack(analogRead(64), 0);
    this->sensor1 = isBlack(analogRead(55), 0);
    this->sensor2 = isBlack(analogRead(56), 0);
    this->sensor3 = isBlack(analogRead(57), 0);
    this->sensor4 = isBlack(analogRead(58), 450);
    this->sensor5 = isBlack(analogRead(63), 0);
    this->sensor6 = isBlack(analogRead(59), 0);
    this->sensor7 = isBlack(analogRead(61), 0);
    this->sensor8 = isBlack(analogRead(62), 0);
    this->sensor9 = isBlack(analogRead(60), 0);
}

void Sensor::isBlack(short sensor_value, int offset)
{
    return sensor_value > this->black_value + offset;
}