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

byte quest = 0;
byte soi_count = 0;
byte finish_soi_count = 0;

enum Status
{
  red,
  yellow,
  green
};

Status status = red;

bool Power = false;
bool isMove = true;
bool isFront = true;
bool isSoi = false;

void lightBlink(uint8_t pin)
{
  if (currentTime - delayLight > 250)
  {
    digitalWrite(pin, HIGH);
    delayLight = currentTime;
  }
  digitalWrite(pin, LOW);
}

void displayStatus()
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

void setup()
{
  Serial.begin(115200);

  sensor.begin();
  motor.begin(255, 255, 100);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  for (uint8_t i = 1; i <= 3; i++)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Hello World!");
    display.println("Robot By EN CMTC");
    display.println("");
    display.print("Start Robot in: ");
    display.println(4 - i);
    display.setCursor(0, 0);
    display.display();
    delay(1000);
  }
}

void loop()
{
  displayStatus();

  // CurrentTimeLine
  currentTime = millis();

  // Read Sensor From Class
  sensor.read();

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

  if (Power && isMove)
  {
    motor.balance_move(isFront);
    status = green;
  }

  if (Power && !isMove)
  {
    if (quest == 0)
    {
      if (currentTime - delayTurn > 800)
      {
        if (sensor.s2 || sensor.s3)
        {
          isMove = true;
          delayTurn = 0;
          quest += 1;
        }
      }
      motor.move(255, "left");
    }
    else if (quest == 1)
    {
      if (currentTime - delayTurn > 800)
      {
        if (sensor.s1 || sensor.s2)
        {
          soi_count += 1;
          delayTurn = currentTime;
          if (soi_count == 4)
          {
            isSoi = true;
            isMove = true;
            soi_count = 0;
            delayTurn = 0;
            quest += 1;
          }
        }
      }
      motor.slide('L');
    }
    else if (quest == 2 && isFront)
    {
      if (currentTime - delayTurn > 800)
      {
        if (sensor.s1 || sensor.s2)
        {
          isMove = true;
          delayTurn = 0;
          quest += 1;
        }
      }
      motor.move(255, "right");
    }
    else if (quest == 3)
    {
      if (currentTime - delayTurn > 800)
      {
        if (sensor.s2 || sensor.s3)
        {
          isMove = true;
          delayTurn = 0;
          quest += 1;
        }
      }
      motor.move(255, "right");
    }
    else if (quest == 5)
    {
      if (currentTime - delayTurn > 800)
      {
        if (sensor.s1 || sensor.s2)
        {
          soi_count += 1;
          delayTurn = currentTime;
          if (soi_count == 4)
          {
            isSoi = true;
            isMove = true;
            soi_count = 0;
            delayTurn = 0;
            quest += 1;
          }
        }
      }
      motor.slide('L');
    }
    else if (quest == 6)
    {
      if (currentTime - delayTurn > 800)
      {
        if (sensor.s2 || sensor.s3)
        {
          isMove = true;
          delayTurn = 0;
          quest += 1;
        }
      }
      motor.move(255, "right");
    }
  }

  if (sensor.isCenter())
  {
    if (quest == 0)
    {
      delay(400);
      if (delayTurn == 0)
      {
        isMove = false;
        delayTurn = currentTime;
      }
    }
    else if (quest == 1)
    {
      delay(300);
      if (delayTurn == 0)
      {
        isMove = false;
        delayTurn = currentTime;
      }
    }
    else if (quest == 4)
    {
      delay(200);
      motor.balance_move(true);
      quest += 1;
    }
    else if (quest == 5)
    {
      delay(200);
      motor.move(255, "right");
      delay(100);
      if (delayTurn == 0)
      {
        isMove = false;
        delayTurn = currentTime;
      }
    }
  }

  if (sensor.isRightCross())
  {
    if ((quest == 2 || quest == 3 || quest == 6) && finish_soi_count == 1)
    {
      delay(500);
      if (delayTurn == 0)
      {
        isMove = false;
        delayTurn = currentTime;
      }
    }
  }

  if (currentTime - delayLog > 250)
  {
    sensor.log();
    delayLog = currentTime;
  }
}
