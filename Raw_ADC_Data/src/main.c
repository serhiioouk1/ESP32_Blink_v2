// Модуль 3.1 — Частина 5: LDR-сигналізатор, з повною обробкою помилок
#include <stdio.h>
#include <unistd.h>

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

#define POT_UNIT       ADC_UNIT_1
#define POT_CHANNEL    ADC_CHANNEL_6     // GPIO7

#define SAMPLE_PERIOD_US       (50 * 1000)

#define ADC_MAX_RAW            4095
#define APPROX_ADC_FULL_MV     3300

static const char *TAG = "POT";

void app_main(void)
{
    adc_oneshot_unit_handle_t adc1_handle = NULL;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = POT_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(     // один юніт ADC1 — два незалежно налаштовані канали
        adc1_handle, POT_CHANNEL, &chan_config));

    adc_cali_handle_t cali_handle = NULL;
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = POT_UNIT,
        .chan = POT_CHANNEL,             // калібрування прив'язане саме до каналу POT
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

    int64_t last_sample = 0;

    while (true) {
        const int64_t now = esp_timer_get_time();

        if (now - last_sample >= SAMPLE_PERIOD_US) {   // неблокуюча дискретизація LDR за розкладом
            last_sample = now;

            int adc_raw = 0;
            adc_oneshot_read(adc1_handle, POT_CHANNEL, &adc_raw);

            if (adc_raw >= 0) {
                int calculated_voltage_mv =
                    (adc_raw * APPROX_ADC_FULL_MV) / ADC_MAX_RAW;   // "наївна" формула — запасний варіант
                
                int calibrated_mv = 0;
                if (calibration_available) {
                    adc_cali_raw_to_voltage(cali_handle, adc_raw, &calibrated_mv);
                }

                // 4. Обчислюємо похибку
                float error_percent = 0.0f;
                if (calibrated_mv != 0) {
                    error_percent = abs(calculated_voltage_mv - calibrated_mv)* 100.0f
                        / calibrated_mv;
                }

                ESP_LOGI(TAG,
                    "RAW: %d | Calculated: %d mV | Calibrated: %d mV | Error: %.2f%%",
                    adc_raw,
                    calculated_voltage_mv,
                    calibrated_mv,
                    error_percent);
            } else {
                ESP_LOGW(TAG, "Failed to read POT ADC channel");   // АЦП повернув помилку — не тихо ігноруємо
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}