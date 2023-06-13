// #include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <Stepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_SSD1306.h>

#include "arm.h"
#include "motor.h"
#include "sensor.h"

#define OLED_RESET -1
#define EMERGENCY_PIN 3
#define STEPPER_SW 32
#define LED_STATUS_RED 28
#define LED_STATUS_YELLOW 29
#define LED_STATUS_GREEN 30

const int stepsPerRevolution = 200;

Stepper motorStepper(stepsPerRevolution, 34, 35, 36, 37);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire, OLED_RESET);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

VL53L0X_RangingMeasurementData_t measure;
u16 range;

Arm arm;
Sensor sensor;
Motor motor;

// Controller Time
uint64_t currentTime = 0;
// Delay Time
uint64_t delayLog = 0;
uint64_t delayLight = 0;
uint64_t delayTurn = 0;
uint64_t delayCross = 0;
uint64_t delayGoToCenter = 0;
uint64_t delayNotFoundSensor = 0;

char buf[2];

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
String mode;

bool P1 = false;
bool servoInit = true;
bool isMove = true;
bool isFront = true;
bool isSoi = false;
bool isObject = true;
bool stepperSW = false;
bool emergencyBTN = false;

void displayStatus(bool Power)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Power: ");
  display.println(Power ? "on" : "off");
  display.print("isMove: ");
  display.println(isMove ? "true" : "false");
  display.print("PID Error: ");
  display.println(sensor.error);
  display.print("Now Quest: ");
  display.println(quest);
  display.display();
}

void balance_move(char LOR)
{
  delayNotFoundSensor = currentTime;
  if (isFront)
  {
    if (sensor.s2)
      motor.move(255, "front");
    else if ((sensor.s0) || (sensor.s1))
      motor.move(255, "left");
    else if ((sensor.s3) || (sensor.s4))
      motor.move(255, "right");
    // else
    //   motor.move(50, "front");
    delayNotFoundSensor = 0;
  }
  else
  {
    if (sensor.isSomeBlack())
    {
      motor.move(255, "back");
      delayNotFoundSensor = 0;
    }
    else
    {
      if (currentTime - delayNotFoundSensor >= 300)
      {
        if (isSoi)
        {
          motor.move(255, "front");
          delay(750);
          if (LOR == 'L')
            motor.move(255, "left");
          if (LOR == 'R')
            motor.move(255, "right");
          delay(1500);

          isFront = true;
          finish_soi_count += 1;
        }
        else
        {
          motor.move(0, "stop");
        }
        delayNotFoundSensor = 0;
      }
      motor.move(255, "back");
    }
  }
}

void gotoCenter()
{
  if (delayGoToCenter == 0)
    delayGoToCenter = currentTime;

  if (currentTime - delayGoToCenter > 3000)
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

void gotoOBJ(char direction)
{
  if (direction == 'L')
  {
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1500)
    {
      if (sensor.s2 || sensor.s3)
      {
        motor.move(255, "stop");
        arm.centerArm();
        for (uint8_t i = 0; i < 5; i++)
        {
          motorStepper.step(-stepsPerRevolution);
        }
        mode = "OBJ";
        delayTurn = 0;
      }
    }

    motor.move(255, "left");
  }

  if (direction == 'R')
  {
    if (delayTurn == 0)
      delayTurn = currentTime;

    if (currentTime - delayTurn > 1500)
    {
      if (sensor.s2 || sensor.s3)
      {
        mode = "OBJ";
        delayTurn = 0;
      }
    }

    motor.move(255, "right");
  }
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
    balance_move(' ');
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
      if (currentTime - delayCross > 750)
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
    balance_move(' ');
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

    if (currentTime - delayGoToCenter > 1200)
    {
      motor.move(0, "stop");
      delay(200);
      arm.getObj();
      delay(1000);
      for (uint8_t i = 0; i < 5; i++)
      {
        motorStepper.step(stepsPerRevolution);
      }
      motor.move(0, "back");
      delay(200);
      quest += 1;
      isFront = false;
      delayTurn = 0;
    }

    balance_move(' ');
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 4750)
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
      mode = "Y2";
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
      if (currentTime - delayCross > 750)
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
    balance_move('\0');
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
      if (sensor.s2)
      {
        quest += 1;
        isSoi = true;
        isFront = false;
        delayGoToCenter = 0;
      }
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
      if (currentTime - delayCross > 750)
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
    balance_move('\0');
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
      if (sensor.s2)
      {
        quest += 1;
        isSoi = true;
        isFront = false;
        delayGoToCenter = 0;
      }
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
        quest = 3;
        delayGoToCenter = 0;
      }
    }
    motor.move(255, "sleft");
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
    balance_move('\0');
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
      if (sensor.isSomeBlack())
      {
        motor.move(255, "front");
        delay(500);
        quest = 6;
        delayGoToCenter = 0;
      }
    }

    balance_move('\0');
    break;
  case 6:
    if (delayGoToCenter == 0)
      delayGoToCenter = currentTime;

    if (currentTime - delayGoToCenter > 500)
    {
      quest = 7;
      delayGoToCenter = 0;
    }
    motor.move(255, "sleft");
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
    motor.move(255, "sright");
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
    balance_move('\0');
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
      if (sensor.isSomeBlack())
      {
        motor.move(255, "front");
        delay(300);
        quest = 6;
        delayGoToCenter = 0;
      }
    }

    balance_move('\0');
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
      if (currentTime - delayCross > 750)
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
    balance_move('\0');
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
      if (sensor.s2)
      {
        quest += 1;
        isSoi = true;
        isFront = false;
        delayGoToCenter = 0;
      }
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
      if (currentTime - delayCross > 750)
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
    balance_move('\0');
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
      if (sensor.s2)
      {
        quest += 1;
        isSoi = true;
        isFront = false;
        delayGoToCenter = 0;
      }
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
      if (currentTime - delayCross > 750)
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
    balance_move('\0');
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
      if (sensor.s2)
      {
        quest += 1;
        isSoi = true;
        isFront = false;
        delayGoToCenter = 0;
      }
    }

    balance_move('\0');
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

