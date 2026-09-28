/**
 * BCA182 - Laboratory Activity No. 1
 * PART V: Data Communication (S24-S25)
 *   - SensorData struct + latest-value queue (rtos_objects)
 *   - SensorTask publishes validated samples via xQueueOverwrite
 *   - LogTask consumes via xQueuePeek (temporary; DisplayTask later)
 * PART III/IV behavior preserved (tasks, DHT22, LDR).
 */
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "dht22.h"
#include "rtos_objects.h"

#define DIAGNOSTIC_MODE  0

UART_HandleTypeDef huart1;
ADC_HandleTypeDef  hadc1;

extern "C" uint32_t g_pfnVectors[];

extern "C" volatile uint32_t g_portFirstLaunchCount;
extern "C" volatile uint32_t g_portYieldCount;
extern "C" volatile uint32_t g_portTickSwitchCount;
extern "C" volatile uint32_t ulPortSwitchPending;
extern "C" uint32_t ulPortGetCriticalNesting(void);

void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
void app_main(void);

/* --------------------------------------------------------- */
static void UartPrint(const char *s)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
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

/* HAL timebase on TIM4 (SysTick = FreeRTOS). PSC assumes 8 MHz HSI. */
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
        if (++ms >= 2000) { ms = 0; PrintStatus(); }
#endif
    }
}

/* HardFault reporter */
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

/* FreeRTOS hooks */
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

/* ---------------- Part III tasks (kept until Part VI cleanup) -------- */
void TaskA(void *pvParameters)
{
    (void)pvParameters;
    TickType_t lastWakeTime = xTaskGetTickCount();
    for (;;) {
        UartPrint("Task A running\r\n");
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
    }
}

void TaskB(void *pvParameters)
{
    (void)pvParameters;
    vTaskDelay(pdMS_TO_TICKS(500));
    TickType_t lastWakeTime = xTaskGetTickCount();
    for (;;) {
        UartPrint("Task B running\r\n");
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
    }
}

/* ---------------- SensorTask (S20-S22 + S25 publish) ------------------ */
void SensorTask(void *pvParameters)
{
    (void)pvParameters;
    dht22_init();

    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        DHT22_Result_t d = dht22_read();

        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        uint32_t raw = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);

        int light_pct = (int)((raw * 100UL + 2047UL) / 4095UL);

        if (d.valid) {
            /* S25: publish ONLY validated complete samples. */
            SensorData sd;
            sd.temperature    = d.temperature;
            sd.humidity       = d.humidity;
            sd.lightLevel     = light_pct;
            sd.motionDetected = false;          /* Part IX: MotionTask */

            xQueueOverwrite(xSensorQueue, &sd);

            int t_int  = (int)d.temperature;
            int t_frac = (int)((d.temperature - (float)t_int) * 100.0f);
            int h_int  = (int)d.humidity;
            int h_frac = (int)((d.humidity - (float)h_int) * 100.0f);

            char buf[64];
            snprintf(buf, sizeof(buf),
                     "Temperature: %d.%02d C\r\nHumidity: %d.%02d %%\r\nLight: %d %%\r\n",
                     t_int, t_frac, h_int, h_frac, light_pct);
            UartPrint(buf);
        } else {
            UartPrint("DHT22 read failed\r\n");
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000));
    }
}

/* ---------------- LogTask (S25 demo consumer; temporary) --------------
 * Will be REPLACED by DisplayTask (Part VI) and AlarmTask (Part VIII).
 * Peeks the latest-value mailbox on its own 2 s schedule. Uses a
 * fixed-point print (no float formatting dependency).                  */
void LogTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        SensorData sd;
        if (xQueuePeek(xSensorQueue, &sd, 0) == pdTRUE) {
            int t_int  = (int)sd.temperature;
            int t_frac = (int)((sd.temperature - (float)t_int) * 100.0f);
            int h_int  = (int)sd.humidity;
            int h_frac = (int)((sd.humidity - (float)h_int) * 100.0f);

            char buf[64];
            snprintf(buf, sizeof(buf),
                     "[QUEUE] T=%d.%02d H=%d.%02d L=%d%% M=%d\r\n",
                     t_int, t_frac, h_int, h_frac, sd.lightLevel,
                     sd.motionDetected ? 1 : 0);
            UartPrint(buf);
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000));
    }
}

/* ---------------- Application entry point (S10) ----------------------- */
void app_main(void)
{
    UartPrint("BCA182 FreeRTOS Multisensor\r\n");
    UartPrint("System starting...\r\n");

    /* S41 flow: object creation BEFORE task creation */
    if (createRtosObjects() != pdPASS) {
        UartPrint("ERROR: RTOS object creation failed\r\n");
        while (1) {}
    }

    BaseType_t a = xTaskCreate(TaskA, "TaskA", 256, NULL, 1, NULL);
    BaseType_t b = xTaskCreate(TaskB, "TaskB", 256, NULL, 1, NULL);
    BaseType_t s = xTaskCreate(SensorTask, "Sensor", 512, NULL, 2, NULL);
    BaseType_t g = xTaskCreate(LogTask, "Log", 256, NULL, 1, NULL);

#if DIAGNOSTIC_MODE
    if (a != pdPASS) UartPrint("ERROR: TaskA creation failed\r\n");
    if (b != pdPASS) UartPrint("ERROR: TaskB creation failed\r\n");
    if (s != pdPASS) UartPrint("ERROR: SensorTask creation failed\r\n");
    if (g != pdPASS) UartPrint("ERROR: LogTask creation failed\r\n");
    UartPrint("Starting scheduler...\r\n");
#else
    (void)a; (void)b; (void)s; (void)g;
#endif

    vTaskStartScheduler();

    UartPrint("ERROR: scheduler returned\r\n");
    while (1) {}
}

/* ---------------- main: hardware init only (S41) ---------------------- */
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

    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef led = {0};
    led.Pin   = GPIO_PIN_13;
    led.Mode  = GPIO_MODE_OUTPUT_PP;
    led.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &led);

    app_main();
    return 0;
}

/* ---------------- Clock + peripherals (unchanged) --------------------- */
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
    sConfig.Channel      = ADC_CHANNEL_0;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADCEx_Calibration_Start(&hadc1);
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