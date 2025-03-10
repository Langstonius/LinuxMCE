/******************************************************************************
 PeT mess@utanet.at 2000,2001
******************************************************************************/

#include "emu.h"
#include "includes/lynx.h"
#include "cpu/m6502/m65sc02.h"
#include "imagedev/cartslot.h"


#define PAD_UP      0x80
#define PAD_DOWN    0x40
#define PAD_LEFT    0x20
#define PAD_RIGHT   0x10


/****************************************

    Graphics Drawing

****************************************/

/*
2008-10 FP:
Current implementation: lynx_blitter reads what will be drawn and sets which line_functions to use.
It then calls lynx_blit_lines which sets the various flip bits (horizontal and vertical) and calls
the chosen line_function. These functions (available in various versions, depending on how many
color bits are to be used) finally call lynx_plot_pixel which draws the sprite.

Notice however that, based on the problems in Electrocop, Jimmy Connors Tennis and Switchblade II
(among the others), it clearly seems that something is being lost in some of the passages. From
my partial understanding while comparing the code with the manual, I would suspect the loops in
the line_functions to be not completely correct.

This whole part will be eventually moved to video/ once I'm satisfied (or I give up).
*/

/* modes from blitter command */
enum {
	BACKGROUND = 0,
	BACKGROUND_NO_COLL,
	BOUNDARY_SHADOW,
	BOUNDARY,
	NORMAL_SPRITE,
	NO_COLL,
	XOR_SPRITE,
	SHADOW
};

static UINT8 lynx_read_ram(lynx_state *state, UINT16 address)
{
	UINT8 result = 0x00;
	if (address <= 0xfbff)
		result = state->m_mem_0000[address - 0x0000];
	else if (address <= 0xfcff)
		result = state->m_mem_fc00[address - 0xfc00];
	else if (address <= 0xfdff)
		result = state->m_mem_fd00[address - 0xfd00];
	else if (address <= 0xfff7)
		result = state->m_mem_fe00[address - 0xfe00];
	else if (address >= 0xfffa)
		result = state->m_mem_fffa[address - 0xfffa];
	return result;
}

static void lynx_write_ram(lynx_state *state, UINT16 address, UINT8 data)
{
	if (address <= 0xfbff)
		state->m_mem_0000[address - 0x0000] = data;
	else if (address <= 0xfcff)
		state->m_mem_fc00[address - 0xfc00] = data;
	else if (address <= 0xfdff)
		state->m_mem_fd00[address - 0xfd00] = data;
	else if (address <= 0xfff7)
		state->m_mem_fe00[address - 0xfe00] = data;
	else if (address >= 0xfffa)
		state->m_mem_fffa[address - 0xfffa] = data;
}

/* The pen numbers range from '0' to 'F. Pen numbers '1' through 'D' are always collidable and opaque. The other
ones have different behavior depending on the sprite type: there are 8 types of sprites, each has different
characteristics relating to some or all of their pen numbers.

* Shadow Error: The hardware is missing an inverter in the 'shadow' generator. This causes sprite types that
did not invoke shadow to now invoke it and vice versa. The only actual functionality loss is that 'exclusive or'
sprites and 'background' sprites will have shadow enabled.

The sprite types relate to specific hardware functions according to the following table:


   -------------------------- SHADOW
  |   ----------------------- BOUNDARY_SHADOW
  |  |   -------------------- NORMAL_SPRITE
  |  |  |   ----------------- BOUNDARY
  |  |  |  |   -------------- BACKGROUND (+ shadow, due to bug in 'E' pen)
  |  |  |  |  |   ----------- BACKGROUND_NO_COLL
  |  |  |  |  |  |   -------- NO_COLL
  |  |  |  |  |  |  |   ----- XOR_SPRITE (+ shadow, due to bug in 'E' pen)
  |  |  |  |  |  |  |  |
  1  0  1  0  1  1  1  1      F is opaque
  0  0  1  1  0  0  0  0      E is collideable
  0  0  1  1  0  0  0  0      0 is opaque and collideable
  1  1  1  1  0  0  0  1      allow collision detect
  1  1  1  1  1  0  0  1      allow coll. buffer access
  0  0  0  0  0  0  0  1      exclusive-or the data
*/

INLINE void lynx_plot_pixel(lynx_state *state, const int mode, const INT16 x, const int y, const int color)
{
	UINT8 back;
	UINT16 screen;
	UINT16 colbuf;

	state->m_blitter.everon = TRUE;
	screen = state->m_blitter.screen + y * 80 + x / 2;
	colbuf = state->m_blitter.colbuf + y * 80 + x / 2;

	/* a note on timing: The hardware packs the pixel data and updates the screen and collision buffer a byte at a time.
	Thus the buffer update for two pixels takes 3 memory accesses for a normal sprite (write to screen buffer, read/write to collision buffer).
	+1 memory access for palette fetch?
	*/

	switch (mode&0x7)
	{
		case NORMAL_SPRITE:
		/* A sprite may be set to 'normal'. This means that pen number '0' will be transparent and
		non-collideable. All other pens will be opaque and collideable */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0x0f) | (color << 4));
				state->m_blitter.memory_accesses++;

				if(state->m_blitter.sprite_collide && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0xf0) | (state->m_blitter.spritenr << 4));
					state->m_blitter.memory_accesses += 2;
					if ((back >> 4) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
				}
				state->m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0xf0) | color);

				if(state->m_blitter.sprite_collide && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0x0f) | (state->m_blitter.spritenr));
					if ((back & 0x0f) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
				}
			}
			break;

		case BOUNDARY:
		/* A sprite may be set to 'boundary'. This is a 'normal' sprite with the exception that pen
		number 'F' is transparent (and still collideable). */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				if (color != 0x0f)
				{
					back = lynx_read_ram(state, screen);
					lynx_write_ram(state, screen, (back & 0x0f) | (color << 4));
					state->m_blitter.memory_accesses++;
				}
				if(state->m_blitter.sprite_collide && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0xf0) | (state->m_blitter.spritenr << 4));
					if ((back >> 4) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
					state->m_blitter.memory_accesses += 2;
				}
				state->m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				if (color != 0x0f)
				{
					back = lynx_read_ram(state, screen);
					lynx_write_ram(state, screen, (back & 0xf0) | color);
				}
				if(state->m_blitter.sprite_collide && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0x0f) | (state->m_blitter.spritenr));
					if ((back & 0x0f) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
				}
			}
			break;

		case SHADOW:
		/* A sprite may be set to 'shadow'. This is a 'normal' sprite with the exception that pen
		number 'E' is non-collideable (but still opaque) */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0x0f) | (color << 4));
				state->m_blitter.memory_accesses++;

				if (state->m_blitter.sprite_collide && (color != 0x0e) && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0xf0) | (state->m_blitter.spritenr << 4));
					if ((back >> 4) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
					state->m_blitter.memory_accesses += 2;
				}
				state->m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0xf0) | color);

				if (state->m_blitter.sprite_collide && (color != 0x0e) && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0x0f) | (state->m_blitter.spritenr));
					if ((back & 0x0f) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
				}
			}
			break;

		case BOUNDARY_SHADOW:
		/* This sprite is a 'normal' sprite with the characteristics of both 'boundary'
		and 'shadow'. That is, pen number 'F' is transparent (and still collideable) and
		pen number 'E' is non-collideable (but still opaque). */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				if (color != 0x0f)
				{
					back = lynx_read_ram(state, screen);
					lynx_write_ram(state, screen, (back & 0x0f) | (color << 4));
					state->m_blitter.memory_accesses++;
				}
				if (state->m_blitter.sprite_collide && (color != 0x0e) && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0xf0) | (state->m_blitter.spritenr << 4));
					if ((back >> 4) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
					state->m_blitter.memory_accesses += 2;
				}
				state->m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				if (color != 0x0f)
				{
					back = lynx_read_ram(state, screen);
					lynx_write_ram(state, screen, (back & 0xf0) | color);
				}
				if (state->m_blitter.sprite_collide && (color != 0x0e) && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0x0f) | (state->m_blitter.spritenr));
					if ((back & 0x0f) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
				}
			}
			break;

		case BACKGROUND:
		/* A sprite may be set to 'background'. This sprite will overwrite the contents of the video and
		collision buffers. Pens '0' and 'F' are no longer transparent. This sprite is used to initialize
		the buffers at the start of a 'painting'. Additionally, no collision detection is done, and no write
		to the collision depository occurs. The 'E' error will cause the pen number 'E' to be non-collideable
		and therefore not clear the collision buffer */
			if (!(x & 0x01))        /* Upper nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0x0f) | (color << 4));
				state->m_blitter.memory_accesses++;

				if (state->m_blitter.sprite_collide && (color != 0x0e) && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0xf0) | (state->m_blitter.spritenr << 4));
					state->m_blitter.memory_accesses++;
				}
				state->m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0xf0) | color);

				if (state->m_blitter.sprite_collide && (color != 0x0e) && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0x0f) | (state->m_blitter.spritenr));
				}
			}
			break;

		case BACKGROUND_NO_COLL:
		/* This is a 'background' sprite with the exception that no activity occurs in the collision buffer */
			if (!(x & 0x01))        /* Upper nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0x0f) | (color << 4));
				state->m_blitter.memory_accesses++;
				state->m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0xf0) | color);
			}
			break;

		case NO_COLL:
		/* A sprite may be set to 'non-collideable'. This means that it will have no affect on the contents of
		the collision buffer and all other collision activities are overridden (pen 'F' is not collideable). */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0x0f) | (color << 4));
				state->m_blitter.memory_accesses++;
				state->m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, (back & 0xf0) | color);
			}
			break;

		case XOR_SPRITE:
		/* This is a 'normal' sprite with the exception that the data from the video buffer is exclusive-ored
		with the sprite data and written back out to the video buffer. Collision activity is 'normal'. The 'E'
		error will cause the pen number 'E' to be non-collideable and therefore not react with the collision
		buffer */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, back^(color << 4));
				state->m_blitter.memory_accesses += 2;
				if (state->m_blitter.sprite_collide && (color != 0x0e) && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0xf0) | (state->m_blitter.spritenr << 4));
					if ((back >> 4) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
					state->m_blitter.memory_accesses += 2;
				}
				state->m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				back = lynx_read_ram(state, screen);
				lynx_write_ram(state, screen, back^color);
				if (state->m_blitter.sprite_collide && (color != 0x0e) && !(state->m_blitter.no_collide))
				{
					back = lynx_read_ram(state, colbuf);
					lynx_write_ram(state, colbuf, (back & ~0x0f) | (state->m_blitter.spritenr));
					if ((back & 0x0f) > state->m_blitter.fred)
						state->m_blitter.fred = back >> 4;
				}
			}
			break;
	}
}

