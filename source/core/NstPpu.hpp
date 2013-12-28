////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstIoPort.hpp"
#include "NstIoAccessor.hpp"
#include "NstIoLine.hpp"
#include "NstHook.hpp"
#include "NstMemory.hpp"
#include "NstVideoScreen.hpp"

namespace Nes
{
	namespace Core
	{
		namespace State
		{
			class Loader;
			class Saver;
		}

		class Cpu;

		class Ppu
		{
		public:

			Ppu(Cpu&);
			~Ppu();

			enum Mirroring
			{
				NMT_HORIZONTAL,
				NMT_VERTICAL,
				NMT_FOURSCREEN,
				NMT_ZERO,
				NMT_ONE,
				NMT_CONTROLLED
			};

			enum
			{
				MC_DIV_NTSC = 4,
				MC_DIV_PAL  = 5
			};

			enum
			{
				SCANLINES_VACTIVE     = 240,
				SCANLINES_VINT_NTSC   = 20,
				SCANLINES_VINT_PAL    = 70,
				SCANLINES_VSLEEP      = 1,
				SCANLINES_VDUMMY      = 1,
				SCANLINES_VBLANK_NTSC = SCANLINES_VSLEEP + SCANLINES_VINT_NTSC + SCANLINES_VDUMMY,
				SCANLINES_VBLANK_PAL  = SCANLINES_VSLEEP + SCANLINES_VINT_PAL + SCANLINES_VDUMMY,
				SCANLINES_VSYNC_NTSC  = SCANLINES_VACTIVE + SCANLINES_VBLANK_NTSC,
				SCANLINES_VSYNC_PAL   = SCANLINES_VACTIVE + SCANLINES_VBLANK_PAL
			};

			enum
			{
				CC_IO           = 2,
				CC_HACTIVE      = CC_IO * 4 * 32,
				CC_HBLANK       = CC_IO * 4 * 8 + CC_IO * 4 * 2 + CC_IO * 2 + 1,
				CC_HSYNC        = CC_HACTIVE + CC_HBLANK,
				CC_VINT_NTSC    = CC_HSYNC * SCANLINES_VINT_NTSC,
				CC_VINT_PAL     = CC_HSYNC * SCANLINES_VINT_PAL,
				CC_FRAME_0_NTSC = CC_HSYNC * SCANLINES_VSYNC_NTSC,
				CC_FRAME_1_NTSC = CC_HSYNC * SCANLINES_VSYNC_NTSC - 1,
				CC_FRAME_PAL    = CC_HSYNC * SCANLINES_VSYNC_PAL
			};

			void Reset(bool=false);
			void ClearScreen();
			void BeginFrame(ibool);
			void Update(Cycle=0);
			void EndFrame();

			void SetMode(Mode);
			void SetMirroring(uint);
			void SetMirroring(const uchar (&)[4]);
			void SetYuvMap(const u8*,bool);
			void SetBgHook(const Hook&);
			void SetSpHook(const Hook&);

			void EnableCpuSynchronization();

			void LoadState(State::Loader&);
			void SaveState(State::Saver&) const;

			void  EnableUnlimSprites(ibool);
			ibool AreUnlimSpritesEnabled() const;
			void  EnableEmphasis(ibool);
			uint  SolidColors() const;

			class ChrMem : public Memory<SIZE_8K,SIZE_1K,2>
			{
				friend class Ppu;

				NES_DECL_ACCESSOR( Pattern )

				NST_FORCE_INLINE uint FetchPattern(uint) const;

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
					return accessors[i].SameComponent( ptr );
				}
			};

			class NmtMem : public Memory<SIZE_4K,SIZE_1K,2>
			{
				friend class Ppu;

				NES_DECL_ACCESSOR( Name_2000 )
				NES_DECL_ACCESSOR( Name_2400 )
				NES_DECL_ACCESSOR( Name_2800 )
				NES_DECL_ACCESSOR( Name_2C00 )

				NST_FORCE_INLINE uint FetchName(uint) const;
				NST_FORCE_INLINE uint FetchAttribute(uint) const;

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
					return accessors[i][j].SameComponent( ptr );
				}
			};

		private:

			typedef void (Ppu::*Phase)();

			enum
			{
				WARM_UP_FRAMES = 2,
				SCANLINE_HDUMMY = -1,
				SCANLINE_VBLANK = 255
			};

			NES_DECL_POKE( 2000 )
			NES_DECL_PEEK( 2002 )
			NES_DECL_POKE( 2001 )
			NES_DECL_POKE( 2003 )
			NES_DECL_PEEK( 2004 )
			NES_DECL_POKE( 2004 )
			NES_DECL_POKE( 2005 )
			NES_DECL_POKE( 2006 )
			NES_DECL_PEEK( 2007 )
			NES_DECL_POKE( 2007 )
			NES_DECL_PEEK( 2xxx )
			NES_DECL_POKE( 2xxx )
			NES_DECL_PEEK( 4014 )
			NES_DECL_POKE( 4014 )

			NES_DECL_HOOK( Sync )
			NES_DECL_HOOK( Nop )

			inline bool IsDead() const;
			inline void UpdateScrollAddress(uint);

			NST_FORCE_INLINE uint FetchName() const;
			NST_FORCE_INLINE uint FetchAttribute() const;
			NST_FORCE_INLINE void EvaluateSprites();

			void UpdateStates();
			void LoadSprite();
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

