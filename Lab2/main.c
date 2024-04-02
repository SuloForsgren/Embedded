#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#define led1 20
#define led2 21
#define led3 22
#define counter_clockwise 10
#define clockwise 11
#define rot_toggle 12
#define pwm_max 1000
#define step 50
#define debounce 100


void init_rotator() {
    gpio_init(counter_clockwise);
    gpio_init(clockwise);
    gpio_init(rot_toggle);

    gpio_set_dir(counter_clockwise, GPIO_IN);
    gpio_set_dir(clockwise, GPIO_IN);
    gpio_set_dir(rot_toggle, GPIO_IN);

    gpio_pull_up(rot_toggle);
}

void init_gpio() {

}

int main() {
    stdio_init_all();
    init_rotator();
    init_gpio();
}