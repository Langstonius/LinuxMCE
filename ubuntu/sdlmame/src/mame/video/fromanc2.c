/******************************************************************************

    Video Hardware for Video System Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -

******************************************************************************/

#include "emu.h"
#include "includes/fromanc2.h"

/******************************************************************************

  Callbacks for the TileMap code

******************************************************************************/

INLINE void fromanc2_get_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int vram, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	int tile, color;

	tile  = (state->m_videoram[vram][layer][tile_index] & 0x3fff) | (state->m_gfxbank[vram][layer] << 14);
	color = ((state->m_videoram[vram][layer][tile_index] & 0xc000) >> 14) | (0x10 * vram);

	SET_TILE_INFO(layer, tile, color, 0);
}

TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_v0_l0_tile_info){ fromanc2_get_tile_info(machine(), tileinfo, tile_index, 0, 0); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_v0_l1_tile_info){ fromanc2_get_tile_info(machine(), tileinfo, tile_index, 0, 1); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_v0_l2_tile_info){ fromanc2_get_tile_info(machine(), tileinfo, tile_index, 0, 2); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_v0_l3_tile_info){ fromanc2_get_tile_info(machine(), tileinfo, tile_index, 0, 3); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_v1_l0_tile_info){ fromanc2_get_tile_info(machine(), tileinfo, tile_index, 1, 0); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_v1_l1_tile_info){ fromanc2_get_tile_info(machine(), tileinfo, tile_index, 1, 1); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_v1_l2_tile_info){ fromanc2_get_tile_info(machine(), tileinfo, tile_index, 1, 2); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromanc2_get_v1_l3_tile_info){ fromanc2_get_tile_info(machine(), tileinfo, tile_index, 1, 3); }


INLINE void fromancr_get_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int vram, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	int tile, color;

	tile  = state->m_videoram[vram][layer][tile_index] | (state->m_gfxbank[vram][layer] << 16);
	color = vram;

	SET_TILE_INFO(layer, tile, color, 0);
}

TILE_GET_INFO_MEMBER(fromanc2_state::fromancr_get_v0_l0_tile_info){ fromancr_get_tile_info(machine(), tileinfo, tile_index, 0, 0); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromancr_get_v0_l1_tile_info){ fromancr_get_tile_info(machine(), tileinfo, tile_index, 0, 1); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromancr_get_v0_l2_tile_info){ fromancr_get_tile_info(machine(), tileinfo, tile_index, 0, 2); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromancr_get_v1_l0_tile_info){ fromancr_get_tile_info(machine(), tileinfo, tile_index, 1, 0); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromancr_get_v1_l1_tile_info){ fromancr_get_tile_info(machine(), tileinfo, tile_index, 1, 1); }
TILE_GET_INFO_MEMBER(fromanc2_state::fromancr_get_v1_l2_tile_info){ fromancr_get_tile_info(machine(), tileinfo, tile_index, 1, 2); }


/******************************************************************************

  Memory handlers

******************************************************************************/

READ16_MEMBER(fromanc2_state::fromanc2_paletteram_0_r)
{
	return m_paletteram[0][offset];
}

READ16_MEMBER(fromanc2_state::fromanc2_paletteram_1_r)
{
	return m_paletteram[1][offset];
}

WRITE16_MEMBER(fromanc2_state::fromanc2_paletteram_0_w)
{
	UINT16 color;

	COMBINE_DATA(&m_paletteram[0][offset]);

	// GGGG_GRRR_RRBB_BBBx
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(machine(), 0x000 + color, pal5bit(data >> 6), pal5bit(data >> 11), pal5bit(data >> 1));
}

WRITE16_MEMBER(fromanc2_state::fromanc2_paletteram_1_w)
{
	UINT16 color;

	COMBINE_DATA(&m_paletteram[1][offset]);

	// GGGG_GRRR_RRBB_BBBx
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(machine(), 0x100 + color, pal5bit(data >> 6), pal5bit(data >> 11), pal5bit(data >> 1));
}


READ16_MEMBER(fromanc2_state::fromancr_paletteram_0_r)
{
	return m_paletteram[0][offset];
}

READ16_MEMBER(fromanc2_state::fromancr_paletteram_1_r)
{
	return m_paletteram[1][offset];
}

WRITE16_MEMBER(fromanc2_state::fromancr_paletteram_0_w)
{
	UINT16 color;

	COMBINE_DATA(&m_paletteram[0][offset]);

	// xGGG_GGRR_RRRB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(machine(), 0x000 + color, pal5bit(data >> 5), pal5bit(data >> 10), pal5bit(data >> 0));
}

