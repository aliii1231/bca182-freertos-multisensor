**Mindanao State University \- Iligan Institute of Technology**  
**College of Computer Studies**  
**Department of Computer Applications**  
**BCA182 Embedded Systems Programming**

| Last Name: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_ | Date: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_ |
| :---- | :---- |
| First Name: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_ | Subject/Section: \_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_\_ |

**Laboratory Activity No. 1**  
**Real-Time Multisensor Room Monitoring System**  
**100 Points**

**1\. Laboratory Objective**  
This laboratory requires students to design and implement, from scratch, a simulated STM32 room-monitoring system. Students must reconstruct the system requirements, redesign the system for Wokwi, implement a FreeRTOS-based architecture, verify the firmware, document development decisions, and publish the finished work as part of a personal technical portfolio.

**2\. Learning Outcomes**  
At the end of this laboratory activity, the students should be able to:

1. Create an STM32 project using PlatformIO.  
2. Construct and simulate a microcontroller circuit in Wokwi.  
3. Interface sensors and actuators with an STM32.  
4. Organize firmware into multiple source modules.  
5. Create and manage FreeRTOS tasks.  
6. Assign and justify task priorities.  
7. Distinguish Running, Ready, Blocked, Suspended, and Deleted task states.  
8. Use queues for inter-task communication.  
9. Use a mutex to protect a shared resource.  
10. Use an event group or task notification for event signaling.  
11. Implement periodic execution using vTaskDelayUntil().  
12. Implement a simple embedded-system state machine.  
13. Separate hardware-independent decision logic from hardware drivers.  
14. Write and execute automated unit tests using PlatformIO.  
15. Perform static code analysis using PlatformIO.  
16. Use Git incrementally and maintain a professional GitHub repository.  
17. Document a project for both academic assessment and public portfolio use.  
18. Communicate the completed project through Hackster.io.

**3\. System to Develop**  
Students will create a FreeRTOS-Based STM32 Room Multisensor. The simulated device shall monitor room/environmental conditions and provide user interaction using components available in Wokwi.

| Component | Purpose |
| ----- | ----- |
| STM32 Blue Pill | Main microcontroller |
| DHT22 | Temperature and humidity |
| Photoresistor / LDR | Ambient light |
| PIR sensor | Motion detection |
| Rotary encoder | User input |
| SSD1306 OLED | Information display |
| Buzzer | Alarm output |

# **4\. Functional Requirements**

| ID | Requirement | Description |
| ----- | ----- | ----- |
| FR-01 | Temperature measurement | The system shall periodically obtain temperature data. |
| FR-02 | Humidity measurement | The system shall periodically obtain humidity data. |
| FR-03 | Ambient-light measurement | The system shall monitor relative ambient-light level. |
| FR-04 | Motion detection | The system shall detect simulated motion using the PIR sensor. |
| FR-05 | OLED display | The system shall display one selected measurement at a time. |
| FR-06 | Rotary encoder navigation | The rotary encoder shall switch among Temperature, Humidity, Light, and Motion. |
| FR-07 | Temperature alarm | The buzzer shall activate when temperature is outside the configured normal range. |
| FR-08 | Activity state | The system shall support ACTIVE and INACTIVE states. |
| FR-09 | Automatic inactivity | If no motion is detected for a specified duration, the system shall enter INACTIVE. |
| FR-10 | Automatic reactivation | Motion shall return the system to ACTIVE. |

For this laboratory, use the following temperature limits: (i) LOW TEMPERATURE LIMIT  \= 18 °C and (ii)HIGH TEMPERATURE LIMIT \= 30 °C.

# **6\. FreeRTOS Is Mandatory**

This laboratory shall be implemented exclusively using PlatformIO with the STM32Cube framework on the STM32 Blue Pill development board, using C and/or C++, STM32 HAL drivers for peripheral access, and native FreeRTOS APIs for task management and synchronization. The Arduino framework and Arduino-specific abstractions are prohibited. The application shall be designed as a concurrent embedded system. A submission that places nearly all application behavior in a single task or otherwise avoids the required FreeRTOS architecture will be treated as an incomplete submission, hence, will automatically be given 0 points.

