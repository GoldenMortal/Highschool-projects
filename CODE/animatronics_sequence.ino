
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ---------- Pin Definitions ----------
const int PISTON_RELAY_PIN = 7;
const int LIGHT_PIN         = 8;
const int DFPLAYER_RX_PIN   = 2; // Arduino RX <- DFPlayer TX
const int DFPLAYER_TX_PIN   = 3; // Arduino TX -> DFPlayer RX

// ---------- Audio Track Numbers (on SD card) ----------
const int TRACK_VOICE_LINE = 1; // 001.mp3 - "The British are coming"
const int TRACK_THUNDER    = 2; // 002.mp3 - thunder sound effect

// ---------- Relay Behavior ----------
// Set true if your relay module is "active LOW" (common on cheap boards)
const bool RELAY_ACTIVE_LOW = true;

// ---------- Timings (ms) ----------
const unsigned long PISTON_FIRE_TIME   = 500;   // how long piston stays extended
const unsigned long VOICE_LINE_WAIT    = 3000;  // time allotted for voice line to finish
const unsigned long FLICKER_DURATION   = 2000;  // total time spent flickering
const unsigned long FLICKER_STEP_MIN   = 60;    // fastest flicker on/off time
const unsigned long FLICKER_STEP_MAX   = 220;   // slowest flicker on/off time
const unsigned long THUNDER_WAIT       = 4000;  // time allotted for thunder sfx to finish
const unsigned long CYCLE_INTERVAL     = 60000; // 1 minute between cycle starts

const int TOTAL_CYCLES = 12;

SoftwareSerial dfPlayerSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFRobotDFPlayerMini dfPlayer;

int cycleCount = 0;

void setup() {
  Serial.begin(9600);

  pinMode(PISTON_RELAY_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pistonOff();
  digitalWrite(LIGHT_PIN, LOW);

  dfPlayerSerial.begin(9600);
  Serial.println("Initializing DFPlayer Mini...");

  if (!dfPlayer.begin(dfPlayerSerial)) {
    Serial.println("DFPlayer Mini not detected! Check wiring/SD card.");
    while (true) {
      // halt here since audio is core to the effect
      delay(1000);
    }
  }

  dfPlayer.volume(25); // 0 (mute) to 30 (max)
  Serial.println("DFPlayer ready. Starting animatronics sequence.");

  randomSeed(analogRead(A0)); // seed randomness for flicker timing
}

void loop() {
  if (cycleCount >= TOTAL_CYCLES) {
    Serial.println("All 12 cycles complete. Halting.");
    while (true) {
      delay(1000); // stop here permanently
    }
  }

  cycleCount++;
  Serial.print("=== Cycle ");
  Serial.print(cycleCount);
  Serial.print(" of ");
  Serial.print(TOTAL_CYCLES);
  Serial.println(" ===");

  runSequence();

  Serial.println("Cycle complete. Waiting for next minute mark...");
  delay(CYCLE_INTERVAL);
}

// ---------- Sequence ----------
void runSequence() {
  // 1. Fire piston
  Serial.println("Piston firing...");
  pistonOn();
  delay(PISTON_FIRE_TIME);
  pistonOff();

  // 2. Play voice line: "The British are coming"
  Serial.println("Playing voice line...");
  dfPlayer.play(TRACK_VOICE_LINE);
  delay(VOICE_LINE_WAIT);

  // 3. Flicker lights
  Serial.println("Flickering lights...");
  flickerLights(FLICKER_DURATION);

  // 4. Play thunder sound
  Serial.println("Playing thunder sound...");
  dfPlayer.play(TRACK_THUNDER);
  delay(THUNDER_WAIT);
}

// ---------- Helper Functions ----------
void flickerLights(unsigned long durationMs) {
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    digitalWrite(LIGHT_PIN, HIGH);
    delay(random(FLICKER_STEP_MIN, FLICKER_STEP_MAX));
    digitalWrite(LIGHT_PIN, LOW);
    delay(random(FLICKER_STEP_MIN, FLICKER_STEP_MAX));
  }
  digitalWrite(LIGHT_PIN, LOW);
}

void pistonOn() {
  digitalWrite(PISTON_RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
}

void pistonOff() {
  digitalWrite(PISTON_RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
}
