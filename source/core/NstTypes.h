////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Nestopia.
// 
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef NST_TYPES_H
#define NST_TYPES_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_NAMESPACE_NAME  NES
#define NES_NAMESPACE_BEGIN namespace NES_NAMESPACE_NAME {
#define NES_NAMESPACE_END   }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_MASTER_CLOCK_FIXED 7

#define NES_MASTER_CLOCK_MULTIPLIER    6
#define NES_CPU_MASTER_CLOCK_DIV_NTSC 12
#define NES_PPU_MASTER_CLOCK_DIV_NTSC  4
#define NES_CPU_MASTER_CLOCK_DIV_PAL  16
#define NES_PPU_MASTER_CLOCK_DIV_PAL   5

#define NES_CPU_NTSC_FIXED ( NES_CPU_MASTER_CLOCK_DIV_NTSC << NES_MASTER_CLOCK_FIXED )
#define NES_PPU_NTSC_FIXED ( NES_PPU_MASTER_CLOCK_DIV_NTSC << NES_MASTER_CLOCK_FIXED )
#define NES_CPU_PAL_FIXED  ( NES_CPU_MASTER_CLOCK_DIV_PAL  << NES_MASTER_CLOCK_FIXED )
#define NES_PPU_PAL_FIXED  ( NES_PPU_MASTER_CLOCK_DIV_PAL  << NES_MASTER_CLOCK_FIXED )

#define NES_CPU_CC_FRAME_COUNTER_NTSC 14915
#define NES_CPU_CC_FRAME_COUNTER_PAL  16626

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_CPU_TO_NTSC(_x_) ( (_x_) * NES_CPU_NTSC_FIXED )
#define NES_PPU_TO_NTSC(_x_) ( (_x_) * NES_PPU_NTSC_FIXED )
#define NES_CPU_TO_PAL(_x_)  ( (_x_) * NES_CPU_PAL_FIXED  )
#define NES_PPU_TO_PAL(_x_)  ( (_x_) * NES_PPU_PAL_FIXED  )

#define NES_NTSC_TO_CPU(_x_) ( (_x_) / NES_CPU_NTSC_FIXED )
#define NES_NTSC_TO_PPU(_x_) ( (_x_) / NES_PPU_NTSC_FIXED )
#define NES_PAL_TO_CPU(_x_)  ( (_x_) / NES_CPU_PAL_FIXED  )
#define NES_PAL_TO_PPU(_x_)  ( (_x_) / NES_PPU_PAL_FIXED  )

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_PI  3.1415926535897932384626433832795L
#define NES_DEG 0.017453292519943295769236907684886L

#define NES_FPS_NTSC 60
#define NES_FPS_PAL  50

#define NES_SUBCARRIER_REAL_NTSC 3579545.45454545454545454545L
#define NES_SUBCARRIER_REAL_PAL  4433618.75000000000000000000L

#define NES_MASTER_CLOCK_HZ_REAL_NTSC ( NES_SUBCARRIER_REAL_NTSC * NES_MASTER_CLOCK_MULTIPLIER )
#define NES_MASTER_CLOCK_HZ_REAL_PAL  ( NES_SUBCARRIER_REAL_PAL  * NES_MASTER_CLOCK_MULTIPLIER )

#define NES_MASTER_CLOCK_HZ_NTSC ULONG( NES_MASTER_CLOCK_HZ_REAL_NTSC * (1U << NES_MASTER_CLOCK_FIXED) )
#define NES_MASTER_CLOCK_HZ_PAL  ULONG( NES_MASTER_CLOCK_HZ_REAL_PAL  * (1U << NES_MASTER_CLOCK_FIXED) )

#define NES_CPU_MCC_FRAME_NTSC ( NES_CPU_TO_NTSC ( ULONG( NES_CPU_CC_FRAME_COUNTER_NTSC ) ) * 2 )
#define NES_CPU_MCC_FRAME_PAL  ( NES_CPU_TO_PAL  ( ULONG( NES_CPU_CC_FRAME_COUNTER_PAL  ) ) * 2 )