# **7\. Minimum Required Tasks**

| Task | Primary Responsibility |
| ----- | ----- |
| SensorTask | Read DHT22 and LDR periodically. |
| DisplayTask | Own and manage the OLED. |
| InputTask | Process rotary-encoder navigation. |
| MotionTask | Monitor PIR activity. |
| AlarmTask | Evaluate alarm state and control the buzzer. |
| StateTask (recommended) | Centralize ACTIVE/INACTIVE state management. |

# **8\. Expected FreeRTOS Architecture**

![][image1]

The actual architecture may differ, but every deviation must be technically defensible and consistent with the implemented code.

# 

# **9\. Required FreeRTOS Concepts**

* Multiple tasks  
* Explicit task priorities  
* Blocking delays  
* vTaskDelayUntil() for at least one periodic task  
* At least one FreeRTOS queue  
* At least one mutex  
* At least one event group or task notification  
* A state machine  
* Inter-task communication  
* Shared-resource protection

Merely declaring a queue, mutex, or event group to satisfy a checklist earns no credit. Each mechanism must have a legitimate role in the implemented design.

# 

# **10\. Development Environment Verification**

Before starting, verify the required development tools. Required firmware environment: STM32Cube with C/C++. Use app\_main() as the application entry point and native STM32Cube/FreeRTOS APIs for GPIO, ADC, I2C, logging, tasks, queues, mutexes, event groups, and timing.  
\- git \--version  
\- pio \--version  
If either command fails, resolve the environment before beginning implementation.

# 

# **PART I — Project Initialization**

## **11\. Create the GitHub Repository**

Create a public GitHub repository. Recommended name: bca182-freertos-multisensor. Avoid vague portfolio repository names such as lab5, final, activity, or project123.

## 

## **12\. Clone the Repository**

\- git clone https://github.com/YOUR\_USERNAME/bca182-freertos-multisensor.git  
\- cd bca182-freertos-multisensor

## 

## **13\. Create the PlatformIO Project**

1. Open PlatformIO Home in Visual Studio Code.  
2. Create a New Project.  
3. Choose an STM32 development board appropriate for Wokwi.  
4. Select STM32Cube as the framework.  
5. Create the project inside the GitHub repository.

The PlatformIO configuration must identify ESP-IDF explicitly. A minimum configuration is:  
\[env:bluepill\_f103c8\]  
platform \= ststm32  
board \= bluepill\_f103c8  
framework \= stm32cube  
monitor\_speed \= 115200

Use of framework \= arduino, Arduino-only libraries, or Arduino-specific APIs is not permitted for this laboratory.

Expected repository structure:  
bca182-freertos-multisensor/  
├── include/  
├── lib/  
├── src/  
├── test/  
├── docs/  
├── platformio.ini  
├── diagram.json  
├── wokwi.toml  
└── README.md

## 

## **14\. First Build**

Compile the empty project before adding sensors or FreeRTOS application logic.  
\- pio run  
Do not proceed until the base project compiles successfully.

## 

## 15\. First Commit

\- git add .  
\- git commit \-m "Initialize STM32 PlatformIO project"  
\- git push

This commit must occur before substantial implementation begins.

# 

# **PART II — Wokwi Simulation**

## **16\. Create the Initial Wokwi Circuit**

Start with the STM32 Blue Pill only. Confirm that the firmware starts in Wokwi and produces a recognizable serial message.  
\- BCA182 FreeRTOS Multisensor  
\- System starting...  
Commit this milestone with a meaningful message such as “Configure initial Wokwi simulation.”

# 

# **PART III — FreeRTOS Foundation**

## **17\. Create Two Simple Tasks First**

Before integrating sensors, create two simple FreeRTOS tasks that periodically print distinct diagnostic messages. Both tasks must block between executions.  
\- Task A running  
\- Task B running  
\- Task A running  
\- Task B running

