/***************************************************************************

  Atari TIA video emulation

***************************************************************************/

#include "emu.h"
#include "tia.h"
#include "sound/tiaintf.h"

static const int nusiz[8][3] =
{
	{ 1, 1, 0 },
	{ 2, 1, 1 },
	{ 2, 1, 3 },
	{ 3, 1, 1 },
	{ 2, 1, 7 },
	{ 1, 2, 0 },
	{ 3, 1, 3 },
	{ 1, 4, 0 }
};

static void extend_palette(running_machine &machine) {
	int i,j;

	for( i = 0; i < 128; i ++ )
	{
		rgb_t   new_rgb = palette_get_color( machine, i );
		UINT8   new_r = RGB_RED( new_rgb );
		UINT8   new_g = RGB_GREEN( new_rgb );
		UINT8   new_b = RGB_BLUE( new_rgb );

		for ( j = 0; j < 128; j++ )
		{
			rgb_t   old_rgb = palette_get_color( machine, j );
			UINT8   old_r = RGB_RED( old_rgb );
			UINT8   old_g = RGB_GREEN( old_rgb );
			UINT8   old_b = RGB_BLUE( old_rgb );

			palette_set_color_rgb(machine, ( ( i + 1 ) << 7 ) | j,
				( new_r + old_r ) / 2,
				( new_g + old_g ) / 2,
				( new_b + old_b ) / 2 );
		}
	}
}

PALETTE_INIT( tia_NTSC )
{
	int i, j;

	static const double color[16][2] =
	{
		{  0.000,  0.000 },
		{  0.144, -0.189 },
		{  0.231, -0.081 },
		{  0.243,  0.032 },
		{  0.217,  0.121 },
		{  0.117,  0.216 },
		{  0.021,  0.233 },
		{ -0.066,  0.196 },
		{ -0.139,  0.134 },
		{ -0.182,  0.062 },
		{ -0.175, -0.022 },
		{ -0.136, -0.100 },
		{ -0.069, -0.150 },
		{  0.005, -0.159 },
		{  0.071, -0.125 },
		{  0.124, -0.089 }
	};

	for (i = 0; i < 16; i++)
	{
		double I = color[i][0];
		double Q = color[i][1];

		for (j = 0; j < 8; j++)
		{
			double Y = j / 7.0;

			double R = Y + 0.956 * I + 0.621 * Q;
			double G = Y - 0.272 * I - 0.647 * Q;
			double B = Y - 1.106 * I + 1.703 * Q;

			R = pow(R, 0.9) / pow(1, 0.9);
			G = pow(G, 0.9) / pow(1, 0.9);
			B = pow(B, 0.9) / pow(1, 0.9);

			if (R < 0) R = 0;
			if (G < 0) G = 0;
			if (B < 0) B = 0;

			if (R > 1) R = 1;
			if (G > 1) G = 1;
			if (B > 1) B = 1;

			palette_set_color_rgb(machine,8 * i + j,
				(UINT8) (255 * R + 0.5),
				(UINT8) (255 * G + 0.5),
				(UINT8) (255 * B + 0.5));
		}
	}
	extend_palette( machine );
}


PALETTE_INIT( tia_PAL )
{
	int i, j;

	static const double color[16][2] =
	{
		{  0.000,  0.000 },
		{  0.000,  0.000 },
		{ -0.222,  0.032 },
		{ -0.199, -0.027 },
		{ -0.173,  0.117 },
		{ -0.156, -0.197 },
		{ -0.094,  0.200 },
		{ -0.071, -0.229 },
		{  0.070,  0.222 },
		{  0.069, -0.204 },
		{  0.177,  0.134 },
		{  0.163, -0.130 },
		{  0.219,  0.053 },
		{  0.259, -0.042 },
		{  0.000,  0.000 },
		{  0.000,  0.000 }
	};

	for (i = 0; i < 16; i++)
	{
		double U = color[i][0];
		double V = color[i][1];

		for (j = 0; j < 8; j++)
		{
			double Y = j / 7.0;

			double R = Y + 1.403 * V;
			double G = Y - 0.344 * U - 0.714 * V;
			double B = Y + 1.770 * U;

			R = pow(R, 1.2) / pow(1, 1.2);
			G = pow(G, 1.2) / pow(1, 1.2);
			B = pow(B, 1.2) / pow(1, 1.2);

			if (R < 0) R = 0;
			if (G < 0) G = 0;
			if (B < 0) B = 0;

			if (R > 1) R = 1;
			if (G > 1) G = 1;
			if (B > 1) B = 1;

			palette_set_color_rgb(machine,8 * i + j,
				(UINT8) (255 * R + 0.5),
				(UINT8) (255 * G + 0.5),
				(UINT8) (255 * B + 0.5));
		}
	}
	extend_palette( machine );
}

// device type definition
const device_type TIA_VIDEO = &device_creator<tia_video_device>;

//-------------------------------------------------
//  tia_video_device - constructor
//-------------------------------------------------

tia_video_device::tia_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TIA_VIDEO, "TIA Video", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tia_video_device::device_config_complete()
{
	// inherit a copy of the static data
	const tia_interface *intf = reinterpret_cast<const tia_interface *>(static_config());
	if (intf != NULL)
		*static_cast<tia_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_read_input_port_cb, 0, sizeof(m_read_input_port_cb));
		memset(&m_databus_contents_cb, 0, sizeof(m_databus_contents_cb));
		memset(&m_vsync_callback_cb, 0, sizeof(m_vsync_callback_cb));
	}
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tia_video_device::device_start()
{
	// resolve callbacks
	m_read_input_port_func.resolve(m_read_input_port_cb, *this);
	m_databus_contents_func.resolve(m_databus_contents_cb, *this);
	m_vsync_callback_func.resolve(m_vsync_callback_cb, *this);


	int cx = machine().primary_screen->width();

	screen_height = machine().primary_screen->height();
	helper[0] = auto_bitmap_ind16_alloc(machine(), cx, TIA_MAX_SCREEN_HEIGHT);
	helper[1] = auto_bitmap_ind16_alloc(machine(), cx, TIA_MAX_SCREEN_HEIGHT);
	helper[2] = auto_bitmap_ind16_alloc(machine(), cx, TIA_MAX_SCREEN_HEIGHT);
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

UINT32 tia_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_height = screen.height();
	copybitmap(bitmap, *helper[2], 0, 0, 0, 0, cliprect);
	return 0;
}


void tia_video_device::draw_sprite_helper(UINT8* p, UINT8 *col, struct player_gfx *gfx,
	UINT8 GRP, UINT8 COLUP, UINT8 REFP)
{
	int i;
	int j;
	int k;

	if (REFP & 8)
	{
		GRP = BITSWAP8(GRP, 0, 1, 2, 3, 4, 5, 6, 7);
	}

	for (i = 0; i < PLAYER_GFX_SLOTS; i++)
	{
		int start_pos = gfx->start_drawing[i];
		for (j = gfx->start_pixel[i]; j < 8; j++)
		{
			for (k = 0; k < gfx->size[i]; k++)
			{
				if (GRP & (0x80 >> j))
				{
					if ( start_pos < 160 || ! gfx->skipclip[i] ) {
						p[start_pos % 160] = COLUP >> 1;
						col[start_pos % 160] = COLUP >> 1;
					}
				}
				start_pos++;
			}
		}
	}
}


