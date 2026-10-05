# Laboratory Report

## 1. Problem and Requirements

This project implements a FreeRTOS-based room-monitoring system for an
STM32F103C8T6 Blue Pill. The system measures temperature, humidity, relative
ambient light, and motion. It displays one selected measurement, accepts rotary
encoder navigation, activates a buzzer outside the 18.0 C to 30.0 C normal
range, and changes between ACTIVE and INACTIVE activity states.

The system is simulated in Wokwi using a DHT22, photoresistor, PIR sensor,
KY-040 rotary encoder, SSD1306 OLED, and buzzer.

## 2. System Architecture and Design

The hardware and software architecture are documented in
[architecture.mmd](architecture.mmd). `main.cpp` performs HAL initialization,
creates the FreeRTOS objects and tasks, and starts the scheduler. Hardware
modules remain separate from decision logic where practical.

The system state machine is documented in [state-machine.mmd](state-machine.mmd).
The system starts ACTIVE, enters INACTIVE after 15 seconds without motion, and
returns to ACTIVE when motion is detected.

## 3. FreeRTOS Architecture

| Task | Responsibility | Period or trigger | Priority | IPC | Blocked condition |
|---|---|---|---:|---|---|
| SensorTask | DHT22 and LDR measurements | 2500 ms | 2 | Sensor queue | Periodic delay |
| DisplayTask | OLED ownership and rendering | 500 ms | 1 | Sensor/nav queues, event group | Periodic delay |
| InputTask | Rotary encoder navigation | 5 ms | 3 | Navigation queue | Periodic delay |
| MotionTask | PIR monitoring | 100 ms | 3 | Motion queue, event group | Periodic delay |
| AlarmTask | Temperature alarm and buzzer | 2500 ms | 2 | Sensor queue, event group | Queue wait/delay |
| StateTask | ACTIVE/INACTIVE state management | 100 ms | 2 | Motion queue, event group | Periodic delay |

`vTaskDelayUntil()` is used for periodic sensor and display execution. Queues
carry `SensorData`, navigation commands, and the latest motion state. The
serial mutex protects shared diagnostic output. `EVENT_ALARM_BIT2` is a level
signal owned by `AlarmTask`: it is set while the latest valid temperature is
outside the normal range and cleared when the temperature is normal.

## 4. Implementation

the OLED. It displays the selected measurement together with `STATE: ACTIVE` or
`SensorTask` reads and validates the DHT22 result, copies the latest PIR state
from `xMotionQueue`, and scales the ADC reading to a relative 0-100% light
value. `DisplayTask` is the only task that writes to the OLED. It displays the
selected measurement together with `STATE: ACTIVE` or `STATE: INACTIVE` and
`ALARM: ACTIVE` or `ALARM: NORMAL`.

The alarm decision is separated into `evaluateTemperature()`. `AlarmTask`
evaluates the latest sample every 100 ms, publishes `EVENT_ALARM_BIT2`, and
controls a 1 kHz TIM1 PWM signal on PA8 while active to generate an audible
tone. The native test build exercises the corresponding hardware-independent
logic.
Navigation and state transitions are also represented by pure logic functions.

## 5. Verification and Testing

The automated test and firmware-build evidence is recorded in
[test-plan.md](test-plan.md). On 2026-10-05, all 13 native Unity tests passed
and the `bluepill_f103c8` firmware build succeeded. The deliberate blocking,
priority, and mutex experiments are specified in
[fault-experiments.md](fault-experiments.md); their observations remain to be
recorded during the final Wokwi session.

Wokwi functional tests FT-01 through FT-10 have now been observed and
recorded as PASS. The DHT22 produced changed
temperature and humidity readings, the photoresistor produced a changed light
percentage, and at 49.40 C the alarm remained active even while the system was
inactive. The PIR activated the system, inactivity produced `STATE: INACTIVE`,
and motion while inactive returned the system to `STATE: ACTIVE`. During the
PIR test, the event message appeared immediately and the following periodic
samples reported `Motion: yes`; earlier `Motion: no` samples were caused by
the sampling interval. Encoder rotation also selected MOTION clockwise and
TEMPERATURE counterclockwise. Returning the temperature to 18.00 C cleared the
alarm and produced `ALARM: NORMAL`. The three required FreeRTOS fault
experiments are also listed in the test plan.

## 6. Static Code Analysis

The `python -m platformio check` result is documented in
[static-analysis.md](static-analysis.md). The run passed with no high- or
medium-severity findings and 43 low-severity findings. The low findings are
primarily per-file unused-function reports and boundary casts that require
review in the context of the embedded build.

## 7. Engineering Discussion

The design uses latest-value queues because consumers need current sensor and
motion state rather than a historical stream. The OLED has a single owning task
to avoid competing I2C transactions. The event group provides shared state
signals without copying sensor data between tasks.

The main limitations are that the LDR value is relative rather than calibrated
lux, Wokwi is not physical hardware. The current design also keeps motion
separate from the `SensorData` sample and uses the event group for the
centralized alarm level.

## 8. Conclusion

The project demonstrates a modular STM32 FreeRTOS architecture with six
cooperating tasks, queues, an event group, a mutex, periodic scheduling, a
state machine, and host-based decision-logic tests. The next verification step
is to perform the fault experiments and export this report to
`docs/laboratory-report.pdf` for final submission.

## Requirements Traceability

| Requirement | Implementation | Verification |
|---|---|---|
| FR-01 / FR-02 | SensorTask and DHT22 driver | FT-01 / FT-02 |
| FR-03 | SensorTask ADC scaling | FT-03 |
| FR-04 | MotionTask and PIR | FT-08 / FT-10 |
| FR-05 | DisplayTask and OLED driver | FT-01 through FT-03 |
| FR-06 | InputTask and navigation logic | Unit tests / FT-04 / FT-05 |
| FR-07 | AlarmTask and `EVENT_ALARM_BIT2` | Unit tests / FT-06 / FT-07 |
| FR-08 / FR-09 / FR-10 | StateTask and event group | Unit tests / FT-08 through FT-10 |
