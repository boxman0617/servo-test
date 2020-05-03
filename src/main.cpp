#include <Arduino.h>
#include <Adafruit_SoftServo.h>
#include "Ticker.h"
#include "Plotter.h"

#define SERVO_PIN1 2
#define SERVO_PIN2 16
#define IS_DEBUG true

// This is the reserved tick function that gets called by Ticker
// every 1 second to then render out our state to our servos
void servoTick();

const int SERVO_START = 90;
const int SERVO_MIN = 0;
const int SERVO_MAX = 180;
const double DEFAULT_INITIAL_SERVO_POSITION = 90.0;

Plotter p;

// ## Servo

class Servo {
    Adafruit_SoftServo _s;
    String _name;
  public:
    double _value; // Value in degrees of a servo
    Servo(String name);
    void set(double value);
    void render();
    void init(uint8_t pin, double default_position);
};

Servo::Servo(String name) {
  this->_name = name;
}
void Servo::set(double value) {
  this->_value = value;
  this->_s.write(this->_value);
}
void Servo::render() {
  Serial.print(this->_name);
  Serial.print(" -> Rendering: ");
  Serial.println(this->_value);
  this->_s.refresh();
}
void Servo::init(uint8_t pin, double default_position) {
  this->_s.attach(pin);
  this->set(default_position);
  this->render();
}

// ## ServoGroup

const int MAX_SERVO_GROUP_SIZE = 4;
const int MAX_SERVO_GROUP_NEST_SIZE = 4;

class ServoGroup {
  unsigned int servoCount = 0;
  unsigned int nestCount = 0;
  String _name;
  public:
    Servo* servos [MAX_SERVO_GROUP_SIZE] {};
    ServoGroup* groups [MAX_SERVO_GROUP_NEST_SIZE] {};
    ServoGroup(String name);
    void add(Servo *s);
    void nest(ServoGroup *sg);
    void render();
};

ServoGroup::ServoGroup(String name) {
  this->_name = name;
}
void ServoGroup::add(Servo *s) {
  if (this->servoCount == MAX_SERVO_GROUP_SIZE) throw "Unable to add more Servos!";
  this->servos[servoCount] = s;
  this->servoCount = this->servoCount + 1;
}
void ServoGroup::nest(ServoGroup *sg) {
  if (nestCount == MAX_SERVO_GROUP_NEST_SIZE) throw "Unable to nest more ServoGroups!";
  groups[nestCount] = sg;
  nestCount = nestCount + 1;
}
// Render will always cycle through its nested groups first,
// then cycle through its servos
void ServoGroup::render() {
  for (unsigned int i = 0; i < this->nestCount; i = i + 1) {
    this->groups[i]->render();
  }
  for (unsigned int i = 0; i < this->servoCount; i = i + 1) {
    this->servos[i]->render();
  }
}

// ## 

// #######

uint32_t time_cycle = 100;
Ticker renderTimer(servoTick, time_cycle, 100, MILLIS);

Servo s1("ShoulderLeft");
Servo s2("ShoulderRight");

ServoGroup body("Body");
ServoGroup shoulders("Shoulders");

// ## Plotter Settings
double p_s1;
double p_s2;

void setupPlotter() {
  p.Begin();

  // p.AddTimeGraph("S", 500, "S1", p_s1, "S2", p_s2);
  p.AddTimeGraph("S", 1000, "S1", p_s1);
}

void debugPlot() {
  p_s1 = body.groups[0]->servos[0]->_value;
  p_s2 = body.groups[0]->servos[1]->_value;
}

// Setup

void setup() {
  Serial.begin(9600);
  Serial.println("Ready!");
  setupPlotter();

  s1.init(SERVO_PIN1, DEFAULT_INITIAL_SERVO_POSITION);
  s2.init(SERVO_PIN2, DEFAULT_INITIAL_SERVO_POSITION);

  shoulders.add(&s1);
  shoulders.add(&s2);

  body.nest(&shoulders);

  renderTimer.start();
}

const double two_pie = 2.0 * PI;
const double speed = 1000.0; // not 100% sure what this does yet

void servoTick() {
  Serial.println("Servo Tick");

  // lets pretend we are in the "walk" state
  body.groups[0]->servos[0]->set(
    abs(SERVO_MAX * sin(
      two_pie * (
        millis() / speed
      )
    ))
  );
  body.groups[0]->servos[1]->set(
    abs(SERVO_MAX * sin(
      two_pie * (
        millis() / speed
      ) + 90
    ))
  );

  debugPlot();
  body.render();
}

void loop() {
  p.Plot();

  renderTimer.update();
}