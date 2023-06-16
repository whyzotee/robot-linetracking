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
    this->arm.write(90);
    this->hand.write(0);
}

void Arm::move(String el, uint8_t deg)
{
    if (el == "arm")
        this->arm.write(deg);
    if (el == "hand")
        this->hand.write(deg);
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

void Arm::keepObjLeft()
{
    delay(1000);
    this->arm.write(20);
    delay(1000);
    this->hand.write(0);
    delay(1000);
}

void Arm::keepObjRight()
{
    delay(1000);
    this->arm.write(160);
    delay(1000);
    this->hand.write(0);
    delay(1000);
}

void Arm::centerArm()
{
    this->arm.write(100);
    delay(1000);
    this->hand.write(0);
    delay(1000);
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