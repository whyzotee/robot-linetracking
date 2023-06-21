#include <SPI.h>
#include <Wire.h>
#include <string.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_SSD1306.h>

#include "arm.h"
#include "motor.h"
#include "sensor.h"
#include "SpeedyStepper.h"

#define TX1 18
#define RX1 19

#define EMERGENCY_PIN 3
#define STEPPER_SW 32

#define MODE_P1 26
#define MODE_P2 28
#define MODE_P3 30
#define MODE_P4 33

#define LED_SENSOR_A0 23
#define LED_SENSOR_A1 25
#define LED_SENSOR_A2 27
#define LED_SENSOR_A3 29
#define LED_SENSOR_A4 31

#define LED_STATUS_RED 38
#define LED_STATUS_YELLOW 39
#define LED_STATUS_GREEN 40

#define LIMIT_SW 42

#define MOTOR_STEP_PIN 35
#define MOTOR_DIRECTION_PIN 34

#define OLED_RESET -1

SpeedyStepper stepper;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire, OLED_RESET);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

VL53L0X_RangingMeasurementData_t measure;
u16 range;

Arm arm;
Sensor sensor;
Motor motor;

// Y1 B1 G1 R1 R2 G2 B2 Y2
bool putOBJ_RIGHT[8] = {false, false, false, false, false, false, false, false};

// Y2 B2 G2 R2 R1 G1 B1 Y1
bool putOBJ_LEFT[8] = {false, false, false, false, false, false, false, false};

// Controller Time
uint64_t currentTime = 0;

// Delay Time
uint64_t delayLog = 0;
uint64_t delayLight = 0;
uint64_t delayTurn = 0;
uint64_t delayCross = 0;
uint64_t delayGoToCenter = 0;
// uint64_t delayNotFoundSensor = 0;
// uint64_t delayNotFoundColor = 0;

char buf[2];

uint8_t quest = 1;
uint8_t soi_count = 0;

enum Status
{
  red,
  yellow,
  green
};

Status status;

String mode = "";
String pMode = "None";

bool checkQRBox = false;
bool isMove = true;
bool isSoi = false;
bool emergencyBTN = false;

uint8_t Kp = 25;
uint8_t Kd = 2;
uint8_t Ki = 0.1;
uint16_t motorSpeed, leftSpeed, rightSpeed;
uint16_t baseSpeed = 200;
uint8_t maxSpeed = 255;

uint16_t error = 0;
uint16_t pre_error = 0;
uint16_t sum_error = 0;

void movePID()
{
  if (!sensor.s0 && !sensor.s1 && !sensor.s2 && !sensor.s3 && sensor.s4)
    error = 4;
  else if (!sensor.s0 && !sensor.s1 && !sensor.s2 && sensor.s3 && sensor.s4)
    error = 3;
  else if (!sensor.s0 && !sensor.s1 && !sensor.s2 && sensor.s3 && !sensor.s4)
    error = 2;
  else if (!sensor.s0 && !sensor.s1 && sensor.s2 && sensor.s3 && !sensor.s4)
    error = 1;
  else if (!sensor.s0 && !sensor.s1 && sensor.s2 && !sensor.s3 && !sensor.s4)
    error = 0;
  else if (!sensor.s0 && sensor.s1 && sensor.s2 && !sensor.s3 && !sensor.s4)
    error = -1;
  else if (!sensor.s0 && sensor.s1 && !sensor.s2 && !sensor.s3 && !sensor.s4)
    error = -2;
  else if (sensor.s0 && sensor.s1 && !sensor.s2 && !sensor.s3 && !sensor.s4)
    error = -3;
  else if (sensor.s0 && !sensor.s1 && !sensor.s2 && !sensor.s3 && !sensor.s4)
    error = -4;
  else if (!sensor.s0 && !sensor.s1 && !sensor.s2 && !sensor.s3 && !sensor.s4)
    error = pre_error;

  // PID Calculate
  motorSpeed = (Kp * error) + (Kd * (error - pre_error)) + (Ki * sum_error);
  leftSpeed = baseSpeed + motorSpeed;
  rightSpeed = baseSpeed - motorSpeed;

  // Limit speed min max
  leftSpeed = constrain(leftSpeed, -maxSpeed, maxSpeed);
  rightSpeed = constrain(rightSpeed, -maxSpeed, maxSpeed);

  // set drive motor speed
  digitalWrite(motor.frontLeft[2], 1);
  digitalWrite(motor.frontRight[2], 1);
  digitalWrite(motor.backLeft[2], 1);
  digitalWrite(motor.backRight[2], 1);

  analogWrite(motor.frontLeft[0], leftSpeed);
  analogWrite(motor.frontRight[1], rightSpeed);
  analogWrite(motor.backLeft[0], leftSpeed);
  analogWrite(motor.backRight[1], rightSpeed);

  pre_error = error;
  sum_error += error;
}

