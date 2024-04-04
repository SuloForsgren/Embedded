#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#define LED1 20
#define LED2 21
#define LED3 22

#define ROT_CCW 10
#define ROT_CW 11
#define ROT_TOGGLE 12

#define PWM_MAX 1000
#define LED_STEP 50
#define DEBOUNCE 100

static queue_t events_clockwise;
static queue_t events_counterclockwise;

volatile bool toggle_leds = false;
volatile bool clockwise = false;
volatile bool counterclockwise = false;
volatile uint16_t prev_brightness = PWM_MAX / 2; // Store previous brightness level

uint16_t steps = PWM_MAX / 2;

void control_leds() {
    uint slice_num_1 = pwm_gpio_to_slice_num(LED1);
    uint slice_num_2 = pwm_gpio_to_slice_num(LED2);
    uint slice_num_3 = pwm_gpio_to_slice_num(LED3);

    uint channel_num_1 = pwm_gpio_to_channel(LED1);
    uint channel_num_2 = pwm_gpio_to_channel(LED2);
    uint channel_num_3 = pwm_gpio_to_channel(LED3);

    pwm_set_chan_level(slice_num_1, channel_num_1, steps);
    pwm_set_chan_level(slice_num_2, channel_num_2, steps);
    pwm_set_chan_level(slice_num_3, channel_num_3, steps);
}

void gpio_handler() {
    static bool prev_state_a = false;
    static bool prev_state_b = false;

    bool state_a = gpio_get(ROT_CW);
    bool state_b = gpio_get(ROT_CCW);

    if (toggle_leds) {
        if (state_a != prev_state_a) {
            if (state_a == state_b) {
                clockwise = true;
                queue_try_add(&events_clockwise, &clockwise);
            }
            else {
                counterclockwise = true;
                queue_try_add(&events_counterclockwise, &counterclockwise);
            }
        }
    }

    prev_state_a = state_a;
    prev_state_b = state_b;
}

void led_toggle() {
    if (!gpio_get(ROT_TOGGLE)) {
        toggle_leds = !toggle_leds;
        if (steps == 0 && !toggle_leds) {
            steps = PWM_MAX / 2;
            toggle_leds = true;
        }
        else if (toggle_leds) {
            steps = prev_brightness;
        }
        else {
            prev_brightness = steps;
            steps = 0;
        }
        control_leds();
    }
}


int main() {
    gpio_init(ROT_TOGGLE);
    gpio_set_dir(ROT_TOGGLE, GPIO_IN);
    gpio_pull_up(ROT_TOGGLE);

    gpio_init(ROT_CCW);
    gpio_set_dir(ROT_CCW, GPIO_IN);

    gpio_init(ROT_CW);
    gpio_set_dir(ROT_CW, GPIO_IN);

    uint slice_num_1 = pwm_gpio_to_slice_num(LED1);
    uint slice_num_2 = pwm_gpio_to_slice_num(LED2);
    uint slice_num_3 = pwm_gpio_to_slice_num(LED3);

    pwm_set_enabled(slice_num_1, false);
    pwm_set_enabled(slice_num_2, false);
    pwm_set_enabled(slice_num_3, false);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 125);
    pwm_config_set_wrap(&config, PWM_MAX - 1);

    pwm_init(slice_num_1, &config, false);
    pwm_init(slice_num_2, &config, false);
    pwm_init(slice_num_3, &config, false);

    gpio_set_function(LED1, GPIO_FUNC_PWM);
    gpio_set_function(LED2, GPIO_FUNC_PWM);
    gpio_set_function(LED3, GPIO_FUNC_PWM);

    pwm_set_enabled(slice_num_1, true);
    pwm_set_enabled(slice_num_2, true);
    pwm_set_enabled(slice_num_3, true);

    stdio_init_all();

    queue_init(&events_clockwise, sizeof(bool), 10);
    queue_init(&events_counterclockwise, sizeof(bool), 10);

    gpio_set_irq_enabled_with_callback(ROT_CW, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_handler);
    gpio_set_irq_enabled_with_callback(ROT_CCW, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_handler);

    bool prev_rot_sw_state = gpio_get(ROT_TOGGLE);

    while (true) {
        bool rot_sw_state = !gpio_get(ROT_TOGGLE);
        if (rot_sw_state != prev_rot_sw_state) {
            sleep_ms(DEBOUNCE);
            prev_rot_sw_state = rot_sw_state;
            if (rot_sw_state) {
                led_toggle();
            }
        }

        while (queue_try_remove(&events_clockwise, &clockwise)) {
            steps += LED_STEP;
            if (steps > PWM_MAX) {
                steps = PWM_MAX;
            }
            control_leds();
        }

        while (queue_try_remove(&events_counterclockwise, &counterclockwise)) {
            if (steps < LED_STEP) {
                steps = 0;
            } else {
                steps -= LED_STEP;
            }
            control_leds();
        }
    }

    queue_free(&events_clockwise);
    queue_free(&events_counterclockwise);

    return 0;
}
