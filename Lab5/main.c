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

void turn_off(void) {
    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
    gpio_put(IN3, 0);
    gpio_put(IN4, 0);
    sleep_ms(1);
}

void go_to_middle(uint8_t (*steps)[4], uint8_t step_count, uint16_t steps_per_rev, uint16_t steps_to_middle)
{
    uint16_t counter = 0;
    while(counter <= (steps_per_rev + steps_to_middle)) {
        for(uint16_t i = 0; i < step_count; i++) {
            gpio_put(IN1, steps[i][0]);
            gpio_put(IN2, steps[i][1]);
            gpio_put(IN3, steps[i][2]);
            gpio_put(IN4, steps[i][3]);
            sleep_ms(1);
            ++counter;
        }
    }
}

void go_to_opto_fork(uint8_t (*steps)[4], uint8_t step_count)
{
    bool opto_fork_state = gpio_get(OPTO_FORK);
    bool last_opto_fork_state = opto_fork_state;

    while(true) {
        opto_fork_state = gpio_get(OPTO_FORK);
        if(!opto_fork_state && last_opto_fork_state) {
            break;
        }

        for(uint16_t i = 0; i < step_count; i++) {
            gpio_put(IN1, steps[i][0]);
            gpio_put(IN2, steps[i][1]);
            gpio_put(IN3, steps[i][2]);
            gpio_put(IN4, steps[i][3]);
            sleep_ms(1);
        }

        last_opto_fork_state = opto_fork_state;
    }
    turn_off();
    printf("At opto fork.\n");
}

void calib(uint8_t (*steps)[4], uint8_t step_count, bool *calibrated, uint16_t *steps_per_rev, uint16_t *steps_to_middle)
{
    go_to_opto_fork(steps, step_count);

    uint32_t steps_outside_opto_fork = 0;
    uint32_t steps_at_opto_fork = 0;

    uint8_t revs = 0;

    while(revs != 3) {
        while(!gpio_get(OPTO_FORK)) {
            for(uint16_t i = 0; i < step_count; i++) {
                gpio_put(IN1, steps[i][0]);
                gpio_put(IN2, steps[i][1]);
                gpio_put(IN3, steps[i][2]);
                gpio_put(IN4, steps[i][3]);
                sleep_ms(1);
                ++steps_at_opto_fork;
            }
        }
        while(gpio_get(OPTO_FORK)) {
            for(uint16_t i = 0; i < step_count; i++) {
                gpio_put(IN1, steps[i][0]);
                gpio_put(IN2, steps[i][1]);
                gpio_put(IN3, steps[i][2]);
                gpio_put(IN4, steps[i][3]);
                sleep_ms(1);
                ++steps_outside_opto_fork;
            }
        }
        ++revs;
    }
    *steps_to_middle = (steps_at_opto_fork / 3) / 2;
    *steps_per_rev = (steps_at_opto_fork + steps_outside_opto_fork) / 3;
    *calibrated = true;
    printf("Calibrated. Going to middle.\n");
    turn_off();
}

void status(bool calibrated, uint16_t steps_per_rev)
{
    if(calibrated) {
        printf("Motor is calibrated.\n");
        printf("Number of steps per revolution: %d\n", steps_per_rev);
    } else {
        printf("Motor is not calibrated.\n");
        printf("Number of steps per revolution not available.\n");
    }
}

void drop_pills(uint8_t (*steps)[4], uint8_t step_count, uint16_t steps_per_rev, uint8_t n)
{
    uint32_t steps_to_run;
    if(n == 0 || n == 8) {
        steps_to_run = steps_per_rev;
    } else {
        steps_to_run = steps_per_rev / 8 * n;
    }

    for(uint32_t i = 0; i < steps_to_run; i++) {
        gpio_put(IN1, steps[i % step_count][0]);
        gpio_put(IN2, steps[i % step_count][1]);
        gpio_put(IN3, steps[i % step_count][2]);
        gpio_put(IN4, steps[i % step_count][3]);
        sleep_ms(1);
    }

    turn_off();
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
    uint16_t steps_per_rev;
    uint16_t steps_to_middle;

    while (true) {
        char command[10];
        printf("Enter command:\n");
        fgets(command, sizeof(command), stdin);

        if (strncmp(command, "calib", 5) == 0) {
            calib(steps, step_count, &calibrated, &steps_per_rev, &steps_to_middle);
            go_to_middle(steps, step_count, steps_per_rev, steps_to_middle);
        }
        else if (strncmp(command, "status", 6) == 0) {
            status(calibrated, steps_per_rev);
        }
        else if (strncmp(command, "run", 3) == 0) {
            if (calibrated) {
                uint8_t n = 0;
                if (strlen(command) > 4) {
                    n = atoi(command + 4);
                }
                drop_pills(steps, step_count, steps_per_rev, n);
            }
        }
    }
    return 0;
}
