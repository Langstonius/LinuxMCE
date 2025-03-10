/*

    Sharp X68000 video functions
    driver by Barry Rodewald

    X68000 video hardware (there are some minor revisions to these custom chips across various X680x0 models):
        Custom sprite controller "Cynthia"
        Custom CRT controller "Vinas / Vicon"
        Custom video controller "VSOP / VIPS"
        Custom video data selector "Cathy"

    In general terms:
        1 "Text" layer - effectively a 4bpp bitmap split into 4 planes at 1bpp each
                         512kB "text" VRAM
                         can write to multiple planes at once
                         can copy one character line to another character line
                         is 1024x1024 in size
        Up to 4 graphic layers - can be 4 layers with a 16 colour palette, 2 layers with a 256 colour palette,
                                 or 1 layer at 16-bit RGB.
                                 512k graphic VRAM
                                 all layers are 512x512, but at 16 colours, the 4 layers can be combined into 1 1024x1024 layer
                                 one or more layers can be cleared at once quickly with a simple hardware function
         2 tilemapped layers - can be 8x8 or 16x16, 16 colours per tile, max 256 colours overall
         1 sprite layer - up to 128 16x16 sprites, 16 colours per sprite, maximum 16 sprites per scanline (not yet implemented).

*/

#include "emu.h"
#include "machine/mc68901.h"
#include "includes/x68k.h"
#include "machine/ram.h"




inline void x68k_state::x68k_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}
/*
static bitmap_ind16* x68k_get_gfx_page(int pri,int type)
{
    if(type == GFX16)
    {
        switch(pri)
        {
        case 0:
            return x68k_gfx_0_bitmap_16;
        case 1:
            return x68k_gfx_1_bitmap_16;
        case 2:
            return x68k_gfx_2_bitmap_16;
        case 3:
            return x68k_gfx_3_bitmap_16;
        default:
            return x68k_gfx_0_bitmap_16;  // should never reach here.
        }
    }
    if(type == GFX256)
    {
        switch(pri)
        {
        case 0:
        case 1:
            return x68k_gfx_0_bitmap_256;
        case 2:
        case 3:
            return x68k_gfx_1_bitmap_256;
        default:
            return x68k_gfx_0_bitmap_256;  // should never reach here.
        }
    }
    if(type == GFX65536)
        return x68k_gfx_0_bitmap_65536;

    return NULL;  // should never reach here either.
}
*/
void x68k_state::x68k_crtc_text_copy(int src, int dest)
{
	// copys one raster in T-VRAM to another raster
	UINT16* tvram;
	int src_ram = src * 256;  // 128 bytes per scanline
	int dest_ram = dest * 256;
	int line;

	if(m_is_32bit)
		tvram = (UINT16*)m_tvram32.target();
	else
		tvram = (UINT16*)m_tvram16.target();

	if(dest > 250)
		return;  // for some reason, Salamander causes a SIGSEGV in a debug build in this function.

	for(line=0;line<8;line++)
	{
		// update RAM in each plane
		memcpy(tvram+dest_ram,tvram+src_ram,128);
		memcpy(tvram+dest_ram+0x10000,tvram+src_ram+0x10000,128);
		memcpy(tvram+dest_ram+0x20000,tvram+src_ram+0x20000,128);
		memcpy(tvram+dest_ram+0x30000,tvram+src_ram+0x30000,128);

		src_ram+=64;
		dest_ram+=64;
	}

}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_crtc_operation_end)
{
	int bit = param;
	m_crtc.operation &= ~bit;
}

