/******************************************************************************************************

    System H1 (c) 1994 Sega

    preliminary driver by David Haywood, Angelo Salese and Tomasz Slanina
    special thanks to Guru for references and HW advices

    TODO:
    - decode compressed GFX ROMs for "sprite" blitter (6,141,122 is the patent number)
    - DMA is still a bit of a mystery;
    - video emulation is pratically non-existant;
    - SCSP;
    - Many SH-1 ports needs investigations;
    - IRQ generation
    - Understand & remove the hacks at the bottom;
    - IC1/IC10 are currently unused, might contain sprite data / music data for the SCSP / chars for the
      text tilemap/blitter;

=======================================================================================================

Cool Riders
Sega 1994

This game runs on SYSTEM-H1 hardware. Only one known game exists on this
PCB and this is it. The hardware seems overly complex for a 2D bike
racing game? The design of the PCB is very similar to vanilla Model 2
(i.e. Daytona etc). However instead of fully custom-badged chips,
many of the custom chips are off-the-shelf Hitachi/Toshiba gate-arrays.


PCB Layouts
-----------

SYSTEM-H1 ROM BD
171-6516C
837-9623
834-11482 (sticker)
|--------------------------------------------------------------|
|  IC17            IC18              IC5               IC10    |
|                                                              |
|  IC15            IC16              IC4               IC9     |
|                                                              |
|  IC13            IC14              IC3               IC8     |
|                                                              |
|  IC11            IC12              IC2               IC7     |
|                                                              |
|  IC31            IC32              IC1               IC6     |
|                                                              |
|  IC29            IC30                                        |
|JP1-JP8                                                       |
|  LED    32.500MHz   CN1         JP9-JP12    CN2              |
|--------------------------------------------------------------|
Notes:
      CN1/2   - Connectors joining to CPU board
      JP1-8   - Jumpers to set ROM sizes
                JP1-4 set to 1-2
                JP5-8 set to 2-3
      JP9-12  - Jumpers to set ROM sizes
                JP9 set to 1-2
                JP10-12 open (no jumpers) but JP12 pin 2 tied to JP10 pin 1
      IC*     - IC29-IC32 are 27C4002 EPROM
                IC1-IC10 are DIP42 32M mask ROM
                IC11-IC18 are DIP42 16M mask ROM


SYSTEM-H1 COMMUNICATION-BD
171-6849B
837-10942
|----------------------------------|
|                 CN2              |
|       74F74   74F245  MB84256    |
|       74F373  74F245  MB84256    |
|MB89237A         CN1              |
|                                  |
|       74F138 74F04   74F125      |
|                                  |
|       74F157 74F161  74F02       |
|                                  |
|       74F04  74F74   74F86     TX|
|MB89374                           |
|       74F02  74F160            RX|
|                         SN75179  |
|                     JP1 JP2 JP3  |
|       LED               CN3      |
|----------------------------------|
Notes: (All IC's shown)
      CN1/2   - Connectors joining to CPU board
      CN3     - Connector joining to Filter board
      RX/TX   - Optical cable connections for network (not used)
      JP*     - 3x 2-pin jumpers. JP1 shorted, other jumpers open
      MB84256 - Fujitsu MB84256 32k x8 SRAM (NDIP28)
      MB89374 - Fujitsu MB89374 Data Link Controller (SDIP42)
      MB89237A- Fujitsu MB89237A 8-Bit Proprietary Microcontroller (?) (DIP40)
      SN75179 - Texas Instruments SN75179 Differential Driver and Receiver Pair (DIP8)


SYSTEM-H1 CPU BD
171-6651A
837-10389
837-11481 (sticker)
|--------------------------------------------------------------|
|                                                EPR-17662.IC12|
|                                                              |
|   |--------|    |--------|                                   |
|   |SEGA    |    |SEGA    |           FM1208S      SEC_CONN   |
|   |315-5758|    |315-5757|       CN2         CN1             |
|   |        |    |        |           |------|            CN10|
|   |--------|    |--------|           |SH7032|                |
|CN8                                   |      |                |
|                                      |------|                |
|                                                              |
|                   PAL1  JP1 JP4 JP6 JP2                      |
|                   PAL2   JP3 JP5     MB3771                  |
|     28MHz   32MHz              PC910                 A1603C  |
|                                               DSW1   DAN803  |
|HM5241605                       |--------| |--------| DAP803  |
|HM5241605      PAL3             |SEGA    | |SEGA    |         |
|                                |315-5687| |315-5687| 315-5649|
|                                |        | |        |         |
|   |-----|        TMP68HC000N-16|--------| |--------|         |
|   |SH2  |                       514270      514270           |
|   |     |                         CN12               A1603C  |
|CN7|-----|         MB84256            TDA1386   TL062     CN11|
|                           22.579MHz   CN14                   |
|                   MB84256                                    |
|                                                              |
|                                      TDA1386   TL062         |
|--------------------------------------------------------------|
Notes:
      22.579MHz   - This OSC is tied to pin 1 of a 74AC04 logic chip. The output from that (pin 2) is tied
                    directly to both 315-5687 chips on pin 14. Therefore the clock input of the YMF292's is 22.579MHz
      514270      - Hitachi HM514270AJ-7 256k x16 DRAM (SOJ40)
      68000       - Clock 16.00MHz [32/2]
      A1603C      - NEC uPA1603C Monolithic N-Channel Power MOS FET Array (DIP16)
      CN7/8       - Connectors joining to ROM board (above)
      CN10/11     - Connectors joining to Filter board
      CN12/14     - Connectors for (possible) extra sound board (not used)
      DAN803      - Diotec Semiconductor DAN803 Small Signal Diode Array with common anodes (SIL9)
      DAP803      - Diotec Semiconductor DAP803 Small Signal Diode Array with common cathodes (SIL9)
      DSW1        - 4-position DIP switch. All OFF
      EPR-17662   - Toshiba TC57H1025 1M EPROM (DIP40)
      FM1208S     - RAMTRON FM1208S 4k (512 bytes x8) Nonvolatile Ferroelectric RAM (SOIC24)
      HM5241605   - Hitachi HM5241605 4M (256k x 16 x 2 banks) SDRAM (SSOP50)
      JP1-6       - Jumpers. JP2 open. JP5 1-2. All others 2-3
      MB3771      - Fujitsu MB3771 Master Reset IC (SOIC8)
      MB84256     - Fujitsu MB84256 32k x8 SRAM (SOP28)
      PAL1        - GAL16V8B also marked '315-5800' (DIP20)
      PAL2        - GAL16V8B also marked '315-5802' (DIP20)
      PAL3        - GAL16V8B also marked '315-5801' (DIP20)
      PC910       - Sharp PC910 opto-isolator (DIP8)
      SEC_CONN    - Sega security-board connector (not used)
      SH7032      - Hitachi HD6417032F20 ROMless SH1 CPU (QFP112). Clock input 16.00MHz on pin 71
      SH2         - Hitachi HD6417095 SH2 CPU (QFP144). Clock input 28.00MHz on pin 118
      TDA1386     - Philips TDA1386T Noise Shaping Filter DAC (SOP24)
      TL062       - Texas Instruments TL062 Low Power JFET Input Operational Amplifier (SOIC8)
      Sega Custom - 315-5757 (QFP160)
                    315-5758 (QFP168) also marked 'HG62G035R26F'
                    315-5649 (QFP100) custom I/O chip (also used on Model 2A/2B/2C, but NOT vanilla Model 2)
                    315-5687 (QFP128 x2) also marked 'YMF292-F' (also used on Model 2A/2B/2C and ST-V)
      Syncs       - Horizontal 24.24506kHz
                    Vertical 57.0426Hz


SYSTEM-H1 VIDEO BD
171-6514F
837-9621
|--------------------------------------------------------------|
|         CN2                                                  |
|                                                           JP4|
|                                   |--------|  TC55328 D431008|
|      |--------|          HM514270 |SEGA    |  TC55328 D431008|
|      |SEGA    |          HM514270 |315-5697|              JP3|
|CN1   |315-5691|  TC55328 HM514270 |        |  D431008 D431008|
|      |        |  TC55328 HM514270 |--------|  D431008 D431008|
|      |--------|                                           JP2|
|                                                           JP1|
|                                     315-5698   315-5648      |
|        |------------|                                        |
|        |  SEGA      |      50MHz                             |
|        |  315-5692  |               315-5696   315-5648      |
|        |            |                                     CN9|
|LED     |------------|                                        |
|40MHz     |--------|                 315-5696   315-5648      |
|          |SEGA    |                                          |
|   PAL1   |315-5693|                                          |
|CN4       |        |       315-5695  315-5695   315-5648      |
||--------||--------|                                          |
||SEGA    |                                                    |
||315-5694||--------| M5M411860 M5M411860 M5M411860 M5M411860  |
||        ||SEGA    | M5M411860 M5M411860 M5M411860 M5M411860  |
||--------||315-5693| M5M411860 M5M411860 M5M411860 M5M411860  |
|   PAL2   |        | M5M411860 M5M411860 M5M411860 M5M411860  |
|          |--------|                                          |
|--------------------------------------------------------------|
Notes:
      CN9         - Connector joining to Filter board
      CN1/2/4     - Connectors joining to CPU board
      JP1/2/3/4   - 4x 3-pin jumpers. All set to 1-2
      D431008     - NEC D431008 128k x8 SRAM (SOJ32)
      HM514270    - Hitachi HM514270AJ7 256k x16 DRAM (SOJ40)
      M5M411860   - Mitsubishi M5M411860TP435SF00-7 DRAM with fast page mode, 64k-words x 18 bits per word (maybe?) (TSOP42)
      TC55328     - Toshiba TC55328AJ-15 32k x8 SRAM (SOJ24)
      PAL1        - GAL16V8B also marked '315-5803' (DIP20)
      PAL2        - GAL16V8B also marked '315-5864' (DIP20)
      Sega Custom - 315-5648 (QFP64, x4)
                    315-5691 also marked 'HG62S0791R17F' (QFP208)
                    315-5692 also marked 'HG51B152FD' (QFP256)
                    315-5693 also marked 'HG62G019R16F' (QFP168, x3)
                    315-5694 (QFP208)
                    315-5695 (QFP100, x2)
                    315-5696 (QFP120, x2)
                    315-5697 (QFP208)
                    315-5698 (QFP144)

*******************************************************************************************************

Note: This hardware appears to have been designed as a test-bed for a new RLE based compression system
      used by the zooming sprites.  It is possible that Sega planned on using this for ST-V, but
      decided against it. Video/CPU part numbers give an interesting insight, since video hardware #
      sits between Model 1 & Model 2.

                 Year on
      System      PCB     PCB #      PALs
      ---------------------------------------------------------
      System32    1990    837-7428   315-5441 315-5442
      SysMulti32  1992    837-8676   315-5596
      Model 1     1992    837-8886   315-5546 315-5483 315-5484
      Model 2     1994    837-10071  315-5737 315-5741
      Model 2A    1994    837-10848  315-5737 315-5815
      STV         1994    837-10934  315-5833

      H1 (CPU)    1994    837-10389  315-5800 315-5801 315-5802
      H1 (Video)  1994    837-9621   315-5803 315-5864


   NOTE:  While the hardware and title screen might list 1994 as a copyright, MAME uses 1995 due to the
   abudance of evidence in trade journals and even it's own service manuals showing the year as 1995.

   References:
   Arcade game magazine called 'Gamest' show released on 04.28.1995
   VGL (Ultimate Video Game List published by AMP group) - year is printed as '94(4.95)'
   Sega Arcade History (published by Enterbrain) is '1995/4'.


******************************************************************************************************/