static void lynx_blit_do_work( lynx_state *state, const int y, const int xdir, const int bits_per_pixel, const int mask )
{
	int next_line_addr,i,j;
	int xi, bits, color;
	UINT16 width_accum, buffer;

	next_line_addr = lynx_read_ram(state, state->m_blitter.bitmap); // offset to second sprite line
	width_accum = (xdir == 1) ? state->m_blitter.width_offset : 0;
	state->m_blitter.memory_accesses++;

	for (xi = state->m_blitter.x_pos - state->m_blitter.xoff, bits = 0, buffer = 0, j = 1; j < next_line_addr;j++)
	{
		buffer = (buffer << 8) | lynx_read_ram(state, state->m_blitter.bitmap + j);
		bits += 8; // current bits in buffer
		state->m_blitter.memory_accesses++;

		for ( ; bits > bits_per_pixel; bits -= bits_per_pixel) // last data packet at end of scanline is not rendered (qix, blulght)
		{
			color = state->m_blitter.color[(buffer >> (bits - bits_per_pixel)) & mask];
			width_accum += state->m_blitter.width;
			for (i = 0; i < (width_accum>>8); i++, xi += xdir)
			{
				if ((xi >= 0) && (xi < 160))
				{
					lynx_plot_pixel(state, state->m_blitter.mode, xi, y, color);
				}
			}
			width_accum &= 0xff;
		}
	}
}

static void lynx_blit_2color_line(lynx_state *state, const int y, const int xdir) {lynx_blit_do_work(state, y, xdir, 1, 0x01);}
static void lynx_blit_4color_line(lynx_state *state, const int y, const int xdir) {lynx_blit_do_work(state, y, xdir, 2, 0x03);}
static void lynx_blit_8color_line(lynx_state *state, const int y, const int xdir) {lynx_blit_do_work(state, y, xdir, 3, 0x07);}
static void lynx_blit_16color_line(lynx_state *state, const int y, const int xdir) {lynx_blit_do_work(state, y, xdir, 4, 0x0f);}

static void lynx_blit_rle_do_work( lynx_state *state, const INT16 y, const int xdir, const int bits_per_pixel, const int mask )
{
	int i;
	int xi;
	int buffer, bits, j;
	int literal_data, count, color;
	UINT16 width_accum;

	width_accum = (xdir == 1) ? state->m_blitter.width_offset : 0;
	for( bits = 0, j = 0, buffer = 0, xi = state->m_blitter.x_pos - state->m_blitter.xoff; ; )      /* through the rle entries */
	{
		if (bits < 5 + bits_per_pixel) /* under 7 bits no complete entry */
		{
			j++;
			if (j >= lynx_read_ram(state, state->m_blitter.bitmap))
				return;

			bits += 8;
			buffer = (buffer << 8) | lynx_read_ram(state,state->m_blitter.bitmap + j);
			state->m_blitter.memory_accesses++;
		}

		literal_data = (buffer >> (bits - 1)) & 0x01;
		bits--;
		count = (buffer >> (bits - 4)) & 0x0f; // repeat count (packed) or pixel count (literal)
		bits -= 4;

		if (literal_data)       /* count of different pixels */
		{
			for ( ; count >= 0; count--)
			{
				if (bits < bits_per_pixel)
				{
					j++;
					if (j >= lynx_read_ram(state, state->m_blitter.bitmap))
						return;
					bits += 8;
					buffer = (buffer << 8) | lynx_read_ram(state, state->m_blitter.bitmap + j);
					state->m_blitter.memory_accesses++;
				}

				color = state->m_blitter.color[(buffer >> (bits - bits_per_pixel)) & mask];
				bits -= bits_per_pixel;
				width_accum += state->m_blitter.width;
				for (i = 0; i < (width_accum>>8); i++, xi += xdir)
				{
					if ((xi >= 0) && (xi < 160))
						lynx_plot_pixel(state, state->m_blitter.mode, xi, y, color);
				}
				width_accum &= 0xff;
			}
		}
		else        /* count of same pixels */
		{
			if (count == 0) // 4 bit count value of zero indicates end-of-line in a packed sprite
				return;

			if (bits < bits_per_pixel)
			{
				j++;
				if (j >= lynx_read_ram(state, state->m_blitter.bitmap))
					return;
				bits += 8;
				buffer = (buffer << 8) | lynx_read_ram(state, state->m_blitter.bitmap + j);
				state->m_blitter.memory_accesses++;
			}

			color = state->m_blitter.color[(buffer >> (bits - bits_per_pixel)) & mask];
			bits -= bits_per_pixel;

			for ( ; count>=0; count--)
			{
				width_accum += state->m_blitter.width;
				for (i = 0; i < (width_accum>>8); i++, xi += xdir)
				{
					if ((xi >= 0) && (xi < 160))
						lynx_plot_pixel(state, state->m_blitter.mode, xi, y, color);
				}
				width_accum &= 0xff;
			}
		}
	}
}

static void lynx_blit_2color_rle_line(lynx_state *state, const int y, const int xdir) {lynx_blit_rle_do_work(state, y, xdir, 1, 0x01);}
static void lynx_blit_4color_rle_line(lynx_state *state, const int y, const int xdir) {lynx_blit_rle_do_work(state, y, xdir, 2, 0x03);}
static void lynx_blit_8color_rle_line(lynx_state *state, const int y, const int xdir) {lynx_blit_rle_do_work(state, y, xdir, 3, 0x07);}
static void lynx_blit_16color_rle_line(lynx_state *state, const int y, const int xdir) {lynx_blit_rle_do_work(state, y, xdir, 4, 0x0f);}

