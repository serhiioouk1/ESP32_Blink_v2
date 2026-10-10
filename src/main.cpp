#include <Arduino.h>
#include "Led.hpp"

unsigned long startTime = 0;
constexpr uint8_t LED_PIN = 16;
constexpr uint16_t blinkInterval = 500;

LedState  state = LedState ::OFF;

void setup() {

  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  
  if (millis() - startTime >= blinkInterval){
    
    state = (state == LedState ::ON)
      ? LedState ::OFF
      : LedState ::ON;
    
    if(state == LedState ::ON) {

      digitalWrite(LED_PIN, HIGH);    
    }
    else {
      digitalWrite(LED_PIN, LOW);
    }

    startTime = millis();
  }
}