## 

## **18\. Observe Scheduling and Task States**

Record assigned priority, delay interval, execution frequency, and what occurs while a task is blocked. Be prepared to distinguish Running, Ready, and Blocked states in your own implementation.  
\- SensorTask executing       \-\> Running  
\- SensorTask waiting 2 sec   \-\> Blocked  
\- SensorTask waiting for CPU \-\> Ready

## 

## **19\. Do Not Write Uncontrolled Busy Loops**

A continuously executing task that never blocks can monopolize processor time and interfere with lower-priority work. Each continuously running task must perform finite work and then block, wait, or yield appropriately.

# 

# **PART IV — Sensor Subsystem**

## **20\. Add the DHT22**

Connect the simulated DHT22. Verify temperature and humidity first through the Serial Monitor before integrating the OLED or other tasks.  
\- Temperature: 25.40 C  
\- Humidity: 61.20 %

## 

## **21\. Add the LDR**

Read the photoresistor through an STM32 ADC input. Convert the raw reading into a documented representation such as 0–100%. Do not claim calibrated lux unless the conversion is implemented and justified.

## 

## **22\. Create SensorTask**

Move sensor acquisition into a FreeRTOS task. Use periodic execution. At least one periodic task in the laboratory must use vTaskDelayUntil().

TickType\_t lastWakeTime \= xTaskGetTickCount();

for (;;) {  
    readSensors();  
    vTaskDelayUntil(\&lastWakeTime, pdMS\_TO\_TICKS(2000));  
}

## **23\. Required Explanation: vTaskDelayUntil()**

In the laboratory report, explain why vTaskDelayUntil() is generally more appropriate than vTaskDelay() for periodic sensor sampling. A correct explanation must address periodic timing and drift, not merely state that the function “creates a delay.”

# **PART V — Data Communication**

## **24\. Define Sensor Data**

struct SensorData {  
    float temperature;  
    float humidity;  
    int lightLevel;  
    bool motionDetected;  
};

## 

## **25\. Create a Queue**

Use a FreeRTOS queue to communicate sensor information to one or more consumers. Avoid unsynchronized global variables as the default design.  
SensorTask  
    |  
    v  
Sensor Queue  
    |  
    \+------\> DisplayTask  
    |  
    \+------\> AlarmTask

# 

# **PART VI — Display Subsystem**

## **26\. Add the OLED**

The OLED shall be owned by DisplayTask. Multiple tasks shall not write directly to the OLED unless the student can justify and correctly synchronize that design.

## 

## **27\. Initial OLED Output**

ROOM MONITOR

Temperature  
25.4 C  
Once this basic display works reliably, proceed to user navigation.

# 

# **PART VII — Rotary Encoder**

## **28\. Add the Encoder and InputTask**

Create InputTask. Use an enumeration for the displayed page rather than arbitrary numeric constants.  
enum class DisplayMode {  
    TEMPERATURE,  
    HUMIDITY,  
    LIGHT,  
    MOTION  
};

## 

## **29\. Navigation Behavior**

Clockwise:       Temperature \-\> Humidity \-\> Light \-\> Motion \-\> Temperature  
Counterclockwise: reverse traversal, including wraparound

# 

# **PART VIII — Alarm Subsystem**

## **30\. Create Testable Alarm Logic**

Separate temperature decision logic from buzzer hardware control. Pure decision logic can be unit tested independently of Wokwi and the STM32.  
enum class AlarmState {  
    NORMAL,  
    LOW\_TEMPERATURE,  
    HIGH\_TEMPERATURE  
};

AlarmState evaluateTemperature(float temperature);

# 

# **PART IX — Motion and System State**

## **31\. Add the PIR and MotionTask**

Create MotionTask to monitor motion. For laboratory testing, use a short inactivity timeout such as 15 seconds.

## 

## **32\. Implement the State Machine**

![][image2]

## **33\. ACTIVE Behavior**

* OLED enabled.  
* Normal sensor processing.  
* Encoder active.  
* Alarm active.

## 