static void lynx_blit_lines(lynx_state *state)
{
	int i;
	INT16 y;
	int ydir = 0, xdir = 0;
	int flip = 0;

	state->m_blitter.everon = FALSE;

	switch (state->m_blitter.spr_ctl1 & 0x03)   /* Initial drawing direction */
	{
		case 0: // Down/Right (quadrant 0)
			xdir = 1;
			ydir = 1;
			flip = 0;
			break;
		case 1: // Down/Left (blockout) (quadrant 3)
			xdir = -1;
			ydir = 1;
			flip = 3;
			break;
		case 2: // Up/Right (fat bobby) (quadrant 1)
			xdir = 1;
			ydir = -1;
			flip = 1;
			break;
		case 3: // Up/Left (quadrant 2)
			xdir = -1;
			ydir = -1;
			flip = 2;
			break;
	}

	if (state->m_blitter.spr_ctl0 & 0x20)   /* Horizontal Flip */
	{
		xdir *= -1;
	}

	if (state->m_blitter.spr_ctl0 & 0x10)   /* Vertical Flip */
	{
		ydir *= -1;
	}

	// Set height accumulator based on drawing direction
	state->m_blitter.height_accumulator = (ydir == 1) ? state->m_blitter.height_offset : 0x00;

	// loop through lines, next line offset of zero indicates end of sprite
	for (y = state->m_blitter.y_pos - state->m_blitter.yoff; (i = lynx_read_ram(state, state->m_blitter.bitmap)); state->m_blitter.bitmap += i)
	{
		state->m_blitter.memory_accesses++;

		if (i == 1) // draw next quadrant
		{
			// centered sprites sprdemo3, fat bobby, blockout
			switch (flip & 0x03)
			{
				case 0:
				case 2:
					ydir *= -1;
					state->m_blitter.y_pos += ydir;
					break;
				case 1:
				case 3:
					xdir *= -1;
					state->m_blitter.x_pos += xdir;
					break;
			}
			flip++;
			y = state->m_blitter.y_pos - state->m_blitter.yoff;
			state->m_blitter.height_accumulator = (ydir == 1) ? state->m_blitter.height_offset : 0x00;
			continue;
		}


		state->m_blitter.height_accumulator += state->m_blitter.height;
		for (int i=0; i < (state->m_blitter.height_accumulator>>8); i++, y += ydir)
		{
			if (y >= 0 && y < 102)
				state->m_blitter.line_function(state, y, xdir);
			state->m_blitter.width += (INT16)state->m_blitter.stretch;
			if (state->m_blitter.vstretch) // doesn't seem to be used
			{
				state->m_blitter.height += (INT16)state->m_blitter.stretch;
				logerror("vertical stretch enabled");
			}
			state->m_blitter.tilt_accumulator += state->m_blitter.tilt;
			state->m_blitter.x_pos += (state->m_blitter.tilt_accumulator>>8);
			state->m_blitter.tilt_accumulator &= 0xff;
		}
		state->m_blitter.height_accumulator &= 0xff;
	}
}

TIMER_CALLBACK_MEMBER(lynx_state::lynx_blitter_timer)
{
	m_blitter.busy=0; // blitter finished
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}

/*
  control 0
   bit 7,6: 00 2 color
            01 4 color
            11 8 colors?
            11 16 color
   bit 5,4: 00 right down
            01 right up
            10 left down
            11 left up

#define SHADOW         (0x07)
#define XORSHADOW      (0x06)
#define NONCOLLIDABLE  (0x05)
#define NORMAL         (0x04)
#define BOUNDARY       (0x03)
#define BOUNDARYSHADOW (0x02)
#define BKGRNDNOCOL    (0x01)
#define BKGRND         (0x00)

  control 1
   bit 7: 0 bitmap rle encoded
          1 not encoded
   bit 3: 0 color info with command
          1 no color info with command

#define RELHVST        (0x30)
#define RELHVS         (0x20)
#define RELHV          (0x10)

#define SKIPSPRITE     (0x04)

#define DUP            (0x02)
#define DDOWN          (0x00)
#define DLEFT          (0x01)
#define DRIGHT         (0x00)


  coll
#define DONTCOLLIDE    (0x20)

  word next
  word data
  word x
  word y
  word width
  word height

  pixel c0 90 20 0000 datapointer x y 0100 0100 color (8 colorbytes)
  4 bit direct?
  datapointer 2 10 0
  98 (0 colorbytes)

  box c0 90 20 0000 datapointer x y width height color
  datapointer 2 10 0

  c1 98 00 4 bit direct without color bytes (raycast)

  40 10 20 4 bit rle (sprdemo2)

  line c1 b0 20 0000 datapointer x y 0100 0100 stretch tilt:x/y color (8 color bytes)
  or
  line c0 b0 20 0000 datapointer x y 0100 0100 stretch tilt:x/y color
  datapointer 2 11 0

  text ?(04) 90 20 0000 datapointer x y width height color
  datapointer 2 10 0

  stretch: hsize adder
  tilt: hpos adder

*/

static void lynx_blitter(running_machine &machine)
{
	lynx_state *state = machine.driver_data<lynx_state>();
	static const int lynx_colors[4]={2,4,8,16};
	UINT8 palette_offset;
	UINT8 coldep;

	static void (* const blit_line[4])(lynx_state *state, const int y, const int xdir)= {
	lynx_blit_2color_line,
	lynx_blit_4color_line,
	lynx_blit_8color_line,
	lynx_blit_16color_line
	};

	static void (* const blit_rle_line[4])(lynx_state *state, const int y, const int xdir)= {
	lynx_blit_2color_rle_line,
	lynx_blit_4color_rle_line,
	lynx_blit_8color_rle_line,
	lynx_blit_16color_rle_line
	};

	int i; int colors;

	state->m_blitter.mem = (UINT8*)machine.device("maincpu")->memory().space(AS_PROGRAM).get_read_ptr(0x0000);

	state->m_blitter.busy = 1; // blitter working
	state->m_blitter.memory_accesses = 0;

	// Last SCB is indicated by zero in the high byte of SCBNEXT
	while (state->m_blitter.scb_next & 0xff00)
	{
		state->m_blitter.stretch = 0;
		state->m_blitter.tilt    = 0;
		state->m_blitter.tilt_accumulator = 0;

		state->m_blitter.scb = state->m_blitter.scb_next; // current scb
		state->m_blitter.scb_next = lynx_read_ram(state, state->m_blitter.scb + SCB_SCBNEXT) | (lynx_read_ram(state, state->m_blitter.scb + SCB_SCBNEXT + 1) << 8); // next scb
		state->m_blitter.spr_ctl0 = lynx_read_ram(state, state->m_blitter.scb + SCB_SPRCTL0);
		state->m_blitter.spr_ctl1 = lynx_read_ram(state, state->m_blitter.scb + SCB_SPRCTL1);
		state->m_blitter.spr_coll = lynx_read_ram(state, state->m_blitter.scb + SCB_SPRCOLL);
		state->m_blitter.memory_accesses += 5;

		if(!(state->m_blitter.spr_ctl1 & 0x04)) // sprite will be processed (if sprite is skipped first 5 bytes are still copied to suzy)
		{
			state->m_blitter.bitmap = lynx_read_ram(state, state->m_blitter.scb + SCB_SPRDLINE) | (lynx_read_ram(state, state->m_blitter.scb + SCB_SPRDLINE + 1) << 8);
			state->m_blitter.x_pos = lynx_read_ram(state, state->m_blitter.scb + SCB_HPOSSTRT) | (lynx_read_ram(state, state->m_blitter.scb + SCB_HPOSSTRT + 1) << 8);
			state->m_blitter.y_pos = lynx_read_ram(state, state->m_blitter.scb + SCB_VPOSSTRT) | (lynx_read_ram(state, state->m_blitter.scb + SCB_VPOSSTRT + 1) << 8);
			state->m_blitter.memory_accesses += 6;

			switch(state->m_blitter.spr_ctl1 & 0x30) // reload sprite scaling
			{
				case 0x30: // width, height, tilt, stretch
					state->m_blitter.tilt = lynx_read_ram(state, state->m_blitter.scb + SCB_TILT) | (lynx_read_ram(state, state->m_blitter.scb + SCB_TILT + 1) << 8);
					state->m_blitter.memory_accesses+=2;
				case 0x20: // width, height, stretch
					state->m_blitter.stretch = lynx_read_ram(state, state->m_blitter.scb + SCB_STRETCH) | (lynx_read_ram(state, state->m_blitter.scb + SCB_STRETCH + 1) << 8);
					state->m_blitter.memory_accesses+=2;
				case 0x10: // width, height
					state->m_blitter.width = lynx_read_ram(state, state->m_blitter.scb + SCB_SPRHSIZ) | (lynx_read_ram(state, state->m_blitter.scb + SCB_SPRHSIZ + 1) << 8);
					state->m_blitter.height = lynx_read_ram(state, state->m_blitter.scb + SCB_SPRVSIZ) | (lynx_read_ram(state, state->m_blitter.scb + SCB_SPRVSIZ + 1) << 8);
					state->m_blitter.memory_accesses+=4;
			}

			if(!(state->m_blitter.spr_ctl1 & 0x08)) // reload palette
			{
				if (state->m_blitter.spr_ctl1 & 0x30)
					palette_offset = 0x0b + 2*(((state->m_blitter.spr_ctl1 & 0x30)>>4) + 1); // palette data offset depends on width, height, etc. reloading
				else
					palette_offset = 0x0b;

				colors = lynx_colors[state->m_blitter.spr_ctl0 >> 6];

				for (i = 0; i < colors / 2; i++)
				{
					state->m_blitter.color[i * 2]      = lynx_read_ram(state, state->m_blitter.scb + palette_offset + i) >> 4;
					state->m_blitter.color[i * 2 + 1 ] = lynx_read_ram(state, state->m_blitter.scb + palette_offset + i) & 0x0f;
					state->m_blitter.memory_accesses++;
				}
			}
			}


		if (!(state->m_blitter.spr_ctl1 & 0x04))        // if 0, we skip this sprite
		{
			state->m_blitter.colpos = state->m_blitter.scb + (state->m_suzy.data[COLLOFFL] | (state->m_suzy.data[COLLOFFH]<<8));
			state->m_blitter.mode = state->m_blitter.spr_ctl0 & 0x07;

			if (state->m_blitter.spr_ctl1 & 0x80) // totally literal sprite
				state->m_blitter.line_function = blit_line[state->m_blitter.spr_ctl0 >> 6];
			else
				state->m_blitter.line_function = blit_rle_line[state->m_blitter.spr_ctl0 >> 6];

			state->m_blitter.sprite_collide = !(state->m_blitter.spr_coll & 0x20);
			state->m_blitter.spritenr = state->m_blitter.spr_coll & 0x0f;
			state->m_blitter.fred = 0;

			/* Draw Sprite */
			lynx_blit_lines(state);

			if (state->m_blitter.sprite_collide && !(state->m_blitter.no_collide))
			{
				switch (state->m_blitter.mode)
				{
					case BOUNDARY_SHADOW:
					case BOUNDARY:
					case NORMAL_SPRITE:
					case XOR_SPRITE:
					case SHADOW:
						lynx_write_ram(state, state->m_blitter.colpos, state->m_blitter.fred);
						break;
				}
			}

			if (state->m_suzy.data[SPRGO] & 0x04) // Everon enabled
			{
				coldep = lynx_read_ram(state, state->m_blitter.colpos);
				if (!state->m_blitter.everon)
					coldep |= 0x80;
				else
					coldep &= 0x7f;
				lynx_write_ram(state, state->m_blitter.colpos, coldep);
			}
		}
	}

	machine.scheduler().timer_set(machine.device<cpu_device>("maincpu")->cycles_to_attotime(state->m_blitter.memory_accesses), timer_expired_delegate(FUNC(lynx_state::lynx_blitter_timer),state));
}


