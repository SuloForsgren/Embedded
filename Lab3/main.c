#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// Define UART pins
#define UART_ID uart0
#define BAUD_RATE 9600

int main() {
    // Initialize stdio for printf
    //stdio_init_all();

    // Initialize UART with baud rate
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(0, GPIO_FUNC_UART);  // GPIO0 is UART TX
    gpio_set_function(1, GPIO_FUNC_UART);  // GPIO1 is UART RX

    // Initialize debug LEDs
    gpio_init(21);
    gpio_set_dir(21, GPIO_OUT);
    gpio_init(22);
    gpio_set_dir(22, GPIO_OUT);

    // Enable FIFOs and set UART RX FIFO level
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);
    uart_set_irq_enables(UART_ID, true, false);

    // Main loop
    while (true) {
        // Check if data is available in the UART receive FIFO

        if (uart_is_readable(UART_ID)) {
            uart_puts(UART_ID, "Hello, UART!\n");
            // Read the received byte from UART
            char rx_byte = uart_getc(UART_ID);

            // Flash the LED
            gpio_put(21, true); // Turn on LED
            sleep_ms(100); // Wait for 100 milliseconds
            gpio_put(21, false); // Turn off LED

            // Print the received byte
            printf("Received: %c\n", rx_byte);
        }

        // Delay before checking for new data
        sleep_ms(100);
    }
}