WRITE16_MEMBER(fromanc2_state::fromancr_paletteram_1_w)
{
	UINT16 color;

	COMBINE_DATA(&m_paletteram[1][offset]);

	// xGGG_GGRR_RRRB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(machine(), 0x100 + color, pal5bit(data >> 5), pal5bit(data >> 10), pal5bit(data >> 0));
}


READ16_MEMBER(fromanc2_state::fromanc4_paletteram_0_r)
{
	return m_paletteram[0][offset];
}

READ16_MEMBER(fromanc2_state::fromanc4_paletteram_1_r)
{
	return m_paletteram[1][offset];
}

WRITE16_MEMBER(fromanc2_state::fromanc4_paletteram_0_w)
{
	UINT16 color;

	COMBINE_DATA(&m_paletteram[0][offset]);

	// xRRR_RRGG_GGGB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(machine(), 0x000 + color, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

WRITE16_MEMBER(fromanc2_state::fromanc4_paletteram_1_w)
{
	UINT16 color;

	COMBINE_DATA(&m_paletteram[1][offset]);

	// xRRR_RRGG_GGGB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(machine(), 0x100 + color, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}


INLINE void fromanc2_dispvram_w( running_machine &machine, offs_t offset, UINT16 data, UINT16 mem_mask, int vram, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	layer += (offset < 0x1000) ? 0 : 1;

	COMBINE_DATA(&state->m_videoram[vram][layer][offset & 0x0fff]);
	state->m_tilemap[vram][layer]->mark_tile_dirty(offset & 0x0fff);
}

WRITE16_MEMBER(fromanc2_state::fromanc2_videoram_0_w){ fromanc2_dispvram_w(machine(), offset, data, mem_mask, 0, 0); }
WRITE16_MEMBER(fromanc2_state::fromanc2_videoram_1_w){ fromanc2_dispvram_w(machine(), offset, data, mem_mask, 0, 2); }
WRITE16_MEMBER(fromanc2_state::fromanc2_videoram_2_w){ fromanc2_dispvram_w(machine(), offset, data, mem_mask, 1, 0); }
WRITE16_MEMBER(fromanc2_state::fromanc2_videoram_3_w){ fromanc2_dispvram_w(machine(), offset, data, mem_mask, 1, 2); }

WRITE16_MEMBER(fromanc2_state::fromanc2_gfxreg_0_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][0] = -(data - 0x000); break;
		case 0x01:  m_scrolly[0][0] = -(data - 0x000); break;
		case 0x02:  m_scrollx[0][1] = -(data - 0x004); break;
		case 0x03:  m_scrolly[0][1] = -(data - 0x000); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

WRITE16_MEMBER(fromanc2_state::fromanc2_gfxreg_1_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[1][0] = -(data - 0x1be); break;
		case 0x01:  m_scrolly[1][0] = -(data - 0x1ef); break;
		case 0x02:  m_scrollx[1][1] = -(data - 0x1c2); break;
		case 0x03:  m_scrolly[1][1] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

WRITE16_MEMBER(fromanc2_state::fromanc2_gfxreg_2_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][2] = -(data - 0x1c0); break;
		case 0x01:  m_scrolly[0][2] = -(data - 0x1ef); break;
		case 0x02:  m_scrollx[0][3] = -(data - 0x1c3); break;
		case 0x03:  m_scrolly[0][3] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

WRITE16_MEMBER(fromanc2_state::fromanc2_gfxreg_3_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[1][2] = -(data - 0x1bf); break;
		case 0x01:  m_scrolly[1][2] = -(data - 0x1ef); break;
		case 0x02:  m_scrollx[1][3] = -(data - 0x1c3); break;
		case 0x03:  m_scrolly[1][3] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

WRITE16_MEMBER(fromanc2_state::fromanc2_gfxbank_0_w)
{
	m_gfxbank[0][0] = (data & 0x000f) >>  0;
	m_gfxbank[0][1] = (data & 0x00f0) >>  4;
	m_gfxbank[0][2] = (data & 0x0f00) >>  8;
	m_gfxbank[0][3] = (data & 0xf000) >> 12;
	m_tilemap[0][0]->mark_all_dirty();
	m_tilemap[0][1]->mark_all_dirty();
	m_tilemap[0][2]->mark_all_dirty();
	m_tilemap[0][3]->mark_all_dirty();
}

WRITE16_MEMBER(fromanc2_state::fromanc2_gfxbank_1_w)
{
	m_gfxbank[1][0] = (data & 0x000f) >>  0;
	m_gfxbank[1][1] = (data & 0x00f0) >>  4;
	m_gfxbank[1][2] = (data & 0x0f00) >>  8;
	m_gfxbank[1][3] = (data & 0xf000) >> 12;
	m_tilemap[1][0]->mark_all_dirty();
	m_tilemap[1][1]->mark_all_dirty();
	m_tilemap[1][2]->mark_all_dirty();
	m_tilemap[1][3]->mark_all_dirty();
}


INLINE void fromancr_vram_w(running_machine &machine, offs_t offset, UINT16 data, UINT16 mem_mask, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	int vram = (offset < 0x1000) ? 0 : 1;

	COMBINE_DATA(&state->m_videoram[vram][layer][offset & 0x0fff]);
	state->m_tilemap[vram][layer]->mark_tile_dirty(offset & 0x0fff);
}

WRITE16_MEMBER(fromanc2_state::fromancr_videoram_0_w){ fromancr_vram_w(machine(), offset, data, mem_mask, 1); }
WRITE16_MEMBER(fromanc2_state::fromancr_videoram_1_w){ fromancr_vram_w(machine(), offset, data, mem_mask, 0); }
WRITE16_MEMBER(fromanc2_state::fromancr_videoram_2_w){ fromancr_vram_w(machine(), offset, data, mem_mask, 2); }

WRITE16_MEMBER(fromanc2_state::fromancr_gfxreg_0_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][0] = -(data - 0x1bf); break;
		case 0x01:  m_scrolly[0][0] = -(data - 0x1ef); break;
		case 0x02:  m_scrollx[1][0] = -(data - 0x1c3); break;
		case 0x03:  m_scrolly[1][0] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

WRITE16_MEMBER(fromanc2_state::fromancr_gfxreg_1_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][1] = -(data - 0x000); break;
		case 0x01:  m_scrolly[0][1] = -(data - 0x000); break;
		case 0x02:  m_scrollx[1][1] = -(data - 0x004); break;
		case 0x03:  m_scrolly[1][1] = -(data - 0x000); break;
		// offset 0x04 - 0x11 unknown
		default:    break;
	}
}

void fromancr_gfxbank_w( running_machine &machine, int data )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();

	state->m_gfxbank[0][0] = (data & 0x0010) >>  4; // BG (1P)
	state->m_gfxbank[0][1] = (data & 0xf000) >> 12; // FG (1P)
	state->m_gfxbank[1][0] = (data & 0x0008) >>  3; // BG (2P)
	state->m_gfxbank[1][1] = (data & 0x0f00) >>  8; // FG (2P)
	state->m_tilemap[0][0]->mark_all_dirty();
	state->m_tilemap[0][1]->mark_all_dirty();
	state->m_tilemap[1][0]->mark_all_dirty();
	state->m_tilemap[1][1]->mark_all_dirty();
}


INLINE void fromanc4_vram_w( running_machine &machine, offs_t offset, UINT16 data, UINT16 mem_mask, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	int vram = (offset < 0x4000) ? 0 : 1;

	COMBINE_DATA(&state->m_videoram[vram][layer][offset & 0x3fff]);
	state->m_tilemap[vram][layer]->mark_tile_dirty(offset & 0x3fff);
}

WRITE16_MEMBER(fromanc2_state::fromanc4_videoram_0_w){ fromanc4_vram_w(machine(), offset, data, mem_mask, 2); }
WRITE16_MEMBER(fromanc2_state::fromanc4_videoram_1_w){ fromanc4_vram_w(machine(), offset, data, mem_mask, 1); }
WRITE16_MEMBER(fromanc2_state::fromanc4_videoram_2_w){ fromanc4_vram_w(machine(), offset, data, mem_mask, 0); }

WRITE16_MEMBER(fromanc2_state::fromanc4_gfxreg_0_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][2] = -(data - 0xfbb); break;
		case 0x01:  m_scrolly[0][2] = -(data - 0x1e4); break;
		case 0x02:  m_scrollx[1][2] = -(data - 0xfbb); break;
		case 0x03:  m_scrolly[1][2] = -(data - 0x1e4); break;
		case 0x05:  m_gfxbank[0][2] = (data & 0x000f) >> 0;
				m_gfxbank[1][2] = (data & 0x0f00) >> 8;
				m_tilemap[0][2]->mark_all_dirty();
				m_tilemap[1][2]->mark_all_dirty();
				break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:    break;
	}
}

