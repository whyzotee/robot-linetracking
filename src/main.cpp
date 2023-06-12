// #include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "motor.h"
#include "sensor.h"

#define OLED_RESET -1

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire, OLED_RESET);

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

Status status = red;

String mode;

bool P1 = false;
bool isMove = true;
bool isFront = true;
bool isSoi = false;
bool isObject = true;

void lightBlink(uint8_t pin)
{
  if (currentTime - delayLight > 250)
  {
    digitalWrite(pin, HIGH);
    delayLight = currentTime;
  }
  digitalWrite(pin, LOW);
}

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

void balance_move()
{
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
  }
  else
  {
    if (sensor.s0 || sensor.s1 || sensor.s2 || sensor.s3 || sensor.s4)
      motor.move(255, "back");
    // else if (sensor.s1 || sensor.s4)
    //   motor.move(255, "back");
    // else if (sensor.s0 || sensor.s3)
    //   motor.move(255, "back");
    else
    {
      if (isSoi)
      {
        motor.move(255, "front");
        delay(500);
        motor.move(255, "right");
        delay(1500);

        isFront = true;
        finish_soi_count += 1;
      }
      else
        motor.move(0, "stop");
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
    balance_move();
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
    balance_move();
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

    balance_move();
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

    balance_move();
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

    balance_move();
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
    balance_move();
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
    balance_move();
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

    balance_move();
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

    balance_move();
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

    balance_move();
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
    balance_move();
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
            quest += 1;
            soi_count = 0;
            delayCross = 0;
            delayGoToCenter = 0;
          }
        }
      }
    }
    balance_move();
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

    balance_move();
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

    balance_move();
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

    balance_move();
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  sensor.begin();
  motor.begin(255, 255, 100);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pinMode(23, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(29, OUTPUT);
  pinMode(31, OUTPUT);

  for (uint8_t i = 0; i < 3; i++)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Hello World!");
    display.println("Robot By EN CMTC");
    display.println("");
    display.print("Start Robot in: ");
    display.println(3 - i);
    display.setCursor(0, 0);
    display.display();
    delay(1000);
  }

  // mode go to center
  mode = "GTCT";
}

void loop()
{
  // Serial1.readBytes(buf, 2);

  // if (Serial1.available())
  // {
  //   Serial.print(buf);
  // }

  // CurrentTimeLine
  currentTime = millis();

  // Start Mode
  P1 = digitalRead(24);

  // Read Sensor From Class
  sensor.read();

  // Status in monitor
  displayStatus(P1);

  digitalWrite(23, !sensor.s0);
  digitalWrite(25, !sensor.s1);
  digitalWrite(27, !sensor.s2);
  digitalWrite(29, !sensor.s3);
  digitalWrite(31, !sensor.s4);

  // Status of LED
  switch (status)
  {
  case red:
    digitalWrite(0, HIGH);
    break;
  case yellow:
    lightBlink(0);
    break;
  case green:
    lightBlink(0);
    break;
  }

  // if (P1 && isMove)
  if (isMove)
  {
    if (mode == "GTCT")
      gotoCenter();
    if (mode == "CT_CROSS")
      gotoOBJ('L');
    if (mode == "OBJ")
      // obj1 แล้วในฟังก์ชันอ่านค่าได้ค่อยแทนด้วยค่า
      first("Y1");
    if (mode == "Y1")
      first("FY1");
    if (mode == "FY1")
      second("B2");
    if (mode == "B2")
      second("FB2");
    if (mode == "FB2")
      third("G2");
    if (mode == "G2")
      third("FG2");
    if (mode == "FG2")
      motor.move(0, "stop");
  }

  if (currentTime - delayLog > 250)
  {
    sensor.log();
    Serial.print("mode: ");
    Serial.println(mode);
    Serial.print("soi_count: ");
    Serial.println(soi_count);
    delayLog = currentTime;
  }
}
