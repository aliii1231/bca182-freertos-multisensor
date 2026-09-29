#ifndef DECISION_LOGIC_H
#define DECISION_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

enum class LogicDisplayMode {
    TEMPERATURE,
    HUMIDITY,
    LIGHT,
    MOTION
};

enum class LogicAlarmState {
    NORMAL,
    LOW_TEMPERATURE,
    HIGH_TEMPERATURE
};

enum class LogicSystemState {
    ACTIVE,
    INACTIVE
};

LogicAlarmState evaluateTemperatureLogic(float temperature);
LogicDisplayMode nextDisplayModeLogic(LogicDisplayMode current);
LogicDisplayMode previousDisplayModeLogic(LogicDisplayMode current);
LogicSystemState evaluateSystemStateLogic(LogicSystemState current,
                                          bool motion,
                                          uint32_t elapsedMs,
                                          uint32_t timeoutMs);

#endif