// SPDX-License-Identifier: BSD-3-Clause
#include <linux/init.h>
#include <linux/module.h>
#include <linux/utsname.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");

static char *msg = "World";
module_param(msg, charp, 0644);
MODULE_PARM_DESC(msg, "Name to be greeted");

static time64_t start_time;

static int __init hello_init(void)
{
	pr_alert("Hello %s. You are usng Linux version %s\n", msg,
			utsname()->release);
	start_time = ktime_get_seconds();
	return 0;
}

static void __exit hello_exit(void)
{
	pr_alert("Goodbye. Thanks for using this module for %lld seconds\n",
			ktime_get_seconds() - start_time);
}

module_init(hello_init);
module_exit(hello_exit);
