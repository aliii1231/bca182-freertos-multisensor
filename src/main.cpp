#include "stm32f1xx_hal.h"
#include <string.h>

UART_HandleTypeDef huart1;

// Initialize UART1 for Serial Output on PA9 (TX) and PA10 (RX)
static void MX_USART1_UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

// Helper function to send strings over UART
void print_msg(const char* msg) {
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

// Application entry point required by Section 10
void app_main() 
    {
    HAL_Delay(1000); // Wait 1 second for terminal to open
    
    print_msg("BCA182 FreeRTOS Multisensor\r\n");
    print_msg("System starting...\r\n");
    
    while (1) {
        print_msg("System running...\r\n");
        HAL_Delay(2000); // Print every 2 seconds
    }
}
int main(void) {
    HAL_Init(); 
    MX_USART1_UART_Init();
    app_main();
    return 0;
}