## **34\. INACTIVE Behavior**

* OLED off or blank.  
* Unnecessary display operations reduced.  
* Motion detection remains operational.  
* Subsequent PIR activity restores ACTIVE.

# 

# **PART X — Event Group or Task Notification**

## **35\. Use an Event Signaling Mechanism**

Use an event group or task notification for meaningful system events. If an event group is selected, document each bit, its producer, its consumer, and when it is set or cleared.  
\#define EVENT\_ACTIVE BIT0  
\#define EVENT\_MOTION BIT1  
\#define EVENT\_ALARM  BIT2

# 

# **PART XI — Mutex**

## **36\. Protect a Shared Resource**

A suitable beginner example is shared Serial output. If multiple tasks print diagnostics, use a mutex to prevent interleaved writes.  
xSemaphoreTake(serialMutex, portMAX\_DELAY);  
// Serial output  
xSemaphoreGive(serialMutex);

## 

## **37\. Required Mutex Explanation**

The laboratory report must identify the actual shared resource, the competing tasks, and the failure mode that the mutex prevents. “A mutex prevents race conditions” by itself is insufficient.

# 

# **PART XII — Task Priorities**

## **38\. Assign Explicit Priorities**

| Task | Suggested Starting Priority |
| ----- | ----- |
| MotionTask | 3 |
| InputTask | 3 |
| SensorTask | 2 |
| AlarmTask | 2 |
| DisplayTask | 1 |

Students may choose different priorities, but must justify them in terms of scheduling urgency and acceptable latency.

## **39\. Priority Is Scheduling Urgency**

Avoid explanations such as “MotionTask has priority 3 because it is important.” Instead explain which tasks require prompt response, which can tolerate latency, and what could happen if priorities are poorly selected.

# **PART XIII — Modular Software Design**

## **40\. Required Source Organization**

include/  
├── sensors.h  
├── display.h  
├── input.h  
├── alarm.h  
├── motion.h  
├── system\_state.h  
└── rtos\_objects.h

src/  
├── main.cpp  
├── sensors.cpp  
├── display.cpp  
├── input.cpp  
├── alarm.cpp  
├── motion.cpp  
├── system\_state.cpp  
└── rtos\_objects.cpp

Exact organization may differ if technically justified. The entire application shall not be placed in main.cpp.

## 

## **41\. Keep main.cpp Focused**

hardware initialization  
        ↓  
FreeRTOS object creation  
        ↓  
task creation  
        ↓  
scheduler-driven operation

# 

# **PART XIV — Unit Testing**

## **42\. What to Unit Test**

Begin with deterministic, hardware-independent application logic. Recommended targets include:  
evaluateTemperature()  
nextDisplayMode()  
previousDisplayMode()  
evaluateSystemState()

## 

## **43\. Minimum Unit Tests**

| Category | Minimum | Required Coverage |
| ----- | ----- | ----- |
| Temperature alarm logic | 5 | Below lower threshold; exactly lower threshold; normal value; exactly upper threshold; above upper threshold. |
| Display navigation | 4 | Forward/reverse transitions and wraparound. |
| System state | 4 | ACTIVE/no timeout; ACTIVE/timeout; INACTIVE/no motion; INACTIVE/motion. |

Minimum total: 13 meaningful unit tests. Trivial tests that do not exercise decision logic will not satisfy the requirement.

## 

## **44\. Run Unit Tests**

pio test  
All final tests must pass. Screenshots do not substitute for test source files committed to the repository.

# 

# **PART XV — Static Code Analysis**

## **45\. Run PlatformIO Check**

pio check  
Students must inspect and interpret findings rather than merely show command output.

## 

## **46\. Static Analysis Findings Table**

| Finding | File/Line | Cause | Resolution |
| ----- | ----- | ----- | ----- |
| Example: unused variable | sensors.cpp:xx | Obsolete variable | Removed |
| Example: conversion warning | display.cpp:xx | Implicit conversion | Type corrected |

# **PART XVI — Functional Verification in Wokwi**

