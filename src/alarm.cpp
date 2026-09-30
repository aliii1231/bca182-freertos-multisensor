#include "alarm.h"
#include "stm32f1xx_hal.h"
#include "rtos_objects.h"

#define BUZZER_GPIO_PORT GPIOB
#define BUZZER_GPIO_PIN GPIO_PIN_12

AlarmState evaluateTemperature(float temperature)
{
    if (temperature < 18.0f) return AlarmState::LOW_TEMPERATURE;
    if (temperature > 30.0f) return AlarmState::HIGH_TEMPERATURE;
    return AlarmState::NORMAL;
}

void AlarmTask(void *pvParameters)
{
    (void)pvParameters;
    
    for (;;) {
        SensorData sample;
        if (xQueuePeek(xSensorQueue, &sample, pdMS_TO_TICKS(100)) == pdTRUE) {
            AlarmState state = evaluateTemperature(sample.temperature);
            
            if (state == AlarmState::NORMAL) {
                // Turn buzzer off
                HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, GPIO_PIN_RESET);
            } else {
                // Toggle buzzer to create a pulsing alarm sound
                HAL_GPIO_TogglePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
            }
        }
        
        // Use a balanced 250ms delay so it beeps cleanly WITHOUT starving the DisplayTask
        vTaskDelay(pdMS_TO_TICKS(2500)); 
    }
}