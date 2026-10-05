/**
 * BCA182 - Laboratory Activity No. 1
 * PART VI: Display Subsystem (S26-S27)
 *   - OLED (SSD1306 I2C1) owned exclusively by DisplayTask
 *   - DisplayTask replaces LogTask as the sensor-queue consumer
 *   - I2C_Scan(): bus bring-up + address scan diagnostic (S26 bring-up)
 * PART V: SensorData struct + latest-value queue (rtos_objects)
 * PART IV: DHT22 + LDR sensor acquisition (S20-S22)
 * PART III: FreeRTOS foundation (S17)
 * PART XI: Mutex protecting shared UART serial output
 *
 * Hardware init -> app_main() -> object creation -> task creation ->
 * scheduler (S10/S41). HAL timebase on TIM4; SysTick = FreeRTOS.
 */
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "dht22.h"
#include "rtos_objects.h"
#include "oled.h"
#include "alarm.h"
#include "motion.h"
#include "system_state.h"
#include "input.h"

/* 1 = scheduler counters + status line (debugging)
 * 0 = clean lab output                                          */
#define DIAGNOSTIC_MODE  0

UART_HandleTypeDef huart1;
ADC_HandleTypeDef  hadc1;
I2C_HandleTypeDef  hi2c1;
TIM_HandleTypeDef  htim1;

extern "C" uint32_t g_pfnVectors[];

/* Instrumented port counters (src/port.c) — diagnostic only */
extern "C" volatile uint32_t g_portFirstLaunchCount;
extern "C" volatile uint32_t g_portYieldCount;
extern "C" volatile uint32_t g_portTickSwitchCount;
extern "C" volatile uint32_t ulPortSwitchPending;
extern "C" uint32_t ulPortGetCriticalNesting(void);

void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_BUZZER_Init(void);
static void MX_PIR_Init(void);
void app_main(void);

static DisplayMode nextDisplayMode(DisplayMode current)
{
    return current == DISPLAY_MOTION
        ? DISPLAY_TEMPERATURE
        : (DisplayMode)((int)current + 1);
}

static DisplayMode previousDisplayMode(DisplayMode current)
{
    return current == DISPLAY_TEMPERATURE
        ? DISPLAY_MOTION
        : (DisplayMode)((int)current - 1);
}

static const char *displayModeName(DisplayMode mode)
{
    switch (mode) {
    case DISPLAY_TEMPERATURE: return "TEMPERATURE";
    case DISPLAY_HUMIDITY:    return "HUMIDITY";
    case DISPLAY_LIGHT:      return "LIGHT";
    case DISPLAY_MOTION:     return "MOTION";
    default:                 return "UNKNOWN";
    }
}

/* ---------------------------------------------------------
 * UART output (USART1: PA9 = TX, PA10 = RX -> Wokwi terminal).
 * Protected by xSerialMutex for Part XI (Mutex).
 * --------------------------------------------------------- */
static void UartPrint(const char *s)
{
    if (xSerialMutex != NULL) {
        if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
            HAL_UART_Transmit(&huart1, (uint8_t *)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
            xSemaphoreGive(xSerialMutex);
        }
    } else {
        HAL_UART_Transmit(&huart1, (uint8_t *)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
    }
}

static void UartPrintNum(unsigned long val)
{
    char digits[12];
    int di = 0;
    if (val == 0) digits[di++] = '0';
    while (val > 0) {
        digits[di++] = (char)('0' + (val % 10));
        val /= 10;
    }
    char out[12];
    int oi = 0;
    while (di > 0) out[oi++] = digits[--di];
    out[oi] = '\0';
    UartPrint(out);
}

static void UartPrintHex(uint32_t v)
{
    char b[11];
    b[0] = '0'; b[1] = 'x';
    for (int i = 0; i < 8; i++) {
        uint32_t n = (v >> (28 - 4 * i)) & 0xF;
        b[2 + i] = (char)(n < 10 ? '0' + n : 'A' + (n - 10));
    }
    b[10] = '\0';
    UartPrint(b);
}

#if DIAGNOSTIC_MODE
static void PrintStatus(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) return;

    UartPrint("[STATUS] tick=");   UartPrintNum((unsigned long)xTaskGetTickCountFromISR());
    UartPrint(" sched=");          UartPrintNum((unsigned long)xTaskGetSchedulerState());
    UartPrint(" stCtrl=");         UartPrintHex(*(volatile uint32_t *)0xE000E010);
    UartPrint(" stVal=");          UartPrintNum(*(volatile uint32_t *)0xE000E018);
    UartPrint(" pend=");           UartPrintNum(ulPortSwitchPending);
    UartPrint(" crit=");           UartPrintHex(ulPortGetCriticalNesting());
    UartPrint(" sw=");             UartPrintNum(g_portTickSwitchCount);
    UartPrint(" cur=");            UartPrint(pcTaskGetName(NULL));
    UartPrint("\r\n");
}
#endif