void tia_video_device::draw_missile_helper(UINT8* p, UINT8* col, int horz, int skipdelay, int latch, int start,
	UINT8 RESMP, UINT8 ENAM, UINT8 NUSIZ, UINT8 COLUM)
{
	int num = nusiz[NUSIZ & 7][0];
	int skp = nusiz[NUSIZ & 7][2];

	int width = 1 << ((NUSIZ >> 4) & 3);

	int i;
	int j;

	for (i = 0; i < num; i++)
	{
		if ( i == 0 )
			horz -= skipdelay;
		if ( i == 1 )
			horz += skipdelay;
		if ( i > 0 || start ) {
			for (j = 0; j < width; j++)
			{
				if ((ENAM & 2) && !(RESMP & 2))
				{
					if ( latch ) {
						switch ( horz % 4 ) {
						case 1:
							if ( horz >= 0 )
							{
								if ( horz < 156 ) {
									p[(horz + 1) % 160] = COLUM >> 1;
									col[(horz + 1) % 160] = COLUM >> 1;
								}
								p[horz % 160] = COLUM >> 1;
								col[horz % 160] = COLUM >> 1;
							}
							break;
						case 2:
						case 3:
							if ( horz >= 0 )
							{
								p[horz % 160] = COLUM >> 1;
								col[horz % 160] = COLUM >> 1;
							}
							break;
						}
					} else {
						if ( horz >= 0 )
						{
							p[horz % 160] = COLUM >> 1;
							col[horz % 160] = COLUM >> 1;
						}
					}
				}

				horz++;
			}
		} else {
			horz+= width;
		}

		horz += 8 * (skp + 1) - width;
	}
}


void tia_video_device::draw_playfield_helper(UINT8* p, UINT8* col, int horz,
	UINT8 COLU, UINT8 REFPF)
{
	UINT32 PF =
		(BITSWAP8(PF0, 0, 1, 2, 3, 4, 5, 6, 7) << 0x10) |
		(BITSWAP8(PF1, 7, 6, 5, 4, 3, 2, 1, 0) << 0x08) |
		(BITSWAP8(PF2, 0, 1, 2, 3, 4, 5, 6, 7) << 0x00);

	int i;
	int j;

	if (REFPF)
	{
		UINT32 swap = 0;

		for (i = 0; i < 20; i++)
		{
			swap <<= 1;

			if (PF & 1)
			{
				swap |= 1;
			}

			PF >>= 1;
		}

		PF = swap;
	}

	for (i = 0; i < 20; i++)
	{
		for (j = 0; j < 4; j++)
		{
			if (PF & (0x80000 >> i))
			{
				p[horz] = COLU >> 1;
				col[horz] = COLU >> 1;
			}

			horz++;
		}
	}
}


void tia_video_device::draw_ball_helper(UINT8* p, UINT8* col, int horz, UINT8 ENAB)
{
	int width = 1 << ((CTRLPF >> 4) & 3);

	int i;

	for (i = 0; i < width; i++)
	{
		if (ENAB & 2)
		{
			p[horz % 160] = COLUPF >> 1;
			col[horz % 160] = COLUPF >> 1;
		}

		horz++;
	}
}


void tia_video_device::drawS0(UINT8* p, UINT8* col)
{
	draw_sprite_helper(p, col, &p0gfx, (VDELP0 & 1) ? prevGRP0 : GRP0, COLUP0, REFP0);
}


void tia_video_device::drawS1(UINT8* p, UINT8* col)
{
	draw_sprite_helper(p, col, &p1gfx, (VDELP1 & 1) ? prevGRP1 : GRP1, COLUP1, REFP1);
}


void tia_video_device::drawM0(UINT8* p, UINT8* col)
{
	draw_missile_helper(p, col, horzM0, skipM0delay, HMM0_latch, startM0, RESMP0, ENAM0, NUSIZ0, COLUP0);
}


void tia_video_device::drawM1(UINT8* p, UINT8* col)
{
	draw_missile_helper(p, col, horzM1, skipM1delay, HMM1_latch, startM1, RESMP1, ENAM1, NUSIZ1, COLUP1);
}


void tia_video_device::drawBL(UINT8* p, UINT8* col)
{
	draw_ball_helper(p, col, horzBL, (VDELBL & 1) ? prevENABL : ENABL);
}


void tia_video_device::drawPF(UINT8* p, UINT8 *col)
{
	draw_playfield_helper(p, col, 0,
		((CTRLPF & 6) == 2) ? COLUP0 : COLUPF, 0);

	draw_playfield_helper(p, col, 80,
		((CTRLPF & 6) == 2) ? COLUP1 : COLUPF, REFLECT);
}


int tia_video_device::collision_check(UINT8* p1, UINT8* p2, int x1, int x2)
{
	int i;

	for (i = x1; i < x2; i++)
	{
		if (p1[i] != 0xFF && p2[i] != 0xFF)
		{
			return 1;
		}
	}

	return 0;
}


int tia_video_device::current_x()
{
	return 3 * ((machine().firstcpu->total_cycles() - frame_cycles) % 76) - 68;
}


int tia_video_device::current_y()
{
	return (machine().firstcpu->total_cycles() - frame_cycles) / 76;
}


void tia_video_device::setup_pXgfx(void)
{
	int i;
	for ( i = 0; i < PLAYER_GFX_SLOTS; i++ )
	{
		if ( i < nusiz[NUSIZ0 & 7][0] && i >= ( startP0 ? 0 : 1 ) )
		{
			p0gfx.size[i] = nusiz[NUSIZ0 & 7][1];
			if ( i )
			{
				p0gfx.start_drawing[i] = ( horzP0 + (p0gfx.size[i] > 1 ? 1 : 0)
											+ i * 8 * ( nusiz[NUSIZ0 & 7][2] + p0gfx.size[i] ) ) % 160;
				p0gfx.skipclip[i] = 0;
			}
			else
			{
				p0gfx.start_drawing[i] = horzP0 + (p0gfx.size[i] > 1 ? 1 : 0 );
				p0gfx.skipclip[i] = skipclipP0;
			}
			p0gfx.start_pixel[i] = 0;
		}
		else
		{
			p0gfx.start_pixel[i] = 8;
		}
		if ( i < nusiz[NUSIZ1 & 7][0] && i >= ( startP1 ? 0 : 1 ) )
		{
			p1gfx.size[i] = nusiz[NUSIZ1 & 7][1];
			if ( i )
			{
				p1gfx.start_drawing[i] = ( horzP1 + (p1gfx.size[i] > 1 ? 1 : 0)
											+ i * 8 * ( nusiz[NUSIZ1 & 7][2] + p1gfx.size[i] ) ) % 160;
				p1gfx.skipclip[i] = 0;
			}
			else
			{
				p1gfx.start_drawing[i] = horzP1 + (p1gfx.size[i] > 1 ? 1 : 0);
				p1gfx.skipclip[i] = skipclipP1;
			}
			p1gfx.start_pixel[i] = 0;
		}
		else
		{
			p1gfx.start_pixel[i] = 8;
		}
	}
}

