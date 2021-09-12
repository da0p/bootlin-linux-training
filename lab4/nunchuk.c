// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>

/* Add your code here */

static int nunchuk_probe(struct i2c_client *cl, const struct i2c_device_id *id) 
{
	pr_info("Probing nunchuk\n");

	return 0;
}

static int nunchuk_remove(struct i2c_client *cl)
{
	pr_info("Removing nunchuk\n");

	return 0;
}

static const struct of_device_id nunchuk_of_match[] = {
	{ .compatible = "nintendo,nunchuk" },
	{}
};

static const struct i2c_device_id nunchuk_id[] = {
	{"nunchuk", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, nunchuk_id);

static struct i2c_driver nunchuk_drv = {
	.driver = {
		.name = "nunchuk-driver",
		.of_match_table = nunchuk_of_match,
	},
	.probe		= nunchuk_probe,
	.remove		= nunchuk_remove,
	.id_table	= nunchuk_id,
};

module_i2c_driver(nunchuk_drv);
MODULE_LICENSE("GPL");

