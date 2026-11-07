#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

const uint8_t BUTTON_PIN = 4;

unsigned long counter = 0;

const uint8_t POLLING_INTERVAL = 5;
const uint8_t DEBOUNCE_TIME = 50;

typedef enum {
    RELEASED = 0,
    PRESSED = 1
} button_state;

button_state buttonState = RELEASED;

bool lastRawState = false;
bool candidateState = false;

uint64_t lastPollingTime = 0;
uint64_t candidateSince = 0;

void setup(void){

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLDOWN_ONLY);

    bool initialState = gpio_get_level(BUTTON_PIN) == 1;

    lastRawState = initialState;
    candidateState = initialState;

    buttonState = initialState
        ? PRESSED
        : RELEASED;

  candidateSince = esp_timer_get_time() / 1000;
}

void loop(void) {

  uint64_t currentTime = esp_timer_get_time() / 1000;

  // Опитування кожні 5 мс
  if (currentTime - lastPollingTime < POLLING_INTERVAL)
    return;

  lastPollingTime = currentTime;

  // HIGH = натиснута, LOW = відпущена
  bool rawState = gpio_get_level(BUTTON_PIN) == 1;

  // Виявлення зміни фізичного стану
  if (rawState != lastRawState)
  {
    lastRawState = rawState;
    candidateState = rawState;
    candidateSince = currentTime;
  }

  // Перевірка стабільності протягом 50 мс
  if (currentTime - candidateSince >= DEBOUNCE_TIME)
  {
    bool stablePressed = buttonState == PRESSED;

    if (candidateState != stablePressed)
    {
      buttonState = candidateState
        ? PRESSED
        : RELEASED;

      if (buttonState == PRESSED)
      {
        printf("Button pressed! Count: %lu\n", ++counter);
      }
    }
  }
}

void loopTask(void* parameter) {
    setup();

    while (1) {
        loop();
    }
}

void app_main(void) {
    xTaskCreatePinnedToCore(
        loopTask,
        "loopTask",
        4096,
        NULL,
        0,
        NULL,
        1
    );
}