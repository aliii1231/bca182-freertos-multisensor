#include "alarm.h"
#include "stm32f1xx_hal.h"
#include "rtos_objects.h"

#define BUZZER_GPIO_PORT GPIOB
#define BUZZER_GPIO_PIN  GPIO_PIN_12

AlarmState evaluateTemperature(float temperature)
{
    if (temperature < 18.0f) return AlarmState::LOW_TEMPERATURE;
    if (temperature > 30.0f) return AlarmState::HIGH_TEMPERATURE;
    return AlarmState::NORMAL;
}

void AlarmTask(void *pvParameters)
{
    (void)pvParameters;
    bool alarming = false;

    for (;;) {
        SensorData sample;
        if (xQueuePeek(xSensorQueue, &sample, 0) == pdTRUE) {
            AlarmState state = evaluateTemperature(sample.temperature);
            alarming = state != AlarmState::NORMAL;
            if (!alarming) {
                HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, GPIO_PIN_RESET);
            }
        } else if (alarming) {
            HAL_GPIO_TogglePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
        }

        vTaskDelay(alarming ? pdMS_TO_TICKS(1) : pdMS_TO_TICKS(10));
    }
}