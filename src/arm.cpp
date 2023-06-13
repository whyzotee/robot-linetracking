#include "arm.h"

void Arm::begin()
{
    this->arm.attach(8);
    this->hand.attach(7);
    this->arm.write(20);
    this->hand.write(180);
}

void Arm::reset()
{
    this->arm.write(30);
    this->hand.write(180);
}

void Arm::keepHand()
{
    this->arm.write(20);
    this->hand.write(0);
}

void Arm::getObj()
{
    this->arm.write(90);
    this->hand.write(180);
}
void Arm::centerArm()
{
    this->arm.write(90);
    this->hand.write(0);
}

void Arm::test()
{
    // กางแขน
    this->hand.write(0);
    this->arm.write(30);
    delay(1000);
    // หุบแขน
    this->arm.write(90);
    this->hand.write(180);
    delay(1000);
}