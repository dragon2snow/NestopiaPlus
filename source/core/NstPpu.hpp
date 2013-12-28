////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#ifndef NST_PPU_H
#define NST_PPU_H

#ifndef NST_IO_PORT_H
#include "NstIoPort.hpp"
#endif

#include "NstIoAccessor.hpp"
#include "NstIoLine.hpp"
#include "NstHook.hpp"
#include "NstMemory.hpp"
#include "NstVideoScreen.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Ppu
		{
		public:

			explicit Ppu(Cpu&);

			enum
			{
				SL_VACTIVE      = 240,
				SL_VINT_NTSC    = 20,
				SL_VINT_PAL     = 70,
				SL_VSLEEP       = 1,
				SL_VDUMMY       = 1,
				SL_VBLANK_NTSC  = SL_VSLEEP + SL_VINT_NTSC + SL_VDUMMY,
				SL_VBLANK_PAL   = SL_VSLEEP + SL_VINT_PAL + SL_VDUMMY,
				SL_VSYNC_NTSC   = SL_VACTIVE + SL_VBLANK_NTSC,
				SL_VSYNC_PAL    = SL_VACTIVE + SL_VBLANK_PAL,
				MC_DIV_NTSC     = 4,
				MC_DIV_PAL      = 5,
				CC_IO           = 2,
				CC_HACTIVE      = CC_IO * 4 * 32,
				CC_HBLANK       = CC_IO * 4 * 8 + CC_IO * 4 * 2 + CC_IO * 2 + 1,
				CC_HSYNC        = CC_HACTIVE + CC_HBLANK,
				CC_VINT_NTSC    = CC_HSYNC * dword( SL_VINT_NTSC  ),
				CC_VINT_PAL     = CC_HSYNC * dword( SL_VINT_PAL   ),
				CC_FRAME_0_NTSC = CC_HSYNC * dword( SL_VSYNC_NTSC ),
				CC_FRAME_1_NTSC = CC_HSYNC * dword( SL_VSYNC_NTSC ) - 1,
				CC_FRAME_PAL    = CC_HSYNC * dword( SL_VSYNC_PAL  )
			};

			enum Mirroring
			{
				NMT_HORIZONTAL,
				NMT_VERTICAL,
				NMT_FOURSCREEN,
				NMT_ZERO,
				NMT_ONE,
				NMT_CONTROLLED
			};

			void Reset(bool=false);
			void ClearScreen();
			void BeginFrame();
			void Update(Cycle=0);
			void EndFrame();

			void SetMode(Mode);
			void SetMirroring(uint);
			void SetMirroring(const byte (&)[4]);
			void SetYuvMap(const byte*,bool);
			void SetBgHook(const Hook&);
			void SetSpHook(const Hook&);

			void EnableCpuSynchronization();

			void LoadState(State::Loader&);
			void SaveState(State::Saver&,dword) const;

			void EnableUnlimSprites(bool);
			bool AreUnlimSpritesEnabled() const;
			uint SolidColors() const;

			class ChrMem : public Memory<SIZE_8K,SIZE_1K,2>
			{
				NES_DECL_ACCESSOR( Pattern );

			protected:

				Io::Accessor accessors[2];

			public:

				void ResetAccessors();
				void SetDefaultAccessor(uint);

				template<typename T,typename U>
				void SetAccessor(uint i,T t,U u)
				{
					NST_ASSERT( i < 2 );
					accessors[i].Set( t, u );
				}

				template<typename T,typename U,typename V>
				void SetAccessors(T t,U u,V v)
				{
					accessors[0].Set( t, u );
					accessors[1].Set( t, v );
				}

				bool SameComponent(uint i,const void* ptr) const
				{
					NST_ASSERT( i < 2 );
					return accessors[i] == ptr;
				}
			};

			class NmtMem : public Memory<SIZE_4K,SIZE_1K,2>
			{
				NES_DECL_ACCESSOR( Name_2000 );
				NES_DECL_ACCESSOR( Name_2400 );
				NES_DECL_ACCESSOR( Name_2800 );
				NES_DECL_ACCESSOR( Name_2C00 );

			protected:

				Io::Accessor accessors[4][2];

			public:

				void ResetAccessors();
				void SetDefaultAccessors(uint);
				void SetDefaultAccessor(uint,uint);

				template<typename T,typename U>
				void SetAccessor(uint i,uint j,T t,U u)
				{
					NST_ASSERT( i < 4 && j < 2 );
					accessors[i][j].Set( t, u );
				}

				template<typename T,typename U>
				void SetAccessors(uint i,T t,U u)
				{
					NST_ASSERT( i < 4 );
					accessors[i][0].Set( t, u[0] );
					accessors[i][1].Set( t, u[1] );
				}

				template<typename T,typename U,typename V,typename W,typename X>
				void SetAccessors(T t,U u,V v,W w,X x)
				{
					accessors[0][0].Set( t, u[0] );
					accessors[0][1].Set( t, u[1] );
					accessors[1][0].Set( t, v[0] );
					accessors[1][1].Set( t, v[1] );
					accessors[2][0].Set( t, w[0] );
					accessors[2][1].Set( t, w[1] );
					accessors[3][0].Set( t, x[0] );
					accessors[3][1].Set( t, x[1] );
				}

				bool SameComponent(uint i,uint j,const void* ptr) const
				{
					NST_ASSERT( i < 4 && j < 2 );
					return accessors[i][j] == ptr;
				}
			};

		private:

			struct Chr : ChrMem
			{
				NST_FORCE_INLINE uint FetchPattern(uint) const;
			};

			struct Nmt : NmtMem
			{
				NST_FORCE_INLINE uint FetchName(uint) const;
				NST_FORCE_INLINE uint FetchAttribute(uint) const;
			};

			typedef void (Ppu::*Phase)();

			enum
			{
				WARM_UP_FRAMES  = 2,
				SCANLINE_HDUMMY = -1,
				SCANLINE_VBLANK = 255
			};

			NES_DECL_POKE( 2000 );
			NES_DECL_PEEK( 2002 );
			NES_DECL_POKE( 2001 );
			NES_DECL_POKE( 2003 );
			NES_DECL_PEEK( 2004 );
			NES_DECL_POKE( 2004 );
			NES_DECL_POKE( 2005 );
			NES_DECL_POKE( 2006 );
			NES_DECL_PEEK( 2007 );
			NES_DECL_POKE( 2007 );
			NES_DECL_PEEK( 2xxx );
			NES_DECL_POKE( 2xxx );
			NES_DECL_PEEK( 4014 );
			NES_DECL_POKE( 4014 );

			NES_DECL_HOOK( Sync );
			NES_DECL_HOOK( Nop  );

			inline bool IsDead() const;
			inline void UpdateScrollAddress(uint);
			inline uint Coloring() const;
			inline uint Emphasis() const;

			NST_FORCE_INLINE uint FetchName() const;
			NST_FORCE_INLINE uint FetchAttribute() const;
			NST_FORCE_INLINE void EvaluateSprites();

			void UpdateStates();
			void LoadSprite();
			void UpdatePalette();

			NST_FORCE_INLINE void LoadTiles();
			NST_FORCE_INLINE void RenderPixel();

			void WarmUp();
			void VBlankIn();
			void VBlank();
			void VBlankOut();
			void HDummy();
			void HDummyBg();
			void HDummySp();
			void HDummyScroll();
			void HActive0();
			void HActive1();
			void HActive2();
			void HActive3();
			void HActive4();
			void HActive5();
			void HActive6();
			void HActive7();
			void HBlank();
			void HBlankSp();
			void HBlankBg();
			void HBlankBg0();
			void HBlankBg1();
			void HBlankBg2();
			void HBlankBg3();
			void HBlankBg4();
			void HBlankBg5();
			void HBlankBg6();
			void HBlankBg7();

			struct Regs
			{
				enum
				{
					CTRL0_NAME_OFFSET     = b00000011,
					CTRL0_INC32           = b00000100,
					CTRL0_SP_OFFSET       = b00001000,
					CTRL0_BG_OFFSET       = b00010000,
					CTRL0_SP8X16          = b00100000,
					CTRL0_NMI             = b10000000,
					CTRL1_MONOCHROME      = b00000001,
					CTRL1_BG_NO_CLIPPING  = b00000010,
					CTRL1_SP_NO_CLIPPING  = b00000100,
					CTRL1_BG_ENABLED      = b00001000,
					CTRL1_SP_ENABLED      = b00010000,
					CTRL1_FULL_BG_ENABLED = CTRL1_BG_ENABLED|CTRL1_BG_NO_CLIPPING,
					CTRL1_FULL_SP_ENABLED = CTRL1_SP_ENABLED|CTRL1_SP_NO_CLIPPING,
					CTRL1_EMPHASIS        = b11100000,
					STATUS_LATCH          = b00011111,
					STATUS_SP_OVERFLOW    = b00100000,
					STATUS_SP_ZERO_HIT    = b01000000,
					STATUS_VBLANK         = b10000000,
					STATUS_BITS           = b11100000,
					STATUS_VBLANKING      = 0x100,
					FRAME_ODD             = CTRL1_BG_ENABLED|CTRL1_SP_ENABLED
				};

				uint ctrl0;
				uint ctrl1;
				uint status;
				uint frame;
				uint oam;
			};

			struct Scroll
			{
				enum
				{
					X_TILE    = 0x001F,
					Y_TILE    = 0x03E0,
					Y_FINE    = 0x7000,
					LOW       = 0x00FF,
					HIGH      = 0xFF00,
					NAME      = 0x0C00,
					NAME_LOW  = 0x0400,
					NAME_HIGH = 0x0800
				};

				NST_FORCE_INLINE void ResetX();
				NST_FORCE_INLINE void ClockX();
				NST_FORCE_INLINE void ClockY();

				uint address;
				uint toggle;
				uint latch;
				uint increase;
				uint xFine;
				uint pattern;
			};

			struct Tiles
			{
				Tiles();

				byte pattern[2];
				byte attribute;
				byte index;
				byte pixels[16];
				uint mask;
				byte show[2];
				const byte padding0;
				const byte padding1;
			};

			struct Palette
			{
				enum
				{
					SIZE          = 0x20,
					COLORS        = 0x40,
					SPRITE_OFFSET = 0x10,
					COLOR         = 0x3F,
					MONO          = 0x30
				};

				byte ram[SIZE];
			};

			struct Output
			{
				explicit Output(Video::Screen::Pixel*);

				uint index;
				Video::Screen::Pixel* target;
				Video::Screen::Pixel* pixels;
				uint burstPhase;
				word palette[Palette::SIZE];
			};

			struct Oam
			{
				Oam();

				enum
				{
					SIZE             = 0x100,
					OFFSET_TO_0_1    = b11111000,
					STD_LINE_SPRITES = 8,
					MAX_LINE_SPRITES = 32,
					DMA_CYCLES       = 512 + 1,
					GARBAGE          = 0xFF,
					COLOR            = b00000011,
					BEHIND           = b00100000,
					X_FLIP           = b01000000,
					Y_FLIP           = b10000000
				};

				struct Buffer
				{
					enum
					{
						XFINE     = b00000111,
						RANGE_MSB = b00001000,
						TILE_LSB  = b00000001
					};

					byte tile;
					byte x;
					byte attribute;
					byte comparitor;
				};

				struct Output
				{
					byte x;
					byte behind;
					byte zero;
					byte palette;
					byte pixels[8];
				};

				Output* visible;
				Buffer* evaluated;
				const Buffer* loaded;
				const Buffer* limit;

				uint mask;
				byte show[2];
				const byte padding0;
				const byte padding1;

				Output output[MAX_LINE_SPRITES];
				Buffer buffer[MAX_LINE_SPRITES];

				byte ram[SIZE];
			};

			struct NameTable
			{
				enum
				{
					SIZE = SIZE_2K,
					GARBAGE = 0x00
				};

				byte ram[SIZE];
			};

			struct TileLut
			{
				TileLut();

				byte block[0x400][4];
			};

			Cpu& cpu;

			struct
			{
				Cycle spriteOverflow;
				byte  one;
				byte  four;
				byte  eight;
				byte  six;
				Cycle count;
				Cycle round;
			}   cycles;

			Phase phase;

			struct
			{
				uint enabled;
				uint address;
				uint pattern;
				uint latch;
				uint buffer;
				Core::Io::Line a12;
			}   io;

			Regs regs;
			Scroll scroll;
			Tiles tiles;
			uint stage;
			Chr chr;
			Nmt nmt;
			int scanline;
			Output output;
			Hook bgHook;
			Hook spHook;
			const byte* rgbMap;
			const byte* yuvMap;
			Oam oam;
			Palette palette;
			NameTable nameTable;
			const TileLut tileLut;
			Video::Screen screen;

		public:

			ibool IsEnabled() const
			{
				return io.enabled;
			}

			bool IsActive() const
			{
				return io.enabled && scanline < Video::Screen::HEIGHT;
			}

			int GetScanline() const
			{
				return scanline;
			}

			uint GetCtrl0(uint flags) const
			{
				return regs.ctrl0 & flags;
			}

			uint GetCtrl1(uint flags) const
			{
				return regs.ctrl1 & flags;
			}

			template<typename A,typename B>
			void ConnectA12(A a,B b)
			{
				io.a12.Set( a, b );
			}

			void DisconnectA12()
			{
				io.a12.Invalidate();
			}

			bool IsA12Connected() const
			{
				return io.a12.InUse();
			}

			Video::Screen& GetScreen()
			{
				return screen;
			}

			Video::Screen::Pixel* GetOutputPixels()
			{
				return output.pixels;
			}

			void SetOutputPixels(Video::Screen::Pixel* pixels)
			{
				NST_ASSERT( pixels );
				output.pixels = pixels;
			}

			const Palette& GetPalette() const
			{
				return palette;
			}

			uint GetPixel(uint i) const
			{
				NST_ASSERT( i < Video::Screen::PIXELS );
				return output.pixels[i];
			}

			uint GetPixelCycles() const
			{
				return output.index;
			}

			uint GetYuvColor(uint i) const
			{
				NST_ASSERT( i < Palette::COLORS );
				return yuvMap ? yuvMap[i] : i;
			}

			ChrMem& GetChrMem()
			{
				return chr;
			}

			NmtMem& GetNmtMem()
			{
				return nmt;
			}

			Cycle GetOneCycle() const
			{
				return cycles.one;
			}

			uint GetVRamAddress() const
			{
				return scroll.address;
			}

			ibool IsShortFrame() const
			{
				return regs.ctrl1 & regs.frame;
			}

			uint GetBurstPhase() const
			{
				return output.burstPhase;
			}
		};
	}
}

#endif
