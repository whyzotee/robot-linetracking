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

#define OLED_RESET -1

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

#define LIMIT_SW_LEFT 42
#define LIMIT_SW_RIGHT 43

#define MOTOR_STEP_PIN 35
#define MOTOR_DIRECTION_PIN 34

SpeedyStepper stepper;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire, OLED_RESET);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

VL53L0X_RangingMeasurementData_t measure;
u16 range;

Arm arm;
Sensor sensor;
Motor motor;

// Y1 B1 G1 R1 R2 G2 B2 Y2
bool isOBJ_FINISH_TO_RIGHT[8] = {false, false, false, false, false, false, false, false};

// Y2 B2 G2 R2 R1 G1 B1 Y1
bool isOBJ_FINISH_TO_LEFT[8] = {false, false, false, false, false, false, false, false};

// Controller Time
uint64_t currentTime = 0;

// Delay Time
uint64_t delayLog = 0;
uint64_t delayLight = 0;
uint64_t delayTurn = 0;
uint64_t delayCross = 0;
uint64_t delayGoToCenter = 0;
uint64_t delayNotFoundSensor = 0;
uint64_t delayNotFoundColor = 0;

char buf[2];
char cacheBox1[2];

byte quest = 1;
byte soi_count = 0;
byte finish_soi_count = 0;

enum Status
{
  red,
  yellow,
  green
};

Status status;

String pMode = "None";
String mode;

bool checkQRBox = false;
bool isMove = true;
bool isFront = true;
bool isSoi = false;
bool qrCodeLight = true;
bool emergencyBTN = false;

int error = 0;
int pre_error = 0;
int sum_error = 0;

int Kp = 25;
int Kd = 2;
int Ki = 0.1;
int motorSpeed, leftSpeed, rightSpeed;
int baseSpeed = 200;
int maxSpeed = 255;

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

  leftSpeed = constrain(leftSpeed, -maxSpeed, maxSpeed);
  rightSpeed = constrain(rightSpeed, -maxSpeed, maxSpeed);

  // old code

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
  display.println(cacheBox1[0]);
  display.print("Qr Code: ");
  display.println(cacheBox1[1]);
  // display.print("Now Quest: ");
  // display.println(quest);
  display.display();
}

void balance_move(char LOR)
{
  delayNotFoundSensor = currentTime;
  if (isFront)
  {
    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    // else
    //   motor.move(50, "front");
    delayNotFoundSensor = 0;
  }
  else
  {
    if (isSoi)
    {
      motor.move(255, "back");
      delay(1500);
      if (LOR == 'L')
        motor.move(255, "left");
      if (LOR == 'R')
        motor.move(255, "right");
      delay(1500);

      isFront = true;
      isSoi = false;
    }
    else
    {
      motor.move(0, "stop");
    }
  }
}

void gotoCenter()
{
  if (delayGoToCenter == 0)
    delayGoToCenter = currentTime;

  if (currentTime - delayGoToCenter > 3250)
  {

    if (sensor.isCenter())
    {
      delay(75);
      mode = "CT_CROSS";
      delayGoToCenter = 0;
    }
  }

  motor.move(255, "front");
}

