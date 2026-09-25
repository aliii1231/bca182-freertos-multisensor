#include "stm32f1xx_hal.h"

void app_main() {
    while (1) {
        // FreeRTOS scheduler will run here later
    }
}

int main(void) {
    HAL_Init(); 
    app_main();
    return 0;
}