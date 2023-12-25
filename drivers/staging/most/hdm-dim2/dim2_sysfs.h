/*
 * dim2_sysfs.h - MediaLB sysfs information
 *
 * Copyright (C) 2015, Microchip Technology Germany II GmbH & Co. KG
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This file is licensed under GPLv2.
 */

/* Author: Andrey Shvetsov <andrey.shvetsov@k2l.de> */

#ifndef DIM2_SYSFS_H
#define	DIM2_SYSFS_H

#include <linux/kobject.h>
#include <linux/mutex.h>

struct medialb_dci {
	struct kobject kobj_group;
	struct mutex mt; /* build vs show */
	u8 ni_state;
	u8 node_position;
	u16 node_address;
	u8 mep_eui48[6];
};

struct medialb_bus {
	struct kobject kobj_group;
};

struct dim2_hdm;

int dim2_sysfs_probe(struct medialb_bus **busp, struct medialb_dci **dcip,
		     struct kobject *parent_kobj);
void dim2_sysfs_destroy(struct medialb_bus *bus, struct medialb_dci *dci);

/*
 * callback,
 * must deliver MediaLB state as true if locked or false if unlocked
 */
bool dim2_sysfs_get_state_cb(void);

#endif	/* DIM2_SYSFS_H */
