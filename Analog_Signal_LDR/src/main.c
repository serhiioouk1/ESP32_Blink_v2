// Модуль 3.1 — Частина 5: LDR-сигналізатор, з повною обробкою помилок
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "driver/gpio.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"

#define LDR_UNIT       ADC_UNIT_1
#define LDR_CHANNEL    ADC_CHANNEL_6     // GPIO7
#define LED_PIN        GPIO_NUM_5

#define SAMPLES                16
#define SAMPLE_PERIOD_US       (50 * 1000)
#define THRESHOLD_ON_RAW       1500
#define THRESHOLD_OFF_RAW      1800

#define ADC_MAX_RAW            4095
#define APPROX_ADC_FULL_MV     3300

static const char *TAG = "LDR";

static int read_averaged(adc_oneshot_unit_handle_t handle,
                         adc_channel_t channel)
{
    int64_t sum = 0;
    int valid_samples = 0;               // рахуємо лише вдалі заміри — не всі SAMPLES обов'язково пройдуть без помилок

    for (int i = 0; i < SAMPLES; i++) {
        int raw = 0;

        if (adc_oneshot_read(handle, channel, &raw) == ESP_OK) {
            sum += raw;
            valid_samples++;
        }
    }

    if (valid_samples == 0) {
        return -1;                        // сигналізуємо виклику: жодного вдалого виміру не було
    }

    return (int)(sum / valid_samples);
}

void app_main(void)
{
    gpio_config_t led_conf = {
        .pin_bit_mask = 1ULL << LED_PIN,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&led_conf));

    adc_oneshot_unit_handle_t adc1_handle = NULL;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = LDR_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(     // один юніт ADC1 — два незалежно налаштовані канали
        adc1_handle, LDR_CHANNEL, &chan_config));

    adc_cali_handle_t cali_handle = NULL;
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = LDR_UNIT,
        .chan = LDR_CHANNEL,             // калібрування прив'язане саме до каналу LDR
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    const esp_err_t cali_ret =
        adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
    const bool calibration_available = (cali_ret == ESP_OK);

    if (!calibration_available) {
        ESP_LOGW(TAG,
                 "ADC factory calibration unavailable; "
                 "LDR voltage will be approximate");
    }

    bool led_on = false;
    int64_t last_sample = 0;

    while (true) {
        const int64_t now = esp_timer_get_time();

        if (now - last_sample >= SAMPLE_PERIOD_US) {   // неблокуюча дискретизація LDR за розкладом
            last_sample = now;

            const int level = read_averaged(adc1_handle, LDR_CHANNEL);

            if (level >= 0) {
                int voltage_mv =
                    (level * APPROX_ADC_FULL_MV) / ADC_MAX_RAW;   // "наївна" формула — запасний варіант

                if (calibration_available) {
                    int calibrated_mv = 0;
                    if (adc_cali_raw_to_voltage(
                            cali_handle, level, &calibrated_mv) == ESP_OK) {
                        voltage_mv = calibrated_mv;               // а тут — точне, якщо калібрування вдалося
                    }
                }

                if (!led_on && level < THRESHOLD_ON_RAW) {
                    led_on = true;
                } else if (led_on && level > THRESHOLD_OFF_RAW) {
                    led_on = false;
                }

                gpio_set_level(LED_PIN, led_on ? 1 : 0);

                ESP_LOGI(TAG,
                         "LDR=%4d (%d mV)\tLED=%s",
                         level,
                         voltage_mv,
                         led_on ? "ON" : "OFF");
            } else {
                ESP_LOGW(TAG, "Failed to read LDR ADC channel");   // АЦП повернув помилку — не тихо ігноруємо
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));    // темп циклу ~2 Гц — і LDR (гейт 50 мс)
    }
}