#include "emu.h"
#include "debugger.h"
#include "cpu/sh2/sh2.h"
#include "cpu/m68000/m68000.h"
#include "sound/scsp.h"


class coolridr_state : public driver_device
{
public:
	coolridr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_textBytesToWrite(0x00),
		m_blitterSerialCount(0x00),
		m_blitterMode(0x00),
		m_textOffset(0x0000),
		m_colorNumber(0x00000000),
		m_vCellCount(0x0000),
		m_hCellCount(0x0000),
		m_vPosition(0x0000),
		m_hPosition(0x0000),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this,"sub"),
		m_soundcpu(*this,"soundcpu"),
		m_h1_vram(*this, "h1_vram"),
		m_h1_charram(*this, "h1_charram"),
		m_framebuffer_vram(*this, "fb_vram"),
		m_txt_vram(*this, "txt_vram"),
		m_sysh1_txt_blit(*this, "sysh1_txt_blit"),
		m_sysh1_workram_h(*this, "sysh1_workrah"),
		m_h1_unk(*this, "h1_unk"){ }

	// Blitter state
	UINT16 m_textBytesToWrite;
	INT16  m_blitterSerialCount;
	UINT8  m_blitterMode;
	UINT16 m_textOffset;
	UINT32 m_colorNumber;
	UINT16 m_vCellCount;
	UINT16 m_hCellCount;
	UINT16 m_vPosition;
	UINT16 m_hPosition;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_soundcpu;

	required_shared_ptr<UINT32> m_h1_vram;
	required_shared_ptr<UINT32> m_h1_charram;
	required_shared_ptr<UINT32> m_framebuffer_vram;
	required_shared_ptr<UINT32> m_txt_vram;
	required_shared_ptr<UINT32> m_sysh1_txt_blit;
	required_shared_ptr<UINT32> m_sysh1_workram_h;
	required_shared_ptr<UINT32> m_h1_unk;
	bitmap_rgb32 m_temp_bitmap_sprites;
	UINT32 m_test_offs;
	int m_color;
	UINT8 m_vblank;



	DECLARE_READ32_MEMBER(sysh1_unk_r);
	DECLARE_WRITE32_MEMBER(sysh1_unk_w);
	DECLARE_READ32_MEMBER(sysh1_ioga_r);
	DECLARE_WRITE32_MEMBER(sysh1_ioga_w);
	DECLARE_WRITE32_MEMBER(sysh1_txt_blit_w);
	DECLARE_WRITE32_MEMBER(sysh1_pal_w);
	DECLARE_WRITE32_MEMBER(sysh1_dma_w);
	DECLARE_WRITE32_MEMBER(sysh1_char_w);
	DECLARE_READ32_MEMBER(coolridr_hack1_r);
	DECLARE_READ32_MEMBER(coolridr_hack2_r);
	DECLARE_DRIVER_INIT(coolridr);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_coolridr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(system_h1);
	TIMER_DEVICE_CALLBACK_MEMBER(system_h1_sub);
};