void gotoOBJ(char direction)
{
  if (delayTurn == 0)
    delayTurn = currentTime;

  if (currentTime - delayTurn > 1000)
  {
    if (sensor.s2 || sensor.s3)
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
  delay(500);
  // stepper.moveRelativeInSteps(32000);
  stepper.moveRelativeInSteps(14000);
  delay(500);

  if (direction == 'C')
  {
    stepper.moveRelativeInSteps(16000);
    quest += 1;
    checkQRBox = false;
    isFront = false;
    isSoi = true;
    delayTurn = 0;
    return;
  }

  if (direction == 'L')
    arm.keepObjLeft();
  if (direction == 'R')
    arm.keepObjRight();
  stepper.moveRelativeInSteps(-5000);

  checkQRBox = true;

  stepper.moveRelativeInSteps(16000);
  delay(1000);
  arm.reset();
  stepper.moveRelativeInSteps(-24000);

  quest += 1;
  checkQRBox = false;
  isFront = false;
  isSoi = true;
  delayTurn = 0;

  // _mode = checkQrFromESP32();

  // if (checkQRBox && _mode != "")
  // {
  //   stepper.moveRelativeInSteps(16000);
  //   delay(1000);
  //   arm.reset();
  //   stepper.moveRelativeInSteps(-20000);

  //   quest += 1;
  //   checkQRBox = false;
  //   isFront = false;
  //   isSoi = true;
  //   delayTurn = 0;
  // }
}

void putOBJ()
{
  motor.move(0, "stop");

  quest += 1;
  isFront = false;
  isSoi = true;
  delayTurn = 0;
}

String checkQrFromESP32()
{
  if (cacheBox1[0] == 'R' && cacheBox1[1] == '1')
    return "R1";
  if (cacheBox1[0] == 'R' && cacheBox1[1] == '2')
    return "R2";
  if (cacheBox1[0] == 'G' && cacheBox1[1] == '1')
    return "G1";
  if (cacheBox1[0] == 'G' && cacheBox1[1] == '2')
    return "G2";
  if (cacheBox1[0] == 'B' && cacheBox1[1] == '1')
    return "B1";
  if (cacheBox1[0] == 'B' && cacheBox1[1] == '2')
    return "B2";
  if (cacheBox1[0] == 'Y' && cacheBox1[1] == '1')
    return "Y1";
  if (cacheBox1[0] == 'Y' && cacheBox1[1] == '2')
    return "Y2";
}

void first(String _mode)
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
        quest += 1;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (sensor.s2)
      {
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    motor.move(255, "left");
    break;
  case 3:
    // เดินเช็คแยกจนกว่าจะเจอซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (currentTime - delayCross > 1200)
      {
        if (sensor.s3 || sensor.s4)
        {
          soi_count += 1;
          delayCross = currentTime;

          if (soi_count == 3)
          {
            quest += 1;
            soi_count = 0;
            delayCross = 0;
            delayGoToCenter = 0;
          }
        }
      }
    }

    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 4:
    // เลี้ยวขวาซอยสุดท้าย
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s2)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "right");
    break;
  case 5:
    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();

      // if (cacheBox1[0] == 'R' || cacheBox1[0] == 'G' || cacheBox1[0] == 'B' || cacheBox1[0] == 'Y')
      // {
      //   stepper.moveRelativeInSteps(16000);
      //   delay(500);
      //   arm.reset();
      //   stepper.moveRelativeInSteps(-20000);

      //   quest += 1;
      //   checkQRBox = false;
      //   isFront = false;
      //   delayTurn = 0;
      // }
    }

    if (!checkQRBox)
    {
      if (sensor.s2)
        motor.move(155, "front");
      else if ((sensor.s0) || (sensor.s1))
        motor.move(100, "left");
      else if ((sensor.s3) || (sensor.s4))
        motor.move(100, "right");
    }

    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1200)
    {
      if (sensor.s3 || sensor.s4)
      {
        delay(250);
        quest += 1;
        isSoi = false;
        delayGoToCenter = 0;
      }
    }

    if (isSoi)
    {
      motor.move(255, "back");
      delay(1000);
      motor.move(255, "right");
      delay(1200);

      isFront = true;
      isSoi = false;
    }
    else
    {
      if (sensor.s2)
        motor.move(200, "front");
      else if ((sensor.s0) || (sensor.s1))
        motor.move(150, "left");
      else if ((sensor.s3) || (sensor.s4))
        motor.move(150, "right");
    }
    break;
  case 7:
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s3 || sensor.s4)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "right");
    break;
  case 8:
    if (sensor.isCenter())
    {
      // mode = cacheBox1[0] + cacheBox1[1];
      mode = _mode;
      quest = 1;
    }

    balance_move('\0');
    break;
  }
}

void second(String _mode)
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
        quest += 1;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (sensor.s2 || sensor.s3)
      {
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    motor.move(255, "left");
    break;
  case 3:
    // เดินเช็คแยกจนกว่าจะเจอซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (currentTime - delayCross > 1200)
      {
        if (sensor.s3 || sensor.s4)
        {
          soi_count += 1;
          delayCross = currentTime;

          if (soi_count == 2)
          {
            quest += 1;
            soi_count = 0;
            delayCross = 0;
            delayGoToCenter = 0;
          }
        }
      }
    }

    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 4:
    // เลี้ยวขวาซอยสุดท้าย
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "right");
    break;
  case 5:
    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
      // if (sensor.s2)
      // {
      //   quest += 1;
      //   isSoi = true;
      //   isFront = false;
      //   delayGoToCenter = 0;
      // }
    }

    balance_move('\0');
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 4250)
    {
      if (sensor.s3 || sensor.s4)
      {
        delay(250);
        quest += 1;
        isSoi = false;
        delayGoToCenter = 0;
      }
    }

    balance_move('R');
    break;
  case 7:
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s3 || sensor.s4)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "right");
    break;
  case 8:
    if (sensor.isCenter())
    {
      mode = _mode;
      quest = 1;
    }

    balance_move('\0');
    break;
  }
}

