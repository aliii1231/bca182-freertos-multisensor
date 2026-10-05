# Deliberate FreeRTOS Fault Experiments

These experiments are temporary demonstrations. Run one experiment at a time,
record the observation, then restore the original source and rebuild before
running the next experiment. Do not commit the intentionally faulty version.

## Experiment 1: Remove Blocking

**Purpose:** demonstrate why every continuously running task must block.

1. Choose a periodic task such as `InputTask`.
2. Temporarily remove its `vTaskDelay()` call.
3. Build and run Wokwi while watching the serial monitor and OLED.
4. Observe CPU usage, response latency, and whether lower-priority work runs.
5. Restore the delay and rebuild.

**Expected technical result:** the task remains Ready or Running instead of
entering Blocked. Because it has priority 3, it can prevent lower-priority
DisplayTask work from receiving CPU time. The result may be excessive CPU use,
missed display refreshes, or reduced responsiveness elsewhere.

**Record:** task changed, observed CPU/scheduling effect, and confirmation that
the delay was restored.

## Experiment 2: Change Priority

**Purpose:** demonstrate that priority represents scheduling urgency.

1. Temporarily raise a frequent task such as `InputTask` to the highest
   available application priority.
2. Keep its normal delay in place.
3. Run Wokwi and compare input response, sensor sampling, alarm response, and
   OLED refresh.
4. Restore the original priority of 3 and rebuild.

**Expected technical result:** a higher-priority task is scheduled before lower-
priority tasks whenever it is Ready. A short periodic task may still allow
other tasks to run when it blocks, but any increase in work or reduction in
blocking can make DisplayTask or slower periodic work less responsive.

**Record:** changed priority, visible latency or ordering changes, and the
restored priority.

## Experiment 3: Remove Mutex Protection

**Purpose:** demonstrate the failure mode prevented by the serial mutex.

1. Temporarily bypass `xSerialMutex` in the shared serial-output path.
2. Run the firmware with several tasks producing diagnostic messages.
3. Observe whether messages overlap, fragment, or become difficult to match to
   their task.
4. Restore mutex protection and rebuild.

**Expected technical result:** task context switches can occur while one task is
transmitting a message, allowing another task to write to the same USART.
Output may become interleaved or appear in an unexpected order. The mutex makes
one complete diagnostic write the critical section and restores readable logs.

**Record:** the interleaved output observed, competing tasks, and confirmation
that the mutex was restored.

## Evidence Record

| Experiment | Source change | Observation | Restored | Date |
|---|---|---|---|---|
| Remove blocking | Temporarily removed `InputTask`'s 5 ms `vTaskDelay()` | The Wokwi terminal showed startup and repeated `[MOTION] detected` messages, but normal sensor/display output did not continue and the OLED remained blank. This is consistent with `InputTask` starving lower-priority work while it stays Ready/Running. | Yes; delay restored and normal firmware rebuilt successfully | 2026-10-05 |
| Change priority | Temporarily changed `InputTask` priority from 3 to 7 | Wokwi stopped during startup with `ASSERT failed: .pio\libdeps\bluepill_f103c8\PlatformIO-FreeRTOS\FreeRTOS-Kernel\tasks.c line 1903`. Priority 7 is outside the configured valid application-priority range, so the scheduler did not start normally. | Yes; priority restored to 3 and normal firmware rebuilt successfully | 2026-10-05 |
| Remove mutex | Temporarily bypassed `xSerialMutex` in `UartPrint()` | Wokwi continued to show readable, complete messages such as sensor samples and input-mode changes. No overlapping or fragmented UART output was observed during this run, although the protection was disabled. | Yes; mutex restored and normal firmware rebuilt successfully | 2026-10-05 |
