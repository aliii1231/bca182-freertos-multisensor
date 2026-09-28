#ifndef RTOS_OBJECTS_H
#define RTOS_OBJECTS_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdbool.h>

/* Section 24: sensor data exchanged between producer and consumers */
typedef struct {
    float temperature;      /* deg C          */
    float humidity;         /* %RH            */
    int   lightLevel;       /* 0-100 %        */
    bool  motionDetected;   /* Part IX: PIR   */
} SensorData;

/* Section 25: latest-value mailbox (length 1, overwrite semantics) */
extern QueueHandle_t xSensorQueue;

/* Section 28: displayed page selector (enum, not numeric constants) */
typedef enum {
    DISPLAY_TEMPERATURE = 0,
    DISPLAY_HUMIDITY,
    DISPLAY_LIGHT,
    DISPLAY_MOTION
} DisplayMode;

/* Section 29: navigation intent published by InputTask */
typedef enum {
    NAV_NONE = 0,
    NAV_NEXT,       /* clockwise        */
    NAV_PREV        /* counterclockwise */
} NavCommand;

/* Section 59: InputTask -> DisplayTask navigation queue */
extern QueueHandle_t xNavQueue;

extern QueueHandle_t xMotionQueue;

/* Creates all FreeRTOS IPC objects. Call ONCE, before any task is
 * created (Section 41 flow). Returns pdPASS / pdFAIL. */
BaseType_t createRtosObjects(void);

#endif /* RTOS_OBJECTS_H */