void x68k_state::x68k_crtc_refresh_mode()
{
//  rectangle rect;
//  double scantime;
	rectangle scr,visiblescr;
	int length;

	// Calculate data from register values
	m_crtc.vmultiple = 1;
	if((m_crtc.reg[20] & 0x10) != 0 && (m_crtc.reg[20] & 0x0c) == 0)
		m_crtc.vmultiple = 2;  // 31.5kHz + 256 lines = doublescan
	if(m_crtc.interlace != 0)
		m_crtc.vmultiple = 0.5f;  // 31.5kHz + 1024 lines or 15kHz + 512 lines = interlaced
	m_crtc.htotal = (m_crtc.reg[0] + 1) * 8;
	m_crtc.vtotal = (m_crtc.reg[4] + 1) / m_crtc.vmultiple; // default is 567 (568 scanlines)
	m_crtc.hbegin = (m_crtc.reg[2] * 8) + 1;
	m_crtc.hend = (m_crtc.reg[3] * 8);
	m_crtc.vbegin = (m_crtc.reg[6]) / m_crtc.vmultiple;
	m_crtc.vend = (m_crtc.reg[7] - 1) / m_crtc.vmultiple;
	m_crtc.hsync_end = (m_crtc.reg[1]) * 8;
	m_crtc.vsync_end = (m_crtc.reg[5]) / m_crtc.vmultiple;
	m_crtc.hsyncadjust = m_crtc.reg[8];
	scr.set(0, m_crtc.htotal - 8, 0, m_crtc.vtotal);
	if(scr.max_y <= m_crtc.vend)
		scr.max_y = m_crtc.vend + 2;
	if(scr.max_x <= m_crtc.hend)
		scr.max_x = m_crtc.hend + 2;
	visiblescr.set(m_crtc.hbegin, m_crtc.hend, m_crtc.vbegin, m_crtc.vend);

	// expand visible area to the size indicated by CRTC reg 20
	length = m_crtc.hend - m_crtc.hbegin;
	if (length < m_crtc.width)
	{
		visiblescr.min_x = m_crtc.hbegin - ((m_crtc.width - length)/2);
		visiblescr.max_x = m_crtc.hend + ((m_crtc.width - length)/2);
	}
	length = m_crtc.vend - m_crtc.vbegin;
	if (length < m_crtc.height)
	{
		visiblescr.min_y = m_crtc.vbegin - ((m_crtc.height - length)/2);
		visiblescr.max_y = m_crtc.vend + ((m_crtc.height - length)/2);
	}
	// bounds check
	if(visiblescr.min_x < 0)
		visiblescr.min_x = 0;
	if(visiblescr.min_y < 0)
		visiblescr.min_y = 0;
	if(visiblescr.max_x >= scr.max_x)
		visiblescr.max_x = scr.max_x - 2;
	if(visiblescr.max_y >= scr.max_y - 1)
		visiblescr.max_y = scr.max_y - 2;

//  logerror("CRTC regs - %i %i %i %i  - %i %i %i %i - %i - %i\n",m_crtc.reg[0],m_crtc.reg[1],m_crtc.reg[2],m_crtc.reg[3],
//      m_crtc.reg[4],m_crtc.reg[5],m_crtc.reg[6],m_crtc.reg[7],m_crtc.reg[8],m_crtc.reg[9]);
	logerror("video_screen_configure(machine.primary_screen,%i,%i,[%i,%i,%i,%i],55.45)\n",scr.max_x,scr.max_y,visiblescr.min_x,visiblescr.min_y,visiblescr.max_x,visiblescr.max_y);
	machine().primary_screen->configure(scr.max_x,scr.max_y,visiblescr,HZ_TO_ATTOSECONDS(55.45));
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_hsync)
{
	int hstate = param;
	attotime hsync_time;

	m_crtc.hblank = hstate;
	m_mfpdev->i7_w(!m_crtc.hblank);
	if(m_crtc.vmultiple == 2) // 256-line (doublescan)
	{
		if(hstate == 1)
		{
			if(m_oddscanline == 1)
			{
				int scan = machine().primary_screen->vpos();
				if(scan > m_crtc.vend)
					scan = m_crtc.vbegin;
				hsync_time = machine().primary_screen->time_until_pos(scan,(m_crtc.htotal + m_crtc.hend) / 2);
				m_scanline_timer->adjust(hsync_time);
				if(scan != 0)
				{
					if((machine().root_device().ioport("options")->read() & 0x04))
					{
						machine().primary_screen->update_partial(scan);
					}
				}
			}
			else
			{
				int scan = machine().primary_screen->vpos();
				if(scan > m_crtc.vend)
					scan = m_crtc.vbegin;
				hsync_time = machine().primary_screen->time_until_pos(scan,m_crtc.hend / 2);
				m_scanline_timer->adjust(hsync_time);
				if(scan != 0)
				{
					if((machine().root_device().ioport("options")->read() & 0x04))
					{
						machine().primary_screen->update_partial(scan);
					}
				}
			}
		}
		if(hstate == 0)
		{
			if(m_oddscanline == 1)
			{
				int scan = machine().primary_screen->vpos();
				if(scan > m_crtc.vend)
					scan = m_crtc.vbegin;
				else
					scan++;
				hsync_time = machine().primary_screen->time_until_pos(scan,m_crtc.hbegin / 2);
				m_scanline_timer->adjust(hsync_time, 1);
				m_oddscanline = 0;
			}
			else
			{
				hsync_time = machine().primary_screen->time_until_pos(machine().primary_screen->vpos(),(m_crtc.htotal + m_crtc.hbegin) / 2);
				m_scanline_timer->adjust(hsync_time, 1);
				m_oddscanline = 1;
			}
		}
	}
	else  // 512-line
	{
		if(hstate == 1)
		{
			int scan = machine().primary_screen->vpos();
			if(scan > m_crtc.vend)
				scan = 0;
			hsync_time = machine().primary_screen->time_until_pos(scan,m_crtc.hend);
			m_scanline_timer->adjust(hsync_time);
			if(scan != 0)
			{
				if((machine().root_device().ioport("options")->read() & 0x04))
				{
					machine().primary_screen->update_partial(scan);
				}
			}
		}
		if(hstate == 0)
		{
			hsync_time = machine().primary_screen->time_until_pos(machine().primary_screen->vpos()+1,m_crtc.hbegin);
			m_scanline_timer->adjust(hsync_time, 1);
	//      if(!(m_mfp.gpio & 0x40))  // if GPIP6 is active, clear it
	//          m_mfp.gpio |= 0x40;
		}
	}
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_crtc_raster_end)
{
	m_mfp.gpio |= 0x40;
	m_mfpdev->i6_w(1);
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_crtc_raster_irq)
{
	int scan = param;
	attotime irq_time;
	attotime end_time;

	if(scan <= m_crtc.vtotal)
	{
		m_mfp.gpio &= ~0x40;  // GPIP6
		m_mfpdev->i6_w(0);
		machine().primary_screen->update_partial(scan);
		irq_time = machine().primary_screen->time_until_pos(scan,m_crtc.hbegin);
		// end of HBlank period clears GPIP6 also?
		end_time = machine().primary_screen->time_until_pos(scan,m_crtc.hend);
		m_raster_irq->adjust(irq_time, scan);
		machine().scheduler().timer_set(end_time, timer_expired_delegate(FUNC(x68k_state::x68k_crtc_raster_end),this));
		logerror("GPIP6: Raster triggered at line %i (%i)\n",scan,machine().primary_screen->vpos());
	}
}

TIMER_CALLBACK_MEMBER(x68k_state::x68k_crtc_vblank_irq)
{
	device_t *x68k_mfp = machine().device(MC68901_TAG);
	int val = param;
	attotime irq_time;
	int vblank_line;

	if(val == 1)  // V-DISP on
	{
		m_crtc.vblank = 1;
		vblank_line = m_crtc.vbegin;
		irq_time = machine().primary_screen->time_until_pos(vblank_line,2);
		m_vblank_irq->adjust(irq_time);
		logerror("CRTC: VBlank on\n");
	}
	if(val == 0)  // V-DISP off
	{
		m_crtc.vblank = 0;
		vblank_line = m_crtc.vend;
		if(vblank_line > m_crtc.vtotal)
			vblank_line = m_crtc.vtotal;
		irq_time = machine().primary_screen->time_until_pos(vblank_line,2);
		m_vblank_irq->adjust(irq_time, 1);
		logerror("CRTC: VBlank off\n");
	}

	if (x68k_mfp != NULL)
	{
		m_mfpdev->tai_w(!m_crtc.vblank);
		m_mfpdev->i4_w(!m_crtc.vblank);
	}
}