#define NES_MASTER_CC_QUARTER_FRAME_NTSC ( NES_CPU_TO_NTSC ( ULONG( NES_CPU_CC_FRAME_COUNTER_NTSC ) ) /	2 )
#define NES_MASTER_CC_QUARTER_FRAME_PAL  ( NES_CPU_TO_PAL  ( ULONG( NES_CPU_CC_FRAME_COUNTER_PAL  ) ) /	2 )

#define NES_CPU_CLOCK_HZ_REAL_NTSC ( NES_MASTER_CLOCK_HZ_REAL_NTSC / NES_CPU_MASTER_CLOCK_DIV_NTSC )
#define NES_CPU_CLOCK_HZ_REAL_PAL  ( NES_MASTER_CLOCK_HZ_REAL_PAL  / NES_CPU_MASTER_CLOCK_DIV_PAL  )

#define NES_FPS_REAL_NTSC ( NES_MASTER_CLOCK_HZ_REAL_NTSC / ( ( ( ULONG( NES_PPU_CC_FRAME_0_NTSC ) + NES_PPU_CC_FRAME_1_NTSC ) / 2.0 ) * NES_PPU_MASTER_CLOCK_DIV_NTSC ) )
#define NES_FPS_REAL_PAL  ( NES_MASTER_CLOCK_HZ_REAL_PAL  / ( ( ( ULONG( NES_PPU_CC_FRAME_0_PAL  ) + NES_PPU_CC_FRAME_1_PAL  ) / 2.0 ) * NES_PPU_MASTER_CLOCK_DIV_PAL  ) )

#define NES_PPU_LN_VACTIVE     240
#define NES_PPU_LN_VINT_NTSC   20
#define NES_PPU_LN_VINT_PAL    70
#define NES_PPU_LN_VSLEEP      1
#define NES_PPU_LN_VDUMMY	   1
#define NES_PPU_LN_VBLANK_NTSC ( NES_PPU_LN_VSLEEP + NES_PPU_LN_VINT_NTSC + NES_PPU_LN_VDUMMY )
#define NES_PPU_LN_VBLANK_PAL  ( NES_PPU_LN_VSLEEP + NES_PPU_LN_VINT_PAL + NES_PPU_LN_VDUMMY  )
#define NES_PPU_LN_VSYNC_NTSC  ( NES_PPU_LN_VACTIVE + NES_PPU_LN_VBLANK_NTSC			      )
#define NES_PPU_LN_VSYNC_PAL   ( NES_PPU_LN_VACTIVE + NES_PPU_LN_VBLANK_PAL				      )

#define NES_PPU_CC_PIXEL      1
#define NES_PPU_CC_FETCH      2
#define NES_PPU_CC_SPRITE_DMA 1536
#define NES_PPU_CC_HACTIVE	  (NES_PPU_CC_FETCH * 4 * 32)
#define NES_PPU_CC_HBLANK     ((NES_PPU_CC_FETCH * 4 * 8) + (NES_PPU_CC_FETCH * 4 * 2) + (NES_PPU_CC_FETCH * 2) + 1)
#define NES_PPU_CC_HSYNC      (NES_PPU_CC_HACTIVE + NES_PPU_CC_HBLANK)

#define NES_PPU_CC_VINT_NTSC    ( NES_PPU_CC_HSYNC * NES_PPU_LN_VINT_NTSC   )
#define NES_PPU_CC_VINT_PAL     ( NES_PPU_CC_HSYNC * NES_PPU_LN_VINT_PAL    )
#define NES_PPU_CC_VBLANK_NTSC  ( NES_PPU_CC_HSYNC * NES_PPU_LN_VBLANK_NTSC )
#define NES_PPU_CC_VBLANK_PAL   ( NES_PPU_CC_HSYNC * NES_PPU_LN_VBLANK_PAL  )
#define NES_PPU_CC_FRAME_0_NTSC ( NES_PPU_CC_HSYNC * NES_PPU_LN_VSYNC_NTSC  ) 
#define NES_PPU_CC_FRAME_0_PAL  ( NES_PPU_CC_HSYNC * NES_PPU_LN_VSYNC_PAL   )
#define NES_PPU_CC_FRAME_1_NTSC ( ( NES_PPU_CC_HSYNC * NES_PPU_LN_VSYNC_NTSC ) - 1 )
#define NES_PPU_CC_FRAME_1_PAL  ( ( NES_PPU_CC_HSYNC * NES_PPU_LN_VSYNC_PAL  ) - 1 )

