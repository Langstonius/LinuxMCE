/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/superqix.h"




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(superqix_state::pb_get_bg_tile_info)
{
	int attr = m_videoram[tile_index + 0x400];
	int code = m_videoram[tile_index] + 256 * (attr & 0x7);
	int color = (attr & 0xf0) >> 4;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(superqix_state::sqix_get_bg_tile_info)
{
	int attr = m_videoram[tile_index + 0x400];
	int bank = (attr & 0x04) ? 0 : 1;
	int code = m_videoram[tile_index] + 256 * (attr & 0x03);
	int color = (attr & 0xf0) >> 4;

	if (bank) code += 1024 * m_gfxbank;

	SET_TILE_INFO_MEMBER(bank, code, color, 0);
	tileinfo.group = (attr & 0x08) >> 3;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(superqix_state,pbillian)
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(superqix_state::pb_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8,32,32);
}

VIDEO_START_MEMBER(superqix_state,superqix)
{
	m_fg_bitmap[0] = auto_bitmap_ind16_alloc(machine(), 256, 256);
	m_fg_bitmap[1] = auto_bitmap_ind16_alloc(machine(), 256, 256);
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(superqix_state::sqix_get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);

	m_bg_tilemap->set_transmask(0,0xffff,0x0000); /* split type 0 is totally transparent in front half */
	m_bg_tilemap->set_transmask(1,0x0001,0xfffe); /* split type 1 has pen 0 transparent in front half */

	save_item(NAME(m_gfxbank));
	save_item(NAME(m_show_bitmap));
	save_item(NAME(*m_fg_bitmap[0]));
	save_item(NAME(*m_fg_bitmap[1]));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(superqix_state::superqix_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(superqix_state::superqix_bitmapram_w)
{
	if (m_bitmapram[offset] != data)
	{
		int x = 2 * (offset % 128);
		int y = offset / 128 + 16;

		m_bitmapram[offset] = data;

		m_fg_bitmap[0]->pix16(y, x)     = data >> 4;
		m_fg_bitmap[0]->pix16(y, x + 1) = data & 0x0f;
	}
}

WRITE8_MEMBER(superqix_state::superqix_bitmapram2_w)
{
	if (data != m_bitmapram2[offset])
	{
		int x = 2 * (offset % 128);
		int y = offset / 128 + 16;

		m_bitmapram2[offset] = data;

		m_fg_bitmap[1]->pix16(y, x)     = data >> 4;
		m_fg_bitmap[1]->pix16(y, x + 1) = data & 0x0f;
	}
}

WRITE8_MEMBER(superqix_state::pbillian_0410_w)
{
	/*
	 -------0  ? [not used]
	 ------1-  coin counter 1
	 -----2--  coin counter 2
	 ----3---  rom 2 HI (reserved for ROM banking , not used)
	 ---4----  nmi enable/disable
	 --5-----  flip screen
	*/

	coin_counter_w(machine(), 0,data & 0x02);
	coin_counter_w(machine(), 1,data & 0x04);

	membank("bank1")->set_entry((data & 0x08) >> 3);

	m_nmi_mask = data & 0x10;
	flip_screen_set(data & 0x20);
}

WRITE8_MEMBER(superqix_state::superqix_0410_w)
{
	/* bits 0-1 select the tile bank */
	if (m_gfxbank != (data & 0x03))
	{
		m_gfxbank = data & 0x03;
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 2 selects which of the two bitmaps to display (for 2 players game) */
	m_show_bitmap = (data & 0x04) >> 2;

	/* bit 3 enables NMI */
	m_nmi_mask = data & 0x08;

	/* bits 4-5 control ROM bank */
	membank("bank1")->set_entry((data & 0x30) >> 4);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void pbillian_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	superqix_state *state = machine.driver_data<superqix_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < state->m_spriteram.bytes(); offs += 4)
	{
		int attr = spriteram[offs + 3];
		int code = ((spriteram[offs] & 0xfc) >> 2) + 64 * (attr & 0x0f);
		int color = (attr & 0xf0) >> 4;
		int sx = spriteram[offs + 1] + 256 * (spriteram[offs] & 0x01);
		int sy = spriteram[offs + 2];

		if (state->flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap,cliprect, machine.gfx[1],
				code,
				color,
				state->flip_screen(), state->flip_screen(),
				sx, sy, 0);
	}
}

static void superqix_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	superqix_state *state = machine.driver_data<superqix_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < state->m_spriteram.bytes(); offs += 4)
	{
		int attr = spriteram[offs + 3];
		int code = spriteram[offs] + 256 * (attr & 0x01);
		int color = (attr & 0xf0) >> 4;
		int flipx = attr & 0x04;
		int flipy = attr & 0x08;
		int sx = spriteram[offs + 1];
		int sy = spriteram[offs + 2];

		if (state->flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect, machine.gfx[2],
				code,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

UINT32 superqix_state::screen_update_pbillian(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	pbillian_draw_sprites(machine(), bitmap,cliprect);

	return 0;
}

UINT32 superqix_state::screen_update_superqix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	copybitmap_trans(bitmap,*m_fg_bitmap[m_show_bitmap],flip_screen(),flip_screen(),0,0,cliprect,0);
	superqix_draw_sprites(machine(), bitmap,cliprect);
	m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}
