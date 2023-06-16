#ifndef ARM_H
#define ARM_H

#include <Arduino.h>
#include <Stepper.h>
#include <Servo.h>

class Arm
{
private:
    Servo hand, arm;

    bool topLimit;

public:
    void begin();
    void reset();
    void keepHand();
    void move(String el, uint8_t deg);
    void moveup();
    void getObj();
    void keepObjLeft();
    void keepObjRight();
    void centerArm();
    void test();
};

#endif