/////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_PPU_MCC_PIXEL_NTSC		NES_PPU_TO_NTSC( NES_PPU_CC_PIXEL        )
#define NES_PPU_MCC_FETCH_NTSC      NES_PPU_TO_NTSC( NES_PPU_CC_FETCH        ) 
#define NES_PPU_MCC_SPRITE_DMA_NTSC NES_PPU_TO_NTSC( NES_PPU_CC_SPRITE_DMA   ) 
#define NES_PPU_MCC_HACTIVE_NTSC	NES_PPU_TO_NTSC( NES_PPU_CC_HACTIVE		 )
#define NES_PPU_MCC_HBLANK_NTSC		NES_PPU_TO_NTSC( NES_PPU_CC_HBLANK       )
#define NES_PPU_MCC_HSYNC_NTSC		NES_PPU_TO_NTSC( NES_PPU_CC_HSYNC        )
#define NES_PPU_MCC_VINT_NTSC		NES_PPU_TO_NTSC( NES_PPU_CC_VINT_NTSC    )
#define NES_PPU_MCC_VBLANK_NTSC		NES_PPU_TO_NTSC( NES_PPU_CC_VBLANK_NTSC  )
#define NES_PPU_MCC_FRAME_0_NTSC	NES_PPU_TO_NTSC( NES_PPU_CC_FRAME_0_NTSC )
#define NES_PPU_MCC_FRAME_1_NTSC	NES_PPU_TO_NTSC( NES_PPU_CC_FRAME_1_NTSC )

#define NES_PPU_MCC_PIXEL_PAL	    NES_PPU_TO_PAL( NES_PPU_CC_PIXEL       )
#define NES_PPU_MCC_FETCH_PAL       NES_PPU_TO_PAL( NES_PPU_CC_FETCH       ) 
#define NES_PPU_MCC_SPRITE_DMA_PAL  NES_PPU_TO_PAL( NES_PPU_CC_SPRITE_DMA  ) 
#define NES_PPU_MCC_HACTIVE_PAL	    NES_PPU_TO_PAL( NES_PPU_CC_HACTIVE	   )
#define NES_PPU_MCC_HBLANK_PAL	    NES_PPU_TO_PAL( NES_PPU_CC_HBLANK      )
#define NES_PPU_MCC_HSYNC_PAL	    NES_PPU_TO_PAL( NES_PPU_CC_HSYNC       )
#define NES_PPU_MCC_VINT_PAL	    NES_PPU_TO_PAL( NES_PPU_CC_VINT_PAL    )
#define NES_PPU_MCC_VBLANK_PAL	    NES_PPU_TO_PAL( NES_PPU_CC_VBLANK_PAL  )
#define NES_PPU_MCC_FRAME_0_PAL	    NES_PPU_TO_PAL( NES_PPU_CC_FRAME_0_PAL )
#define NES_PPU_MCC_FRAME_1_PAL	    NES_PPU_TO_PAL( NES_PPU_CC_FRAME_1_PAL )

/////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NES_PEEK(_o_,_a_)  UINT _o_::Peek_##_a_(const UINT address)
#define NES_POKE(_o_,_a_)  VOID _o_::Poke_##_a_(const UINT address,const UINT data)
#define NES_LATCH(_o_,_a_) UINT _o_::Latch_##_a_(const UINT address)

#define NES_DECL_PEEK(_a_)  UINT Peek_##_a_(const UINT)
#define NES_DECL_POKE(_a_)  VOID Poke_##_a_(const UINT,const UINT)
#define NES_DECL_LATCH(_a_) UINT Latch_##_a_(const UINT)

/////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class PDXFILE;
#include "../paradox/PdxString.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