void third(String _mode)
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
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    balance_move('\0');
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (sensor.s2 || sensor.s3)
      {
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    motor.move(255, "left");
    break;
  case 3:
    // เดินเช็คแยกจนกว่าจะเจอซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (currentTime - delayCross > 1200)
      {
        if (sensor.s3 || sensor.s4)
        {
          soi_count += 1;
          delayCross = currentTime;

          if (soi_count == 1)
          {
            delay(200);
            quest += 1;
            soi_count = 0;
            delayCross = 0;
            delayGoToCenter = 0;
          }
        }
      }
    }

    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 4:
    // เลี้ยวขวาซอยสุดท้าย
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "right");
    break;
  case 5:
    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
      // if (sensor.s2)
      // {
      //   quest += 1;
      //   isSoi = true;
      //   isFront = false;
      //   delayGoToCenter = 0;
      // }
    }

    balance_move('\0');
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 4000)
    {
      if (sensor.s3 || sensor.s4)
      {
        delay(250);
        quest += 1;
        isSoi = false;
        delayGoToCenter = 0;
      }
    }

    balance_move('R');
    break;
  case 7:
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s3 || sensor.s4)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "right");
    break;
  case 8:
    if (sensor.isCenter())
    {
      mode = _mode;
      quest = 1;
    }

    balance_move('\0');
    break;
  }
}

void fourth(String _mode)
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
        delay(200);
        quest = 2;
        delayGoToCenter = 0;
      }
    }
    balance_move('\0');
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 200)
    {
      if (sensor.s2 || sensor.s3)
      {
        quest = 5;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "sleft");
    break;
  case 5:
    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      quest = 7;
      delayGoToCenter = 0;
    }
    motor.move(150, "back");
    break;
  case 7:
    if (sensor.isCenter())
    {
      mode = _mode;
      quest = 1;
    }

    balance_move('\0');
    break;
  }
}

void fifth(String _mode)
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
        delay(100);
        quest = 2;
        delayGoToCenter = 0;
      }
    }
    balance_move('\0');
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 200)
    {
      if (sensor.s2 || sensor.s3)
      {
        quest = 3;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "sright");
    break;
  case 3:
    // เดินเช็คแยกจนกว่าจะเจอซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1250)
    {
      quest = 4;
      delayGoToCenter = 0;
    }
    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 4:
    // เลี้ยวขวาซอยสุดท้าย
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest = 5;
        delayTurn = 0;
      }
    }

    motor.move(255, "left");
    break;
  case 5:
    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      // if (sensor.isSomeBlack())
      // {
      //   motor.move(255, "front");
      //   delay(300);
      //   quest = 6;
      //   delayGoToCenter = 0;
      // }

      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
    }

    if (sensor.s2)
      motor.move(155, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      quest = 7;
      delayGoToCenter = 0;
    }
    motor.move(255, "sright");
    break;
  case 7:
    if (sensor.isCenter())
    {
      mode = _mode;
      quest = 1;
    }

    balance_move('\0');
    break;
  }
}

void sixth(String _mode)
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
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    balance_move('\0');
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    motor.move(255, "right");
    break;
  case 3:
    // เดินเช็คแยกจนกว่าจะเจอซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (currentTime - delayCross > 1200)
      {
        if (sensor.s0 || sensor.s1)
        {
          soi_count += 1;
          delayCross = currentTime;

          if (soi_count == 1)
          {
            delay(200);
            quest += 1;
            soi_count = 0;
            delayCross = 0;
            delayGoToCenter = 0;
          }
        }
      }
    }
    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 4:
    // เลี้ยวขวาซอยสุดท้าย
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "left");
    break;
  case 5:
    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
      // if (sensor.s2)
      // {
      //   quest += 1;
      //   isSoi = true;
      //   isFront = false;
      //   delayGoToCenter = 0;
      // }
    }

    balance_move('\0');
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 4000)
    {
      if (sensor.s0 || sensor.s1)
      {
        delay(250);
        quest += 1;
        isSoi = false;
        delayGoToCenter = 0;
      }
    }

    balance_move('L');
    break;
  case 7:
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s0 || sensor.s1)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "left");
    break;
  case 8:
    if (sensor.isCenter())
    {
      mode = _mode;
      quest = 1;
    }

    balance_move('\0');
    break;
  }
}

