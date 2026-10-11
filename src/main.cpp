#include <Arduino.h>
#include "Led.hpp"

unsigned long startTime = 0;
LedState  state = LedState ::OFF;
GpioConfig config;
Led led;

void setup() {

  led.init(config);
}

void loop() {
  
  if (millis() - startTime >= config.blinkInterval){
    
    state = (state == LedState ::ON)
      ? LedState ::OFF
      : LedState ::ON;
    
    led.set(config, state);

    startTime = millis();
  }
}