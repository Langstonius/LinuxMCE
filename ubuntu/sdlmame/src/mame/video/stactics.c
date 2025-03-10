/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)


The Video system used in Space Tactics is unusual.
Here are my notes on how the video system works:

There are 4, 4K pages of Video RAM. (B,D,E & F)

The first 1K of each VRAM page contains the following:
0 0 V V V V V H H H H H   offset value for each 8x8 bitmap
     (v-tile)  (h-tile)

The offset values are used to generate an access into the
last 2K of the VRAM page:
1 D D D D D D D D V V V   here we find 8x8 character data
     (offset)    (line)

In addition, in Page B, the upper nibble of the offset is
also used to select the color palette for the tile.

Page B, D, E, and F all work similarly, except that pages
D, E, and F can be offset from Page B by a given
number of scanlines, based on the contents of the offset
RAM

The offset RAM is addressed in this format:
1 0 0 0 P P P V V V V V V V V V
        (Page)   (scanline)
Page 4=D, 5=E, 6=F

Page D, E, and F are drawn offset from the top of the screen,
starting on the first scanline which contains a 1 in the
appropriate memory location

---

The composited monitor image is seen in a mirror.  It appears
to move when the player moves the handle, due to motors which
tilt the mirror up and down, and the monitor left and right.

---

***************************************************************************/

#include "emu.h"
#include "includes/stactics.h"



/*************************************
 *
 *  Palette
 *
 *************************************/

PALETTE_INIT_MEMBER(stactics_state,stactics)
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	for (i = 0; i < 0x400; i++)
	{
		int bit0 = (color_prom[i] >> 0) & 0x01;
		int bit1 = (color_prom[i] >> 1) & 0x01;
		int bit2 = (color_prom[i] >> 2) & 0x01;
		int bit3 = (color_prom[i] >> 3) & 0x01;

		/* red component */
		int r = 0xff * bit0;

		/* green component */
		int g = 0xff * bit1 - 0xcc * bit3;

		/* blue component */
		int b = 0xff * bit2;

		palette_set_color(machine(), i, MAKE_RGB(r, g, b));
	}
}



/*************************************
 *
 *  Scrolling
 *
 *************************************/

WRITE8_MEMBER(stactics_state::stactics_scroll_ram_w)
{

	if (data & 0x01)
	{
		switch (offset >> 8)
		{
			case 4: m_y_scroll_d = offset & 0xff; break;
			case 5: m_y_scroll_e = offset & 0xff; break;
			case 6: m_y_scroll_f = offset & 0xff; break;
		}
	}
}



/*************************************
 *
 *  Frane counter
 *
 *************************************/

CUSTOM_INPUT_MEMBER(stactics_state::stactics_get_frame_count_d3)
{

	return (m_frame_count >> 3) & 0x01;
}



/*************************************
 *
 *  Beam handling
 *
 *************************************/

WRITE8_MEMBER(stactics_state::stactics_speed_latch_w)
{

	/* This writes to a shift register which is clocked by   */
	/* a 555 oscillator.  This value determines the speed of */
	/* the LED fire beams as follows:                        */

	/*   555_freq / bits_in_SR * edges_in_SR / states_in_PR67 / frame_rate */
	/*      = num_led_states_per_frame  */
	/*   36439 / 8 * x / 32 / 60 ~= 19/8*x */

	/* Here, we will count the number of rising edges in the shift register */

	int i;
	int num_rising_edges = 0;

	for(i=0;i<8;i++)
	{
		if ( (((data>>i)&0x01) == 1) && (((data>>((i+1)%8))&0x01) == 0))
			num_rising_edges++;
	}

	m_beam_states_per_frame = num_rising_edges*19/8;
}


WRITE8_MEMBER(stactics_state::stactics_shot_trigger_w)
{

	m_shot_standby = 0;
}


WRITE8_MEMBER(stactics_state::stactics_shot_flag_clear_w)
{

	m_shot_arrive = 0;
}


CUSTOM_INPUT_MEMBER(stactics_state::stactics_get_shot_standby)
{

	return m_shot_standby;
}


CUSTOM_INPUT_MEMBER(stactics_state::stactics_get_not_shot_arrive)
{

	return !m_shot_arrive;
}


static void update_beam(stactics_state *state)
{
	/* first, update the firebeam state */
	state->m_old_beam_state = state->m_beam_state;
	if (state->m_shot_standby == 0)
		state->m_beam_state = state->m_beam_state + state->m_beam_states_per_frame;

	/* These are thresholds for the two shots from the LED fire ROM */
	/* (Note: There are two more for sound triggers, */
	/*        whenever that gets implemented)        */
	if ((state->m_old_beam_state < 0x8b) & (state->m_beam_state >= 0x8b))
		state->m_shot_arrive = 1;

	if ((state->m_old_beam_state < 0xca) & (state->m_beam_state >= 0xca))
		state->m_shot_arrive = 1;

	if (state->m_beam_state >= 0x100)
	{
		state->m_beam_state = 0;
		state->m_shot_standby = 1;
	}
}



/*************************************
 *
 *  Video drawing
 *
 *************************************/

INLINE int get_pixel_on_plane(UINT8 *videoram, UINT8 y, UINT8 x, UINT8 y_scroll)
{
	UINT8 code;
	UINT8 gfx;

	/* compute effective row */
	y = y - y_scroll;

	/* get the character code at the given pixel */
	code = videoram[((y >> 3) << 5) | (x >> 3)];

	/* get the gfx byte */
	gfx = videoram[0x800 | (code << 3) | (y & 0x07)];

	/* return the appropriate pixel within the byte */
	return (gfx >> (~x & 0x07)) & 0x01;
}


