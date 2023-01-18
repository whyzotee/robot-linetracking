#include "movement.h"

// Extend To Array { speed, speed - 5, speed, speed - 5} max value's 255
byte speed = 255;

// Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
byte sensor[] = {54, 55, 56, 57, 58};

// Sensor Value
int a0_value, a1_value, a2_value, a3_value, a4_value;

// Check Move the Robot
bool isMove = true;

// Check Robot's Turning Left or Right
bool isTurn = false;

bool checksome = false;
// Check How Many Robot Go to Crossroad
byte cross_count = 0;

// Delay Time
unsigned long prevTime = 0;
unsigned long logTime = 0;
unsigned long delayCenter = 0;

void setup()
{
  Serial.begin(115200);

  move_setup();

  for (byte i = 0; i < 5; i++)
  {
    pinMode(sensor[i], INPUT);
  }

  // Move Out From Start Point
  move(speed, 1);
  delay(500);
}

void loop()
{
  a0_value = analogRead(sensor[0]);
  a1_value = analogRead(sensor[1]);
  a2_value = analogRead(sensor[2]);
  a3_value = analogRead(sensor[3]);
  a4_value = analogRead(sensor[4]);

  // CurrentTimeLine
  unsigned long currentTime = millis();

  /*
  function move(TestRun, Speed, Direction)
  Direction Position 0 = Break Move, 1 = Move Front,
  2 = Move Left, 3 = Move Right, 4 = Move Back,
  200 = Test Run
  */

  if (isCenter())
  {
    if (isMove && cross_count < 2)
    {
      isMove = false;
    }
  }

  if (!isMove && !isTurn)
  {
    if (prevTime == 0)
    {
      prevTime = currentTime;
    }

    if (currentTime - prevTime > 500)
    {
      isTurn = true;
    }
  }

  if (isTurn)
  {
    switch (cross_count)
    {
    case 0:
      move(speed, 3);

      if (delayCenter == 0)
      {
        delayCenter = currentTime;
      }
      if (currentTime - delayCenter > 250)
      {
        if (a3_value > 200 || a2_value > 200)
        {
          cross_count += 1;
          isMove = true;
          isTurn = false;
          prevTime = 0;
          delayCenter = 0;
        }
      }
      break;
    case 1:
      move(speed, 2);

      if (delayCenter == 0)
      {
        delayCenter = currentTime;
      }
      if (currentTime - delayCenter > 800)
      {
        if (a1_value > 200 || a2_value > 200)
        {
          cross_count += 1;
          isMove = true;
          isTurn = false;
          prevTime = 0;
          delayCenter = 0;
        }
      }
    }
  }

  if (isMove == true && isTurn == false)
  {
    balance_move();
    if (currentTime - logTime > 500)
    {
      log_sensor();
      logTime = currentTime;
    }
  }
  delay(30);
}

bool isCenter()
{
  if (a0_value > 400 && a1_value > 400 && a2_value > 400 && a3_value > 400 && a4_value > 400)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void balance_move()
{
  if (a2_value > 250 && isMove)
  {
    move(speed, 1);
  }

  else if (a1_value > 250 && isMove)
  {
    move(speed, 2);
  }

  else if (a3_value > 250 && isMove)
  {
    move(speed, 3);
  }
  else
  {
    move(speed, 0);
    // isMove = false;
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
  Serial.println(cross_count);
}