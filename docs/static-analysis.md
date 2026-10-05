# Static Analysis Record

Command:

```text
python -m platformio check
```

Result recorded on 2026-10-05: **PASSED**, with 0 high, 0 medium, and 43 low
findings reported by cppcheck.

Most findings are `unusedFunction` reports. PlatformIO analyzes source files
individually, so functions called from other translation units, through FreeRTOS
registration, or through the interrupt vector table can appear unused.

| Finding group | Count | Interpretation | Action |
|---|---:|---|---|
| `unusedFunction` | 25 | Cross-translation-unit calls, task entry points, or interrupt handlers are not visible to the per-file analysis | Review and retain where required by the firmware architecture |
| C-style pointer casts | 14 | HAL, UART, GPIO, or FreeRTOS boundary conversions | Review during cleanup; required in some C/C++ and register interfaces |
| `constParameterPointer` | 3 | Parameters could be more const-correct | Low-priority cleanup |
| `knownConditionTrueFalse` | 1 | `motionDetected` is currently initialized false in `SensorTask` and motion is handled separately | Remove the stale field assignment or merge motion into the sensor sample in a later cleanup |

No high- or medium-severity finding was reported. The low-severity findings
should remain documented until a whole-program analysis confirms whether each
is actionable.
