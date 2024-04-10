#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

#define LED_PIN_0 20
#define LED_PIN_1 21
#define LED_PIN_2 22

#define ROTARY_A_PIN 10
#define ROTARY_B_PIN 11
#define ROTARY_SWITCH_PIN 12

#define PWM_TOP_VALUE (1000)
#define PWM_CHANGE_STEP 25
#define DEBOUNCE_DELAY_MS 100

typedef enum {
    ROTATION_CLOCKWISE,
    ROTATION_COUNTERCLOCKWISE
} RotationDirection;

static queue_t rotation_events_queue;

volatile bool leds_enabled = false;

void initializeGPIO(void) {
    gpio_init(ROTARY_SWITCH_PIN);
    gpio_set_dir(ROTARY_SWITCH_PIN, GPIO_IN);
    gpio_pull_up(ROTARY_SWITCH_PIN);

    gpio_init(ROTARY_A_PIN);
    gpio_set_dir(ROTARY_A_PIN, GPIO_IN);

    gpio_init(ROTARY_B_PIN);
    gpio_set_dir(ROTARY_B_PIN, GPIO_IN);
}

void initializePWM(void) {
    uint pwm_slices[] = {
            pwm_gpio_to_slice_num(LED_PIN_0),
            pwm_gpio_to_slice_num(LED_PIN_1),
            pwm_gpio_to_slice_num(LED_PIN_2)
    };

    pwm_set_enabled(pwm_slices[0], false);
    pwm_set_enabled(pwm_slices[1], false);
    pwm_set_enabled(pwm_slices[2], false);

    pwm_config pwm_config_default = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&pwm_config_default, 125);
    pwm_config_set_wrap(&pwm_config_default, PWM_TOP_VALUE - 1);

    for (int i = 0; i < 3; i++) {
        pwm_init(pwm_slices[i], &pwm_config_default, false);
        gpio_set_function(LED_PIN_0 + i, GPIO_FUNC_PWM);
        pwm_set_enabled(pwm_slices[i], true);
    }
}

void setLedBrightness(uint16_t brightness) {
    for (int i = 0; i < 3; i++) {
        uint pwm_slice = pwm_gpio_to_slice_num(LED_PIN_0 + i);
        uint pwm_channel = pwm_gpio_to_channel(LED_PIN_0 + i);
        pwm_set_chan_level(pwm_slice, pwm_channel, brightness);
    }
}

void handleRotationEvent(bool isClockwise) {
    if (leds_enabled) {
        RotationDirection direction = isClockwise ? ROTATION_CLOCKWISE : ROTATION_COUNTERCLOCKWISE;
        queue_try_add(&rotation_events_queue, &direction);
    }
}


void toggleLeds(uint16_t *brightness) {
    if (!leds_enabled) {
        setLedBrightness(*brightness);
        leds_enabled = true;
    }
    else {
        if (*brightness == 0) {
            *brightness = 500;
            setLedBrightness(*brightness);
        }
        else {
            setLedBrightness(0);
            leds_enabled = false;
        }
    }
}

void rotaryEncoderISR(void) {
    static bool previousStateA = false;
    static bool previousStateB = false;

    bool currentStateA = gpio_get(ROTARY_B_PIN);
    bool currentStateB = gpio_get(ROTARY_A_PIN);

    if (currentStateA != previousStateA) {
        if (currentStateA == currentStateB) {
            handleRotationEvent(true);
        }
        else {
            handleRotationEvent(false);
        }
    }

    previousStateA = currentStateA;
    previousStateB = currentStateB;
}

int main(void) {
    stdio_init_all();

    initializeGPIO();
    initializePWM();

    queue_init(&rotation_events_queue, sizeof(RotationDirection), 10);

    uint16_t brightnessLevel = PWM_TOP_VALUE / 2;

    gpio_set_irq_enabled_with_callback(ROTARY_A_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &rotaryEncoderISR);
    gpio_set_irq_enabled_with_callback(ROTARY_B_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &rotaryEncoderISR);

    bool previousRotarySwitchState = !gpio_get(ROTARY_SWITCH_PIN);

    while (true) {
        bool currentRotarySwitchState = !gpio_get(ROTARY_SWITCH_PIN);

        if (currentRotarySwitchState != previousRotarySwitchState) {
            busy_wait_ms(DEBOUNCE_DELAY_MS);
            previousRotarySwitchState = currentRotarySwitchState;
            if (currentRotarySwitchState) {
                toggleLeds(&brightnessLevel);
            }
        }

        RotationDirection rotationDirection;
        while (queue_try_remove(&rotation_events_queue, &rotationDirection)) {
            if (rotationDirection == ROTATION_CLOCKWISE) {
                brightnessLevel += PWM_CHANGE_STEP;
                if (brightnessLevel > PWM_TOP_VALUE) {
                    brightnessLevel = PWM_TOP_VALUE;
                }
            }
            else {
                if (brightnessLevel < PWM_CHANGE_STEP) {
                    brightnessLevel = 0;
                }
                else {
                    brightnessLevel -= PWM_CHANGE_STEP;
                }
            }
            setLedBrightness(brightnessLevel);
        }
    }

    queue_free(&rotation_events_queue);

    return 0;
}
