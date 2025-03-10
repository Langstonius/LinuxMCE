/*
 *
 *  $Id: pvrusb2-i2c-track.h 2226 2009-03-07 05:17:32Z isely $
 *
 *  Copyright (C) 2005 Mike Isely <isely@pobox.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef __PVRUSB2_I2C_TRACK_H
#define __PVRUSB2_I2C_TRACK_H

#include <linux/list.h>
#include <linux/i2c.h>
#include "pvrusb2-options.h"
#ifdef PVR2_ENABLE_V4L2SUBDEV
#include <media/v4l2-device.h>
#endif

#ifdef PVR2_ENABLE_OLD_I2COPS

struct pvr2_hdw;
struct pvr2_i2c_client;
struct pvr2_i2c_handler;
struct pvr2_i2c_handler_functions;
struct pvr2_i2c_op;
struct pvr2_i2c_op_functions;

struct pvr2_i2c_client {
	struct i2c_client *client;
	struct pvr2_i2c_handler *handler;
	struct list_head list;
	struct pvr2_hdw *hdw;
	int detected_flag;
	int recv_enable;
	unsigned long pend_mask;
	unsigned long ctl_mask;
	void (*status_poll)(struct pvr2_i2c_client *);
};

struct pvr2_i2c_handler {
	void *func_data;
	const struct pvr2_i2c_handler_functions *func_table;
};

struct pvr2_i2c_handler_functions {
	void (*detach)(void *);
	int (*check)(void *);
	void (*update)(void *);
	unsigned int (*describe)(void *,char *,unsigned int);
};

struct pvr2_i2c_op {
	int (*check)(struct pvr2_hdw *);
	void (*update)(struct pvr2_hdw *);
	const char *name;
};

void pvr2_i2c_track_init(struct pvr2_hdw *);
void pvr2_i2c_track_done(struct pvr2_hdw *);
void pvr2_i2c_track_attach_inform(struct i2c_client *);
void pvr2_i2c_track_detach_inform(struct i2c_client *);

int pvr2_i2c_client_cmd(struct pvr2_i2c_client *,unsigned int cmd,void *arg);
int pvr2_i2c_core_cmd(struct pvr2_hdw *,unsigned int cmd,void *arg);

int pvr2_i2c_core_check_stale(struct pvr2_hdw *);
void pvr2_i2c_core_sync(struct pvr2_hdw *);
void pvr2_i2c_core_status_poll(struct pvr2_hdw *);
unsigned int pvr2_i2c_report(struct pvr2_hdw *,char *buf,unsigned int maxlen);
#define PVR2_I2C_DETAIL_DEBUG   0x0001
#define PVR2_I2C_DETAIL_HANDLER 0x0002
#define PVR2_I2C_DETAIL_CTLMASK 0x0004
#define PVR2_I2C_DETAIL_ALL (\
	PVR2_I2C_DETAIL_DEBUG |\
	PVR2_I2C_DETAIL_HANDLER |\
	PVR2_I2C_DETAIL_CTLMASK)

void pvr2_i2c_probe(struct pvr2_hdw *,struct pvr2_i2c_client *);
const struct pvr2_i2c_op *pvr2_i2c_get_op(unsigned int idx);

#ifdef PVR2_ENABLE_V4L2SUBDEV
void pvr2_i2c_untrack_subdev(struct pvr2_hdw *, struct v4l2_subdev *sd);
#endif

#endif

#endif /* __PVRUSB2_I2C_CORE_H */


/*
  Stuff for Emacs to see, in order to encourage consistent editing style:
  *** Local Variables: ***
  *** mode: c ***
  *** fill-column: 75 ***
  *** tab-width: 8 ***
  *** c-basic-offset: 8 ***
  *** End: ***
  */