/****************************************

    Suzy Emulation

****************************************/


/* Math bugs of the original hardware:

- in signed multiply, the hardware thinks that 8000 is a positive number
- in signed multiply, the hardware thinks that 0 is a negative number. This is not an immediate
problem for a multiply by zero, since the answer will be re-negated to the correct polarity of
zero. However, since it will set the sign flag, you can not depend on the sign flag to be correct
if you just load the lower byte after a multiply by zero.
- in divide, the remainder will have 2 possible errors, depending on its actual value (no further
notes on these errors available) */

void lynx_state::lynx_divide()
{
	UINT32 left;
	UINT16 right;
	UINT32 res, mod;
	/*
	Hardware divide:
	            EFGH
	*             NP
	----------------
	            ABCD
	Remainder (JK)LM
	*/

	left = m_suzy.data[MATH_H] | (m_suzy.data[MATH_G] << 8) | (m_suzy.data[MATH_F] << 16) | (m_suzy.data[MATH_E] << 24);
	right = m_suzy.data[MATH_P] | (m_suzy.data[MATH_N] << 8);

	m_suzy.accumulate_overflow = FALSE;
	if (right == 0)
	{
		m_suzy.accumulate_overflow = TRUE;  /* during divisions, this bit is used to detect denominator = 0 */
		res = 0xffffffff;
		mod = 0; //?
	}
	else
	{
		res = left / right;
		mod = left % right;
	}
//  logerror("coprocessor %8x / %8x = %4x\n", left, right, res);
	m_suzy.data[MATH_D] = res & 0xff;
	m_suzy.data[MATH_C] = res >> 8;
	m_suzy.data[MATH_B] = res >> 16;
	m_suzy.data[MATH_A] = res >> 24;

	m_suzy.data[MATH_M] = mod & 0xff;
	m_suzy.data[MATH_L] = mod >> 8;
	m_suzy.data[MATH_K] = 0; // documentation states the hardware sets these to zero on divides
	m_suzy.data[MATH_J] = 0;
}

void lynx_state::lynx_multiply()
{
	UINT16 left, right;
	UINT32 res, accu;
	/*
	Hardware multiply:
	              AB
	*             CD
	----------------
	            EFGH
	Accumulate  JKLM
	*/
	m_suzy.accumulate_overflow = FALSE;

	left = m_suzy.data[MATH_B] | (m_suzy.data[MATH_A] << 8);
	right = m_suzy.data[MATH_D] | (m_suzy.data[MATH_C] << 8);

	res = left * right;

	if (m_suzy.signed_math)
	{
		if (!(m_sign_AB + m_sign_CD))   /* different signs */
			res = (res ^ 0xffffffff) + 1;
	}

	m_suzy.data[MATH_H] = res & 0xff;
	m_suzy.data[MATH_G] = res >> 8;
	m_suzy.data[MATH_F] = res >> 16;
	m_suzy.data[MATH_E] = res >> 24;

	if (m_suzy.accumulate)
	{
		accu = m_suzy.data[MATH_M] | m_suzy.data[MATH_L] << 8 | m_suzy.data[MATH_K] << 16 | m_suzy.data[MATH_J] << 24;
		accu += res;

		if (accu < res)
			m_suzy.accumulate_overflow = TRUE;

		m_suzy.data[MATH_M] = accu;
		m_suzy.data[MATH_L] = accu >> 8;
		m_suzy.data[MATH_K] = accu >> 16;
		m_suzy.data[MATH_J] = accu >> 24;
	}
}

READ8_MEMBER(lynx_state::suzy_read)
{
	UINT8 value = 0, input;

	switch (offset)
	{
		case TILTACUML:
			return m_blitter.tilt_accumulator & 0xff;
		case TILTACUMH:
			return m_blitter.tilt_accumulator>>8;
		case HOFFL:
			return m_blitter.xoff & 0xff;
		case HOFFH:
			return m_blitter.xoff>>8;
		case VOFFL:
			return m_blitter.yoff & 0xff;
		case VOFFH:
			return m_blitter.yoff>>8;
		case VIDBASL:
			return m_blitter.screen & 0xff;
		case VIDBASH:
			return m_blitter.screen>>8;
		case COLLBASL:
			return m_blitter.colbuf & 0xff;
		case COLLBASH:
			return m_blitter.colbuf>>8;
		case SCBNEXTL:
			return m_blitter.scb_next & 0xff;
		case SCBNEXTH:
			return m_blitter.scb_next>>8;
		case SPRDLINEL:
			return m_blitter.bitmap & 0xff;
		case SPRDLINEH:
			return m_blitter.bitmap>>8;
		case HPOSSTRTL:
			return m_blitter.x_pos & 0xff;
		case HPOSSTRTH:
			return m_blitter.x_pos>>8;
		case VPOSSTRTL:
			return m_blitter.y_pos & 0xff;
		case VPOSSTRTH:
			return m_blitter.y_pos>>8;
		case SPRHSIZL:
			return m_blitter.width & 0xff;
		case SPRHSIZH:
			return m_blitter.width>>8;
		case SPRVSIZL:
			return m_blitter.height & 0xff;
		case SPRVSIZH:
			return m_blitter.height>>8;
		case STRETCHL:
			return m_blitter.stretch & 0xff;
		case STRETCHH:
			return m_blitter.stretch>>8;
		case TILTL:
			return m_blitter.tilt & 0xff;
		case TILTH:
			return m_blitter.tilt>>8;
		// case SPRDOFFL:
		// case SPRVPOSL:
		// case COLLOFFL:
		case VSIZACUML:
			return m_blitter.height_accumulator & 0xff;
		case VSIZACUMH:
			return m_blitter.height_accumulator>>8;
		case HSIZOFFL:
			return m_blitter.width_offset & 0xff;
		case HSIZOFFH:
			return m_blitter.width_offset>>8;
		case VSIZOFFL:
			return m_blitter.height_offset & 0xff;
		case VSIZOFFH:
			return m_blitter.height_offset>>8;
		case SCBADRL:
			return m_blitter.scb & 0xff;
		case SCBADRH:
			return m_blitter.scb>>8;
		//case PROCADRL:
		case SUZYHREV:
			return 0x01; // must not be 0 for correct power up
		case SPRSYS:
			// math busy, last carry, unsafe access, and stop on current sprite bits not implemented.
			if (m_suzy.accumulate_overflow)
				value |= 0x40;
			if (m_blitter.vstretch)
				value |= 0x10;
			if (m_blitter.lefthanded)
				value |= 0x08;
			if (m_blitter.busy)
				value |= 0x01;
			break;
		case JOYSTICK:
			input = ioport("JOY")->read();
			switch (m_rotate)
			{
				case 1:
					value = input;
					input &= 0x0f;
					if (value & PAD_UP) input |= PAD_LEFT;
					if (value & PAD_LEFT) input |= PAD_DOWN;
					if (value & PAD_DOWN) input |= PAD_RIGHT;
					if (value & PAD_RIGHT) input |= PAD_UP;
					break;
				case 2:
					value = input;
					input &= 0x0f;
					if (value & PAD_UP) input |= PAD_RIGHT;
					if (value & PAD_RIGHT) input |= PAD_DOWN;
					if (value & PAD_DOWN) input |= PAD_LEFT;
					if (value & PAD_LEFT) input |= PAD_UP;
					break;
			}
			if (m_blitter.lefthanded)
			{
				value = input & 0x0f;
				if (input & PAD_UP) value |= PAD_DOWN;
				if (input & PAD_DOWN) value |= PAD_UP;
				if (input & PAD_LEFT) value |= PAD_RIGHT;
				if (input & PAD_RIGHT) value |= PAD_LEFT;
			}
			else
				value = input;
			break;
		case SWITCHES:
			value = ioport("PAUSE")->read();
			break;
		case RCART:
			value = *(machine().root_device().memregion("user1")->base() + (m_suzy.high * m_granularity) + m_suzy.low);
			m_suzy.low = (m_suzy.low + 1) & (m_granularity - 1);
			break;
		//case RCART_BANK1: /* we need bank 1 emulation!!! */
		case SPRCTL0:
		case SPRCTL1:
		case SPRCOLL:
		case SPRINIT:
		case SUZYBUSEN:
		case SPRGO:
			logerror("read from write only register %x\n", offset);
			value = 0;
			break;
		default:
			value = m_suzy.data[offset];
	}
	//logerror("suzy read %.2x %.2x\n",offset,value);
	return value;
}