void displayStatus()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("P Mode : ");
  display.println(pMode);
  display.print("PID Sum Error: ");
  display.println(sum_error);
  display.print("Color: ");
  display.println(buf[0]);
  display.print("Qr Code: ");
  display.println(buf[1]);
  // display.print("Now Quest: ");
  // display.println(quest);
  display.display();
}

void gotoCenter()
{
  if (delayGoToCenter == 0)
    delayGoToCenter = currentTime;

  if (currentTime - delayGoToCenter > 3250)
  {

    if (sensor.isCenter())
    {
      delay(100);
      mode = "CT_CROSS";
      delayGoToCenter = 0;
    }
  }

  motor.move(255, "front");
}

void gotoSoi()
{
  if (sensor.s2)
    motor.move(90, "front");
  else if ((sensor.s0) || (sensor.s1))
    motor.move(55, "left");
  else if ((sensor.s3) || (sensor.s4))
    motor.move(55, "right");
}

void gotoOBJ(char direction)
{
  if (delayTurn == 0)
    delayTurn = currentTime;

  if (currentTime - delayTurn > 1000)
  {
    if (sensor.s2)
    {
      motor.move(255, "stop");
      arm.centerArm();
      stepper.moveRelativeInSteps(-32000);
      mode = "OBJ";
      delayTurn = 0;
    }
  }

  if (direction == 'L')
    motor.move(255, "left");

  if (direction == 'R')
    motor.move(255, "right");
}

void keepOBJ(char direction)
{
  motor.move(0, "stop");
  delay(500);
  arm.getObj();
  delay(1000);
  stepper.moveRelativeInSteps(14000);
  delay(500);

  if (direction == 'C')
  {
    stepper.moveRelativeInSteps(16000);
    delay(500);

    quest += 1;
    checkQRBox = false;
    isSoi = true;
    delayGoToCenter = 0;
    return;
  }

  if (direction == 'L')
    arm.keepObjLeft();
  if (direction == 'R')
    arm.keepObjRight();
  stepper.moveRelativeInSteps(-5000);

  checkQRBox = true;

  stepper.moveRelativeInSteps(16000);
  delay(500);
  arm.reset();
  stepper.moveRelativeInSteps(-26000);
  delay(500);

  quest += 1;
  checkQRBox = false;
  isSoi = true;
  delayGoToCenter = 0;
}

void putOBJ(char direction)
{
  if (direction == 'C')
  {
    motor.move(0, "stop");
    delay(500);
    arm.reset();
    delay(1000);
    stepper.moveRelativeInSteps(16000);
    delay(500);
    motor.move(150, "back");
    delay(1000);
    mode = "END";
  }

  if (direction == 'R')
  {
    motor.move(0, "stop");
    delay(1000);
    arm.reset();
    motor.move(150, "back");
    delay(1000);
  }

  if (direction == 'L')
  {
    motor.move(0, "stop");
    delay(1000);
    arm.reset();
    motor.move(150, "back");
    delay(1000);
  }

  stepper.moveRelativeInSteps(-20000);

  quest += 1;
  isSoi = true;
  delayTurn = 0;

  mode = "END";
}

