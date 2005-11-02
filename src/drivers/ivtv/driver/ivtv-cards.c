/*
    Functions to query card hardware
    Copyright (C) 2003-2004  Kevin Thayer <nufan_wfk at yahoo.com>

    Audio input/output:
    Copyright (C) 2004  Hans Verkuil <hverkuil@xs4all.nl>

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

#include "ivtv-driver.h"
#include "ivtv-cards.h"

#include "ivtv-i2c.h"

/********************** card configuration *******************************/

/* Inputs for Hauppauge cards */
static struct v4l2_input ivtv_pvr_inputs[] = {
        {
                .index = 0,
                .name = "Composite 0",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 1,
                .name = "Composite 1",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 2,
                .name = "Composite 2",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 3,
                .name = "Composite 3",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 4,
                .name = "Tuner 0",
                .type = V4L2_INPUT_TYPE_TUNER,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
        }, {
                .index = 5,
                .name = "Composite 4",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 6,
                .name = "S-Video 0",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 7,
                .name = "S-Video 1",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 8,
                .name = "S-Video 2",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 9,
                .name = "S-Video 3",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }
};

static struct v4l2_input ivtv_pvr150_inputs[] = {
        {
                .index = 0,
                .name = "S-Video 0",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 1,
                .name = "S-Video 1",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 2,
                .name = "Composite 0",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 3,
                .name = "Composite 1",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 4,
                .name = "Composite 2",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 5,
                .name = "Composite 3",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 6,
                .name = "Tuner 0",
                .type = V4L2_INPUT_TYPE_TUNER,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 7,
                .name = "Tuner 1",
                .type = V4L2_INPUT_TYPE_TUNER,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }
};

static const int ivtv_pvr_inputs_size =
    sizeof(ivtv_pvr_inputs) / sizeof(ivtv_pvr_inputs[0]);
static const int ivtv_pvr150_inputs_size =
    sizeof(ivtv_pvr150_inputs) / sizeof(ivtv_pvr150_inputs[0]);

static struct v4l2_input ivtv_pg600_inputs[] = {
        {
                .index = 0,
                .name = "S-Video 0",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 1,
                .name = "Tuner",
                .type = V4L2_INPUT_TYPE_TUNER,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 2,
                .name = "Composite",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 3,
                .name = "Audio tuner in",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 4,
                .name = "Audio line in",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }
};

static const int ivtv_pg600_inputs_size =
    sizeof(ivtv_pg600_inputs) / sizeof(ivtv_pg600_inputs[0]);

/* Inputs for MPG600/MPG160/M179 cards */
static struct v4l2_input ivtv_mpg_inputs[] = {
        {
                .index = 0,
                .name = "N/A",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 1,
                .name = "N/A",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 2,
                .name = "N/A",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 3,
                .name = "Composite 0",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 4,
                .name = "Tuner 0",
                .type = V4L2_INPUT_TYPE_TUNER,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 5,
                .name = "N/A",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
                .std = V4L2_STD_ALL,
        }, {
                .index = 6,
                .name = "S-Video 0",
                .type = V4L2_INPUT_TYPE_CAMERA,
                .audioset = 3,
                .tuner = 0,
                .status = 0,
        }
};

static const int ivtv_mpg_inputs_size =
    sizeof(ivtv_mpg_inputs) / sizeof(ivtv_mpg_inputs[0]);

const struct v4l2_input *ivtv_get_input(struct ivtv *itv, u16 index)
{
	switch (itv->card->type) {
	case IVTV_CARD_MPG600:
	case IVTV_CARD_MPG160:
	case IVTV_CARD_M179:
		if (index >= ivtv_mpg_inputs_size)
			return NULL;
		return &ivtv_mpg_inputs[index];
	case IVTV_CARD_PVR_150:
		if (index >= ivtv_pvr150_inputs_size)
			return NULL;
		return &ivtv_pvr150_inputs[index];
	case IVTV_CARD_PG600:
		if (index >= ivtv_pg600_inputs_size)
			return NULL;
		return &ivtv_pg600_inputs[index];
	default:
		if (index >= ivtv_pvr_inputs_size)
			return NULL;
		return &ivtv_pvr_inputs[index];
	}
}

