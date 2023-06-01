#include "movement.h"

// Extend To Array { speed, speed - 5, speed, speed - 5} max value's 255
byte speed = 100;

// Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
byte sensor_front[] = { 54, 55, 56, 57, 58 };

// Sensor Value
bool a0_value, a1_value, a2_value, a3_value, a4_value;
bool centerL_value;
bool centerR_value;

bool isMove = true;
bool isBack = false;

// Delay Time
unsigned long prevTime = 0;
unsigned long logTime = 0;
unsigned long delayTurn = 0;

// PID Parameters
double Kp = 0.5;  // Proportional gain
double Ki = 0.2;  // Integral gain
double Kd = 0.1;  // Derivative gain

double targetAngle = 0.0;  // Target angle for balancing

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
  }

  // Move Out From Start Point
  move(speed, "front");
  delay(500);
}

void loop() {
  a0_value = digitalRead(sensor_front[0]);
  a1_value = digitalRead(sensor_front[1]);
  a2_value = digitalRead(sensor_front[2]);
  a3_value = digitalRead(sensor_front[3]);
  a4_value = digitalRead(sensor_front[4]);
  centerL_value = digitalRead(59);
  centerR_value = digitalRead(60);

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
        if (a2_value || a3_value) {
          isMove = true;
          delayTurn = 0;
        }
      }
      move(speed, "left");
    }
    if (cross == 6) {
      if (currentTime - delayTurn > 800) {
        if (a2_value || a3_value) {
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

  if (a2_value && a3_value && a4_value) {
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

  // if (currentTime - logTime > 250) {
  //   log_sensor();
  //   logTime = currentTime;
  // }

  delay(50);
}

bool isCenter() {
  return centerL_value && centerR_value;
  //return (a0_value && a1_value && a2_value && a3_value && a4_value);
}

void balance_move() {
  // Apply the control output to the movement
  if (a2_value) {
    move(speed, "front");
  } else if (a0_value || a1_value) {
    move(speed, "left");
  } else if (a3_value || a4_value) {
    move(speed, "right");
  }
  // } else {
  //   move(0, "stop");
  //   isMove = false;
  // }
}

void balance_move_back() {
  // Apply the control output to the movement
  if (a2_value) {
    move(speed, "back");
  } else if (a0_value && a1_value) {
    move(speed, "left");
  } else if (a3_value && a4_value) {
    move(speed, "right");
  } else if (a0_value) {
    move(speed, "left");
  } else if (a4_value) {
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
  Serial.print("Center Left: ");
  Serial.print(centerL_value);
  Serial.print(" Center Right: ");
  Serial.print(centerR_value);
  Serial.println("");
  Serial.println("");
  Serial.println("");
}
