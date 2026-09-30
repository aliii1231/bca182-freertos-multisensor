#ifndef RTOS_OBJECTS_H
#define RTOS_OBJECTS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h" // Required for Event Groups
#include "semphr.h" // Required for mutexes

// Existing structs and enums...
enum DisplayMode {
    DISPLAY_TEMPERATURE = 0,
    DISPLAY_HUMIDITY,
    DISPLAY_LIGHT,
    DISPLAY_MOTION
};

enum NavCommand {
    NAV_NEXT = 1,
    NAV_PREV = 2
};

struct SensorData {
    float temperature;
    float humidity;
    int lightLevel;
    bool motionDetected;
};

// Queue handles
extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xNavQueue;
extern QueueHandle_t xMotionQueue;

// Part XI: Mutex for shared UART resource
extern SemaphoreHandle_t xSerialMutex;

// Part X: Event Group and Bits (Lab Manual Section 35)
extern EventGroupHandle_t xSystemEventGroup;

#define EVENT_ACTIVE_BIT0  (1 << 0)
#define EVENT_MOTION_BIT1  (1 << 1)
#define EVENT_ALARM_BIT2   (1 << 2)

BaseType_t createRtosObjects(void);

#endif // RTOS_OBJECTS_H