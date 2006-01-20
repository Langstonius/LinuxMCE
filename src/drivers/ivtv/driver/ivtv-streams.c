/*
    init/start/stop/exit stream functions
    Copyright (C) 2003-2004  Kevin Thayer <nufan_wfk at yahoo.com>

    Copyright (C) 2004  Chris Kennedy <c@groovy.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* License: GPL
 * Author: Kevin Thayer <nufan_wfk at yahoo dot com>
 *
 * This file will hold API related functions, both internal (firmware api)
 * and external (v4l2, etc)
 *
 * -----
 * MPG600/MPG160 support by  T.Adachi <tadachi@tadachi-net.com>
 *                      and Takeru KOMORIYA<komoriya@paken.org>
 *
 * AVerMedia M179 GPIO info by Chris Pinkham <cpinkham@bc2va.org>
 *                using information provided by Jiun-Kuei Jung @ AVerMedia.
 */

#include "ivtv-driver.h"
#include "ivtv-reset.h"
#include "ivtv-fileops.h"
#include "ivtv-i2c.h"
#include "ivtv-reset.h"
#include "ivtv-queue.h"
#include "ivtv-mailbox.h"
#include "ivtv-audio.h"
#include "ivtv-video.h"
#include "ivtv-kthreads.h"
#include "ivtv-vbi.h"
#include "ivtv-ioctl.h"
#include "ivtv-irq.h"
#include "ivtv-streams.h"
#include "ivtv-cards.h"
#include "audiochip.h"
#include <linux/smp_lock.h>

#define IVTV_V4L2_MAX_MINOR 15

static struct file_operations ivtv_v4l2_enc_fops = {
      owner:THIS_MODULE,
      read:ivtv_v4l2_read,
      write:ivtv_v4l2_write,
      open:ivtv_v4l2_open,
      ioctl:ivtv_v4l2_ioctl,
      release:ivtv_v4l2_close,
      poll:ivtv_v4l2_enc_poll,
};

static struct file_operations ivtv_v4l2_dec_fops = {
      owner:THIS_MODULE,
      read:ivtv_v4l2_read,
      write:ivtv_v4l2_write,
      open:ivtv_v4l2_open,
      ioctl:ivtv_v4l2_ioctl,
      release:ivtv_v4l2_close,
      poll:ivtv_v4l2_dec_poll,
};

static struct video_device ivtv_v4l2dev_template = {
	.name = "cx2341x",
	.type = VID_TYPE_CAPTURE | VID_TYPE_TUNER | VID_TYPE_TELETEXT |
	    VID_TYPE_CLIPPING | VID_TYPE_SCALES | VID_TYPE_MPEG_ENCODER,
	.fops = &ivtv_v4l2_enc_fops,
	.minor = -1,
};