WRITE8_MEMBER(lynx_state::suzy_write)
{
	m_suzy.data[offset] = data;
	//logerror("suzy write %.2x %.2x\n",offset,data);
	/* Additional effects of a write */
	/* Even addresses are the LSB. Any CPU write to an LSB in 0x00-0x7f will set the MSB to 0. */
	/* This in particular holds for math quantities:  Writing to B (0x54), D (0x52),
	F (0x62), H (0x60), K (0x6e) or M (0x6c) will force a '0' to be written to A (0x55),
	C (0x53), E (0x63), G (0x61), J (0x6f) or L (0x6d) respectively */
	if ((offset < 0x80) && !(offset & 0x01))
	m_suzy.data[offset + 1] = 0;

	switch(offset)
	{
		//case TMPADRL:
		//case TMPADRH:
		case TILTACUML:
			m_blitter.tilt_accumulator = data; // upper byte forced to zero see above.
			break;
		case TILTACUMH:
			m_blitter.tilt_accumulator &= 0xff;
			m_blitter.tilt_accumulator |= data<<8;
			break;
		case HOFFL:
			m_blitter.xoff = data;
			break;
		case HOFFH:
			m_blitter.xoff &= 0xff;
			m_blitter.xoff |= data<<8;
			break;
		case VOFFL:
			m_blitter.yoff = data;
			break;
		case VOFFH:
			m_blitter.yoff &= 0xff;
			m_blitter.yoff |= data<<8;
			break;
		case VIDBASL:
			m_blitter.screen = data;
			break;
		case VIDBASH:
			m_blitter.screen &= 0xff;
			m_blitter.screen |= data<<8;
			break;
		case COLLBASL:
			m_blitter.colbuf = data;
			break;
		case COLLBASH:
			m_blitter.colbuf &= 0xff;
			m_blitter.colbuf |= data<<8;
			break;
		case SCBNEXTL:
			m_blitter.scb_next = data;
			break;
		case SCBNEXTH:
			m_blitter.scb_next &= 0xff;
			m_blitter.scb_next |= data<<8;
			break;
		case SPRDLINEL:
			m_blitter.bitmap = data;
			break;
		case SPRDLINEH:
			m_blitter.bitmap &= 0xff;
			m_blitter.bitmap |= data<<8;
			break;
		case HPOSSTRTL:
			m_blitter.x_pos = data;
		case HPOSSTRTH:
			m_blitter.x_pos &= 0xff;
			m_blitter.x_pos |= data<<8;
		case VPOSSTRTL:
			m_blitter.y_pos = data;
		case VPOSSTRTH:
			m_blitter.y_pos &= 0xff;
			m_blitter.y_pos |= data<<8;
		case SPRHSIZL:
			m_blitter.width = data;
			break;
		case SPRHSIZH:
			m_blitter.width &= 0xff;
			m_blitter.width |= data<<8;
			break;
		case SPRVSIZL:
			m_blitter.height = data;
			break;
		case SPRVSIZH:
			m_blitter.height &= 0xff;
			m_blitter.height |= data<<8;
			break;
		case STRETCHL:
			m_blitter.stretch = data;
			break;
		case STRETCHH:
			m_blitter.stretch &= 0xff;
			m_blitter.stretch |= data<<8;
			break;
		case TILTL:
			m_blitter.tilt = data;
			break;
		case TILTH:
			m_blitter.tilt &= 0xff;
			m_blitter.tilt |= data<<8;
			break;
		// case SPRDOFFL:
		// case SPRVPOSL:
		// case COLLOFFL:
		case VSIZACUML:
			m_blitter.height_accumulator = data;
			break;
		case VSIZACUMH:
			m_blitter.height_accumulator &= 0xff;
			m_blitter.height_accumulator |= data<<8;
			break;
		case HSIZOFFL:
			m_blitter.width_offset = data;
			break;
		case HSIZOFFH:
			m_blitter.width_offset &= 0xff;
			m_blitter.width_offset |= data<<8;
			break;
		case VSIZOFFL:
			m_blitter.height_offset = data;
			break;
		case VSIZOFFH:
			m_blitter.height_offset &= 0xff;
			m_blitter.height_offset |= data<<8;
			break;
		case SCBADRL:
			m_blitter.scb = data;
			break;
		case SCBADRH:
			m_blitter.scb &= 0xff;
			m_blitter.scb |= data<<8;
			break;
		//case PROCADRL:

		/* Writing to M (0x6c) will also clear the accumulator overflow bit */
		case MATH_M:
			m_suzy.accumulate_overflow = FALSE;
			break;
		case MATH_C:
			/* If we are going to perform a signed multiplication, we store the sign and convert the number
			to an unsigned one */
			if (m_suzy.signed_math)
			{
				UINT16 factor, temp;
				factor = m_suzy.data[MATH_D] | (m_suzy.data[MATH_C] << 8);
				if ((factor - 1) & 0x8000)      /* here we use -1 to cover the math bugs on the sign of 0 and 0x8000 */
				{
					temp = (factor ^ 0xffff) + 1;
					m_sign_CD = - 1;
					m_suzy.data[MATH_D] = temp & 0xff;
					m_suzy.data[MATH_C] = temp >> 8;
				}
				else
					m_sign_CD = 1;
			}
			break;
		case MATH_D:
		/* Documentation states that writing to the MATH_D will set MATH_C to zero but not update the sign flag.
		Implementing the sign detection as described in the documentation causes Stun Runners to not work.
		Either the sign error in the docs is not as described or writing to the lower byte does update the sign flag.
		Here I assume the sign flag gets updated. */
			if (data)
				m_sign_CD = 1;
			break;
		/* Writing to A will start a 16 bit multiply */
		/* If we are going to perform a signed multiplication, we also store the sign and convert the
		number to an unsigned one */
		case MATH_A:
			if (m_suzy.signed_math)
			{
				UINT16 factor, temp;
				factor = m_suzy.data[MATH_B] | (m_suzy.data[MATH_A] << 8);
				if ((factor - 1) & 0x8000)      /* here we use -1 to cover the math bugs on the sign of 0 and 0x8000 */
				{
					temp = (factor ^ 0xffff) + 1;
					m_sign_AB = - 1;
					m_suzy.data[MATH_B] = temp & 0xff;
					m_suzy.data[MATH_A] = temp >> 8;
				}
				else
					m_sign_AB = 1;
			}
			lynx_multiply();
			break;
		/* Writing to E will start a 16 bit divide */
		case MATH_E:
			lynx_divide();
			break;
		case SPRCTL0:
			m_blitter.spr_ctl0 = data;
			break;
		case SPRCTL1:
			m_blitter.spr_ctl1 = data;
			break;
		case SPRCOLL:
			m_blitter.spr_coll = data;
			break;
		case SUZYBUSEN:
			logerror("write to SUSYBUSEN %x \n", data);
			break;
		case SPRSYS:
				m_suzy.signed_math = (data & 0x80) ? 1:0;
				m_suzy.accumulate = (data & 0x40) ? 1:0;
				m_blitter.no_collide = (data & 0x20) ? 1:0;
				m_blitter.vstretch = (data & 0x10) ? 1:0;
				m_blitter.lefthanded = (data & 0x08) ? 1:0;
				// unsafe access clear and sprite engine stop request are not enabled
				if (data & 0x02) logerror("sprite engine stop request\n");
				break;
		case SPRGO:
			if ((data & 0x01) && m_suzy.data[SUZYBUSEN])
			{
				//m_blitter.time = machine().time();
				lynx_blitter(machine());
			}
			break;
		case JOYSTICK:
		case SWITCHES:
			logerror("warning: write to read-only button registers\n");
			break;
	}
}