/* video */

void coolridr_state::video_start()
{
	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites);
	m_test_offs = 0x2000;
}

UINT32 coolridr_state::screen_update_coolridr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* planes seems to basically be at 0x8000 and 0x28000... */
	gfx_element *gfx = machine().gfx[2];
	UINT32 count;
	int y,x;


	if(machine().input().code_pressed(KEYCODE_Z))
		m_test_offs+=4;

	if(machine().input().code_pressed(KEYCODE_X))
		m_test_offs-=4;

	if(machine().input().code_pressed(KEYCODE_C))
		m_test_offs+=0x40;

	if(machine().input().code_pressed(KEYCODE_V))
		m_test_offs-=0x40;

	if(machine().input().code_pressed(KEYCODE_B))
		m_test_offs+=0x400;

	if(machine().input().code_pressed(KEYCODE_N))
		m_test_offs-=0x400;

	if(machine().input().code_pressed_once(KEYCODE_A))
		m_color++;

	if(machine().input().code_pressed_once(KEYCODE_S))
		m_color--;

	if(m_test_offs > 0x100000*4)
		m_test_offs = 0;

	count = m_test_offs/4;

	popmessage("%08x %04x",m_test_offs,m_color);

	for (y=0;y<64;y++)
	{
		for (x=0;x<128;x+=2)
		{
			int tile;

			tile = (m_h1_vram[count] & 0x0fff0000) >> 16;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,m_color,0,0,(x+0)*16,y*16);

			tile = (m_h1_vram[count] & 0x00000fff) >> 0;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,m_color,0,0,(x+1)*16,y*16);

			count++;
		}
	}

	copybitmap_trans(bitmap, m_temp_bitmap_sprites, 0, 0, 0, 0, cliprect, 0);
	m_temp_bitmap_sprites.fill(0, cliprect);


	return 0;
}

