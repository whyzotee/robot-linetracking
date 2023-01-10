#include "movement.h"
#include <elapsedMillis.h>

// Extend To Array { speed, speed - 5, speed, speed - 5} max value's 255
byte speed = 127;

// Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
byte sensor[] = {54, 55, 56, 57, 58};

// Sensor Value
byte a0_value, a1_value, a2_value, a3_value, a4_value;

// Check Move the Robot
bool isMove = true;

// Check Robot's Turning Left or Right
bool isTurn = false;

// Check How Many Robot Go to Crossroad
byte cross_count = 0;

// Test Out loop Delay Time
elapsedMillis x;

void setup()
{
  Serial.begin(115200);

  move_setup();

  for (byte i = 0; i < 5; i++)
    pinMode(sensor[i], INPUT);

  // Move Out From Start Point
  move(speed, "front");
  delay(1000);
}

void loop()
{
  a0_value = analogRead(sensor[0]);
  a1_value = analogRead(sensor[1]);
  a2_value = analogRead(sensor[2]);
  a3_value = analogRead(sensor[3]);
  a4_value = analogRead(sensor[4]);

  // function move(Speed, Direction)

  if (isCenter(a0_value, a1_value, a2_value, a3_value, a4_value))
    if (isMove && cross_count < 2)
    {
      isTurn = true;
      isMove = false;
    }

  if (isTurn)
  {
    Serial.println("Turning Now!");
    delay(500);

    switch (cross_count)
    {
    case 0:
      Serial.println("Now Robot's Turn Right");
      move(speed, "right");
      delay(500);
      if (a2_value > 200 && a3_value > 200)
      {
        cross_count += 1;
        isTurn = false;
        isMove = true;
      }
      break;
    case 1:
      Serial.println("Now Robot's Turn Left");
      move(speed, "left");
      delay(500);
      if (a1_value > 200 && a2_value > 200)
      {
        // cross_count += 1;
        isTurn = false;
        isMove = true;
      }
      break;
    default:
      break;
    }
  }

  if (!isTurn)
  {
    balance_move();
    log_sensor();
  }

  delay(50);
}

bool isCenter(int a0, int a1, int a2, int a3, int a4)
{
  if (a0 > 200 && a1 > 200 && a2 > 200 && a3 > 200 && a4 > 200)
    return true;
  else
    return false;
}

void balance_move()
{
  if (a2_value > 200 && isMove)
    move(speed, "front");
  else if (a1_value > 200 && isMove)
    move(speed, "left");
  else if (a3_value > 200 && isMove)
    move(speed, "right");
  else
  {
    move(speed, "back");
    delay(1000);
  }
}

void log_sensor()
{
  Serial.print("A0: ");
  Serial.print(a0_value);
  Serial.print(" A1: ");
  Serial.print(a1_value);
  Serial.print(" A2: ");
  Serial.print(a2_value);
  Serial.print(" A3: ");
  Serial.print(a3_value);
  Serial.print(" A4: ");
  Serial.print(a4_value);
  Serial.print(" Is Move: ");
  Serial.print(isMove);
  Serial.print(" Center: ");
  Serial.println(center);
}