/**
 * rtos_objects.cpp - FreeRTOS IPC object creation (S25 + S28/S29)
 * xSensorQueue: length-1 latest-value mailbox (xQueueOverwrite).
 * xNavQueue:    length-4 navigation commands from InputTask;
 *               DisplayTask drains it fully each cycle (freshest wins).
 */
#include "rtos_objects.h"

QueueHandle_t xSensorQueue = NULL;
QueueHandle_t xNavQueue    = NULL;
QueueHandle_t xMotionQueue = NULL;

BaseType_t createRtosObjects(void)
{
    xSensorQueue = xQueueCreate(1, sizeof(SensorData));
    if (xSensorQueue == NULL) { return pdFAIL; }

    xNavQueue = xQueueCreate(4, sizeof(NavCommand));
    if (xNavQueue == NULL) { return pdFAIL; }

    xMotionQueue = xQueueCreate(1, sizeof(bool));
    if (xMotionQueue == NULL) { return pdFAIL; }

    return pdPASS;
}