// CRTC "VINAS 1+2 / VICON" at 0xe80000
/* 0xe80000 - Registers (all are 16-bit):
 * 0 - Horizontal Total (in characters)
 * 1 - Horizontal Sync End
 * 2 - Horizontal Display Begin
 * 3 - Horizontal Display End
 * 4 - Vertical Total (in scanlines)
 * 5 - Vertical Sync End
 * 6 - Vertical Display Begin
 * 7 - Vertical Display End
 * 8 - Fine Horizontal Sync Adjustment
 * 9 - Raster Line (for Raster IRQ mapped to MFP GPIP6)
 * 10/11 - Text Layer X and Y Scroll
 * 12/13 - Graphic Layer 0 X and Y Scroll
 * 14/15 - Graphic Layer 1 X and Y Scroll
 * 16/17 - Graphic Layer 2 X and Y Scroll
 * 18/19 - Graphic Layer 3 X and Y Scroll
 * 20 - bit 12 - Text VRAM mode : 0 = display, 1 = buffer
 *      bit 11 - Graphic VRAM mode : 0 = display, 1 = buffer
 *      bit 10 - "Real" screen size : 0 = 512x512, 1 = 1024x1024
 *      bits 8,9 - Colour mode :
 *                 00 = 16 colour      01 = 256 colour
 *                 10 = Undefined      11 = 65,536 colour
 *      bit 4 - Horizontal Frequency : 0 = 15.98kHz, 1 = 31.50kHz
 *      bits 2,3 - Vertical dots :
 *                 00 = 256            01 = 512
 *                 10 or 11 = 1024 (interlaced)
 *      bits 0,1 - Horizontal dots :
 *                 00 = 256            01 = 512
 *                 10 = 768            11 = 50MHz clock mode (Compact XVI or later)
 * 21 - bit 9 - Text Screen Access Mask Enable
 *      bit 8 - Text Screen Simultaneous Plane Access Enable
 *      bits 4-7 - Text Screen Simultaneous Plane Access Select
 *      bits 0-3 - Text Screen Line Copy Plane Select
 *                 Graphic Screen High-speed Clear Page Select
 * 22 - Text Screen Line Copy
 *      bits 15-8 - Source Line
 *      bits 7-0 - Destination Line
 * 23 - Text Screen Mask Pattern
 *
 * 0xe80481 - Operation Port (8-bit):
 *      bit 3 - Text Screen Line Copy Begin
 *      bit 1 - Graphic Screen High-speed Clear Begin
 *      bit 0 - Image Taking Begin (?)
 *    Operation Port bits are cleared automatically when the requested
 *    operation is completed.
 */
WRITE16_MEMBER(x68k_state::x68k_crtc_w )
{
	COMBINE_DATA(m_crtc.reg+offset);
	switch(offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		x68k_crtc_refresh_mode();
		break;
	case 9:  // CRTC raster IRQ (GPIP6)
		{
			attotime irq_time;
			irq_time = machine().primary_screen->time_until_pos((data) / m_crtc.vmultiple,2);

			if(irq_time.as_double() > 0)
				m_raster_irq->adjust(irq_time, (data) / m_crtc.vmultiple);
		}
		logerror("CRTC: Write to raster IRQ register - %i\n",data);
		break;
	case 20:
		if(ACCESSING_BITS_0_7)
		{
			m_crtc.interlace = 0;
			switch(data & 0x0c)
			{
			case 0x00:
				m_crtc.height = 256;
				break;
			case 0x08:
			case 0x0c:  // TODO: 1024 vertical, if horizontal freq = 31kHz
				m_crtc.height = 512;
				m_crtc.interlace = 1;  // if 31kHz, 1024 lines = interlaced
				break;
			case 0x04:
				m_crtc.height = 512;
				if(!(m_crtc.reg[20] & 0x0010))  // if 15kHz, 512 lines = interlaced
					m_crtc.interlace = 1;
				break;
			}
			switch(data & 0x03)
			{
			case 0x00:
				m_crtc.width = 256;
				break;
			case 0x01:
				m_crtc.width = 512;
				break;
			case 0x02:
			case 0x03:  // 0x03 = 50MHz clock mode (XVI only)
				m_crtc.width = 768;
				break;
			}
		}
/*      if(ACCESSING_BITS_8_15)
        {
            m_crtc.interlace = 0;
            if(data & 0x0400)
                m_crtc.interlace = 1;
        }*/
		x68k_crtc_refresh_mode();
		break;
	case 576:  // operation register
		m_crtc.operation = data;
		if(data & 0x08)  // text screen raster copy
		{
			x68k_crtc_text_copy((m_crtc.reg[22] & 0xff00) >> 8,(m_crtc.reg[22] & 0x00ff));
			machine().scheduler().timer_set(attotime::from_msec(1), timer_expired_delegate(FUNC(x68k_state::x68k_crtc_operation_end),this), 0x02);  // time taken to do operation is a complete guess.
		}
		if(data & 0x02)  // high-speed graphic screen clear
		{
			if(m_is_32bit)
				memset(m_gvram32,0,0x40000);
			else
				memset(m_gvram16,0,0x40000);
			machine().scheduler().timer_set(attotime::from_msec(10), timer_expired_delegate(FUNC(x68k_state::x68k_crtc_operation_end),this), 0x02);  // time taken to do operation is a complete guess.
		}
		break;
	}
//  logerror("CRTC: [%08x] Wrote %04x to CRTC register %i\n",machine().device("maincpu")->safe_pc(),data,offset);
}

READ16_MEMBER(x68k_state::x68k_crtc_r )
{
#if 0
	switch(offset)
	{
	default:
		logerror("CRTC: [%08x] Read from CRTC register %i\n",activecpu_get_pc(),offset);
		return 0xff;
	}
#endif

	if(offset < 24)
	{
//      logerror("CRTC: [%08x] Read %04x from CRTC register %i\n",machine().device("maincpu")->safe_pc(),m_crtc.reg[offset],offset);
		switch(offset)
		{
		case 9:
			return 0;
		case 10:  // Text X/Y scroll
		case 11:
		case 12:  // Graphic layer 0 scroll
		case 13:
			return m_crtc.reg[offset] & 0x3ff;
		case 14:  // Graphic layer 1 scroll
		case 15:
		case 16:  // Graphic layer 2 scroll
		case 17:
		case 18:  // Graphic layer 3 scroll
		case 19:
			return m_crtc.reg[offset] & 0x1ff;
		default:
			return m_crtc.reg[offset];
		}
	}
	if(offset == 576) // operation port, operation bits are set to 0 when operation is complete
		return m_crtc.operation;
//  logerror("CRTC: [%08x] Read from unknown CRTC register %i\n",activecpu_get_pc(),offset);
	return 0xffff;
}