/* ---------------------------------------------------------
 * HAL timebase on TIM4 — SysTick belongs to FreeRTOS.
 * PSC assumes HCLK = 8 MHz (HSI). If the clock changes to 72 MHz,
 * PSC must become 71 AND configCPU_CLOCK_HZ updated.
 * --------------------------------------------------------- */
extern "C" HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    __HAL_RCC_TIM4_CLK_ENABLE();
    TIM4->PSC = 7;
    TIM4->ARR = 999;
    TIM4->CNT = 0;
    TIM4->DIER |= TIM_DIER_UIE;
    TIM4->CR1 |= TIM_CR1_CEN;

    HAL_NVIC_SetPriority(TIM4_IRQn, TickPriority, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    return HAL_OK;
}

extern "C" void TIM4_IRQHandler(void)
{
    if (TIM4->SR & TIM_SR_UIF) {
        TIM4->SR &= ~TIM_SR_UIF;
        HAL_IncTick();
#if DIAGNOSTIC_MODE
        static uint32_t ms = 0;
        if (++ms >= 2000) {
            ms = 0;
            PrintStatus();
        }
#endif
    }
}

/* ---------------------------------------------------------
 * HardFault reporter
 * --------------------------------------------------------- */
extern "C" void HardFault_C(uint32_t *frame)
{
    __disable_irq();
    UartPrint("\r\n*** HARDFAULT pc=");
    UartPrintHex(frame[6]);
    UartPrint(" lr=");
    UartPrintHex(frame[5]);
    UartPrint(" cfsr=");
    UartPrintHex(SCB->CFSR);
    UartPrint("\r\n");
    for (;;) {}
}

extern "C" __attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile
    (
        "tst lr, #4      \n"
        "ite eq          \n"
        "mrseq r0, msp   \n"
        "mrsne r0, psp   \n"
        "b HardFault_C   \n"
    );
}

/* ---------------------------------------------------------
 * FreeRTOS hooks
 * --------------------------------------------------------- */
extern "C" void vApplicationIdleHook(void)
{
    static TickType_t lastToggle = 0;
    TickType_t now = xTaskGetTickCount();
    if ((now - lastToggle) >= 500) {
        lastToggle = now;
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    }
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    __disable_irq();
    UartPrint("STACK OVERFLOW in task: ");
    UartPrint(pcTaskName);
    UartPrint("\r\n");
    for (;;) {}
}

extern "C" void vAssertCalled(const char *file, int line)
{
    __disable_irq();
    UartPrint("ASSERT failed: ");
    UartPrint(file);
    UartPrint(" line ");
    UartPrintNum((unsigned long)line);
    UartPrint("\r\n");
    for (;;) {}
}

/* ---------------------------------------------------------
 * Part III tasks: priority 1, 1000 ms, vTaskDelayUntil().
 * --------------------------------------------------------- */
void TaskA(void *pvParameters)
{
    (void)pvParameters;
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
#if DIAGNOSTIC_MODE
        UartPrint("Task A running, tick=");
        UartPrintNum((unsigned long)xTaskGetTickCount());
        UartPrint(" first=");   UartPrintNum(g_portFirstLaunchCount);
        UartPrint(" yield=");   UartPrintNum(g_portYieldCount);
        UartPrint(" tickSw=");  UartPrintNum(g_portTickSwitchCount);
        UartPrint("\r\n");
#else
        UartPrint("Task A running\r\n");
#endif
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
    }
}

void TaskB(void *pvParameters)
{
    (void)pvParameters;

    vTaskDelay(pdMS_TO_TICKS(500));
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
#if DIAGNOSTIC_MODE
        UartPrint("Task B running, tick=");
        UartPrintNum((unsigned long)xTaskGetTickCount());
        UartPrint("\r\n");
#else
        UartPrint("Task B running\r\n");
#endif
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
    }
}

/* ---------------------------------------------------------
 * SensorTask (S20-S22 + S25 publish): DHT22 + LDR every 2500 ms
 * via vTaskDelayUntil(); validated samples published to the
 * latest-value queue via xQueueOverwrite().
 * --------------------------------------------------------- */
