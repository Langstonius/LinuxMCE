/***************************************************************************
  Functions to emulate video hardware on these Taito games:

  - rastan

***************************************************************************/

#include "emu.h"
#include "video/taitoic.h"
#include "includes/rastan.h"

/***************************************************************************/

WRITE16_MEMBER(rastan_state::rastan_spritectrl_w)
{

	/* bits 5-7 are the sprite palette bank */
	pc090oj_set_sprite_ctrl(m_pc090oj, (data & 0xe0) >> 5);

	/* bit 4 unused */

	/* bits 0 and 1 are coin lockout */
	coin_lockout_w(machine(), 1, ~data & 0x01);
	coin_lockout_w(machine(), 0, ~data & 0x02);

	/* bits 2 and 3 are the coin counters */
	coin_counter_w(machine(), 1, data & 0x04);
	coin_counter_w(machine(), 0, data & 0x08);
}

/***************************************************************************/

UINT32 rastan_state::screen_update_rastan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[2];

	pc080sn_tilemap_update(m_pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	machine().priority_bitmap.fill(0, cliprect);

	pc080sn_tilemap_draw(m_pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(m_pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(m_pc090oj, bitmap, cliprect, 0);
	return 0;
}
