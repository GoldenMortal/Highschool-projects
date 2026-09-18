#include <Servo.h>[cite: 1]

// components[cite: 1]
const int greenPin = 13;[cite: 1]
const int redPin = 12;[cite: 1]
const int buzzer = 8;[cite: 1]
const int servoPin = 11;[cite: 1]
const int lockPin = 7;[cite: 1]
const int pistonPin = 4;[cite: 1]

// sensors[cite: 1]
const int sensorPin1 = 6;[cite: 1]
const int sensorPin2 = 5;[cite: 1]

// servo positions[cite: 1]
const int guardRaised = 90;[cite: 1]
const int guardLowered = 180;[cite: 1]

Servo bridgeServo;[cite: 1]

// buzzer[cite: 1]
const int buzzerNoise = 250;[cite: 1]

// timings[cite: 1]
const unsigned long servotime = 700;[cite: 1]
const unsigned long locktime = 1000;[cite: 1]
const unsigned long bridgetime = 4000;[cite: 1]
const unsigned long waittime = 10000;[cite: 1]

// edge detection[cite: 1]
bool detectRisingEdge(int pin, int &lastState) {[cite: 1]
  int current = digitalRead(pin);[cite: 1]
  bool edge = (lastState == LOW && current == HIGH);[cite: 1]
  lastState = current;[cite: 1]
  return edge;[cite: 1]
}

// state machine[cite: 1]
enum BridgeState { SAFE, WAITING_TO_RAISE, RAISING, RAISED, LOWERING };[cite: 1]
BridgeState state = SAFE;[cite: 1]

unsigned long stateStart = 0; // when we entered the current state[cite: 1]

// sensor memory[cite: 2]
int lastSensor1 = LOW;[cite: 2]
int lastSensor2 = LOW;[cite: 2]

void setup() {[cite: 2]
  Serial.begin(9600);[cite: 2]

  // outputs[cite: 2]
  pinMode(greenPin, OUTPUT);[cite: 2]
  pinMode(redPin, OUTPUT);[cite: 2]
  pinMode(buzzer, OUTPUT);[cite: 2]
  bridgeServo.attach(servoPin);[cite: 2]
  pinMode(lockPin, OUTPUT);[cite: 2]
  pinMode(pistonPin, OUTPUT);[cite: 2]

  // inputs[cite: 2]
  pinMode(sensorPin1, INPUT);[cite: 2]
  pinMode(sensorPin2, INPUT);[cite: 2]

  lastSensor1 = digitalRead(sensorPin1);[cite: 2]
  lastSensor2 = digitalRead(sensorPin2);[cite: 2]

  // starting stage[cite: 2]
  state = SAFE;[cite: 2]
  Serial.println("Bridge system ready.");[cite: 2]
}

void loop() {[cite: 2]
  unsigned long now = millis();[cite: 2]

  // sensor checks[cite: 2]
  if (detectRisingEdge(sensorPin1, lastSensor1) && state == SAFE) {[cite: 2]
    state = WAITING_TO_RAISE;[cite: 2]
    stateStart = now;[cite: 2]
    Serial.println("Boat detected entering. Preparing to raise...");[cite: 2]
  }

  if (detectRisingEdge(sensorPin2, lastSensor2) && state == RAISED) {[cite: 2]
    state = LOWERING;[cite: 2]
    stateStart = now;[cite: 2]
    Serial.println("Boat detected leaving. Lowering bridge...");[cite: 2, 3]
  }

  // state machine[cite: 3]
  switch (state) {[cite: 3]
    case SAFE:[cite: 3]
      digitalWrite(lockPin, HIGH);[cite: 3]
      bridgeServo.write(guardRaised);[cite: 3]
      if (now - stateStart >= locktime) {[cite: 3]
        digitalWrite(greenPin, HIGH);[cite: 3]
        digitalWrite(redPin, LOW);[cite: 3]
        noTone(buzzer);[cite: 3]
        stateStart = now;[cite: 3]
      }
      break;[cite: 3]

    case WAITING_TO_RAISE:[cite: 3]
      digitalWrite(greenPin, LOW);[cite: 3]
      digitalWrite(redPin, HIGH);[cite: 3]
      if (now - stateStart >= waittime) {[cite: 3]
        bridgeServo.write(guardLowered);[cite: 3]
        state = RAISING;[cite: 3]
        stateStart = now;[cite: 3]
        Serial.println("Lowering guard, raising bridge...");[cite: 3]
      }
      break;[cite: 3]

    case RAISING:[cite: 3]
      if (now - stateStart >= servotime) {[cite: 3]
        tone(buzzer, buzzerNoise);[cite: 3]
        digitalWrite(lockPin, LOW);[cite: 3]
        if (now - stateStart >= locktime) {[cite: 3]
          digitalWrite(pistonPin, HIGH);[cite: 3]
          state = RAISED;[cite: 3]
          stateStart = now;[cite: 3]
          Serial.println("Bridge is raising...");[cite: 3]
        }
      }
      break;[cite: 3]

    case RAISED:[cite: 4]
      noTone(buzzer);[cite: 4]
      Serial.println("Bridge fully raised. Waiting for boat to leave...");[cite: 4]
      // stays in RAISED until sensorPin2 triggers[cite: 4]
      break;[cite: 4]

    case LOWERING:[cite: 4]
      digitalWrite(pistonPin, LOW);[cite: 4]
      if (now - stateStart >= bridgetime) {[cite: 4]
        bridgeServo.write(guardRaised);[cite: 4]
        digitalWrite(lockPin, HIGH);[cite: 4]
        if (now - stateStart >= locktime) {[cite: 4]
          digitalWrite(greenPin, HIGH);[cite: 4]
          digitalWrite(redPin, LOW);[cite: 4]
          state = SAFE;[cite: 4]
          stateStart = now;[cite: 4]
          Serial.println("Bridge lowered. Safe mode restored.");[cite: 4]
        }
      }
      break;[cite: 4]
  }
}