## **47\. Required Functional Tests**

| ID | Stimulus | Expected Result |
| ----- | ----- | ----- |
| FT-01 | Change temperature | Displayed temperature updates. |
| FT-02 | Change humidity | Displayed humidity updates. |
| FT-03 | Change light input | Light value changes. |
| FT-04 | Rotate encoder clockwise | Next page is selected. |
| FT-05 | Rotate encoder counterclockwise | Previous page is selected. |
| FT-06 | Set temperature above 30 °C | Alarm activates. |
| FT-07 | Return temperature to normal | Alarm stops. |
| FT-08 | Trigger PIR | System is ACTIVE. |
| FT-09 | Allow inactivity timeout | System becomes INACTIVE. |
| FT-10 | Trigger PIR while INACTIVE | System returns to ACTIVE. |

## **48\. Verification Record**

| Test ID | Input/Stimulus | Expected | Actual | Result |
| ----- | ----- | ----- | ----- | ----- |
| FT-01 | Example: 28 °C | 28 °C displayed | \[record observation\] | PASS/FAIL |

A test shall not be marked PASS without recording the actual observed behavior.

# 

# **PART XVII — Deliberate FreeRTOS Fault Experiments**

## **49\. Fault Experiment 1 — Remove Blocking**

Temporarily remove the blocking delay from a continuously executing task. Observe scheduling and responsiveness. Explain CPU usage implications, task starvation risk, and the effect on other tasks. Restore the correct implementation afterward.

## 

## **50\. Fault Experiment 2 — Change Priority**

Temporarily assign an unnecessarily high priority to a task that performs frequent work. Observe whether other tasks become less responsive. Explain why. Restore the intended priority afterward.

## 

## **51\. Fault Experiment 3 — Remove Mutex**

Temporarily remove Serial protection. Observe whether output becomes interleaved. Explain the result and restore the mutex afterward.

# 

# **PART XVIII — Git and GitHub**

## **52\. Git Is Part of the Laboratory**

GitHub is not a final upload location. Commit after meaningful engineering milestones so that the repository documents development history.

- Initialize STM32 PlatformIO project  
- Configure Wokwi simulation  
- Add initial FreeRTOS tasks  
- Implement DHT22 sensor acquisition  
- Add LDR measurement  
- Add sensor data queue  
- Implement OLED display task  
- Add rotary encoder navigation  
- Implement alarm task  
- Add PIR motion monitoring  
- Add system state machine  
- Add FreeRTOS event group  
- Protect serial output with mutex  
- Add alarm unit tests  
- Add navigation unit tests  
- Add state machine unit tests  
- Resolve static analysis findings  
- Complete Wokwi verification  
- Finalize technical documentation

## 

## 53\. Unacceptable Commit History

- update  
- changes  
- working  
- final  
- final2  
- finalfinal

A repository containing only one final bulk commit demonstrates little or no development process.

# 

# **PART XIX — Public README.md**

## **54\. Purpose of the README**

The README is not the laboratory report. It is the public technical introduction to the portfolio project. Assume the reader is an embedded systems engineer, potential employer, internship evaluator, or another developer.

## **55\. Required README Structure**

\# Project Title

\#\# Project Overview  
\#\# Features  
\#\# Learning Objectives  
\#\# System Architecture  
\#\# FreeRTOS Architecture  
\#\# Hardware / Simulated Components  
\#\# Pin Configuration  
\#\# Task Design  
\#\# Inter-Task Communication  
\#\# State Machine  
\#\# Repository Structure  
\#\# Getting Started  
\#\# Building the Project  
\#\# Running the Wokwi Simulation  
\#\# Unit Testing  
\#\# Static Code Analysis  
\#\# Functional Verification  
\#\# Engineering Decisions  
\#\# Limitations  
\#\# Future Improvements  
\#\# References and Acknowledgments

## 

## **56\. Required README Visuals**

1. Wokwi circuit image.  
2. System architecture diagram.  
3. FreeRTOS task-communication diagram.  
4. State-machine diagram.  
5. Finished-system screenshot.

