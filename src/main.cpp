#include <Arduino.h>
#include "Led.hpp"

unsigned long startTime = 0;
LedState  state = LedState ::OFF;

Led led;

void setup() {

  led.init();
}

void loop() {
  
  if (millis() - startTime >= led.blinkInterval){
    
    state = (state == LedState ::ON)
      ? LedState ::OFF
      : LedState ::ON;
    
    led.set(state);

    startTime = millis();
  }
}