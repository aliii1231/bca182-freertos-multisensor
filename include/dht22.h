#ifndef DHT22_H
#define DHT22_H

#include <stdbool.h>

/* Data pin: PA1 (see diagram.json) */
#define DHT22_GPIO_PORT  GPIOA
#define DHT22_GPIO_PIN   GPIO_PIN_1
#define DHT22_RCC_EN()   __HAL_RCC_GPIOA_CLK_ENABLE()

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temperature;   /* deg C, negative-capable */
    float humidity;      /* %RH                      */
    bool  valid;         /* checksum + framing OK    */
} DHT22_Result_t;

void           dht22_init(void);
DHT22_Result_t dht22_read(void);

#ifdef __cplusplus
}
#endif

#endif /* DHT22_H */