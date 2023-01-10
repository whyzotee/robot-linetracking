#include "movement.h"
#include <elapsedMillis.h>

// speed_fl, speed_fr, speed_bl, speed_br
// byte speed[] = {127, 120, 127, 120};
// test
byte speed[] = {254, 254, 254, 254};
// Sensor tracking position -> { a0  a1  a2  a3  a4 }
byte sensor[] = {54, 55, 56, 57, 58};
// Move the Robot
bool isMove = true;

int a0_value, a1_value, a2_value, a3_value, a4_value;

byte center = 0;

elapsedMillis x;

void setup()
{
  Serial.begin(115200);

  move_setup();

  for (byte i = 0; i < 5; i++)
    pinMode(sensor[i], INPUT);

  while (true)
  {
    move(false, speed, 1);
    delay(1000);
    break;
  }
}

void loop()
{
  // move(TestRun, Speed, Direction)
  // Direction Position 1 = Move Front, 2 = Move Left
  // 3 = Move Right, 4 = Move Back, 101 = Break Move

  // Test Run
  // move(true, speed, 0);

  a0_value = analogRead(sensor[0]);
  a1_value = analogRead(sensor[1]);
  a2_value = analogRead(sensor[2]);
  a3_value = analogRead(sensor[3]);
  a4_value = analogRead(sensor[4]);

  // if (Serial.available())
  // {
  //   isMove = Serial.read();

  //   if (isMove = 1)
  //   {
  //     isMove = true;
  //     Serial.println("ON");
  //   }
  //   if (isMove = 0)
  //   {
  //     isMove = false;
  //     Serial.println("OFF");
  //   }
  // }
  if (a0_value > 200 && a1_value > 200 && a2_value > 200 && a3_value > 200 && a4_value > 200 && isMove && center <= 2)
  {
    delay(500);
    while (center == 1)
    {
      move(false, speed, 2);
      delay(1000);

      if (a1_value > 200 && a2_value > 200)
      {
        center += 1;
        break;
      }
    }

    while (center == 0)
    {
      move(false, speed, 3);
      delay(1000);

      if (a3_value > 200 && a2_value > 200)
      {
        center += 1;
        break;
      }
    }
  }

  // if (a0_value > 200 && a1_value > 200 && a2_value > 200 && a3_value > 200 && a4_value > 200 && isMove)
  // {
  //   // move(false, speed, 101);
  //   // delay(500);
  //   // center += 1;
  //   move(false, speed, 1);
  //   delay(700);
  //   while (true)
  //   {
  //     move(false, speed, 2);
  //     // a1_value < 200 || a3_value < 200
  //     if (a2_value > 200 || a1_value > 200)
  //     {
  //       delay(1500);
  //       break;
  //     }
  //   }
  // }

  // if (a0_value > 200 && a1_value > 200 && a2_value && isMove)
  // {
  //   move(false, speed, 1);
  //   delay(1200);
  //   while (true)
  //   {
  //     move(false, speed, 2);
  //     // a1_value < 200 || a3_value < 200
  //     if (a2_value > 200 || a1_value > 200)
  //     {
  //       delay(1300);
  //       break;
  //     }
  //   }
  // }

  // if (a0_value > 200 && a1_value > 200 && a2_value > 200 && isMove)
  // {
  //   while (true)
  //   {
  //     move(false, speed, 4);
  //     if (a0_value > 200 && a1_value > 200 && a2_value > 200 && a3_value > 200 && a4_value > 200 && isMove)
  //     {
  //       delay(1000);
  //       break;
  //     }
  //   }
  // }

  if (a2_value > 200 && isMove)
  {
    move(false, speed, 1);
  }
  else if (a1_value > 200 && isMove)
  {
    move(false, speed, 2);
  }
  else if (a3_value > 200 && isMove)
  {
    move(false, speed, 3);
  }
  // else if (a0_value > 200 && isMove)
  // {
  //   move(false, speed, 3);
  // }
  // else if (a4_value > 200 && isMove)
  // {
  //   move(false, speed, 2);
  // }
  else
  {
    move(false, speed, 101);
    delay(1000);
  }

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

  delay(30);
}