void SensorTask(void *pvParameters)
{
    (void)pvParameters;
    dht22_init();

    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        DHT22_Result_t d;
        taskENTER_CRITICAL();
        d = dht22_read();
        taskEXIT_CRITICAL();

        /* LDR on PA0/ADC1: one software-triggered conversion */
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        uint32_t raw = HAL_ADC_GetValue(&hadc1);      /* 0..4095 */
        HAL_ADC_Stop(&hadc1);

        /* Documented representation (S21): raw 12-bit ADC count
         * scaled linearly to 0-100 % of full scale. NOT calibrated lux. */
        int light_pct = (int)((raw * 100UL + 2047UL) / 4095UL);

        if (d.valid) {
            /* S25: publish ONLY validated complete samples. */
            SensorData sd;
            sd.temperature    = d.temperature;
            sd.humidity       = d.humidity;
            sd.lightLevel     = light_pct;
            sd.motionDetected = false;
            xQueuePeek(xMotionQueue, &sd.motionDetected, 0);

            xQueueOverwrite(xSensorQueue, &sd);

            int t_int = (int)d.temperature;
            int t_frac = (int)((d.temperature - (float)t_int) * 100.0f);
            int h_int = (int)d.humidity;
            int h_frac = (int)((d.humidity - (float)h_int) * 100.0f);
            char buf[96];
            snprintf(buf, sizeof(buf),
                     "Sample: Temperature: %d.%02d C, Humidity: %d.%02d %%, Light: %d %%, Motion: %s\r\n",
                     t_int, t_frac, h_int, h_frac, light_pct,
                     sd.motionDetected ? "yes" : "no");
            UartPrint(buf);
        } else {
            UartPrint("DHT22 read failed\r\n");
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2500));
    }
}

/* ---------------------------------------------------------
 * DisplayTask (S26-S27): SOLE owner of the OLED. Consumes the
 * sensor queue (peek = latest snapshot) and renders the current
 * page. No other task touches I2C1 or the framebuffer.
 * --------------------------------------------------------- */
void DisplayTask(void *pvParameters)
{
    (void)pvParameters;
    if (oled_init()) {
        UartPrint("[DISPLAY] ready\r\n");
    } else {
        UartPrint("[DISPLAY] I2C ERROR\r\n");
    }

    TickType_t lastWake = xTaskGetTickCount();
    char line[32]; 
    DisplayMode mode = DISPLAY_TEMPERATURE;

    for (;;) {
        NavCommand command;
        while (xQueueReceive(xNavQueue, &command, 0) == pdTRUE) {
            if (command == NAV_NEXT) mode = nextDisplayMode(mode);
            if (command == NAV_PREV) mode = previousDisplayMode(mode);
            UartPrint("[INPUT] mode: ");
            UartPrint(displayModeName(mode));
            UartPrint("\r\n");
        }

        SensorData sd;
        if (xQueuePeek(xSensorQueue, &sd, 0) == pdTRUE) {
            oled_clear();

            // Draw the top header
            oled_show_text(10, 0, "ROOM MONITOR");

            EventBits_t events = xEventGroupGetBits(xSystemEventGroup);
            oled_show_text(10, 1,
                           (events & EVENT_ACTIVE_BIT0) != 0 ? "STATE: ACTIVE" : "STATE: INACTIVE");
            oled_show_text(10, 2,
                           (events & EVENT_ALARM_BIT2) != 0 ? "ALARM: ACTIVE" : "ALARM: NORMAL");

            // Prepare the string for the specific sensor
            if (mode == DISPLAY_TEMPERATURE) {
                int t_tenths = (int)(sd.temperature * 10.0f + 0.5f);
                int t_int = t_tenths / 10;
                int t_frac = t_tenths % 10;
                snprintf(line, sizeof(line), "Temp: %d.%d C", t_int, t_frac);
            } else if (mode == DISPLAY_HUMIDITY) {
                int h_int = (int)sd.humidity;
                int h_frac = (int)((sd.humidity - (float)h_int) * 10.0f);
                snprintf(line, sizeof(line), "Humid: %d.%u %%", h_int, (unsigned)h_frac);
            } else if (mode == DISPLAY_LIGHT) {
                snprintf(line, sizeof(line), "Light: %d %%", sd.lightLevel);
            } else {
                snprintf(line, sizeof(line), "Motion: %s", sd.motionDetected ? "YES" : "NO");
            }
            
            // Draw the formatted text cleanly underneath
            oled_show_text(10, 3, line);

        } else {
            oled_clear();
            oled_show_text(10, 3, "NO DATA");
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(500));
    }
}