WRITE16_MEMBER(x68k_state::x68k_gvram_w )
{
	UINT16* gvram;
//  int xloc,yloc,pageoffset;
	/*
	   G-VRAM usage is determined by colour depth and "real" screen size.

	   For screen size of 1024x1024, all G-VRAM space is used, in one big page.
	   At 1024x1024 real screen size, colour depth is always 4bpp, and ranges from
	   0xc00000-0xdfffff.

	   For screen size of 512x512, the colour depth determines the page usage.
	   16 colours = 4 pages
	   256 colours = 2 pages
	   65,536 colours = 1 page
	   Page 1 - 0xc00000-0xc7ffff    Page 2 - 0xc80000-0xcfffff
	   Page 3 - 0xd00000-0xd7ffff    Page 4 - 0xd80000-0xdfffff
	*/

	if(m_is_32bit)
		gvram = (UINT16*)m_gvram32.target();
	else
		gvram = (UINT16*)m_gvram16.target();

	// handle different G-VRAM page setups
	if(m_crtc.reg[20] & 0x08)  // G-VRAM set to buffer
	{
		if(offset < 0x40000)
			COMBINE_DATA(gvram+offset);
	}
	else
	{
		switch(m_crtc.reg[20] & 0x0300)
		{
			case 0x0300:
				if(offset < 0x40000)
					COMBINE_DATA(gvram+offset);
				break;
			case 0x0100:
				if(offset < 0x40000)
				{
					gvram[offset] = (gvram[offset] & 0xff00) | (data & 0x00ff);
				}
				if(offset >= 0x40000 && offset < 0x80000)
				{
					gvram[offset-0x40000] = (gvram[offset-0x40000] & 0x00ff) | ((data & 0x00ff) << 8);
				}
				break;
			case 0x0000:
				if(offset < 0x40000)
				{
					gvram[offset] = (gvram[offset] & 0xfff0) | (data & 0x000f);
				}
				if(offset >= 0x40000 && offset < 0x80000)
				{
					gvram[offset-0x40000] = (gvram[offset-0x40000] & 0xff0f) | ((data & 0x000f) << 4);
				}
				if(offset >= 0x80000 && offset < 0xc0000)
				{
					gvram[offset-0x80000] = (gvram[offset-0x80000] & 0xf0ff) | ((data & 0x000f) << 8);
				}
				if(offset >= 0xc0000 && offset < 0x100000)
				{
					gvram[offset-0xc0000] = (gvram[offset-0xc0000] & 0x0fff) | ((data & 0x000f) << 12);
				}
				break;
			default:
				logerror("G-VRAM written while layer setup is undefined.\n");
		}
	}
}

WRITE16_MEMBER(x68k_state::x68k_tvram_w )
{
	UINT16* tvram;
	UINT16 text_mask;

	if(m_is_32bit)
		tvram = (UINT16*)m_tvram32.target();
	else
		tvram = (UINT16*)m_tvram16.target();

	text_mask = ~(m_crtc.reg[23]) & mem_mask;

	if(!(m_crtc.reg[21] & 0x0200)) // text access mask enable
		text_mask = 0xffff & mem_mask;

	mem_mask = text_mask;

	if(m_crtc.reg[21] & 0x0100)
	{  // simultaneous T-VRAM plane access (I think ;))
		int plane,wr;
		offset = offset & 0x00ffff;
		wr = (m_crtc.reg[21] & 0x00f0) >> 4;
		for(plane=0;plane<4;plane++)
		{
			if(wr & (1 << plane))
			{
				COMBINE_DATA(tvram+offset+(0x10000*plane));
			}
		}
	}
	else
	{
		COMBINE_DATA(tvram+offset);
	}
}

READ16_MEMBER(x68k_state::x68k_gvram_r )
{
	const UINT16* gvram;
	UINT16 ret = 0;

	if(m_is_32bit)
		gvram = (const UINT16*)m_gvram32.target();
	else
		gvram = (const UINT16*)m_gvram16.target();

	if(m_crtc.reg[20] & 0x08)  // G-VRAM set to buffer
		return gvram[offset];

	switch(m_crtc.reg[20] & 0x0300)  // colour setup determines G-VRAM use
	{
		case 0x0300: // 65,536 colour (RGB) - 16-bits per word
			if(offset < 0x40000)
				ret = gvram[offset];
			else
				ret = 0xffff;
			break;
		case 0x0100:  // 256 colour (paletted) - 8 bits per word
			if(offset < 0x40000)
				ret = gvram[offset] & 0x00ff;
			if(offset >= 0x40000 && offset < 0x80000)
				ret = (gvram[offset-0x40000] & 0xff00) >> 8;
			if(offset >= 0x80000)
				ret = 0xffff;
			break;
		case 0x0000:  // 16 colour (paletted) - 4 bits per word
			if(offset < 0x40000)
				ret = gvram[offset] & 0x000f;
			if(offset >= 0x40000 && offset < 0x80000)
				ret = (gvram[offset-0x40000] & 0x00f0) >> 4;
			if(offset >= 0x80000 && offset < 0xc0000)
				ret = (gvram[offset-0x80000] & 0x0f00) >> 8;
			if(offset >= 0xc0000 && offset < 0x100000)
				ret = (gvram[offset-0xc0000] & 0xf000) >> 12;
			break;
		default:
			logerror("G-VRAM read while layer setup is undefined.\n");
			ret = 0xffff;
	}

	return ret;
}

READ16_MEMBER(x68k_state::x68k_tvram_r )
{
	const UINT16* tvram;

	if(m_is_32bit)
		tvram = (const UINT16*)m_tvram32.target();
	else
		tvram = (const UINT16*)m_tvram16.target();

	return tvram[offset];
}

READ32_MEMBER(x68k_state::x68k_tvram32_r )
{
	UINT32 ret = 0;

	if(ACCESSING_BITS_0_15)
		ret |= (x68k_tvram_r(space,(offset*2)+1,0xffff));
	if(ACCESSING_BITS_16_31)
		ret |= x68k_tvram_r(space,offset*2,0xffff) << 16;

	return ret;
}

READ32_MEMBER(x68k_state::x68k_gvram32_r )
{
	UINT32 ret = 0;

	if(ACCESSING_BITS_0_15)
		ret |= x68k_gvram_r(space,offset*2+1,0xffff);
	if(ACCESSING_BITS_16_31)
		ret |= x68k_gvram_r(space,offset*2,0xffff) << 16;

	return ret;
}

WRITE32_MEMBER(x68k_state::x68k_tvram32_w )
{
	if(ACCESSING_BITS_0_7)
		x68k_tvram_w(space,(offset*2)+1,data,0x00ff);
	if(ACCESSING_BITS_8_15)
		x68k_tvram_w(space,(offset*2)+1,data,0xff00);
	if(ACCESSING_BITS_16_23)
		x68k_tvram_w(space,offset*2,data >> 16,0x00ff);
	if(ACCESSING_BITS_24_31)
		x68k_tvram_w(space,offset*2,data >> 16,0xff00);
}

