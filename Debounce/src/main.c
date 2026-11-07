#include <stdint.h>
#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

const uint8_t BUTTON_PIN = 4;
const uint8_t DEBOUNCE_TIME = 50;

uint64_t lastInterruptTime = 0;
unsigned long counter = 0;

volatile bool buttonPressed = false;

void IRAM_ATTR buttonISR(void* arg) {
  buttonPressed = true;
}

void setup(void){

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLDOWN_ONLY);


    // Вибір фронту переривання - FALLING
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);

    // Ініціалізація сервісу GPIO Interrupt
    gpio_install_isr_service(0);

    // Прив'язка ISR до піна
    gpio_isr_handler_add(BUTTON_PIN, buttonISR, NULL);
}

void loop(void) {

  if (buttonPressed) {

    buttonPressed = false;

    // Отримання поточного часу в мілісекундах, esp_timer_get_time повертає час в мікросекундах, тому ділимо на 1000
    uint64_t currentTime = esp_timer_get_time() / 1000;

    if (currentTime - lastInterruptTime >= DEBOUNCE_TIME) {
        
      lastInterruptTime = currentTime;
        
      printf("Button pressed! Count: %lu\n", ++counter);
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