/* end video */

/* unknown purpose */
READ32_MEMBER(coolridr_state::sysh1_unk_r)
{
	switch(offset)
	{
		case 0x08/4:
		{

			m_vblank^=1;

			return (m_h1_unk[offset] & 0xfdffffff) | (m_vblank<<25);
		}
		case 0x14/4:
			return m_h1_unk[offset];
		//case 0x20/4:
	}

	return 0xffffffff;//m_h1_unk[offset];
}

WRITE32_MEMBER(coolridr_state::sysh1_unk_w)
{
	COMBINE_DATA(&m_h1_unk[offset]);
}

/* According to Guru, this is actually the same I/O chip of Sega Model 2 HW */
#if 0
READ32_MEMBER(coolridr_state::sysh1_ioga_r)
{
	//return machine().rand();//h1_ioga[offset];
	return h1_ioga[offset];
}

WRITE32_MEMBER(coolridr_state::sysh1_ioga_w)
{
	COMBINE_DATA(&h1_ioga[offset]);
}
#endif


/* This is a RLE-based sprite blitter (US Patent #6,141,122), very unusual from Sega... */
WRITE32_MEMBER(coolridr_state::sysh1_txt_blit_w)
{

	COMBINE_DATA(&m_sysh1_txt_blit[offset]);

	switch(offset)
	{
		// The mode register
		case 0x04:
		{
			m_blitterMode = (data & 0x00ff0000) >> 16;

			if (m_blitterMode == 0xf4)
			{
				// Some sort of addressing state.
				// In the case of text, simply writes 4 characters per 32-bit word.
				// These values may be loaded into RAM somewhere as they are written.
				// The number of characters is determined by the upper-most 8 bits.
				m_textBytesToWrite = (data & 0xff000000) >> 24;
				m_textOffset = (data & 0x0000ffff);
				m_blitterSerialCount = 0;
			}
			else if (m_blitterMode == 0x30 || m_blitterMode == 0x90)
			{
				// The blitter function(s).
				// After this is set a fixed count of 11 32-bit words are sent to the data register.
				// The lower word always seems to be 0x0001 and the upper byte always 0xac.
				m_blitterSerialCount = 0;
			}
			else if (m_blitterMode == 0x10)
			{
				// Could be a full clear of VRAM?
				for(UINT32 vramAddr = 0x3f40000; vramAddr < 0x3f4ffff; vramAddr+=4)
					space.write_dword(vramAddr, 0x00000000);
			}
			break;
		}

		// The data register
		case 0x05:
		{
			if (m_blitterMode == 0xf4)
			{
				// Uploads a series of bytes that index into the encoded sprite table
				const size_t memOffset = 0x03f40000 + m_textOffset + m_blitterSerialCount;
				space.write_dword(memOffset, data);
				m_blitterSerialCount += 0x04;

				// DEBUG: Uncomment to see the ASCII strings as they are being blitted
				//if (m_blitterSerialCount >= m_textBytesToWrite)
				//{
				//  for (int i = 0; i < m_textBytesToWrite+1; i++)
				//      printf("%c", read_byte(0x03f40000 + m_textOffset + i));
				//  printf("\n");
				//}
			}
			else if (m_blitterMode == 0x30 || m_blitterMode == 0x90)
			{
				// Serialized 32-bit words in order of appearance:
				//  0: 00000000 - totally unknown : always seems to be zero
				//  1: xxxxxxxx - "Color Number" (all bits or just lower 16/8?)
				//  2: 00000000 - unknown : OT flag?  (transparency)
				//  3: 00000000 - unknown : RF flag?  (90 degree rotation)
				//  4: 07000000 - unknown : VF flag?  (vertically flipped)
				//  5: 00010000 - unknown : HF flag?  (horizontally flipped)
				//  6: vvvv---- - "Vertical Cell Count"
				//  6: ----hhhh - "Horizontal Cell Count"
				//  7: 00000000 - unknown : "Vertical|Horizontal Zoom Centers"?
				//  8: 00400040 - unknown : "Vertical|Horizontal Zoom Ratios"?
				//  9: xxxx---- - "Display Vertical Position"
				//  9: ----yyyy - "Display Horizontal Position"
				// 10: 00000000 - unknown : always seems to be zero
				// 11: ........ - complex - likely an address into bytes uploaded by mode 0xf4
				//                (See ifdef'ed out code below for a closer examination)

				// Serialized counts
				if (m_blitterSerialCount == 1)
				{
					m_colorNumber = (data & 0x000000ff);    // Probably more bits
				}
				else if (m_blitterSerialCount == 6)
				{
					m_vCellCount = (data & 0xffff0000) >> 16;
					m_hCellCount = (data & 0x0000ffff);
				}
				else if (m_blitterSerialCount == 9)
				{
					m_vPosition = (data & 0xffff0000) >> 16;
					m_hPosition = (data & 0x0000ffff);
				}
				else if (m_blitterSerialCount == 11)
				{
					const UINT32 memOffset = data;

					// Splat some sprites
					for (int h = 0; h < m_hCellCount; h++)
					{
						for (int v = 0; v < m_vCellCount; v++)
						{
							const int pixelOffsetX = m_hPosition + (h*16);
							const int pixelOffsetY = m_vPosition + (v*16);

							// It's unknown if it's row-major or column-major
							// TODO: Study the CRT test and "Cool Riders" logo for clues.
							UINT8 spriteNumber = space.read_byte(memOffset + h + (v*h));

							// DEBUG: For demo purposes, skip &spaces and NULL characters
							if (spriteNumber == 0x20 || spriteNumber == 0x00)
								continue;

							// DEBUG: Draw 16x16 block
							for (int x = 1; x < 15; x++)
							{
								for (int y = 1; y < 15; y++)
								{
									UINT32 color;
									if (m_colorNumber == 0x5b)
										color = 0xffff0000;
									else if (m_colorNumber == 0x5d)
										color = 0xff00ff00;
									else if (m_colorNumber == 0x5e)
										color = 0xff0000ff;
									else
										color = 0xff00ffff;
									m_temp_bitmap_sprites.pix32(pixelOffsetY+y, pixelOffsetX+x) = color;
								}
							}
						}
					}
				}

				m_blitterSerialCount++;
			}
			break;
		}
	}
}


