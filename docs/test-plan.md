# Test Plan and Evidence

## Automated Unit Tests

Command:

```text
python -m platformio test -e native
```

Result recorded on 2026-10-05: **13 test cases passed**.

| Suite | Coverage | Result |
|---|---|---|
| Alarm logic | Below 18.0 C, exactly 18.0 C, normal, exactly 30.0 C, above 30.0 C | PASS |
| Display navigation | Forward, reverse, and wraparound | PASS |
| System state | Active timeout, inactive persistence, and motion reactivation | PASS |

## Firmware Build

Command:

```text
python -m platformio run -e bluepill_f103c8
```

Result recorded on 2026-10-05: **SUCCESS**. Firmware used 33.2% of flash
(21,772 bytes of 65,536) and 68.4% of RAM (14,000 bytes of 20,480).

## Wokwi Functional Tests

Record the actual observation before marking a test PASS.

| ID | Stimulus | Expected result | Actual observation | Result |
|---|---|---|---|---|
| FT-01 | Change DHT22 temperature | Temperature updates on OLED and serial output | To be recorded | Pending |
| FT-02 | Change DHT22 humidity | Humidity updates on OLED and serial output | To be recorded | Pending |
| FT-03 | Change LDR input | Relative light percentage changes | To be recorded | Pending |
| FT-04 | Rotate encoder clockwise | Next display mode is selected | To be recorded | Pending |
| FT-05 | Rotate encoder counterclockwise | Previous display mode is selected | To be recorded | Pending |
| FT-06 | Set temperature above 30 C | Buzzer and `ALARM: ACTIVE` indicator turn on | To be recorded | Pending |
| FT-07 | Return temperature to normal | Buzzer stops and indicator becomes `ALARM: NORMAL` | To be recorded | Pending |
| FT-08 | Trigger PIR | Activity indicator is `STATE: ACTIVE` | To be recorded | Pending |
| FT-09 | Wait 15 seconds without motion | System changes to `STATE: INACTIVE` | To be recorded | Pending |
| FT-10 | Trigger PIR while inactive | System returns to `STATE: ACTIVE` | To be recorded | Pending |

## Fault Experiments

Detailed procedures, expected technical effects, and an evidence table are in
[fault-experiments.md](fault-experiments.md). Each experiment must be
performed temporarily and reverted afterward.
