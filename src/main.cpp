#include <Arduino.h>
#include "Led.hpp"

void BlinkLogic();

unsigned long startTime = 0;
unsigned long blinkStartTime = 0;

LedState  state = LedState::OFF;
GpioConfig config;
Led led;
LoopMeasurement lm;

void setup() {

  Serial0.begin(115200);

  led.init(config);
}

void loop() {
  
  uint32_t currentTime = micros();

  if (lm.previousTime != 0)
  {
      uint32_t loopTime = currentTime - lm.previousTime;

      lm.totalTime += loopTime;
      lm.ind++;

      if (lm.ind == 1000)
      {
          float averageTime = (float)lm.totalTime / lm.ind;

          Serial0.print(lm.ind);
          Serial0.print(" iterations loop time: ");
          Serial0.print(averageTime);
          Serial0.println(" microseconds.");

          lm.totalTime = 0;
          lm.ind = 0;
      }
  }

  lm.previousTime = currentTime;

  BlinkLogic();
}

void BlinkLogic()
{
  if (millis() - blinkStartTime >= config.blinkInterval){
    
    state = (state == LedState ::ON)
      ? LedState ::OFF
      : LedState ::ON;
    
    led.set(config, state);

    blinkStartTime = millis();
  }
}