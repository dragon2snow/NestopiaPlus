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

#include <cstring>
#include "NstLog.hpp"
#include "NstCpu.hpp"
#include "NstPpu.hpp"
#include "NstState.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		#if defined(NST_TAILCALL_OPTIMIZE) && defined(NDEBUG)

          #define NES_PPU_NEXT_PHASE(function_) \
												\
          if (cycles.count < cycles.round)      \
              function_();                      \
          else                                  \
              phase = &Ppu::function_

		#else

          #define NES_PPU_NEXT_PHASE(function_) phase = &Ppu::function_

		#endif

		Ppu::Tiles::Tiles()
		: padding0(0), padding1(0) {}

		Ppu::Oam::Oam()
		: limit(buffer + STD_LINE_SPRITES), padding0(0), padding1(0) {}

		Ppu::Output::Output(Video::Screen::Pixel* p)
		: pixels(p) {}

		Ppu::TileLut::TileLut()
		{
			for (uint i=0; i < 0x400; ++i)
			{
				block[i][0] = (i & 0x03) ? (i >> 6 & 0xC) | (i >> 0 & 0x3) : 0;
				block[i][1] = (i & 0x0C) ? (i >> 6 & 0xC) | (i >> 2 & 0x3) : 0;
				block[i][2] = (i & 0x30) ? (i >> 6 & 0xC) | (i >> 4 & 0x3) : 0;
				block[i][3] = (i & 0xC0) ? (i >> 6 & 0xC) | (i >> 6 & 0x3) : 0;
			}
		}

		#if NST_MSVC >= 1200
		#pragma warning( push )
		#pragma warning( disable : 4355 )
		#endif

		Ppu::Ppu(Cpu& c)
		:
		cpu    (c),
		output (screen.pixels),
		bgHook (this,&Ppu::Hook_Nop),
		spHook (this,&Ppu::Hook_Nop),
		rgbMap (NULL),
		yuvMap (NULL)
		{
			SetMode( cpu.GetMode() );
		}

		#if NST_MSVC >= 1200
		#pragma warning( pop )
		#endif

		void Ppu::Reset(const bool hard)
		{
			for (uint i=0x2000; i < 0x4000; i += 0x8)
			{
				cpu.Map( i+0 ).Set( this, &Ppu::Peek_2xxx, &Ppu::Poke_2000 );
				cpu.Map( i+1 ).Set( this, &Ppu::Peek_2xxx, &Ppu::Poke_2001 );
				cpu.Map( i+2 ).Set( this, &Ppu::Peek_2002, &Ppu::Poke_2xxx );
				cpu.Map( i+3 ).Set( this, &Ppu::Peek_2xxx, &Ppu::Poke_2003 );
				cpu.Map( i+4 ).Set( this, &Ppu::Peek_2004, &Ppu::Poke_2004 );
				cpu.Map( i+5 ).Set( this, &Ppu::Peek_2xxx, &Ppu::Poke_2005 );
				cpu.Map( i+6 ).Set( this, &Ppu::Peek_2xxx, &Ppu::Poke_2006 );
				cpu.Map( i+7 ).Set( this, &Ppu::Peek_2007, &Ppu::Poke_2007 );
			}

			cpu.Map( 0x4014U ).Set( this, &Ppu::Peek_4014, &Ppu::Poke_4014 );

			if (hard)
			{
				io.a12.Invalidate();

				static const byte powerUpPalette[] =
				{
					0x3F,0x01,0x00,0x01,0x00,0x02,0x02,0x0D,
					0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2C,
					0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,
					0x08,0x3A,0x00,0x02,0x00,0x20,0x2C,0x08
				};

				std::memset( oam.ram, Oam::GARBAGE, Oam::SIZE );
				std::memcpy( palette.ram, powerUpPalette, Palette::SIZE );
				std::memset( nameTable.ram, NameTable::GARBAGE, NameTable::SIZE );

				io.latch = 0;
				io.buffer = 0;

				stage = WARM_UP_FRAMES;
				phase = &Ppu::WarmUp;

				regs.ctrl0 = 0;
				regs.ctrl1 = 0;
				regs.frame = 0;
				regs.status = 0;

				scroll.address = 0;
				scroll.toggle = 0;
				scroll.latch = 0;
				scroll.xFine = 0;

				output.burstPhase = 0;
			}
			else
			{
				stage = 0;
				phase = &Ppu::HDummy;

				regs.status = Regs::STATUS_VBLANK;
			}

			if (chr.Source().Empty())
			{
				chr.Source().Set( nameTable.ram, NameTable::SIZE, true, false );
				chr.SwapBanks<SIZE_2K,0x0000>(0,0,0,0);
			}

			if (nmt.Source().Empty())
			{
				nmt.Source().Set( nameTable.ram, NameTable::SIZE, true, true );
				nmt.SwapBanks<SIZE_2K,0x0000>(0,0);
			}

			chr.ResetAccessors();
			nmt.ResetAccessors();

			cycles.count = Cpu::CYCLE_MAX;
			cycles.spriteOverflow = Cpu::CYCLE_MAX;
			cycles.round = 0;

			regs.oam = 0;

			io.address = 0;
			io.pattern = 0;

			scanline = SCANLINE_VBLANK;

			tiles.pattern[0] = 0;
			tiles.pattern[1] = 0;
			tiles.attribute = 0;
			tiles.mask = 0;

			yuvMap = NULL;
			rgbMap = NULL;

			oam.visible = oam.output;
			oam.evaluated = oam.buffer;
			oam.loaded = oam.buffer;
			oam.mask = 0;

			output.index = 0;
			output.target = NULL;

			bgHook = Hook( this, &Ppu::Hook_Nop );
			spHook = Hook( this, &Ppu::Hook_Nop );

			UpdateStates();

			screen.Clear();
		}

		void Ppu::SetBgHook(const Hook& hook)
		{
			bgHook = hook;
		}

		void Ppu::SetSpHook(const Hook& hook)
		{
			spHook = hook;
		}

		void Ppu::EnableUnlimSprites(bool state)
		{
			oam.limit = oam.buffer + (state ? Oam::MAX_LINE_SPRITES : Oam::STD_LINE_SPRITES);
		}

		bool Ppu::AreUnlimSpritesEnabled() const
		{
			return oam.limit == (oam.buffer + Oam::MAX_LINE_SPRITES);
		}

		void Ppu::ClearScreen()
		{
			screen.Clear();
		}

		void Ppu::UpdateStates()
		{
			io.enabled = regs.ctrl1 & (Regs::CTRL1_BG_ENABLED|Regs::CTRL1_SP_ENABLED);
			scroll.increase = (regs.ctrl0 & Regs::CTRL0_INC32) ? 32 : 1;
			scroll.pattern = (regs.ctrl0 & Regs::CTRL0_BG_OFFSET) << 8;
			tiles.show[0] = (regs.ctrl1 & Regs::CTRL1_BG_ENABLED) ? 0xFF : 0x00;
			tiles.show[1] = (regs.ctrl1 & Regs::CTRL1_FULL_BG_ENABLED) == Regs::CTRL1_FULL_BG_ENABLED ? 0xFF : 0x00;
			oam.show[0] = (regs.ctrl1 & Regs::CTRL1_SP_ENABLED) ? 0xFF : 0x00;
			oam.show[1] = (regs.ctrl1 & Regs::CTRL1_FULL_SP_ENABLED) == Regs::CTRL1_FULL_SP_ENABLED ? 0xFF : 0x00;

			UpdatePalette();
		}

		void Ppu::UpdatePalette()
		{
			for (uint i=0, c=Coloring(), e=Emphasis(); i < Palette::SIZE; ++i)
				output.palette[i] = (rgbMap ? rgbMap[palette.ram[i] & uint(Palette::COLOR)] : palette.ram[i]) & c | e;
		}

		void Ppu::SaveState(State::Saver& state,const dword id) const
		{
			state.Begin( id );

			{
				const byte data[11] =
				{
					regs.ctrl0,
					regs.ctrl1,
					regs.status,
					scroll.address & 0xFF,
					scroll.address >> 8,
					scroll.latch & 0xFF,
					scroll.latch >> 8,
					scroll.xFine | scroll.toggle << 3,
					regs.oam,
					io.buffer,
					io.latch
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			state.Begin( AsciiId<'P','A','L'>::V ).Compress( palette.ram   ).End();
			state.Begin( AsciiId<'O','A','M'>::V ).Compress( oam.ram       ).End();
			state.Begin( AsciiId<'N','M','T'>::V ).Compress( nameTable.ram ).End();

			if (cpu.GetMode() == MODE_NTSC)
				state.Begin( AsciiId<'F','R','M'>::V ).Write8( (regs.frame & Regs::FRAME_ODD) == 0 ).End();

			if (phase == &Ppu::WarmUp)
				state.Begin( AsciiId<'P','O','W'>::V ).Write8( WARM_UP_FRAMES-stage ).End();

			state.End();
		}

		void Ppu::LoadState(State::Loader& state)
		{
			phase = &Ppu::HDummy;
			regs.frame = 0;
			stage = 0;
			output.burstPhase = 0;

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<11> data( state );

						regs.ctrl0     = data[0];
						regs.ctrl1     = data[1];
						regs.status    = data[2] & Regs::STATUS_BITS;
						scroll.address = data[3] | (data[4] << 8 & 0x7F00);
						scroll.latch   = data[5] | (data[6] << 8 & 0x7F00);
						scroll.xFine   = data[7] & 0x7;
						scroll.toggle  = data[7] >> 3 & 0x1;
						regs.oam       = data[8];
						io.buffer      = data[9];
						io.latch       = data[10];

						break;
					}

					case AsciiId<'P','A','L'>::V:

						state.Uncompress( palette.ram );
						break;

					case AsciiId<'O','A','M'>::V:

						state.Uncompress( oam.ram );
						break;

					case AsciiId<'N','M','T'>::V:

						state.Uncompress( nameTable.ram );
						break;

					case AsciiId<'F','R','M'>::V:

						if (cpu.GetMode() == MODE_NTSC)
							regs.frame = (state.Read8() & 0x1) ? 0 : Regs::FRAME_ODD;

						break;

					case AsciiId<'P','O','W'>::V:

						stage = WARM_UP_FRAMES - (state.Read8() & 0x7);

						if (stage > WARM_UP_FRAMES)
							stage = WARM_UP_FRAMES;

						phase = &Ppu::WarmUp;
						break;
				}

				state.End();
			}

			UpdateStates();
		}

		void Ppu::EnableCpuSynchronization()
		{
			cpu.AddHook( Hook(this,&Ppu::Hook_Sync) );
		}

		void Ppu::ChrMem::ResetAccessors()
		{
			accessors[0].Set( this, &ChrMem::Access_Pattern );
			accessors[1].Set( this, &ChrMem::Access_Pattern );
		}

		void Ppu::ChrMem::SetDefaultAccessor(uint i)
		{
			NST_ASSERT( i < 2 );
			accessors[i].Set( this, &ChrMem::Access_Pattern );
		}

		void Ppu::NmtMem::ResetAccessors()
		{
			accessors[0][0].Set( this, &NmtMem::Access_Name_2000 );
			accessors[0][1].Set( this, &NmtMem::Access_Name_2000 );
			accessors[1][0].Set( this, &NmtMem::Access_Name_2400 );
			accessors[1][1].Set( this, &NmtMem::Access_Name_2400 );
			accessors[2][0].Set( this, &NmtMem::Access_Name_2800 );
			accessors[2][1].Set( this, &NmtMem::Access_Name_2800 );
			accessors[3][0].Set( this, &NmtMem::Access_Name_2C00 );
			accessors[3][1].Set( this, &NmtMem::Access_Name_2C00 );
		}

		void Ppu::NmtMem::SetDefaultAccessor(uint i,uint j)
		{
			NST_ASSERT( i < 4 && j < 2 );

			accessors[i][j].Set
			(
				this,
				i == 0 ? &NmtMem::Access_Name_2000 :
				i == 1 ? &NmtMem::Access_Name_2400 :
				i == 2 ? &NmtMem::Access_Name_2800 :
                         &NmtMem::Access_Name_2C00
			);
		}

		void Ppu::NmtMem::SetDefaultAccessors(uint i)
		{
			SetDefaultAccessor( i, 0 );
			SetDefaultAccessor( i, 1 );
		}

		void Ppu::SetMode(const Mode mode)
		{
			regs.frame = 0;
			output.burstPhase = 0;

			if (mode == MODE_NTSC)
			{
				cycles.one   = MC_DIV_NTSC * 1;
				cycles.four  = MC_DIV_NTSC * 4;
				cycles.eight = MC_DIV_NTSC * 8;
				cycles.six   = MC_DIV_NTSC * 6;
			}
			else
			{
				cycles.one   = MC_DIV_PAL * 1;
				cycles.four  = MC_DIV_PAL * 4;
				cycles.eight = MC_DIV_PAL * 8;
				cycles.six   = MC_DIV_PAL * 6;
			}
		}

		void Ppu::SetYuvMap(const byte* map,bool preEffect)
		{
			yuvMap = preEffect ? NULL : map;
			rgbMap = preEffect ? map : NULL;

			UpdatePalette();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Ppu::BeginFrame()
		{
			NST_ASSERT
			(
				scanline == SCANLINE_VBLANK &&
				(phase == &Ppu::HDummy || phase == &Ppu::WarmUp) &&
				(cpu.GetMode() == MODE_NTSC) == (cycles.one == MC_DIV_NTSC)
			);

			output.target = output.pixels;

			Cycle frame;

			if (cycles.one == MC_DIV_NTSC)
			{
				regs.frame ^= Regs::FRAME_ODD;
				cycles.count = MC_DIV_NTSC * dword(CC_VINT_NTSC);

				if (phase != &Ppu::WarmUp)
					frame = MC_DIV_NTSC * dword(CC_FRAME_0_NTSC);
				else
					frame = MC_DIV_NTSC * dword(CC_FRAME_0_NTSC - CC_VINT_NTSC);
			}
			else
			{
				NST_ASSERT( regs.frame == 0 && output.burstPhase == 0 );

				cycles.count = MC_DIV_PAL * dword(CC_VINT_PAL);
				frame = MC_DIV_PAL * dword(CC_FRAME_PAL);
			}

			cpu.SetupFrame( frame );
		}

		NES_HOOK(Ppu,Nop)
		{
		}

		NES_HOOK(Ppu,Sync)
		{
			const Cycle elapsed = cpu.GetMasterClockCycles();

			if (cycles.count < elapsed)
			{
				cycles.round = elapsed;

			#if defined(NST_TAILCALL_OPTIMIZE) && defined(NDEBUG)
				(*this.*phase)();
			#else
				do
				{
					(*this.*phase)();
				}
				while (cycles.count < elapsed);
			#endif
			}
		}

		void Ppu::EndFrame()
		{
			if (cycles.count != Cpu::CYCLE_MAX)
			{
				cycles.round = Cpu::CYCLE_MAX;

			#if defined(NST_TAILCALL_OPTIMIZE) && defined(NDEBUG)
				(*this.*phase)();
			#else
				do
				{
					(*this.*phase)();
				}
				while (cycles.count != Cpu::CYCLE_MAX);
			#endif
			}
		}

		void Ppu::Update(Cycle dataSetup)
		{
			dataSetup += cpu.GetMasterClockCycles();

			if (cycles.count < dataSetup)
			{
				cycles.round = dataSetup;

			#if defined(NST_TAILCALL_OPTIMIZE) && defined(NDEBUG)
				(*this.*phase)();
			#else
				do
				{
					(*this.*phase)();
				}
				while (cycles.count < dataSetup);
			#endif
			}
		}

		void Ppu::SetMirroring(const byte (&banks)[4])
		{
			Update( cycles.one );

			NST_ASSERT( banks[0] < 4 && banks[1] < 4 && banks[2] < 4 && banks[3] < 4 );
			nmt.SwapBanks<SIZE_1K,0x0000>( banks[0], banks[1], banks[2], banks[3] );
		}

		void Ppu::SetMirroring(uint type)
		{
			NST_ASSERT( type < 6 );
			NST_VERIFY( type < 5 );

			NST_COMPILE_ASSERT
			(
				NMT_HORIZONTAL == 0 &&
				NMT_VERTICAL   == 1 &&
				NMT_FOURSCREEN == 2 &&
				NMT_ZERO       == 3 &&
				NMT_ONE        == 4 &&
				NMT_CONTROLLED == 5
			);

			static const byte banks[6][4] =
			{
				{0,0,1,1}, // horizontal
				{0,1,0,1}, // vertical
				{0,1,2,3}, // four-screen
				{0,0,0,0}, // banks #1
				{1,1,1,1}, // banks #2
				{0,1,0,1}  // controlled, default to vertical
			};

			SetMirroring( banks[type] );
		}

		NES_ACCESSOR(Ppu::ChrMem,Pattern)
		{
			return Peek( address );
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2000)
		{
			return (*this)[0][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2400)
		{
			return (*this)[1][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2800)
		{
			return (*this)[2][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2C00)
		{
			return (*this)[3][address];
		}

		NST_FORCE_INLINE uint Ppu::Chr::FetchPattern(uint address) const
		{
			address &= 0x1FFF;
			return accessors[address >> 12].Fetch( address );
		}

		NST_FORCE_INLINE uint Ppu::Nmt::FetchName(uint address) const
		{
			const uint offset = address & 0x03FF;
			return accessors[address >> 10 & 0x3][(offset + 0x40) >> 10].Fetch( offset );
		}

		NST_FORCE_INLINE uint Ppu::Nmt::FetchAttribute(uint address) const
		{
			return accessors[address >> 10 & 0x3][1].Fetch( 0x3C0 | (address >> 4 & 0x038) | (address >> 2 & 0x007) );
		}

		NST_FORCE_INLINE uint Ppu::FetchName() const
		{
			return nmt.FetchName( io.address ) << 4 | scroll.address >> 12 | scroll.pattern;
		}

		NST_FORCE_INLINE uint Ppu::FetchAttribute() const
		{
			return nmt.FetchAttribute( io.address ) >> ((scroll.address & 0x2) | (scroll.address >> 4 & 0x4)) & 0x3;
		}

		inline bool Ppu::IsDead() const
		{
			return !io.enabled || scanline >= 240;
		}

		inline void Ppu::UpdateScrollAddress(const uint newAddress)
		{
			const uint oldAddress = scroll.address;
			scroll.address = newAddress;

			if (io.a12.InUse() && (newAddress & 0x1000) > (oldAddress & 0x1000))
				io.a12.Toggle( cpu.GetMasterClockCycles() );
		}

		inline uint Ppu::Coloring() const
		{
			return (regs.ctrl1 & Regs::CTRL1_MONOCHROME) ? Palette::MONO : Palette::COLOR;
		}

		inline uint Ppu::Emphasis() const
		{
			return (regs.ctrl1 & Regs::CTRL1_EMPHASIS) << 1;
		}

		NES_POKE(Ppu,2000)
		{
			Update( cycles.one );

			scroll.latch = (scroll.latch & 0x73FF) | (data & 0x03) << 10;
			scroll.increase = (data & Regs::CTRL0_INC32) ? 32 : 1;
			scroll.pattern = (data & Regs::CTRL0_BG_OFFSET) << 8;

			io.latch = data;
			data = regs.ctrl0;
			regs.ctrl0 = io.latch;

			if ((regs.ctrl0 & regs.status & Regs::CTRL0_NMI) > data)
				cpu.DoNMI();
		}

		NES_POKE(Ppu,2001)
		{
			Update( cycles.one );

			io.enabled = data & (Regs::CTRL1_BG_ENABLED|Regs::CTRL1_SP_ENABLED);

			tiles.show[0] = (data & Regs::CTRL1_BG_ENABLED) ? 0xFF : 0x00;
			tiles.show[1] = (data & Regs::CTRL1_FULL_BG_ENABLED) == Regs::CTRL1_FULL_BG_ENABLED ? 0xFF : 0x00;

			oam.show[0] = (data & Regs::CTRL1_SP_ENABLED) ? 0xFF : 0x00;
			oam.show[1] = (data & Regs::CTRL1_FULL_SP_ENABLED) == Regs::CTRL1_FULL_SP_ENABLED ? 0xFF : 0x00;

			const uint pos = ((output.index & 0xFFU) - 8) >= (256-16);

			tiles.mask = tiles.show[pos];
			oam.mask = oam.show[pos];

			io.latch = data;
			data = (regs.ctrl1 ^ data) & (Regs::CTRL1_EMPHASIS|Regs::CTRL1_MONOCHROME);
			regs.ctrl1 = io.latch;

			if (data)
			{
				const uint c = Coloring();
				const uint e = Emphasis();

				const byte* const NST_RESTRICT map = rgbMap;

				if (!map)
				{
					for (uint i=0; i < Palette::SIZE; ++i)
						output.palette[i] = palette.ram[i] & c | e;
				}
				else
				{
					for (uint i=0; i < Palette::SIZE; ++i)
						output.palette[i] = map[palette.ram[i] & Palette::COLOR] & c | e;
				}
			}
		}

		NES_PEEK(Ppu,2002)
		{
			Update( cycles.one );

			uint status = regs.status & 0xFF;

			if (cycles.spriteOverflow <= cpu.GetMasterClockCycles())
				status |= Regs::STATUS_SP_OVERFLOW;

			regs.status &= (Regs::STATUS_VBLANK^0xFFU);
			scroll.toggle = 0;
			io.latch = (io.latch & Regs::STATUS_LATCH) | status;

			return io.latch;
		}

		NES_POKE(Ppu,2003)
		{
			Update( cycles.one );

			regs.oam = data;
			io.latch = data;
		}

		NES_POKE(Ppu,2004)
		{
			Update( cycles.one );

			NST_ASSERT( regs.oam < Oam::SIZE );
			NST_VERIFY( IsDead() );

			if (!IsDead())
				data = Oam::GARBAGE;

			byte* const NST_RESTRICT value = oam.ram + regs.oam;
			regs.oam = (regs.oam + 1) & 0xFF;
			io.latch = data;
			*value = data;
		}

		NES_PEEK(Ppu,2004)
		{
			Update( cycles.one );

			NST_ASSERT( regs.oam < Oam::SIZE );

			uint data;

			if (IsDead())
			{
				data = oam.ram[regs.oam];
			}
			else
			{
				data = cpu.GetMasterClockCycles() / cycles.one % 341 + (cpu.GetMasterClockFrameCycles() == MC_DIV_NTSC*dword(CC_FRAME_1_NTSC));

				if (data < 64)
				{
					data = Oam::GARBAGE;
				}
				else if (data < 192)
				{
					data = oam.ram[(data-64) << 1 & 0xFC];
				}
				else if (data < 256)
				{
					data = oam.ram[(data & 1) ? 0xFC : (data-192) << 1 & 0xFC];
				}
				else if (data < 320)
				{
					data = Oam::GARBAGE;
				}
				else
				{
					data = oam.ram[0];
				}
			}

			io.latch = data;

			return data;
		}

		NES_POKE(Ppu,2005)
		{
			Update( cycles.one );

			io.latch = data;

			if (scroll.toggle ^= 1)
			{
				scroll.latch = (scroll.latch & 0x7FE0) | (data >> 3);
				scroll.xFine = data & 0x7;
			}
			else
			{
				scroll.latch = (scroll.latch & 0x0C1F) | ((data << 2 | data << 12) & 0x73E0);
			}
		}

		NES_POKE(Ppu,2006)
		{
			Update( cycles.one );

			io.latch = data;

			if (scroll.toggle ^= 1)
			{
				scroll.latch = (scroll.latch & 0x00FF) | (data & 0x3F) << 8;
			}
			else
			{
				scroll.latch = (scroll.latch & 0x7F00) | data;

				UpdateScrollAddress( scroll.latch );
			}
		}

		NES_POKE(Ppu,2007)
		{
			Update( cycles.four );

			NST_VERIFY( IsDead() );

			address = scroll.address;
			UpdateScrollAddress( (scroll.address + scroll.increase) & 0x7FFF );

			io.latch = data;

			if ((address & 0x3F00) == 0x3F00)
			{
				address &= 0x1F;

				const uint final = (!rgbMap ? data : rgbMap[data & Palette::COLOR]) & Coloring() | Emphasis();

				palette.ram[address] = data;
				output.palette[address] = final;

				if (!(address & 0x3))
				{
					palette.ram[address ^ 0x10] = data;
					output.palette[address ^ 0x10] = final;
				}
			}
			else
			{
				address &= 0x3FFF;

				if (address >= 0x2000)
					nmt.Poke( address & 0xFFF, data );
				else
					chr.Poke( address, data );
			}
		}

		NES_PEEK(Ppu,2007)
		{
			Update( cycles.one );

			NST_VERIFY( IsDead() );

			address = scroll.address & 0x3FFF;

			UpdateScrollAddress( (scroll.address + scroll.increase) & 0x7FFF );

			io.latch = (address & 0x3F00) != 0x3F00 ? io.buffer : palette.ram[address & 0x1F] & Coloring();
			io.buffer = (address >= 0x2000 ? nmt.FetchName( address ) : chr.FetchPattern( address ));

			return io.latch;
		}

		NES_POKE(Ppu,2xxx)
		{
			io.latch = data;
		}

		NES_PEEK(Ppu,2xxx)
		{
			return io.latch;
		}

		NES_POKE(Ppu,4014)
		{
			Update( cycles.one );

			NST_ASSERT( regs.oam < Oam::SIZE );
			NST_VERIFY( IsDead() );

			cpu.StealCycles( cpu.GetMasterClockCycle(1) * Oam::DMA_CYCLES );

			if (IsDead())
			{
				data <<= 8;

				if (regs.oam == 0x00 && data < 0x2000)
				{
					std::memcpy( oam.ram, cpu.SystemRam(data & (Cpu::RAM_SIZE-1)), Oam::SIZE );
					io.latch = oam.ram[0xFF];
				}
				else
				{
					uint dst = (regs.oam - 1U) & 0xFF;
					const uint end = dst;
					uint tmp;

					do
					{
						oam.ram[(dst = (dst + 1) & 0xFF)] = (tmp = cpu.Peek( data++ ));
					}
					while (dst != end);

					io.latch = tmp;
				}
			}
			else
			{
				io.latch = Oam::GARBAGE;
				std::memset( oam.ram, Oam::GARBAGE, Oam::SIZE );
			}
		}

		NES_PEEK(Ppu,4014)
		{
			return 0x40;
		}

		NST_FORCE_INLINE void Ppu::Scroll::ClockX()
		{
			if ((address & X_TILE) != X_TILE)
				++address;
			else
				address ^= (X_TILE|NAME_LOW);
		}

		NST_FORCE_INLINE void Ppu::Scroll::ResetX()
		{
			address &= (X_TILE|NAME_LOW) ^ 0x7FFFU;
			address |= latch & (X_TILE|NAME_LOW);
		}

		NST_FORCE_INLINE void Ppu::Scroll::ClockY()
		{
			if ((address & Y_FINE) != (7U << 12))
			{
				address += (1U << 12);
			}
			else switch (address & Y_TILE)
			{
				default:         address = (address & (Y_FINE ^ 0x7FFFU)) + (1U << 5); break;
				case (29U << 5): address ^= NAME_HIGH;
				case (31U << 5): address &= (Y_FINE|Y_TILE) ^ 0x7FFFU; break;
			}
		}

		NST_FORCE_INLINE void Ppu::EvaluateSprites()
		{
			NST_ASSERT( scanline >= 0 && scanline <= 239 && regs.oam < Oam::SIZE );

			oam.evaluated = oam.buffer;

			if (io.enabled)
			{
				uint i = 0;
				const uint height = (regs.ctrl0 >> 2 & 8) + 8;

				for (;;)
				{
					const byte* const NST_RESTRICT obj = oam.ram + i + (regs.oam & (i >= 8 ? 0 : Oam::OFFSET_TO_0_1));
					i += 4;

					uint y = uint(scanline) - obj[0];

					if (y >= height)
					{
						if (i == Oam::SIZE)
							return;
					}
					else
					{
						Oam::Buffer* const NST_RESTRICT eval = oam.evaluated;

						eval->comparitor = y ^ ((obj[2] & uint(Oam::Y_FLIP)) ? 0xF : 0x0);

						eval->attribute =
						(
							(i == 4) |
							(obj[2] & uint(Oam::COLOR)) << 2 |
							(obj[2] & uint(Oam::BEHIND|Oam::X_FLIP))
						);

						eval->tile = obj[1];
						eval->x = obj[3];

						++oam.evaluated;

						if (i == Oam::SIZE)
							return;

						if (oam.evaluated == oam.limit)
							break;
					}
				}

				if (cycles.spriteOverflow == Cpu::CYCLE_MAX)
				{
					do
					{
						if (uint(scanline) - oam.ram[i] >= height)
						{
							i = ((i + 4) & 0x1FC) + ((i + 1) & 0x03);
						}
						else
						{
							cycles.spriteOverflow = cycles.count + cycles.one * (45 + i/4 * 2);
							break;
						}
					}
					while (i < Oam::SIZE);
				}
			}

		}

		void Ppu::LoadSprite()
		{
			NST_ASSERT( oam.loaded < oam.evaluated );

			uint address;

			if (regs.ctrl0 & Regs::CTRL0_SP8X16)
			{
				address =
				(
					((oam.loaded->tile & uint(Oam::Buffer::TILE_LSB)) << 12) |
					((oam.loaded->tile & (Oam::Buffer::TILE_LSB ^ 0xFFU)) << 4) |
					((oam.loaded->comparitor & uint(Oam::Buffer::RANGE_MSB)) << 1)
				);
			}
			else
			{
				address = (regs.ctrl0 & Regs::CTRL0_SP_OFFSET) << 9 | oam.loaded->tile << 4;
			}

			address |= oam.loaded->comparitor & uint(Oam::Buffer::XFINE);

			if (io.a12.InUse() && address & 0x1000)
				io.a12.Toggle( cycles.count );

			uint pattern[2] =
			{
				chr.FetchPattern( address | 0x0 ),
				chr.FetchPattern( address | 0x8 )
			};

			if (pattern[0] | pattern[1])
			{
				address = (oam.loaded->attribute & uint(Oam::X_FLIP)) ? 7 : 0;

				uint p =
				(
					(pattern[0] >> 1 & 0x0055) | (pattern[1] << 0 & 0x00AA) |
					(pattern[0] << 8 & 0x5500) | (pattern[1] << 9 & 0xAA00)
				);

				Oam::Output* const NST_RESTRICT entry = oam.visible++;

				entry->pixels[( address^=6 )] = ( p       ) & 0x3;
				entry->pixels[( address^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=6 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=7 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=6 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=2 )] = ( p >>= 2 );

				const uint attribute = oam.loaded->attribute;

				entry->x       = oam.loaded->x;
				entry->palette = Palette::SPRITE_OFFSET + (attribute & uint(Oam::COLOR) << 2);
				entry->zero    = (attribute & 0x1) ? 0x3 : 0x0;
				entry->behind  = (attribute & Oam::BEHIND) ? 0x3 : 0x0;
			}

			++oam.loaded;
		}

		NST_FORCE_INLINE void Ppu::LoadTiles()
		{
			const byte* const NST_RESTRICT src[] =
			{
				tileLut.block[((tiles.pattern[0] & 0xAAU) >> 1 | (tiles.pattern[1] & 0xAAU) << 0) | (uint(tiles.attribute) << 8)],
				tileLut.block[((tiles.pattern[0] & 0x55U) >> 0 | (tiles.pattern[1] & 0x55U) << 1) | (uint(tiles.attribute) << 8)]
			};

			byte* const NST_RESTRICT dst = tiles.pixels + tiles.index;
			tiles.index ^= 8U;

			dst[6] = src[0][0];
			dst[4] = src[0][1];
			dst[2] = src[0][2];
			dst[0] = src[0][3];
			dst[7] = src[1][0];
			dst[5] = src[1][1];
			dst[3] = src[1][2];
			dst[1] = src[1][3];
		}

		NST_FORCE_INLINE void Ppu::RenderPixel()
		{
			uint pixel = 0;

			if (io.enabled)
			{
				const uint index = output.index & 0xFF;
				pixel = tiles.pixels[(index + scroll.xFine) & 15] & tiles.mask;

				for (const Oam::Output* NST_RESTRICT sprite=oam.output, *const end=oam.visible; sprite != end; ++sprite)
				{
					uint x = index - sprite->x;

					if (x > 7)
						continue;

					x = sprite->pixels[x] & oam.mask;

					if (x)
					{
						// first two bits of sprite->zero and sprite->behind booleans
						// are masked if true (for minimizing branching)

						if (pixel & sprite->zero)
							regs.status |= Regs::STATUS_SP_ZERO_HIT;

						if (!(pixel & sprite->behind))
							pixel = sprite->palette + x;

						break;
					}
				}
			}
			else if ((scroll.address & 0x3F00) == 0x3F00)
			{
				pixel = scroll.address & 0x1F;
			}

			++output.index;
			Video::Screen::Pixel* const NST_RESTRICT target = output.target++;
			*target = output.palette[pixel];
		}

		void Ppu::HActive0()
		{
			LoadTiles();

			cycles.count += cycles.one;

			if (io.enabled)
				io.address = scroll.address;

			RenderPixel();

			if (stage != 8)
			{
				NES_PPU_NEXT_PHASE( HActive1 );
			}
			else
			{
				EvaluateSprites();
				NES_PPU_NEXT_PHASE( HActive1 );
			}
		}

		void Ppu::HActive1()
		{
			cycles.count += cycles.one;

			if (io.enabled)
				io.pattern = FetchName();

			RenderPixel();
			NES_PPU_NEXT_PHASE( HActive2 );
		}

		void Ppu::HActive2()
		{
			cycles.count += cycles.one;

			if (io.enabled)
				io.address = scroll.address;

			RenderPixel();
			NES_PPU_NEXT_PHASE( HActive3 );
		}

		void Ppu::HActive3()
		{
			cycles.count += cycles.one;

			if (io.enabled)
			{
				tiles.attribute = FetchAttribute();
				scroll.ClockX();

				if (stage == 31)
					scroll.ClockY();
			}

			RenderPixel();
			NES_PPU_NEXT_PHASE( HActive4 );
		}

		void Ppu::HActive4()
		{
			if (io.enabled)
			{
				io.address = io.pattern | 0x0;

				if (io.a12.InUse() && scroll.pattern)
					io.a12.Toggle( cycles.count );
			}

			cycles.count += cycles.one;

			RenderPixel();
			NES_PPU_NEXT_PHASE( HActive5 );
		}

		void Ppu::HActive5()
		{
			cycles.count += cycles.one;

			if (io.enabled)
				tiles.pattern[0] = chr.FetchPattern( io.address );

			RenderPixel();
			NES_PPU_NEXT_PHASE( HActive6 );
		}

		void Ppu::HActive6()
		{
			cycles.count += cycles.one;

			if (io.enabled)
				io.address = io.pattern | 0x8;

			RenderPixel();
			NES_PPU_NEXT_PHASE( HActive7 );
		}

		void Ppu::HActive7()
		{
			cycles.count += cycles.one;

			const uint noSpHitOn255 = regs.status;

			RenderPixel();

			tiles.mask = tiles.show[0];
			oam.mask = oam.show[0];

			if (io.enabled)
				tiles.pattern[1] = chr.FetchPattern( io.address );

			NST_ASSERT( stage < 32 );

			stage = (stage + 1) & 31;

			if (stage)
			{
				NES_PPU_NEXT_PHASE( HActive0 );
			}
			else
			{
				cycles.count += cycles.one;
				regs.status = noSpHitOn255;

				NES_PPU_NEXT_PHASE( HBlank );
			}
		}

		void Ppu::HBlank()
		{
			NST_ASSERT( stage == 0 );

			spHook.Execute();

			cycles.count += cycles.four - cycles.one;

			oam.loaded = oam.buffer;
			oam.visible = oam.output;

			if (io.enabled)
				scroll.ResetX();

			NES_PPU_NEXT_PHASE( HBlankSp );
		}

		void Ppu::HBlankSp()
		{
		hell:

			if (io.enabled)
			{
				if (oam.loaded == oam.evaluated)
				{
					if (io.a12.InUse() && regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16))
						io.a12.Toggle( cycles.count );
				}
				else
				{
					LoadSprite();
				}
			}

			NST_ASSERT( stage < 8 );

			stage = (stage + 1) & 7;

			if (stage)
			{
				cycles.count += cycles.eight;

				if (cycles.count < cycles.round)
					goto hell;
				else
					phase = &Ppu::HBlankSp;
			}
			else
			{
				if (oam.loaded != oam.evaluated && io.enabled)
				{
					// extended +9 sprites

					do
					{
						LoadSprite();
					}
					while (oam.loaded != oam.evaluated);
				}

				cycles.count += cycles.four;
				NES_PPU_NEXT_PHASE( HBlankBg );
			}
		}

		void Ppu::HBlankBg()
		{
			NST_ASSERT( stage == 0 );

			bgHook.Execute();

			cycles.count += cycles.one;
			tiles.index = 0;

			if (io.enabled)
				io.address = scroll.address;

			NES_PPU_NEXT_PHASE( HBlankBg1 );
		}

		void Ppu::HBlankBg0()
		{
			LoadTiles();

			cycles.count += cycles.one;

			if (io.enabled)
				io.address = scroll.address;

			NES_PPU_NEXT_PHASE( HBlankBg1 );
		}

		void Ppu::HBlankBg1()
		{
			cycles.count += cycles.one;

			if (io.enabled)
				io.pattern = FetchName();

			NES_PPU_NEXT_PHASE( HBlankBg2 );
		}

		void Ppu::HBlankBg2()
		{
			cycles.count += cycles.one;

			if (io.enabled)
				io.address = scroll.address;

			NES_PPU_NEXT_PHASE( HBlankBg3 );
		}

		void Ppu::HBlankBg3()
		{
			cycles.count += cycles.one;

			if (io.enabled)
			{
				tiles.attribute = FetchAttribute();
				scroll.ClockX();
			}

			NES_PPU_NEXT_PHASE( HBlankBg4 );
		}

		void Ppu::HBlankBg4()
		{
			if (io.enabled)
			{
				io.address = io.pattern | 0x0;

				if (io.a12.InUse() && scroll.pattern)
					io.a12.Toggle( cycles.count );
			}

			cycles.count += cycles.one;
			NES_PPU_NEXT_PHASE( HBlankBg5 );
		}

		void Ppu::HBlankBg5()
		{
			cycles.count += cycles.one;

			if (io.enabled)
				tiles.pattern[0] = chr.FetchPattern( io.address );

			NES_PPU_NEXT_PHASE( HBlankBg6 );
		}

		void Ppu::HBlankBg6()
		{
			cycles.count += cycles.one;

			if (io.enabled)
				io.address = io.pattern | 0x8;

			NES_PPU_NEXT_PHASE( HBlankBg7 );
		}

		void Ppu::HBlankBg7()
		{
			if (io.enabled)
				tiles.pattern[1] = chr.FetchPattern( io.address );

			NST_ASSERT( stage < 2 && scanline < 240 );

			if (stage ^= 1)
			{
				cycles.count += cycles.one;
				NES_PPU_NEXT_PHASE( HBlankBg0 );
			}
			else if (scanline++ != 239)
			{
				cycles.count += cycles.six;
				tiles.mask = tiles.show[1];
				oam.mask = oam.show[1];

				if (scanline != 0)
				{
					NES_PPU_NEXT_PHASE( HActive0 );
				}
				else
				{
					output.index = 0;

					if (regs.ctrl1 & regs.frame)
					{
						NST_ASSERT( cycles.one == MC_DIV_NTSC );

						output.burstPhase = (output.burstPhase + 2) % 3;
						cycles.count -= cycles.one;
						cpu.SetupFrame( MC_DIV_NTSC * dword(CC_FRAME_1_NTSC) );
					}
					else if (cycles.one == MC_DIV_NTSC)
					{
						output.burstPhase = (output.burstPhase + 1) % 3;
					}

					NES_PPU_NEXT_PHASE( HActive0 );
				}
			}
			else
			{
				output.index = ~0U;
				cycles.count = cpu.GetMasterClockFrameCycles() - cycles.one;
				NES_PPU_NEXT_PHASE( VBlankIn );
			}
		}

		void Ppu::VBlankIn()
		{
			cycles.count += cycles.one;
			regs.status |= Regs::STATUS_VBLANKING;
			NES_PPU_NEXT_PHASE( VBlank );
		}

		void Ppu::VBlank()
		{
			NST_ASSERT( scanline != SCANLINE_VBLANK && regs.oam < Oam::SIZE );
			NST_VERIFY_MSG( regs.status & Regs::STATUS_VBLANKING, "Ppu $2002/VBlank conflict!" );

			regs.status = (regs.status & 0xFF) | (regs.status >> 1 & Regs::STATUS_VBLANK);
			cycles.count += cycles.one * 2;
			regs.oam = 0x00;
			scanline = SCANLINE_VBLANK;
			oam.visible = oam.output;

			if (cycles.spriteOverflow != Cpu::CYCLE_MAX)
			{
				cycles.spriteOverflow = Cpu::CYCLE_MAX;
				regs.status |= Regs::STATUS_SP_OVERFLOW;
			}

			NES_PPU_NEXT_PHASE( VBlankOut );
		}

		void Ppu::VBlankOut()
		{
			cycles.count = Cpu::CYCLE_MAX;
			phase = &Ppu::HDummy;

			if (regs.ctrl0 & regs.status & Regs::CTRL0_NMI)
				cpu.DoNMI( cpu.GetMasterClockFrameCycles() );
		}

		void Ppu::HDummy()
		{
			NST_ASSERT( scanline == SCANLINE_VBLANK );

			cycles.count += cycles.four;
			regs.status = 0;
			scanline = SCANLINE_HDUMMY;

			NES_PPU_NEXT_PHASE( HDummyBg );
		}

		void Ppu::HDummyBg()
		{
			NST_ASSERT( scanline == SCANLINE_HDUMMY );

		hell:

			if (io.a12.InUse() && scroll.pattern && io.enabled)
				io.a12.Toggle( cycles.count );

			cycles.count += cycles.eight;
			stage = (stage + 1) & 31;

			if (stage)
			{
				if (cycles.count < cycles.round)
					goto hell;
				else
					phase = &Ppu::HDummyBg;
			}
			else
			{
				NES_PPU_NEXT_PHASE( HDummySp );
			}
		}

		void Ppu::HDummySp()
		{
			NST_ASSERT( scanline == SCANLINE_HDUMMY );

		hell:

			if (io.a12.InUse() && regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16) && io.enabled)
				io.a12.Toggle( cycles.count );

			stage = (stage + 1) & 7;

			if (stage)
			{
				if (stage != 6)
				{
					cycles.count += cycles.eight;

					if (cycles.count < cycles.round)
						goto hell;
					else
						phase = &Ppu::HDummySp;
				}
				else
				{
					cycles.count += cycles.four;
					NES_PPU_NEXT_PHASE( HDummyScroll );
				}
			}
			else
			{
				cycles.count += cycles.four;
				NES_PPU_NEXT_PHASE( HBlankBg );
			}
		}

		void Ppu::HDummyScroll()
		{
			NST_ASSERT( scanline == SCANLINE_HDUMMY );

			cycles.count += cycles.four;

			if (io.enabled)
				scroll.address = scroll.latch;

			NES_PPU_NEXT_PHASE( HDummySp );
		}

		void Ppu::WarmUp()
		{
			NST_ASSERT( stage && phase == &Ppu::WarmUp );

			cycles.count = Cpu::CYCLE_MAX;

			if (!--stage)
			{
				regs.status |= Regs::STATUS_VBLANK;
				regs.oam = 0x00;
				phase = &Ppu::HDummy;
			}
		}
	}
}