/****************************************

    Mikey emulation

****************************************/

/*
 0xfd0a r sync signal?
 0xfd81 r interrupt source bit 2 vertical refresh
 0xfd80 w interrupt quit
 0xfd87 w bit 1 !clr bit 0 blocknumber clk
 0xfd8b w bit 1 blocknumber hi B
 0xfd94 w 0
 0xfd95 w 4
 0xfda0-f rw farben 0..15
 0xfdb0-f rw bit0..3 farben 0..15
*/


/*
DISPCTL EQU $FD92       ; set to $D by INITMIKEY

; B7..B4        0
; B3    1 EQU color
; B2    1 EQU 4 bit mode
; B1    1 EQU flip screen
; B0    1 EQU video DMA enabled
*/

static void lynx_draw_line(running_machine &machine)
{
	lynx_state *state = machine.driver_data<lynx_state>();
	int x, y;
	UINT16 j; // clipping needed!
	UINT8 byte;
	UINT16 *line;


	// calculate y: first three lines are vblank,
	y = 101-state->m_timer[2].counter;
	// Documentation states lower two bits of buffer address are ignored (thus 0xfffc mask)
	j = (state->m_mikey.disp_addr & 0xfffc) + y * 160 / 2;

	if (state->m_mikey.data[0x92] & 0x02)
	{
		j -= 160 * 102 / 2 - 1;
		line = &state->m_bitmap_temp.pix16(102 - 1 - y);
		for (x = 160 - 2; x >= 0; j++, x -= 2)
		{
			byte = lynx_read_ram(state, j);
			line[x + 1] = state->m_palette[(byte >> 4) & 0x0f];
			line[x + 0] = state->m_palette[(byte >> 0) & 0x0f];
		}
	}
	else
	{
		line = &state->m_bitmap_temp.pix16(y);
		for (x = 0; x < 160; j++, x += 2)
		{
			byte = lynx_read_ram(state, j);
			line[x + 0] = state->m_palette[(byte >> 4) & 0x0f];
			line[x + 1] = state->m_palette[(byte >> 0) & 0x0f];
		}
	}
}

/****************************************

    Timers

****************************************/

/*
HCOUNTER        EQU TIMER0
VCOUNTER        EQU TIMER2
SERIALRATE      EQU TIMER4

TIM_BAKUP       EQU 0   ; backup-value (count+1)
TIM_CNTRL1      EQU 1   ; timer-control register
TIM_CNT EQU 2   ; current counter
TIM_CNTRL2      EQU 3   ; dynamic control

; TIM_CNTRL1
TIM_IRQ EQU %10000000   ; enable interrupt (not TIMER4 !)
TIM_RESETDONE   EQU %01000000   ; reset timer done
TIM_MAGMODE     EQU %00100000   ; nonsense in Lynx !!
TIM_RELOAD      EQU %00010000   ; enable reload
TIM_COUNT       EQU %00001000   ; enable counter
TIM_LINK        EQU %00000111
; link timers (0->2->4 / 1->3->5->7->Aud0->Aud1->Aud2->Aud3->1
TIM_64us        EQU %00000110
TIM_32us        EQU %00000101
TIM_16us        EQU %00000100
TIM_8us EQU %00000011
TIM_4us EQU %00000010
TIM_2us EQU %00000001
TIM_1us EQU %00000000

;TIM_CNTRL2 (read-only)
; B7..B4 unused
TIM_DONE        EQU %00001000   ; set if timer's done; reset with TIM_RESETDONE
TIM_LAST        EQU %00000100   ; last clock (??)
TIM_BORROWIN    EQU %00000010
TIM_BORROWOUT   EQU %00000001
*/

#define NR_LYNX_TIMERS  8




static void lynx_timer_init(running_machine &machine, int which)
{
	lynx_state *state = machine.driver_data<lynx_state>();
	memset( &state->m_timer[which], 0, sizeof(LYNX_TIMER) );
	state->m_timer[which].timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(lynx_state::lynx_timer_shot),state));
}

static void lynx_timer_signal_irq(running_machine &machine, int which)
{
	lynx_state *state = machine.driver_data<lynx_state>();
	if ( ( state->m_timer[which].cntrl1 & 0x80 ) && ( which != 4 ) ) // if interrupts are enabled and timer != 4
	{
		state->m_mikey.data[0x81] |= ( 1 << which ); // set interupt poll register
		machine.device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		machine.device("maincpu")->execute().set_input_line(M65SC02_IRQ_LINE, ASSERT_LINE);
	}
	switch ( which ) // count down linked timers
	{
	case 0:
		switch (state->m_timer[2].counter)
		{
			case 104:
				break;
			case 103:
				state->m_mikey.vb_rest = 1;
				break;
			case 102:
				state->m_mikey.disp_addr = state->m_mikey.data[0x94] | (state->m_mikey.data[0x95] << 8);
				break;
			case 101:
				state->m_mikey.vb_rest = 0;
				lynx_draw_line( machine );
				break;
			default:
				lynx_draw_line( machine );
		}
		lynx_timer_count_down( machine, 2 );
		break;
	case 2:
		copybitmap(state->m_bitmap, state->m_bitmap_temp, 0, 0, 0, 0, machine.primary_screen->cliprect());
		lynx_timer_count_down( machine, 4 );
		break;
	case 1:
		lynx_timer_count_down( machine, 3 );
		break;
	case 3:
		lynx_timer_count_down( machine, 5 );
		break;
	case 5:
		lynx_timer_count_down( machine, 7 );
		break;
	case 7:
		lynx_audio_count_down( state->m_audio, 0 );
		break;
	}
}

void lynx_timer_count_down(running_machine &machine, int which)
{
	lynx_state *state = machine.driver_data<lynx_state>();
	if ( ( state->m_timer[which].cntrl1 & 0x0f ) == 0x0f ) // count and linking enabled
	{
		if ( state->m_timer[which].counter > 0 )
		{
			state->m_timer[which].counter--;
			//state->m_timer[which].borrow_in = 1;
			return;
		}
		if ( state->m_timer[which].counter == 0 )
		{
			if (state->m_timer[which].cntrl2 & 0x01) // borrow out
			{
				lynx_timer_signal_irq(machine, which);
				if ( state->m_timer[which].cntrl1 & 0x10 ) // if reload enabled
				{
					state->m_timer[which].counter = state->m_timer[which].bakup;
				}
				else
				{
					state->m_timer[which].cntrl2 |= 8; // set timer done
				}
				state->m_timer[which].cntrl2 &= ~0x01; // clear borrow out
			}
			else
				state->m_timer[which].cntrl2 |= 0x01; // set borrow out
			return;
		}
	}
	else
	{
		//state->m_timer[which].borrow_in = 0;
		state->m_timer[which].cntrl2 &= ~0x01;
	}
}

static UINT32 lynx_time_factor(int val)
{
	switch(val)
	{
		case 0: return 1000000;
		case 1: return 500000;
		case 2: return 250000;
		case 3: return 125000;
		case 4: return 62500;
		case 5: return 31250;
		case 6: return 15625;
		default: fatalerror("invalid value\n");
	}
}

TIMER_CALLBACK_MEMBER(lynx_state::lynx_timer_shot)
{
	lynx_timer_signal_irq( machine(), param );
	if ( ! ( m_timer[param].cntrl1 & 0x10 ) ) // if reload not enabled
	{
		m_timer[param].timer_active = 0;
		m_timer[param].cntrl2 |= 8; // set timer done
	}
	else
	{
		attotime t = (attotime::from_hz(lynx_time_factor(m_timer[param].cntrl1 & 0x07)) * (m_timer[param].bakup + 1));
		m_timer[param].timer->adjust(t, param);
	}
}

UINT8 lynx_state::lynx_timer_read(int which, int offset)
{
	UINT8 value = 0;

	switch (offset)
	{
		case 0:
			value = m_timer[which].bakup;
			break;
		case 1:
			value = m_timer[which].cntrl1;
			break;
		case 2:
			if ((m_timer[which].cntrl1 & 0x07) == 0x07) // linked timer
			{
				value = m_timer[which].counter;
			}
			else
			{
				if ( m_timer[which].timer_active )
				{
					value = (UINT8) (m_timer[which].timer->remaining().as_ticks(1000000>>(m_timer[which].cntrl1 & 0x07)));
					value -= value ? 1 : 0;
				}
			}
			break;

		case 3:
			value = m_timer[which].cntrl2;
			break;
	}
	// logerror("timer %d read %x %.2x\n", which, offset, value);
	return value;
}

