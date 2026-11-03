#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 4;

unsigned long counter = 0;

volatile bool buttonPressed = false;

void IRAM_ATTR onButtonClick() {
  buttonPressed = true;
}

void setup() {
  Serial0.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonClick, FALLING);
}

void loop() {

  if (buttonPressed) {
      
    buttonPressed = false;

    if (digitalRead(BUTTON_PIN) == HIGH) {
      
      Serial0.printf("Button pressed! Count: %lu\n", ++counter);
    }
  }
}
