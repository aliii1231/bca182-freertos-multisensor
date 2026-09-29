#include <unity.h>
#include "decision_logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_alarm_below_lower_limit(void) { TEST_ASSERT_EQUAL_INT((int)LogicAlarmState::LOW_TEMPERATURE, (int)evaluateTemperatureLogic(17.9f)); }
void test_alarm_at_lower_limit(void) { TEST_ASSERT_EQUAL_INT((int)LogicAlarmState::NORMAL, (int)evaluateTemperatureLogic(18.0f)); }
void test_alarm_normal_temperature(void) { TEST_ASSERT_EQUAL_INT((int)LogicAlarmState::NORMAL, (int)evaluateTemperatureLogic(25.4f)); }
void test_alarm_at_upper_limit(void) { TEST_ASSERT_EQUAL_INT((int)LogicAlarmState::NORMAL, (int)evaluateTemperatureLogic(30.0f)); }
void test_alarm_above_upper_limit(void) { TEST_ASSERT_EQUAL_INT((int)LogicAlarmState::HIGH_TEMPERATURE, (int)evaluateTemperatureLogic(30.1f)); }

void test_navigation_forward(void) { TEST_ASSERT_EQUAL_INT((int)LogicDisplayMode::HUMIDITY, (int)nextDisplayModeLogic(LogicDisplayMode::TEMPERATURE)); }
void test_navigation_reverse(void) { TEST_ASSERT_EQUAL_INT((int)LogicDisplayMode::LIGHT, (int)previousDisplayModeLogic(LogicDisplayMode::MOTION)); }
void test_navigation_forward_wrap(void) { TEST_ASSERT_EQUAL_INT((int)LogicDisplayMode::TEMPERATURE, (int)nextDisplayModeLogic(LogicDisplayMode::MOTION)); }
void test_navigation_reverse_wrap(void) { TEST_ASSERT_EQUAL_INT((int)LogicDisplayMode::MOTION, (int)previousDisplayModeLogic(LogicDisplayMode::TEMPERATURE)); }

void test_state_active_without_timeout(void) { TEST_ASSERT_EQUAL_INT((int)LogicSystemState::ACTIVE, (int)evaluateSystemStateLogic(LogicSystemState::ACTIVE, false, 14999, 15000)); }
void test_state_active_with_timeout(void) { TEST_ASSERT_EQUAL_INT((int)LogicSystemState::INACTIVE, (int)evaluateSystemStateLogic(LogicSystemState::ACTIVE, false, 15000, 15000)); }
void test_state_inactive_without_motion(void) { TEST_ASSERT_EQUAL_INT((int)LogicSystemState::INACTIVE, (int)evaluateSystemStateLogic(LogicSystemState::INACTIVE, false, 20000, 15000)); }
void test_state_motion_reactivates(void) { TEST_ASSERT_EQUAL_INT((int)LogicSystemState::ACTIVE, (int)evaluateSystemStateLogic(LogicSystemState::INACTIVE, true, 20000, 15000)); }

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_alarm_below_lower_limit);
    RUN_TEST(test_alarm_at_lower_limit);
    RUN_TEST(test_alarm_normal_temperature);
    RUN_TEST(test_alarm_at_upper_limit);
    RUN_TEST(test_alarm_above_upper_limit);
    RUN_TEST(test_navigation_forward);
    RUN_TEST(test_navigation_reverse);
    RUN_TEST(test_navigation_forward_wrap);
    RUN_TEST(test_navigation_reverse_wrap);
    RUN_TEST(test_state_active_without_timeout);
    RUN_TEST(test_state_active_with_timeout);
    RUN_TEST(test_state_inactive_without_motion);
    RUN_TEST(test_state_motion_reactivates);
    return UNITY_END();
}