void tia_video_device::update_bitmap(int next_x, int next_y)
{
	int x;
	int y;

	UINT8 linePF[160];
	UINT8 lineP0[160];
	UINT8 lineP1[160];
	UINT8 lineM0[160];
	UINT8 lineM1[160];
	UINT8 lineBL[160];

	UINT8 temp[160];

	if (prev_y >= next_y && prev_x >= next_x)
	{
		return;
	}

	memset(linePF, 0xFF, sizeof linePF);
	memset(lineP0, 0xFF, sizeof lineP0);
	memset(lineP1, 0xFF, sizeof lineP1);
	memset(lineM0, 0xFF, sizeof lineM0);
	memset(lineM1, 0xFF, sizeof lineM1);
	memset(lineBL, 0xFF, sizeof lineBL);

	if (VBLANK & 2)
	{
		memset(temp, 0, 160);
	}
	else
	{
		memset(temp, COLUBK >> 1, 160);

		if (CTRLPF & 4)
		{
			drawS1(temp, lineP1);
			drawM1(temp, lineM1);
			drawS0(temp, lineP0);
			drawM0(temp, lineM0);
			drawPF(temp, linePF);
			drawBL(temp, lineBL);
		}
		else
		{
			drawPF(temp, linePF);
			drawBL(temp, lineBL);
			drawS1(temp, lineP1);
			drawM1(temp, lineM1);
			drawS0(temp, lineP0);
			drawM0(temp, lineM0);
		}
	}

	for (y = prev_y; y <= next_y; y++)
	{
		UINT16* p;

		int x1 = prev_x;
		int x2 = next_x;
		int colx1;

		/* Check if we have crossed a line boundary */
		if ( y !=  prev_y ) {
			int redraw_line = 0;

			HMOVE_started_previous = HMOVE_INACTIVE;

			if ( HMOVE_started != HMOVE_INACTIVE ) {
				/* Apply pending motion clocks from a HMOVE initiated during the scanline */
				if ( HMOVE_started >= 97 && HMOVE_started < 157 ) {
					horzP0 -= motclkP0;
					horzP1 -= motclkP1;
					horzM0 -= motclkM0;
					horzM1 -= motclkM1;
					horzBL -= motclkBL;
					if (horzP0 < 0)
						horzP0 += 160;
					if (horzP1 < 0)
						horzP1 += 160;
					if (horzM0 < 0)
						horzM0 += 160;
					if (horzM1 < 0)
						horzM1 += 160;
					if (horzBL < 0)
						horzBL += 160;
					HMOVE_started_previous = HMOVE_started;
				}
				HMOVE_started = HMOVE_INACTIVE;
				redraw_line = 1;
			}

			/* Redraw line if the playfield reflect bit was changed after the center of the screen */
			if ( REFLECT != ( CTRLPF & 0x01 ) ) {
				REFLECT = CTRLPF & 0x01;
				redraw_line = 1;
			}

			/* Redraw line if a RESPx or NUSIZx occurred during the last line */
			if ( ! startP0 || ! startP1 || ! startM0 || ! startM1) {
				startP0 = 1;
				startP1 = 1;
				startM0 = 1;
				startM1 = 1;

				redraw_line = 1;
			}

			if ( skipclipP0 ) {
				skipclipP0--;
				redraw_line = 1;
			}

			if ( skipclipP1 ) {
				skipclipP1--;
				redraw_line = 1;
			}

			/* Redraw line if HMP0 latch is still set */
			if ( HMP0_latch ) {
				horzP0 -= 17;
				if ( horzP0 < 0 )
					horzP0 += 160;
				redraw_line = 1;
			}

			/* Redraw line if HMP1 latch is still set */
			if ( HMP1_latch ) {
				horzP1 -= 17;
				if ( horzP1 < 0 )
					horzP1 += 160;
				redraw_line = 1;
			}

			/* Redraw line if HMM0 latch is still set */
			if ( HMM0_latch ) {
				horzM0 -= 17;
				if ( horzM0 < 0 )
					horzM0 += 160;
				redraw_line = 1;
			}

			/* Redraw line if HMM1 latch is still set */
			if ( HMM1_latch ) {
				horzM1 -= 17;
				if ( horzM1 < 0 )
					horzM1 += 160;
				redraw_line = 1;
			}

			/* Redraw line if HMBL latch is still set */
			if ( HMBL_latch ) {
				horzBL -= 17;
				if ( horzBL < 0 )
					horzBL += 160;
				redraw_line = 1;
			}

			/* Redraw line if NUSIZx data was changed */
			if ( NUSIZx_changed ) {
				NUSIZx_changed = 0;
				redraw_line = 1;
			}

			if ( skipM0delay || skipM1delay ) {
				skipM0delay = 0;
				skipM1delay = 0;
				redraw_line = 1;
			}

			if ( redraw_line ) {
				if (VBLANK & 2)
				{
					setup_pXgfx();
					memset(temp, 0, 160);
				}
				else
				{
					memset(linePF, 0xFF, sizeof linePF);
					memset(lineP0, 0xFF, sizeof lineP0);
					memset(lineP1, 0xFF, sizeof lineP1);
					memset(lineM0, 0xFF, sizeof lineM0);
					memset(lineM1, 0xFF, sizeof lineM1);
					memset(lineBL, 0xFF, sizeof lineBL);

					memset(temp, COLUBK >> 1, 160);

					setup_pXgfx();

					if (CTRLPF & 4)
					{
						drawS1(temp, lineP1);
						drawM1(temp, lineM1);
						drawS0(temp, lineP0);
						drawM0(temp, lineM0);
						drawPF(temp, linePF);
						drawBL(temp, lineBL);
					}
					else
					{
						drawPF(temp, linePF);
						drawBL(temp, lineBL);
						drawS1(temp, lineP1);
						drawM1(temp, lineM1);
						drawS0(temp, lineP0);
						drawM0(temp, lineM0);
					}
				}
			}
		}
		if (y != prev_y || x1 < 0)
		{
			x1 = 0;
		}
		if (y != next_y || x2 > 160)
		{
			x2 = 160;
		}

		/* Collision detection also takes place under the extended hblank area */
		colx1 = ( x1 == 8 && HMOVE_started != HMOVE_INACTIVE ) ? 0 : x1;

		if (collision_check(lineM0, lineP1, colx1, x2))
			CXM0P |= 0x80;
		if (collision_check(lineM0, lineP0, colx1, x2))
			CXM0P |= 0x40;
		if (collision_check(lineM1, lineP0, colx1, x2))
			CXM1P |= 0x80;
		if (collision_check(lineM1, lineP1, colx1, x2))
			CXM1P |= 0x40;
		if (collision_check(lineP0, linePF, colx1, x2))
			CXP0FB |= 0x80;
		if (collision_check(lineP0, lineBL, colx1, x2))
			CXP0FB |= 0x40;
		if (collision_check(lineP1, linePF, colx1, x2))
			CXP1FB |= 0x80;
		if (collision_check(lineP1, lineBL, colx1, x2))
			CXP1FB |= 0x40;
		if (collision_check(lineM0, linePF, colx1, x2))
			CXM0FB |= 0x80;
		if (collision_check(lineM0, lineBL, colx1, x2))
			CXM0FB |= 0x40;
		if (collision_check(lineM1, linePF, colx1, x2))
			CXM1FB |= 0x80;
		if (collision_check(lineM1, lineBL, colx1, x2))
			CXM1FB |= 0x40;
		if (collision_check(lineBL, linePF, colx1, x2))
			CXBLPF |= 0x80;
		if (collision_check(lineP0, lineP1, colx1, x2))
			CXPPMM |= 0x80;
		if (collision_check(lineM0, lineM1, colx1, x2))
			CXPPMM |= 0x40;

		p = &helper[current_bitmap]->pix16(y % screen_height, 34);

		for (x = x1; x < x2; x++)
		{
			p[x] = temp[x];
		}

		if ( x2 == 160 && y % screen_height == (screen_height - 1) ) {
			int t_y;
			for ( t_y = 0; t_y < helper[2]->height(); t_y++ ) {
				UINT16* l0 = &helper[current_bitmap]->pix16(t_y);
				UINT16* l1 = &helper[1 - current_bitmap]->pix16(t_y);
				UINT16* l2 = &helper[2]->pix16(t_y);
				int t_x;
				for( t_x = 0; t_x < helper[2]->width(); t_x++ ) {
					if ( l0[t_x] != l1[t_x] ) {
						/* Combine both entries */
						l2[t_x] = ( ( l0[t_x] + 1 ) << 7 ) | l1[t_x];
					} else {
						l2[t_x] = l0[t_x];
					}
				}
			}
			current_bitmap ^= 1;
		}
	}

	prev_x = next_x;
	prev_y = next_y;
}