static int ivtv_stream_init(struct ivtv *itv, int streamtype,
		     int buffers, int bufsize, int dma)
{
	struct ivtv_stream *s = &itv->streams[streamtype];

	if (buffers) {
		if (itv->options.dynbuf) {
		        IVTV_INFO("Create %s%s stream: "
				"%d x %d buffers (%dKB total)\n",
				dma != PCI_DMA_NONE ? "DMA " : "",
				ivtv_stream_name(streamtype), 
				(buffers / bufsize), bufsize,
				(buffers / 1024));
                }
	} else {
		IVTV_INFO("Create %s%s stream\n",
			       dma != PCI_DMA_NONE ? "DMA " : "",
			       ivtv_stream_name(streamtype)); 
	}

	/* Translate streamtype to buffers limit */
	if (streamtype == IVTV_ENC_STREAM_TYPE_MPG) {
		s->buf_min = (int)(itv->options.mpg_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_mpg_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_ENC_STREAM_TYPE_YUV) {
		s->buf_min = (int)(itv->options.yuv_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_yuv_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_ENC_STREAM_TYPE_VBI) {
		s->buf_min = (int)(itv->options.vbi_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_vbi_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_ENC_STREAM_TYPE_PCM) {
		s->buf_min = (int)(itv->options.pcm_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_pcm_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_ENC_STREAM_TYPE_RAD) {
		s->buf_min = (int)(itv->options.pcm_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_pcm_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_DEC_STREAM_TYPE_MPG) {
		s->buf_min = (int)(itv->options.dec_mpg_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_dec_mpg_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_DEC_STREAM_TYPE_VBI) {
		s->buf_min = (int)(itv->options.dec_vbi_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_dec_vbi_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_DEC_STREAM_TYPE_VOUT) {
		s->buf_min = (int)(itv->options.dec_vbi_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_dec_vbi_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_DEC_STREAM_TYPE_YUV) {
		s->buf_min = (int)(itv->options.dec_yuv_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_dec_yuv_buffers * ((1024 * 1024)));
	} else if (streamtype == IVTV_DEC_STREAM_TYPE_OSD) {
		s->buf_min = (int)(itv->options.dec_osd_buffers * ((1024 * 1024)));
		s->buf_max = (int)(itv->options.max_dec_osd_buffers * ((1024 * 1024)));
	} else {
		IVTV_DEBUG_WARN(
			   "Stream Init: Unknown stream: %d\n", streamtype);
		return -EIO;
	}

	/* Make it easier to know what type it is */
	s->type = streamtype;

	/* Step 2: initialize ivtv_stream fields */
	init_waitqueue_head(&s->waitq);
	init_waitqueue_head(&s->udma.waitq);
	init_MUTEX(&s->mlock);
	s->slock = SPIN_LOCK_UNLOCKED;
	s->ubytes = 0;
	s->dma = dma;
	s->id = -1;
	s->SG_handle = IVTV_DMA_UNMAPPED;
	s->SGarray = NULL;
	s->SGlist = NULL;
	s->SGlen = 0;
	s->SG_length = 0;
	s->buffers = buffers;
	s->bufsize = bufsize;
	s->buf_total = 0;
	s->buf_fill = 0;
	s->dmatype = 0;
	atomic_set(&s->allocated_buffers, 0);
	INIT_LIST_HEAD(&s->free_q.list);
	INIT_LIST_HEAD(&s->full_q.list);
	INIT_LIST_HEAD(&s->dma_q.list);
	INIT_LIST_HEAD(&s->io_q.list);

	s->udma.SGarray = NULL;
	s->udma.SGlist = NULL;
	s->udma.map = NULL;
	s->udma.SGlen = 0;
	s->udma.SG_length = 0;
	s->udma.page_count = 0;
	s->udma.offset = 0;
	s->udma.sg_size = 0;

	return 0;
}

static int ivtv_reg_dev(struct ivtv *itv, int streamtype, int minor, int reg_type)
{
	struct ivtv_stream *s = &itv->streams[streamtype];

	if (minor > -1)
		minor += ivtv_first_minor;	/* skip any other TV cards */

	/* Step 1: allocate and initialize the v4l2 video device structure */
	s->v4l2dev = video_device_alloc();
	if (NULL == s->v4l2dev) {
		IVTV_ERR("Couldn't allocate v4l2 video_device\n");
		return -ENOMEM;
	}
	memcpy(s->v4l2dev, &ivtv_v4l2dev_template, sizeof(struct video_device));
	if (itv->v4l2_cap & V4L2_CAP_VIDEO_OUTPUT) {
		s->v4l2dev->type |= VID_TYPE_MPEG_DECODER;
	}
	snprintf(s->v4l2dev->name, sizeof(s->v4l2dev->name), "ivtv%d %s",
			itv->num, ivtv_stream_name(streamtype));

	s->v4l2dev->minor = minor;
#ifdef LINUX26
	s->v4l2dev->dev = &itv->dev->dev;
#endif /* LINUX26 */
	s->v4l2dev->release = video_device_release;

	/* We're done if this is a 'hidden' stream (OSD) */
	if (minor < 0)
		return 0;

	/* Step 3: register device. First try the desired minor, 
	   then any free one. */
	if (video_register_device(s->v4l2dev, reg_type, minor) &&
	    video_register_device(s->v4l2dev, reg_type, -1)) {
		IVTV_ERR("Couldn't register v4l2 device for %s minor %d\n",
			      ivtv_stream_name(streamtype), minor);

		video_device_release(s->v4l2dev);
		s->v4l2dev = NULL;

		return -ENOMEM;
	}
	IVTV_DEBUG_INFO("Registered v4l2 device for %s minor %d\n",
		       ivtv_stream_name(streamtype), s->v4l2dev->minor);

	/* Success! All done. */

	return 0;
}

static int ivtv_dev_setup(struct ivtv *itv, int s)
{
	switch (s) {
		case IVTV_ENC_STREAM_TYPE_MPG:
			return ivtv_reg_dev(itv, s, itv->num, VFL_TYPE_GRABBER);
		case IVTV_ENC_STREAM_TYPE_YUV:
			return ivtv_reg_dev(itv, s, itv->num + IVTV_V4L2_ENC_YUV_OFFSET,
				 VFL_TYPE_GRABBER);
		case IVTV_ENC_STREAM_TYPE_VBI:
			return ivtv_reg_dev(itv, s, itv->num, VFL_TYPE_VBI);
		case IVTV_ENC_STREAM_TYPE_PCM:
			return ivtv_reg_dev(itv, s, itv->num + IVTV_V4L2_ENC_PCM_OFFSET,
				 VFL_TYPE_GRABBER);
		case IVTV_ENC_STREAM_TYPE_RAD:
			if (!(itv->v4l2_cap & V4L2_CAP_RADIO))
				return 0;
			return ivtv_reg_dev(itv, s, itv->num, VFL_TYPE_RADIO);
		case IVTV_DEC_STREAM_TYPE_MPG:
			if (ivtv_reg_dev(itv, s, itv->num + IVTV_V4L2_DEC_MPG_OFFSET,
				 VFL_TYPE_GRABBER)) {
				return -ENOMEM;
			}
			itv->streams[s].v4l2dev->fops = &ivtv_v4l2_dec_fops;
			break;
		case IVTV_DEC_STREAM_TYPE_VBI:
			return ivtv_reg_dev(itv, s, itv->num + IVTV_V4L2_DEC_VBI_OFFSET,
				 VFL_TYPE_VBI);
		case IVTV_DEC_STREAM_TYPE_VOUT:
			if (ivtv_reg_dev(itv, s, itv->num + IVTV_V4L2_DEC_VOUT_OFFSET,
				 VFL_TYPE_VBI)) {
				return -ENOMEM;
			}
			itv->streams[s].v4l2dev->fops = &ivtv_v4l2_dec_fops;
			break;
		case IVTV_DEC_STREAM_TYPE_YUV:
			if (ivtv_reg_dev(itv, s, itv->num + IVTV_V4L2_DEC_YUV_OFFSET,
				 VFL_TYPE_GRABBER)) {
				return -ENOMEM;
			}
			itv->streams[s].v4l2dev->fops = &ivtv_v4l2_dec_fops;
			break;
		case IVTV_DEC_STREAM_TYPE_OSD:
			return ivtv_reg_dev(itv, s, -1, 0);
	}
	return 0;
}

static int ivtv_stream_setup(struct ivtv *itv, int x)
{
	int ret = 0;
	if (x == IVTV_ENC_STREAM_TYPE_MPG) {
		if (ivtv_stream_init(itv, x, itv->options.mpg_buffers,
				     itv->dma_cfg.enc_buf_size,
				     PCI_DMA_FROMDEVICE)) {
			return -ENOMEM;
		}
	} else if (x == IVTV_ENC_STREAM_TYPE_YUV) {
		if (ivtv_stream_init(itv, x, itv->options.yuv_buffers,
				     itv->dma_cfg.enc_yuv_buf_size,
				     PCI_DMA_FROMDEVICE)) {
			return -ENOMEM;
		}
	} else if (x == IVTV_ENC_STREAM_TYPE_VBI) {
		if (ivtv_stream_init(itv, x, itv->options.vbi_buffers,
				     itv->dma_cfg.vbi_buf_size, 
					PCI_DMA_FROMDEVICE)) {
			return -ENOMEM;
		}
	} else if (x == IVTV_ENC_STREAM_TYPE_PCM) {
		if (ivtv_stream_init(itv, x, itv->options.pcm_buffers,
				     itv->dma_cfg.enc_pcm_buf_size,
				     PCI_DMA_FROMDEVICE)) {
			return -ENOMEM;
		}
	} else if ((itv->v4l2_cap & V4L2_CAP_RADIO) &&
		   (x == IVTV_ENC_STREAM_TYPE_RAD)) {
		if (ivtv_stream_init(itv, x, 0, 0, PCI_DMA_NONE)) {
			return -ENOMEM;
		}
	} else if (x == IVTV_DEC_STREAM_TYPE_MPG) {
		if (ivtv_stream_init(itv, x,
				     itv->options.dec_mpg_buffers,
				     itv->dma_cfg.dec_buf_size,
				     PCI_DMA_TODEVICE)) {
			return -ENOMEM;
		}
	} else if (x == IVTV_DEC_STREAM_TYPE_VBI) {
		if (ivtv_stream_init(itv, x, itv->options.vbi_buffers,
				     itv->dma_cfg.dec_vbi_buf_size,
				     PCI_DMA_FROMDEVICE)) {
			return -ENOMEM;
		}
	} else if (x == IVTV_DEC_STREAM_TYPE_VOUT) {
		if (ivtv_stream_init(itv, x, 0, 0, PCI_DMA_NONE)) {
			return -ENOMEM;
		}
	} else if (x == IVTV_DEC_STREAM_TYPE_YUV) {
		if (ivtv_stream_init(itv, x,
				     itv->options.dec_yuv_buffers,
				     itv->dma_cfg.dec_yuv_buf_size,
				     PCI_DMA_TODEVICE)) {
			return -ENOMEM;
		}
	} else if (x == IVTV_DEC_STREAM_TYPE_OSD) {
		if (ivtv_stream_init(itv, x,
				     itv->options.dec_osd_buffers,
				     IVTV_DMA_OSD_BUF_SIZE, PCI_DMA_TODEVICE)) {
			return -ENOMEM;
		}
	}
	return ret;
}

/* Initialize v4l2 variables and register v4l2 device */
int ivtv_streams_setup(struct ivtv *itv)
{
	int x;

	IVTV_DEBUG_INFO("v4l2 streams setup\n");

	itv->streamcount = 4;
	if (itv->v4l2_cap & V4L2_CAP_RADIO)
		itv->streamcount++;

	if (itv->v4l2_cap & V4L2_CAP_VIDEO_OUTPUT) {
		itv->streamcount += 5;
		ivtv_v4l2dev_template.type |= VID_TYPE_MPEG_DECODER;
	}

	/* fill in appropriate v4l2 device */
	IVTV_DEBUG_INFO("Configuring %s card with %d streams\n",
		       itv->card_name, itv->streamcount);

	/* Allocate streams */
	itv->streams = (struct ivtv_stream *)
	    kmalloc(itv->streamcount * sizeof(struct ivtv_stream), GFP_KERNEL);
	if (NULL == itv->streams) {
		IVTV_ERR("Couldn't allocate v4l2 streams\n");
		return -ENOMEM;
	}
	memset(itv->streams, 0, itv->streamcount * sizeof(struct ivtv_stream));

	/* Setup V4L2 Devices */
	for (x = 0; x < itv->streamcount; x++) {
		/* Register Device */
		if (ivtv_dev_setup(itv, x))
			break;

		/* Setup Stream */
		if (ivtv_stream_setup(itv, x))
			break;

		if (!itv->options.dynbuf) 
		{
			/* Allocate Stream */
			if (x != IVTV_DEC_STREAM_TYPE_OSD) {
				if (ivtv_stream_alloc(itv, x))
					break;
			}
		}
	}
	if (x == itv->streamcount) {
		atomic_set(&itv->streams_setup, 1);
		return 0;
	}

	/* One or more streams could not be initialized. Clean 'em all up. */
	ivtv_streams_cleanup(itv);
	return -ENOMEM;
}

static struct ivtv_stream *ivtv_stream_safeget(const char *desc, 
		struct ivtv *itv, int type)
{
	if (type >= itv->streamcount) {
		IVTV_DEBUG_WARN(
			"%s accessing uninitialied %s stream\n",
			desc, ivtv_stream_name(type));
		return NULL;
	}

	return &itv->streams[type];
}

static void ivtv_unreg_dev(struct ivtv *itv, int stream)
{
	struct ivtv_stream *s = ivtv_stream_safeget(
		"ivtv_unreg_dev", itv, stream);
	if (s == NULL)
		return;

	if (s->v4l2dev == NULL)
		return;

	/* Free Device */
	if (s->v4l2dev->minor == -1) {
		// 'Hidden' never registered stream (OSD)
		video_device_release(s->v4l2dev);
	} else {
		// All others, just unregister.
		video_unregister_device(s->v4l2dev);
	}
	return;
}

void ivtv_streams_cleanup(struct ivtv *itv)
{
	int x;

	/* Teardown all streams */
	for (x = 0; x < itv->streamcount; x++) {
		ivtv_stream_free(itv, x);
		ivtv_unreg_dev(itv, x);
	}

	kfree(itv->streams);
	itv->streams = NULL;

	atomic_set(&itv->streams_setup, 0);
}

void ivtv_setup_v4l2_encode_stream(struct ivtv *itv, int type)
{
	struct ivtv_stream *s = ivtv_stream_safeget(
		"ivtv_setup_v4l2_encode_stream", itv, type);
        int dnr_temporal;

	if (s == NULL)
		return;

	/* streams_lock must be held */
	IVTV_ASSERT(ivtv_sem_count(&s->mlock) <= 0);

	/* assign stream type */
	ivtv_vapi(itv, IVTV_API_ASSIGN_STREAM_TYPE, 1, itv->codec.stream_type);

	/* assign output port */
	ivtv_vapi(itv, IVTV_API_ASSIGN_OUTPUT_PORT, 1, 0);	/* 0 = Memory */

	/* assign framerate */
	ivtv_vapi(itv, IVTV_API_ASSIGN_FRAMERATE, 1,
		  (itv->std & V4L2_STD_625_50) ? 1 : 0);

	/* assign frame size */
	ivtv_vapi(itv, IVTV_API_ASSIGN_FRAME_SIZE, 2, itv->height, itv->width);

	/* assign aspect ratio */
	ivtv_vapi(itv, IVTV_API_ASSIGN_ASPECT_RATIO, 1, itv->codec.aspect);

	/* automatically calculate bitrate on the fly */
	if (itv->codec.bitrate > 15000000) {
		IVTV_DEBUG_WARN(
			   "Bitrate too high, adjusted 15mbit, see mpeg specs\n");
		itv->codec.bitrate = 15000000;
	}

	/* assign bitrates */
	ivtv_vapi(itv, IVTV_API_ASSIGN_BITRATES, 5, itv->codec.bitrate_mode,
		  itv->codec.bitrate,	/* bps */
		  itv->codec.bitrate_peak / 400,	/* peak/400 */
		  0x00, 0x00, 0x70);	/* encoding buffer, ckennedy */

	/* assign gop properties */
	ivtv_vapi(itv, IVTV_API_ASSIGN_GOP_PROPERTIES, 2,
		  itv->codec.framespergop, itv->codec.bframes);

	/* assign 3 2 pulldown */
	ivtv_vapi(itv, IVTV_API_ASSIGN_3_2_PULLDOWN, 1, itv->codec.pulldown);

	/* assign gop closure */
	ivtv_vapi(itv, IVTV_API_ASSIGN_GOP_CLOSURE, 1, itv->codec.gop_closure);

	/* assign audio properties */
	ivtv_vapi(itv, IVTV_API_ASSIGN_AUDIO_PROPERTIES, 1,
		  itv->codec.audio_bitmask);
	ivtv_dualwatch_start_encoding(itv);

	/* assign dnr filter mode */
	ivtv_vapi(itv, IVTV_API_ASSIGN_DNR_FILTER_MODE, 2,
		  itv->codec.dnr_mode, itv->codec.dnr_type);

	/* assign dnr filter props */
        dnr_temporal = itv->codec.dnr_temporal;
        if (itv->width != 720 || 
            (itv->is_50hz && itv->height != 576) ||
            (itv->is_60hz && itv->height != 480)) {
                // dnr_temporal != 0 for scaled images gives ghosting effect.
                // Always set to 0 for scaled images.
                dnr_temporal = 0;
        }
	ivtv_vapi(itv, IVTV_API_ASSIGN_DNR_FILTER_PROPS, 2,
		  itv->codec.dnr_spatial, dnr_temporal);

	/* assign coring levels (luma_h, luma_l, chroma_h, chroma_l) */
	ivtv_vapi(itv, IVTV_API_ASSIGN_CORING_LEVELS, 4, 0, 255, 0, 255);

	/* assign spatial filter type: luma_t: 1 = horiz_only, chroma_t: 1 = horiz_only */
	ivtv_vapi(itv, IVTV_API_ASSIGN_SPATIAL_FILTER_TYPE, 2, 1, 1);

	/* assign frame drop rate */
	ivtv_vapi(itv, IVTV_API_ASSIGN_FRAME_DROP_RATE, 1, 0);
}

int ivtv_passthrough_mode(struct ivtv *itv, int enable)
{
	int x;
	struct ivtv_stream *yuv_stream = ivtv_stream_safeget(
		"ivtv_passthrough_mode", itv, IVTV_ENC_STREAM_TYPE_YUV);
	struct ivtv_stream *dec_stream = ivtv_stream_safeget(
		"ivtv_passthrough_mode", itv, IVTV_DEC_STREAM_TYPE_YUV);

	if (yuv_stream == NULL || dec_stream == NULL)
		return -EINVAL;

	IVTV_DEBUG_INFO("ivtv ioctl: Select passthrough mode\n");

	/* Prevent others from starting/stopping streams while we
	   initiate/terminate passthrough mode */
	if (enable) {
                struct v4l2_format fmt;

		if (test_bit(IVTV_F_I_PASSTHROUGH, &itv->i_flags)) {
			return 0;
		}
		if (atomic_read(&itv->decoding) > 0) {
			return -EBUSY;
		}

		down(&dec_stream->mlock); 
		set_bit(IVTV_F_I_PASSTHROUGH, &itv->i_flags);

		/* Fully initialize stream, and then unflag init */
		set_bit(IVTV_F_S_NO_DMA, &dec_stream->s_flags);
		set_bit(IVTV_F_S_DECODING, &dec_stream->s_flags);

		/* Setup YUV Decoder */
		ivtv_setup_v4l2_decode_stream(itv, IVTV_DEC_STREAM_TYPE_YUV);

		/* Start Decoder */
		ivtv_vapi(itv, IVTV_API_DEC_START_PLAYBACK, 2, 0, 1);
		atomic_inc(&itv->decoding);
		ivtv_start_vbi_timer(itv);

		/* Setup capture if not already done */
		if (atomic_read(&itv->capturing) == 0) {
			down(&yuv_stream->mlock);
			ivtv_setup_v4l2_encode_stream(itv,
						      IVTV_ENC_STREAM_TYPE_YUV);
			up(&yuv_stream->mlock);
		}

                /* Setup the sliced VBI registers */
		fmt.type = V4L2_BUF_TYPE_SLICED_VBI_CAPTURE;
                vbi_setup_lcr(itv, itv->vbi_passthrough, !(itv->std & V4L2_STD_NTSC), &fmt.fmt.sliced);
                itv->card->video_dec_func(itv, VIDIOC_S_FMT, &fmt);

                /* Start Passthrough Mode */
		x = ivtv_vapi(itv, IVTV_API_BEGIN_CAPTURE, 2, 2, 11);

		if (x)
			IVTV_DEBUG_WARN(
				   "passthrough start capture error. Code %d\n",
				   x);
		up(&dec_stream->mlock);

		return 0;
	}

	if (!test_bit(IVTV_F_I_PASSTHROUGH, &itv->i_flags)) {
		return 0;
	}

	/* Stop Passthrough Mode */
	down(&dec_stream->mlock);
	x = ivtv_vapi(itv, IVTV_API_END_CAPTURE, 3, 1, 1, 11);
	x = ivtv_vapi(itv, IVTV_API_DEC_STOP_PLAYBACK, 3, 1, 0, 0);
	if (x)
		IVTV_DEBUG_WARN(
			   "passthrough stop decode error. Code %d\n", x);

	atomic_dec(&itv->decoding);
	clear_bit(IVTV_F_S_NO_DMA, &dec_stream->s_flags);
	clear_bit(IVTV_F_S_DECODING, &dec_stream->s_flags);

	clear_bit(IVTV_F_I_PASSTHROUGH, &itv->i_flags);
	up(&dec_stream->mlock);

	return 0;
}

void ivtv_vbi_setup(struct ivtv *itv, int mode)
{
	int raw = itv->vbi_sliced_in->service_set == 0;
	u32 data[IVTV_MBOX_MAX_DATA], result;
	int lines;
	int x;
	int i;
	int h = (itv->std & V4L2_STD_NTSC) ? 480 : 576;

	/* If Embed then streamtype must be Program */
	if (!(mode & 0x04) && (mode & 0x02) && 
		!raw && itv->vbi_insert_mpeg) 
	{
		itv->codec.stream_type = 0;

        	/* assign stream type */
        	ivtv_vapi(itv, IVTV_API_ASSIGN_STREAM_TYPE, 1, 
			itv->codec.stream_type);
	}

	/* Reset VBI */
	ivtv_vapi(itv, IVTV_API_SELECT_VBI_LINE, 5, 0xffff , 0, 0, 0, 0);

	/* Don't use VBI if scaling */
	if (raw && itv->height != h) {
		itv->card->video_dec_func(itv, VIDIOC_S_FMT, &itv->vbi_in);
		return;
	}

        if (itv->std & V4L2_STD_NTSC) {
                itv->vbi_count = 12;
                itv->vbi_start[0] = 10;
                itv->vbi_start[1] = 273;
        } else {        /* PAL/SECAM */
                itv->vbi_count = 18;
                itv->vbi_start[0] = 6;
                itv->vbi_start[1] = 318;
        }

	// setup VBI registers
	itv->card->video_dec_func(itv, VIDIOC_S_FMT, &itv->vbi_in);

	itv->vbi_search_ba = 0;

	// determine number of lines and total number of VBI bytes.
	// A raw line takes 1443 bytes: 2 * 720 + 4 byte frame header - 1
	// The '- 1' byte is probably an unused U or V byte. Or something...
	// A sliced line takes 51 bytes: 4 byte frame header, 4 byte internal
	// header, 42 data bytes + checksum (to be confirmed) 
	if (raw) {
		lines = itv->vbi_count * 2;
	} else {
                if (itv->std & V4L2_STD_NTSC) {
                        lines = 24;
                } else {
		        lines = 38;
                }
	}

        ivtv_vapi(itv, IVTV_API_ASSIGN_NUM_VSYNC_LINES,
        	2, itv->digitizer, itv->digitizer);

	itv->vbi_enc_size =
	    (lines * (raw ? itv->vbi_raw_size : itv->vbi_sliced_size));

	// Note: sliced vs raw flag doesn't seem to have any effect
	data[0] = raw | mode | (0xbd << 8);

	// Every X number of frames a VBI interrupt arrives (frames as in 25 or 30 fps)
	data[1] = 1; 	
	// The VBI frames are stored in a ringbuffer with this size (with a VBI frame as unit)
	if (raw) {
		data[2] = 4;
	} else {
		data[2] = 2880 / itv->vbi_sliced_size;
	}
	// The start/stop codes determine which VBI lines end up in the raw VBI data area.
	// The codes are from table 24 in the saa7115 datasheet. Each raw/sliced/video line
	// is framed with codes FF0000XX where XX is the SAV/EAV (Start/End of Active Video)
	// code. These values for raw VBI are obtained from a driver disassembly. The sliced
	// start/stop codes was deduced from this, but they do not appear in the driver.
	// Other code pairs that I found are: 0x250E6249/0x13545454 and 0x25256262/0x38137F54.
	// However, I have no idea what these values are for.
        if (itv->card->type == IVTV_CARD_PVR_150 ||
               itv->card->type == IVTV_CARD_PG600)
	{
        	/* Setup VBI for the cx25840 digitizer */
        	if (raw) {
			data[3] = 0x20602060;
			data[4] = 0x30703070;
        	} else {
                        data[3] = 0xB0F0B0F0;
                        data[4] = 0xA0E0A0E0;
        	}
		// Lines per frame
		data[5] = lines;
		// bytes per line
		data[6] = (raw ? itv->vbi_raw_size : itv->vbi_sliced_size);
	} else {
        	/* Setup VBI for the saa7115 digitizer */
		if (raw) {
			data[3] = 0x25256262;
			data[4] = 0x387F7F7F;
		} else {
			data[3] = 0xABABECEC;
			data[4] = 0xB6F1F1F1;
		}
		// Lines per frame
		data[5] = lines;
		// bytes per line
		data[6] = itv->vbi_enc_size / lines;
	}

	IVTV_DEBUG_INFO(
		"Setup VBI API header 0x%08x pkts %d buffs %d ln %d sz %d\n", 
			data[0], data[1], data[2], data[5], data[6]);

	x = ivtv_api(itv, itv->enc_mbox, &itv->enc_msem, IVTV_API_CONFIG_VBI,
		     &result, 7, &data[0]);
	if (x)
		IVTV_DEBUG_WARN("init error 21. Code %d\n", x);

	// returns the VBI encoder memory area.
	itv->vbi_enc_start = data[2];
	itv->vbi_total_frames = data[1];
	itv->vbi_fpi = data[0];
	if (!itv->vbi_fpi)
		itv->vbi_fpi = 1;
	itv->vbi_index = 0;

	IVTV_DEBUG_INFO("Setup VBI start 0x%08x frames %d fpi %d lines 0x%08x\n",
		itv->vbi_enc_start, itv->vbi_total_frames, itv->vbi_fpi, itv->digitizer); 

	// select VBI lines.
	// Note that the sliced argument seems to have no effect.
	for (i = 2; i <= 24; i++) {
		int valid;

                if (itv->std & V4L2_STD_NTSC) {
		        valid = i >= 10 && i < 22;
                } else {
		        valid = i >= 6 && i < 24;
                }
		ivtv_vapi(itv, IVTV_API_SELECT_VBI_LINE, 5, i - 1,
                                valid, 0 , 0, 0);
		ivtv_vapi(itv, IVTV_API_SELECT_VBI_LINE, 5, (i - 1) | 0x80000000,
                                valid, 0, 0, 0);
	}

	// Remaining VBI questions:
	// - Is it possible to select particular VBI lines only for inclusion in the MPEG
	// stream? Currently you can only get the first X lines.
	// - Is mixed raw and sliced VBI possible? 
	// - What's the meaning of the raw/sliced flag?
	// - What's the meaning of params 2, 3 & 4 of the Select VBI command?
}

static void unknown_setup_api(struct ivtv *itv)
{
	ivtv_vapi(itv, IVTV_API_ENC_UNKNOWN, 1, 0);
	ivtv_vapi(itv, IVTV_API_ENC_MISC, 2, 7, 0);
	ivtv_vapi(itv, IVTV_API_ENC_MISC, 2, 8, 0);
	ivtv_vapi(itv, IVTV_API_ENC_MISC, 2, 4, 1);
	ivtv_vapi(itv, IVTV_API_ENC_MISC, 2, 12, 0);
}

int ivtv_init_digitizer(struct ivtv *itv)
{
	int dummy;

        /* Disable digitizer */
        IVTV_DEBUG_INFO("Disabling digitizer\n");
	itv->card->video_dec_func(itv, VIDIOC_STREAMOFF, &dummy);

        /* initialize or refresh input */
        if (atomic_read(&itv->capturing) == 0)
                ivtv_vapi(itv, IVTV_API_INITIALIZE_INPUT, 0);
        else
                ivtv_vapi(itv, IVTV_API_REFRESH_INPUT, 0);

        /* enable digitizer (saa7115) */
        IVTV_DEBUG_INFO("Enabling digitizer\n");
	itv->card->video_dec_func(itv, VIDIOC_STREAMON, &dummy);

	return 0;
}

int ivtv_start_v4l2_encode_stream(struct ivtv *itv, int type)
{
	u32 data[IVTV_MBOX_MAX_DATA], result;
	int x = 0;
	int captype = 0, subtype = 0;
	struct ivtv_stream *st = ivtv_stream_safeget(
		"ivtv_start_v4l2_encode_stream", itv, type);
	int enable_passthrough = 0;

	if (st == NULL)
		return -EINVAL;

	/* Lock */
	down(&st->mlock);

	IVTV_DEBUG_INFO("ivtv start v4l2 stream\n");

	captype = type;

	switch (type) {
	case IVTV_ENC_STREAM_TYPE_MPG:
		captype = 0;
		subtype = 3;

		/* Stop Passthrough */
		if (test_bit(IVTV_F_I_PASSTHROUGH, &itv->i_flags)) {
			ivtv_passthrough_mode(itv, 0);
			enable_passthrough = 1;
		}
		set_bit(IVTV_F_T_ENC_VID_STARTED, &itv->t_flags);
		if (!test_bit(IVTV_F_T_ENC_RAWVID_STARTED, &itv->t_flags) &&
			!test_bit(IVTV_F_T_ENC_RAWAUD_STARTED, &itv->t_flags))
		{
			atomic_set(&itv->enc_dma_stat_intr, 0);
		}
		break;

	case IVTV_ENC_STREAM_TYPE_YUV:
		captype = 1;
		if (test_bit(IVTV_F_I_PASSTHROUGH, &itv->i_flags)) {
			captype = 2;
			subtype = 11;	/* video+audio+decoder */
			break;
		}
		subtype = 1;
		set_bit(IVTV_F_T_ENC_RAWVID_STARTED, &itv->t_flags);
                if (!test_bit(IVTV_F_T_ENC_VID_STARTED, &itv->t_flags) &&
                        !test_bit(IVTV_F_T_ENC_RAWAUD_STARTED, &itv->t_flags))
                {
                        atomic_set(&itv->enc_dma_stat_intr, 0);
                }

		break;
	case IVTV_ENC_STREAM_TYPE_PCM:
		captype = 1;
		subtype = 2;
		set_bit(IVTV_F_T_ENC_RAWAUD_STARTED, &itv->t_flags);
                if (!test_bit(IVTV_F_T_ENC_VID_STARTED, &itv->t_flags) &&
                        !test_bit(IVTV_F_T_ENC_RAWVID_STARTED, &itv->t_flags))
                {
                        atomic_set(&itv->enc_dma_stat_intr, 0);
                }
		break;
	case IVTV_ENC_STREAM_TYPE_VBI:
		captype = 1;
		subtype = 4;

                itv->vbi_frame = 0;
                itv->vbi_inserted_frame = 0;
                memset(itv->vbi_sliced_mpeg_offset,
                        0, sizeof(itv->vbi_sliced_mpeg_offset));
                memset(itv->vbi_sliced_mpeg_size,
                        0, sizeof(itv->vbi_sliced_mpeg_size));

		set_bit(IVTV_F_T_ENC_VBI_STARTED, &itv->t_flags);
		atomic_set(&itv->enc_vbi_dma_stat_intr, 0);
		break;
	default:
		break;
	}
	st->subtype = subtype;
	st->first_read = 1;
	st->trans_id = 0;
	st->stolen_bufs = 0;

	/* mute/unmute video */
	ivtv_vapi(itv, IVTV_API_MUTE_VIDEO, 1,
		  test_bit(IVTV_F_I_RADIO_USER, &itv->i_flags) ? 1 : 0);

	/* Clear Streamoff flags in case left from last capture */
	clear_bit(IVTV_F_S_STREAMOFF, &st->s_flags);
	clear_bit(IVTV_F_S_OVERFLOW, &st->s_flags);

        if (atomic_read(&itv->capturing) == 0) {
		/*assign dma block len */
		data[0] = itv->dma_cfg.fw_enc_dma_xfer;	
				/* 2^9(512), 2^8(256), 2^7(128) 
				   bytes ((2^9)1024) or num frames */
		/* 131072, 262144, 524288. 0 defaults to 131072 */
		data[1] = itv->dma_cfg.fw_enc_dma_type;	/* 0=bytes, 1=frames */
		ivtv_api(itv, itv->enc_mbox, &itv->enc_msem,
	     		IVTV_API_ASSIGN_DMA_BLOCKLEN, &result, 2, &data[0]);

		/* Stuff from Windows, we don't know what it is */
		unknown_setup_api(itv);

       		/* assign placeholder */
       		ivtv_vapi(itv, IVTV_API_ASSIGN_PLACEHOLDER, 12, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

		/* Setup VBI */
        	if ((itv->v4l2_cap & V4L2_CAP_VBI_CAPTURE)) {
                	int pkt_type = 0x02;

                	/* Raw mode, no private packet */
                	if (itv->vbi_sliced_in->service_set == 0)
                        	pkt_type = 0x00;

                	if (test_bit(IVTV_F_T_ENC_VBI_STARTED, &itv->t_flags) &&
                                !itv->vbi_insert_mpeg) {
                        	ivtv_vbi_setup(itv, 0x04|pkt_type);
                        	itv->dec_vbi_pkt = 1;
                	} else if (itv->vbi_insert_mpeg) {
                        	ivtv_vbi_setup(itv, (0x04|0x02));
                        	itv->dec_vbi_pkt = 1;
                	} else {
                        	ivtv_vbi_setup(itv, (0x08|0x04));
                        	itv->dec_vbi_pkt = 0;
                	}
        	} else {
			ivtv_vapi(itv, IVTV_API_ASSIGN_NUM_VSYNC_LINES,
	  			2, itv->digitizer, itv->digitizer);
		}

		/*assign program index info. Mask 0: Disable, Num_req: 400 max*/
		data[0] = itv->idx_sdf_mask;
		data[1] = itv->idx_sdf_num;
		ivtv_api(itv, itv->enc_mbox, &itv->enc_msem,
	 		IVTV_API_ASSIGN_PGM_INDEX_INFO, &result, 2, &data[0]);
		itv->idx_sdf_offset = data[0];
		itv->idx_sdf_num = data[1];

		IVTV_DEBUG_INFO(
	   		"ENC: PGM Index at 0x%08x with 0x%08x elements\n",
	   		itv->idx_sdf_offset, itv->idx_sdf_num);

		/* Setup API for Stream */
		ivtv_setup_v4l2_encode_stream(itv, type);
	}

	if (atomic_read(&itv->capturing) == 0)
		ivtv_vapi(itv, IVTV_API_ENC_MISC, 2, 3, itv->has_cx25840);

	/* Vsync Setup */
	if (itv->has_cx23415 && !test_and_set_bit(IVTV_F_I_DIG_RST, &itv->i_flags)) {
		/* event notification (on) */
		ivtv_vapi(itv, IVTV_API_EVENT_NOTIFICATION,
	  		4, 0, 1, 0x10000000, -1);
		ivtv_clear_irq_mask(itv, IVTV_IRQ_ENC_VIM_RST);
	}

	/* Check if capturing already and trying VBI capture */
	if (type == IVTV_ENC_STREAM_TYPE_VBI && atomic_read(&itv->capturing)) {
		IVTV_DEBUG_WARN(
			"Starting VBI after starting an encoding, seems to not work.\n"); 
	}

        if (atomic_read(&itv->capturing) == 0) {

        	/* Clear all Pending Interrupts */
        	ivtv_set_irq_mask(itv, IVTV_IRQ_MASK_CAPTURE);

        	clear_bit(IVTV_F_I_EOS, &itv->i_flags);

		/* Initialize Digitizer for Capture */
		ivtv_init_digitizer(itv);	

        	/*IVTV_DEBUG_INFO("Sleeping for 100ms\n"); */
        	ivtv_sleep_timeout(HZ / 10, 0);
        }

	/* begin_capture */
	if (ivtv_vapi(itv, IVTV_API_BEGIN_CAPTURE, 2, captype, subtype)) 
	{
		IVTV_DEBUG_WARN( "Error starting capture!\n");
		return -EINVAL;
	}

	/* Start Passthrough */
	if (enable_passthrough) {
		ivtv_passthrough_mode(itv, 1);
	}

	if (type == IVTV_ENC_STREAM_TYPE_VBI)
		ivtv_clear_irq_mask(itv, IVTV_IRQ_ENC_VBI_CAP);
	else
		ivtv_clear_irq_mask(itv, IVTV_IRQ_MASK_CAPTURE);

	/* you're live! sit back and await interrupts :) */
	atomic_inc(&itv->capturing);

	/* Unlock */
	up(&st->mlock);

	return x;
}

int ivtv_setup_v4l2_decode_stream(struct ivtv *itv, int type)
{
	u32 data[IVTV_MBOX_MAX_DATA], result;
	int x;
	struct ivtv_stream *s = ivtv_stream_safeget(
		"ivtv_setup_v4l2_decode_stream", itv, type);
	if (s == NULL)
		return -EINVAL;

	IVTV_DEBUG_INFO("Setting some initial decoder settings\n");

	/* streams_lock must be held */
	IVTV_ASSERT(ivtv_sem_count(&s->mlock) <= 0);

	/* disable VBI signals, if the MPEG stream contains VBI data,
	   then that data will be processed automatically for you. */
	ivtv_disable_vbi(itv);

	/* set audio mode to stereo */
	ivtv_vapi(itv, IVTV_API_DEC_SELECT_AUDIO, 2, 0, 0);

	/* set number of internal decoder buffers */
	ivtv_vapi(itv,
		  IVTV_API_DEC_DISPLAY_BUFFERS, 1, itv->dec_options.decbuffers);

	/* prebuffering */
	data[0] = itv->dec_options.prebuffer;	/* 0 = no prebuffering,
						   1 = enabled, see docs */
	ivtv_vapi(itv, IVTV_API_DEC_BUFFER, 1, itv->dec_options.prebuffer);

	/* extract from user packets */
	data[0] = itv->dec_vbi_pkt;
        ivtv_api(itv, itv->dec_mbox, &itv->dec_msem,
                     IVTV_API_DEC_EXTRACT_VBI, &result, 2, &data[0]);
	itv->vbi_dec_start = data[0];
	itv->vbi_dec_size = data[1];

	IVTV_DEBUG_INFO(
		"Decoder VBI RE-Insert start 0x%08x size 0x%08x type %d\n",
		itv->vbi_dec_start, itv->vbi_dec_size = data[1], 
		itv->dec_vbi_pkt);

        /* event notification (on)*/
        ivtv_vapi(itv, IVTV_API_DEC_EVENT_NOTIFICATION, 4,
                5, 1, 0x04000000, -1);

	/* set decoder source settings */
	/* Data type: 0 = mpeg from host,
	   1 = yuv from encoder,
	   2 = yuv_from_host */
	switch (type) {
	case IVTV_DEC_STREAM_TYPE_YUV:
		if (test_bit(IVTV_F_I_PASSTHROUGH, &itv->i_flags)) {
			data[0] = 1;
			break;
		}
		data[0] = 2;
		IVTV_DEBUG_INFO(
			   "Setup DEC YUV Stream data[0] = %d\n", data[0]);
		break;
	case IVTV_DEC_STREAM_TYPE_MPG:
	default:
		data[0] = 0;
		break;
	}
	data[1] = itv->width;	/* YUV source width */
	data[2] = itv->height;
	data[3] = itv->codec.audio_bitmask & 0xffff;	/* Audio settings to use,
							   bitmap. see docs. */
	x = ivtv_api(itv, itv->dec_mbox,
		     &itv->dec_msem,
		     IVTV_API_DEC_DECODE_SOURCE, &result, 4, &data[0]);
	if (x)
		IVTV_DEBUG_WARN(
			   "COULDN'T INITIALIZE DECODER SOURCE %d\n", x);
	return 0;
}

int ivtv_start_v4l2_decode_stream(struct ivtv *itv, int type)
{
	int x;
	int yuv_num = 0;
	struct ivtv_stream *stream = ivtv_stream_safeget(
		"ivtv_start_v4l2_decode_stream", itv, type);

	if (stream == NULL)
		return -EINVAL;

	/* Lock Stream */
	down(&stream->mlock);

	IVTV_DEBUG_INFO("Starting v4l2_decode \n");

	/* Clear Streamoff */
	if (type == IVTV_DEC_STREAM_TYPE_YUV) {
			/* Always start at first YUV Buffer */
			atomic_set(&itv->yuv_num, 0);
			yuv_num = atomic_read(&itv->yuv_num);

			/* Initialize Decoder */
                        /* Reprogram Decoder YUV Buffers for YUV */
                        ivtv_write_reg((yuv_offset[yuv_num]>>4),
                                itv->reg_mem + 0x82c);
                        ivtv_write_reg(((yuv_offset[yuv_num] +
                                IVTV_YUV_BUFFER_UV_OFFSET)>>4),
                                itv->reg_mem + 0x830);
                        ivtv_write_reg((yuv_offset[yuv_num]>>4),
                                itv->reg_mem + 0x834);
                        ivtv_write_reg(((yuv_offset[yuv_num] +
                                IVTV_YUV_BUFFER_UV_OFFSET)>>4),
                                itv->reg_mem + 0x838);

			ivtv_write_reg(0x00000000|(0x0c<<16)|(0x0b<<8), 
				itv->reg_mem + 0x2d24);

                        ivtv_write_reg(0x00108080, itv->reg_mem + 0x2898);
                        ivtv_write_reg(0x01, itv->reg_mem + 0x2800);
	}

	ivtv_setup_v4l2_decode_stream(itv, type);

	/* set dma size */
	if (itv->dma_cfg.fw_dec_dma_xfer > 0) {
		x = ivtv_vapi(itv,
			      IVTV_API_DEC_DMA_BLOCKSIZE, 1,
			      itv->dma_cfg.fw_dec_dma_xfer);
		if (x)
			IVTV_DEBUG_WARN(
				   "COULDN'T INITIALIZE DMA SIZE %d\n", x);
	}

	clear_bit(IVTV_F_S_STREAMOFF, &stream->s_flags);
	clear_bit(IVTV_F_S_OVERFLOW, &stream->s_flags);

	/* Decoder DMA State */
	itv->dec_dma_stat.last_addr = 0;
	itv->dec_dma_stat.bytes_filled = 0;
	itv->dec_dma_stat.bytes_needed = 0;
	itv->dec_dma_stat.total_xfer = 0;
	itv->dec_dma_stat.sg_bytes = 0;
	itv->dec_dma_stat.ts = jiffies;

	itv->dec_dma_stat.data[0] = 0x0;
	itv->dec_dma_stat.data[1] = 0x0;
	itv->dec_dma_stat.data[2] = 0x80000000;
	itv->dec_dma_stat.data[3] = 0x0;
	itv->dec_dma_stat.wq_runs = 0;
	atomic_set(&itv->dec_dma_stat.intr, 0);

	/* Zero out decoder counters */
	writel(0, &itv->dec_mbox[IVTV_MBOX_FIELD_DISPLAYED].data[0]);
	writel(0, &itv->dec_mbox[IVTV_MBOX_FIELD_DISPLAYED].data[1]);
	writel(0, &itv->dec_mbox[IVTV_MBOX_FIELD_DISPLAYED].data[2]);
	writel(0, &itv->dec_mbox[IVTV_MBOX_FIELD_DISPLAYED].data[3]);
	writel(0, &itv->dec_mbox[9].data[0]);
	writel(0, &itv->dec_mbox[9].data[1]);
	writel(0, &itv->dec_mbox[9].data[2]);
	writel(0, &itv->dec_mbox[9].data[3]);

	/* Flag we are starting the decoder */
        if (test_and_set_bit(IVTV_F_T_DEC_DONE, &itv->t_flags)) {
		IVTV_DEBUG_INFO(
			"Decoder already started!!!!\n");
		up(&stream->mlock);
		return -EBUSY;
	}
        set_bit(IVTV_F_T_DEC_STARTED, &itv->t_flags);
	/* start playback */
	if (ivtv_vapi(itv, IVTV_API_DEC_START_PLAYBACK, 2,
			     itv->dec_options.gop_offset,
			     itv->dec_options.mute_frames))
	{
		clear_bit(IVTV_F_T_DEC_DONE, &itv->t_flags);
		up(&stream->mlock);
		return -EBUSY;
	}

	/*Clear the following Interrupt mask bits for decoding */
	ivtv_clear_irq_mask(itv, IVTV_IRQ_MASK_DECODE);
	IVTV_DEBUG_IRQ("IRQ Mask is now: 0x%08x\n", itv->irqmask);

	/* you're live! sit back and await interrupts :) */
	atomic_inc(&itv->decoding);

	/* Unlock */
	up(&stream->mlock);

	/* Start Work Queue */
	atomic_inc(&itv->dec_dma_stat.intr);
	dec_schedule_work(itv);

	return 0;
}

int ivtv_stop_all_captures(struct ivtv *itv)
{
	int x;

	for (x = itv->streamcount-1; x >= 0; x--) {
		if (test_bit(IVTV_F_S_CAPTURING, &itv->streams[x].s_flags)) {
			ivtv_stop_capture(itv, x);
		}
	}

	return 0;
}

int ivtv_stop_capture(struct ivtv *itv, int type)
{
	DECLARE_WAITQUEUE(wait, current);
	struct ivtv_stream *st = ivtv_stream_safeget(
		"ivtv_stop_capture", itv, type);
	int cap_type;
	unsigned long then;
	int x;
	int stopmode;
	u32 data[IVTV_MBOX_MAX_DATA], result;
	int thread_to_stop = 0;

	if (st == NULL)
		return -EINVAL;


	/* This function assumes that you are allowed to stop the capture
	   and that we are actually capturing */

	IVTV_DEBUG_INFO("Stop Capture\n");

	if (type == IVTV_DEC_STREAM_TYPE_VBI) {
        	/* Wait till thread done */
        	for (then=0;
                	test_bit(IVTV_F_T_DEC_VBI_RUNNING, 
				&itv->t_flags) && then < 100;
                        	then++)
        	{
            		IVTV_DEBUG_INFO(
                        	"ENCODE thread is active count %lu\n", then);
            		/* Buffers still full */
            		wake_up(&st->waitq);
            		ivtv_sleep_timeout(HZ/100, 1);
        	}

		return 0;
	}
	if (type == IVTV_DEC_STREAM_TYPE_VOUT)
		return 0;
	if (atomic_read(&itv->capturing) == 0)
		return 0;

	/* Lock */
	down(&st->mlock);

	switch (type) {
	case IVTV_ENC_STREAM_TYPE_YUV:
		cap_type = 1;
		break;
	case IVTV_ENC_STREAM_TYPE_PCM:
		cap_type = 1;
		break;
	case IVTV_ENC_STREAM_TYPE_VBI:
		cap_type = 1;
		break;
	case IVTV_ENC_STREAM_TYPE_MPG:
	default:
		cap_type = 0;
		break;
	}

	/* clear Overflow */
	clear_bit(IVTV_F_S_OVERFLOW, &st->s_flags);

	ivtv_dualwatch_stop_encoding(itv);

	/* Stop Capture Mode */
	if (type == IVTV_ENC_STREAM_TYPE_MPG && itv->end_gop == 1) {
		stopmode = 0;
	} else {
		stopmode = 1;
	}

	/* end_capture */
	x = ivtv_vapi(itv, IVTV_API_END_CAPTURE, 3, stopmode,	
		/* when: 0 =  end of GOP  1 = NOW! */
		      cap_type,	/* type: 0 = mpeg */
		      st->subtype);	/* subtype: 3 = video+audio */
	if (x)
		IVTV_DEBUG_WARN("ENC: Failed stopping capture %d\n",
			   x);

	/* only run these if we're shutting down the last cap */
	if (atomic_read(&itv->capturing) - 1 == 0) {
		/* event notification (off) */
		if (test_and_clear_bit(IVTV_F_I_DIG_RST, &itv->i_flags)) {
			/* type: 0 = refresh */
			x = ivtv_vapi(itv, IVTV_API_EVENT_NOTIFICATION, 4, 0, 0,	/* on/off: 0 = off */
				      0x10000000,	/* intr: 0x10000000 */
				      -1);	/* mbox_id: -1: none */
			if (x)
				IVTV_DEBUG_WARN(
					   "ENC: Failed to stop VIM"
					   "event notification %d\n", x);
			ivtv_set_irq_mask(itv, IVTV_IRQ_ENC_VIM_RST);
		}
                ivtv_vapi(itv, IVTV_API_ENC_MISC, 2, 3, 0);

	}

	then = jiffies;

	/* UnLock */
	up(&st->mlock);

	if (!test_bit(IVTV_F_S_NO_DMA, &st->s_flags)) {
		if ((type == IVTV_ENC_STREAM_TYPE_MPG) && itv->end_gop == 1) {
			/* only run these if we're shutting down the last cap */
			unsigned long duration;

			then = jiffies;
			add_wait_queue(&itv->cap_w, &wait);

			set_current_state(TASK_INTERRUPTIBLE);

			/* wait 2s for EOS interrupt */
			while ((!test_bit(IVTV_F_I_EOS,
					  &itv->i_flags)) &&
			       (jiffies < (then + (1 * HZ)))) {
				schedule_timeout(HZ / 100);
			}

			/* To convert jiffies to ms, we must multiply by 1000
			 * and divide by HZ.  To avoid runtime division, we
			 * convert this to multiplication by 1000/HZ.
			 * Since integer division truncates, we get the best
			 * accuracy if we do a rounding calculation of the constant.
			 * Think of the case where HZ is 1024.
			 */
			duration = ((1000 + HZ / 2) / HZ) * (jiffies - then);

			if (!test_bit(IVTV_F_I_EOS, &itv->i_flags)) {
				IVTV_DEBUG_WARN(
					   "ENC: EOS interrupt not "
					   "received! stopping anyway.\n");
				IVTV_DEBUG_WARN(
					   "ENC: waited %lu ms.\n", duration);
			} else {
				IVTV_DEBUG_WARN(
					   "ENC: EOS took %lu "
					   "ms to occur.\n", duration);
			}
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&itv->cap_w, &wait);

			if (test_bit(IVTV_F_S_DMAP, &st->s_flags)) {
				IVTV_DEBUG_INFO(
					   "ENC: EOS DMA still Pending!!!\n");
			}
		}

		then = jiffies;
		/* Make sure DMA is complete */
		add_wait_queue(&st->waitq, &wait);
		set_current_state(TASK_INTERRUPTIBLE);
		do {
			/* check if DMA is pending */
			if ((type == IVTV_ENC_STREAM_TYPE_MPG) &&	/* MPG Only */
			    (readl
			     ((unsigned char *)itv->reg_mem +
			      IVTV_REG_DMASTATUS) & 0x02)
			    && !test_bit(IVTV_F_T_ENC_DMA_NEEDED, &itv->t_flags)
			    && !test_bit(IVTV_F_S_DMAP, &st->s_flags)) {
				/* Check for last DMA */
				data[0] = data[1] = 0;
				ivtv_api(itv, itv->enc_mbox,
					 &itv->enc_msem,
					 0xC6, &result, 2, &data[0]);

				if (data[0] == 1) {
					IVTV_DEBUG_DMA(
						   "ENC: Last DMA of size 0x%08x\n",
						   data[1]);
					break;
				}
			} else if (!test_bit(IVTV_F_S_DMAP, &st->s_flags) &&	/* VBI and other streams */
				   (readl
				    ((unsigned char *)itv->reg_mem +
				     IVTV_REG_DMASTATUS) & 0x02)) {
				break;
			}

			ivtv_sleep_timeout(HZ / 100, 1);
		} while (((then + (HZ * 2)) > jiffies));

		set_current_state(TASK_RUNNING);
		remove_wait_queue(&st->waitq, &wait);

		if (test_bit(IVTV_F_S_DMAP, &st->s_flags)) {
			IVTV_DEBUG_WARN(
				   "ENC: DMA still Pending while stopping capture!\n");
		}
	}

	atomic_dec(&itv->capturing);

	/* Clear capture and no-read bits */
	if (!test_bit(IVTV_F_S_STREAMOFF, &st->s_flags))
		clear_bit(IVTV_F_S_CAPTURING, &st->s_flags);

	/* Clear capture started bit */
        if (type == IVTV_ENC_STREAM_TYPE_VBI) {
                clear_bit(IVTV_F_T_ENC_VBI_STARTED, &itv->t_flags);
		thread_to_stop = IVTV_F_T_ENC_VBI_RUNNING;
        } else if ((atomic_read(&itv->capturing) == 0) || 
		(atomic_read(&itv->capturing) == 1 && 
			test_bit(IVTV_F_T_ENC_VBI_STARTED, &itv->t_flags))) 
	{
                clear_bit(IVTV_F_T_ENC_VID_STARTED, &itv->t_flags);
		thread_to_stop = IVTV_F_T_ENC_RUNNING;
	} else if (test_bit(IVTV_F_T_ENC_RAWVID_STARTED, &itv->t_flags)) {
		clear_bit(IVTV_F_T_ENC_RAWVID_STARTED, &itv->t_flags);
		thread_to_stop = IVTV_F_T_ENC_RUNNING;
	} else if (test_bit(IVTV_F_T_ENC_RAWAUD_STARTED, &itv->t_flags)) {
		clear_bit(IVTV_F_T_ENC_RAWAUD_STARTED, &itv->t_flags);
		thread_to_stop = IVTV_F_T_ENC_RUNNING;
	} else
		thread_to_stop = IVTV_F_T_ENC_RUNNING;
	

        /* Wait till thread done */
        for (then=0;
                test_bit(thread_to_stop, &itv->t_flags) && then < 100;
                        then++)
        {
            IVTV_DEBUG_INFO(
                        "ENCODE thread is active count %lu\n", then);
            /* Buffers still full */
            wake_up(&st->waitq);
            ivtv_sleep_timeout(HZ/100, 1);
        }

        if (type == IVTV_ENC_STREAM_TYPE_VBI)
                ivtv_set_irq_mask(itv, IVTV_IRQ_ENC_VBI_CAP);

	if (atomic_read(&itv->capturing) > 0) {
		return 0;
	}

	/*Set the following Interrupt mask bits for capture */
	ivtv_set_irq_mask(itv, IVTV_IRQ_MASK_CAPTURE);
	IVTV_DEBUG_IRQ(
		   "ENC: IRQ Mask is now: 0x%08x\n", itv->irqmask);

	wake_up(&st->waitq);

	return 0;
}

int ivtv_stop_v4l2_decode_stream(struct ivtv *itv, int type)
{
	struct ivtv_stream *st = ivtv_stream_safeget(
		"ivtv_stop_v4l2_decode_stream", itv, type);
	int count, maxcount = 30;
	int no_stop = itv->dec_options.no_stop;
	int hide_last_frame = itv->dec_options.hide_last_frame;

	if (st == NULL)
		return -EINVAL;

	if (type == IVTV_DEC_STREAM_TYPE_YUV) {
		no_stop = 1;
		hide_last_frame = 0;
	} else if (hide_last_frame == 0) {
		no_stop = 1;
	}

	IVTV_DEBUG_INFO("Stop Decode.\n");
	if (type == IVTV_DEC_STREAM_TYPE_YUV)
		maxcount = 30;

	if (itv->dec_options.fast_stop)
		maxcount = 0;

        /* event notification (off)*/
	ivtv_vapi(itv, IVTV_API_DEC_EVENT_NOTIFICATION, 4, 
		5, 0, 0x04000000, -1);

        /* Clear all Buffers */
        clear_bit(IVTV_F_T_DEC_STARTED, &itv->t_flags);
        for (count=0;count < maxcount;count++) {
		if (ivtv_buf_fill_lock(st, 0, BUF_USED) <= 0) {
			break;
		} else {
			/* Buffers still full */
			ivtv_sleep_timeout(HZ/10, 0);
		}

		if (!(readl(itv->reg_mem + IVTV_REG_DMASTATUS) & 0x14)&&
               		(!test_bit(IVTV_F_S_DMAP, &st->s_flags)||
                        wait_event_interruptible(itv->r_intr_wq,
                                        atomic_read(&itv->r_intr)))) 
		{
			IVTV_DEBUG_INFO(
				"DEC STOP WAIT: count=%d buffer has 0x%08x bytes\n",
				 count, ivtv_buf_fill_lock(st, 0, BUF_USED)); 
		}
        }
	/* Stop Decoder */
       	if (ivtv_vapi(itv, IVTV_API_DEC_STOP_PLAYBACK, 4,
        	/*  0 = last frame, 1 = black */
               	hide_last_frame,
               	itv->dec_options.pts_low,
               	itv->dec_options.pts_hi, no_stop))
       	{
       		IVTV_DEBUG_WARN(
			"Error stopping decoding\n");
       	}

	for (count=0; test_bit(IVTV_F_T_DEC_RUNNING, &itv->t_flags) && count < 100; count++)
        {
            IVTV_DEBUG_INFO("DECODE thread is active count %d\n",count);
            /* Buffers still full */
	    wake_up(&st->waitq);
            ivtv_sleep_timeout(HZ/100, 1);
	}
            
        clear_bit(IVTV_F_T_DEC_DONE, &itv->t_flags);

	IVTV_DEBUG_INFO(
		"DEC STOP: buffers have 0x%08x bytes left\n",
		ivtv_buf_fill_lock(st, 0, BUF_USED));

	ivtv_set_irq_mask(itv, IVTV_IRQ_MASK_DECODE);

	/* Lock */
	down(&st->mlock);

	clear_bit(IVTV_F_S_NEEDS_DATA, &st->s_flags);
	clear_bit(IVTV_F_S_DECODING, &st->s_flags);
	clear_bit(IVTV_F_S_OVERFLOW, &st->s_flags);

	if (!test_bit(IVTV_F_S_NO_DMA, &st->s_flags)) {
		/* disable VBI on TV-out */
		ivtv_disable_vbi(itv);
	}

	/* Decoder DMA State */
	itv->dec_dma_stat.last_addr = 0;
	itv->dec_dma_stat.bytes_filled = 0;
	itv->dec_dma_stat.bytes_needed = 0;
	itv->dec_dma_stat.total_xfer = 0;
	itv->dec_dma_stat.sg_bytes = 0;
	itv->dec_dma_stat.data[0] = 0x0;
	itv->dec_dma_stat.data[1] = 0x0;
	itv->dec_dma_stat.data[2] = 0x80000000;
	itv->dec_dma_stat.data[3] = 0x0;
	itv->dec_dma_stat.ts = jiffies;
	itv->dec_dma_stat.wq_runs = 0;
	atomic_set(&itv->dec_dma_stat.intr, 0);

	/* deincrement decoding */
	atomic_dec(&itv->decoding);

	/* UnLock */
	up(&st->mlock);

	/* wake up wait queues */
	wake_up(&st->waitq);

	return 0;
}