void OBJ_LEFT()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(850);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 4)
        {
          quest = 3;
          soi_count = 0;
        }
      }
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('R');

    gotoSoi();
    break;
  case 4:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 5;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 5:
    // เข้าไปหยิบกล่อง
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        quest = 6;
        delayGoToCenter = 0;
      }
    }

    motor.move(150, "sright");
    break;
  case 6:
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('L');

    gotoSoi();
    break;
  case 7:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 8;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 8:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        quest = 9;
        delayGoToCenter = 0;
      }
    }

    motor.move(150, "sright");
    break;
  case 9:
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('C');

    gotoSoi();
    break;
  case 10:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 11;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 11:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        delay(300);
        quest = 12;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "sright");
    break;
  case 12:
    if (sensor.isCenter())
      quest = 13;

    motor.move(150, "back");
    break;
  case 13:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest = 14;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "left");
    break;
  case 14:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.isCenter())
      {
        quest = 1;
        mode = "PUTOBJ";
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  }
}

void OBJ_LEFT_2()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(850);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      quest = 3;
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (range > 80 && range < 100 && !checkQRBox)
    {
      keepOBJ('C');
      motor.move(150, "sright");
      delay(500);
    }

    gotoSoi();
    break;
  case 4:
    if (sensor.isCenter())
      quest = 5;

    motor.move(150, "back");
    break;

  case 5:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.s2)
      {
        quest = 6;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "right");
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.isCenter())
      {
        quest = 1;
        mode = "END_TEST";
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  }
}

void OBJ_RIGHT_3RIHGT()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(500);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 600)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 4)
        {
          quest = 3;
          soi_count = 0;
        }
      }
    }
    motor.move(150, "sright");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('R');

    gotoSoi();
    break;
  case 4:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 5;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 5:
    // เข้าไปหยิบกล่อง
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        quest = 6;
        delayGoToCenter = 0;
      }
    }

    motor.move(150, "sleft");
    break;
  case 6:
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('L');

    gotoSoi();
    break;
  case 7:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 8;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 8:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        quest = 9;
        delayGoToCenter = 0;
      }
    }

    motor.move(150, "sleft");
    break;
  case 9:
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('C');

    gotoSoi();
    break;
  case 10:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 11;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 11:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        delay(300);
        quest = 12;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "sleft");
    break;
  case 12:
    if (sensor.isCenter())
      quest = 13;

    motor.move(150, "back");
    break;
  case 13:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest = 14;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "right");
    break;
  case 14:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.isCenter())
      {
        quest = 1;
        mode = "PUT_TEST";
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  }
}

void OBJ_RIGHT_3CENTER()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(650);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        quest = 3;
        delayGoToCenter = currentTime;
      }
    }
    motor.move(150, "sright");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('C');

    gotoSoi();
    break;
  case 4:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      quest = 12;
      delayGoToCenter = 0;
    }
    motor.move(150, "sleft");
    break;
  case 5:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        quest = 13;
        delayGoToCenter = 0;
      }
    }

    motor.move(150, "sright");
    break;
  case 6:
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('L');

    gotoSoi();
    break;
  case 7:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 8;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 8:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        quest = 9;
        delayGoToCenter = 0;
      }
    }

    motor.move(150, "sleft");
    break;
  case 9:
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('C');

    gotoSoi();
    break;
  case 10:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 11;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 11:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        delay(300);
        quest = 12;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "sright");
    break;
  case 12:
    if (sensor.isCenter())
      quest = 13;

    motor.move(150, "back");
    break;
  case 13:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest = 14;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "right");
    break;
  case 14:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.isCenter())
      {
        quest = 1;
        mode = "END_TEST";
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  }
}