void liftUp()
{
  for (uint8_t i = 0; i < 3; i++)
  {
    motorStepper.step(stepsPerRevolution);
  }
  stepperSW = true;
}

void liftDown()
{
  for (uint8_t i = 0; i < 5; i++)
  {
    motorStepper.step(-stepsPerRevolution);
  }
  stepperSW = true;
}

void setup()
{
  Serial.begin(115200);

  arm.begin();
  sensor.begin();
  motor.begin(255, 255, 100);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  motorStepper.setSpeed(250);

  // pinMode(23, OUTPUT);
  // pinMode(25, OUTPUT);
  // pinMode(27, OUTPUT);
  // pinMode(29, OUTPUT);
  // pinMode(31, OUTPUT);

  pinMode(EMERGENCY_PIN, INPUT);
  pinMode(STEPPER_SW, INPUT_PULLUP);
  pinMode(LED_STATUS_RED, OUTPUT);
  pinMode(LED_STATUS_YELLOW, OUTPUT);
  pinMode(LED_STATUS_GREEN, OUTPUT);

  digitalWrite(LED_STATUS_RED, LOW);
  digitalWrite(LED_STATUS_YELLOW, HIGH);
  digitalWrite(LED_STATUS_GREEN, HIGH);

  // for (uint8_t i = 0; i < 3; i++)
  // {
  //   display.clearDisplay();
  //   display.setTextSize(1);
  //   display.setTextColor(SSD1306_WHITE);
  //   display.setCursor(0, 0);
  //   display.println("Hello World!");
  //   display.println("Robot By EN CMTC");
  //   display.println("");
  //   display.print("Start Robot in: ");
  //   display.println(3 - i);
  //   display.setCursor(0, 0);
  //   display.display();
  //   delay(1000);
  // }

  //  if (!lox.begin())
  // {
  //   Serial.println(F("Failed to boot VL53L0X"));
  //   return;
  // }

  for (uint8_t i = 0; i < 4; i++)
  {
    motorStepper.step(stepsPerRevolution);
  }

  // mode go to center
  mode = "GTCT";
}

void loop()
{
  //  Serial1.readBytes(buf, 2);

  // if (Serial1.available())
  // {
  //   Serial.print(buf);
  // }

  // lox.rangingTest(&measure, false);

  // if (measure.RangeStatus != 4)
  //   range = measure.RangeMilliMeter;
  // else
  //   range = 0;

  // CurrentTimeLine
  currentTime = millis();

  // Start Mode
  P1 = digitalRead(24);
  emergencyBTN = digitalRead(EMERGENCY_PIN);

  // Read Sensor From Class
  sensor.read();

  // Status in monitor
  displayStatus(P1);

  // digitalWrite(23, !sensor.s0);
  // digitalWrite(25, !sensor.s1);
  // digitalWrite(27, !sensor.s2);
  // digitalWrite(29, !sensor.s3);
  // digitalWrite(31, !sensor.s4);

  if (emergencyBTN)
    status = yellow;

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

  if (!emergencyBTN && isMove)
  {
    status = green;
    if (mode == "GTCT")
      gotoCenter();
    if (mode == "CT_CROSS")
      gotoOBJ('L');

    // เก็บกล่อง
    if (mode == "OBJ")
      first("");
    if (mode == "FINISH_1")
      second("");
    if (mode == "FINISH_2")
      second("");
    if (mode == "FINISH_3")
      third("");
    if (mode == "FINISH_4")
      fourth("");
    if (mode == "FINISH_5")
      fifth("");
    if (mode == "FINISH_6")
      sixth("");
    if (mode == "FINISH_7")
      seventh("");
    if (mode == "FINISH_8")
      eighth("");

    // วางกล่อง
    if (mode == "Y2")
      first("FINISH_1");
    if (mode == "B2")
      second("FINISH_2");
    if (mode == "G2")
      third("FINISH_3");
    if (mode == "R2")
      fourth("FINISH_4");
    if (mode == "R1")
      fifth("FINISH_5");
    if (mode == "G1")
      sixth("FINISH_6");
    if (mode == "B1")
      seventh("FINISH_7");
    if (mode == "Y1")
      eighth("FINISH_8");
  }

  if (currentTime - delayLog > 250)
  {
    sensor.log();
    Serial.print("mode: ");
    Serial.println(mode);
    Serial.print("emergencyBTN: ");
    Serial.println(emergencyBTN);
    delayLog = currentTime;
  }
}