WRITE8_MEMBER( tia_video_device::WSYNC_w )
{
	int cycles = machine().firstcpu->total_cycles() - frame_cycles;

	if (cycles % 76)
	{
		space.device().execute().adjust_icount(cycles % 76 - 76);
	}
}


WRITE8_MEMBER( tia_video_device::VSYNC_w )
{
	if (data & 2)
	{
		if (!(VSYNC & 2))
		{
			int curr_y = current_y();

			if ( curr_y > 5 )
				update_bitmap(
					machine().primary_screen->width(),
					machine().primary_screen->height());

			if ( !m_vsync_callback_func.isnull() ) {
				m_vsync_callback_func(0, curr_y, 0xFFFF );
			}

			prev_y = 0;
			prev_x = 0;

			frame_cycles += 76 * current_y();
		}
	}

	VSYNC = data;
}


WRITE8_MEMBER( tia_video_device::VBLANK_w )
{
	if (data & 0x80)
	{
		paddle_cycles = machine().firstcpu->total_cycles();
	}
	if ( ! ( VBLANK & 0x40 ) ) {
		INPT4 = 0x80;
		INPT5 = 0x80;
	}
	VBLANK = data;
}


WRITE8_MEMBER( tia_video_device::CTRLPF_w )
{
	int curr_x = current_x();

	CTRLPF = data;
	if ( curr_x < 80 ) {
		REFLECT = CTRLPF & 1;
	}
}

WRITE8_MEMBER( tia_video_device::HMP0_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMP0 )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 6 + motclkP0 * 4, 7 ) ) {
		int new_motclkP0 = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkP0 > motclkP0 || curr_x <= MIN( HMOVE_started + 6 + new_motclkP0 * 4, 7 ) ) {
			horzP0 -= ( new_motclkP0 - motclkP0 );
			motclkP0 = new_motclkP0;
		} else {
			horzP0 -= ( 15 - motclkP0 );
			motclkP0 = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMP0_latch = 1;
			}
		}
		if ( horzP0 < 0 )
			horzP0 += 160;
		horzP0 %= 160;
		setup_pXgfx();
	}
	HMP0 = data;
}

WRITE8_MEMBER( tia_video_device::HMP1_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMP1 )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 6 + motclkP1 * 4, 7 ) ) {
		int new_motclkP1 = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkP1 > motclkP1 || curr_x <= MIN( HMOVE_started + 6 + new_motclkP1 * 4, 7 ) ) {
			horzP1 -= ( new_motclkP1 - motclkP1 );
			motclkP1 = new_motclkP1;
		} else {
			horzP1 -= ( 15 - motclkP1 );
			motclkP1 = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMP1_latch = 1;
			}
		}
		if ( horzP1 < 0 )
			horzP1 += 160;
		horzP1 %= 160;
		setup_pXgfx();
	}
	HMP1 = data;
}

WRITE8_MEMBER( tia_video_device::HMM0_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMM0 )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 6 + motclkM0 * 4, 7 ) ) {
		int new_motclkM0 = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkM0 > motclkM0 || curr_x <= MIN( HMOVE_started + 6 + new_motclkM0 * 4, 7 ) ) {
			horzM0 -= ( new_motclkM0 - motclkM0 );
			motclkM0 = new_motclkM0;
		} else {
			horzM0 -= ( 15 - motclkM0 );
			motclkM0 = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMM0_latch = 1;
			}
		}
		if ( horzM0 < 0 )
			horzM0 += 160;
		horzM0 %= 160;
	}
	HMM0 = data;
}

WRITE8_MEMBER( tia_video_device::HMM1_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMM1 )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 6 + motclkM1 * 4, 7 ) ) {
		int new_motclkM1 = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkM1 > motclkM1 || curr_x <= MIN( HMOVE_started + 6 + new_motclkM1 * 4, 7 ) ) {
			horzM1 -= ( new_motclkM1 - motclkM1 );
			motclkM1 = new_motclkM1;
		} else {
			horzM1 -= ( 15 - motclkM1 );
			motclkM1 = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMM1_latch = 1;
			}
		}
		if ( horzM1 < 0 )
			horzM1 += 160;
		horzM1 %= 160;
	}
	HMM1 = data;
}

WRITE8_MEMBER( tia_video_device::HMBL_w )
{
	int curr_x = current_x();

	data &= 0xF0;

	if ( data == HMBL )
		return;

	/* Check if HMOVE cycles are still being applied */
	if ( HMOVE_started != HMOVE_INACTIVE && curr_x < MIN( HMOVE_started + 6 + motclkBL * 4, 7 ) ) {
		int new_motclkBL = ( data ^ 0x80 ) >> 4;

		/* Check if new horizontal move can still be applied normally */
		if ( new_motclkBL > motclkBL || curr_x <= MIN( HMOVE_started + 6 + new_motclkBL * 4, 7 ) ) {
			horzBL -= ( new_motclkBL - motclkBL );
			motclkBL = new_motclkBL;
		} else {
			horzBL -= ( 15 - motclkBL );
			motclkBL = 15;
			if ( data != 0x70 && data != 0x80 ) {
				HMBL_latch = 1;
			}
		}
		if ( horzBL < 0 )
			horzBL += 160;
		horzBL %= 160;
	}
	HMBL = data;
}

