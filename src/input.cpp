#include "input.h"
#include "stm32f1xx_hal.h"
#include "rtos_objects.h"
#include <cstring>

extern UART_HandleTypeDef huart1;

void MX_ENCODER_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);
}

void InputTask(void *pvParameters)
{
    (void)pvParameters;
    uint8_t previousState = 0;
    int8_t transitionSum = 0;

    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET) previousState |= 2;
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) previousState |= 1;
    
    const char* readyMsg = "[INPUT] ready\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)readyMsg, strlen(readyMsg), HAL_MAX_DELAY);

    for (;;) {
        uint8_t currentState = 0;
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET) currentState |= 2;
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) currentState |= 1;

        uint8_t transition = (uint8_t)((previousState << 2) | currentState);
        int8_t delta = 0;
        switch (transition) {
        case 0x01: case 0x07: case 0x0E: case 0x08: delta = 1; break;
        case 0x02: case 0x0B: case 0x0D: case 0x04: delta = -1; break;
        default: break;
        }

        transitionSum += delta;
        previousState = currentState;

        if (transitionSum >= 4 || transitionSum <= -4) {
            NavCommand command = transitionSum >= 4 ? NAV_NEXT : NAV_PREV;
            xQueueSend(xNavQueue, &command, 0);
            transitionSum = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}