WRITE32_MEMBER(coolridr_state::sysh1_pal_w)
{
	int r,g,b;
	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	r = ((m_generic_paletteram_32[offset] & 0x00007c00) >> 10);
	g = ((m_generic_paletteram_32[offset] & 0x000003e0) >> 5);
	b = ((m_generic_paletteram_32[offset] & 0x0000001f) >> 0);
	palette_set_color_rgb(machine(),(offset*2)+1,pal5bit(r),pal5bit(g),pal5bit(b));
	r = ((m_generic_paletteram_32[offset] & 0x7c000000) >> 26);
	g = ((m_generic_paletteram_32[offset] & 0x03e00000) >> 21);
	b = ((m_generic_paletteram_32[offset] & 0x001f0000) >> 16);
	palette_set_color_rgb(machine(),offset*2,pal5bit(r),pal5bit(g),pal5bit(b));
}


/* FIXME: this seems to do a hell lot of stuff, it's not ST-V SCU but still somewhat complex :/ */
static void sysh1_dma_transfer( address_space &space, UINT16 dma_index )
{
	coolridr_state *state = space.machine().driver_data<coolridr_state>();
	UINT32 src,dst,size,type,s_i;
	UINT8 end_dma_mark;

	end_dma_mark = 0;

	do{
		src = (state->m_framebuffer_vram[(0+dma_index)/4] & 0x0fffffff);
		dst = (state->m_framebuffer_vram[(4+dma_index)/4]);
		size = state->m_framebuffer_vram[(8+dma_index)/4];
		type = (state->m_framebuffer_vram[(0+dma_index)/4] & 0xf0000000) >> 28;

		#if 0
		if(type == 0xc || type == 0xd || type == 0xe)
			printf("* %08x %08x %08x %08x\n",src,dst,size,type);
		else if(type != 0 && type != 0x4)
			printf("%08x %08x %08x %08x\n",src,dst,size,type);
		#endif

		if(type == 0x3 || type == 0x4)
		{
			//type 3 sets a DMA state->m_param, type 4 sets some kind of table? Skip it for now
			dma_index+=4;
			continue;
		}

		if(type == 0xc)
		{
			dst &= 0xfffff;

			dst |= 0x3000000; //to videoram, FIXME: unknown offset
			size*=2;
		}
		if(type == 0xd)
		{
			dst &= 0xfffff;

			dst |= 0x3d00000; //to charram, FIXME: unknown offset
			size*=2;
		}

		if(type == 0xe)
		{
			dst &= 0xfffff;

			dst |= 0x3c00000; //to paletteram FIXME: unknown offset
			//size/=2;
			if((src & 0xff00000) == 0x3e00000)
				return; //FIXME: kludge to avoid palette corruption
			//debugger_break(space.machine());
		}

		if(type == 0xc || type == 0xd || type == 0xe)
		{
			for(s_i=0;s_i<size;s_i+=4)
			{
				space.write_dword(dst,space.read_dword(src));
				dst+=4;
				src+=4;
			}
		}
		else
		{
			//printf("%08x %08x %08x %08x\n",src,dst,size,type);
		}

		if(type == 0x00)
			end_dma_mark = 1; //end of DMA list

		dma_index+=0xc;

	}while(!end_dma_mark );
}

