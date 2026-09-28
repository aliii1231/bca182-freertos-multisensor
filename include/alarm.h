#ifndef ALARM_H
#define ALARM_H

#include "FreeRTOS.h"

enum class AlarmState {
    NORMAL,
    LOW_TEMPERATURE,
    HIGH_TEMPERATURE
};

AlarmState evaluateTemperature(float temperature);
void AlarmTask(void *pvParameters);

#endif