WRITE16_MEMBER(fromanc2_state::fromanc4_gfxreg_1_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][1] = -(data - 0xfba); break;
		case 0x01:  m_scrolly[0][1] = -(data - 0x1e4); break;
		case 0x02:  m_scrollx[1][1] = -(data - 0xfba); break;
		case 0x03:  m_scrolly[1][1] = -(data - 0x1e4); break;
		case 0x05:  m_gfxbank[0][1] = (data & 0x000f) >> 0;
				m_gfxbank[1][1] = (data & 0x0f00) >> 8;
				m_tilemap[0][1]->mark_all_dirty();
				m_tilemap[1][1]->mark_all_dirty();
				break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:    break;
	}
}

WRITE16_MEMBER(fromanc2_state::fromanc4_gfxreg_2_w)
{
	switch (offset)
	{
		case 0x00:  m_scrollx[0][0] = -(data - 0xfbb); break;
		case 0x01:  m_scrolly[0][0] = -(data - 0x1e4); break;
		case 0x02:  m_scrollx[1][0] = -(data - 0xfbb); break;
		case 0x03:  m_scrolly[1][0] = -(data - 0x1e4); break;
		case 0x05:  m_gfxbank[0][0] = (data & 0x000f) >> 0;
				m_gfxbank[1][0] = (data & 0x0f00) >> 8;
				m_tilemap[0][0]->mark_all_dirty();
				m_tilemap[1][0]->mark_all_dirty();
				break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:    break;
	}
}


