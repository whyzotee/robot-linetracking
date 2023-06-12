#include "motor.h"

void Motor::begin(int baseSpeed, int maxSpeed, int minSpeed)
{
    this->old_direction = "";

    for (byte i = 0; i < 3; i++)
    {
        pinMode(this->frontLeft[i], OUTPUT);
        pinMode(this->frontRight[i], OUTPUT);
        pinMode(this->backLeft[i], OUTPUT);
        pinMode(this->backRight[i], OUTPUT);
    }

    // this->reset();

    this->Kp = 20;
    this->Kd = 5;
    this->Ki = 5;
    this->baseSpeed = baseSpeed;
    this->maxSpeed = maxSpeed;
    this->minSpeed = minSpeed;
}

void Motor::reset()
{
    for (byte i = 0; i < 3; i++)
    {
        digitalWrite(this->frontLeft[i], LOW);
        digitalWrite(this->frontRight[i], LOW);
        digitalWrite(this->backLeft[i], LOW);
        digitalWrite(this->backRight[i], LOW);
    }
    // analogWrite(this->frontLeft[2], 0);
    // analogWrite(this->frontRight[2], 0);
    // analogWrite(this->backLeft[2], 0);
    // analogWrite(this->backRight[2], 0);
}

void Motor::move(int speed, String direction)
{
    // Reset Digital Value intA and intB
    if (this->old_direction != direction)
    {
        this->reset();
        this->old_direction = direction;
    }

    // PID Calculate
    // this->motorSpeed = (this->Kp * this->sensor.error) + (this->Kd * (this->sensor.error - this->sensor.pre_error)) + (this->Ki * this->sensor.sum_error);
    // this->leftSpeed = this->baseSpeed + this->motorSpeed;
    // this->rightSpeed = this->baseSpeed - this->motorSpeed;

    // if (this->leftSpeed > this->maxSpeed)
    //     this->leftSpeed = this->maxSpeed;
    // if (this->rightSpeed > this->maxSpeed)
    //     this->rightSpeed = this->maxSpeed;

    // if (this->leftSpeed < this->minSpeed)
    //     this->leftSpeed = this->minSpeed;
    // if (this->rightSpeed < this->minSpeed)
    //     this->rightSpeed = this->minSpeed;

    // this->sensor.pre_error = this->sensor.error;
    // this->sensor.sum_error += this->sensor.error;

    // Set Enable Pin Speed
    // if (direction == "sleft")
    // {
    //     analogWrite(this->frontLeft[2], speed);
    //     analogWrite(this->frontRight[2], speed - 160);
    //     analogWrite(this->backLeft[2], speed);
    //     analogWrite(this->backRight[2], speed - 160);
    // }
    // else if (direction == "sright")
    // {
    //     analogWrite(this->frontLeft[2], speed);
    //     analogWrite(this->frontRight[2], speed - 175);
    //     analogWrite(this->backLeft[2], speed);
    //     analogWrite(this->backRight[2], speed);
    // }
    // else
    // {
    analogWrite(this->frontLeft[2], speed);
    analogWrite(this->frontRight[2], speed);
    analogWrite(this->backLeft[2], speed);
    analogWrite(this->backRight[2], speed);
    // }

    if (direction == "stop")
    {
        // Break Move
        analogWrite(this->frontLeft[2], 0);
        analogWrite(this->frontRight[2], 0);
        analogWrite(this->backLeft[2], 0);
        analogWrite(this->backRight[2], 0);
    }
    else if (direction == "front")
    {
        // Move Front
        digitalWrite(this->frontLeft[0], 1);
        digitalWrite(this->frontRight[1], 1);
        digitalWrite(this->backLeft[0], 1);
        digitalWrite(this->backRight[1], 1);
    }
    else if (direction == "left")
    {
        // Move Left
        digitalWrite(this->frontLeft[1], 1);
        digitalWrite(this->frontRight[1], 1);
        digitalWrite(this->backLeft[1], 1);
        digitalWrite(this->backRight[1], 1);
    }
    else if (direction == "sleft")
    {
        // Move Slide Left
        digitalWrite(this->frontLeft[1], 1);
        digitalWrite(this->frontRight[1], 1);
        digitalWrite(this->backLeft[0], 1);
        digitalWrite(this->backRight[0], 1);
    }
    else if (direction == "right")
    {
        // Move Right
        digitalWrite(this->frontLeft[0], 1);
        digitalWrite(this->frontRight[0], 1);
        digitalWrite(this->backLeft[0], 1);
        digitalWrite(this->backRight[0], 1);
    }
    else if (direction == "sright")
    {
        // Move Slide Right
        digitalWrite(this->frontLeft[0], 1);
        digitalWrite(this->frontRight[0], 1);
        digitalWrite(this->backLeft[1], 1);
        digitalWrite(this->backRight[1], 1);
    }
    else if (direction == "back")
    {
        // Move Back
        digitalWrite(this->frontLeft[1], 1);
        digitalWrite(this->frontRight[0], 1);
        digitalWrite(this->backLeft[1], 1);
        digitalWrite(this->backRight[0], 1);
    }
}

void Motor::balance_move(bool isFront)
{
    if (isFront)
    {
        if (sensor.s2)
        {
            // this->move(this->baseSpeed, "front");
            this->move(255, "front");
        }

        else if ((this->sensor.s0) || (this->sensor.s1))
            this->move(255, "left");
        else if ((this->sensor.s3) || (this->sensor.s4))
            this->move(255, "right");
        else
            this->move(0, "stop");
        // else
        // {
        //     if (isSoi)
        //     {
        //         if (!isFront)
        //         {
        //             motor.move(SPEED, "front");
        //         }
        //         else
        //         {
        //             motor.move(0, "stop");
        //             delay(1000);
        //             motor.move(SPEED, "back");
        //             delay(500);
        //             isFront = false;
        //         }
        //     }
        // }
    }
    else
    {
        if (this->sensor.s2)
            this->move(this->baseSpeed, "back");
        else if (this->sensor.s1 || this->sensor.s4)
            this->move(this->baseSpeed, "back");
        else if (this->sensor.s0 || this->sensor.s3)
            this->move(this->baseSpeed, "back");
        else
            this->move(0, "stop");
        // else
        // {
        //     if (isSoi)
        //     {
        //         motor.move(SPEED, "front");
        //         delay(750);
        //         motor.move(SPEED, "right");
        //         delay(750);
        //         isMove = false;
        //         isSoi = false;
        //         isFront = true;
        //         finish_soi_count += 1;
        //         Serial.println("back");
        //     }
        //     else
        //         motor.move(0, "stop");
        // }
    }
}

void Motor::controllPID()
{
}

void Motor::turn(char direction)
{
    if (direction == 'L')
    {
    }

    if (direction == 'R')
    {
    }
}

void Motor::slide(char direction)
{
    if (direction == 'L')
    {
        if (this->sensor.s5 || this->sensor.s6)
            this->move(this->baseSpeed, "sleft");
        else if (this->sensor.s7 || this->sensor.s8 || this->sensor.s9)
            this->move(100, "back");
        else
            this->move(this->baseSpeed, "stop");
    }

    if (direction == 'R')
    {
        if (this->sensor.s8 || this->sensor.s9)
            this->move(this->baseSpeed, "sright");
        else if (this->sensor.s5 || this->sensor.s6 || this->sensor.s7)
            this->move(100, "front");
        else
            this->move(this->baseSpeed, "stop");
    }
}