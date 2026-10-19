/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;

const int PIN_ADC = 28;      // GP28 (pelo circuito)
const int ADC_CHANNEL = 2;   // GP28 = canal 2

const float conversion_factor = 3.3f / (1 << 12);

volatile int g_timer_flag = 0;

bool timer_callback(repeating_timer_t *rt) {
    g_timer_flag = 1;
    return true; // continua repetindo
}

int get_zona(float voltage) {
    if (voltage < 1.0f) {
        return 0;
    } else if (voltage < 2.0f) {
        return 1;
    }
    return 2;
}

/**
 * 0..1.0V: Desligado
 * 1..2.0V: 300 ms
 * 2..3.3V: 500 ms
 */
int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);

    adc_init();
    adc_gpio_init(PIN_ADC);
    adc_select_input(ADC_CHANNEL);

    repeating_timer_t timer;
    int timer_ativo = 0;
    int zona_atual = 0;
    int led = 0;

    while (1) {
        float voltage = adc_read() * conversion_factor;
        int zona = get_zona(voltage);

        if (zona != zona_atual) {
            if (timer_ativo) {
                cancel_repeating_timer(&timer);
                timer_ativo = 0;
            }

            led = 0;
            gpio_put(PIN_LED_B, 0);
            g_timer_flag = 0;

            if (zona == 1) {
                add_repeating_timer_ms(300, timer_callback, NULL, &timer);
                timer_ativo = 1;
            } else if (zona == 2) {
                add_repeating_timer_ms(500, timer_callback, NULL, &timer);
                timer_ativo = 1;
            }

            zona_atual = zona;
        }

        if (g_timer_flag) {
            g_timer_flag = 0;
            led = !led;
            gpio_put(PIN_LED_B, led);
        }
    }
}