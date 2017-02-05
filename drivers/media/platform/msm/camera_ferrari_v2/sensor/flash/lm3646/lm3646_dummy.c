/*
 * LM3646 Dummy driver to allow userspace control
 *
 * Copyright (c) 2017, thewisenerd <thewisenerd@protonmail.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/leds.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/sysfs.h>
#include "lm3646_dummy.h"

#define DRIVER_NAME "dummy,lm3646"
#define LOGTAG "[LM3646]"
#define LOGTAGD LOGTAG "[D] "
#define LOGTAGE LOGTAG "[E] "
#define LM3646_DBG(fmt, args...) pr_err(LOGTAGD fmt, ##args)
#define LM3646_ERR(fmt, args...) pr_err(LOGTAGE fmt, ##args)

struct __lm3646_dummy_data lm3646_dummy_data;

static uint16_t lm3646_get_torch_brightness_value(unsigned int brightness) {
	return LM3646_REG_MAX_CURRENT(			\
		(brightness >> 5),			\
		DEFAULT_MAX_FLASH_CURRENT		\
	);
};

static void lm3646_set_brightness(unsigned int brightness) {
	int rc = 0;

	LM3646_DBG("%s: enter!\n", __func__);

	brightness = brightness & 0xFF;

	if (lm3646_dummy_data.brightness == brightness) {
		LM3646_DBG("%s: same brightness value = %u; return\n",
				__func__, brightness);
		return;
	}

	LM3646_DBG("%s: %u\n", __func__, brightness);

	if (brightness == 0) {
		if (fctrl.func_tbl->flash_led_low) {
			rc = fctrl.func_tbl->flash_led_low(&fctrl);
			if (rc < 0) {
				LM3646_ERR("%s:%d fail\n", __func__, __LINE__);
				return;
			}
		}
		if (fctrl.func_tbl->flash_led_release) {
			rc = fctrl.func_tbl->flash_led_release(&fctrl);
			if (rc < 0) {
				LM3646_ERR("%s:%d fail\n", __func__, __LINE__);
				return;
			}
		}
	} else {
		lm3646_init_array[4].reg_data =		\
			lm3646_get_torch_brightness_value(brightness);

		if (fctrl.func_tbl->flash_led_init) {
			rc = fctrl.func_tbl->flash_led_init(&fctrl);
			if (rc < 0) {
				LM3646_ERR("%s:%d fail\n", __func__, __LINE__);
				return;
			}
		}
		if (fctrl.func_tbl->flash_led_low) {
			rc = fctrl.func_tbl->flash_led_low(&fctrl);
			if (rc < 0) {
				LM3646_ERR("%s:%d fail\n", __func__, __LINE__);
				return;
			}
		}
	}

	lm3646_dummy_data.brightness = brightness;
}

static void lm3646_dummy_led_set_brightness(struct led_classdev *cdev,
						enum led_brightness brightness)
{
	lm3646_set_brightness(brightness);
}

static enum led_brightness lm3646_dummy_led_get_brightness(struct led_classdev *cdev)
{
	LM3646_DBG("%s: enter!\n", __func__);
	return lm3646_dummy_data.brightness;
}

static struct led_classdev lm3646_dummy_led_cdev = {
	.name = "torch",
	.brightness_set = lm3646_dummy_led_set_brightness,
	.brightness_get = lm3646_dummy_led_get_brightness,
};

static int lm3646_dummy_led_probe(struct platform_device *pdev)
{
	return led_classdev_register(&pdev->dev, &lm3646_dummy_led_cdev);
}

static int lm3646_dummy_led_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&lm3646_dummy_led_cdev);
	return 0;
}

static struct platform_driver lm3646_dummy_led_driver = {
	.probe = lm3646_dummy_led_probe,
	.remove = lm3646_dummy_led_remove,
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static struct platform_device lm3646_dummy_led_pdev = {
	.name   = DRIVER_NAME,
	.id     = 0,
};

static int __init lm3646_dummy_led_init(void)
{
	int ret;

	LM3646_DBG("%s: enter!\n", __func__);

	if ((ret = platform_driver_register(&lm3646_dummy_led_driver)) != 0) {
		LM3646_ERR("%s: platform_driver_register failed!\n", __func__);
		return ret;
	}

	if ((ret = platform_device_register(&lm3646_dummy_led_pdev)) != 0) {
		LM3646_ERR("%s: platform_device_register failed!\n", __func__);
		platform_driver_unregister(&lm3646_dummy_led_driver);
	}

	return ret;
}

static void __exit lm3646_dummy_led_exit(void)
{
	platform_driver_unregister(&lm3646_dummy_led_driver);
}

module_init(lm3646_dummy_led_init);
module_exit(lm3646_dummy_led_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("LM3646 Dummy LED driver");
MODULE_AUTHOR("thewisenerd <thewisenerd@protonmail.com>");
