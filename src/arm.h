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
    void moveup();
    void getObj();
    void centerArm();
    void test();
};

#endif