void seventh(String _mode)
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
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    balance_move('\0');
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    motor.move(255, "right");
    break;
  case 3:
    // เดินเช็คแยกจนกว่าจะเจอซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (currentTime - delayCross > 1200)
      {
        if (sensor.s0 || sensor.s1)
        {
          soi_count += 1;
          delayCross = currentTime;

          if (soi_count == 2)
          {
            quest += 1;
            soi_count = 0;
            delayCross = 0;
            delayGoToCenter = 0;
          }
        }
      }
    }
    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 4:
    // เลี้ยวขวาซอยสุดท้าย
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s2 || sensor.s3)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "left");
    break;
  case 5:
    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      // if (sensor.s2)
      // {
      //   quest += 1;
      //   isSoi = true;
      //   isFront = false;
      //   delayGoToCenter = 0;
      // }
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
    }

    balance_move('\0');
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 4250)
    {
      if (sensor.s0 || sensor.s1)
      {
        delay(250);
        quest += 1;
        isSoi = false;
        delayGoToCenter = 0;
      }
    }

    balance_move('L');
    break;
  case 7:
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s0 || sensor.s1)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "left");
    break;
  case 8:
    if (sensor.isCenter())
    {
      mode = _mode;
      quest = 1;
    }

    balance_move('\0');
    break;
  }
}

void eighth(String _mode)
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
        quest += 1;
        delayGoToCenter = 0;
      }
    }

    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest += 1;
        delayGoToCenter = 0;
      }
    }
    motor.move(255, "right");
    break;
  case 3:
    // เดินเช็คแยกจนกว่าจะเจอซอยสุดท้าย
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (currentTime - delayCross > 1200)
      {
        if (sensor.s0 || sensor.s1)
        {
          soi_count += 1;
          delayCross = currentTime;

          if (soi_count == 3)
          {
            quest += 1;
            soi_count = 0;
            delayCross = 0;
            delayGoToCenter = 0;
          }
        }
      }
    }

    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 4:
    // เลี้ยวขวาซอยสุดท้าย
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s2 || sensor.s3)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(200, "left");
    break;
  case 5:

    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
    }

    if (!checkQRBox)
    {
      if (sensor.s2)
        motor.move(155, "front");
      else if ((sensor.s0) || (sensor.s1))
        motor.move(100, "left");
      else if ((sensor.s3) || (sensor.s4))
        motor.move(100, "right");
    }
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 4750)
    {
      if (sensor.s0 || sensor.s1)
      {
        delay(250);
        quest += 1;
        delayGoToCenter = 0;
      }
    }

    if (isSoi)
    {
      motor.move(255, "back");
      delay(1200);
      motor.move(255, "left");
      delay(1000);
      isSoi = false;
    }

    motor.move(255, "left");
    break;
  case 7:
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1000)
    {
      if (sensor.s1 || sensor.s2)
      {
        quest += 1;
        delayTurn = 0;
      }
    }

    motor.move(255, "left");
    break;
  case 8:
    if (sensor.isCenter())
    {
      mode = _mode;
      quest = 1;
    }

    balance_move('\0');
    break;
  }
}