WRITE32_MEMBER(x68k_state::x68k_gvram32_w )
{
	if(ACCESSING_BITS_0_7)
		x68k_gvram_w(space,(offset*2)+1,data,0x00ff);
	if(ACCESSING_BITS_8_15)
		x68k_gvram_w(space,(offset*2)+1,data,0xff00);
	if(ACCESSING_BITS_16_23)
		x68k_gvram_w(space,offset*2,data >> 16,0x00ff);
	if(ACCESSING_BITS_24_31)
		x68k_gvram_w(space,offset*2,data >> 16,0xff00);
}

WRITE16_MEMBER(x68k_state::x68k_spritereg_w )
{
	COMBINE_DATA(m_spritereg+offset);
	switch(offset)
	{
	case 0x400:
		m_bg0_8->set_scrollx(0,(data - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
		m_bg0_16->set_scrollx(0,(data - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
		break;
	case 0x401:
		m_bg0_8->set_scrolly(0,(data - m_crtc.vbegin) & 0x3ff);
		m_bg0_16->set_scrolly(0,(data - m_crtc.vbegin) & 0x3ff);
		break;
	case 0x402:
		m_bg1_8->set_scrollx(0,(data - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
		m_bg1_16->set_scrollx(0,(data - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
		break;
	case 0x403:
		m_bg1_8->set_scrolly(0,(data - m_crtc.vbegin) & 0x3ff);
		m_bg1_16->set_scrolly(0,(data - m_crtc.vbegin) & 0x3ff);
		break;
	case 0x406:  // BG H-DISP (normally equals CRTC reg 2 value + 4)
		if(data != 0x00ff)
		{
			m_crtc.bg_visible_width = (m_crtc.reg[3] - ((data & 0x003f) - 4)) * 8;
			m_crtc.bg_hshift = ((data - (m_crtc.reg[2]+4)) * 8);
			if(m_crtc.bg_hshift > 0)
				m_crtc.bg_hshift = 0;
		}
		break;
	case 0x407:  // BG V-DISP (like CRTC reg 6)
		m_crtc.bg_vshift = m_crtc.vshift;
		break;
	case 0x408:  // BG H/V-Res
		m_crtc.bg_hvres = data & 0x1f;
		if(data != 0xff)
		{  // Handle when the PCG is using 256 and the CRTC is using 512
			if((m_crtc.bg_hvres & 0x0c) == 0x00 && (m_crtc.reg[20] & 0x0c) == 0x04)
				m_crtc.bg_double = 2;
			else
				m_crtc.bg_double = 1;
		}
		else
			m_crtc.bg_double = 1;
		break;
	}
}

READ16_MEMBER(x68k_state::x68k_spritereg_r )
{
	if(offset >= 0x400 && offset < 0x404)
		return m_spritereg[offset] & 0x3ff;
	return m_spritereg[offset];
}

WRITE16_MEMBER(x68k_state::x68k_spriteram_w )
{
	COMBINE_DATA(m_spriteram+offset);
	m_video.tile8_dirty[offset / 16] = 1;
	m_video.tile16_dirty[offset / 64] = 1;
	if(offset < 0x2000)
	{
		m_bg1_8->mark_all_dirty();
		m_bg1_16->mark_all_dirty();
		m_bg0_8->mark_all_dirty();
		m_bg0_16->mark_all_dirty();
	}
	if(offset >= 0x2000 && offset < 0x3000)
	{
		m_bg1_8->mark_tile_dirty(offset & 0x0fff);
		m_bg1_16->mark_tile_dirty(offset & 0x0fff);
	}
	if(offset >= 0x3000)
	{
		m_bg0_8->mark_tile_dirty(offset & 0x0fff);
		m_bg0_16->mark_tile_dirty(offset & 0x0fff);
	}
}

READ16_MEMBER(x68k_state::x68k_spriteram_r )
{
	return m_spriteram[offset];
}

void x68k_state::x68k_draw_text(bitmap_ind16 &bitmap, int xscr, int yscr, rectangle rect)
{
	const UINT16* tvram;
	unsigned int line,pixel; // location on screen
	UINT32 loc;  // location in TVRAM
	UINT32 colour;
	int bit;

	if(m_is_32bit)
		tvram = (const UINT16*)m_tvram32.target();
	else
		tvram = (const UINT16*)m_tvram16.target();

	for(line=rect.min_y;line<=rect.max_y;line++)  // per scanline
	{
		// adjust for scroll registers
		loc = (((line - m_crtc.vbegin) + yscr) & 0x3ff) * 64;
		loc += (xscr / 16) & 0x7f;
		loc &= 0xffff;
		bit = 15 - (xscr & 0x0f);
		for(pixel=rect.min_x;pixel<=rect.max_x;pixel++)  // per pixel
		{
			colour = (((tvram[loc] >> bit) & 0x01) ? 1 : 0)
				+ (((tvram[loc+0x10000] >> bit) & 0x01) ? 2 : 0)
				+ (((tvram[loc+0x20000] >> bit) & 0x01) ? 4 : 0)
				+ (((tvram[loc+0x30000] >> bit) & 0x01) ? 8 : 0);
			if(m_video.text_pal[colour] != 0x0000)  // any colour but black
			{
				// Colour 0 is displayable if the text layer is at the priority level 2
				if(colour == 0 && (m_video.reg[1] & 0x0c00) == 0x0800)
					bitmap.pix16(line, pixel) = 512 + (m_video.text_pal[colour] >> 1);
				else
					if(colour != 0)
						bitmap.pix16(line, pixel) = 512 + (m_video.text_pal[colour] >> 1);
			}
			bit--;
			if(bit < 0)
			{
				bit = 15;
				loc++;
				loc &= 0xffff;
			}
		}
	}
}

void x68k_state::x68k_draw_gfx_scanline( bitmap_ind16 &bitmap, rectangle cliprect, UINT8 priority)
{
	const UINT16* gvram;
	int pixel;
	int page;
	UINT32 loc;  // location in GVRAM
	UINT32 lineoffset;
	UINT16 xscr,yscr;
	UINT16 colour = 0;
	int shift;
	int scanline;

	if(m_is_32bit)
		gvram = (const UINT16*)m_gvram32.target();
	else
		gvram = (const UINT16*)m_gvram16.target();

	for(scanline=cliprect.min_y;scanline<=cliprect.max_y;scanline++)  // per scanline
	{
		if(m_crtc.reg[20] & 0x0400)  // 1024x1024 "real" screen size - use 1024x1024 16-colour gfx layer
		{
			// adjust for scroll registers
			if(m_video.reg[2] & 0x0010 && priority == m_video.gfxlayer_pri[0])
			{
				xscr = (m_crtc.reg[12] & 0x3ff);
				yscr = (m_crtc.reg[13] & 0x3ff);
				lineoffset = (((scanline - m_crtc.vbegin) + yscr) & 0x3ff) * 1024;
				loc = xscr & 0x3ff;
				for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
				{
					switch(lineoffset & 0xc0000)
					{
					case 0x00000:
						colour = gvram[lineoffset + (loc & 0x3ff)] & 0x000f;
						break;
					case 0x40000:
						colour = (gvram[(lineoffset - 0x40000) + (loc & 0x3ff)] & 0x00f0) >> 4;
						break;
					case 0x80000:
						colour = (gvram[(lineoffset - 0x80000) + (loc & 0x3ff)] & 0x0f00) >> 8;
						break;
					case 0xc0000:
						colour = (gvram[(lineoffset - 0xc0000) + (loc & 0x3ff)] & 0xf000) >> 12;
						break;
					}
					if(colour != 0)
						bitmap.pix16(scanline, pixel) = 512 + (m_video.gfx_pal[colour] >> 1);
					loc++;
					loc &= 0x3ff;
				}
			}
		}
		else  // else 512x512 "real" screen size
		{
			if(m_video.reg[2] & (1 << priority))
			{
				page = m_video.gfxlayer_pri[priority];
				// adjust for scroll registers
				switch(m_video.reg[0] & 0x03)
				{
				case 0x00: // 16 colours
					xscr = ((m_crtc.reg[12+(page*2)])) & 0x1ff;
					yscr = ((m_crtc.reg[13+(page*2)])) & 0x1ff;
					lineoffset = (((scanline - m_crtc.vbegin) + yscr) & 0x1ff) * 512;
					loc = xscr & 0x1ff;
					shift = 4;
					for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
					{
						colour = ((gvram[lineoffset + loc] >> page*shift) & 0x000f);
						if(colour != 0)
							bitmap.pix16(scanline, pixel) = 512 + (m_video.gfx_pal[colour & 0x0f] >> 1);
						loc++;
						loc &= 0x1ff;
					}
					break;
				case 0x01: // 256 colours
					if(page == 0 || page == 2)
					{
						xscr = ((m_crtc.reg[12+(page*2)])) & 0x1ff;
						yscr = ((m_crtc.reg[13+(page*2)])) & 0x1ff;
						lineoffset = (((scanline - m_crtc.vbegin) + yscr) & 0x1ff) * 512;
						loc = xscr & 0x1ff;
						shift = 4;
						for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
						{
							colour = ((gvram[lineoffset + loc] >> page*shift) & 0x00ff);
							if(colour != 0)
								bitmap.pix16(scanline, pixel) = 512 + (m_video.gfx_pal[colour & 0xff] >> 1);
							loc++;
							loc &= 0x1ff;
						}
					}
					break;
				case 0x03: // 65536 colours
					xscr = ((m_crtc.reg[12])) & 0x1ff;
					yscr = ((m_crtc.reg[13])) & 0x1ff;
					lineoffset = (((scanline - m_crtc.vbegin) + yscr) & 0x1ff) * 512;
					loc = xscr & 0x1ff;
					for(pixel=m_crtc.hbegin;pixel<=m_crtc.hend;pixel++)
					{
						colour = gvram[lineoffset + loc];
						if(colour != 0)
							bitmap.pix16(scanline, pixel) = 512 + (colour >> 1);
						loc++;
						loc &= 0x1ff;
					}
					break;
				}
			}
		}
	}
}

void x68k_state::x68k_draw_gfx(bitmap_ind16 &bitmap,rectangle cliprect)
{
	int priority;
	//rectangle rect;
	//int xscr,yscr;
	//int gpage;

	if(m_crtc.reg[20] & 0x0800)  // if graphic layers are set to buffer, then they aren't visible
		return;

	for(priority=3;priority>=0;priority--)
	{
		x68k_draw_gfx_scanline(bitmap,cliprect,priority);
	}
}

// Sprite controller "Cynthia" at 0xeb0000
void x68k_state::x68k_draw_sprites(bitmap_ind16 &bitmap, int priority, rectangle cliprect)
{
	/*
	   0xeb0000 - 0xeb07ff - Sprite registers (up to 128)
	       + 00 : b9-0,  Sprite X position
	       + 02 : b9-0,  Sprite Y position
	       + 04 : b15,   Vertical Reversing (flipping?)
	              b14,   Horizontal Reversing
	              b11-8, Sprite colour
	              b7-0,  Sprite tile code (in PCG)
	       + 06 : b1-0,  Priority
	                     00 = Sprite not displayed

	   0xeb0800 - BG0 X Scroll  (10-bit)
	   0xeb0802 - BG0 Y Scroll
	   0xeb0804 - BG1 X Scroll
	   0xeb0806 - BG1 Y Scroll
	   0xeb0808 - BG control
	              b9,    BG/Sprite display (RAM and register access is faster if 1)
	              b4,    PCG area 1 available
	              b3,    BG1 display enable
	              b1,    PCG area 0 available
	              b0,    BG0 display enable
	   0xeb080a - Horizontal total (like CRTC reg 0 - is 0xff if in 256x256?)
	   0xeb080c - Horizontal display position (like CRTC reg 2 - +4)
	   0xeb080e - Vertical display position (like CRTC reg 6)
	   0xeb0810 - Resolution setting
	              b4,    "L/H" (apparently 15kHz/31kHz switch for sprites/BG?)
	              b3-2,  V-Res
	              b1-0,  H-Res (0 = 8x8 tilemaps, 1 = 16x16 tilemaps, 2 or 3 = unknown)
	*/
	int ptr,pri;

	for(ptr=508;ptr>=0;ptr-=4)  // stepping through sprites
	{
		pri = m_spritereg[ptr+3] & 0x03;
#ifdef MAME_DEBUG
		if(!(machine().input().code_pressed(KEYCODE_I)))
#endif
		if(pri == priority)
		{  // if at the right priority level, draw the sprite
			rectangle rect;
			int code = m_spritereg[ptr+2] & 0x00ff;
			int colour = (m_spritereg[ptr+2] & 0x0f00) >> 8;
			int xflip = m_spritereg[ptr+2] & 0x4000;
			int yflip = m_spritereg[ptr+2] & 0x8000;
			int sx = (m_spritereg[ptr+0] & 0x3ff) - 16;
			int sy = (m_spritereg[ptr+1] & 0x3ff) - 16;

			rect.min_x=m_crtc.hshift;
			rect.min_y=m_crtc.vshift;
			rect.max_x=rect.min_x + m_crtc.visible_width-1;
			rect.max_y=rect.min_y + m_crtc.visible_height-1;

			sx += m_crtc.bg_hshift;
			sx += m_sprite_shift;

			drawgfxzoom_transpen(bitmap,cliprect,machine().gfx[1],code,colour+0x10,xflip,yflip,m_crtc.hbegin+sx,m_crtc.vbegin+(sy*m_crtc.bg_double),0x10000,0x10000*m_crtc.bg_double,0x00);
		}
	}
}

PALETTE_INIT_MEMBER(x68k_state,x68000)
{
	int pal;
	int r,g,b;

	for(pal=0;pal<32768;pal++)
	{  // create 64k colour lookup
		g = (pal & 0x7c00) >> 7;
		r = (pal & 0x03e0) >> 2;
		b = (pal & 0x001f) << 3;
		palette_set_color_rgb(machine(),pal+512,r,g,b);
	}
}

static const gfx_layout x68k_pcg_8 =
{
	8,8,
	256,
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28,16,20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout x68k_pcg_16 =
{
	16,16,
	256,
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28,16,20,8+64*8,12+64*8,64*8,4+64*8,24+64*8,28+64*8,16+64*8,20+64*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

#if 0
static GFXDECODEINFO_START( x68k )
	GFXDECODE_ENTRY( "user1", 0, x68k_pcg_8, 0x100, 16 )  // 8x8 sprite tiles
	GFXDECODE_ENTRY( "user1", 0, x68k_pcg_16, 0x100, 16 )  // 16x16 sprite tiles
GFXDECODEINFO_END
#endif

TILE_GET_INFO_MEMBER(x68k_state::x68k_get_bg0_tile)
{
	int code = m_spriteram[0x3000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x3000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x3000+tile_index] & 0xc000) >> 14;
	SET_TILE_INFO_MEMBER(0,code,colour+16,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::x68k_get_bg1_tile)
{
	int code = m_spriteram[0x2000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x2000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x2000+tile_index] & 0xc000) >> 14;
	SET_TILE_INFO_MEMBER(0,code,colour+16,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::x68k_get_bg0_tile_16)
{
	int code = m_spriteram[0x3000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x3000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x3000+tile_index] & 0xc000) >> 14;
	SET_TILE_INFO_MEMBER(1,code,colour+16,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::x68k_get_bg1_tile_16)
{
	int code = m_spriteram[0x2000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x2000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x2000+tile_index] & 0xc000) >> 14;
	SET_TILE_INFO_MEMBER(1,code,colour+16,flags);
}

VIDEO_START_MEMBER(x68k_state,x68000)
{
	int gfx_index;

	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine().gfx[gfx_index] == 0)
			break;

	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine().gfx[gfx_index] = auto_alloc(machine(), gfx_element(machine(), x68k_pcg_8, machine().root_device().memregion("user1")->base(), 32, 0));

	gfx_index++;

	machine().gfx[gfx_index] = auto_alloc(machine(), gfx_element(machine(), x68k_pcg_16, memregion("user1")->base(), 32, 0));
	machine().gfx[gfx_index]->set_colors(32);

	/* Tilemaps */
	m_bg0_8 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(x68k_state::x68k_get_bg0_tile),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_bg1_8 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(x68k_state::x68k_get_bg1_tile),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_bg0_16 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(x68k_state::x68k_get_bg0_tile_16),this),TILEMAP_SCAN_ROWS,16,16,64,64);
	m_bg1_16 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(x68k_state::x68k_get_bg1_tile_16),this),TILEMAP_SCAN_ROWS,16,16,64,64);

	m_bg0_8->set_transparent_pen(0);
	m_bg1_8->set_transparent_pen(0);
	m_bg0_16->set_transparent_pen(0);
	m_bg1_16->set_transparent_pen(0);

//  m_scanline_timer->adjust(attotime::zero, 0, attotime::from_hz(55.45)/568);
}

UINT32 x68k_state::screen_update_x68000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle rect(0,0,0,0);
	int priority;
	int xscr,yscr;
	int x;
	tilemap_t* x68k_bg0;
	tilemap_t* x68k_bg1;
	//UINT8 *rom;

	if((m_spritereg[0x408] & 0x03) == 0x00)  // Sprite/BG H-Res 0=8x8, 1=16x16, 2 or 3 = undefined.
	{
		x68k_bg0 = m_bg0_8;
		x68k_bg1 = m_bg1_8;
	}
	else
	{
		x68k_bg0 = m_bg0_16;
		x68k_bg1 = m_bg1_16;
	}
//  rect.max_x=m_crtc.width;
//  rect.max_y=m_crtc.height;
	bitmap.fill(0, cliprect);

	if(m_sysport.contrast == 0)  // if monitor contrast is 0, then don't bother displaying anything
		return 0;

	rect.min_x=m_crtc.hbegin;
	rect.min_y=m_crtc.vbegin;
//  rect.max_x=rect.min_x + m_crtc.visible_width-1;
//  rect.max_y=rect.min_y + m_crtc.visible_height-1;
	rect.max_x=m_crtc.hend;
	rect.max_y=m_crtc.vend;

	if(rect.min_y < cliprect.min_y)
		rect.min_y = cliprect.min_y;
	if(rect.max_y > cliprect.max_y)
		rect.max_y = cliprect.max_y;

	// update tiles
	//rom = machine().root_device().memregion("user1")->base();
	for(x=0;x<256;x++)
	{
		if(m_video.tile16_dirty[x] != 0)
		{
			machine().gfx[1]->mark_dirty(x);
			m_video.tile16_dirty[x] = 0;
		}
		if(m_video.tile8_dirty[x] != 0)
		{
			machine().gfx[0]->mark_dirty(x);
			m_video.tile8_dirty[x] = 0;
		}
	}

	for(priority=3;priority>=0;priority--)
	{
		// Graphics screen(s)
		if(priority == m_video.gfx_pri)
			x68k_draw_gfx(bitmap,rect);

		// Sprite / BG Tiles
		if(priority == m_video.sprite_pri /*&& (m_spritereg[0x404] & 0x0200)*/ && (m_video.reg[2] & 0x0040))
		{
			x68k_draw_sprites(bitmap,1,rect);
			if((m_spritereg[0x404] & 0x0008))
			{
				if((m_spritereg[0x404] & 0x0030) == 0x10)  // BG1 TXSEL
				{
					x68k_bg0->set_scrollx(0,(m_spritereg[0x402] - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
					x68k_bg0->set_scrolly(0,(m_spritereg[0x403] - m_crtc.vbegin) & 0x3ff);
					x68k_bg0->draw(bitmap,rect,0,0);
				}
				else
				{
					x68k_bg1->set_scrollx(0,(m_spritereg[0x402] - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
					x68k_bg1->set_scrolly(0,(m_spritereg[0x403] - m_crtc.vbegin) & 0x3ff);
					x68k_bg1->draw(bitmap,rect,0,0);
				}
			}
			x68k_draw_sprites(bitmap,2,rect);
			if((m_spritereg[0x404] & 0x0001))
			{
				if((m_spritereg[0x404] & 0x0006) == 0x02)  // BG0 TXSEL
				{
					x68k_bg0->set_scrollx(0,(m_spritereg[0x400] - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
					x68k_bg0->set_scrolly(0,(m_spritereg[0x401] - m_crtc.vbegin) & 0x3ff);
					x68k_bg0->draw(bitmap,rect,0,0);
				}
				else
				{
					x68k_bg1->set_scrollx(0,(m_spritereg[0x400] - m_crtc.hbegin - m_crtc.bg_hshift) & 0x3ff);
					x68k_bg1->set_scrolly(0,(m_spritereg[0x401] - m_crtc.vbegin) & 0x3ff);
					x68k_bg1->draw(bitmap,rect,0,0);
				}
			}
			x68k_draw_sprites(bitmap,3,rect);
		}

		// Text screen
		if(m_video.reg[2] & 0x0020 && priority == m_video.text_pri)
		{
			xscr = (m_crtc.reg[10] & 0x3ff);
			yscr = (m_crtc.reg[11] & 0x3ff);
			if(!(m_crtc.reg[20] & 0x1000))  // if text layer is set to buffer, then it's not visible
				x68k_draw_text(bitmap,xscr,yscr,rect);
		}
	}

#ifdef MAME_DEBUG
	if(machine().input().code_pressed(KEYCODE_I))
	{
		m_mfp.isra = 0;
		m_mfp.isrb = 0;
//      mfp_trigger_irq(MFP_IRQ_GPIP6);
//      machine().device("maincpu")->execute().set_input_line_and_vector(6,ASSERT_LINE,0x43);
	}
	if(machine().input().code_pressed(KEYCODE_9))
	{
		m_sprite_shift--;
		popmessage("Sprite shift = %i",m_sprite_shift);
	}
	if(machine().input().code_pressed(KEYCODE_0))
	{
		m_sprite_shift++;
		popmessage("Sprite shift = %i",m_sprite_shift);
	}

#endif

#ifdef MAME_DEBUG
//  popmessage("Layer priorities [%04x] - Txt: %i  Spr: %i  Gfx: %i  Layer Pri0-3: %i %i %i %i",m_video.reg[1],m_video.text_pri,m_video.sprite_pri,
//      m_video.gfx_pri,m_video.gfxlayer_pri[0],m_video.gfxlayer_pri[1],m_video.gfxlayer_pri[2],m_video.gfxlayer_pri[3]);
//  popmessage("CRTC regs - %i %i %i %i  - %i %i %i %i - %i - %i",m_crtc.reg[0],m_crtc.reg[1],m_crtc.reg[2],m_crtc.reg[3],
//      m_crtc.reg[4],m_crtc.reg[5],m_crtc.reg[6],m_crtc.reg[7],m_crtc.reg[8],m_crtc.reg[9]);
//  popmessage("Visible resolution = %ix%i (%s) Screen size = %ix%i",m_crtc.visible_width,m_crtc.visible_height,m_crtc.interlace ? "Interlaced" : "Non-interlaced",m_crtc.video_width,m_crtc.video_height);
//  popmessage("VBlank : scanline = %i",m_scanline);
//  popmessage("CRTC/BG compare H-TOTAL %i/%i H-DISP %i/%i V-DISP %i/%i BG Res %02x",m_crtc.reg[0],m_spritereg[0x405],m_crtc.reg[2],m_spritereg[0x406],
//      m_crtc.reg[6],m_spritereg[0x407],m_spritereg[0x408]);
//  popmessage("IER %02x %02x  IPR %02x %02x  ISR %02x %02x  IMR %02x %02x", m_mfp.iera,m_mfp.ierb,m_mfp.ipra,m_mfp.iprb,
//      m_mfp.isra,m_mfp.isrb,m_mfp.imra,m_mfp.imrb);
//  popmessage("BG Scroll - BG0 X %i Y %i  BG1 X %i Y %i",m_spriteram[0x400],m_spriteram[0x401],m_spriteram[0x402],m_spriteram[0x403]);
//  popmessage("Keyboard buffer position = %i",m_keyboard.headpos);
//  popmessage("IERA = 0x%02x, IERB = 0x%02x",m_mfp.iera,m_mfp.ierb);
//  popmessage("IPRA = 0x%02x, IPRB = 0x%02x",m_mfp.ipra,m_mfp.iprb);
//  popmessage("uPD72065 status = %02x",upd765_status_r(machine(), space, 0));
//  popmessage("Layer enable - 0x%02x",m_video.reg[2] & 0xff);
//  popmessage("Graphic layer scroll - %i, %i - %i, %i - %i, %i - %i, %i",
//      m_crtc.reg[12],m_crtc.reg[13],m_crtc.reg[14],m_crtc.reg[15],m_crtc.reg[16],m_crtc.reg[17],m_crtc.reg[18],m_crtc.reg[19]);
//  popmessage("IOC IRQ status - %02x",m_ioc.irqstatus);
//  popmessage("RAM: mouse data - %02x %02x %02x %02x",machine().device<ram_device>(RAM_TAG)->pointer()[0x931],machine().device<ram_device>(RAM_TAG)->pointer()[0x930],machine().device<ram_device>(RAM_TAG)->pointer()[0x933],machine().device<ram_device>(RAM_TAG)->pointer()[0x932]);
#endif
	return 0;
}
