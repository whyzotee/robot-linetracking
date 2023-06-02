#include "movement.h"

// Extend To Array { speed, speed - 5, speed, speed - 5} max value's 255
byte speed = 255;

// value sensor is in black line
int black_value = 200;

// Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
byte sensor_front[] = { 54, 55, 56, 57, 58 };

// Sensor Tracking Position -> { a5  a6  a7  ?  a9 }
byte sensor_center[] = { 60, 0, 61, 59, 63 };

// Sensor Value
short a0_value, a1_value, a2_value, a3_value, a4_value;
short a5_value, a6_value, a7_value, a8_value, a9_value;

bool isMove = true;
bool isBack = false;
bool isSoi = false;
bool fake_left = false;

// Delay Time
unsigned long logTime = 0;
unsigned long delayTurn = 0;
unsigned long delaybeforeTurn = 0;

int cross = 0;
int right_count = 0;
int soi_count = 0;

void setup() {
  Serial.begin(115200);

  move_setup();

  for (byte i = 0; i < 5; i++) {
    pinMode(sensor_front[i], INPUT);
    pinMode(sensor_center[i], INPUT);
  }

  // Move Out From Start Point
  move(speed, "front");
  delay(500);
}

void loop() {
  sensor_read();

  // CurrentTimeLine
  unsigned long currentTime = millis();

  if (isMove) {
    if (!isBack)
      balance_move();
    else
      balance_move_back();
  }


  if (!isMove) {
    if (cross == 0) {
      if (currentTime - delayTurn > 800) {
        if (a2_value > black_value || a3_value > black_value) {
          isMove = true;
          delayTurn = 0;
          cross += 1;
        }
      }
      move(speed, "left");
    } else if (cross == 1) {
      // if (currentTime - delayTurn > 1000) {
      //   if (a2_value > black_value || a3_value > black_value) {
      //     if(right_count == 0) {
      //       isMove = true;
      //       delayTurn = 0;
      //     }
      //   }
      // }
      // move(speed, "left");
      while (true) {
        sensor_read();
        move(speed, "left");
        if (a2_value > black_value && a3_value > black_value) {
          if (fake_left) {
            isMove = true;
            delayTurn = 0;
            cross += 1;
            break;
          }
          fake_left = true;
          delay(50);
        }
      }
    }

    if (right_count == 4 && soi_count == 0) {
      if (currentTime - delayTurn > 800) {
        if (a2_value > black_value || a3_value > black_value) {
          isSoi = true;
          isMove = true;
          delayTurn = 0;
        }
      }
      move(speed, "right");
    }

    if (soi_count == 1) {
      if (currentTime - delayTurn > 800) {
        if (a2_value > black_value || a3_value > black_value) {
          isMove = true;
          delayTurn = 0;
        }
      }
      Serial.println(currentTime - delayTurn > 800);

      move(speed, "right");
    }
  }

  if (soi_count == 1 && isMove) {
    if (a1_value > black_value || a2_value > black_value || a3_value > black_value) {
      delay(500);
      if (delayTurn == 0) {
        isMove = false;
        delayTurn = currentTime;
      }
    }
  }

  // เลี้ยวเข้าซอยสุดท้าย
  if (cross == 2 && right_count <= 4) {
    if (a2_value > black_value && a3_value > black_value && a4_value > black_value) {
      right_count += 1;
      if (right_count == 4 && delayTurn == 0) {
        if (delayTurn == 0) {
          delay(500);
          isMove = false;
          delayTurn = currentTime;
        }
      }
    }
  }

  if (isCenter()) {
    if (cross == 0) {
      delay(500);
      isMove = false;
      delayTurn = currentTime;
    } else if (cross == 1) {
      delay(300);
      if (delayTurn == 0) {
        isMove = false;
        delayTurn = currentTime;
      }
    }
  }

  if (currentTime - logTime > 250) {
    //log_sensor();
    logTime = currentTime;
  }

  delay(50);
}

bool isCenter() {
  return (a0_value > black_value && a1_value > black_value && a2_value > black_value && a3_value > black_value && a4_value > black_value);
}

void balance_move() {
  // Apply the control output to the movement
  if (a2_value > black_value) {
    move(speed, "front");
  } else if (a0_value > black_value || a1_value > black_value) {
    move(speed, "left");
  } else if (a3_value > black_value || a4_value > black_value) {
    move(speed, "right");
  } else {
    if (isSoi) {
      move(0, "stop");
      delay(1000);
      move(speed, "back");
      delay(500);
      isBack = true;
    } else {
      move(0, "stop");
    }
  }
}

void balance_move_back() {
  while (isSoi) {
    sensor_read();
    if (a2_value > black_value) {
      move(speed, "back");
    } else if (a1_value > black_value) {
      move(speed, "right");
      delay(100);
      move(speed, "back");
      delay(100);
    } else if (a3_value > black_value) {
      move(speed, "left");
      delay(100);
      move(speed, "back");
      delay(100);
    } else if (a0_value > black_value) {
      move(speed, "left");
      delay(100);
      move(speed, "back");
      delay(100);
    } else if (a4_value > black_value) {
      move(speed, "right");
      delay(100);
      move(speed, "back");
      delay(100);
    } else {
      move(speed, "front");
      delay(300);
      isSoi = false;
      isBack = false;
      soi_count += 1;
      break;
    }
    delay(50);
  }

  if (a2_value > black_value) {
    move(speed, "back");
    Serial.println("real back");
  } else if (a1_value > black_value) {
    move(speed, "back_right");
    delay(100);
    move(speed, "back");
    delay(100);
  } else if (a3_value > black_value) {
    move(speed, "back_left");
    delay(100);
    move(speed, "back");
    delay(100);
  } else if (a0_value > black_value) {
    move(speed, "back_left");
    delay(100);
    move(speed, "back");
    delay(100);
  } else if (a4_value > black_value) {
    move(speed, "back_right");
    delay(100);
    move(speed, "back");
    delay(100);
  } else {
    move(0, "stop");
  }
}

void sensor_read() {
  a0_value = analogRead(sensor_front[0]);
  a1_value = analogRead(sensor_front[1]);
  a2_value = analogRead(sensor_front[2]);
  a3_value = analogRead(sensor_front[3]);
  a4_value = analogRead(sensor_front[4]);
}

void log_sensor() {
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
  Serial.println("");
  Serial.print(" cross: ");
  Serial.print(cross);
  Serial.print(" right_count: ");
  Serial.print(right_count);
  Serial.println("");
  Serial.println("");
  Serial.println("");
}
