#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#define SW_0 9

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define STR_LEN (80)
#define LORA_ID_LEN (16)
#define UART_TIMEOUT_US 500000

typedef enum LoRaState {
    IDLE,
    CONNECTING,
    READING_FIRMWARE,
    READING_DEVEUI,
    ERROR
} LoRaState;

bool test_LoRa() {
    char *send_AT = "AT\r\n";
    if (uart_is_writable(UART_ID)) {
        uart_write_blocking(UART_ID, (uint8_t *)send_AT, strlen(send_AT));
        char response[STR_LEN];
        size_t response_length = 0;
        while (uart_is_readable(UART_ID)) {
            response[response_length++] = uart_getc(UART_ID);
            if (response_length >= STR_LEN - 1) {
                break;
            }
        }
        response[response_length] = '\0';
        printf("Response: %s\n", response);

        return true;
    }
    else {
        return false;
    }
}


int main(void)
{
    stdio_init_all();
    printf("Booting...\n");

    gpio_init(SW_0);
    gpio_set_dir(SW_0, GPIO_IN);
    gpio_pull_up(SW_0);

    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    LoRaState state = IDLE;

    while(true) {
        if (!gpio_get(SW_0)) {
            if (state == IDLE) {
                printf("State Connecting..\n");
                sleep_ms(200);
                if (test_LoRa()) {
                    state = CONNECTING;
                }
                else {
                    printf("Error!");
                    return 1;
                }
            }
            else if (state == CONNECTING) {
                printf("State READING_FIRMWARE..\n");
                sleep_ms(200);
                state = READING_FIRMWARE;
            }
            else if (state == READING_FIRMWARE) {
                printf("State READING_DEVEUI..\n");
                sleep_ms(200);
                state = READING_DEVEUI;
            }
            else {
                printf("Errors\n");
                state = ERROR;
                return 1;
            }
        }
    }

    return 0;
}