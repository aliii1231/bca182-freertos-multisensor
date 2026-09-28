/**
 * BCA182 - Laboratory Activity No. 1
 * PART III (done) + PART IV step 1: DHT22 sensor acquisition (Section 20)
 *
 * Hardware init -> app_main() -> task creation -> scheduler (S10/S41).
 * HAL timebase on TIM4; SysTick reserved for FreeRTOS.
 * UART not yet mutex-protected: TaskA/TaskB/SensorTask prints are
 * phase-offset; the Part XI serial mutex is the proper fix.
 */
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "dht22.h"

/* 1 = scheduler counters + status line (debugging)
 * 0 = clean lab output                                          */
#define DIAGNOSTIC_MODE  0

UART_HandleTypeDef huart1;
extern "C" uint32_t g_pfnVectors[];

/* Instrumented port counters (src/port.c) — diagnostic only */
extern "C" volatile uint32_t g_portFirstLaunchCount;
extern "C" volatile uint32_t g_portYieldCount;
extern "C" volatile uint32_t g_portTickSwitchCount;
extern "C" volatile uint32_t ulPortSwitchPending;
extern "C" uint32_t ulPortGetCriticalNesting(void);

void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
void app_main(void);

/* ---------------------------------------------------------
 * UART output (USART1: PA9 = TX, PA10 = RX -> Wokwi terminal).
 * --------------------------------------------------------- */
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

/* ---------------------------------------------------------
 * HAL timebase on TIM4 — SysTick belongs to FreeRTOS.
 * PSC assumes HCLK = 8 MHz (HSI). If the clock is ever changed
 * to 72 MHz, PSC must become 71 AND configCPU_CLOCK_HZ updated.
 * --------------------------------------------------------- */
extern "C" HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    __HAL_RCC_TIM4_CLK_ENABLE();
    TIM4->PSC = 7;              /* 8 MHz / 8 = 1 MHz */
    TIM4->ARR = 999;            /* 1 kHz -> 1 ms     */
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
 * TaskB 500 ms phase offset -> deterministic A/B alternation.
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
 * SensorTask (Sections 20-22): DHT22 every 2000 ms via
 * vTaskDelayUntil() — the lab's required periodic pattern.
 * Prints fixed-point without -u _printf_float.
 * --------------------------------------------------------- */
void SensorTask(void *pvParameters)
{
    (void)pvParameters;
    dht22_init();

    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        DHT22_Result_t d = dht22_read();

        if (d.valid) {
            /* 25.4 C -> "25.40"; 61.2 % -> "61.20" */
            int  t_int  = (int)d.temperature;
            int  t_frac = (int)((d.temperature - (float)t_int) * 100.0f);
            int  h_int  = (int)d.humidity;
            int  h_frac = (int)((d.humidity - (float)h_int) * 100.0f);

            char buf[48];
            snprintf(buf, sizeof(buf),
                     "Temperature: %d.%02d C\r\nHumidity: %d.%02d %%\r\n",
                     t_int, t_frac, h_int, h_frac);
            UartPrint(buf);
        } else {
            UartPrint("DHT22 read failed\r\n");
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000));
    }
}

/* ---------------------------------------------------------
 * Application entry point (Section 10)
 * --------------------------------------------------------- */
void app_main(void)
{
    UartPrint("BCA182 FreeRTOS Multisensor\r\n");
    UartPrint("System starting...\r\n");

    BaseType_t a = xTaskCreate(TaskA, "TaskA", 256, NULL, 1, NULL);
    BaseType_t b = xTaskCreate(TaskB, "TaskB", 256, NULL, 1, NULL);
    BaseType_t s = xTaskCreate(SensorTask, "Sensor", 512, NULL, 2, NULL);

#if DIAGNOSTIC_MODE
    if (a != pdPASS) UartPrint("ERROR: TaskA creation failed\r\n");
    if (b != pdPASS) UartPrint("ERROR: TaskB creation failed\r\n");
    if (s != pdPASS) UartPrint("ERROR: SensorTask creation failed\r\n");
    UartPrint("Starting scheduler...\r\n");
#else
    (void)a; (void)b; (void)s;
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
 * Clock (8 MHz HSI) and USART1
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