			struct Cycles
			{
				Cycle spriteOverflow;
				Cycle count;
				u8    one;
				u8    four;
				u8    eight;
				u8    six;
				Cycle round;
			};

			struct Regs
			{
				enum
				{
					CTRL0_NAME_OFFSET = b00000011,
					CTRL0_INC32       = b00000100,
					CTRL0_SP_OFFSET   = b00001000,
					CTRL0_BG_OFFSET   = b00010000,
					CTRL0_SP8X16      = b00100000,
					CTRL0_NMI         = b10000000
				};

				enum
				{
					CTRL1_MONOCHROME     = b00000001,
					CTRL1_BG_NO_CLIPPING = b00000010,
					CTRL1_SP_NO_CLIPPING = b00000100,
					CTRL1_BG_ENABLED     = b00001000,
					CTRL1_SP_ENABLED     = b00010000,
					CTRL1_BG_COLOR       = b11100000,
					CTRL1_BG_COLOR_R     = b00100000,
					CTRL1_BG_COLOR_G     = b01000000,
					CTRL1_BG_COLOR_B     = b10000000,
					CTRL1_BG_COLOR_SHIFT = 5
				};

				enum
				{
					STATUS_LATCH       = b00011111,
					STATUS_SP_OVERFLOW = b00100000,
					STATUS_SP_ZERO_HIT = b01000000,
					STATUS_VBLANK      = b10000000,
					STATUS_BITS        = b11100000,
					STATUS_VBLANKING   = 0x100
				};

				enum
				{
					FRAME_ODD = CTRL1_BG_ENABLED|CTRL1_SP_ENABLED
				};

				uint ctrl0;
				uint ctrl1;
				uint status;
				uint frame;
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

				inline void ResetX();
				inline void ClockX();
				inline void ClockY();

				uint address;
				uint toggle;
				uint latch;
				uint increase;
				uint xFine;
				uint pattern;
			};

			struct Io
			{
				uint enabled;
				uint address;
				uint pattern;
				uint latch;
				uint buffer;
				Core::Io::Line a12;
			};

			struct Tiles
			{
				NST_FORCE_INLINE void Load();

				u8 pattern[2];
				u8 attribute;
				u8 pad;

				uint index;

				union
				{
					u8 pixels[16];
					u32 block[4];
				};

				uint show;
				uint clip;
			};

			struct Output
			{
				Output(Video::Screen::Pixels&);

				uint index;
				uint emphasis;
				uint coloring;
				u16* target;
				uint next;
				uint emphasisMask;
				u16* pixels;
				uint burstPhase;

				static u16 dummy[4];
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

				u8 ram[SIZE];
				u8 map[COLORS];
			};

			struct Oam
			{
				enum
				{
					SIZE = 0x100,
					NUM_SPRITES = SIZE / 4,
					OFFSET_TO_0_1 = b11111000,
					STD_LINE_SPRITES = 8,
					MAX_LINE_SPRITES = 32,
					DMA_CYCLES = 512 + 1,
					GARBAGE = 0xFF
				};

				enum
				{
					COLOR  = b00000011,
					BEHIND = b00100000,
					X_FLIP = b01000000,
					Y_FLIP = b10000000
				};

				struct Buffer
				{
					enum
					{
						XFINE     = b00000111,
						RANGE_MSB = b00001000,
						TILE_LSB  = b00000001
					};

					u8 tile;
					u8 x;
					u8 attribute;
					u8 comparitor;
				};

				struct Output
				{
					u8 x;
					u8 behind;
					u8 zero;
					u8 palette;

					union
					{
						u8 pixels[8];
						u32 block[2];
					};
				};

				Output* visible;
				Buffer* evaluated;
				const Buffer* loaded;
				const Buffer* limit;
				uint show;
				uint clip;

				Output output[MAX_LINE_SPRITES];
				Buffer buffer[MAX_LINE_SPRITES];

				uint address;
				u8 ram[SIZE];
			};

			struct NameTable
			{
				enum
				{
					SIZE = SIZE_2K,
					GARBAGE = 0x00
				};

				u8 ram[SIZE];
			};

			struct YuvMap
			{
				YuvMap(const u8*);

				u8 colors[Palette::COLORS];
			};

			Cpu& cpu;
			Cycles cycles;
			Phase phase;
			Io io;
			Regs regs;
			Scroll scroll;
			Tiles tiles;
			ChrMem chrMem;
			NmtMem nmtMem;
			uint stage;
			int scanline;
			Output output;
			Hook bgHook;
			Hook spHook;
			Palette palette;
			Oam oam;
			NameTable nameTable;
			const YuvMap* yuvMap;
			Video::Screen screen;

			static void LogMsg(cstring,uint,uint);

			template<size_t N>
			static inline void LogMsg(const char (&)[N],uint);

			static dword logged;

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

			u16* GetOutputPixels()
			{
				return output.pixels;
			}

			void SetOutputPixels(u16* pixels)
			{
				NST_ASSERT( pixels );
				output.pixels = pixels;
			}

			const Palette& GetPalette() const
			{
				return palette;
			}

			uint GetYuvPixel(uint i) const
			{
				NST_ASSERT( i < Video::Screen::PIXELS );
				return yuvMap ? yuvMap->colors[output.pixels[i] & 0x3F] : output.pixels[i];
			}

			uint GetPixelCycles() const
			{
				return output.index;
			}

			ChrMem& GetChrMem()
			{
				return chrMem;
			}

			NmtMem& GetNmtMem()
			{
				return nmtMem;
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
