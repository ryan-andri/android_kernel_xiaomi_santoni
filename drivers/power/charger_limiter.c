/*
 * drivers/power/charger_limiter.c
 *
 * Charger limiter.
 *
 * Copyright (C) 2019, Ryan Andri <https://github.com/ryan-andri>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. For more details, see the GNU
 * General Public License included with the Linux kernel or available
 * at www.gnu.org/licenses
 */

#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include "power_supply.h"

/* DO NOT EDIT */
static struct kobject *charger_limiter;
static struct delayed_work charger_limiter_work;

/* Tunables */
static int charging_limit = 100;

static int battery_charging_enabled(struct power_supply *batt_psy, bool enable)
{
	const union power_supply_propval ret = {enable,};

	if (batt_psy->set_property)
		return batt_psy->set_property(batt_psy,
				POWER_SUPPLY_PROP_BATTERY_CHARGING_ENABLED,
				&ret);

	return -ENXIO;
}

static void charger_limiter_worker(struct work_struct *work)
{
	struct power_supply *batt_psy = power_supply_get_by_name("battery");
	struct power_supply *usb_psy = power_supply_get_by_name("usb");
	union power_supply_propval status, bat_percent;
	union power_supply_propval present = {0,}, charging_enabled = {0,};
	int ms_timer = 1000, rc = 0;

	/* re-schdule and increase the timer if not ready */
	if (!batt_psy->get_property || !usb_psy->get_property) {
		ms_timer = 10000;
		goto reschedule;
	}

	/* get values from /sys/class/power_supply/battery */
	batt_psy->get_property(batt_psy,
			POWER_SUPPLY_PROP_STATUS, &status);
	batt_psy->get_property(batt_psy,
			POWER_SUPPLY_PROP_CAPACITY, &bat_percent);
	batt_psy->get_property(batt_psy,
			POWER_SUPPLY_PROP_BATTERY_CHARGING_ENABLED, &charging_enabled);

	/* get values from /sys/class/power_supply/usb */
	usb_psy->get_property(usb_psy,
			POWER_SUPPLY_PROP_PRESENT, &present);

	if (status.intval == POWER_SUPPLY_STATUS_CHARGING || present.intval) {
		if (charging_enabled.intval &&
			bat_percent.intval >= charging_limit) {
			rc = battery_charging_enabled(batt_psy, 0);
			if (rc)
				pr_err("Failed to disable battery charging!\n");
		} else if (!charging_enabled.intval &&
			bat_percent.intval < charging_limit) {
			rc = battery_charging_enabled(batt_psy, 1);
			if (rc)
				pr_err("Failed to enable battery charging!\n");
		}
	} else if (bat_percent.intval < charging_limit || !present.intval) {
		if (!charging_enabled.intval) {
			rc = battery_charging_enabled(batt_psy, 1);
			if (rc)
				pr_err("Failed to enable battery charging!\n");
		}
	}

reschedule:
	schedule_delayed_work(&charger_limiter_work,
				msecs_to_jiffies(ms_timer));
}

/******************************************************************/
/*                         SYSFS START                            */
/******************************************************************/
static ssize_t show_charging_limit(struct kobject *kobj,
					struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", charging_limit);
}

static ssize_t store_charging_limit(struct kobject *kobj,
					struct attribute *attr, const char *buf,
					size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	if (input > 100)
		input = 100;

	if (input < 1)
		input = 1;

	charging_limit = input;

	return count;
}

static struct global_attr charging_limit_attr =
	__ATTR(charging_limit, 0644,
	show_charging_limit, store_charging_limit);

static struct attribute *charger_limiter_attributes[] = {
	&charging_limit_attr.attr,
	NULL
};

static struct attribute_group charger_limiter_attr_group = {
	.attrs = charger_limiter_attributes,
};
/******************************************************************/
/*                           SYSFS END                            */
/******************************************************************/

static int __init charger_limiter_init(void)
{
	int rc = 0;

	charger_limiter = kobject_create_and_add("charger_limiter", kernel_kobj);
	rc = sysfs_create_group(charger_limiter, &charger_limiter_attr_group);
	if (!rc) {
		INIT_DELAYED_WORK(&charger_limiter_work, charger_limiter_worker);
		schedule_delayed_work(&charger_limiter_work,
						msecs_to_jiffies(10000));
	}

	return 0;
}
late_initcall(charger_limiter_init);