void rightOBJ(String _mode)
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
        delay(200);
        quest = 2;
        delayGoToCenter = 0;
      }
    }
    if (sensor.s2)
      motor.move(200, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(150, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(150, "right");
    break;
  case 2:
    // เลี้ยวซ้ายจนเจอเส้น
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 200)
    {
      if (sensor.s2)
      {
        quest = 3;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "sleft");
    break;
  case 3:
    // เดินตรง 1 วิ แล้วถอยหลังกลับ
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('R');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
    }

    if (sensor.s2)
      motor.move(90, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(55, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(55, "right");
    break;
  case 4:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        motor.move(150, "front");
        delay(1000);
        quest = 5;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "back");
    break;
  case 5:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
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
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('L');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
    }

    if (sensor.s2)
      motor.move(90, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(55, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(55, "right");
    break;
  case 7:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        motor.move(150, "front");
        delay(1000);
        quest = 8;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "back");
    break;
  case 8:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
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
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1000)
    {
      if (range > 80 && range < 100 && !checkQRBox)
        keepOBJ('C');
      if (!digitalRead(LIMIT_SW_LEFT) && !digitalRead(LIMIT_SW_RIGHT))
        putOBJ();
    }

    if (sensor.s2)
      motor.move(90, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(55, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(55, "right");
    break;
  case 10:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.isCenter())
      {
        motor.move(150, "left");
        delay(3000);
        quest = 11;
        delayGoToCenter = 0;
      }
    }
    motor.move(150, "back");
    break;
  case 11:
    if (sensor.s1 || sensor.s2)
    {
      quest = 12;
      delayGoToCenter = 0;
    }
    motor.move(150, "sright");
    break;
  case 12:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      if (sensor.s0 || sensor.s1)
      {
        quest = 13;
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
  case 13:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 1500)
    {
      if (sensor.s2)
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

    if (currentTime - delayGoToCenter > 1000)
    {
      if (sensor.isCenter())
        quest = 15;
    }

    if (sensor.s2)
      motor.move(150, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(100, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(100, "right");
    break;
  case 15:
    if (sensor.isCenter())
    {
      delay(2000);
      mode = "END";
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
  {
    // if (!isOBJ_FINISH_TO_RIGHT[0])
    //   first("");
    // else if (!isOBJ_FINISH_TO_RIGHT[1])
    //   second("");
    // else if (!isOBJ_FINISH_TO_RIGHT[2])
    //   third("");
    if (!isOBJ_FINISH_TO_RIGHT[3])
      fourth("");
    else if (!isOBJ_FINISH_TO_RIGHT[4])
      fifth("");
    else if (!isOBJ_FINISH_TO_RIGHT[5])
      sixth("");
    else if (!isOBJ_FINISH_TO_RIGHT[6])
      seventh("");
    else if (!isOBJ_FINISH_TO_RIGHT[7])
      eighth("");
  }

  // วางกล่อง
  if (mode == "Y1")
    first("OBJ");
  if (mode == "B1")
    second("OBJ");
  if (mode == "G1")
    third("OBJ");
  if (mode == "R1")
    fourth("OBJ");
  if (mode == "R2")
    fifth("OBJ");
  if (mode == "G2")
    sixth("OBJ");
  if (mode == "B2")
    seventh("OBJ");
  if (mode == "Y2")
    eighth("OBJ");
}

void ModeP1_OBJ_RIGHT()
{
  status = green;
  pMode = "P1 OR";

  if (mode == "GTCT")
    gotoCenter();
  if (mode == "CT_CROSS")
    gotoOBJ('R');

  // เก็บกล่อง
  if (mode == "OBJ")
  {
    if (!isOBJ_FINISH_TO_LEFT[0])
      eighth("Y2");
    else if (!isOBJ_FINISH_TO_LEFT[1])
      seventh("B2");
    else if (!isOBJ_FINISH_TO_LEFT[2])
      sixth("G2");
    else if (!isOBJ_FINISH_TO_LEFT[3])
      fifth("R2");
    else if (!isOBJ_FINISH_TO_LEFT[4])
      fourth("R1");
    else if (!isOBJ_FINISH_TO_LEFT[5])
      third("G1");
    else if (!isOBJ_FINISH_TO_LEFT[6])
      second("B1");
    else if (!isOBJ_FINISH_TO_LEFT[7])
      first("Y1");
  }

  // วางกล่อง
  if (mode == "Y2")
    first("OBJ");
  if (mode == "B2")
    second("OBJ");
  if (mode == "G2")
    third("OBJ");
  if (mode == "R2")
    fourth("OBJ");
  if (mode == "R1")
    fifth("OBJ");
  if (mode == "G1")
    sixth("OBJ");
  if (mode == "B1")
    seventh("OBJ");
  if (mode == "Y1")
    eighth("OBJ");
}

void ModeP2()
{
  status = green;
  pMode = "P2";

  if (mode == "GTCT")
    gotoCenter();
  if (mode == "CT_CROSS")
    gotoOBJ('L');

  if (mode == "OBJ")
    rightOBJ("");
}

void ModeP3()
{
  status = green;
  pMode = "P3";

  if (mode == "GTCT")
    gotoCenter();
  if (mode == "CT_CROSS")
    motor.move(0, "stop");

  // arm.move("arm", 90);
  // arm.move("hand", 180);
  // delay(1000);
  // stepper.moveRelativeInSteps(12000);
  // delay(1000);
  // arm.move("arm", 20);
  // delay(500);
  // arm.move("hand", 0);
  // delay(1000);
  // // หลังจากวางเสร็จ

  // // Reset แขน
  // stepper.moveRelativeInSteps(16000);
  // delay(500);
  // arm.move("arm", 90);
  // stepper.moveRelativeInSteps(-32000);
  // delay(2000);

  // // เอากล่องจาก stock
  // stepper.moveRelativeInSteps(32000);
  // delay(1000);
  // arm.move("arm", 20);
  // arm.move("hand", 0);
  // delay(500);
  // stepper.moveRelativeInSteps(-16000);
  // delay(500);
  // arm.move("hand", 180);
  // delay(500);
  // stepper.moveRelativeInSteps(16000);
  // delay(500);
  // arm.move("arm", 90);
  // delay(1500);
  // arm.move("hand", 0);
  // delay(2000);
  // stepper.moveRelativeInSteps(-16000);
  // delay(2000);
}

void ModeP4()
{
  status = green;
  pMode = "P4";
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

  pinMode(46, OUTPUT);

  pinMode(LIMIT_SW_LEFT, INPUT_PULLUP);
  pinMode(LIMIT_SW_RIGHT, INPUT_PULLUP);

  digitalWrite(LED_STATUS_RED, LOW);
  digitalWrite(LED_STATUS_YELLOW, HIGH);
  digitalWrite(LED_STATUS_GREEN, HIGH);

  if (!digitalRead(MODE_P1) && digitalRead(MODE_P2) && digitalRead(MODE_P3) && digitalRead(MODE_P4))
    for (uint8_t i = 0; i < 8; i++)
      isOBJ_FINISH_TO_RIGHT[i] = EEPROM.read(i);

  if (digitalRead(MODE_P1) && !digitalRead(MODE_P2) && !digitalRead(MODE_P3) && !digitalRead(MODE_P4))
    for (uint8_t i = 0; i < 8; i++)
      isOBJ_FINISH_TO_LEFT[i] = EEPROM.read(i);

  for (uint8_t i = 3; i >= 1; i--)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Hello World!");
    display.println("Robot By EN CMTC");
    display.println("");
    display.print("Start Robot in: ");
    display.println(i);
    display.setCursor(0, 0);
    display.display();
    delay(1000);
  }

  stepper.setSpeedInMillimetersPerSecond(1000000.0f);
  stepper.setAccelerationInStepsPerSecondPerSecond(1000000.0f);

  if (digitalRead(MODE_P3))
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

  if (buf[0] == 'R' || buf[0] == 'G' || buf[0] == 'B' || buf[0] == 'Y')
  {
    if (buf != cacheBox1)
    {
      cacheBox1[0] = buf[0];
      cacheBox1[1] = buf[1];
      qrCodeLight = false;
    }
  }

  // qr code light
  digitalWrite(46, qrCodeLight);

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

  // if Robot have error
  if (mode == "error")
    status = red;

  // Emergency Button
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

    // P1 Mode RIGHT (Switch OFF)
    if (digitalRead(MODE_P1) && !digitalRead(MODE_P2) && !digitalRead(MODE_P3) && !digitalRead(MODE_P4))
      ModeP1_OBJ_RIGHT();

    // P2 Mode (Switch ON)
    if (digitalRead(MODE_P1) && !digitalRead(MODE_P2) && digitalRead(MODE_P3) && digitalRead(MODE_P4))
      ModeP2();

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
      pMode = "None";
      motor.move(0, "stop");
    }
  }

  /*




  End Core Function




  */

  /*

    Log Function

  */
  if (currentTime - delayLog > 250)
  {
    sensor.log();

    // Serial.print("mode: ");
    // Serial.println(mode);
    Serial.print("distance: ");
    Serial.println(range);

    // Serial.print("buf: ");
    // Serial.print(buf[0]);
    // Serial.println(buf[1]);

    Serial.println(" ");
    Serial.println(" ");

    delayLog = currentTime;
  }

  /*

  End Log Function

  */
}
