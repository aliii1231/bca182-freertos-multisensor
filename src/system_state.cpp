#include "system_state.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
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

    // Set the ACTIVE bit initially
    if (xSystemEventGroup != NULL) {
        xEventGroupSetBits(xSystemEventGroup, EVENT_ACTIVE_BIT0);
    }

    for (;;) {
        bool motion = false;
        
        // Check queue
        if (xQueueReceive(xMotionQueue, &motion, 0) == pdTRUE && motion) {
            lastMotion = xTaskGetTickCount();
        }

        // Part X: Check event group bits for motion signaling
        if (xSystemEventGroup != NULL) {
            EventBits_t bits = xEventGroupWaitBits(
                xSystemEventGroup,
                EVENT_MOTION_BIT1,
                pdTRUE,   // Clear bit on exit
                pdFALSE,  // Wait for any bit
                0         // Non-blocking poll
            );
            if ((bits & EVENT_MOTION_BIT1) != 0) {
                motion = true;
                lastMotion = xTaskGetTickCount();
            }
        }

        TickType_t elapsed = xTaskGetTickCount() - lastMotion;
        SystemState nextState = evaluateSystemState(
            state, motion, (uint32_t)pdTICKS_TO_MS(elapsed));
            
        if (nextState != state) {
            state = nextState;
            if (state == SystemState::ACTIVE) {
                if (xSystemEventGroup != NULL) {
                    xEventGroupSetBits(xSystemEventGroup, EVENT_ACTIVE_BIT0);
                }
                printState("[STATE] ACTIVE\r\n");
            } else {
                if (xSystemEventGroup != NULL) {
                    xEventGroupClearBits(xSystemEventGroup, EVENT_ACTIVE_BIT0);
                }
                printState("[STATE] INACTIVE\r\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}