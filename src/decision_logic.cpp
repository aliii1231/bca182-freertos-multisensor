#include "decision_logic.h"

LogicAlarmState evaluateTemperatureLogic(float temperature)
{
    if (temperature < 18.0f) return LogicAlarmState::LOW_TEMPERATURE;
    if (temperature > 30.0f) return LogicAlarmState::HIGH_TEMPERATURE;
    return LogicAlarmState::NORMAL;
}

LogicDisplayMode nextDisplayModeLogic(LogicDisplayMode current)
{
    switch (current) {
    case LogicDisplayMode::TEMPERATURE: return LogicDisplayMode::HUMIDITY;
    case LogicDisplayMode::HUMIDITY: return LogicDisplayMode::LIGHT;
    case LogicDisplayMode::LIGHT: return LogicDisplayMode::MOTION;
    case LogicDisplayMode::MOTION: return LogicDisplayMode::TEMPERATURE;
    }
    return LogicDisplayMode::TEMPERATURE;
}

LogicDisplayMode previousDisplayModeLogic(LogicDisplayMode current)
{
    switch (current) {
    case LogicDisplayMode::TEMPERATURE: return LogicDisplayMode::MOTION;
    case LogicDisplayMode::HUMIDITY: return LogicDisplayMode::TEMPERATURE;
    case LogicDisplayMode::LIGHT: return LogicDisplayMode::HUMIDITY;
    case LogicDisplayMode::MOTION: return LogicDisplayMode::LIGHT;
    }
    return LogicDisplayMode::TEMPERATURE;
}

LogicSystemState evaluateSystemStateLogic(LogicSystemState current,
                                          bool motion,
                                          uint32_t elapsedMs,
                                          uint32_t timeoutMs)
{
    if (motion) return LogicSystemState::ACTIVE;
    if (current == LogicSystemState::ACTIVE && elapsedMs >= timeoutMs) {
        return LogicSystemState::INACTIVE;
    }
    return current;
}