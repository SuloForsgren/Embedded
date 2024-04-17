#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#define SW_0 9

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 4
#define UART_RX_PIN 5

#define STR_LEN 80
#define UART_TIMEOUT_US 500000

typedef enum lora_state {
    TESTING,
    ERROR
}lora_state;

void initialize_uart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

bool test_LoRa(lora_state *current_state) {
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

        if (strstr(response, "OK") != NULL) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        *current_state = ERROR;
        return false;
    }
}

int main() {
    stdio_init_all();
    printf("Waiting for user to press SW_0...\n");

    gpio_init(SW_0);
    gpio_set_dir(SW_0, GPIO_IN);
    gpio_pull_up(SW_0);

    initialize_uart();
    lora_state current_state = TESTING;

    while (true) {
        if (!gpio_get(SW_0)) {
            if (test_LoRa(&current_state) && current_state == TESTING) {
                printf("LoRa communication successful!\n");
            }
            else if (current_state == ERROR){
                printf("Error communicating with LoRa module.\n");
            }
        }
        sleep_ms(100);
    }
    return 0;
}
