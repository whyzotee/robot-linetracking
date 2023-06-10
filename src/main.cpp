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

enum Status
{
  red,
  yellow,
  green
};

Status status = red;

bool P1 = false;
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

void setup()
{
  Serial.begin(115200);

  sensor.begin();
  motor.begin(255, 255, 100);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // for (uint8_t i = 1; i <= 3; i++)
  // {
  //   display.clearDisplay();
  //   display.setTextSize(1);
  //   display.setTextColor(SSD1306_WHITE);
  //   display.setCursor(0, 0);
  //   display.println("Hello World!");
  //   display.println("Robot By EN CMTC");
  //   display.println("");
  //   display.print("Start Robot in: ");
  //   display.println(4 - i);
  //   display.setCursor(0, 0);
  //   display.display();
  //   delay(1000);
  // }
}

void loop()
{
  motor.move(255, "sleft");
  delay(3500);
  motor.move(255, "sright");
  delay(3500);

  // CurrentTimeLine
  currentTime = millis();

  // Start Mode
  P1 = digitalRead(24);

  // Read Sensor From Class
  sensor.read();

  // Status in monitor
  // displayStatus(P1);

  // Status of LED
  // switch (status)
  // {
  // case red:
  //   digitalWrite(0, HIGH);
  //   break;
  // case yellow:
  //   lightBlink(0);
  //   break;
  // case green:
  //   lightBlink(0);
  //   break;
  // }

  // if (P1 && isMove)
  // {
  //   motor.balance_move(isFront);
  //   status = green;
  // }

  // if (P1 && !isMove)
  // {
  //   switch (quest)
  //   {
  //   case 1:
  //     /* code */
  //     break;

  //   default:
  //     break;
  //   }
  // }

  if (currentTime - delayLog > 250)
  {
    sensor.log();
    delayLog = currentTime;
  }
}
