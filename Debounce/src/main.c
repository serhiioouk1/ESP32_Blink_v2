#include <stdint.h>
#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"

const uint8_t BUTTON_PIN = 4;

volatile bool buttonPressed = false;

unsigned long counter = 0;

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
    printf("Button pressed! Count: %lu\n", ++counter);
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