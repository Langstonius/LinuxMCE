/*
 *
 *  $Id: pvrusb2-video-ivtv.h 2226 2009-03-07 05:17:32Z isely $
 *
 *  Copyright (C) 2005 Mike Isely <isely@pobox.com>
 *  Copyright (C) 2004 Aurelien Alleaume <slts@free.fr>
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

#ifndef __PVRUSB2_VIDEO_IVTV_H
#define __PVRUSB2_VIDEO_IVTV_H

/*

   This module connects the pvrusb2 driver to the I2C chip level
   driver which handles device video processing.  This interface is
   used internally by the driver; higher level code should only
   interact through the interface provided by pvrusb2-hdw.h.

*/

#include "pvrusb2-options.h"
#include "compat.h"

#ifdef PVR2_ENABLE_OLD_I2COPS
#ifdef PVR2_ENABLE_SAA7115

#include "pvrusb2-i2c-track.h"

int pvr2_i2c_decoder_ivtv_setup(struct pvr2_hdw *,struct pvr2_i2c_client *);

#endif /* PVR2_ENABLE_SAA7115 */
#endif /* PVR2_ENABLE_OLD_I2COPS */

#endif /* __PVRUSB2_VIDEO__IVTV_H */

/*
  Stuff for Emacs to see, in order to encourage consistent editing style:
  *** Local Variables: ***
  *** mode: c ***
  *** fill-column: 70 ***
  *** tab-width: 8 ***
  *** c-basic-offset: 8 ***
  *** End: ***
  */