void OBJ_RIGHT_2LEFT()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 600)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 4)
        {
          quest = 3;
          soi_count = 0;
        }
      }
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('R');

    gotoSoi();
    break;
  case 4:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 5;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 5:
    // เข้าไปหยิบกล่อง
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        quest = 6;
        delayGoToCenter = 0;
      }
    }

    motor.move(150, "sright");
    break;
  case 6:
    if (range > 80 && range < 100 && !checkQRBox)
      keepOBJ('L');

    gotoSoi();
    break;
  case 7:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 8;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 8:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      if (sensor.s2)
      {
        delay(300);
        quest = 9;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "sright");
    break;
  case 9:
    if (sensor.isCenter())
      quest = 10;

    motor.move(150, "back");
    break;
  case 10:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest = 11;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "left");
    break;
  case 11:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.isCenter())
      {
        quest = 1;
        mode = "PUTOBJ";
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  }
}

void putOBJ_Test()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        quest = 3;
        delayGoToCenter = currentTime;
      }
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ('C');

    gotoSoi();
    break;
  case 4:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 750)
    {
      quest = 5;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  }
}

void putOBJ_LEFT_Y2()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 4)
          quest = 3;
      }
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ(' ');

    gotoSoi();
    break;
  case 4:
    quest = 1;
    mode == "END";
    break;
  }
}

void putOBJ_LEFT_B2()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 3)
          quest = 3;
      }
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ(' ');

    gotoSoi();
    break;
  case 4:
    quest = 1;
    mode == "END";
    break;
  }
}

void putOBJ_LEFT_G2()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 2)
          quest = 3;
      }
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ(' ');

    gotoSoi();
    break;
  case 4:
    quest = 1;
    mode == "END";
    break;
  }
}

void putOBJ_LEFT_R2()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 2)
          quest = 3;
      }
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ(' ');

    gotoSoi();
    break;
  case 4:
    quest = 1;
    mode == "END";
    break;
  }
}

void putOBJ_LEFT_R1()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        quest = 3;
        delayGoToCenter = currentTime;
      }
    }
    motor.move(150, "sright");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ(' ');

    gotoSoi();
    break;
  case 4:
    quest = 1;
    mode == "END";
    break;
  }
}

void putOBJ_LEFT_G1()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 2)
          quest = 3;
      }
    }
    motor.move(150, "sright");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ(' ');

    gotoSoi();
    break;
  case 4:
    quest = 1;
    mode == "END";
    break;
  }
}

void putOBJ_LEFT_B1()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 3;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 3)
          quest = 3;
      }
    }
    motor.move(150, "sright");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ(' ');

    gotoSoi();
    break;
  case 4:
    quest = 1;
    mode == "END";
    break;
  }
}

void putOBJ_LEFT_Y1()
{
  switch (quest)
  {
  case 1:
    // เดินจนถึงแยก
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        delay(950);
        quest = 2;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 2:
    // slide จนถึงซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s2)
      {
        soi_count += 1;
        delayGoToCenter = currentTime;

        if (soi_count == 4)
          quest = 3;
      }
    }
    motor.move(150, "sright");
    break;
  case 3:
    // เข้าไปหยิบกล่อง
    if (digitalRead(LIMIT_SW) == 0)
      putOBJ(' ');

    gotoSoi();
    break;
  case 4:
    quest = 1;
    mode == "END";
    break;
  }
}

void lightBlink(uint8_t pin)
{
  if (currentTime - delayLight >= 1000)
  {
    digitalWrite(pin, LOW);
    if (currentTime - delayLight >= 2000)
    {
      digitalWrite(pin, HIGH);
      delayLight = currentTime;
    }
  }
}

void lightReset()
{
  digitalWrite(LED_STATUS_RED, HIGH);
  digitalWrite(LED_STATUS_YELLOW, HIGH);
  digitalWrite(LED_STATUS_GREEN, HIGH);
}

void lightSensor()
{
  digitalWrite(LED_SENSOR_A0, !sensor.s0);
  digitalWrite(LED_SENSOR_A1, !sensor.s1);
  digitalWrite(LED_SENSOR_A2, !sensor.s2);
  digitalWrite(LED_SENSOR_A3, !sensor.s3);
  digitalWrite(LED_SENSOR_A4, !sensor.s4);
}