void lynx_state::lynx_timer_write(int which, int offset, UINT8 data)
{
	//logerror("timer %d write %x %.2x\n", which, offset, data);
	attotime t;

	if ( m_timer[which].timer_active && ((m_timer[which].cntrl1 & 0x07) != 0x07))
	{
		m_timer[which].counter = (UINT8) (m_timer[which].timer->remaining().as_ticks(1000000>>(m_timer[which].cntrl1 & 0x07)));
		m_timer[which].counter -= (m_timer[which].counter) ? 1 : 0;
	}

	switch (offset)
	{
		case 0:
			m_timer[which].bakup = data;
			break;
		case 1:
			m_timer[which].cntrl1 = data;
			if (data & 0x40) // reset timer done
				m_timer[which].cntrl2 &= ~0x08;
			break;
		case 2:
			m_timer[which].counter = data;
			break;
		case 3:
			m_timer[which].cntrl2 = (m_timer[which].cntrl2 & ~0x08) | (data & 0x08);
			break;
	}

	/* Update timers */
	//if ( offset < 3 )
	//{
		m_timer[which].timer->reset();
		m_timer[which].timer_active = 0;
		if ((m_timer[which].cntrl1 & 0x08) && !(m_timer[which].cntrl2 & 0x08))      // if enable count
		{
			if ((m_timer[which].cntrl1 & 0x07) != 0x07)  // if not set to link mode
			{
				t = (attotime::from_hz(lynx_time_factor(m_timer[which].cntrl1 & 0x07)) * (m_timer[which].counter + 1));
				m_timer[which].timer->adjust(t, which);
				m_timer[which].timer_active = 1;
			}
		}
	//}
}


/****************************************

    UART Emulation

****************************************/


static void lynx_uart_reset(lynx_state *state)
{
	memset(&state->m_uart, 0, sizeof(state->m_uart));
}

TIMER_CALLBACK_MEMBER(lynx_state::lynx_uart_loopback_timer)
{
	m_uart.received = FALSE;
}

TIMER_CALLBACK_MEMBER(lynx_state::lynx_uart_timer)
{
	if (m_uart.buffer_loaded)
	{
		m_uart.data_to_send = m_uart.buffer;
		m_uart.buffer_loaded = FALSE;
		machine().scheduler().timer_set(attotime::from_usec(11*16), timer_expired_delegate(FUNC(lynx_state::lynx_uart_timer),this));
	}
	else
	{
		m_uart.sending = FALSE;
		m_uart.received = TRUE;
		m_uart.data_received = m_uart.data_to_send;
		machine().scheduler().timer_set(attotime::from_usec(11*16), timer_expired_delegate(FUNC(lynx_state::lynx_uart_loopback_timer),this));
		if (m_uart.serctl & 0x40)
		{
			m_mikey.data[0x81] |= 0x10;
			machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			machine().device("maincpu")->execute().set_input_line(M65SC02_IRQ_LINE, ASSERT_LINE);
		}
	}

	if (m_uart.serctl & 0x80)
	{
		m_mikey.data[0x81] |= 0x10;
		machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		machine().device("maincpu")->execute().set_input_line(M65SC02_IRQ_LINE, ASSERT_LINE);
	}
}

static  READ8_HANDLER(lynx_uart_r)
{
	lynx_state *state = space.machine().driver_data<lynx_state>();
	UINT8 value = 0x00;
	switch (offset)
	{
		case 0x8c:
			if (!state->m_uart.buffer_loaded)
				value |= 0x80;
			if (state->m_uart.received)
				value |= 0x40;
			if (!state->m_uart.sending)
				value |= 0x20;
			break;

		case 0x8d:
			value = state->m_uart.data_received;
			break;
	}
	logerror("uart read %.2x %.2x\n", offset, value);
	return value;
}

WRITE8_MEMBER(lynx_state::lynx_uart_w)
{
	logerror("uart write %.2x %.2x\n", offset, data);
	switch (offset)
	{
		case 0x8c:
			m_uart.serctl = data;
			break;

		case 0x8d:
			if (m_uart.sending)
			{
				m_uart.buffer = data;
				m_uart.buffer_loaded = TRUE;
			}
			else
			{
				m_uart.sending = TRUE;
				m_uart.data_to_send = data;
				// timing not accurate, baude rate should be calculated from timer 4 backup value and clock rate
				machine().scheduler().timer_set(attotime::from_usec(11*16), timer_expired_delegate(FUNC(lynx_state::lynx_uart_timer),this));
			}
			break;
	}
}


/****************************************

    Mikey memory handlers

****************************************/


READ8_MEMBER(lynx_state::mikey_read)
{
	UINT8 direction, value = 0x00;

	switch (offset)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		value = lynx_timer_read(offset >> 2, offset & 0x03);
		break;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x50:
		value = lynx_audio_read(m_audio, offset);
		break;

	case 0x80:
	case 0x81:
		value = m_mikey.data[0x81]; // both registers access the same interupt status byte
		// logerror( "mikey read %.2x %.2x\n", offset, value );
		break;

	case 0x84:
	case 0x85:
		value = 0x00;
		break;

	case 0x86:
		value = 0x80;
		break;

	case 0x88:
		value = 0x01;
		break;

	case 0x8b:
		direction = m_mikey.data[0x8a];
		value |= (direction & 0x01) ? (m_mikey.data[offset] & 0x01) : 0x01; // External Power input
		value |= (direction & 0x02) ? (m_mikey.data[offset] & 0x02) : 0x00; // Cart Address Data output (0 turns cart power on)
		value |= (direction & 0x04) ? (m_mikey.data[offset] & 0x04) : 0x04; // noexp input
		// REST read returns actual rest state anded with rest output bit
		value |= (direction & 0x08) ? (((m_mikey.data[offset] & 0x08) && (m_mikey.vb_rest)) ? 0x00 : 0x08) : 0x00;  // rest output
		value |= (direction & 0x10) ? (m_mikey.data[offset] & 0x10) : 0x10; // audin input
		/* Hack: we disable COMLynx  */
		value |= 0x04;
		/* B5, B6 & B7 are not used */
		break;

	case 0x8c:
	case 0x8d:
		value = lynx_uart_r(space, offset, mem_mask);
		break;

	default:
		value = m_mikey.data[offset];
		//logerror( "mikey read %.2x %.2x\n", offset, value );
	}
	return value;
}

WRITE8_MEMBER(lynx_state::mikey_write)
{
	switch (offset)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		lynx_timer_write(offset >> 2, offset & 3, data);
		return;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x50:
		lynx_audio_write(m_audio, offset, data);
		return;

	case 0x80:
		m_mikey.data[0x81] &= ~data; // clear interrupt source
		// logerror("mikey write %.2x %.2x\n", offset, data);
		if (!m_mikey.data[0x81])
			machine().device("maincpu")->execute().set_input_line(M65SC02_IRQ_LINE, CLEAR_LINE);
		break;

	/* Is this correct? */ // Notes say writing to register will result in interupt being triggered.
	case 0x81:
		m_mikey.data[0x81] |= data;
		if (data)
		{
			machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			machine().device("maincpu")->execute().set_input_line(M65SC02_IRQ_LINE, ASSERT_LINE);
			logerror("direct write to interupt register\n");
		}
		break;

	case 0x87:
		m_mikey.data[offset] = data;
		if (data & 0x02)        // Power (1 = on)
		{
			if (data & 0x01)    // Cart Address Strobe
			{
				m_suzy.high <<= 1;
				if (m_mikey.data[0x8b] & 0x02)
					m_suzy.high |= 1;
				m_suzy.high &= 0xff;
				m_suzy.low = 0;
			}
		}
		else
		{
			m_suzy.high = 0;
			m_suzy.low = 0;
		}
		break;

	case 0x8c: case 0x8d:
		lynx_uart_w(space, offset, data);
		break;

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		m_mikey.data[offset] = data;

		/* RED = 0xb- & 0x0f, GREEN = 0xa- & 0x0f, BLUE = (0xb- & 0xf0) >> 4 */
		m_palette[offset & 0x0f] = machine().pens[
			((m_mikey.data[0xb0 + (offset & 0x0f)] & 0x0f)) |
			((m_mikey.data[0xa0 + (offset & 0x0f)] & 0x0f) << 4) |
			((m_mikey.data[0xb0 + (offset & 0x0f)] & 0xf0) << 4)];
		break;

	/* TODO: properly implement these writes */
	case 0x8b:
		m_mikey.data[offset] = data;
		if (m_mikey.data[0x8a] & 0x10)
			logerror("Trying to enable bank 1 write. %d\n", m_mikey.data[offset] & 0x10);
		break;

	//case 0x90: // SDONEACK - Suzy Done Acknowledge
	case 0x91: // CPUSLEEP - CPU Bus Request Disable
		m_mikey.data[offset] = data;
		if (!data && m_blitter.busy)
		{
			machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			/* A write of '0' to this address will reset the CPU bus request flip flop */
		}
		break;
	case 0x94: case 0x95:
		m_mikey.data[offset]=data;
		break;
	case 0x9c: case 0x9d: case 0x9e:
		m_mikey.data[offset]=data;
		logerror("Mtest%d write: %x\n", offset&0x3, data);
		break;

	default:
		m_mikey.data[offset]=data;
		//logerror("mikey write %.2x %.2x\n",offset,data);
		break;
	}
}