enum
{
	n1k  = 0x0400,
	n2k  = 0x0800,
	n4k  = 0x1000,
	n8k  = 0x2000,
	n16k = 0x4000,
	n32k = 0x8000
};

enum MIRRORING
{
	MIRROR_HORIZONTAL,
	MIRROR_VERTICAL,
	MIRROR_FOURSCREEN,
	MIRROR_ZERO,
	MIRROR_ONE,
	MIRROR_TWO,
	MIRROR_THREE       
};

enum MODE
{
	MODE_NTSC,
	MODE_PAL,
	MODE_AUTO
};

enum SYSTEM
{
	SYSTEM_NTSC,
	SYSTEM_PAL,
	SYSTEM_VS,
	SYSTEM_PC10
};

enum CONTROLLERTYPE
{
	CONTROLLER_UNCONNECTED,
	CONTROLLER_PAD1,       
	CONTROLLER_PAD2,       
	CONTROLLER_PAD3,       
	CONTROLLER_PAD4,       
	CONTROLLER_ZAPPER,     
	CONTROLLER_PADDLE,     
	CONTROLLER_POWERPAD,   
	CONTROLLER_KEYBOARD    
};

class MACHINE;

/////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

namespace IO
{
	struct GFX
	{
		typedef U16 PIXEL;

		enum
		{
			WIDTH          = 256,
			HEIGHT         = 240,
			PALETTE_CHUNK  = 64,
			PALETTE_LENGTH = PALETTE_CHUNK * 8
		};

		GFX()
		: 
		device         ( NULL  ),
		pixels         ( NULL  ),
		palette    	   ( NULL  ),
		PaletteChanged ( TRUE  ),
		failure        ( FALSE )
		{}

		PDXRESULT Lock();
		PDXRESULT Unlock();

		UINT GetPixel(const UINT,const UINT) const;

		VOID* device;
		PIXEL* pixels;
		const U8* palette;
		BOOL PaletteChanged;
		BOOL failure;

		enum PALETTE
		{
			PALETTE_EMULATED,
			PALETTE_INTERNAL,
			PALETTE_CUSTOM
		};

		struct CONTEXT
		{
			CONTEXT()
			: 
			palette         ( NULL  ),
			PaletteMode     ( PALETTE_EMULATED ),
			brightness      ( 128   ),
			saturation      ( 128   ),
			hue             ( 128   ),
			InfiniteSprites ( FALSE )
			{}

			const U8* palette;
			BOOL PaletteMode;
			UINT brightness;
			UINT saturation;
			UINT hue;
			BOOL InfiniteSprites;
		};
	};

	struct SFX
	{
		SFX()
		: 
		device    ( NULL  ),
		samples   ( NULL  ),
		length    ( 0     ),
		failure   ( FALSE )
		{}

		PDXRESULT Lock();
		PDXRESULT Unlock();
		PDXRESULT Clear();

		VOID* device;
		VOID* samples;
		TSIZE length;
		BOOL failure;

		struct CONTEXT
		{
			CONTEXT()
			:
			enabled         ( TRUE      ),
			square1         ( TRUE      ),
			square2         ( TRUE      ),
			triangle        ( TRUE      ),
			noise           ( TRUE      ),
			dpcm            ( TRUE      ),
			external        ( TRUE      ),
			SampleBits      ( 16        ),
			SampleRate      ( 44100     )
			{}

			UINT  SampleBits;
			ULONG SampleRate;
			BOOL  enabled;
			BOOL  square1;
			BOOL  square2;
			BOOL  triangle;
			BOOL  noise;
			BOOL  dpcm;
			BOOL  external;
		};
	};

	struct INPUT
	{
		struct PAD
		{
			inline PAD()
			: buttons(0) 
			{}

			enum
			{
				A      = b00000001,
				B      = b00000010,
				SELECT = b00000100,
				START  = b00001000,
				UP     = b00010000,
				DOWN   = b00100000,
				LEFT   = b01000000,
				RIGHT  = b10000000
			};

