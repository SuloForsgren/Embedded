#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define wrap_max 999
#define clock_divder_max 125
#define default_value 500
#define led_max 1000
#define led_step 50
#define led_off 0

int main() {
    const uint led_pin1 = 20;
    const uint led_pin2 = 21;
    const uint led_pin3 = 22;
    const uint button_dim_down = 7;
    const uint button_toggle = 8;
    const uint button_dim_up = 9;
    bool led_toggle = true;
    bool button_pressed = false;
    int led_brightness = 500;

    uint slice_led1 = pwm_gpio_to_slice_num(led_pin1);
    uint slice_led2 = pwm_gpio_to_slice_num(led_pin2);
    uint slice_led3 = pwm_gpio_to_slice_num(led_pin3);

    uint pwm_channel1 = pwm_gpio_to_channel(led_pin1);
    uint pwm_channel2 = pwm_gpio_to_channel(led_pin2);
    uint pwm_channel3 = pwm_gpio_to_channel(led_pin3);

    pwm_set_enabled(slice_led1, false);
    pwm_set_enabled(slice_led2, false);
    pwm_set_enabled(slice_led3, false);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, clock_divder_max);
    pwm_config_set_wrap(&config,wrap_max);

    pwm_init(slice_led1,&config,false);
    pwm_init(slice_led2,&config,false);
    pwm_init(slice_led3,&config,false);

    pwm_set_chan_level(slice_led1, pwm_channel1, default_value);
    pwm_set_chan_level(slice_led2, pwm_channel2, default_value);
    pwm_set_chan_level(slice_led3, pwm_channel3, default_value);

    gpio_set_function(led_pin1, GPIO_FUNC_PWM);
    gpio_set_function(led_pin2, GPIO_FUNC_PWM);
    gpio_set_function(led_pin3, GPIO_FUNC_PWM);

    pwm_set_enabled(slice_led1, true);
    pwm_set_enabled(slice_led2, true);
    pwm_set_enabled(slice_led3, true);

    gpio_init(button_dim_down);
    gpio_init(button_dim_up);
    gpio_init(button_toggle);

    gpio_pull_up(button_dim_down);
    gpio_pull_up(button_dim_up);
    gpio_pull_up(button_toggle);

    gpio_set_dir(button_dim_down, GPIO_IN);
    gpio_set_dir(button_dim_up, GPIO_IN);
    gpio_set_dir(button_toggle, GPIO_IN);
    stdio_init_all();

    while(1) {
        if (!gpio_get(button_dim_up) && led_toggle) {
            printf("Dimming up...\n");

            if (led_brightness < led_max) {
                led_brightness += led_step;
            }

            pwm_set_chan_level(slice_led1, pwm_channel1, led_brightness);
            pwm_set_chan_level(slice_led2, pwm_channel2, led_brightness);
            pwm_set_chan_level(slice_led3, pwm_channel3, led_brightness);
        }

        else if (!gpio_get(button_dim_down) && led_toggle) {
            printf("Dimming down...\n");
            if (led_brightness >= led_step) {
                led_brightness -= led_step;
            }
            else {
                led_brightness = led_off;
            }

            pwm_set_chan_level(slice_led1, pwm_channel1, led_brightness);
            pwm_set_chan_level(slice_led2, pwm_channel2, led_brightness);
            pwm_set_chan_level(slice_led3, pwm_channel3, led_brightness);
        }
        else if (!gpio_get(button_toggle) && !button_pressed) {
            button_pressed = true;

            if (led_toggle && led_brightness == 0) {
                led_brightness = default_value;
                pwm_set_chan_level(slice_led1, pwm_channel1, led_brightness);
                pwm_set_chan_level(slice_led2, pwm_channel2, led_brightness);
                pwm_set_chan_level(slice_led3, pwm_channel3, led_brightness);
            }
            else {
                if (!led_toggle) {
                    printf("Leds on...\n");
                    pwm_set_chan_level(slice_led1, pwm_channel1, led_brightness);
                    pwm_set_chan_level(slice_led2, pwm_channel2, led_brightness);
                    pwm_set_chan_level(slice_led3, pwm_channel3, led_brightness);
                    led_toggle = true;
                }
                else {
                    printf("Leds off...\n");
                    led_toggle = false;
                    pwm_set_chan_level(slice_led1, pwm_channel1, led_off);
                    pwm_set_chan_level(slice_led2, pwm_channel2, led_off);
                    pwm_set_chan_level(slice_led3, pwm_channel3, led_off);
                }
            }
        }
        else if (gpio_get(button_toggle) && button_pressed) {
            button_pressed = false;
        }
        sleep_ms(100);
    }
}
