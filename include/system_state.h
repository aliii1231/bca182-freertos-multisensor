#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <stdbool.h>
#include <stdint.h>

enum class SystemState {
    ACTIVE,
    INACTIVE
};

SystemState evaluateSystemState(SystemState current, bool motion, uint32_t elapsedMs);
void StateTask(void *pvParameters);

#endif