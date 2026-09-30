/**
 * rtos_objects.cpp - FreeRTOS IPC object creation (S25 + S28/S29 + Part X)
 * xSensorQueue: length-1 latest-value mailbox (xQueueOverwrite).
 * xNavQueue:    length-4 navigation commands from InputTask.
 * xMotionQueue: length-1 motion status queue.
 * xSystemEventGroup: Event group for system signaling (ACTIVE, MOTION, ALARM).
 */
#include "rtos_objects.h"

QueueHandle_t xSensorQueue      = NULL;
QueueHandle_t xNavQueue         = NULL;
QueueHandle_t xMotionQueue      = NULL;
EventGroupHandle_t xSystemEventGroup = NULL;
SemaphoreHandle_t xSerialMutex  = NULL; // Define mutex handle

BaseType_t createRtosObjects(void)
{
    xSensorQueue = xQueueCreate(1, sizeof(SensorData));
    if (xSensorQueue == NULL) { return pdFAIL; }

    xNavQueue = xQueueCreate(4, sizeof(NavCommand));
    if (xNavQueue == NULL) { return pdFAIL; }

    xMotionQueue = xQueueCreate(1, sizeof(bool));
    if (xMotionQueue == NULL) { return pdFAIL; }

    // Create the Event Group for Part X compliance
    xSystemEventGroup = xEventGroupCreate();
    if (xSystemEventGroup == NULL) { return pdFAIL; }

    // Part XI: Create the Mutex for thread-safe Serial output
    xSerialMutex = xSemaphoreCreateMutex();
    if (xSerialMutex == NULL) { return pdFAIL; }
    
    return pdPASS;
}