/* Outputs for Hauppauge cards */
static struct v4l2_output ivtv_pvr_outputs[] = {
        {
                .index = 0,
                .name = "S-Video + Composite",
                .type = V4L2_OUTPUT_TYPE_ANALOG,
                .audioset = 1,
                .std = V4L2_STD_ALL,
        }, {
                .index = 1,
                .name = "Composite",
                .type = V4L2_OUTPUT_TYPE_ANALOG,
                .audioset = 1,
                .std = V4L2_STD_ALL,
        }, {
                .index = 2,
                .name = "S-Video",
                .type = V4L2_OUTPUT_TYPE_ANALOG,
                .audioset = 1,
                .std = V4L2_STD_ALL,
        }, {
                .index = 3,
                .name = "RGB",
                .type = V4L2_OUTPUT_TYPE_ANALOG,
                .audioset = 1,
                .std = V4L2_STD_ALL,
        }, {
                .index = 4,
                .name = "YUV C",
                .type = V4L2_OUTPUT_TYPE_ANALOG,
                .audioset = 1,
                .std = V4L2_STD_ALL,
        }, {
                .index = 5,
                .name = "YUV V",
                .type = V4L2_OUTPUT_TYPE_ANALOG,
                .audioset = 1,
                .std = V4L2_STD_ALL,
        }
};

static const int ivtv_pvr_outputs_size =
    sizeof(ivtv_pvr_outputs) / sizeof(ivtv_pvr_outputs[0]);

const struct v4l2_output *ivtv_get_output(struct ivtv *itv, u16 index)
{
	if (!(itv->v4l2_cap & V4L2_CAP_VIDEO_OUTPUT))
		return NULL;
	if (index >= ivtv_pvr_outputs_size)
		return NULL;
	return &ivtv_pvr_outputs[index];
}

/* Audio inputs */
static struct v4l2_audio ivtv_pvr_audio_inputs[] = {
        {
                .index = 0,
                .name = "Tuner Audio In",
                .capability = V4L2_AUDCAP_STEREO,
        }, {
                .index = 1,
                .name = "Audio Line In",
                .capability = V4L2_AUDCAP_STEREO,
        }
};

static struct v4l2_audio ivtv_pvr150_audio_inputs[] = {
	{
	 .index = 0,
	 .name = "Tuner Audio In",
	 .capability = V4L2_AUDCAP_STEREO,
	}, {
	 .index = 1,
	 .name = "Audio Line 1",
	 .capability = V4L2_AUDCAP_STEREO,
	}, {
	 .index = 2,
	 .name = "Audio Line 2",
	 .capability = V4L2_AUDCAP_STEREO,
	}, {
	 .index = 3,
	 .name = "Audio Line 3",
	 .capability = V4L2_AUDCAP_STEREO,
	}, {
	 .index = 4,
	 .name = "Audio Line 4",
	 .capability = V4L2_AUDCAP_STEREO,
	}
};

static const int ivtv_pvr_audio_inputs_size =
    sizeof(ivtv_pvr_audio_inputs) / sizeof(ivtv_pvr_audio_inputs[0]);

static const int ivtv_pvr150_audio_inputs_size =
    sizeof(ivtv_pvr150_audio_inputs) / sizeof(ivtv_pvr150_audio_inputs[0]);

const struct v4l2_audio *ivtv_get_audio_input(struct ivtv *itv, u16 index)
{
	switch (itv->card->type) {
	case IVTV_CARD_PVR_150:
		if (index >= ivtv_pvr150_audio_inputs_size)
			return NULL;
		return &ivtv_pvr150_audio_inputs[index];
		break;
	default:
		if (index >= ivtv_pvr_audio_inputs_size)
			return NULL;
		return &ivtv_pvr_audio_inputs[index];
	}
}

/* Audio outputs */
static struct v4l2_audioout ivtv_pvr_audio_outputs[] = {
	{
	 .index = 0,
	 .name = "A/V Audio Out",
	 }
};

const struct v4l2_audioout *ivtv_get_audio_output(struct ivtv *itv, u16 index)
{
	if (!(itv->v4l2_cap & V4L2_CAP_VIDEO_OUTPUT))
		return NULL;
	if (index != 0)
		return NULL;
	return &ivtv_pvr_audio_outputs[index];
}

