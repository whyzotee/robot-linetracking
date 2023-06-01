// Extend To Array { speed, speed - 5, speed, speed - 5} max value's 255
byte speed = 255;

// Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
const byte sensor_front[] = {54, 55, 56, 57, 58};

// Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
const byte sensor_back[] = {59, 60, 61, 62, 63};

// Array Position { intA, intB, en }
const byte frontLeft[] = {13, 12, 11};
const byte frontRight[] = {10, 9, 8};
const byte backLeft[] = {7, 6, 5};
const byte backRight[] = {4, 3, 2};

class Movement
{
public:
  void a();
}

void
setup()
{
  Serial.begin(115200);
  for (int i = 0; i < 3; i++)
  {
    pinMode(frontLeft[i], OUTPUT);
    pinMode(frontRight[i], OUTPUT);
    pinMode(backLeft[i], OUTPUT);
    pinMode(backRight[i], OUTPUT);
  }

  for (byte i = 0; i < 5; i++)
  {
    pinMode(sensor_front[i], INPUT);
    pinMode(sensor_back[i], INPUT);
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}