WRITE8_MEMBER( tia_video_device::HMOVE_w )
{
	int curr_x = current_x();
	int curr_y = current_y();

	HMOVE_started = curr_x;

	/* Check if we have to undo some of the already applied cycles from an active graphics latch */
	if ( curr_x + 68 < 17 * 4 ) {
		int cycle_fix = 17 - ( ( curr_x + 68 + 7 ) / 4 );
		if ( HMP0_latch )
			horzP0 = ( horzP0 + cycle_fix ) % 160;
		if ( HMP1_latch )
			horzP1 = ( horzP1 + cycle_fix ) % 160;
		if ( HMM0_latch )
			horzM0 = ( horzM0 + cycle_fix ) % 160;
		if ( HMM1_latch )
			horzM1 = ( horzM1 + cycle_fix ) % 160;
		if ( HMBL_latch )
			horzBL = ( horzBL + cycle_fix ) % 160;
	}

	HMP0_latch = 0;
	HMP1_latch = 0;
	HMM0_latch = 0;
	HMM1_latch = 0;
	HMBL_latch = 0;

	/* Check if HMOVE activities can be ignored */
	if ( curr_x >= -5 && curr_x < 97 ) {
		motclkP0 = 0;
		motclkP1 = 0;
		motclkM0 = 0;
		motclkM1 = 0;
		motclkBL = 0;
		HMOVE_started = HMOVE_INACTIVE;
		return;
	}

	motclkP0 = ( HMP0 ^ 0x80 ) >> 4;
	motclkP1 = ( HMP1 ^ 0x80 ) >> 4;
	motclkM0 = ( HMM0 ^ 0x80 ) >> 4;
	motclkM1 = ( HMM1 ^ 0x80 ) >> 4;
	motclkBL = ( HMBL ^ 0x80 ) >> 4;

	/* Adjust number of graphics motion clocks for active display */
	if ( curr_x >= 97 && curr_x < 151 ) {
		int skip_motclks = ( 160 - HMOVE_started - 6 ) / 4;
		motclkP0 -= skip_motclks;
		motclkP1 -= skip_motclks;
		motclkM0 -= skip_motclks;
		motclkM1 -= skip_motclks;
		motclkBL -= skip_motclks;
		if ( motclkP0 < 0 )
			motclkP0 = 0;
		if ( motclkP1 < 0 )
			motclkP1 = 0;
		if ( motclkM0 < 0 )
			motclkM0 = 0;
		if ( motclkM1 < 0 )
			motclkM1 = 0;
		if ( motclkBL < 0 )
			motclkBL = 0;
	}

	if ( curr_x >= -56 && curr_x < -5 ) {
		int max_motclks = ( 7 - ( HMOVE_started + 5 ) ) / 4;
		if ( motclkP0 > max_motclks )
			motclkP0 = max_motclks;
		if ( motclkP1 > max_motclks )
			motclkP1 = max_motclks;
		if ( motclkM0 > max_motclks )
			motclkM0 = max_motclks;
		if ( motclkM1 > max_motclks )
			motclkM1 = max_motclks;
		if ( motclkBL > max_motclks )
			motclkBL = max_motclks;
	}

	/* Apply horizontal motion */
	if ( curr_x < -5 || curr_x >= 157 ) {
		horzP0 += 8 - motclkP0;
		horzP1 += 8 - motclkP1;
		horzM0 += 8 - motclkM0;
		horzM1 += 8 - motclkM1;
		horzBL += 8 - motclkBL;

		if (horzP0 < 0)
			horzP0 += 160;
		if (horzP1 < 0)
			horzP1 += 160;
		if (horzM0 < 0)
			horzM0 += 160;
		if (horzM1 < 0)
			horzM1 += 160;
		if (horzBL < 0)
			horzBL += 160;

		horzP0 %= 160;
		horzP1 %= 160;
		horzM0 %= 160;
		horzM1 %= 160;
		horzBL %= 160;

		/* When HMOVE is triggered on CPU cycle 75, the HBlank period on the
		   next line is also extended. */
		if (curr_x >= 157)
		{
			curr_y += 1;
			update_bitmap( -8, curr_y );
		}
		else
		{
			setup_pXgfx();
		}
		if (curr_y < screen_height)
		{
			memset(&helper[current_bitmap]->pix16(curr_y, 34), 0, 16);
		}

		prev_x = 8;
	}
}


WRITE8_MEMBER( tia_video_device::RSYNC_w )
{
	/* this address is used in chip testing */
}


WRITE8_MEMBER( tia_video_device::NUSIZ0_w )
{
	int curr_x = current_x();

	/* Check if relevant bits have changed */
	if ( ( data & 7 ) != ( NUSIZ0 & 7 ) ) {
		int i;
		/* Check if we are (about to start) drawing a copy of the player 0 graphics */
		for ( i = 0; i < PLAYER_GFX_SLOTS; i++ ) {
			if ( p0gfx.start_pixel[i] < 8 ) {
				int min_x = p0gfx.start_drawing[i];
				int size = ( 8 - p0gfx.start_pixel[i] ) * p0gfx.size[i];
				if ( curr_x >= ( min_x - 5 ) % 160 && curr_x < ( min_x + size ) % 160 ) {
					if ( curr_x >= min_x % 160 || p0gfx.start_pixel[i] != 0 ) {
						/* This copy has started drawing */
						if ( p0gfx.size[i] == 1 && nusiz[data & 7][1] > 1 ) {
							int delay = 1 + ( ( p0gfx.start_pixel[i] + ( curr_x - p0gfx.start_drawing[i] ) ) & 1 );
							update_bitmap( curr_x + delay, current_y() );
							p0gfx.start_pixel[i] += ( curr_x + delay - p0gfx.start_drawing[i] );
							if ( p0gfx.start_pixel[i] > 8 )
								p0gfx.start_pixel[i] = 8;
							p0gfx.start_drawing[i] = curr_x + delay;
						} else if ( p0gfx.size[1] > 1 && nusiz[data & 7][1] == 1 ) {
							int delay = ( curr_x - p0gfx.start_drawing[i] ) & ( p0gfx.size[i] - 1 );
							if ( delay ) {
								delay = p0gfx.size[i] - delay;
							}
							update_bitmap( curr_x + delay, current_y() );
							p0gfx.start_pixel[i] += ( curr_x - p0gfx.start_drawing[i] ) / p0gfx.size[i];
							p0gfx.start_drawing[i] = curr_x + delay;
						} else {
							p0gfx.start_pixel[i] += ( curr_x - p0gfx.start_drawing[i] ) / p0gfx.size[i];
							p0gfx.start_drawing[i] = curr_x;
						}
						p0gfx.size[i] = nusiz[data & 7][1];
					} else {
						/* This copy was just about to start drawing (meltdown) */
						/* Adjust for 1 clock delay between zoomed and non-zoomed sprites */
						if ( p0gfx.size[i] == 1 && nusiz[data & 7][1] > 1 ) {
							/* Check for hardware oddity */
							if ( p0gfx.start_drawing[i] - curr_x == 2 ) {
								p0gfx.start_drawing[i]--;
							} else {
								p0gfx.start_drawing[i]++;
							}
						} else if ( p0gfx.size[i] > 1 && nusiz[data & 7][1] == 1 ) {
							p0gfx.start_drawing[i]--;
						}
						p0gfx.size[i] = nusiz[data & 7][1];
					}
				} else {
					/* We are passed the copy or the copy still needs to be done. Mark
					   it as done/invalid, the data will be reset in the next loop. */
					p0gfx.start_pixel[i] = 8;
				}
			}
		}
		/* Apply NUSIZ updates to not yet drawn copies */
		for ( i = ( startP0 ? 0 : 1 ); i < nusiz[data & 7][0]; i++ ) {
			int j;
			/* Find an unused p0gfx entry */
			for ( j = 0; j < PLAYER_GFX_SLOTS; j++ ) {
				if ( p0gfx.start_pixel[j] == 8 )
					break;
			}
			p0gfx.size[j] = nusiz[data & 7][1];
			p0gfx.start_drawing[j] = ( horzP0 + (p0gfx.size[j] > 1 ? 1 : 0)
									+ i * 8 * ( nusiz[data & 7][2] + p0gfx.size[j] ) ) % 160;
			if ( curr_x < p0gfx.start_drawing[j] % 160 ) {
				p0gfx.start_pixel[j] = 0;
			}
		}
		NUSIZx_changed = 1;
	}
	NUSIZ0 = data;
}


