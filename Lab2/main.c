#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"


int main() {
    const uint led_pin1 = 20;
    const uint led_pin2 = 21;
    const uint led_pin3 = 22;
    const uint rotator_toggle = 12;
    const uint rot_A = 10;
    const uint rot_B = 11;

    bool led_toggle = true;
    bool rotator_pressed = false;

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


    gpio_init(rotator_toggle);
    gpio_init(rot_A);
    gpio_init(rot_B);

    gpio_set_dir(rotator_toggle, GPIO_IN);
    gpio_set_dir(rotator_toggle, GPIO_IN);
    gpio_set_dir(rotator_toggle, GPIO_IN);

    gpio_pull_up(rotator_toggle);

    stdio_init_all();

    while (true) {
        if (!gpio_get(rotator_toggle) && !rotator_pressed) {
            rotator_pressed = true;
            if (led_toggle) {
                pwm_set_chan_level(slice_led1, pwm_channel1, 0);
                pwm_set_chan_level(slice_led2, pwm_channel2, 0);
                pwm_set_chan_level(slice_led3, pwm_channel3, 0);
                led_toggle = false;
            }
            else if (!led_toggle) {
                pwm_set_chan_level(slice_led1, pwm_channel1, 500);
                pwm_set_chan_level(slice_led2, pwm_channel2, 500);
                pwm_set_chan_level(slice_led3, pwm_channel3, 500);
                led_toggle = true;
            }
        }
        else if (gpio_get(rotator_toggle) && rotator_pressed) {
            rotator_pressed= false;
        }
        sleep_ms(100);
    }
}