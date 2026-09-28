/**
 * dht22.c - Minimal single-wire DHT22 driver for STM32F103 (Wokwi-safe)
 *
 * Uses the DWT cycle counter for microsecond timing. Clock-agnostic:
 * derives cycles-per-us from SystemCoreClock.
 */
#include "stm32f1xx_hal.h"
#include "dht22.h"
#include <stdint.h>

/* ---------------- DWT microsecond timing ---------------- */
static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t cyc(void) { return DWT->CYCCNT; }

static void delay_us(uint32_t us)
{
    uint32_t start = cyc();
    uint32_t ticks = us * (SystemCoreClock / 1000000UL);
    while ((cyc() - start) < ticks) { /* busy-wait, init context only */ }
}

/* Measure how long the pin stays in `state`; returns cycles.
 * Timeout guard prevents a hang on a missing sensor.             */
static uint32_t pulse_width(GPIO_PinState state)
{
    uint32_t start = cyc();
    while (HAL_GPIO_ReadPin(DHT22_GPIO_PORT, DHT22_GPIO_PIN) == state) {
        if ((cyc() - start) > (SystemCoreClock / 1000UL)) { /* >1 ms: lost */
            return 0xFFFFFFFFUL;
        }
    }
    return cyc() - start;
}

/* ---------------- GPIO mode helpers ---------------- */
static void pin_output(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = DHT22_GPIO_PIN; g.Mode = GPIO_MODE_OUTPUT_PP; g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &g);
}

static void pin_input(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = DHT22_GPIO_PIN; g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &g);
}

void dht22_init(void)
{
    DHT22_RCC_EN();
    dwt_init();
    pin_input();            /* idle: released, pulled high */
}

/* ---------------- one full read (blocking, ~4 ms) ---------------- */
DHT22_Result_t dht22_read(void)
{
    DHT22_Result_t r = { 0.0f, 0.0f, false };
    uint8_t data[5] = {0, 0, 0, 0, 0};
    uint32_t w;

    /* --- 1. Host start pulse: >= 1 ms low, then release --- */
    pin_output();
    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_GPIO_PIN, GPIO_PIN_RESET);
    delay_us(1200);
    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_GPIO_PIN, GPIO_PIN_SET);
    pin_input();
    delay_us(30);           /* let the line settle high */

    /* --- 2. Sensor response: 80 us low, then 80 us high --- */
    w = pulse_width(GPIO_PIN_RESET);              /* wait out response-low  */
    if (w == 0xFFFFFFFFUL || w == 0) return r;    /* no response            */
    w = pulse_width(GPIO_PIN_SET);
    if (w == 0xFFFFFFFFUL) return r;

    /* --- 3. 40 bits: 50 us low start, then high pulse --- */
    for (int i = 0; i < 40; i++) {
        w = pulse_width(GPIO_PIN_RESET);          /* bit start low          */
        if (w == 0xFFFFFFFFUL) return r;
        w = pulse_width(GPIO_PIN_SET);            /* bit value: short=0 long=1 */
        if (w == 0xFFFFFFFFUL) return r;

        uint32_t us = w / (SystemCoreClock / 1000000UL);
        data[i / 8] <<= 1;
        if (us > 40) {                            /* 40 us threshold: robust */
            data[i / 8] |= 1;
        }
    }

    /* --- 4. Checksum: low byte of byte0..3 sum --- */
    uint8_t sum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (sum != data[4]) { r.valid = false; return r; }

    /* --- 5. Decode --- */
    uint16_t humi_raw = ((uint16_t)data[0] << 8) | data[1];
    uint16_t temp_raw = ((uint16_t)data[2] << 8) | data[3];
    bool     negative = (temp_raw & 0x8000) != 0;

    r.humidity    = humi_raw / 10.0f;
    r.temperature = (temp_raw & 0x7FFF) / 10.0f;
    if (negative) r.temperature = -r.temperature;
    r.valid = true;
    return r;
}