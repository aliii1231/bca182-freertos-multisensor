#include "motion.h"
#include "stm32f1xx_hal.h"
#include "rtos_objects.h"
#include "event_groups.h"
#include <cstring>

extern UART_HandleTypeDef huart1;

#define PIR_GPIO_PORT GPIOB
#define PIR_GPIO_PIN  GPIO_PIN_13

void MotionTask(void *pvParameters)
{
    (void)pvParameters;
    bool previousMotion = false;

    for (;;) {
        bool motion = HAL_GPIO_ReadPin(PIR_GPIO_PORT, PIR_GPIO_PIN) == GPIO_PIN_SET;
        xQueueOverwrite(xMotionQueue, &motion);

        if (motion && !previousMotion) {
            // Part X: Set the event bit for motion signaling
            if (xSystemEventGroup != NULL) {
                xEventGroupSetBits(xSystemEventGroup, EVENT_MOTION_BIT1);
            }

            const char *message = "[MOTION] detected\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t *)message,
                              (uint16_t)std::strlen(message), HAL_MAX_DELAY);
        }
        previousMotion = motion;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}