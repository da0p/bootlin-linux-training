// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/delay.h>

/* Add your code here */

struct nunchuk_dev {
	struct i2c_client *i2c_client;
};

static int nunchuk_read_registers(struct i2c_client *client, u8 *recv)
{
	u8 buf[1];
	int ret;

	// Ask the device to get ready for a read 
	usleep_range(10000, 20000);
	
	buf[0] = 0x00;
	ret = i2c_master_send(client, buf, 1);
	if (ret < 0) {
		dev_err(&client->dev, "i2c send failed (%d)\n", ret);
		return ret;
	}

	usleep_range(10000, 20000);

	// Now read registers
	ret = i2c_master_recv(client, recv, 6);
	if (ret < 0) {
		dev_err(&client->dev, "i2c recv failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

static void nunchuk_poll(struct input_dev *input)
{
	u8 recv[6];
	int zpressed, cpressed, bx, by;

	// Retrieve the physical i2c device
	struct nunchuk_dev *nunchuk = input_get_drvdata(input);
	struct i2c_client *client = nunchuk->i2c_client;

	// Get the state of the device registers
	if (nunchuk_read_registers(client, recv) < 0)
		return;

	zpressed = (recv[5] & BIT(0)) ? 0 : 1;
	cpressed = (recv[5] & BIT(1)) ? 0 : 1;
	bx = recv[0];
	by = recv[1];

	// Send events to the INPUT subsystem
	input_report_key(input, BTN_Z, zpressed);
	input_report_key(input, BTN_C, cpressed);

	input_report_abs(input, ABS_X, bx);
	input_report_abs(input, ABS_Y, by);

	input_sync(input);
}

static int nunchuk_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{
	struct nunchuk_dev *nunchuk;
	struct input_dev *input;
	u8 buf[2];
	int ret;

	// Alocate per device structure
	nunchuk = devm_kzalloc(&client->dev, sizeof(*nunchuk), GFP_KERNEL);
	if (!nunchuk)
		// No message necessary here, already issued by allocation
		// functions
		return -ENOMEM;
	
	// Initialize device
	buf[0] = 0xf0;
	buf[1] = 0x55;

	ret = i2c_master_send(client, buf, 2);
	if (ret < 0) {
		dev_err(&client->dev, "i2c send failed (%d)\n", ret);
		return ret;
	}

	udelay(1000);

	buf[0] = 0xfb;
	buf[1] = 0x00;

	ret = i2c_master_send(client, buf, 2);
	if (ret < 0) {
		dev_err(&client->dev, "i2c send failed (%d)\n", ret);
	}

	// Alocate input device 
	input = devm_input_allocate_device(&client->dev);
	if (!input)
		return -ENOMEM;

	// Implement pointers from logical to physical. Here, no need for
	// physical to logical pointers as unregistering and freeing the
	// polled_input device will be automatic.
	nunchuk->i2c_client = client;
	input_set_drvdata(input, nunchuk);
	
	// Configure input device
	input->name = "Wii Nunchuk";
	input->id.bustype = BUS_I2C;

	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_C, input->keybit);
	set_bit(BTN_Z, input->keybit);

	set_bit(ABS_X, input->absbit);
	set_bit(ABS_Y, input->absbit);
	input_set_abs_params(input, ABS_X, 30, 220, 4, 8);
	input_set_abs_params(input, ABS_Y, 40, 200, 4, 8);

	// classic buttons
	set_bit(BTN_TL, input->keybit);
	set_bit(BTN_SELECT, input->keybit);
	set_bit(BTN_MODE, input->keybit);
	set_bit(BTN_START, input->keybit);
	set_bit(BTN_TR, input->keybit);
	set_bit(BTN_TL2, input->keybit);
	set_bit(BTN_B, input->keybit);
	set_bit(BTN_Y, input->keybit);
	set_bit(BTN_A, input->keybit);
	set_bit(BTN_X, input->keybit);
	set_bit(BTN_TR2, input->keybit);

	// Register and configure polling function
	ret = input_setup_polling(input, nunchuk_poll);
	if (ret) {
		dev_err(&client->dev, "Failed to set polling function (%d)\n", ret);
		return ret;
	}

	input_set_poll_interval(input, 50);

	// Register the input device when everything is ready
	ret = input_register_device(input);
	if (ret) {
		dev_err(&client->dev, "Cannot register input device (%d)\n", ret);
		return ret;
	}

	return 0;
}

static int nunchuk_remove(struct i2c_client *client)
{
	pr_info("Removing nunchuk\n");
	// Nothing to do here, as the polled input device is automatically
	// unregistered and freed since we use devm_input_allocate_device.

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