void ModeP1_OBJ_LEFT()
{
  status = green;
  pMode = "P1 OL";

  if (mode == "GTCT")
    gotoCenter();
  if (mode == "CT_CROSS")
    gotoOBJ('L');

  // เก็บกล่อง
  if (mode == "OBJ")
    OBJ_LEFT();

  if (mode == "PUTOBJ")
    putOBJ_Test();
  // putOBJ_RIGHT();

  if (mode == "END")
    motor.move(0, "stop");
}

void ModeP2_OBJ_RIGHT()
{
  status = green;
  pMode = "P2 OR";
  if (mode == "GTCT")
    gotoCenter();
  if (mode == "CT_CROSS")
    gotoOBJ('R');

  // เก็บกล่อง
  if (mode == "OBJ")
    OBJ_RIGHT_3RIHGT();
  if (mode == "OBJ_2")
    OBJ_RIGHT_3CENTER();
  if (mode == "OBJ_3")
    OBJ_RIGHT_2LEFT();

  // วางของรอบแรก
  if (mode == "PUT_TEST")
    putOBJ_Test();

  // วางกล่อง
  if (mode == "Y2")
    putOBJ_LEFT_Y2();
  if (mode == "B1")
    putOBJ_LEFT_B2();
  if (mode == "G2")
    putOBJ_LEFT_G2();
  if (mode == "R2")
    putOBJ_LEFT_R2();
  if (mode == "R1")
    putOBJ_LEFT_R1();
  if (mode == "G1")
    putOBJ_LEFT_G1();
  if (mode == "B1")
    putOBJ_LEFT_B1();
  if (mode == "Y1")
    putOBJ_LEFT_Y1();

  if (mode == "END")
    motor.move(0, "stop");
}

void ModeP3()
{
  status = green;
  pMode = "P3";
  if (mode == "GTCT")
    gotoCenter();
  if (mode == "CT_CROSS")
    gotoOBJ('R');

  if (mode == "OBJ")
    OBJ_RIGHT_3CENTER();

  if (mode == "END_TEST")
  {
    motor.move(255, "front");
    delay(1000);
    motor.move(0, "stop");
  }
}

void ModeP4()
{
  status = green;
  pMode = "P4";
  if (mode == "GTCT")
    gotoCenter();
  if (mode == "CT_CROSS")
    gotoOBJ('L');

  if (mode == "OBJ")
    OBJ_LEFT_2();

  if (mode == "END_TEST")
  {
    motor.move(255, "front");
    delay(1000);
    motor.move(0, "stop");
    mode = "";
  }
}

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);

  arm.begin();
  sensor.begin();
  motor.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);

  pinMode(EMERGENCY_PIN, INPUT_PULLUP);
  pinMode(STEPPER_SW, INPUT_PULLUP);

  pinMode(MODE_P1, INPUT_PULLUP);
  pinMode(MODE_P2, INPUT_PULLUP);
  pinMode(MODE_P3, INPUT_PULLUP);
  pinMode(MODE_P4, INPUT_PULLUP);

  pinMode(LED_STATUS_RED, OUTPUT);
  pinMode(LED_STATUS_YELLOW, OUTPUT);
  pinMode(LED_STATUS_GREEN, OUTPUT);

  pinMode(LED_SENSOR_A0, OUTPUT);
  pinMode(LED_SENSOR_A1, OUTPUT);
  pinMode(LED_SENSOR_A2, OUTPUT);
  pinMode(LED_SENSOR_A3, OUTPUT);
  pinMode(LED_SENSOR_A4, OUTPUT);

  pinMode(LIMIT_SW, INPUT_PULLUP);

  digitalWrite(LED_STATUS_RED, LOW);
  digitalWrite(LED_STATUS_YELLOW, HIGH);
  digitalWrite(LED_STATUS_GREEN, HIGH);

  if (!digitalRead(MODE_P1) && digitalRead(MODE_P2) && digitalRead(MODE_P3) && digitalRead(MODE_P4))
    for (uint8_t i = 0; i < 8; i++)
      putOBJ_RIGHT[i] = EEPROM.read(i);

  if (digitalRead(MODE_P1) && !digitalRead(MODE_P2) && !digitalRead(MODE_P3) && !digitalRead(MODE_P4))
    for (uint8_t i = 0; i < 8; i++)
      putOBJ_LEFT[i] = EEPROM.read(i);

  stepper.setSpeedInMillimetersPerSecond(1000000.0f);
  stepper.setAccelerationInStepsPerSecondPerSecond(1000000.0f);

  stepper.moveRelativeInSteps(22500);

  if (!lox.begin())
  {
    Serial.println(F("Failed to boot VL53L0X"));
    mode = "error";
    return;
  }

  // mode go to center
  mode = "GTCT";
}

