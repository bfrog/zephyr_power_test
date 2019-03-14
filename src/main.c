/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sample.h"

#define BUSY_WAIT_DELAY_US		(10 * 1000000)

#define LPS1_STATE_ENTER_TO		10
#define LPS2_STATE_ENTER_TO		30
#define DEEP_SLEEP_STATE_ENTER_TO	90

#define DEMO_DESCRIPTION	\
	"Demo Description\n"	\
	"Application creates idleness, due to which System Idle Thread is\n"\
	"scheduled and it enters into various Low Power States.\n"\

struct device *gpio_port;

#ifdef CONFIG_BT_ENABLE

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <misc/printk.h>
#include <misc/byteorder.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

struct bt_conn *default_conn;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x05, 0x18),
};

static void connected(struct bt_conn *conn, u8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
	} else {
		default_conn = bt_conn_ref(conn);
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	if (default_conn) {
		bt_conn_unref(default_conn);
		default_conn = NULL;
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.cancel = auth_cancel,
};

#endif

/* Application main Thread */
void main(void)
{
	u32_t level = 0U;

	printk("\n\n*** Power Management Demo on %s ***\n", CONFIG_BOARD);
	printk(DEMO_DESCRIPTION);

	int err;

#ifdef CONFIG_BT_ENABLE
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	bt_conn_cb_register(&conn_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);
#endif


	//gpio_port = device_get_binding(PORT);

	///* Configure Button 1 as deep sleep trigger event */
	//gpio_pin_configure(gpio_port, BUTTON_1, GPIO_DIR_IN
	//					| GPIO_PUD_PULL_UP);

	///* Configure Button 2 as wake source from deep sleep */
	//gpio_pin_configure(gpio_port, BUTTON_2, GPIO_DIR_IN
	//					| GPIO_PUD_PULL_UP
	//					| GPIO_INT | GPIO_INT_LEVEL
	//					| GPIO_CFG_SENSE_LOW);

	//gpio_pin_enable_callback(gpio_port, BUTTON_2);

	///* Configure LEDs */
	//gpio_pin_configure(gpio_port, LED_1, GPIO_DIR_OUT);
	//gpio_pin_write(gpio_port, LED_1, LED_ON);

	//gpio_pin_configure(gpio_port, LED_2, GPIO_DIR_OUT);
	//gpio_pin_write(gpio_port, LED_2, LED_ON);

	/*
	 * Start the demo.
	 */
	for (int i = 1; i <= 8; i++) {
		unsigned int sleep_seconds;

		switch (i) {
		case 3:
			printk("\n<-- Disabling %s state --->\n",
					STRINGIFY(SYS_POWER_STATE_LOW_POWER_3));
			sys_pm_ctrl_disable_state(SYS_POWER_STATE_LOW_POWER_3);
			break;

		case 5:
			printk("\n<-- Enabling %s state --->\n",
				       STRINGIFY(SYS_POWER_STATE_LOW_POWER_3));
			sys_pm_ctrl_enable_state(SYS_POWER_STATE_LOW_POWER_3);

			printk("<-- Disabling %s state --->\n",
					STRINGIFY(SYS_POWER_STATE_LOW_POWER_2));
			sys_pm_ctrl_disable_state(SYS_POWER_STATE_LOW_POWER_2);
			break;

		case 7:
			printk("\n<-- Enabling %s state --->\n",
				       STRINGIFY(SYS_POWER_STATE_LOW_POWER_2));
			sys_pm_ctrl_enable_state(SYS_POWER_STATE_LOW_POWER_2);

			printk("<-- Forcing %s state --->\n",
				       STRINGIFY(SYS_POWER_STATE_LOW_POWER_3));
			sys_pm_force_power_state(SYS_POWER_STATE_LOW_POWER_3);
			break;

		default:
			/* Do nothing. */
			break;
		}

		printk("\n<-- App doing busy wait for 10 Sec -->\n");
		k_busy_wait(BUSY_WAIT_DELAY_US);

		sleep_seconds = (i % 2 != 0) ? LPS1_STATE_ENTER_TO :
					       LPS2_STATE_ENTER_TO;

		printk("\n<-- App going to sleep for %u Sec -->\n",
							sleep_seconds);
		k_sleep(K_SECONDS(sleep_seconds));
	}

	/* Restore automatic power management. */
	printk("\n<-- Forcing %s state --->\n",
		       STRINGIFY(SYS_POWER_STATE_AUTO));
	sys_pm_force_power_state(SYS_POWER_STATE_AUTO);

	printk("\nPress BUTTON1 to enter into Deep Sleep state. "
			"Press BUTTON2 to exit Deep Sleep state\n");
	while (1) {
		//gpio_pin_read(gpio_port, BUTTON_1, &level);
		//if (level == LOW) {
		//	k_sleep(K_SECONDS(DEEP_SLEEP_STATE_ENTER_TO));
		//}
		k_sleep(K_SECONDS(30));
	}
}
