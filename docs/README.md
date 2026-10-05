# Project Documentation

This directory contains the technical evidence and design material for the
BCA182 FreeRTOS multisensor project.

## Contents

- [architecture.mmd](architecture.mmd): hardware and peripheral architecture.
- [task-communication.mmd](task-communication.mmd): FreeRTOS tasks and IPC.
- [state-machine.mmd](state-machine.mmd): ACTIVE/INACTIVE state transitions.
- [test-plan.md](test-plan.md): unit and Wokwi functional verification record.
- [fault-experiments.md](fault-experiments.md): deliberate FreeRTOS fault experiments.
- [static-analysis.md](static-analysis.md): PlatformIO cppcheck findings.
- [laboratory-report.md](laboratory-report.md): structured laboratory report draft.
- [laboratory-report.html](laboratory-report.html): print-ready formatted report for PDF export.

The `.mmd` files can be rendered with Mermaid-compatible tooling. Wokwi
screenshots have been added under `screenshots/`; exported diagram images may
be added here if required for the final submission.