WRITE32_MEMBER(coolridr_state::sysh1_dma_w)
{
	COMBINE_DATA(&m_framebuffer_vram[offset]);

	if(offset*4 == 0x000)
	{
		if((m_framebuffer_vram[offset] & 0xff00000) == 0xfe00000)
			sysh1_dma_transfer(space, m_framebuffer_vram[offset] & 0xffff);
	}
}

WRITE32_MEMBER(coolridr_state::sysh1_char_w)
{
	COMBINE_DATA(&m_h1_charram[offset]);

	{
		UINT8 *gfx = memregion("ram_gfx")->base();

		gfx[offset*4+0] = (m_h1_charram[offset] & 0xff000000) >> 24;
		gfx[offset*4+1] = (m_h1_charram[offset] & 0x00ff0000) >> 16;
		gfx[offset*4+2] = (m_h1_charram[offset] & 0x0000ff00) >> 8;
		gfx[offset*4+3] = (m_h1_charram[offset] & 0x000000ff) >> 0;

		machine().gfx[2]->mark_dirty(offset/64); //*4/256
	}
}

static ADDRESS_MAP_START( system_h1_map, AS_PROGRAM, 32, coolridr_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_SHARE("share1") AM_WRITENOP
	AM_RANGE(0x01000000, 0x01ffffff) AM_ROM AM_REGION("gfx_data",0x0000000)

	AM_RANGE(0x03000000, 0x030fffff) AM_RAM AM_SHARE("h1_vram")//bg vram
	AM_RANGE(0x03c00000, 0x03c0ffff) AM_RAM_WRITE(sysh1_pal_w) AM_SHARE("paletteram")
	AM_RANGE(0x03d00000, 0x03dfffff) AM_RAM_WRITE(sysh1_char_w) AM_SHARE("h1_charram") //FIXME: half size
	AM_RANGE(0x03e00000, 0x03efffff) AM_RAM_WRITE(sysh1_dma_w) AM_SHARE("fb_vram") //FIXME: not all of it

	AM_RANGE(0x03f00000, 0x03f0ffff) AM_RAM AM_SHARE("share3") /*Communication area RAM*/
	AM_RANGE(0x03f40000, 0x03f4ffff) AM_RAM AM_SHARE("txt_vram")//text tilemap + "lineram"
	AM_RANGE(0x04000000, 0x0400003f) AM_RAM_WRITE(sysh1_txt_blit_w) AM_SHARE("sysh1_txt_blit")
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_SHARE("sysh1_workrah")
	AM_RANGE(0x20000000, 0x201fffff) AM_ROM AM_SHARE("share1")

	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( coolridr_submap, AS_PROGRAM, 32, coolridr_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_SHARE("share2")

	AM_RANGE(0x01000000, 0x0100ffff) AM_RAM //communication RAM

	AM_RANGE(0x03008800, 0x03008803) AM_RAM /*???*/
	AM_RANGE(0x03008900, 0x03008903) AM_RAM /*???*/
	AM_RANGE(0x03100400, 0x03100403) AM_RAM /*irq enable?*/
	AM_RANGE(0x03208800, 0x03208803) AM_RAM /*???*/
	AM_RANGE(0x03208900, 0x03208903) AM_RAM /*???*/
	AM_RANGE(0x03300400, 0x03300403) AM_RAM /*irq enable?*/

	AM_RANGE(0x04000000, 0x0400003f) AM_READWRITE(sysh1_unk_r,sysh1_unk_w) AM_SHARE("h1_unk")
	AM_RANGE(0x04200000, 0x0420003f) AM_RAM /*???*/

	AM_RANGE(0x05000000, 0x05000fff) AM_RAM
	AM_RANGE(0x05200000, 0x052001ff) AM_RAM
	AM_RANGE(0x05300000, 0x0530ffff) AM_RAM AM_SHARE("share3") /*Communication area RAM*/
	AM_RANGE(0x05ff0000, 0x05ffffff) AM_RAM /*???*/
	AM_RANGE(0x06000000, 0x06000fff) AM_RAM //UART TX/RX ports
	AM_RANGE(0x06100000, 0x06100003) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x06100004, 0x06100007) AM_READ_PORT("IN1")
	AM_RANGE(0x06100008, 0x0610000b) AM_READ_PORT("IN5")
	AM_RANGE(0x0610000c, 0x0610000f) AM_READ_PORT("IN6")
	AM_RANGE(0x06100010, 0x06100013) AM_READ_PORT("IN2") AM_WRITENOP
	AM_RANGE(0x06100014, 0x06100017) AM_READ_PORT("IN3")
	AM_RANGE(0x0610001c, 0x0610001f) AM_READ_PORT("IN4") AM_WRITENOP
	AM_RANGE(0x06200000, 0x06200fff) AM_RAM //check this!
	AM_RANGE(0x07fff000, 0x07ffffff) AM_RAM
	AM_RANGE(0x20000000, 0x2001ffff) AM_ROM AM_SHARE("share2")

	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP
ADDRESS_MAP_END

// SH-1 or SH-2 almost certainly copies the program down to here: the ROM containing the program is 32-bit wide and the 68000 is 16-bit
// the SCSP is believed to be hardcoded to decode the first 4 MB like this for a master/slave config
// (see also Model 3):
static ADDRESS_MAP_START( system_h1_sound_map, AS_PROGRAM, 16, coolridr_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
//  AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE_LEGACY("scsp1", scsp_r, scsp_w)
	AM_RANGE(0x800000, 0x80ffff) AM_RAM
//  AM_RANGE(0x300000, 0x300fff) AM_DEVREADWRITE_LEGACY("scsp2", scsp_r, scsp_w)
ADDRESS_MAP_END



static const gfx_layout tiles8x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	16*128
};

#if 0
static const gfx_layout test =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0*4,1*4,2*4,3*4,4*4,5*4,6*4, 7*4 },
	{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
	8*8*4
};
#endif