static void draw_background(stactics_state *state, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;

	bitmap.fill(0, cliprect);

	/* for every row */
	for (y = 0; y < 0x100; y++)
	{
		int x;

		/* for every pixel on the row */
		for (x = 0; x < 0x100; x++)
		{
			/* get the pixels for the four planes */
			int pixel_b = get_pixel_on_plane(state->m_videoram_b, y, x, 0);
			int pixel_d = get_pixel_on_plane(state->m_videoram_d, y, x, state->m_y_scroll_d);
			int pixel_e = get_pixel_on_plane(state->m_videoram_e, y, x, state->m_y_scroll_e);
			int pixel_f = get_pixel_on_plane(state->m_videoram_f, y, x, state->m_y_scroll_f);

			/* get the color for this pixel */
			UINT8 color = state->m_videoram_b[((y >> 3) << 5) | (x >> 3)] >> 4;

			/* assemble the pen index */
			int pen = color |
						(pixel_b << 4) |
						(pixel_f << 5) |
						(pixel_e << 6) |
						(pixel_d << 7) |
						((state->m_palette[0] & 0x01) << 8) |
						((state->m_palette[1] & 0x01) << 9);

			/* compute the effective pixel coordinate after adjusting for the
			   mirror movement - this is mechanical on the real machine */
			int sy = y + state->m_vert_pos;
			int sx = x - state->m_horiz_pos;

			/* plot if visible */
			if ((sy >= 0) && (sy < 0x100) && (sx >= 0) && (sx < 0x100))
				bitmap.pix16(sy, sx) = pen;
		}
	}
}



/*************************************
 *
 *  Non-video artwork
 *
 *************************************/

/* from 7448 datasheet */
static const int to_7seg[0x10] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07,
	0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00
};


static void set_indicator_leds(int data, const char *output_name, int base_index)
{
	/* decode the data */
	data = to_7seg[~data & 0x0f];

	/* set the 4 LEDs */
	output_set_indexed_value(output_name, base_index + 0, (data >> 2) & 0x01);
	output_set_indexed_value(output_name, base_index + 1, (data >> 6) & 0x01);
	output_set_indexed_value(output_name, base_index + 2, (data >> 5) & 0x01);
	output_set_indexed_value(output_name, base_index + 3, (data >> 4) & 0x01);
}


static void update_artwork(running_machine &machine, stactics_state *state)
{
	int i;
	UINT8 *beam_region = machine.root_device().memregion("user1")->base();

	/* set the lamps first */
	output_set_indexed_value("base_lamp", 4, state->m_lamps[0] & 0x01);
	output_set_indexed_value("base_lamp", 3, state->m_lamps[1] & 0x01);
	output_set_indexed_value("base_lamp", 2, state->m_lamps[2] & 0x01);
	output_set_indexed_value("base_lamp", 1, state->m_lamps[3] & 0x01);
	output_set_indexed_value("base_lamp", 0, state->m_lamps[4] & 0x01);
	output_set_value("start_lamp",   state->m_lamps[5] & 0x01);
	output_set_value("barrier_lamp", state->m_lamps[6] & 0x01);  /* this needs to flash on/off, not implemented */

	/* laser beam - loop for each LED */
	for (i = 0; i < 0x40; i++)
	{
		offs_t beam_data_offs = ((i & 0x08) << 7) | ((i & 0x30) << 4) | state->m_beam_state;
		UINT8 beam_data = beam_region[beam_data_offs];
		int on = (beam_data >> (i & 0x07)) & 0x01;

		output_set_indexed_value("beam_led_left", i, on);
		output_set_indexed_value("beam_led_right", i, on);
	}

	/* sight LED */
	output_set_value("sight_led", *state->m_motor_on & 0x01);

	/* score display */
	for (i = 0x01; i < 0x07; i++)
		output_set_digit_value(i - 1, to_7seg[~state->m_display_buffer[i] & 0x0f]);

	/* credits indicator */
	set_indicator_leds(state->m_display_buffer[0x07], "credit_led", 0x00);
	set_indicator_leds(state->m_display_buffer[0x08], "credit_led", 0x04);

	/* barriers indicator */
	set_indicator_leds(state->m_display_buffer[0x09], "barrier_led", 0x00);
	set_indicator_leds(state->m_display_buffer[0x0a], "barrier_led", 0x04);
	set_indicator_leds(state->m_display_buffer[0x0b], "barrier_led", 0x08);

	/* rounds indicator */
	set_indicator_leds(state->m_display_buffer[0x0c], "round_led", 0x00);
	set_indicator_leds(state->m_display_buffer[0x0d], "round_led", 0x04);
	set_indicator_leds(state->m_display_buffer[0x0e], "round_led", 0x08);
	set_indicator_leds(state->m_display_buffer[0x0f], "round_led", 0x0c);
}



/*************************************
 *
 *  Start
 *
 *************************************/

VIDEO_START_MEMBER(stactics_state,stactics)
{

	m_y_scroll_d = 0;
	m_y_scroll_e = 0;
	m_y_scroll_f = 0;

	m_frame_count = 0;
	m_shot_standby = 1;
	m_shot_arrive = 0;
	m_beam_state = 0;
	m_old_beam_state = 0;
}



/*************************************
 *
 *  Update
 *
 *************************************/

UINT32 stactics_state::screen_update_stactics(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	update_beam(this);
	draw_background(this, bitmap, cliprect);
	update_artwork(machine(), this);

	m_frame_count = (m_frame_count + 1) & 0x0f;

	return 0;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( stactics_video )

	MCFG_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(stactics_state, screen_update_stactics)

	MCFG_PALETTE_LENGTH(0x400)

	MCFG_PALETTE_INIT_OVERRIDE(stactics_state,stactics)
	MCFG_VIDEO_START_OVERRIDE(stactics_state,stactics)
MACHINE_CONFIG_END
