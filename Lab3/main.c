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
#define UART_TIMEOUT_MS 500
#define MAX_RETRIES 5
#define FIRMWARE_LEN 30


typedef enum lora_state {
    DO_NOTHING,
    TEST_CONNECTION,
    FIRMWARE,
    ERROR
}
lora_state;

void initialize_uart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void test_LoRa(lora_state *current_state) {
    char *send_AT = "AT\r\n";
    int retries = 0;

    while (retries < MAX_RETRIES) {
        printf("Retry number %d\n", retries);
        if (uart_is_writable(UART_ID)) {
            uart_write_blocking(UART_ID, (uint8_t *) send_AT, strlen(send_AT));

            sleep_ms(UART_TIMEOUT_MS);

            char response[STR_LEN];
            size_t response_length = 0;

            while (uart_is_readable(UART_ID)) {
                response[response_length++] = uart_getc(UART_ID);
                if (response_length >= STR_LEN - 1) {
                    break;
                }
            }
            response[response_length] = '\0';
            printf("Response: %s", response);

            if (strstr(response, "OK") != NULL) {
                *current_state = FIRMWARE;
                break;
            }
            else {
                *current_state = ERROR;
            }
        }
        else {
            *current_state = ERROR;
        }
        retries += 1;
    }
}

bool get_firmware(lora_state *current_state, char firmware[FIRMWARE_LEN]) {
    char *request = "AT+VERSION\r\n";

    if (uart_is_writable(UART_ID)) {
        uart_write_blocking(UART_ID, (uint8_t *)request, strlen(request));

        sleep_ms(UART_TIMEOUT_MS);

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

        // Check if response contains version information
        if (strstr(response, "OK") != NULL) {
            // Extract version information from the response
            char *version_start = strstr(response, "VERSION:");
            if (version_start != NULL) {
                version_start += strlen("VERSION:");
                strcpy(firmware, version_start);
                return true;
            }
        }
    }
    *current_state = ERROR;
    return false;
}

void firmware_init(lora_state *current_state) {
    char firmware[FIRMWARE_LEN];
    if (get_firmware(current_state, firmware)) {
        printf("Firmware version: %s", firmware);
    }
    else {
        printf("Module stopped responding!");
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
                printf("Connected to LoRa module!\n");
                break;

            case FIRMWARE:
                firmware_init(&current_state);
                current_state = ERROR;
                break;

            case ERROR:
                printf("module not responding\n");
                current_state = DO_NOTHING;
                break;
        }
        sleep_ms(100);
    }
}