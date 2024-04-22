#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h> // Include for isxdigit and tolower
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
#define LORA_ID_LEN 16 // Assuming LoRa device ID length

typedef enum lora_state {
    DO_NOTHING,
    DEV,
    TEST_CONNECTION,
    FIRMWARE,
    ERROR
} lora_state;

void initialize_uart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

bool send_command(const char *command, bool print_response) {
    uart_puts(UART_ID, command);
    bool found = false;

    uint32_t start_time = time_us_32();
    char response[STR_LEN];
    size_t response_length = 0;

    for (int i = 0; i < 5; i++) {
        while ((time_us_32() - start_time) < UART_TIMEOUT_US) {
            if (uart_is_readable(UART_ID)) {
                response[response_length++] = uart_getc(UART_ID);
                if (response_length >= STR_LEN - 1) {
                    found = true;
                    break;
                }
                if (strstr(response, "OK") != NULL) {
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            return true;
        }
    }

    response[response_length] = '\0';
    if (!print_response) {
        printf("%s\n", response);
        return (strstr(response, "OK") != NULL);
    }
    else if (print_response) {
        
    }
    return (strstr(response, "VER") != NULL);
}

void lora_fmt_deveui(char *input, char *output) {
    char *start = strstr(input, ", ");
    if (start != NULL) {
        start += 2;
        uint8_t j = 0;
        for (uint8_t i = 0; start[i] != '\0'; i++) {
            if (isxdigit(start[i])) {
                output[j++] = tolower(start[i]);
            }
        }
        output[j] = '\0';
    }
}

bool lora_get_deveui(uart_inst_t *uart, char *lora_deveui) {
    const char *send = "AT+ID=DevEui\r\n";
    uart_write_blocking(uart, (const uint8_t *)send, strlen(send));

    uint32_t start_time = time_us_32();
    char response[STR_LEN];
    size_t response_length = 0;
    bool found = false;

    for (int i = 0; i < 5; i++) {
        while ((time_us_32() - start_time) < UART_TIMEOUT_US) {
            if (uart_is_readable(uart)) {
                char received_char = uart_getc(uart);
                response[response_length++] = received_char;

                if (received_char == '\n') { // Check for end of line
                    response[response_length] = '\0'; // Null-terminate the string
                    lora_fmt_deveui(response, lora_deveui);
                    return true;
                }

                if (response_length >= STR_LEN - 1) {
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            break;
        }
    }

    return false; // Return false if no response or invalid response format
}

void test_LoRa(lora_state *current_state) {
    if (send_command("AT\r\n", false)) {
        *current_state = FIRMWARE;
    }
    else {
        *current_state = ERROR;
    }
}

void firmware_init(lora_state *current_state) {
    if (send_command("AT+VER\r\n", true)) {
        *current_state = DEV;
    }
    else {
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

    char lora_deveui[LORA_ID_LEN + 1];

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
                    printf("Connected to LoRa module!");
                }
                break;

            case FIRMWARE:
                firmware_init(&current_state);
                break;

            case DEV:
                if (lora_get_deveui(UART_ID, lora_deveui)) {
                    printf("LoRa DevEUI: %s\n", lora_deveui);
                    current_state = DO_NOTHING;
                }
                else {
                    printf("Failed to get LoRa DevEUI\n");
                    current_state = ERROR;
                }
                break;

            case ERROR:
                printf("Module not responding");
                current_state = DO_NOTHING;
                break;
        }
        sleep_ms(100);
    }
    return 0;
}
