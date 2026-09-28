#include "system_state.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_objects.h"
#include "stm32f1xx_hal.h"
#include <cstring>

extern UART_HandleTypeDef huart1;

static const uint32_t INACTIVITY_TIMEOUT_MS = 15000;

static void printState(const char *message)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)message,
                      (uint16_t)std::strlen(message), HAL_MAX_DELAY);
}

SystemState evaluateSystemState(SystemState current, bool motion, uint32_t elapsedMs)
{
    if (motion) return SystemState::ACTIVE;
    if (current == SystemState::ACTIVE && elapsedMs >= INACTIVITY_TIMEOUT_MS) {
        return SystemState::INACTIVE;
    }
    return current;
}

void StateTask(void *pvParameters)
{
    (void)pvParameters;
    SystemState state = SystemState::ACTIVE;
    TickType_t lastMotion = xTaskGetTickCount();

    for (;;) {
        bool motion = false;
        if (xQueueReceive(xMotionQueue, &motion, 0) == pdTRUE && motion) {
            lastMotion = xTaskGetTickCount();
        }

        TickType_t elapsed = xTaskGetTickCount() - lastMotion;
        SystemState nextState = evaluateSystemState(
            state, motion, (uint32_t)pdTICKS_TO_MS(elapsed));
        if (nextState != state) {
            state = nextState;
            printState(state == SystemState::ACTIVE
                       ? "[STATE] ACTIVE\r\n"
                       : "[STATE] INACTIVE\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}