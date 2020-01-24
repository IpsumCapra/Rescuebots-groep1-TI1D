// nano
#include <Wire.h>

#define HALL_PIN 6
#define IR_PIN_LEFT 7
#define IR_PIN_RIGHT 8
#define ULTRASOON_TRIGPIN_LEFT 9
#define ULTRASOON_ECHOPIN_LEFT A3
#define ULTRASOON_TRIGPIN_RIGHT 10
#define ULTRASOON_ECHOPIN_RIGHT A2
#define ULTRASOON_TRIGPIN_UNDER 11
#define ULTRASOON_ECHOPIN_UNDER A1
#define ULTRASOON_TRIGPIN_FRONT 12
#define ULTRASOON_ECHOPIN_FRONT A0
long ultrasoon_left;
long ultrasoon_right;
long ultrasoon_under;
long ultrasoon_front;
long distance;
long duration;

int hall;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(8);
  Wire.onRequest(request);

  pinMode(IR_PIN_LEFT, INPUT);
  pinMode(IR_PIN_RIGHT, INPUT);
  pinMode(ULTRASOON_TRIGPIN_LEFT, OUTPUT);
  pinMode(ULTRASOON_ECHOPIN_LEFT, INPUT);
  pinMode(ULTRASOON_TRIGPIN_RIGHT, OUTPUT);
  pinMode(ULTRASOON_ECHOPIN_RIGHT, INPUT);
  pinMode(ULTRASOON_TRIGPIN_UNDER, OUTPUT);
  pinMode(ULTRASOON_ECHOPIN_UNDER, INPUT);
  pinMode(ULTRASOON_TRIGPIN_FRONT, OUTPUT);
  pinMode(ULTRASOON_ECHOPIN_FRONT, INPUT);
  pinMode(HALL_PIN, INPUT);
}

void request() {
  Wire.write(digitalRead(IR_PIN_LEFT));
  Wire.write(digitalRead(IR_PIN_RIGHT));

  Wire.write((byte)ultrasoon_left);
  Wire.write((byte)ultrasoon_right);
  Wire.write((byte)ultrasoon_under);
  Wire.write((byte)ultrasoon_front);

  Wire.write(digitalRead(HALL_PIN));
}

void loop() {
  // put your main code here, to run repeatedly:
  ultrasoon_left = SonarSensor(ULTRASOON_TRIGPIN_LEFT, ULTRASOON_ECHOPIN_LEFT);
  if (ultrasoon_left < 9 && ultrasoon_left != 0) {
    ultrasoon_left = 1;
  } else {
    ultrasoon_left = 0;
  }
  ultrasoon_right = SonarSensor(ULTRASOON_TRIGPIN_RIGHT, ULTRASOON_ECHOPIN_RIGHT);
  if (ultrasoon_right < 9 && ultrasoon_right != 0) {
    ultrasoon_right = 1;
  } else {
    ultrasoon_right = 0;
  }
  ultrasoon_under = SonarSensor(ULTRASOON_TRIGPIN_UNDER, ULTRASOON_ECHOPIN_UNDER);
  if (ultrasoon_under > 9 && ultrasoon_under != 0) {
    ultrasoon_under = 1;
  } else {
    ultrasoon_under = 0;
  }
  ultrasoon_front = SonarSensor(ULTRASOON_TRIGPIN_FRONT, ULTRASOON_ECHOPIN_FRONT);
  if (ultrasoon_front < 6 && ultrasoon_front != 0) {
    ultrasoon_front = 1;
  } else {
    ultrasoon_front = 0;
  }
}

long SonarSensor(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;
  return distance;
}
