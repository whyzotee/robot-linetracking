#ifndef MORTOR_H
#define MORTOR_H

#include <Arduino.h>

void motor_setup();
void move(byte speed, String direction);
void reset();

#endif