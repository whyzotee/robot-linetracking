#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <Arduino.h>

void move_setup();
void move(byte speed, byte direction);
void reset();

#endif