Every figure must support a technical point and use a clear caption. Decorative screenshots do not substitute for technical documentation.

# 

# **PART XX — Laboratory Report**

## **57\. Separate Academic Report**

The academic report must be separate from the public README and stored in the repository as:  
docs/laboratory-report.pdf  
The README showcases the project. The report demonstrates technical reasoning and evidence.

## 

## **58\. Required Report Sections**

| Section | Required Content |
| ----- | ----- |
| 1\. Problem and Requirements | Explain the problem, the required system behavior, and the Wokwi adaptation. |
| 2\. System Architecture and Design | Hardware architecture, software architecture, subsystem decomposition, state machine. |
| 3\. FreeRTOS Architecture | Tasks, priorities, periods/events, task states, communication, synchronization. |
| 4\. Implementation | Explain significant implementation decisions; do not narrate source code line-by-line. |
| 5\. Verification and Testing | Unit tests, Wokwi functional tests, deliberate fault experiments. |
| 6\. Static Code Analysis | Findings, severity, interpretation, and corrective actions. |
| 7\. Engineering Discussion | Trade-offs, limitations, debugging, problems encountered, alternative approaches. |
| 8\. Conclusion | What was learned and what should be improved. |

## **59\. Required FreeRTOS Task Table**

| Task | Responsibility | Trigger / Period | Priority | IPC | Typical Blocked Condition |
| ----- | ----- | ----- | ----- | ----- | ----- |
| SensorTask | Measurements | 2 s | 2 | Queue | Delay |
| DisplayTask | OLED | event/update | 1 | Queue | Waiting for data |
| InputTask | Encoder | short periodic/event | 3 | Notification/queue | Delay/wait |
| MotionTask | PIR | periodic/event | 3 | Event group | Delay/wait |
| AlarmTask | Alarm logic | sensor update | 2 | Queue | Waiting for data |

The table must be accompanied by technical justification, not merely reproduced as a checklist.

# PART XXI — Requirements Traceability

## 60\. Requirements Traceability Matrix

| Requirement | Implementation | Verification |
| ----- | ----- | ----- |
| FR-01 | SensorTask | FT-01 |
| FR-05 | DisplayTask | FT-01–FT-03 |
| FR-06 | InputTask | Unit navigation tests \+ FT-04/FT-05 |
| FR-07 | AlarmTask | Unit alarm tests \+ FT-06/FT-07 |
| FR-09 | MotionTask / StateTask | Unit state tests \+ FT-09 |

Every major requirement must be traceable to implementation and verification evidence.

# 

# **PART XXII — Hackster.io Portfolio Publication**

## **61\. Publish Only After Technical Completion**

Before creating the Hackster.io post, complete the final technical verification and push the final source code to GitHub.  
pio run  
pio test  
pio check  
git status  
git push

## 

## **62\. Purpose of the Hackster.io Post**

The Hackster.io article is the public project showcase. It should explain what was built, why it matters, how it works, how FreeRTOS is used, what was learned, and where the complete source code can be found. Add me as a collaborator, Paul Rodolf P. Castor or use my email: paulrodolf.castor@g.msuiit.edu.ph, in your project for BSCA program accreditation purposes.

## 

## **63\. Recommended Hackster.io Sections**

* Project Overview  
* Motivation  
* Features  
* Components  
* Circuit  
* System Architecture  
* FreeRTOS Architecture  
* How It Works  
* Testing and Verification  
* Demonstration  
* Challenges Encountered  
* Lessons Learned  
* Limitations  
* Future Improvements  
* GitHub Repository  
* References

# 

# **PART XXIV — Individual Technical Checkoff**

## 64\. Technical Defense

Each student must be prepared for a brief technical defense. A working simulation does not, by itself, prove understanding.

