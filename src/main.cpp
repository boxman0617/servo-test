#include <Arduino.h>
#include <Adafruit_SoftServo.h>

#define SERVO_PIN 2

Adafruit_SoftServo myServo;

void setup() {
  Serial.begin(9600);
  Serial.println("Ready!");

  myServo.attach(SERVO_PIN);
  myServo.write(90);
  delay(15);
}

int counter = 0;
int degree = 0;
bool inc = true;
int cycle = 45;

void setServo() {
  Serial.println("degree");
  Serial.println(degree);
  Serial.println("counter");
  Serial.println(counter);

  myServo.write(degree);
  myServo.refresh();
}

void loop() {
  counter = counter + 1;

  if (degree >= 180 && inc) {
    inc = false;
  } else if (!inc && degree <= 0) {
    inc = true;
  }

  if (counter >= 10) {
    if (inc) {
      degree = degree + cycle;
    } else {
      degree = degree - cycle;
    }
    counter = 0;
    setServo();
  }

  delay(10);
}