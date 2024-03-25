/*
 * Copyright (c) 2022 Mr. Green's Workshop https://www.MrGreensWorkshop.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/counter.h>

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>

#define DELAY 4000000
#define ALARM_CHANNEL_ID 0

struct counter_alarm_cfg alarm_cfg;

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);
#define TIMER DT_INST(0, st_stm32_rtc)

/* The devicetree node identifier for the "led" alias. */
#define LED_NODE DT_ALIAS(led)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

static void test_counter_interrupt_fn(const struct device *counter_dev,
                                      uint8_t chan_id, uint32_t ticks,
                                      void *user_data)
{
    struct counter_alarm_cfg *config = user_data;
    uint32_t now_ticks;
    uint64_t now_usec;
    int now_sec;
    int err;

    err = counter_get_value(counter_dev, &now_ticks);
    if (err)
    {
        LOG_INF("Failed to read counter value (err %d)", err);
        return;
    }

    now_usec = counter_ticks_to_us(counter_dev, now_ticks);
    now_sec = (int)(now_usec / USEC_PER_SEC);

    LOG_INF("!!! Alarm !!!");
    LOG_INF("Now: %u", now_sec);

    gpio_pin_toggle_dt(&led);

    /* Set a new alarm with a double length duration */
    // config->ticks = config->ticks * 2U;

    LOG_INF("Set alarm in %u sec (%u ticks)",
            (uint32_t)(counter_ticks_to_us(counter_dev,
                                           config->ticks) /
                       USEC_PER_SEC),
            config->ticks);

    err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
                                    user_data);
    if (err != 0)
    {
        LOG_INF("Alarm could not be set");
    }
}

int main(void)
{
    int ret;
    bool led_state = true;

    if (!gpio_is_ready_dt(&led))
    {
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        return 0;
    }

    LOG_INF("Zephyr running on H723VG with early USB console");

    const struct device *const counter_dev = DEVICE_DT_GET(TIMER);

    if (!device_is_ready(counter_dev))
    {
        LOG_INF("device not ready.\n");
        return 0;
    }

    counter_start(counter_dev);

    alarm_cfg.flags = 0;
    alarm_cfg.ticks = counter_us_to_ticks(counter_dev, DELAY);
    alarm_cfg.callback = test_counter_interrupt_fn;
    alarm_cfg.user_data = &alarm_cfg;

    int err;
    err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
                                    &alarm_cfg);
    LOG_INF("Set alarm in %u sec (%u ticks)\n",
            (uint32_t)(counter_ticks_to_us(counter_dev,
                                           alarm_cfg.ticks) /
                       USEC_PER_SEC),
            alarm_cfg.ticks);

    if (-EINVAL == err)
    {
        LOG_INF("Alarm settings invalid\n");
    }
    else if (-ENOTSUP == err)
    {
        LOG_INF("Alarm setting request not supported\n");
    }
    else if (err != 0)
    {
        LOG_INF("Error\n");
    }

    while (1)
    {
        k_sleep(K_FOREVER);
    }
    return 0;
}