/******************************************************************************

  Start the video hardware emulation.

******************************************************************************/

VIDEO_START_MEMBER(fromanc2_state,fromanc2)
{

	m_tilemap[0][0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromanc2_get_v0_l0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromanc2_get_v0_l1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromanc2_get_v0_l2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][3] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromanc2_get_v0_l3_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromanc2_get_v1_l0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromanc2_get_v1_l1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromanc2_get_v1_l2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][3] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromanc2_get_v1_l3_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[0][1]->set_transparent_pen(0x000);
	m_tilemap[0][2]->set_transparent_pen(0x000);
	m_tilemap[0][3]->set_transparent_pen(0x000);
	m_tilemap[1][1]->set_transparent_pen(0x000);
	m_tilemap[1][2]->set_transparent_pen(0x000);
	m_tilemap[1][3]->set_transparent_pen(0x000);

	m_videoram[0][0] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[0][1] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[0][2] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[0][3] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[1][0] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[1][1] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[1][2] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[1][3] = auto_alloc_array(machine(), UINT16, (64 * 64));

	m_paletteram[0] = auto_alloc_array(machine(), UINT16, 0x800);
	m_paletteram[1] = auto_alloc_array(machine(), UINT16, 0x800);

	save_pointer(NAME(m_videoram[0][0]), (64 * 64));
	save_pointer(NAME(m_videoram[0][1]), (64 * 64));
	save_pointer(NAME(m_videoram[0][2]), (64 * 64));
	save_pointer(NAME(m_videoram[0][3]), (64 * 64));
	save_pointer(NAME(m_videoram[1][0]), (64 * 64));
	save_pointer(NAME(m_videoram[1][1]), (64 * 64));
	save_pointer(NAME(m_videoram[1][2]), (64 * 64));
	save_pointer(NAME(m_videoram[1][3]), (64 * 64));
	save_pointer(NAME(m_paletteram[0]), 0x800);
	save_pointer(NAME(m_paletteram[1]), 0x800);
	save_item(NAME(m_scrollx[0]));
	save_item(NAME(m_scrollx[1]));
	save_item(NAME(m_scrolly[0]));
	save_item(NAME(m_scrolly[1]));
	save_item(NAME(m_gfxbank[0]));
	save_item(NAME(m_gfxbank[1]));
}

