#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#define IN1 2
#define IN2 3
#define IN3 6
#define IN4 13
#define OPTO_FORK 28

#define UART_ID uart0

void init_gpio(void) {
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    gpio_init(IN1);
    gpio_set_dir(IN1, GPIO_OUT);

    gpio_init(IN2);
    gpio_set_dir(IN2, GPIO_OUT);

    gpio_init(IN3);
    gpio_set_dir(IN3, GPIO_OUT);

    gpio_init(IN4);
    gpio_set_dir(IN4, GPIO_OUT);
}

bool is_opto_fork_triggered(void) {
    return !gpio_get(OPTO_FORK);
}

void turn_off(void) {
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);
    sleep_ms(1);
}

void calibrate(bool *calibrated, uint8_t steps[][4], uint8_t step_count) {
    if (!(*calibrated)) {
        *calibrated = true;
        printf("Calibrating...\n");
        int i = 0;
        int rounds = 0;

        for (int j = 0; j < 900; j++) {
            gpio_put(IN1, steps[i % step_count][0]);
            gpio_put(IN2, steps[i % step_count][1]);
            gpio_put(IN3, steps[i % step_count][2]);
            gpio_put(IN4, steps[i % step_count][3]);
            sleep_ms(1); // Adjust the delay as needed
            i++;
        }


        while(!is_opto_fork_triggered() && rounds <= 3) {
            gpio_put(IN1, steps[i % step_count][0]);
            gpio_put(IN2, steps[i % step_count][1]);
            gpio_put(IN3, steps[i % step_count][2]);
            gpio_put(IN4, steps[i % step_count][3]);

            if (is_opto_fork_triggered()) {
                rounds++;
            }

            sleep_ms(1);
            i++;
        }
        printf("Calibration complete.\n");
        turn_off();
    }
    else {
        printf("Motor already calibrated.\n");
    }
}


void print_status(bool calibrated) {
    if (calibrated) {
        printf("Motor is calibrated.\n");
    } else {
        printf("Motor is not calibrated. Calibrate motor first.\n");
    }
}

int main(void) {
    stdio_init_all();
    printf("Booting...\n");

    init_gpio();

    uint8_t steps[][4] = {
            {1, 0, 0, 0},
            {1, 1, 0, 0},
            {0, 1, 0, 0},
            {0, 1, 1, 0},
            {0, 0, 1, 0},
            {0, 0, 1, 1},
            {0, 0, 0, 1},
            {1, 0, 0, 1}
    };

    uint8_t step_count = sizeof(steps) / sizeof(steps[0]);

    bool calibrated = false;

    while (true) {
        char command[10];
        printf("Enter command ('status', 'calib', 'run N'): ");
        scanf("%s", command);

        if (strcmp(command, "calib") == 0) {
            calibrate(&calibrated, steps, step_count);
        } else if (strcmp(command, "status") == 0) {
            print_status(calibrated);
        }
    }

    return 0;
}
