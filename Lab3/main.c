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
    DO_NOTHING,
    TEST_CONNECTION,
    FIRMWARE,
    ERROR
} lora_state;

void initialize_uart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

bool send_command(const char *command) {
    uart_puts(UART_ID, command);

    uint32_t start_time = time_us_32();
    char response[STR_LEN];
    size_t response_length = 0;

    while ((time_us_32() - start_time) < UART_TIMEOUT_US) {
        if (uart_is_readable(UART_ID)) {
            response[response_length++] = uart_getc(UART_ID);
            if (response_length >= STR_LEN - 1) {
                break;
            }
        }
    }
    response[response_length] = '\0';
    printf("Response: %s", response);

    return (strstr(response, "OK") != NULL);
}

void test_LoRa(lora_state *current_state) {
    if (send_command("AT\r\n")) {
        *current_state = FIRMWARE;
    }
    else {
        *current_state = ERROR;
    }
}

void firmware_init(lora_state *current_state) {
    if (send_command("AT+VER\r\n")) {
        *current_state = FIRMWARE;
    } else {
        *current_state = ERROR;
    }
}

int main() {
    stdio_init_all();
    printf("Waiting for user to press SW_0...\n");

    gpio_init(SW_0);
    gpio_set_dir(SW_0, GPIO_IN);
    gpio_pull_up(SW_0);

    initialize_uart();
    lora_state current_state = DO_NOTHING;

    while (true) {
        switch (current_state) {
            case DO_NOTHING:
                if (!gpio_get(SW_0)) {
                    sleep_ms(100);
                    current_state = TEST_CONNECTION;
                }
                break;

            case TEST_CONNECTION:
                test_LoRa(&current_state);
                if (current_state == FIRMWARE) {
                    printf("Connected to LoRa module!\n");
                }
                break;

            case FIRMWARE:
                firmware_init(&current_state);
                break;

            case ERROR:
                printf("Module not responding\n");
                current_state = DO_NOTHING;
                break;
        }
        sleep_ms(100);
    }
    return 0;
}
