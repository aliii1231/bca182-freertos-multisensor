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
| FT-01 | Change DHT22 temperature | Temperature updates on OLED and serial output | DHT22 temperature was changed to 11.80 C, and the serial monitor repeatedly reported `Temperature: 11.80 C`. | PASS |
| FT-02 | Change DHT22 humidity | Humidity updates on OLED and serial output | DHT22 humidity was changed to 21.00 %, and the serial monitor repeatedly reported `Humidity: 21.00 %`. | PASS |
| FT-03 | Change LDR input | Relative light percentage changes | The photoresistor was adjusted to 95499 lux, and the serial monitor reported `Light: 1 %`. | PASS |
| FT-04 | Rotate encoder clockwise | Next display mode is selected | The encoder was rotated clockwise. The serial monitor showed `[INPUT] mode: MOTION`, and the OLED changed to the MOTION screen. | PASS |
| FT-05 | Rotate encoder counterclockwise | Previous display mode is selected | The encoder was rotated counterclockwise. The serial monitor showed `[INPUT] mode: TEMPERATURE`, and the OLED changed back to the TEMPERATURE screen. | PASS |
| FT-06 | Set temperature above 30 C | Buzzer and `ALARM: ACTIVE` indicator turn on | DHT22 temperature was set to 49.40 C. The OLED showed `ALARM: ACTIVE` while the system was `STATE: INACTIVE`, and the buzzer indicator was visible. | PASS |
| FT-07 | Return temperature to normal | Buzzer stops and indicator becomes `ALARM: NORMAL` | DHT22 temperature was returned to 18.00 C, the normal-range boundary. The OLED showed `ALARM: NORMAL` and the alarm indicator was cleared while the serial monitor reported `Temperature: 18.00 C`. | PASS |
| FT-08 | Trigger PIR | Activity indicator is `STATE: ACTIVE` | Wokwi PIR motion was simulated. The serial monitor showed `[MOTION] detected`, followed by `[STATE] ACTIVE`. The next sensor samples reported `Motion: yes` while the OLED showed the active state. Earlier samples reported `Motion: no` because sensor samples are periodic and the PIR signal is time-dependent. | PASS |
| FT-09 | Wait 15 seconds without motion | System changes to `STATE: INACTIVE` | With no motion, the serial monitor showed `[STATE] INACTIVE`, and the OLED showed `STATE: INACTIVE`. | PASS |
| FT-10 | Trigger PIR while inactive | System returns to `STATE: ACTIVE` | Motion was simulated while inactive. The serial monitor showed `[MOTION] detected`, and the OLED returned to `STATE: ACTIVE`. | PASS |

## Fault Experiments

Detailed procedures, expected technical effects, and an evidence table are in
[fault-experiments.md](fault-experiments.md). Each experiment must be
performed temporarily and reverted afterward.