VIDEO_START_MEMBER(fromanc2_state,fromancr)
{

	m_tilemap[0][0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v0_l0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v0_l1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v0_l2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[0][3] = 0;
	m_tilemap[1][0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v1_l0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v1_l1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v1_l2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][3] = 0;

	m_tilemap[0][1]->set_transparent_pen(0x0ff);
	m_tilemap[0][2]->set_transparent_pen(0x0ff);
	m_tilemap[1][1]->set_transparent_pen(0x0ff);
	m_tilemap[1][2]->set_transparent_pen(0x0ff);

	m_videoram[0][0] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[0][1] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[0][2] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[1][0] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[1][1] = auto_alloc_array(machine(), UINT16, (64 * 64));
	m_videoram[1][2] = auto_alloc_array(machine(), UINT16, (64 * 64));

	m_paletteram[0] = auto_alloc_array(machine(), UINT16, 0x800);
	m_paletteram[1] = auto_alloc_array(machine(), UINT16, 0x800);

	save_pointer(NAME(m_videoram[0][0]), (64 * 64));
	save_pointer(NAME(m_videoram[0][1]), (64 * 64));
	save_pointer(NAME(m_videoram[0][2]), (64 * 64));
	save_pointer(NAME(m_videoram[1][0]), (64 * 64));
	save_pointer(NAME(m_videoram[1][1]), (64 * 64));
	save_pointer(NAME(m_videoram[1][2]), (64 * 64));
	save_pointer(NAME(m_paletteram[0]), 0x800);
	save_pointer(NAME(m_paletteram[1]), 0x800);
	save_item(NAME(m_scrollx[0]));
	save_item(NAME(m_scrollx[1]));
	save_item(NAME(m_scrolly[0]));
	save_item(NAME(m_scrolly[1]));
	save_item(NAME(m_gfxbank[0]));
	save_item(NAME(m_gfxbank[1]));
}


VIDEO_START_MEMBER(fromanc2_state,fromanc4)
{

	m_tilemap[0][0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v0_l0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[0][1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v0_l1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[0][2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v0_l2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[0][3] = 0;
	m_tilemap[1][0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v1_l0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[1][1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v1_l1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[1][2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(fromanc2_state::fromancr_get_v1_l2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_tilemap[1][3] = 0;

	m_tilemap[0][1]->set_transparent_pen(0x000);
	m_tilemap[0][2]->set_transparent_pen(0x000);
	m_tilemap[1][1]->set_transparent_pen(0x000);
	m_tilemap[1][2]->set_transparent_pen(0x000);

	m_videoram[0][0] = auto_alloc_array(machine(), UINT16, (256 * 64));
	m_videoram[0][1] = auto_alloc_array(machine(), UINT16, (256 * 64));
	m_videoram[0][2] = auto_alloc_array(machine(), UINT16, (256 * 64));
	m_videoram[1][0] = auto_alloc_array(machine(), UINT16, (256 * 64));
	m_videoram[1][1] = auto_alloc_array(machine(), UINT16, (256 * 64));
	m_videoram[1][2] = auto_alloc_array(machine(), UINT16, (256 * 64));

	m_paletteram[0] = auto_alloc_array(machine(), UINT16, 0x800);
	m_paletteram[1] = auto_alloc_array(machine(), UINT16, 0x800);

	save_pointer(NAME(m_videoram[0][0]), (256 * 64));
	save_pointer(NAME(m_videoram[0][1]), (256 * 64));
	save_pointer(NAME(m_videoram[0][2]), (256 * 64));
	save_pointer(NAME(m_videoram[1][0]), (256 * 64));
	save_pointer(NAME(m_videoram[1][1]), (256 * 64));
	save_pointer(NAME(m_videoram[1][2]), (256 * 64));
	save_pointer(NAME(m_paletteram[0]), 0x800);
	save_pointer(NAME(m_paletteram[1]), 0x800);
	save_item(NAME(m_scrollx[0]));
	save_item(NAME(m_scrollx[1]));
	save_item(NAME(m_scrolly[0]));
	save_item(NAME(m_scrolly[1]));
	save_item(NAME(m_gfxbank[0]));
	save_item(NAME(m_gfxbank[1]));
}

/******************************************************************************

  Display refresh

******************************************************************************/

UINT32 fromanc2_state::screen_update_fromanc2_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		if (m_tilemap[0][i])
		{
			m_tilemap[0][i]->set_scrollx(0, -m_scrollx[0][i]);
			m_tilemap[0][i]->set_scrolly(0, -m_scrolly[0][i]);
			m_tilemap[0][i]->draw(bitmap, cliprect, 0, 0);
		}
	}

	return 0;
}

UINT32 fromanc2_state::screen_update_fromanc2_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		if (m_tilemap[1][i])
		{
			m_tilemap[1][i]->set_scrollx(0, -m_scrollx[1][i]);
			m_tilemap[1][i]->set_scrolly(0, -m_scrolly[1][i]);
			m_tilemap[1][i]->draw(bitmap, cliprect, 0, 0);
		}
	}

	return 0;
}