static GFXDECODE_START( coolridr )
//  GFXDECODE_ENTRY( "maincpu_data", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx_data", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "ram_gfx", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static INPUT_PORTS_START( coolridr )
	PORT_START("IN0")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN0-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN0-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN1-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	/* Service port*/
	PORT_BIT(0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x00040000, IP_ACTIVE_LOW )
	PORT_BIT(0x00080000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x00100000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x00200000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x00400000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1P Push Switch") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x00800000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2P Push Switch") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN2-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN2-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN3-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN3-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN4-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN4-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN5-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN5-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN6-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN6-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


// IRQs 4, 6 (& 8?) are valid on SH-2
INTERRUPT_GEN_MEMBER(coolridr_state::system_h1)
{
	device.execute().set_input_line(4, HOLD_LINE);
}

//IRQs 10,12 and 14 are valid on SH-1 instead
TIMER_DEVICE_CALLBACK_MEMBER(coolridr_state::system_h1_sub)
{
	int scanline = param;

	switch(scanline)
	{
		case 512:m_subcpu->set_input_line(0xa, HOLD_LINE); break;
		case 256:m_subcpu->set_input_line(0xc, HOLD_LINE); break;
		case 0:m_subcpu->set_input_line(0xe, HOLD_LINE); break;
	}
}