WRITE8_MEMBER( tia_video_device::NUSIZ1_w )
{
	int curr_x = current_x();

	/* Check if relevant bits have changed */
	if ( ( data & 7 ) != ( NUSIZ1 & 7 ) ) {
		int i;
		/* Check if we are (about to start) drawing a copy of the player 1 graphics */
		for ( i = 0; i < PLAYER_GFX_SLOTS; i++ ) {
			if ( p1gfx.start_pixel[i] < 8 ) {
				int min_x = p1gfx.start_drawing[i];
				int size = ( 8 - p1gfx.start_pixel[i] ) * p1gfx.size[i];
				if ( curr_x >= ( min_x - 5 ) % 160 && curr_x < ( min_x + size ) % 160 ) {
					if ( curr_x >= min_x % 160 || p1gfx.start_pixel[i] != 0 ) {
						/* This copy has started drawing */
						if ( p1gfx.size[i] == 1 && nusiz[data & 7][1] > 1 ) {
							int delay = 1 + ( ( p0gfx.start_pixel[i] + ( curr_x - p0gfx.start_drawing[i] ) ) & 1 );
							update_bitmap( curr_x + delay, current_y() );
							p1gfx.start_pixel[i] += ( curr_x + delay - p1gfx.start_drawing[i] );
							if ( p1gfx.start_pixel[i] > 8 )
								p1gfx.start_pixel[i] = 8;
							p1gfx.start_drawing[i] = curr_x + delay;
						} else if ( p1gfx.size[1] > 1 && nusiz[data & 7][1] == 1 ) {
							int delay = ( curr_x - p1gfx.start_drawing[i] ) & ( p1gfx.size[i] - 1 );
							if ( delay ) {
								delay = p1gfx.size[i] - delay;
							}
							update_bitmap( curr_x + delay, current_y() );
							p1gfx.start_pixel[i] += ( curr_x - p1gfx.start_drawing[i] ) / p1gfx.size[i];
							p1gfx.start_drawing[i] = curr_x + delay;
						} else {
							p1gfx.start_pixel[i] += ( curr_x - p1gfx.start_drawing[i] ) / p1gfx.size[i];
							p1gfx.start_drawing[i] = curr_x;
						}
						p1gfx.size[i] = nusiz[data & 7][1];
					} else {
						/* This copy was just about to start drawing (meltdown) */
						/* Adjust for 1 clock delay between zoomed and non-zoomed sprites */
						if ( p1gfx.size[i] == 1 && nusiz[data & 7][1] > 1 ) {
							/* Check for hardware oddity */
							if ( p1gfx.start_drawing[i] - curr_x == 2 ) {
								p1gfx.start_drawing[i]--;
							} else {
								p1gfx.start_drawing[i]++;
							}
						} else if ( p1gfx.size[i] > 1 && nusiz[data & 7][1] == 1 ) {
							p1gfx.start_drawing[i]--;
						}
						p1gfx.size[i] = nusiz[data & 7][1];
					}
				} else {
					/* We are passed the copy or the copy still needs to be done. Mark
					   it as done/invalid, the data will be reset in the next loop. */
					p1gfx.start_pixel[i] = 8;
				}
			}
		}
		/* Apply NUSIZ updates to not yet drawn copies */
		for ( i = ( startP1 ? 0 : 1 ); i < nusiz[data & 7][0]; i++ ) {
			int j;
			/* Find an unused p1gfx entry */
			for ( j = 0; j < PLAYER_GFX_SLOTS; j++ ) {
				if ( p1gfx.start_pixel[j] == 8 )
					break;
			}
			p1gfx.size[j] = nusiz[data & 7][1];
			p1gfx.start_drawing[j] = ( horzP1 + (p1gfx.size[j] > 1 ? 1 : 0)
									+ i * 8 * ( nusiz[data & 7][2] + p1gfx.size[j] ) ) % 160;
			if ( curr_x < p1gfx.start_drawing[j] % 160 ) {
				p1gfx.start_pixel[j] = 0;
			}
		}
		NUSIZx_changed = 1;
	}
	NUSIZ1 = data;
}


WRITE8_MEMBER( tia_video_device::HMCLR_w )
{
	HMP0_w( space, offset, 0 );
	HMP1_w( space, offset, 0 );
	HMM0_w( space, offset, 0 );
	HMM1_w( space, offset, 0 );
	HMBL_w( space, offset, 0 );
}


WRITE8_MEMBER( tia_video_device::CXCLR_w )
{
	CXM0P = 0;
	CXM1P = 0;
	CXP0FB = 0;
	CXP1FB = 0;
	CXM0FB = 0;
	CXM1FB = 0;
	CXBLPF = 0;
	CXPPMM = 0;
}


#define RESXX_APPLY_ACTIVE_HMOVE(HORZ,MOTION,MOTCLK)                                    \
	if ( curr_x < MIN( HMOVE_started + 6 + 16 * 4, 7 ) ) {                              \
		int decrements_passed = ( curr_x - ( HMOVE_started + 4 ) ) / 4;                 \
		HORZ += 8;                                                                      \
		if ( ( MOTCLK - decrements_passed ) > 0 ) {                                     \
			HORZ -= ( MOTCLK - decrements_passed );                                     \
			if ( HORZ < 0 )                                                             \
				HORZ += 160;                                                            \
		}                                                                               \
	}

#define RESXX_APPLY_PREVIOUS_HMOVE(HORZ,MOTION)                                             \
	if ( HMOVE_started_previous != HMOVE_INACTIVE )                                         \
	{                                                                                       \
		UINT8   motclk = ( MOTION ^ 0x80 ) >> 4;                                            \
		if ( curr_x <= HMOVE_started_previous - 228 + 5 + motclk * 4 )                      \
		{                                                                                   \
			UINT8   motclk_passed = ( curr_x - ( HMOVE_started_previous - 228 + 6 ) ) / 4;  \
			HORZ -= ( motclk - motclk_passed );                                             \
		}                                                                                   \
	}

