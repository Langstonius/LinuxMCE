/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/snk6502.h"


#define TOTAL_COLORS(m,gfxn) ((m).gfx[gfxn]->colors() * (m).gfx[gfxn]->granularity())
#define COLOR(m,gfxn,offs) ((m).config().m_gfxdecodeinfo[gfxn].color_codes_start + offs)



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Zarzon has a different PROM layout from the others.

***************************************************************************/
PALETTE_INIT_MEMBER(snk6502_state,snk6502)
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	for (i = 0; i < machine().total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */

		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;

		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */

		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;

		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */

		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;

		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		m_palette[i] = MAKE_RGB(r, g, b);

		color_prom++;
	}

	m_backcolor = 0;    /* background color can be changed by the game */

	for (i = 0; i < TOTAL_COLORS(machine(),0); i++)
		palette_set_color(machine(), COLOR(machine(), 0, i), m_palette[i]);

	for (i = 0; i < TOTAL_COLORS(machine(),1); i++)
	{
		if (i % 4 == 0)
			palette_set_color(machine(), COLOR(machine(), 1, i), m_palette[4 * m_backcolor + 0x20]);
		else
			palette_set_color(machine(), COLOR(machine(), 1, i), m_palette[i + 0x20]);
	}
}

WRITE8_MEMBER(snk6502_state::snk6502_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(snk6502_state::snk6502_videoram2_w)
{

	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(snk6502_state::snk6502_colorram_w)
{

	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(snk6502_state::snk6502_charram_w)
{

	if (m_charram[offset] != data)
	{
		m_charram[offset] = data;
		machine().gfx[0]->mark_dirty((offset/8) % 256);
	}
}


WRITE8_MEMBER(snk6502_state::snk6502_flipscreen_w)
{
	int bank;

	/* bits 0-2 select background color */

	if (m_backcolor != (data & 7))
	{
		int i;

		m_backcolor = data & 7;

		for (i = 0;i < 32;i += 4)
			palette_set_color(machine(), COLOR(machine(), 1, i), m_palette[4 * m_backcolor + 0x20]);
	}

	/* bit 3 selects char bank */

	bank = (~data & 0x08) >> 3;

	if (m_charbank != bank)
	{
		m_charbank = bank;
		machine().tilemap().mark_all_dirty();
	}

	/* bit 7 flips screen */

	if (flip_screen() != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(snk6502_state::snk6502_scrollx_w)
{

	m_bg_tilemap->set_scrollx(0, data);
}

WRITE8_MEMBER(snk6502_state::snk6502_scrolly_w)
{

	m_bg_tilemap->set_scrolly(0, data);
}


TILE_GET_INFO_MEMBER(snk6502_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + 256 * m_charbank;
	int color = (m_colorram[tile_index] & 0x38) >> 3;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}

TILE_GET_INFO_MEMBER(snk6502_state::get_fg_tile_info)
{
	int code = m_videoram2[tile_index];
	int color = m_colorram[tile_index] & 0x07;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

VIDEO_START_MEMBER(snk6502_state,snk6502)
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(snk6502_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(snk6502_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	machine().gfx[0]->set_source(m_charram);
}

VIDEO_START_MEMBER(snk6502_state,pballoon)
{

	VIDEO_START_CALL_MEMBER( snk6502 );

	m_bg_tilemap->set_scrolldy(-16, -16);
	m_fg_tilemap->set_scrolldy(-16, -16);
}


UINT32 snk6502_state::screen_update_snk6502(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

/* Satan of Saturn */

PALETTE_INIT_MEMBER(snk6502_state,satansat)
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	for (i = 0; i < machine().total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */

		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;

		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */

		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;

		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */

		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;

		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		m_palette[i] = MAKE_RGB(r, g, b);

		color_prom++;
	}

	m_backcolor = 0;    /* background color can be changed by the game */

	for (i = 0; i < TOTAL_COLORS(machine(),0); i++)
		palette_set_color(machine(), COLOR(machine(), 0, i), m_palette[4 * (i % 4) + (i / 4)]);

	for (i = 0; i < TOTAL_COLORS(machine(),1); i++)
	{
		if (i % 4 == 0)
			palette_set_color(machine(), COLOR(machine(), 1, i), m_palette[m_backcolor + 0x10]);
		else
			palette_set_color(machine(), COLOR(machine(), 1, i), m_palette[4 * (i % 4) + (i / 4) + 0x10]);
	}
}

WRITE8_MEMBER(snk6502_state::satansat_b002_w)
{
	/* bit 0 flips screen */

	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}

	/* bit 1 enables interrupts */
	m_irq_mask = data & 2;

	/* other bits unused */
}

WRITE8_MEMBER(snk6502_state::satansat_backcolor_w)
{

	/* bits 0-1 select background color. Other bits unused. */

	if (m_backcolor != (data & 0x03))
	{
		int i;

		m_backcolor = data & 0x03;

		for (i = 0; i < 16; i += 4)
			palette_set_color(machine(), COLOR(machine(), 1, i), m_palette[m_backcolor + 0x10]);
	}
}

TILE_GET_INFO_MEMBER(snk6502_state::satansat_get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = (m_colorram[tile_index] & 0x0c) >> 2;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}

TILE_GET_INFO_MEMBER(snk6502_state::satansat_get_fg_tile_info)
{
	int code = m_videoram2[tile_index];
	int color = m_colorram[tile_index] & 0x03;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

VIDEO_START_MEMBER(snk6502_state,satansat)
{

	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(snk6502_state::satansat_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(snk6502_state::satansat_get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	machine().gfx[0]->set_source(m_charram);
}
