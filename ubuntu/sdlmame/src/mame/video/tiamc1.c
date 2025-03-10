/***************************************************************************

  TIA-MC1 video hardware

  driver by Eugene Sandulenko
  special thanks to Shiru for his standalone emulator and documentation

***************************************************************************/

#include "emu.h"
#include "includes/tiamc1.h"


WRITE8_MEMBER(tiamc1_state::tiamc1_videoram_w)
{
	if(!(m_layers_ctrl & 2))
		m_charram[offset + 0x0000] = data;
	if(!(m_layers_ctrl & 4))
		m_charram[offset + 0x0800] = data;
	if(!(m_layers_ctrl & 8))
		m_charram[offset + 0x1000] = data;
	if(!(m_layers_ctrl & 16))
		m_charram[offset + 0x1800] = data;

	if ((m_layers_ctrl & (16|8|4|2)) != (16|8|4|2))
		machine().gfx[0]->mark_dirty((offset / 8) & 0xff);

	if(!(m_layers_ctrl & 1)) {
		m_tileram[offset] = data;
		if (offset < 1024)
			m_bg_tilemap1->mark_tile_dirty(offset & 0x3ff);
		else
			m_bg_tilemap2->mark_tile_dirty(offset & 0x3ff);
	}
}

WRITE8_MEMBER(tiamc1_state::tiamc1_bankswitch_w)
{
	if ((data & 128) != (m_layers_ctrl & 128))
		machine().tilemap().mark_all_dirty();

	m_layers_ctrl = data;
}

WRITE8_MEMBER(tiamc1_state::tiamc1_sprite_x_w)
{
	m_spriteram_x[offset] = data;
}

WRITE8_MEMBER(tiamc1_state::tiamc1_sprite_y_w)
{
	m_spriteram_y[offset] = data;
}

WRITE8_MEMBER(tiamc1_state::tiamc1_sprite_a_w)
{
	m_spriteram_a[offset] = data;
}

WRITE8_MEMBER(tiamc1_state::tiamc1_sprite_n_w)
{
	m_spriteram_n[offset] = data;
}

WRITE8_MEMBER(tiamc1_state::tiamc1_bg_vshift_w)
{
	m_bg_vshift = data;
}

WRITE8_MEMBER(tiamc1_state::tiamc1_bg_hshift_w)
{
	m_bg_hshift = data;
}

WRITE8_MEMBER(tiamc1_state::tiamc1_palette_w)
{
	palette_set_color(machine(), offset, m_palette[data]);
}

void tiamc1_state::palette_init()
{
	// Voltage computed by Proteus
	//static const float g_v[8]={1.05f,0.87f,0.81f,0.62f,0.44f,0.25f,0.19f,0.00f};
	//static const float r_v[8]={1.37f,1.13f,1.00f,0.75f,0.63f,0.38f,0.25f,0.00f};
	//static const float b_v[4]={1.16f,0.75f,0.42f,0.00f};

	// Voltage adjusted by Shiru
	static const float g_v[8] = { 1.2071f,0.9971f,0.9259f,0.7159f,0.4912f,0.2812f,0.2100f,0.0000f};
	static const float r_v[8] = { 1.5937f,1.3125f,1.1562f,0.8750f,0.7187f,0.4375f,0.2812f,0.0000f};
	static const float b_v[4] = { 1.3523f,0.8750f,0.4773f,0.0000f};

	int col;
	int r, g, b, ir, ig, ib;
	float tcol;

	m_palette = auto_alloc_array(machine(), rgb_t, 256);

	for (col = 0; col < 256; col++) {
		ir = (col >> 3) & 7;
		ig = col & 7;
		ib = (col >> 6) & 3;
		tcol = 255.0f * r_v[ir] / r_v[0];
		r = 255 - (((int)tcol) & 255);
		tcol = 255.0f * g_v[ig] / g_v[0];
		g = 255 - (((int)tcol) & 255);
		tcol = 255.0f * b_v[ib] / b_v[0];
		b = 255 - (((int)tcol) & 255);

		m_palette[col] = MAKE_RGB(r,g,b);
	}
}

TILE_GET_INFO_MEMBER(tiamc1_state::get_bg1_tile_info)
{
	SET_TILE_INFO_MEMBER(0, m_tileram[tile_index], 0, 0);
}

TILE_GET_INFO_MEMBER(tiamc1_state::get_bg2_tile_info)
{
	SET_TILE_INFO_MEMBER(0, m_tileram[tile_index + 1024], 0, 0);
}

void tiamc1_state::video_start()
{
	UINT8 *video_ram;

	video_ram = auto_alloc_array_clear(machine(), UINT8, 0x3040);

		m_charram = video_ram + 0x0800;     /* Ram is banked */
		m_tileram = video_ram + 0x0000;

	m_spriteram_y = video_ram + 0x3000;
	m_spriteram_x = video_ram + 0x3010;
	m_spriteram_n = video_ram + 0x3020;
	m_spriteram_a = video_ram + 0x3030;

	state_save_register_global_pointer(machine(), video_ram, 0x3040);

	m_bg_tilemap1 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(tiamc1_state::get_bg1_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_bg_tilemap2 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(tiamc1_state::get_bg2_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_bg_vshift = 0;
	m_bg_hshift = 0;

	state_save_register_global(machine(), m_layers_ctrl);
	state_save_register_global(machine(), m_bg_vshift);
	state_save_register_global(machine(), m_bg_hshift);

	machine().gfx[0]->set_source(m_charram);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	tiamc1_state *state = machine.driver_data<tiamc1_state>();
	int offs;

	for (offs = 0; offs < 16; offs++)
	{
		int flipx, flipy, sx, sy, spritecode;

		sx = state->m_spriteram_x[offs] ^ 0xff;
		sy = state->m_spriteram_y[offs] ^ 0xff;
		flipx = !(state->m_spriteram_a[offs] & 0x08);
		flipy = !(state->m_spriteram_a[offs] & 0x02);
		spritecode = state->m_spriteram_n[offs] ^ 0xff;

		if (!(state->m_spriteram_a[offs] & 0x01))
			drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
				spritecode,
				0,
				flipx, flipy,
				sx, sy, 15);
	}
}

UINT32 tiamc1_state::screen_update_tiamc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#if 0
	int i;

	for (i = 0; i < 32; i++)
	{
		m_bg_tilemap1->set_scrolly(i, m_bg_vshift ^ 0xff);
		m_bg_tilemap2->set_scrolly(i, m_bg_vshift ^ 0xff);
	}

	for (i = 0; i < 32; i++)
	{
		m_bg_tilemap1->set_scrollx(i, m_bg_hshift ^ 0xff);
		m_bg_tilemap2->set_scrollx(i, m_bg_hshift ^ 0xff);
	}
#endif

	if (m_layers_ctrl & 0x80)
		m_bg_tilemap2->draw(bitmap, cliprect, 0, 0);
	else
		m_bg_tilemap1->draw(bitmap, cliprect, 0, 0);


	draw_sprites(machine(), bitmap, cliprect);

	return 0;
}
