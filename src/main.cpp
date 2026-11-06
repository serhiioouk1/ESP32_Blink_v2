#include <Arduino.h>

constexpr uint8_t PIN1 = 9;
constexpr uint8_t PIN2 = 10;
constexpr uint8_t PIN3 = 11;

constexpr uint64_t PIN1_INTERVAL = 200;
constexpr uint64_t PIN2_INTERVAL = 500;
constexpr uint64_t PIN3_INTERVAL = 1000;

unsigned long Pin1_last_toggle = 0;
unsigned long Pin2_last_toggle = 0;
unsigned long Pin3_last_toggle = 0;

void setup() {

  pinMode(PIN1, OUTPUT);
  pinMode(PIN2, OUTPUT);
  pinMode(PIN3, OUTPUT);
}

void loop() {
   
  const unsigned long now = millis();

  if (now - Pin1_last_toggle >= PIN1_INTERVAL) {
      digitalWrite(PIN1, !digitalRead(PIN1));
      Pin1_last_toggle = now;
  }

  if (now - Pin2_last_toggle >= PIN2_INTERVAL) {
      digitalWrite(PIN2, !digitalRead(PIN2));
      Pin2_last_toggle = now;
  }

  if (now - Pin3_last_toggle >= PIN3_INTERVAL) {
      digitalWrite(PIN3, !digitalRead(PIN3));
      Pin3_last_toggle = now;
  }
}