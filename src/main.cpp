#include <Arduino.h>
#include "Led.hpp"

void BlinkLogic();
void IRAM_ATTR onButtonInterrupt();

unsigned long startTime = 0;
unsigned long blinkStartTime = 0;

LedState  state = LedState::OFF;
GpioConfig config;
Led led;
BlinkMode blinkMode = BlinkMode::ALWAYS_ON;

volatile bool button_pressed = false;
uint32_t last_debounce_time = 0;

void setup() {

  led.init(config);

  attachInterrupt(
    digitalPinToInterrupt(config.BUTTON_READ_PIN),
    onButtonInterrupt,
    RISING);
}

void loop() {
  
  if(button_pressed)
  {
    button_pressed = false;

    //debounce the button press
    uint32_t current_time = millis();

    if (current_time - last_debounce_time >= config.DEBOUNCE_MS)
    {
        last_debounce_time = current_time;

        // Cycle through the blink modes
        blinkMode = static_cast<BlinkMode>((blinkMode + 1) % 3);
    }
  }

  // Handle the LED state based on the current blink mode
  if(blinkMode == BlinkMode::ALWAYS_ON)
  {
    state = LedState::ON;
    led.set(config, state);
  }
  else if(blinkMode == BlinkMode::ALWAYS_OFF)
  {
    state = LedState::OFF;
    led.set(config, state);
  }
  else if(blinkMode == BlinkMode::BLINK)
  {
    BlinkLogic();
  }
}

void IRAM_ATTR onButtonInterrupt()
{
    button_pressed = true;
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