void coolridr_state::machine_reset()
{
//  machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	machine().device("soundcpu")->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

static MACHINE_CONFIG_START( coolridr, coolridr_state )
	MCFG_CPU_ADD("maincpu", SH2, 28000000)  // 28 mhz
	MCFG_CPU_PROGRAM_MAP(system_h1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", coolridr_state, system_h1)

	MCFG_CPU_ADD("soundcpu", M68000, 11289600) //256 x 44100 Hz = 11.2896 MHz
	MCFG_CPU_PROGRAM_MAP(system_h1_sound_map)

	MCFG_CPU_ADD("sub", SH1, 16000000)  // SH7032 HD6417032F20!! 16 mhz
	MCFG_CPU_PROGRAM_MAP(coolridr_submap)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", coolridr_state, system_h1_sub, "screen", 0, 1)

	MCFG_GFXDECODE(coolridr)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(128*8+22, 64*8+44)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 128*8-1, 0*8, 64*8-1) //TODO: these are just two different screens
	MCFG_SCREEN_UPDATE_DRIVER(coolridr_state, screen_update_coolridr)

	MCFG_PALETTE_LENGTH(0x10000)

MACHINE_CONFIG_END

ROM_START( coolridr )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "ep17659.30", 0x0000000, 0x080000, CRC(473027b0) SHA1(acaa212869dd79550235171b9f054e82750f74c3) )
	ROM_LOAD32_WORD_SWAP( "ep17658.29", 0x0000002, 0x080000, CRC(7ecfdfcc) SHA1(97cb3e6cf9764c8db06de12e4e958148818ef737) )
	ROM_LOAD32_WORD_SWAP( "ep17661.32", 0x0100000, 0x080000, CRC(81a7d90b) SHA1(99f8c3e75b94dd1b60455c26dc38ce08db82fe32) )
	ROM_LOAD32_WORD_SWAP( "ep17660.31", 0x0100002, 0x080000, CRC(27b7a507) SHA1(4c28b1d18d75630a73194b5d4fd166f3b647c595) )

	/* Page 12 of the service manual states that these 4 regions are tested, so I believe that they are read by the SH-2 */
	ROM_REGION32_BE( 0x1000000, "gfx_data", 0 ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "mp17650.11", 0x0000002, 0x0200000, CRC(0ccc84a1) SHA1(65951685b0c8073f6bd1cf9959e1b4d0fc6031d8) )
	ROM_LOAD32_WORD_SWAP( "mp17651.12", 0x0000000, 0x0200000, CRC(25fd7dde) SHA1(a1c3f3d947ce20fbf61ea7ab235259be9b7d35a8) )
	ROM_LOAD32_WORD_SWAP( "mp17652.13", 0x0400002, 0x0200000, CRC(be9b4d05) SHA1(0252ba647434f69d6eacb4efc6f55e6af534c7c5) )
	ROM_LOAD32_WORD_SWAP( "mp17653.14", 0x0400000, 0x0200000, CRC(64d1406d) SHA1(779dbbf42a14a6be1de9afbae5bbb18f8f36ceb3) )
	ROM_LOAD32_WORD_SWAP( "mp17654.15", 0x0800002, 0x0200000, CRC(5dee5cba) SHA1(6e6ec8574bdd35cc27903fc45f0d4a36ce9df103) )
	ROM_LOAD32_WORD_SWAP( "mp17655.16", 0x0800000, 0x0200000, CRC(02903cf2) SHA1(16d555fda144e0f1b62b428e9158a0e8ebf7084e) )
	ROM_LOAD32_WORD_SWAP( "mp17656.17", 0x0c00002, 0x0200000, CRC(945c89e3) SHA1(8776d74f73898d948aae3c446d7c710ad0407603) )
	ROM_LOAD32_WORD_SWAP( "mp17657.18", 0x0c00000, 0x0200000, CRC(74676b1f) SHA1(b4a9003a052bde93bebfa4bef9e8dff65003c3b2) )

	ROM_REGION32_BE( 0x100000, "ram_gfx", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_REGION( 0x100000, "soundcpu", ROMREGION_ERASE00 )   /* 68000 */
	ROM_COPY( "maincpu", 0x100000, 0x000000, 0x080000 ) //hardcoded from SH-2 roms? no, It doesn't seem so...

	ROM_REGION( 0x100000, "sub", 0 ) /* SH1 */
	ROM_LOAD16_WORD_SWAP( "ep17662.12", 0x000000, 0x020000,  CRC(50d66b1f) SHA1(f7b7f2f5b403a13b162f941c338a3e1207762a0b) )

	/* these are compressed sprite data */
	ROM_REGION( 0x2800000, "gfx5", ROMREGION_ERASEFF )
	/* logical interleaving according to the readme? */
	ROM_LOAD32_WORD_SWAP( "mpr-17640.ic1", 0x0000002, 0x0400000, CRC(981e3e69) SHA1(d242055e0359ec4b5fac4676b2f974fbc974cc68) )
	ROM_LOAD32_WORD_SWAP( "mpr-17645.ic6", 0x0000000, 0x0400000, CRC(56968d07) SHA1(e88c3d66ea05affb4681a25d155f097bd1b5a84b) )
	ROM_LOAD32_WORD_SWAP( "mpr-17641.ic2", 0x0800002, 0x0400000, CRC(fccc3dae) SHA1(0df7fd8b1110ba9063dc4dc40301267229cb9a35) )
	ROM_LOAD32_WORD_SWAP( "mpr-17646.ic7", 0x0800000, 0x0400000, CRC(b77eb2ad) SHA1(b832c0f1798aca39adba840d56ae96a75346670a) )
	ROM_LOAD32_WORD_SWAP( "mpr-17642.ic3", 0x1000002, 0x0400000, CRC(1a5bcc73) SHA1(a7df04c0a326323ea185db5f55b3e0449d76c535) )
	ROM_LOAD32_WORD_SWAP( "mpr-17647.ic8", 0x1000000, 0x0400000, CRC(9dd9330c) SHA1(c91a7f497c1f4bd283bd683b06dff88893724d51) )
	ROM_LOAD32_WORD_SWAP( "mpr-17643.ic4", 0x1800002, 0x0400000, CRC(5100f23b) SHA1(659c2300399ff1cbd24fb1eb18cfd6c26e06fd96) )
	ROM_LOAD32_WORD_SWAP( "mpr-17648.ic9", 0x1800000, 0x0400000, CRC(bf184cce) SHA1(62c004ea279f9a649d21426369336c2e1f9d24da) )
	ROM_LOAD32_WORD_SWAP( "mpr-17644.ic5", 0x2000002, 0x0400000, CRC(80199c79) SHA1(e525d8ee9f9176101629853e50cca73b02b16a38) )
	ROM_LOAD32_WORD_SWAP( "mpr-17649.ic10",0x2000000, 0x0400000, CRC(618c47ae) SHA1(5b69ad36fcf8e70d34c3b2fc71412ce953c5ceb3) )
ROM_END

#if 0
READ32_MEMBER(coolridr_state::coolridr_hack1_r)
{
	offs_t pc = downcast<cpu_device *>(&space.device())->pc();
	if(pc == 0x6012374 || pc == 0x6012392)
		return 0;

	return m_sysh1_workram_h[0xd88a4/4];
}
#endif

/*TODO: there must be an irq line with custom vector located somewhere that writes to here...*/
READ32_MEMBER(coolridr_state::coolridr_hack2_r)
{
	offs_t pc = downcast<cpu_device *>(&space.device())->pc();
	if(pc == 0x6002cba || pc == 0x6002d42)
		return 0;

	return m_sysh1_workram_h[0xd8894/4];
}

DRIVER_INIT_MEMBER(coolridr_state,coolridr)
{
//  machine().device("maincpu")->memory().space(AS_PROGRAM).install_legacy_read_handler(0x60d88a4, 0x060d88a7, FUNC(coolridr_hack1_r) );
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(0x60d8894, 0x060d8897, read32_delegate(FUNC(coolridr_state::coolridr_hack2_r), this));
}

GAME( 1995, coolridr,    0, coolridr,    coolridr, coolridr_state,    coolridr, ROT0,  "Sega", "Cool Riders (US)",GAME_NOT_WORKING|GAME_NO_SOUND )