			enum
			{
				INDEX_A      = 0,
				INDEX_B      = 1,
				INDEX_SELECT = 2,
				INDEX_START  = 3,
				INDEX_UP     = 4,
				INDEX_DOWN   = 5,
				INDEX_LEFT   = 6,
				INDEX_RIGHT  = 7
			};

			UINT buttons;
		};

		struct ZAPPER
		{
			inline ZAPPER()
			: x(0), y(0), fire(0) {}

   			UINT x;
   			UINT y;
   			UINT fire;
		};

		struct PADDLE
		{
			PADDLE()
			: x(0), fire(0) {}

   			UINT x;
   			UINT fire;
		};

		struct POWERPAD
		{
			POWERPAD()
			{ 
				PDXMemZero( SideA, NUM_SIDE_A_BUTTONS ); 
				PDXMemZero( SideB, NUM_SIDE_B_BUTTONS ); 
			}

			enum 
			{
				NUM_SIDE_A_BUTTONS = 12,
				NUM_SIDE_B_BUTTONS = 8
			};

			U8 SideA[NUM_SIDE_A_BUTTONS];
			U8 SideB[NUM_SIDE_B_BUTTONS];
		};

		struct FAMILYKEYBOARD
		{
			FAMILYKEYBOARD() : device(NULL)
			{ PDXMemZero( parts, NUM_PARTS ); }

			VOID Poll(const UINT,const UINT);

			enum 
			{
				NUM_PARTS = 10,
				NUM_MODES = 2
			};

			VOID* device;
			UINT parts[NUM_PARTS];
		};

		struct VS
		{
			inline VS()
			: InsertCoin(0) {}

			enum
			{
				COIN_1 = b00100000,
				COIN_2 = b01000000
			};

			UINT InsertCoin;
		};

		PAD            pad[4];
		ZAPPER         zapper;
		PADDLE         paddle;
		POWERPAD       PowerPad;		
		FAMILYKEYBOARD FamilyKeyboard;
		VS             vs;
	};

	namespace CARTRIDGE
	{
		enum
		{
			YES     = 1,
			GOOD    = 1,
			NO      = 2,
			BAD     = 2,
			UNKNOWN = 0
		};

		struct INFO
		{
			INFO()
			{ 
				Reset(); 
			}

			VOID Reset()
			{
				file.Clear();
				name.Clear();
				copyright.Clear();
				board.Clear();
		
				mapper         = 0;
				pRom           = 0;
				cRom           = 0;
				wRam           = 0;
				IsCRam         = FALSE;
				crc            = 0;
				pRomCrc        = 0;
				cRomCrc        = 0;
				system         = SYSTEM_NTSC;
				mirroring      = MIRROR_HORIZONTAL;
				battery        = FALSE;
				trained        = FALSE;
				controllers[0] = CONTROLLER_PAD1;
				controllers[1] = CONTROLLER_PAD2;
				controllers[2] = CONTROLLER_UNCONNECTED;
				controllers[3] = CONTROLLER_UNCONNECTED;
				controllers[4] = CONTROLLER_UNCONNECTED;
				condition      = GOOD;
				translated     = UNKNOWN;
				hacked         = UNKNOWN;
				licensed       = UNKNOWN;
				bootleg        = UNKNOWN;
			}
		
			PDXSTRING      file;
			PDXSTRING      name;
			PDXSTRING      copyright;
			PDXSTRING      board;
			UINT           mapper;
			ULONG          pRom;
			ULONG          cRom;
			ULONG          wRam;
			BOOL           IsCRam;
			ULONG          crc;
			ULONG          pRomCrc;
			ULONG          cRomCrc;
			SYSTEM         system;
			MIRRORING      mirroring;
			BOOL           battery;
			BOOL           trained;		
			CONTROLLERTYPE controllers[5];
			UINT           condition;
			UINT           translated;
			UINT           hacked;
			UINT           licensed;
			UINT           bootleg;
		};
	}

	namespace FDS
	{
		struct CONTEXT
		{
			CONTEXT()
			{ Reset(); }

			VOID Reset()
			{
				bios           = NULL;
				NumDisks       = 0;
				DiskInserted   = FALSE;
				CurrentDisk    = 0;
				CurrentSide    = 0;
				WriteProtected = FALSE;
			}