/****************************************

    Init / Config

****************************************/

READ8_MEMBER(lynx_state::lynx_memory_config_r)
{
	return m_memory_config;
}

WRITE8_MEMBER(lynx_state::lynx_memory_config_w)
{
	/* bit 7: hispeed, uses page mode accesses (4 instead of 5 cycles )
	 * when these are safe in the cpu */
	m_memory_config = data;

	if (data & 1) {
		space.install_readwrite_bank(0xfc00, 0xfcff, "bank1");
	} else {
		space.install_readwrite_handler(0xfc00, 0xfcff, read8_delegate(FUNC(lynx_state::suzy_read),this), write8_delegate(FUNC(lynx_state::suzy_write),this));
	}
	if (data & 2) {
		space.install_readwrite_bank(0xfd00, 0xfdff, "bank2");
	} else {
		space.install_readwrite_handler(0xfd00, 0xfdff, read8_delegate(FUNC(lynx_state::mikey_read),this), write8_delegate(FUNC(lynx_state::mikey_write),this));
	}

	if (data & 1)
		membank("bank1")->set_base(m_mem_fc00);
	if (data & 2)
		membank("bank2")->set_base(m_mem_fd00);
	membank("bank3")->set_entry((data & 4) ? 1 : 0);
	membank("bank4")->set_entry((data & 8) ? 1 : 0);
}

static void lynx_reset(running_machine &machine)
{
	lynx_state *state = machine.driver_data<lynx_state>();
	state->lynx_memory_config_w(machine.device("maincpu")->memory().space(AS_PROGRAM), 0, 0);

	machine.device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	machine.device("maincpu")->execute().set_input_line(M65SC02_IRQ_LINE, CLEAR_LINE);

	memset(&state->m_suzy, 0, sizeof(state->m_suzy));
	memset(&state->m_mikey, 0, sizeof(state->m_mikey));

	state->m_suzy.data[0x88]  = 0x01;
	state->m_suzy.data[0x90]  = 0x00;
	state->m_suzy.data[0x91]  = 0x00;
	state->m_mikey.data[0x80] = 0x00;
	state->m_mikey.data[0x81] = 0x00;
	state->m_mikey.data[0x88] = 0x01;
	state->m_mikey.data[0x8a] = 0x00;
	state->m_mikey.data[0x8c] = 0x00;
	state->m_mikey.data[0x90] = 0x00;
	state->m_mikey.data[0x92] = 0x00;

	lynx_uart_reset(state);

	// hack to allow current object loading to work
#if 0
	lynx_timer_write( state, 0, 0, 160 ); // set backup value (hpos) = 160
	lynx_timer_write( state, 0, 1, 0x10 | 0x8 | 0 ); // enable count, enable reload, 1us period
	lynx_timer_write( state, 2, 0, 105 ); // set backup value (vpos) = 102
	lynx_timer_write( state, 2, 1, 0x10 | 0x8 | 7 ); // enable count, enable reload, link
#endif
}

static void lynx_postload(lynx_state *state)
{
	state->lynx_memory_config_w( state->machine().device("maincpu")->memory().space(AS_PROGRAM), 0, state->m_memory_config);
}

void lynx_state::machine_start()
{
	m_bitmap_temp.allocate(160,102,0,0);

	int i;
	save_item(NAME(m_memory_config));
	save_pointer(NAME(m_mem_fe00.target()), m_mem_fe00.bytes());
	machine().save().register_postload(save_prepost_delegate(FUNC(lynx_postload), this));

	membank("bank3")->configure_entry(0, machine().root_device().memregion("maincpu")->base() + 0x0000);
	membank("bank3")->configure_entry(1, m_mem_fe00);
	membank("bank4")->configure_entry(0, memregion("maincpu")->base() + 0x01fa);
	membank("bank4")->configure_entry(1, m_mem_fffa);

	m_audio = machine().device("custom");

	memset(&m_suzy, 0, sizeof(m_suzy));

	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(lynx_reset),&machine()));

	for (i = 0; i < NR_LYNX_TIMERS; i++)
		lynx_timer_init(machine(), i);
}

void lynx_state::machine_reset()
{
	render_target *target = machine().render().first_target();
	target->set_view(m_rotate);
}

/****************************************

    Image handling

****************************************/


static void lynx_partialhash(hash_collection &dest, const unsigned char *data,
	unsigned long length, const char *functions)
{
	if (length <= 64)
		return;
	dest.compute(&data[64], length - 64, functions);
}

int lynx_verify_cart (char *header, int kind)
{
	if (kind)
	{
		if (strncmp("BS93", &header[6], 4))
		{
			logerror("This is not a valid Lynx image\n");
			return IMAGE_VERIFY_FAIL;
		}
	}
	else
	{
		if (strncmp("LYNX",&header[0],4))
		{
			if (!strncmp("BS93", &header[6], 4))
			{
				logerror("This image is probably a Quickload image with .lnx extension\n");
				logerror("Try to load it with -quickload\n");
			}
			else
				logerror("This is not a valid Lynx image\n");
			return IMAGE_VERIFY_FAIL;
		}
	}

	return IMAGE_VERIFY_PASS;
}

static DEVICE_IMAGE_LOAD( lynx_cart )
{
	/* Lynx carts have 19 address lines, the upper 8 used for bank select. The lower
	11 bits are used to address data within the selected bank. Valid bank sizes are 256,
	512, 1024 or 2048 bytes. Commercial roms use all 256 banks.*/

	lynx_state *state = image.device().machine().driver_data<lynx_state>();
	UINT8 *rom = state->memregion("user1")->base();
	UINT32 size;
	UINT8 header[0x40];

	if (image.software_entry() == NULL)
	{
		const char *filetype;
		size = image.length();
/* 64 byte header
   LYNX
   intelword lower counter size
   0 0 1 0
   32 chars name
   22 chars manufacturer
*/

		filetype = image.filetype();

		if (!mame_stricmp (filetype, "lnx"))
		{
			if (image.fread( header, 0x40) != 0x40)
				return IMAGE_INIT_FAIL;

			/* Check the image */
			if (lynx_verify_cart((char*)header, LYNX_CART) == IMAGE_VERIFY_FAIL)
				return IMAGE_INIT_FAIL;

			/* 2008-10 FP: According to Handy source these should be page_size_bank0. Are we using
			it correctly in MESS? Moreover, the next two values should be page_size_bank1. We should
			implement this as well */
			state->m_granularity = header[4] | (header[5] << 8);

			logerror ("%s %dkb cartridge with %dbyte granularity from %s\n",
					header + 10, size / 1024, state->m_granularity, header + 42);

			size -= 0x40;
		}
		else if (!mame_stricmp (filetype, "lyx"))
		{
			/* 2008-10 FP: FIXME: .lyx file don't have an header, hence they miss "lynx_granularity"
			(see above). What if bank 0 has to be loaded elsewhere? And what about bank 1?
			These should work with most .lyx files, but we need additional info on raw cart images */
			if (size == 0x20000)
				state->m_granularity = 0x0200;
			else if (size == 0x80000)
				state->m_granularity = 0x0800;
			else
				state->m_granularity = 0x0400;
		}

		if (image.fread( rom, size) != size)
			return IMAGE_INIT_FAIL;
	}
	else
	{
		size = image.get_software_region_length("rom");
		if (size > 0xffff) // 64,128,256,512k cartridges
		state->m_granularity = size >> 8;
		else
		state->m_granularity = 0x400; // Homebrew roms not using all 256 banks (T-Tris) (none currently in softlist)

		memcpy(rom, image.get_software_region("rom"), size);

		const char *rotate = image.get_feature("rotation");
		state->m_rotate = 0;
		if (rotate)
		{
			if(strcmp(rotate, "RIGHT") == 0) {
				state->m_rotate = 1;
			}
			else if (strcmp(rotate, "LEFT") == 0) {
				state->m_rotate = 2;
			}
		}

	}

	return IMAGE_INIT_PASS;
}

MACHINE_CONFIG_FRAGMENT(lynx_cartslot)
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("lnx,lyx")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("lynx_cart")
	MCFG_CARTSLOT_LOAD(lynx_cart)
	MCFG_CARTSLOT_PARTIALHASH(lynx_partialhash)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","lynx")
MACHINE_CONFIG_END
