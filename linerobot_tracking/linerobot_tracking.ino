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
int a0_value, a1_value, a2_value, a3_value, a4_value;
int a5_value, a6_value, a7_value, a8_value, a9_value;

bool isMove = true;
bool isBack = false;

// Delay Time
unsigned long prevTime = 0;
unsigned long logTime = 0;
unsigned long delayTurn = 0;

// Variables for PID control
double integral = 0.0;
double previousError = 0.0;

int cross = 0;
int left_count = 0;
int object = 0;


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
  a0_value = analogRead(sensor_front[0]);
  a1_value = analogRead(sensor_front[1]);
  a2_value = analogRead(sensor_front[2]);
  a3_value = analogRead(sensor_front[3]);
  a4_value = analogRead(sensor_front[4]);
  
  
  a5_value = analogRead(sensor_center[0]);
  a6_value = analogRead(sensor_center[1]);
  a7_value = analogRead(sensor_center[2]);
  a8_value = analogRead(sensor_center[3]);
  a9_value = analogRead(sensor_center[4]);

  // CurrentTimeLine
  unsigned long currentTime = millis();

  if (isMove) {
    if (!isBack)
      balance_move();
    if (isBack)
      balance_move_back();
  }

  if (!isMove) {
    if (cross == 1 || cross == 2) {
      if (currentTime - delayTurn > 800) {
        if (a2_value > black_value || a3_value > black_value) {
          isMove = true;
          delayTurn = 0;
        }
      }
      move(speed, "left");
    }
    if (cross == 6) {
      if (currentTime - delayTurn > 800) {
        if (a2_value > black_value || a3_value > black_value) {
          isMove = true;
          delayTurn = 0;
        }
      }
      move(speed, "right");
    }
  }

  if (isCenter()) {
    if (cross == 0 && delayTurn == 0) {
      // delay(500);
      Serial.println("ok");
      isMove = false;
      delayTurn = currentTime;
      cross += 1;
    } else if (cross == 1 && delayTurn == 0) {
      // delay(500);
      if (delayTurn == 0) {
        isMove = false;
        delayTurn = currentTime;
        cross += 1;
      }
    }
  }

  if (a2_value > black_value && a3_value > black_value && a4_value > black_value) {
    if (cross == 6 && delayTurn == 0) {
      // delay(500);
      if (delayTurn == 0) {
        isMove = false;
        delayTurn = currentTime;
        cross += 1;
      }
    }
    cross += 1;
  }

  if (currentTime - logTime > 250) {
    log_sensor();
    logTime = currentTime;
  }

  delay(50);
}

bool isCenter() {
  return (a5_value > black_value && a7_value > black_value && a9_value > black_value);
  //return (a0_value && a1_value && a2_value && a3_value && a4_value);
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
    move(0, "stop");
    isMove = false;
  }
}

void balance_move_back() {
  // Apply the control output to the movement
  if (a2_value > black_value) {
    move(speed, "back");
  } else if (a0_value > black_value && a1_value > black_value) {
    move(speed, "left");
  } else if (a3_value > black_value && a4_value > black_value) {
    move(speed, "right");
  } else if (a0_value > black_value) {
    move(speed, "left");
  } else if (a4_value > black_value) {
    move(speed, "right");
  } else {
    move(0, "stop");
    isMove = false;
  }
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
  Serial.print("A5: ");
  Serial.print(a5_value);
  Serial.print(" A6: ");
  Serial.print(a6_value);
  Serial.print(" A7: ");
  Serial.print(a7_value);
  Serial.print(" A?: ");
  Serial.print("0");
  Serial.print(" A9: ");
  Serial.print(a9_value);
  Serial.println("");
  Serial.println("");
}
