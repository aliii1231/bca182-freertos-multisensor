#include "alarm.h"
#include "stm32f1xx_hal.h"
#include "rtos_objects.h"

extern TIM_HandleTypeDef htim1;

AlarmState evaluateTemperature(float temperature)
{
    if (temperature < 18.0f) return AlarmState::LOW_TEMPERATURE;
    if (temperature > 30.0f) return AlarmState::HIGH_TEMPERATURE;
    return AlarmState::NORMAL;
}

void AlarmTask(void *pvParameters)
{
    (void)pvParameters;

    bool alarmActive = false;
    TickType_t lastEvaluation = xTaskGetTickCount();

    for (;;) {
        TickType_t now = xTaskGetTickCount();
        if ((now - lastEvaluation) >= pdMS_TO_TICKS(100)) {
            lastEvaluation = now;

            SensorData sample;
            if (xQueuePeek(xSensorQueue, &sample, 0) == pdTRUE) {
                AlarmState state = evaluateTemperature(sample.temperature);
                alarmActive = state != AlarmState::NORMAL;

                if (xSystemEventGroup != NULL) {
                    if (alarmActive) {
                        xEventGroupSetBits(xSystemEventGroup, EVENT_ALARM_BIT2);
                    } else {
                        xEventGroupClearBits(xSystemEventGroup, EVENT_ALARM_BIT2);
                    }
                }
            }
        }

        if (alarmActive) {
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 500);
        } else {
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}