			PDXFILE* bios;
			UINT NumDisks;
			BOOL DiskInserted;
			UINT CurrentDisk;
			UINT CurrentSide;
			BOOL WriteProtected;
		};
	}

	namespace DIPSWITCH
	{
		struct CONTEXT
		{
			CONTEXT()
			: index(0) {}

    		typedef PDXARRAY<PDXSTRING> SETTINGS;

    		PDXSTRING name;
    		SETTINGS settings;
    		UINT index;
		};
	}
	
	namespace NSF
	{
		enum OP
		{
			PLAY,
			STOP,
			NEXT,
			PREV
		};

		struct CONTEXT
		{
			CONTEXT()
			: 
			song	 ( 0     ),
			NumSongs ( 0     ),
			pal      ( FALSE ),
			op		 ( PLAY  )
			{}

			UINT      song;
			UINT      NumSongs;
			PDXSTRING name;
			PDXSTRING artist;
			PDXSTRING copyright;
			PDXSTRING chip;
			BOOL      pal;
			OP        op;
		};
	}

	class GAMEGENIE
	{
	public:

		GAMEGENIE(MACHINE&);

		static PDXRESULT Encode (const ULONG,PDXSTRING&);
		static PDXRESULT Decode (const CHAR* const,ULONG&);
		static PDXRESULT Pack   (const UINT,const UINT,const UINT,const BOOL,ULONG&);
		static PDXRESULT Unpack (const ULONG,UINT&,UINT&,UINT&,BOOL&);

		PDXRESULT AddCode    (const ULONG);
		PDXRESULT DeleteCode (const ULONG);

		TSIZE NumCodes  () const;
		ULONG GetCode   (const TSIZE) const;
		VOID ClearCodes ();

	private:

		VOID* const handler;
	};

	namespace GENERAL
	{
		struct CONTEXT
		{
			CONTEXT()
			: 
			DisableWarnings     ( FALSE ), 
			WriteProtectBattery ( FALSE ),
			UseRomDatabase      ( TRUE  )
			{}

			BOOL DisableWarnings;
			BOOL WriteProtectBattery;
			BOOL UseRomDatabase;
		};
	}

	namespace NSP
	{
		struct CONTEXT
		{
			CONTEXT()
			{
				Reset();
			}

			VOID Reset()
			{				
				ImageFile.Clear();
				IpsFile.Clear();
				SaveFile.Clear();
				StateFile.Clear();
				MovieFile.Clear();
			
				for (UINT i=0; i < 10; ++i)
					StateSlots[i].Clear();

				PaletteFile.Clear();

				GenieCodes.Clear();

				pal = -1;

				controllers[0] = CONTROLLER_PAD1;
				controllers[1] = CONTROLLER_PAD2;
				controllers[2] = CONTROLLER_UNCONNECTED;
				controllers[3] = CONTROLLER_UNCONNECTED;
				controllers[4] = CONTROLLER_UNCONNECTED;
			}

			struct GENIECODE
			{
				BOOL enabled;
				PDXSTRING code;
				PDXSTRING comment;
			};

			typedef PDXARRAY<GENIECODE> GENIECODES;

			PDXSTRING      ImageFile;
			PDXSTRING      IpsFile;
			PDXSTRING      SaveFile;
			PDXSTRING      StateFile;
			PDXSTRING      MovieFile;
			PDXSTRING      StateSlots[10];
			PDXSTRING      PaletteFile;
			GENIECODES     GenieCodes;	
			INT            pal;
			CONTROLLERTYPE controllers[5];
		};
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// global callback functions which have to be implemented for the specific target platform
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MsgError    (const CHAR* const);
PDXRESULT MsgWarning  (const CHAR* const);
BOOL      MsgQuestion (const CHAR* const,const CHAR* const);
BOOL      MsgInput    (const CHAR* const,const CHAR* const,PDXSTRING&);
VOID      MsgOutput   (const CHAR* const);
VOID      LogOutput   (const PDXSTRING&);

NES_NAMESPACE_END

#endif
