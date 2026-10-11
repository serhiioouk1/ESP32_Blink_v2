#include <Arduino.h>
#include "Led.hpp"

void Led::init(const GpioConfig& config){
    pinMode(config.LED_PIN, OUTPUT);
}

void Led::set(const GpioConfig& config, LedState state){
    if(state == LedState ::ON) {

      digitalWrite(config.LED_PIN, HIGH);    
    }
    else {
      digitalWrite(config.LED_PIN, LOW);
    }
}