#define V4L2_CAP_ENCODER (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_TUNER | \
		          V4L2_CAP_AUDIO | V4L2_CAP_READWRITE | V4L2_CAP_VBI_CAPTURE)
#define V4L2_CAP_ENCODER_SLICED (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_TUNER | \
		          V4L2_CAP_AUDIO | V4L2_CAP_READWRITE | V4L2_CAP_VBI_CAPTURE | \
                          V4L2_CAP_SLICED_VBI_CAPTURE)
#define V4L2_CAP_DECODER (V4L2_CAP_VBI_OUTPUT | V4L2_CAP_VIDEO_OUTPUT | \
                          V4L2_CAP_SLICED_VBI_OUTPUT)

/* tuner I2C addresses */
#define IVTV_TUNER_I2C_ADDR 		0x61
#define IVTV_MPG600_TUNER_I2C_ADDR 	0x60
#define IVTV_MPG160_TUNER_I2C_ADDR 	0x60
#define IVTV_M179_TUNER_I2C_ADDR 	0x60
#define IVTV_PG600_TUNER_I2C_ADDR 	0x61
#define IVTV_AVC2410_TUNER_I2C_ADDR	0x60

static const struct ivtv_card_pci_info ivtv_pci_pvr150[] = {
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x8801},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x8003},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0xe807},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x0807},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0xe817},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0xc801},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x0801},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x0009},
        // New Hauppauge vendor ID? 
	{PCI_DEVICE_ID_IVTV16, 0x0270,                0x0801},
	{0, 0, 0}
};

static const struct ivtv_card_pci_info ivtv_pci_pvr250[] = {
	{PCI_DEVICE_ID_IVTV15, IVTV_PCI_ID_HAUPPAUGE, 0x4001},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x4009},
	{PCI_DEVICE_ID_IVTV15, IVTV_PCI_ID_HAUPPAUGE, 0x4801},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x4801},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x4803},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_HAUPPAUGE, 0x0003},
	{0, 0, 0}
};

static const struct ivtv_card_pci_info ivtv_pci_pvr350[] = {
	{PCI_DEVICE_ID_IVTV15, IVTV_PCI_ID_HAUPPAUGE, 0x4000},
	{PCI_DEVICE_ID_IVTV15, IVTV_PCI_ID_HAUPPAUGE, 0x4800},
	{PCI_DEVICE_ID_IVTV15, IVTV_PCI_ID_HAUPPAUGE, 0},
	{0, 0, 0}
};

static const struct ivtv_card_pci_info ivtv_pci_m179[] = {
	{PCI_DEVICE_ID_IVTV15, IVTV_PCI_ID_AVERMEDIA, 0xa3cf},
	{PCI_DEVICE_ID_IVTV15, IVTV_PCI_ID_AVERMEDIA, 0xa3ce},
	{0, 0, 0}
};

static const struct ivtv_card_pci_info ivtv_pci_mpg600[] = {
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_YUAN, 0xfff3},
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_YUAN, 0xffff},
	{0, 0, 0}
};

static const struct ivtv_card_pci_info ivtv_pci_mpg160[] = {
	{PCI_DEVICE_ID_IVTV15, IVTV_PCI_ID_YUAN, 0},
	{0, 0, 0}
};

static const struct ivtv_card_pci_info ivtv_pci_pg600[] = {
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_DIAMONDMM, 0x0070},
	{0, 0, 0}
};

static const struct ivtv_card_pci_info ivtv_pci_avc2410[] = {
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_ADAPTEC, 0x0093 },
	{0, 0, 0}
};

static const struct ivtv_card_pci_info ivtv_pci_avc2010[] = {
	{PCI_DEVICE_ID_IVTV16, IVTV_PCI_ID_ADAPTEC, 0x0092 },
	{0, 0, 0}
};
	
