#include <Arduino.h>
#include "Led.hpp"

void Led::init(){
    pinMode(LED_PIN, OUTPUT);
}

void Led::set(LedState state){
    if(state == LedState ::ON) {

      digitalWrite(LED_PIN, HIGH);    
    }
    else {
      digitalWrite(LED_PIN, LOW);
    }
}