/* ---------------------------------------------------------
 * Application entry point (Section 10)
 * --------------------------------------------------------- */
void app_main(void)
{
    UartPrint("BCA182 FreeRTOS Multisensor\r\n");
    UartPrint("System starting...\r\n");

    /* S41 flow: object creation BEFORE task creation */
    if (createRtosObjects() != pdPASS) {
        UartPrint("ERROR: RTOS object creation failed\r\n");
        while (1) {}
    }

    BaseType_t s = xTaskCreate(SensorTask, "Sensor", 512, NULL, 2, NULL);
    BaseType_t d = xTaskCreate(DisplayTask, "Display", 512, NULL, 1, NULL);
    BaseType_t i = xTaskCreate(InputTask, "Input", 256, NULL, 3, NULL);
    BaseType_t a = xTaskCreate(AlarmTask, "Alarm", 256, NULL, 2, NULL);
    BaseType_t m = xTaskCreate(MotionTask, "Motion", 256, NULL, 3, NULL);
    BaseType_t st = xTaskCreate(StateTask, "State", 256, NULL, 2, NULL);

    if (i != pdPASS) {
        UartPrint("ERROR: InputTask creation failed\r\n");
    }
    if (s != pdPASS) {
        UartPrint("ERROR: SensorTask creation failed\r\n");
    }
    if (d != pdPASS) {
        UartPrint("ERROR: DisplayTask creation failed\r\n");
    }
    if (a != pdPASS) {
        UartPrint("ERROR: AlarmTask creation failed\r\n");
    }
    if (m != pdPASS) {
        UartPrint("ERROR: MotionTask creation failed\r\n");
    }
    if (st != pdPASS) {
        UartPrint("ERROR: StateTask creation failed\r\n");
    }

#if DIAGNOSTIC_MODE
    if (s != pdPASS) UartPrint("ERROR: SensorTask creation failed\r\n");
    if (d != pdPASS) UartPrint("ERROR: DisplayTask creation failed\r\n");
    if (i != pdPASS) UartPrint("ERROR: InputTask creation failed\r\n");
    UartPrint("Starting scheduler...\r\n");
#else
    (void)s; (void)d; (void)i; (void)a; (void)m; (void)st;
#endif

    vTaskStartScheduler();

    UartPrint("ERROR: scheduler returned\r\n");
    while (1) {}
}

/* ---------------------------------------------------------
 * main: hardware initialization only (Section 41)
 * --------------------------------------------------------- */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    SCB->VTOR = (uint32_t)g_pfnVectors;
    __DSB();
    __ISB();

    MX_USART1_UART_Init();
    MX_ADC1_Init();
    MX_ENCODER_Init();
    MX_BUZZER_Init();
    MX_PIR_Init();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef led = {0};
    led.Pin   = GPIO_PIN_13;
    led.Mode  = GPIO_MODE_OUTPUT_PP;
    led.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &led);

    app_main();

    return 0;
}

/* ---------------------------------------------------------
 * Clock (8 MHz HSI) and peripherals
 * --------------------------------------------------------- */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

static void MX_ADC1_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &gpio);

    hadc1.Instance               = ADC1;
    hadc1.Init.ScanConvMode      = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv  = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign         = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion   = 1;
    HAL_ADC_Init(&hadc1);

    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel      = ADC_CHANNEL_0;      /* PA0 */
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADCEx_Calibration_Start(&hadc1);       /* F1: calibrate before use */
}

static void MX_BUZZER_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_8;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 7;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 999;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&htim1);

    TIM_OC_InitTypeDef channel = {0};
    channel.OCMode = TIM_OCMODE_PWM1;
    channel.Pulse = 0;
    channel.OCPolarity = TIM_OCPOLARITY_HIGH;
    channel.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    channel.OCFastMode = TIM_OCFAST_DISABLE;
    channel.OCIdleState = TIM_OCIDLESTATE_RESET;
    channel.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&htim1, &channel, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

static void MX_PIR_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &gpio);
}

extern "C" void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (huart->Instance == USART1) {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

extern "C" void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        __HAL_RCC_AFIO_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_ENABLE();

        GPIO_InitTypeDef gpio = {0};
        gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        gpio.Mode = GPIO_MODE_AF_OD;
        gpio.Pull = GPIO_PULLUP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &gpio);
    }
}