void loop()
{
  // เก็บค่าที่ส่งมาจาก ESP32_CAM
  if (Serial1.available() > 0)
    Serial1.readBytes(buf, 2);

  // set stepper motor speed
  stepper.setSpeedInMillimetersPerSecond(1000000.0f);
  stepper.setAccelerationInStepsPerSecondPerSecond(1000000.0f);

  // CurrentTimeLine
  currentTime = millis();

  // emergency button
  if (!digitalRead(EMERGENCY_PIN))
    emergencyBTN = true;

  // Read Sensor From Class
  sensor.read();

  // Status in monitor
  displayStatus();

  // เซ็นเซอร์วัดระยะ เก็บค่า
  lox.rangingTest(&measure, false);

  // ดักค่าระยะทางจากเซ็นเซอร์วัดระยะ
  if (measure.RangeStatus != 4)
    range = measure.RangeMilliMeter;
  else
    range = 0;

  // if robot have error
  if (mode == "error")
    status = red;

  // Emergency button
  if (emergencyBTN)
    status = yellow;

  // LED Sensor Status
  lightSensor();

  // Status of LED
  switch (status)
  {
  case red:
    lightReset();
    digitalWrite(LED_STATUS_RED, LOW);
    break;
  case yellow:
    lightReset();
    lightBlink(LED_STATUS_YELLOW);
    break;
  case green:
    lightReset();
    lightBlink(LED_STATUS_GREEN);
    break;
  }

  /*




  Core Function




  */

  if (!emergencyBTN)
  {
    // P1 Mode LEFT (Switch ON)
    if (!digitalRead(MODE_P1) && digitalRead(MODE_P2) && digitalRead(MODE_P3) && digitalRead(MODE_P4))
      ModeP1_OBJ_LEFT();

    // P2 Mode (Switch ON)
    if (digitalRead(MODE_P1) && !digitalRead(MODE_P2) && digitalRead(MODE_P3) && digitalRead(MODE_P4))
      ModeP2_OBJ_RIGHT();

    // P3 Mode (Switch ON)
    if (digitalRead(MODE_P1) && digitalRead(MODE_P2) && !digitalRead(MODE_P3) && digitalRead(MODE_P4))
      ModeP3();

    // P4 Mode (Switch ON)
    if (digitalRead(MODE_P1) && digitalRead(MODE_P2) && digitalRead(MODE_P3) && !digitalRead(MODE_P4))
      ModeP4();

    // Default mode
    if (digitalRead(MODE_P1) && digitalRead(MODE_P2) && digitalRead(MODE_P3) && digitalRead(MODE_P4))
    {
      status = red;
      mode = "";
      pMode = "Default None";
      motor.move(0, "stop");
    }
  }

  /*




  End Core Function




  */

  /*

    Log Function

  */

  digitalWrite(LED_SENSOR_A0, digitalRead(LIMIT_SW));
  if (currentTime - delayLog > 250)
  {
    sensor.log();

    // Serial.print("mode: ");
    // Serial.println(mode);
    // Serial.print("distance: ");
    // Serial.println(range);

    // Serial.print("buf: ");
    // Serial.print(buf[0]);
    // Serial.println(buf[1]);

    Serial.print("LIMIT_SW : ");
    Serial.println(digitalRead(LIMIT_SW));

    Serial.println(" ");
    Serial.println(" ");

    delayLog = currentTime;
  }

  /*

  End Log Function

  */
}