WRITE8_MEMBER( tia_video_device::RESP0_w )
{
	int curr_x = current_x();
	int new_horzP0;

	/* Check if HMOVE is activated during this line */
	if ( HMOVE_started != HMOVE_INACTIVE ) {
		new_horzP0 = ( curr_x < 7 ) ? 3 : ( curr_x + 5 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( new_horzP0, HMP0, motclkP0 );
	} else {
		new_horzP0 = ( curr_x < -2 ) ? 3 : ( curr_x + 5 );
		RESXX_APPLY_PREVIOUS_HMOVE( new_horzP0, HMP0 );
	}

	if ( new_horzP0 != horzP0 ) {
		int i;
		horzP0 = new_horzP0;
		startP0 = 0;
		skipclipP0 = 2;
		/* Check if we are (about to start) drawing a copy of the player 0 graphics */
		for ( i = 0; i < PLAYER_GFX_SLOTS; i++ ) {
			if ( p0gfx.start_pixel[i] < 8 ) {
				int min_x = p0gfx.start_drawing[i];
				int size = ( 8 - p0gfx.start_pixel[i] ) * p0gfx.size[i];
				if ( curr_x >= ( min_x - 5 ) % 160 && curr_x < ( min_x + size ) % 160 ) {
					if ( curr_x >= min_x ) {
						/* This copy has started drawing */
						p0gfx.start_pixel[i] += ( curr_x - p0gfx.start_drawing[i] ) / p0gfx.size[i];
						p0gfx.start_drawing[i] = curr_x;
					} else {
						/* This copy is waiting to start drawing */
						p0gfx.start_drawing[i] = horzP0;
					}
				} else {
					/* We are passed the copy or the copy still needs to be done. Mark
					   it as done/invalid, the data will be reset in the next loop. */
					p0gfx.start_pixel[i] = 8;
				}
			}
		}
		/* Apply NUSIZ and position updates to not yet drawn copies */
		for ( i = 1; i < nusiz[NUSIZ0 & 7][0]; i++ ) {
			int j;
			/* Find an unused p0gfx entry */
			for ( j = 0; j < PLAYER_GFX_SLOTS; j++ ) {
				if ( p0gfx.start_pixel[j] == 8 )
					break;
			}
			p0gfx.size[j] = nusiz[NUSIZ0 & 7][1];
			p0gfx.start_drawing[j] = ( horzP0 + (p0gfx.size[j] > 1 ? 1 : 0)
									+ i * 8 * ( nusiz[NUSIZ0 & 7][2] + p0gfx.size[j] ) ) % 160;
			if ( curr_x < p0gfx.start_drawing[j] % 160 ) {
				p0gfx.start_pixel[j] = 0;
			}
		}
	}
}


WRITE8_MEMBER( tia_video_device::RESP1_w )
{
	int curr_x = current_x();
	int new_horzP1;

	/* Check if HMOVE is activated during this line */
	if ( HMOVE_started != HMOVE_INACTIVE ) {
		new_horzP1 = ( curr_x < 7 ) ? 3 : ( curr_x + 5 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( new_horzP1, HMP1, motclkP1 );
	} else {
		new_horzP1 = ( curr_x < -2 ) ? 3 : ( curr_x + 5 );
		RESXX_APPLY_PREVIOUS_HMOVE( new_horzP1, HMP1 );
	}

	if ( new_horzP1 != horzP1 ) {
		int i;
		horzP1 = new_horzP1;
		startP1 = 0;
		skipclipP1 = 2;
		/* Check if we are (about to start) drawing a copy of the player 1 graphics */
		for ( i = 0; i < PLAYER_GFX_SLOTS; i++ ) {
			if ( p1gfx.start_pixel[i] < 8 ) {
				int min_x = p1gfx.start_drawing[i];
				int size = ( 8 - p1gfx.start_pixel[i] ) * p1gfx.size[i];
				if ( curr_x >= ( min_x - 5 ) % 160 && curr_x < ( min_x + size ) % 160 ) {
					if ( curr_x >= min_x ) {
						/* This copy has started drawing */
						p1gfx.start_pixel[i] += ( curr_x - p1gfx.start_drawing[i] ) / p1gfx.size[i];
						p1gfx.start_drawing[i] = curr_x;
					} else {
						/* This copy is waiting to start drawing */
						p1gfx.start_drawing[i] = horzP1;
					}
				} else {
					/* We are passed the copy or the copy still needs to be done. Mark
					   it as done/invalid, the data will be reset in the next loop. */
					p1gfx.start_pixel[i] = 8;
				}
			}
		}
		/* Apply NUSIZ and position updates to not yet drawn copies */
		for ( i = 1; i < nusiz[NUSIZ1 & 7][0]; i++ ) {
			int j;
			/* Find an unused p1gfx entry */
			for ( j = 0; j < PLAYER_GFX_SLOTS; j++ ) {
				if ( p1gfx.start_pixel[j] == 8 )
					break;
			}
			p1gfx.size[j] = nusiz[NUSIZ1 & 7][1];
			p1gfx.start_drawing[j] = ( horzP1 + (p1gfx.size[j] > 1 ? 1 : 0)
									+ i * 8 * ( nusiz[NUSIZ1 & 7][2] + p1gfx.size[j] ) ) % 160;
			if ( curr_x < p1gfx.start_drawing[j] % 160 ) {
				p1gfx.start_pixel[j] = 0;
			}
		}
	}
}


WRITE8_MEMBER( tia_video_device::RESM0_w )
{
	int curr_x = current_x();
	int new_horzM0;

	/* Check if HMOVE is activated during this line */
	if ( HMOVE_started != HMOVE_INACTIVE ) {
		new_horzM0 = ( curr_x < 7 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( new_horzM0, HMM0, motclkM0 );
	} else {
		new_horzM0 = ( curr_x < -1 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		skipM0delay = ( curr_x < -1 && horzM0 % 160 >= 0 && horzM0 % 160 < 1 ) ? 4 : 0;
		RESXX_APPLY_PREVIOUS_HMOVE( new_horzM0, HMM0 );
	}
	if ( new_horzM0 != horzM0 ) {
		startM0 = skipM0delay ? 1 : 0;
		horzM0 = new_horzM0;
	}
}


WRITE8_MEMBER( tia_video_device::RESM1_w )
{
	int curr_x = current_x();
	int new_horzM1;

	/* Check if HMOVE is activated during this line */
	if ( HMOVE_started != HMOVE_INACTIVE ) {
		new_horzM1 = ( curr_x < 7 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( new_horzM1, HMM1, motclkM1 );
	} else {
		new_horzM1 = ( curr_x < -1 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		skipM1delay = ( curr_x < -1 && horzM1 % 160 >= 0 && horzM1 % 160 < 1 ) ? 4 : 0;
		RESXX_APPLY_PREVIOUS_HMOVE( new_horzM1, HMM1 );
	}
	if ( new_horzM1 != horzM1 ){
		startM1 = skipM1delay ? 1 : 0;
		horzM1 = new_horzM1;
	}
}


WRITE8_MEMBER( tia_video_device::RESBL_w )
{
	int curr_x = current_x();

	/* Check if HMOVE is activated during this line */
	if ( HMOVE_started != HMOVE_INACTIVE ) {
		horzBL = ( curr_x < 7 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		/* If HMOVE is active, adjust for remaining horizontal move clocks if any */
		RESXX_APPLY_ACTIVE_HMOVE( horzBL, HMBL, motclkBL );
	} else {
		horzBL = ( curr_x < 0 ) ? 2 : ( ( curr_x + 4 ) % 160 );
		RESXX_APPLY_PREVIOUS_HMOVE( horzBL, HMBL );
	}
}


WRITE8_MEMBER( tia_video_device::RESMP0_w )
{
	if (RESMP0 & 2)
	{
		if ( nusiz[NUSIZ0 & 7][1] > 1 ) {
			horzM0 = horzP0 + 3 * nusiz[NUSIZ0 & 7][1] - 1;
		} else {
			horzM0 = horzP0 + 4 * nusiz[NUSIZ0 & 7][1];
		}
		if ( HMOVE_started != HMOVE_INACTIVE ) {
			horzM0 -= ( 8 - motclkP0 );
			horzM0 += 8 - motclkM0;
			if ( horzM0 < 0 )
				horzM0 += 160;
		}
		horzM0 %= 160;
	}

	RESMP0 = data;
}


WRITE8_MEMBER( tia_video_device::RESMP1_w )
{
	if (RESMP1 & 2)
	{
		if ( nusiz[NUSIZ1 & 7][1] > 1 ) {
			horzM1 = horzP1 + 3 * nusiz[NUSIZ1 & 7][1] - 1;
		} else {
			horzM1 = horzP1 + 4 * nusiz[NUSIZ1 & 7][1];
		}
		if ( HMOVE_started != HMOVE_INACTIVE ) {
			horzM1 -= ( 8 - motclkP1 );
			horzM1 += 8 - motclkM1;
			if ( horzM1 < 0 )
				horzM1 += 160;
		}
		horzM1 %= 160;
	}

	RESMP1 = data;
}


WRITE8_MEMBER( tia_video_device::GRP0_w )
{
	prevGRP1 = GRP1;

	GRP0 = data;
}


WRITE8_MEMBER( tia_video_device::GRP1_w )
{
	prevGRP0 = GRP0;

	GRP1 = data;

	prevENABL = ENABL;
}


READ8_MEMBER( tia_video_device::INPT_r )
{
	UINT64 elapsed = machine().firstcpu->total_cycles() - paddle_cycles;
	int input = TIA_INPUT_PORT_ALWAYS_ON;
	if ( !m_read_input_port_func.isnull() )
	{
		input = m_read_input_port_func(offset & 3, 0xFFFF);
	}

	if ( input == TIA_INPUT_PORT_ALWAYS_ON )
		return 0x80;
	if ( input == TIA_INPUT_PORT_ALWAYS_OFF )
		return 0x00;

	return elapsed > 76 * input ? 0x80 : 0x00;
}


READ8_MEMBER( tia_video_device::read )
{
		/* lower bits 0 - 5 seem to depend on the last byte on the
		 data bus. If the driver supplied a routine to retrieve
		 that we will call that, otherwise we will use the lower
		 bit of the offset.
	*/
	UINT8 data = offset & 0x3f;

	if ( !m_databus_contents_func.isnull() )
	{
		data = m_databus_contents_func(offset) & 0x3f;
	}

	if (!(offset & 0x8))
	{
		update_bitmap(current_x(), current_y());
	}

	switch (offset & 0xF)
	{
	case 0x0:
		return data | CXM0P;
	case 0x1:
		return data | CXM1P;
	case 0x2:
		return data | CXP0FB;
	case 0x3:
		return data | CXP1FB;
	case 0x4:
		return data | CXM0FB;
	case 0x5:
		return data | CXM1FB;
	case 0x6:
		return data | CXBLPF;
	case 0x7:
		return data | CXPPMM;
	case 0x8:
		return data | INPT_r(space,0);
	case 0x9:
		return data | INPT_r(space,1);
	case 0xA:
		return data | INPT_r(space,2);
	case 0xB:
		return data | INPT_r(space,3);
	case 0xC:
		{
			int button = !m_read_input_port_func.isnull() ? ( m_read_input_port_func(4,0xFFFF) & 0x80 ) : 0x80;
			INPT4 = ( VBLANK & 0x40) ? ( INPT4 & button ) : button;
		}
		return data | INPT4;
	case 0xD:
		{
			int button = !m_read_input_port_func.isnull() ? ( m_read_input_port_func(5,0xFFFF) & 0x80 ) : 0x80;
			INPT5 = ( VBLANK & 0x40) ? ( INPT5 & button ) : button;
		}
		return data | INPT5;
	}

	return data;
}


WRITE8_MEMBER( tia_video_device::write )
{
	static const int delay[0x40] =
	{
			0,  // VSYNC
			0,  // VBLANK
			0,  // WSYNC
			0,  // RSYNC
			0,  // NUSIZ0
			0,  // NUSIZ1
			0,  // COLUP0
			0,  // COLUP1
			0,  // COLUPF
			0,  // COLUBK
			0,  // CTRLPF
			1,  // REFP0
			1,  // REFP1
			4,  // PF0
			4,  // PF1
			4,  // PF2
			0,  // RESP0
			0,  // RESP1
			0,  // RESM0
			0,  // RESM1
			0,  // RESBL
		-1, // AUDC0
		-1, // AUDC1
		-1, // AUDF0
		-1, // AUDF1
		-1, // AUDV0
		-1, // AUDV1
			1,  // GRP0
			1,  // GRP1
			1,  // ENAM0
			1,  // ENAM1
			1,  // ENABL
			0,  // HMP0
			0,  // HMP1
			0,  // HMM0
			0,  // HMM1
			0,  // HMBL
			0,  // VDELP0
			0,  // VDELP1
			0,  // VDELBL
			0,  // RESMP0
			0,  // RESMP1
			3,  // HMOVE
			0,  // HMCLR
			0,  // CXCLR
	};
	int curr_x = current_x();
	int curr_y = current_y();

	offset &= 0x3F;

	if (offset >= 0x0D && offset <= 0x0F)
	{
		curr_x = ( curr_x + 1 ) & ~3;
	}

	if (delay[offset] >= 0)
	{
		update_bitmap(curr_x + delay[offset], curr_y);
	}

	switch (offset)
	{
	case 0x00:
		VSYNC_w(space, offset, data);
		break;
	case 0x01:
		VBLANK_w(space, offset, data);
		break;
	case 0x02:
		WSYNC_w(space, offset, data);
		break;
	case 0x03:
		RSYNC_w(space, offset, data);
		break;
	case 0x04:
		NUSIZ0_w(space, offset, data);
		break;
	case 0x05:
		NUSIZ1_w(space, offset, data);
		break;
	case 0x06:
		COLUP0 = data;
		break;
	case 0x07:
		COLUP1 = data;
		break;
	case 0x08:
		COLUPF = data;
		break;
	case 0x09:
		COLUBK = data;
		break;
	case 0x0A:
		CTRLPF_w(space, offset, data);
		break;
	case 0x0B:
		REFP0 = data;
		break;
	case 0x0C:
		REFP1 = data;
		break;
	case 0x0D:
		PF0 = data;
		break;
	case 0x0E:
		PF1 = data;
		break;
	case 0x0F:
		PF2 = data;
		break;
	case 0x10:
		RESP0_w(space, offset, data);
		break;
	case 0x11:
		RESP1_w(space, offset, data);
		break;
	case 0x12:
		RESM0_w(space, offset, data);
		break;
	case 0x13:
		RESM1_w(space, offset, data);
		break;
	case 0x14:
		RESBL_w(space, offset, data);
		break;

	case 0x15: /* AUDC0 */
	case 0x16: /* AUDC1 */
	case 0x17: /* AUDF0 */
	case 0x18: /* AUDF1 */
	case 0x19: /* AUDV0 */
	case 0x1A: /* AUDV1 */
		tia_sound_w(machine().device("tia"), space, offset, data);
		break;

	case 0x1B:
		GRP0_w(space, offset, data);
		break;
	case 0x1C:
		GRP1_w(space, offset, data);
		break;
	case 0x1D:
		ENAM0 = data;
		break;
	case 0x1E:
		ENAM1 = data;
		break;
	case 0x1F:
		ENABL = data;
		break;
	case 0x20:
		HMP0_w(space, offset, data);
		break;
	case 0x21:
		HMP1_w(space, offset, data);
		break;
	case 0x22:
		HMM0_w(space, offset, data);
		break;
	case 0x23:
		HMM1_w(space, offset, data);
		break;
	case 0x24:
		HMBL_w(space, offset, data);
		break;
	case 0x25:
		VDELP0 = data;
		break;
	case 0x26:
		VDELP1 = data;
		break;
	case 0x27:
		VDELBL = data;
		break;
	case 0x28:
		RESMP0_w(space, offset, data);
		break;
	case 0x29:
		RESMP1_w(space, offset, data);
		break;
	case 0x2A:
		HMOVE_w(space, offset, data);
		break;
	case 0x2B:
		HMCLR_w(space, offset, data);
		break;
	case 0x2C:
		CXCLR_w(space, offset, 0);
		break;
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tia_video_device::device_reset()
{
	int i;

	frame_cycles = 0;

	INPT4 = 0x80;
	INPT5 = 0x80;

	HMP0 = 0;
	HMP1 = 0;
	HMM0 = 0;
	HMM1 = 0;
	HMBL = 0;

	startP0 = 1;
	startP1 = 1;

	HMP0_latch = 0;
	HMP1_latch = 0;
	HMM0_latch = 0;
	HMM1_latch = 0;
	HMBL_latch = 0;

	startM0 = 1;
	startM1 = 1;
	skipM0delay = 0;
	skipM1delay = 0;

	REFLECT = 0;

	prev_x = 0;
	prev_y = 0;

	HMOVE_started = HMOVE_INACTIVE;

	motclkP0 = 0;
	motclkP1 = 0;
	motclkM0 = 0;
	motclkM1 = 0;
	motclkBL = 0;

	horzP0 = 0;
	horzP1 = 0;

	for( i = 0; i < PLAYER_GFX_SLOTS; i++ )
	{
		p0gfx.start_pixel[i] = 8;
		p0gfx.size[i] = 1;
		p1gfx.start_pixel[i] = 8;
		p1gfx.size[i] = 1;
	}

	current_bitmap = 0;

	NUSIZx_changed = 0;
}
