
#include <Servo.h>

// ---------- Components ----------
const int greenPin  = 13;
const int redPin    = 12;
const int buzzer    = 8;
const int servoPin  = 11;
const int lockPin   = 7;
const int pistonPin = 4;

// ---------- Sensors ----------
const int sensorPin1 = 6; // entry sensor
const int sensorPin2 = 5; // exit sensor

// ---------- Servo Positions ----------
const int guardRaised  = 90;
const int guardLowered = 180;

Servo bridgeServo;

// ---------- Buzzer ----------
const int buzzerNoise = 250;

// ---------- Timings (ms) ----------
const unsigned long servotime  = 700;
const unsigned long locktime   = 1000;
const unsigned long bridgetime = 4000;
const unsigned long waittime   = 10000;

// ---------- Edge Detection ----------
bool detectRisingEdge(int pin, int &lastState) {
  int current = digitalRead(pin);
  bool edge = (lastState == LOW && current == HIGH);
  lastState = current;
  return edge;
}

// ---------- State Machine ----------
enum BridgeState { SAFE, WAITING_TO_RAISE, RAISING, RAISED, LOWERING };
BridgeState state = SAFE;

unsigned long stateStart = 0; // when we entered the current state

// ---------- Sensor Memory ----------
int lastSensor1 = LOW;
int lastSensor2 = LOW;

void setup() {
  Serial.begin(9600);

  // Outputs
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  bridgeServo.attach(servoPin);
  pinMode(lockPin, OUTPUT);
  pinMode(pistonPin, OUTPUT);

  // Inputs
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);

  lastSensor1 = digitalRead(sensorPin1);
  lastSensor2 = digitalRead(sensorPin2);

  // Starting stage
  state = SAFE;
  stateStart = millis();
  Serial.println("Bridge system ready.");
}

void loop() {
  unsigned long now = millis();

  // ---------- Sensor Checks ----------
  if (detectRisingEdge(sensorPin1, lastSensor1) && state == SAFE) {
    state = WAITING_TO_RAISE;
    stateStart = now;
    Serial.println("Boat detected entering. Preparing to raise...");
  }

  if (detectRisingEdge(sensorPin2, lastSensor2) && state == RAISED) {
    state = LOWERING;
    stateStart = now;
    Serial.println("Boat detected leaving. Lowering bridge...");
  }

  // ---------- State Machine ----------
  switch (state) {
    case SAFE:
      digitalWrite(lockPin, HIGH);
      bridgeServo.write(guardRaised);
      if (now - stateStart >= locktime) {
        digitalWrite(greenPin, HIGH);
        digitalWrite(redPin, LOW);
        noTone(buzzer);
      }
      break;

    case WAITING_TO_RAISE:
      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
      if (now - stateStart >= waittime) {
        bridgeServo.write(guardLowered);
        state = RAISING;
        stateStart = now;
        Serial.println("Lowering guard, raising bridge...");
      }
      break;

    case RAISING:
      if (now - stateStart >= servotime) {
        tone(buzzer, buzzerNoise);
        digitalWrite(lockPin, LOW);
        if (now - stateStart >= (servotime + locktime)) {
          digitalWrite(pistonPin, HIGH);
          state = RAISED;
          stateStart = now;
          Serial.println("Bridge is raising...");
        }
      }
      break;

    case RAISED:
      noTone(buzzer);
      // Stays in RAISED until sensorPin2 triggers.
      break;

    case LOWERING:
      digitalWrite(pistonPin, LOW);
      if (now - stateStart >= bridgetime) {
        bridgeServo.write(guardRaised);
        digitalWrite(lockPin, HIGH);
        if (now - stateStart >= (bridgetime + locktime)) {
          digitalWrite(greenPin, HIGH);
          digitalWrite(redPin, LOW);
          state = SAFE;
          stateStart = now;
          Serial.println("Bridge lowered. Safe mode restored.");
        }
      }
      break;
  }
}
