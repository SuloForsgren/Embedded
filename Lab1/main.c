#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

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

    uint slice_led1 = pwm_gpio_to_slice_num(20);
    uint slice_led2 = pwm_gpio_to_slice_num(21);
    uint slice_led3 = pwm_gpio_to_slice_num(22);

    uint pwm_channel1 = pwm_gpio_to_channel(20);
    uint pwm_channel2 = pwm_gpio_to_channel(21);
    uint pwm_channel3 = pwm_gpio_to_channel(22);

    pwm_set_enabled(slice_led1, false);
    pwm_set_enabled(slice_led2, false);
    pwm_set_enabled(slice_led3, false);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 125);
    pwm_config_set_wrap(&config,1000);

    pwm_init(slice_led1,&config,false);
    pwm_init(slice_led2,&config,false);
    pwm_init(slice_led3,&config,false);

    pwm_set_chan_level(slice_led1, pwm_channel1, 500);
    pwm_set_chan_level(slice_led2, pwm_channel2, 500);
    pwm_set_chan_level(slice_led3, pwm_channel3, 500);

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

            if (led_brightness < 1000) {
                led_brightness += 50;
            }

            pwm_set_chan_level(slice_led1, pwm_channel1, led_brightness);
            pwm_set_chan_level(slice_led2, pwm_channel2, led_brightness);
            pwm_set_chan_level(slice_led3, pwm_channel3, led_brightness);
        }

        else if (!gpio_get(button_dim_down) && led_toggle) {
            printf("Dimming down...\n");
            if (led_brightness >= 50) {
                led_brightness -= 50;
            }
            else {
                led_brightness = 0;
            }

            pwm_set_chan_level(slice_led1, pwm_channel1, led_brightness);
            pwm_set_chan_level(slice_led2, pwm_channel2, led_brightness);
            pwm_set_chan_level(slice_led3, pwm_channel3, led_brightness);
        }
        else if (!gpio_get(button_toggle) && !button_pressed) {
            button_pressed = true;

            if (led_toggle && led_brightness == 0) {
                led_brightness = 500;
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
                    pwm_set_chan_level(slice_led1, pwm_channel1, 0);
                    pwm_set_chan_level(slice_led2, pwm_channel2, 0);
                    pwm_set_chan_level(slice_led3, pwm_channel3, 0);
                }
            }
        }
        else if (gpio_get(button_toggle) && button_pressed) {
            button_pressed = false;
        }
        sleep_ms(100);
    }
}