static const struct ivtv_card ivtv_card_list[] = {
        {
                .type = IVTV_CARD_PVR_250,
                .name = "WinTV PVR 250",
                .v4l2_capabilities = V4L2_CAP_ENCODER_SLICED,
                .audio_selector = USE_MSP34XX,

                .video_dec_func = ivtv_saa7115,
                .init_ex_func = NULL,
                .chg_channel_ex_func = NULL,
                .chg_input_ex_func = NULL,

                .pci_list = ivtv_pci_pvr250,
        }, {
                .type = IVTV_CARD_PVR_350,
                .name = "WinTV PVR 350",
                .v4l2_capabilities = V4L2_CAP_ENCODER_SLICED | V4L2_CAP_DECODER,
                .audio_selector = USE_MSP34XX,

                .video_dec_func = ivtv_saa7115,
                .init_ex_func = NULL,
                .chg_channel_ex_func = NULL,
                .chg_input_ex_func = NULL,

                .pci_list = ivtv_pci_pvr350,
        }, {
                .type = IVTV_CARD_M179,
                .name = "AVerMedia M179",
                .v4l2_capabilities = V4L2_CAP_ENCODER & ~V4L2_CAP_VBI_CAPTURE,	// No VBI (yet)
                .audio_selector = USE_GPIO,

                .video_dec_func = ivtv_saa7115,
                .init_ex_func = NULL,
                .chg_channel_ex_func = NULL,
                .chg_input_ex_func = NULL,

                .pci_list = ivtv_pci_m179,
        }, {
                .type = IVTV_CARD_MPG600,
                .name = "YUAN MPG600/Kuroutoshikou ITVC16-STVLP",
                .v4l2_capabilities = V4L2_CAP_ENCODER_SLICED,
                .audio_selector = USE_GPIO,

                .video_dec_func = ivtv_saa7115,
                .init_ex_func = NULL,
                .chg_channel_ex_func = NULL,
                .chg_input_ex_func = NULL,

                .pci_list = ivtv_pci_mpg600,
        }, {
                .type = IVTV_CARD_MPG160,
                .name = "YUAN MPG160/Kuroutoshikou ITVC15-STVLP",
                .v4l2_capabilities = V4L2_CAP_ENCODER_SLICED,
                .audio_selector = USE_GPIO,

                .video_dec_func = ivtv_saa7115,
                .init_ex_func = NULL,
                .chg_channel_ex_func = NULL,
                .chg_input_ex_func = NULL,

                .pci_list = ivtv_pci_mpg160,
        }, {
                .type = IVTV_CARD_PVR_150,
                .name = "WinTV PVR 150",
                .v4l2_capabilities = V4L2_CAP_ENCODER_SLICED,
                .audio_selector = USE_PVR150,

                .video_dec_func = ivtv_cx25840,
                .init_ex_func = NULL,
                .chg_channel_ex_func = NULL,
                .chg_input_ex_func = NULL,

                .pci_list = ivtv_pci_pvr150,
        }, {
                .type = IVTV_CARD_PG600,
                .name = "YUAN PG600/DIAMOND PVR-550 (CX FALCON2)",
                .v4l2_capabilities = V4L2_CAP_ENCODER_SLICED,
                .audio_selector = USE_CX25840,

                .video_dec_func = ivtv_cx25840,
                .init_ex_func = NULL,
                .chg_channel_ex_func = NULL,
                .chg_input_ex_func = NULL,

                .pci_list = ivtv_pci_pg600,
        }, {
		.type = IVTV_CARD_AVC2410,
		.name = "Adaptec VIDEOH! AVC-2410",
		.v4l2_capabilities = V4L2_CAP_ENCODER_SLICED,
		.audio_selector = USE_MSP34XX,

		.video_dec_func = ivtv_saa7115,
		.init_ex_func = NULL,
		.chg_input_ex_func = NULL,

		.pci_list = ivtv_pci_avc2410,		
	}, {
		.type = IVTV_CARD_AVC2010,
		.name = "Adaptec VIDEOH! AVC-2010",
		.v4l2_capabilities = V4L2_CAP_ENCODER_SLICED,
		.audio_selector = USE_CS53L32A,

		.video_dec_func = ivtv_saa7115,
		.init_ex_func = NULL,
		.chg_input_ex_func = NULL,

		.pci_list = ivtv_pci_avc2010,
	}
};

static const int ivtv_cards_size =
    sizeof(ivtv_card_list) / sizeof(ivtv_card_list[0]);

const struct ivtv_card *ivtv_get_card(u16 index)
{
	if (index >= ivtv_cards_size)
		return NULL;
	return &ivtv_card_list[index];
}