1. Why did you create SensorTask?  
2. Why does each task have its assigned priority?  
3. What does vTaskDelayUntil() do?  
4. What happens to a task while it is delayed?  
5. What information crosses your queue?  
6. Why did you use a queue rather than unsynchronized global variables?  
7. What resource does your mutex protect?  
8. Where could a race condition occur?  
9. What does your event group or notification represent?  
10. Which task owns the OLED, and why?  
11. What happens if a high-priority task never blocks?  
12. What is the difference between Ready and Blocked?  
13. What functionality did your unit tests actually verify?  
14. What did static analysis discover?  
15. What would differ if this system ran on physical hardware?

# 

# **PART XXV — Final Submission Checklist**

## 65\. Submission Checklist

| Category | Requirement |
| ----- | ----- |
| Build | ☐ pio run succeeds |
| FreeRTOS | ☐ At least five meaningful tasks |
| FreeRTOS | ☐ Explicit priorities |
| FreeRTOS | ☐ Queue |
| FreeRTOS | ☐ Mutex |
| FreeRTOS | ☐ Event group or task notification |
| FreeRTOS | ☐ vTaskDelayUntil() used appropriately |
| FreeRTOS | ☐ ACTIVE/INACTIVE state machine |
| Simulation | ☐ DHT22, LDR, PIR, rotary encoder, OLED, buzzer |
| Simulation | ☐ Wokwi simulation operational |
| Testing | ☐ At least 13 meaningful unit tests |
| Testing | ☐ pio test succeeds |
| Testing | ☐ Functional verification table completed |
| Testing | ☐ Fault experiments completed |
| Quality | ☐ pio check completed |
| Quality | ☐ Findings analyzed and significant warnings addressed |
| GitHub | ☐ Public repository |
| GitHub | ☐ Meaningful commit history |
| GitHub | ☐ Professional README |
| GitHub | ☐ Architecture diagrams |
| GitHub | ☐ No unnecessary binaries/build artifacts |
| Report | ☐ Laboratory report included |
| Report | ☐ FreeRTOS task table |
| Report | ☐ Requirements traceability matrix |
| Report | ☐ Test results and static-analysis discussion |
| Report | ☐ Limitations documented |
| Portfolio | ☐ Hackster.io article |
| Portfolio | ☐ GitHub link and attribution included |

# **66\. Grounds for Rejection or Major Deduction**

Submissions may receive severe deductions (i.e., \-5 points each) or be returned with 0 points when any of the following apply:

* The project does not compile.  
* The Wokwi simulation does not run.  
* Most functionality is placed in one task.  
* FreeRTOS objects exist only to satisfy the checklist.  
* Task priorities cannot be justified.  
* A task contains an uncontrolled busy loop.  
* Shared resources are accessed unsafely.  
* The student cannot explain the queue, mutex, event group, or task notification.  
* Unit tests are trivial or unrelated to requirements.  
* pio check output is shown without interpretation.  
* Screenshots are substituted for analysis.  
* The README is incomplete or written merely as an academic worksheet.  
* Git history contains only a final bulk commit.  
* Architecture diagrams disagree with source code.  
* Test results cannot be reproduced.  
* The Hackster.io article makes unsupported technical or performance claims.  
* External material is used without attribution.  
* The student cannot explain the submitted implementation during technical checkoff.

# 

# **67\. Grading System for this laboratory activity**

| Criterion | Weight |
| ----- | ----- |
| System implementation and Wokwi functionality | 15% |
| FreeRTOS task architecture | 15% |
| Scheduling and priority understanding | 10% |
| IPC and synchronization | 10% |
| Unit testing | 10% |
| Static analysis and code quality | 10% |
| Laboratory report and engineering analysis | 10% |
| Git/GitHub engineering practice | 8% |
| Public README / portfolio quality | 7% |
| Hackster.io technical publication | 5% |
| TOTAL | 100% |

The technical checkoff functions as an authenticity and understanding gate. Serious inability to explain the submitted FreeRTOS architecture may cap the corresponding FreeRTOS marks even if the simulation appears to work. 0 points will automatically given if you cannot explain your own work.  


[image1]: <data:image/png;base64,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>

[image2]: <data:image/png;base64,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>