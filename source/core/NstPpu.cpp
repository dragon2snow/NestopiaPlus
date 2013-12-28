////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#include <cstring>
#include "NstLog.hpp"
#include "NstState.hpp"
#include "NstCpu.hpp"
#include "NstPpu.hpp"
						 
namespace Nes
{
	namespace Core
	{
		static const u8 reverseLut[256] =
		{
			0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,
			0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
			0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,
			0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
			0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,
			0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
			0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,
			0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
			0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,
			0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
			0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,
			0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
			0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,
			0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
			0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,
			0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
			0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,
			0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
			0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,
			0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
			0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,
			0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
			0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,
			0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
			0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,
			0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
			0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,
			0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
			0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,
			0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
			0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,
			0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
		};

		static const union { u8 pixels[4]; u32 block; } chrLut[2][16] =
		{	
			{
				{0x00,0x00,0x00,0x00},
				{0x01,0x00,0x00,0x00},
				{0x00,0x01,0x00,0x00},
				{0x01,0x01,0x00,0x00},
				{0x00,0x00,0x01,0x00},
				{0x01,0x00,0x01,0x00},
				{0x00,0x01,0x01,0x00},
				{0x01,0x01,0x01,0x00},
				{0x00,0x00,0x00,0x01},
				{0x01,0x00,0x00,0x01},
				{0x00,0x01,0x00,0x01},
				{0x01,0x01,0x00,0x01},
				{0x00,0x00,0x01,0x01},
				{0x01,0x00,0x01,0x01},
				{0x00,0x01,0x01,0x01},
				{0x01,0x01,0x01,0x01}
			},
			{
				{0x00,0x00,0x00,0x00},
				{0x02,0x00,0x00,0x00},
				{0x00,0x02,0x00,0x00},
				{0x02,0x02,0x00,0x00},
				{0x00,0x00,0x02,0x00},
				{0x02,0x00,0x02,0x00},
				{0x00,0x02,0x02,0x00},
				{0x02,0x02,0x02,0x00},
				{0x00,0x00,0x00,0x02},
				{0x02,0x00,0x00,0x02},
				{0x00,0x02,0x00,0x02},
				{0x02,0x02,0x00,0x02},
				{0x00,0x00,0x02,0x02},
				{0x02,0x00,0x02,0x02},
				{0x00,0x02,0x02,0x02},
				{0x02,0x02,0x02,0x02}
			}
		};

		static const union { u8 pixels[4]; u32 block; } chrAttLut[2][64] =
		{	
			{
				{0x00,0x00,0x00,0x00},
				{0x01,0x00,0x00,0x00},
				{0x00,0x01,0x00,0x00},
				{0x01,0x01,0x00,0x00},
				{0x00,0x00,0x01,0x00},
				{0x01,0x00,0x01,0x00},
				{0x00,0x01,0x01,0x00},
				{0x01,0x01,0x01,0x00},
				{0x00,0x00,0x00,0x01},
				{0x01,0x00,0x00,0x01},
				{0x00,0x01,0x00,0x01},
				{0x01,0x01,0x00,0x01},
				{0x00,0x00,0x01,0x01},
				{0x01,0x00,0x01,0x01},
				{0x00,0x01,0x01,0x01},
				{0x01,0x01,0x01,0x01},
				{0x00,0x00,0x00,0x00},
				{0x05,0x00,0x00,0x00},
				{0x00,0x05,0x00,0x00},
				{0x05,0x05,0x00,0x00},
				{0x00,0x00,0x05,0x00},
				{0x05,0x00,0x05,0x00},
				{0x00,0x05,0x05,0x00},
				{0x05,0x05,0x05,0x00},
				{0x00,0x00,0x00,0x05},
				{0x05,0x00,0x00,0x05},
				{0x00,0x05,0x00,0x05},
				{0x05,0x05,0x00,0x05},
				{0x00,0x00,0x05,0x05},
				{0x05,0x00,0x05,0x05},
				{0x00,0x05,0x05,0x05},
				{0x05,0x05,0x05,0x05},
				{0x00,0x00,0x00,0x00},
				{0x09,0x00,0x00,0x00},
				{0x00,0x09,0x00,0x00},
				{0x09,0x09,0x00,0x00},
				{0x00,0x00,0x09,0x00},
				{0x09,0x00,0x09,0x00},
				{0x00,0x09,0x09,0x00},
				{0x09,0x09,0x09,0x00},
				{0x00,0x00,0x00,0x09},
				{0x09,0x00,0x00,0x09},
				{0x00,0x09,0x00,0x09},
				{0x09,0x09,0x00,0x09},
				{0x00,0x00,0x09,0x09},
				{0x09,0x00,0x09,0x09},
				{0x00,0x09,0x09,0x09},
				{0x09,0x09,0x09,0x09},
				{0x00,0x00,0x00,0x00},
				{0x0D,0x00,0x00,0x00},
				{0x00,0x0D,0x00,0x00},
				{0x0D,0x0D,0x00,0x00},
				{0x00,0x00,0x0D,0x00},
				{0x0D,0x00,0x0D,0x00},
				{0x00,0x0D,0x0D,0x00},
				{0x0D,0x0D,0x0D,0x00},
				{0x00,0x00,0x00,0x0D},
				{0x0D,0x00,0x00,0x0D},
				{0x00,0x0D,0x00,0x0D},
				{0x0D,0x0D,0x00,0x0D},
				{0x00,0x00,0x0D,0x0D},
				{0x0D,0x00,0x0D,0x0D},
				{0x00,0x0D,0x0D,0x0D},
				{0x0D,0x0D,0x0D,0x0D}
			},
			{
				{0x00,0x00,0x00,0x00},
				{0x02,0x00,0x00,0x00},
				{0x00,0x02,0x00,0x00},
				{0x02,0x02,0x00,0x00},
				{0x00,0x00,0x02,0x00},
				{0x02,0x00,0x02,0x00},
				{0x00,0x02,0x02,0x00},
				{0x02,0x02,0x02,0x00},
				{0x00,0x00,0x00,0x02},
				{0x02,0x00,0x00,0x02},
				{0x00,0x02,0x00,0x02},
				{0x02,0x02,0x00,0x02},
				{0x00,0x00,0x02,0x02},
				{0x02,0x00,0x02,0x02},
				{0x00,0x02,0x02,0x02},
				{0x02,0x02,0x02,0x02},
				{0x00,0x00,0x00,0x00},
				{0x06,0x00,0x00,0x00},
				{0x00,0x06,0x00,0x00},
				{0x06,0x06,0x00,0x00},
				{0x00,0x00,0x06,0x00},
				{0x06,0x00,0x06,0x00},
				{0x00,0x06,0x06,0x00},
				{0x06,0x06,0x06,0x00},
				{0x00,0x00,0x00,0x06},
				{0x06,0x00,0x00,0x06},
				{0x00,0x06,0x00,0x06},
				{0x06,0x06,0x00,0x06},
				{0x00,0x00,0x06,0x06},
				{0x06,0x00,0x06,0x06},
				{0x00,0x06,0x06,0x06},
				{0x06,0x06,0x06,0x06},
				{0x00,0x00,0x00,0x00},
				{0x0A,0x00,0x00,0x00},
				{0x00,0x0A,0x00,0x00},
				{0x0A,0x0A,0x00,0x00},
				{0x00,0x00,0x0A,0x00},
				{0x0A,0x00,0x0A,0x00},
				{0x00,0x0A,0x0A,0x00},
				{0x0A,0x0A,0x0A,0x00},
				{0x00,0x00,0x00,0x0A},
				{0x0A,0x00,0x00,0x0A},
				{0x00,0x0A,0x00,0x0A},
				{0x0A,0x0A,0x00,0x0A},
				{0x00,0x00,0x0A,0x0A},
				{0x0A,0x00,0x0A,0x0A},
				{0x00,0x0A,0x0A,0x0A},
				{0x0A,0x0A,0x0A,0x0A},
				{0x00,0x00,0x00,0x00},
				{0x0E,0x00,0x00,0x00},
				{0x00,0x0E,0x00,0x00},
				{0x0E,0x0E,0x00,0x00},
				{0x00,0x00,0x0E,0x00},
				{0x0E,0x00,0x0E,0x00},
				{0x00,0x0E,0x0E,0x00},
				{0x0E,0x0E,0x0E,0x00},
				{0x00,0x00,0x00,0x0E},
				{0x0E,0x00,0x00,0x0E},
				{0x00,0x0E,0x00,0x0E},
				{0x0E,0x0E,0x00,0x0E},
				{0x00,0x00,0x0E,0x0E},
				{0x0E,0x00,0x0E,0x0E},
				{0x00,0x0E,0x0E,0x0E},
				{0x0E,0x0E,0x0E,0x0E}
			}
		};

		dword Ppu::logged;
		u16 Ppu::Output::dummy[4];

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
		
        #ifdef NST_TAILCALL_OPTIMIZE
  
          #define NST_PPU_NEXT_PHASE(function_)	\
												\
		  if (cycles.count < cycles.round)		\
			  function_();						\
		  else									\
			  phase = &Ppu::function_

        #else

          #define NST_PPU_NEXT_PHASE(function_) phase = &Ppu::function_

        #endif

		Ppu::Output::Output(Screen s)
		: 
		emphasisMask (Regs::CTRL1_BG_COLOR),
		screen       (&s) 
		{
		}
	
		void Ppu::Output::ClearScreen()
		{
			NST_ASSERT( screen );
			std::memset( screen, 0, sizeof(u16) * SCREEN );
		}
	
        #ifdef _MSC_VER
        #pragma warning( push )
        #pragma warning( disable : 4355 )
        #endif

		Ppu::Ppu(Cpu& c,Screen screen)
		: cpu(c), output(screen), bgHook(this,&Ppu::Hook_Null), spHook(this,&Ppu::Hook_Null)
		{
			oam.limit = oam.buffer + Oam::STD_LINE_SPRITES;
			SetMode( cpu.GetMode() );
		}

        #ifdef _MSC_VER
        #pragma warning( pop )
        #endif

		Ppu::~Ppu()
		{
		}

		void Ppu::Reset(const bool hard)
		{
			logged = 0;
	
			for (uint i=0x2000U; i < 0x4000U; i += 0x8)
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

				static const u8 powerUpPalette[] =
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
	
			if (chrMem.Source().Empty())
			{
				chrMem.Source().Set( nameTable.ram, NameTable::SIZE, true, false );
				chrMem.SwapBanks<SIZE_2K,0x0000>(0,0,0,0);
			}
	
			if (nmtMem.Source().Empty())
			{
				nmtMem.Source().Set( nameTable.ram, NameTable::SIZE, true, true );
				nmtMem.SwapBanks<SIZE_2K,0x0000>(0,0);
			}
	
			chrMem.ResetAccessors();
			nmtMem.ResetAccessors();
	
			cycles.count = NES_CYCLE_MAX;
			cycles.spriteOverflow = NES_CYCLE_MAX;
	
			io.address = 0;
			io.pattern = 0;
			
			scanline = SCANLINE_VBLANK;
	
			tiles.pattern[0] = 0;
			tiles.pattern[1] = 0;
			tiles.attribute = 0;
			tiles.pad = 0;
	
			oam.address = 0;
			oam.visible = oam.output;
			oam.evaluated = oam.buffer;
	
			UpdateStates();
	
			output.index = 0;
			output.ClearScreen();

			RemoveBgHook();
			RemoveSpHook();
		}
	
		void Ppu::UpdateStates()
		{
			io.enabled = regs.ctrl1 & (Regs::CTRL1_BG_ENABLED|Regs::CTRL1_SP_ENABLED);
			scroll.increase = (regs.ctrl0 & Regs::CTRL0_INC32) ? 32 : 1;
			scroll.pattern = (regs.ctrl0 & Regs::CTRL0_BG_OFFSET) << 8;
			tiles.show = (regs.ctrl1 & Regs::CTRL1_BG_ENABLED) ? ~0U : 0U;
			oam.show = (regs.ctrl1 & Regs::CTRL1_SP_ENABLED) ? ~0U : 0U;
			output.emphasis = (regs.ctrl1 & output.emphasisMask) << 1;
			output.coloring = (regs.ctrl1 & Regs::CTRL1_MONOCHROME) ? Palette::MONO : Palette::COLOR;
		}
	
		void Ppu::SaveState(State::Saver& state) const
		{
			{
				const u8 data[11] =
				{
					regs.ctrl0,
					regs.ctrl1,
					regs.status,
					scroll.address & 0xFF,
					scroll.address >> 8,
					scroll.latch & 0xFF,
					scroll.latch >> 8,
					scroll.xFine | (scroll.toggle << 3),
					oam.address,
					io.buffer,
					io.latch
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}
	
          	state.Begin('P','A','L','\0').Compress( palette.ram   ).End();
			state.Begin('O','A','M','\0').Compress( oam.ram       ).End();
			state.Begin('N','M','T','\0').Compress( nameTable.ram ).End();

			if (cpu.GetMode() == MODE_NTSC)
				state.Begin('F','R','M','\0').Write8( (regs.frame & Regs::FRAME_ODD) == 0 ).End();
		}
	
		void Ppu::LoadState(State::Loader& state)
		{
			output.burstPhase = 0;

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'): 
					{
						const State::Loader::Data<11> data( state );

						regs.ctrl0     = data[0];
						regs.ctrl1     = data[1];
						regs.status    = data[2] & Regs::STATUS_BITS;
						scroll.address = data[3] | ((data[4] & 0x7F) << 8);
						scroll.latch   = data[5] | ((data[6] & 0x7F) << 8);
						scroll.xFine   = data[7] & 0x7;
						scroll.toggle  = (data[7] & 0x8) >> 3;
						oam.address    = data[8];
						io.buffer      = data[9];
						io.latch       = data[10];

						phase = &Ppu::HDummy;
						stage = 0;
						regs.status |= Regs::STATUS_VBLANK;

						UpdateStates();
						break;
					}
	
					case NES_STATE_CHUNK_ID('P','A','L','\0'):
	
						state.Uncompress( palette.ram );
						break;
	
					case NES_STATE_CHUNK_ID('O','A','M','\0'): 
	
						state.Uncompress( oam.ram );
						break;
	
					case NES_STATE_CHUNK_ID('N','M','T','\0'): 
	
						state.Uncompress( nameTable.ram );
						break;

					case NES_STATE_CHUNK_ID('F','R','M','\0'):
					
						if (cpu.GetMode() == MODE_NTSC)
							regs.frame = (state.Read8() & 0x1) ? 0 : Regs::FRAME_ODD;

						break;
				}
	
				state.End();
			}
		}
	  
		void Ppu::EnableCpuSynchronization()
		{
			cpu.AddHook( Hook(this,&Ppu::Hook_Synchronize) );
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
	
		void Ppu::LogMsg(cstring const msg,const uint length,const uint which)
		{
			NST_ASSERT( msg );
	
			if (!(logged & which))
			{
				logged |= which;
				Log::Flush( msg, length );
			}
		}
	
		template<size_t N>
		inline void Ppu::LogMsg(const char (&c)[N],const uint e)
		{
			LogMsg( c, N-1, e );
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

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		void Ppu::BeginFrame(const bool render)
		{
			NST_ASSERT
			( 
     			scanline == SCANLINE_VBLANK &&
     			(phase == &Ppu::HDummy || phase == &Ppu::WarmUp) &&
		     	(cpu.GetMode() == MODE_NTSC) == (cycles.one == MC_DIV_NTSC)				
			);

			if (render)
			{
				NST_ASSERT( output.screen );
				output.pixels = *output.screen;
				output.next = sizeof(u16);
			}
			else
			{
				output.pixels = Output::dummy;
				output.next = 0;
			}

			Cycle frame;

			if (cpu.GetMode() == MODE_NTSC)
			{
				regs.frame ^= Regs::FRAME_ODD;
				cycles.count = MC_DIV_NTSC * CC_VINT_NTSC;

				if (phase != &Ppu::WarmUp || stage < WARM_UP_FRAMES)
					frame = MC_DIV_NTSC * CC_FRAME_0_NTSC;
				else
					frame = MC_DIV_NTSC * (CC_FRAME_0_NTSC - CC_VINT_NTSC);
			}
			else
			{
				NST_ASSERT( regs.frame == 0 && output.burstPhase == 0 );

				cycles.count = MC_DIV_PAL * CC_VINT_PAL;
				frame = MC_DIV_PAL * CC_FRAME_PAL;
			}

			cpu.SetupFrame( frame );
		}
	
		NES_HOOK(Ppu,Null)
		{
		}

		NES_HOOK(Ppu,Synchronize)
		{
			const Cycle elapsed = cpu.GetMasterClockCycles();
	
			if (cycles.count < elapsed)
			{
				cycles.round = elapsed;

            #ifdef NST_TAILCALL_OPTIMIZE
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
			if (cycles.count != NES_CYCLE_MAX)
			{
				cycles.round = NES_CYCLE_MAX;

            #ifdef NST_TAILCALL_OPTIMIZE
				(*this.*phase)();
            #else
				do 
				{
					(*this.*phase)();
				}
				while (cycles.count != NES_CYCLE_MAX);
            #endif
			}
		}
	
		void Ppu::Update()
		{
			const Cycle elapsed = cpu.GetMasterClockCycles();
	
			if (cycles.count < elapsed)
			{
				cycles.round = elapsed;

            #ifdef NST_TAILCALL_OPTIMIZE
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
	
		void Ppu::UpdateLatency()
		{
			const Cycle elapsed = cpu.GetMasterClockCycles() + cycles.one;

			if (cycles.count < elapsed)
			{
				cycles.round = elapsed;

            #ifdef NST_TAILCALL_OPTIMIZE
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
				NMT_ONE        == 4	&&
				NMT_CONTROLLED == 5
			);
	
			Update();
	
			static const uchar banks[6][4] =
			{
				{0,0,1,1}, // horizontal
				{0,1,0,1}, // vertical
				{0,1,2,3}, // four-screen
				{0,0,0,0}, // banks #1
				{1,1,1,1}, // banks #2
				{0,1,0,1}  // controlled, default to vertical 
			};
	
			nmtMem.SwapBanks<SIZE_1K,0x0000U>
			( 
		    	banks[type][0],
				banks[type][1],
				banks[type][2],
				banks[type][3]
			);
		}
	
		void Ppu::SetMirroring(const uchar (&banks)[4])
		{
			Update();
	
			NST_ASSERT( banks[0] < 4 && banks[1] < 4 && banks[2] < 4 && banks[3] < 4 );
	
			nmtMem.SwapBanks<SIZE_1K,0x0000U>
			( 
		       	banks[0], 
				banks[1], 
				banks[2], 
				banks[3] 
			);
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
	
		inline uint Ppu::ChrMem::FetchPattern(uint address) const
		{
			address &= 0x1FFF;
			return accessors[address >> 12].Fetch( address );
		}
	
		inline uint Ppu::NmtMem::FetchName(const uint address) const
		{
			const uint offset = address & (Scroll::Y_TILE|Scroll::X_TILE);
   	      	return accessors[(address & Scroll::NAME) >> 10][(offset + 0x40) >> 10].Fetch( offset );
		}
	
		inline uint Ppu::NmtMem::FetchAttribute(const uint address) const
		{
  	  		return accessors[(address & Scroll::NAME) >> 10][1].Fetch
  	  		( 
			    0x3C0 |
          		((address & (Scroll::X_TILE ^ b00000011)) >> 2) |
          		((address & (Scroll::Y_TILE ^ b01100000)) >> 4)
			);
		}
	
		inline uint Ppu::FetchName() const
		{
			return scroll.pattern | (nmtMem.FetchName( io.address ) << 4) | (scroll.address >> 12);
		}
	
		inline uint Ppu::FetchAttribute() const
		{
			return (nmtMem.FetchAttribute( io.address ) >> ((scroll.address & 0x02) | ((scroll.address & 0x40) >> 4))) & 0x3;
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
	
		NES_POKE(Ppu,2000)
		{
			Update();

			io.latch = data;
	
			scroll.latch &= (Scroll::NAME ^ 0x7FFFU);
			scroll.latch |= (data & Regs::CTRL0_NAME_OFFSET) << 10;
			scroll.increase = (data & Regs::CTRL0_INC32) ? 32 : 1;
			scroll.pattern = (data & Regs::CTRL0_BG_OFFSET) << 8;
	
			register const uint old = regs.ctrl0;
			regs.ctrl0 = data;
	
			if ((data & regs.status & Regs::CTRL0_NMI) > old)
				cpu.DoNMI();
		}
	
		NES_POKE(Ppu,2001)
		{
			UpdateLatency();
	
			regs.ctrl1 = io.latch = data;
			io.enabled = data & (Regs::CTRL1_BG_ENABLED|Regs::CTRL1_SP_ENABLED);
			tiles.show = (data & Regs::CTRL1_BG_ENABLED) ? ~0U : 0U;
			oam.show = (data & Regs::CTRL1_SP_ENABLED) ? ~0U : 0U;
			
			output.emphasis = (data & output.emphasisMask) << 1;
			output.coloring = (data & Regs::CTRL1_MONOCHROME) ? Palette::MONO : Palette::COLOR;		
		}
	
		NES_PEEK(Ppu,2002)
		{
			Update();

			register uint status = regs.status & 0xFF;
			regs.status &= (Regs::STATUS_VBLANK ^ 0xFF);

			scroll.toggle = 0;

			if (cycles.spriteOverflow < cpu.GetMasterClockCycles())
				status |= Regs::STATUS_SP_OVERFLOW;

			io.latch = (io.latch & Regs::STATUS_LATCH) | status;

			return io.latch;
		}
	
		NES_POKE(Ppu,2003)
		{
			UpdateLatency();
	
			oam.address = io.latch = data;
		}
	
		NES_POKE(Ppu,2004)
		{
			UpdateLatency();
	
			NST_ASSERT( oam.address < Oam::SIZE );
			NST_VERIFY( IsDead() );
	
			io.latch = data = IsDead() ? data : Oam::GARBAGE;
			u8& value = oam.ram[oam.address];
			oam.address = (oam.address + 1) & 0xFF;
			value = data;
		}
	
		NES_PEEK(Ppu,2004)
		{
			Update();
	
			NST_ASSERT( oam.address < Oam::SIZE );
	
			uint data;
	
			if (IsDead())
			{
				data = oam.ram[oam.address];
			}
			else
			{
				data = cpu.GetMasterClockCycles() / cycles.one % 341;
	
				if (data < 64)
				{
					data = Oam::GARBAGE;
				}
				else if (data < 192)
				{
					data = oam.ram[((data - 64) << 1) & 0xFC];
				}
				else if (data < 256)
				{
					data = oam.ram[(data & 1) ? 0xFC : ((data - 192) << 1) & 0xFC];
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
	
			return io.latch = data;
		}
	
		NES_POKE(Ppu,2005)
		{
			UpdateLatency();
	
			io.latch = data;
	
			if (scroll.toggle ^= 1)
			{
				scroll.latch &= (Scroll::X_TILE ^ 0x7FFFU);
				scroll.latch |= data >> 3;
				scroll.xFine = data & b111;
			}
			else
			{
				scroll.latch &= ((Scroll::Y_TILE|Scroll::Y_FINE) ^ 0x7FFFU);
				scroll.latch |= ((data << 2) | (data << 12)) & (Scroll::Y_TILE|Scroll::Y_FINE);
			}
		}
	
		NES_POKE(Ppu,2006)
		{
			UpdateLatency();
	
			io.latch = data;
	
			if (scroll.toggle ^= 1)
			{
				scroll.latch &= (Scroll::HIGH ^ 0x7FFFU);
				scroll.latch |= (data & b00111111) << 8;
			}
			else
			{
				scroll.latch &= (Scroll::LOW ^ 0x7FFFU);
				scroll.latch |= data;

				UpdateScrollAddress( scroll.latch );
			}
		}
	
		NES_POKE(Ppu,2007)
		{
			UpdateLatency();
	
			NST_VERIFY( IsDead()  );

			io.latch = data;
			address = scroll.address;

			UpdateScrollAddress( (scroll.address + scroll.increase) & 0x7FFF );

			if ((address & 0x3F00) == 0x3F00)
			{
				address &= 0x1F;
				data &= Palette::COLOR;
				
				palette.ram[address] = data;
	
				if (!(address & 0x3))
					palette.ram[address ^ 0x10] = data;
			}
			else 
			{
				address &= 0x3FFF;
									 
				if (address >= 0x2000)
					nmtMem.Poke( address & 0xFFF, data );	
				else
					chrMem.Poke( address, data );
			}
		}
		
		NES_PEEK(Ppu,2007)
		{
			Update();

			NST_VERIFY( IsDead() );

			address = scroll.address & 0x3FFF;
			
			UpdateScrollAddress( (scroll.address + scroll.increase) & 0x7FFF );

			if ((address & 0x3F00) != 0x3F00)
				io.latch = io.buffer;
			else
				io.latch = palette.ram[address & 0x1F] & output.coloring;

			if (address >= 0x2000)
				io.buffer = nmtMem.FetchName( address );
			else
				io.buffer = chrMem.FetchPattern( address );

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
			UpdateLatency();
	
			NST_ASSERT( oam.address < Oam::SIZE );
			NST_VERIFY( IsDead() );
	
			if (IsDead())
			{
				data <<= 8;
	
				if (oam.address == 0x00 && data < 0x2000)
				{
					std::memcpy( oam.ram, cpu.GetSystemRam() + (data & (Cpu::RAM_SIZE-1)), Oam::SIZE );
					io.latch = oam.ram[0xFF];
				}
				else 
				{
					uint dst = oam.address;
					const uint end = oam.address;
					register uint tmp;
	
					do 
					{					
						oam.ram[dst] = tmp = cpu.Peek( data++ );
						dst = (dst + 1) & 0xFF;
					} 
					while (dst != end);
	
					io.latch = tmp;
				}
			}
			else
			{
				std::memset( oam.ram, Oam::GARBAGE, Oam::SIZE );
				io.latch = Oam::GARBAGE;
	
				LogMsg("Ppu: garbage DMA transfer!" NST_LINEBREAK,1UL << 0);
			}
	
			cpu.StealCycles( cpu.GetMasterClockCycle(1) * Oam::DMA_CYCLES );
		}
	
		NES_PEEK(Ppu,4014)
		{
			return 0x40;
		}
	
		inline void Ppu::Scroll::ClockX()
		{
			if ((address & X_TILE) != X_TILE)
				++address;
			else
				address ^= (X_TILE|NAME_LOW);
		}
	
		inline void Ppu::Scroll::ResetX()
		{
			address &= ((X_TILE|NAME_LOW) ^ 0x7FFFU);
			address |= latch & (X_TILE|NAME_LOW);
		}
	
		inline void Ppu::Scroll::ClockY()
		{
			if ((address & Y_FINE) != (7U << 12))
			{
				address += (1U << 12);
			}
			else switch (address & Y_TILE)
			{
				case (29U << 5): address ^= NAME_HIGH;
				case (31U << 5): address &= ((Y_FINE|Y_TILE) ^ 0x7FFFU); break;
				default:         address = (address & (Y_FINE ^ 0x7FFFU)) + (1U << 5); break;
			}
		}
	
		void Ppu::EvaluateSpritesAndRenderPixel()
		{
			NST_ASSERT( scanline >= 0 && scanline <= 239 && oam.address < Oam::SIZE );
	
			oam.evaluated = oam.buffer;
	
			if (io.enabled && scanline != 239)
			{
				const uint height = ((regs.ctrl0 & Regs::CTRL0_SP8X16) >> 2) + 8;
				const uint line = scanline;
	
				uint i = 0;
	
				for (;;)
				{
					const u8* const NST_RESTRICT obj = oam.ram + i + (i <= 4 ? (oam.address & Oam::OFFSET_TO_0_1) : 0);
					i += 4;
	
					uint y = line - obj[0];
	
					if (y >= height)
					{
						if (i == Oam::SIZE)
						{
							RenderPixel();
							return;
						}
					}
					else
					{
						if (obj[2] & Oam::Y_FLIP)
							y ^= 0xF;
	
						oam.evaluated->comparitor = y;
	
						oam.evaluated->attribute = 
						(
							(i == 4) |
							((obj[2] & Oam::COLOR) << 2) |
							(obj[2] & (Oam::BEHIND|Oam::X_FLIP))
						);
	
						oam.evaluated->tile = obj[1];
						oam.evaluated->x = obj[3];
						
						++oam.evaluated;
	
						if (i == Oam::SIZE)
						{
							RenderPixel();
							return;
						}
	
						if (oam.evaluated == oam.limit)
							break;
					}
				}
	
				if (cycles.spriteOverflow == NES_CYCLE_MAX)
				{
					do
					{
						if (line - oam.ram[i] >= height)
						{
							i += 5;
						}
						else
						{
							cycles.spriteOverflow = cycles.count + cycles.one * (i / 4 * 3 + 2);
							break;
						}
					}
					while (i < Oam::SIZE);
				}
			}
	
			RenderPixel();
		}
		
		void Ppu::LoadSprite()
		{
			NST_ASSERT( oam.loaded < oam.evaluated );
	
			uint address;
	
			if (regs.ctrl0 & Regs::CTRL0_SP8X16)
			{
				address = 
				(
					((oam.loaded->tile & (Oam::Buffer::TILE_LSB)) << 12) | 
					((oam.loaded->tile & (Oam::Buffer::TILE_LSB ^ 0xFF)) << 4) |
					((oam.loaded->comparitor & Oam::Buffer::RANGE_MSB) << 1)
				);
			}
			else
			{
				address = ((regs.ctrl0 & Regs::CTRL0_SP_OFFSET) << 9) | (oam.loaded->tile << 4);
			}
	
			address |= oam.loaded->comparitor & Oam::Buffer::XFINE;
	
			if (io.a12.InUse() && (address & 0x1000))
				io.a12.Toggle( cycles.count );
	
			uint pattern[2] =
			{
				chrMem.FetchPattern( address | 0x0 ),
				chrMem.FetchPattern( address | 0x8 )
			};
	
			if (pattern[0] | pattern[1])
			{
				if (!(oam.loaded->attribute & Oam::X_FLIP))
				{
					pattern[0] = reverseLut[pattern[0]];
					pattern[1] = reverseLut[pattern[1]];
				}
	
				const u32 block[] =
				{
					chrLut[0][pattern[0] & 0xF].block | 
					chrLut[1][pattern[1] & 0xF].block,
					chrLut[0][pattern[0] >>  4].block | 
					chrLut[1][pattern[1] >>  4].block
				};
	
				Oam::Output* const NST_RESTRICT output = oam.visible++;
	
				output->block[0] = block[0];
				output->block[1] = block[1];

				const uint attribute = oam.loaded->attribute;

				output->x       = oam.loaded->x;
				output->palette = Palette::SPRITE_OFFSET + (attribute & (uint(Oam::COLOR) << 2));
				output->zero    = (attribute & 0x1) ? b11 : b00;
				output->behind  = (attribute & Oam::BEHIND) ? b11 : b00;	
			}
	
			++oam.loaded;
		}

		void Ppu::Tiles::Load()
		{	
			const u32 tmp[] =
			{
				chrAttLut[0][(reverseLut[pattern[0]] & 0xF) | (attribute << 4)].block | 
				chrAttLut[1][(reverseLut[pattern[1]] & 0xF) | (attribute << 4)].block,
				chrAttLut[0][(reverseLut[pattern[0]] >>  4) | (attribute << 4)].block | 
				chrAttLut[1][(reverseLut[pattern[1]] >>  4) | (attribute << 4)].block
			};

			u32* const NST_RESTRICT dst = block + index;
			index ^= 2;

			dst[0] = tmp[0];
			dst[1] = tmp[1];
		}
	
		void Ppu::RenderPixel()
		{
			register uint pixel = 0;
	
			if (io.enabled)
			{
				pixel = tiles.pixels[(output.index + scroll.xFine) & 15] & tiles.show & tiles.clip;

				const Oam::Output* NST_RESTRICT sprite = oam.output;

				for (const Oam::Output* const end = oam.visible; sprite != end; ++sprite)
				{
					register uint x = (output.index & 0xFF) - sprite->x;

					if (x > 7)
						continue;

					x = sprite->pixels[x] & oam.show & oam.clip;
	
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
			u16* const NST_RESTRICT screen = output.pixels;
			output.pixels = reinterpret_cast<u16*>(reinterpret_cast<u8*>(output.pixels) + output.next);
			*screen = output.emphasis + (output.coloring & palette.ram[pixel]);
		}
	
		void Ppu::HActive0()
		{
			if (io.enabled)
				io.address = scroll.address;
	
			tiles.Load();
	
			if (stage != 8)
				RenderPixel();
			else
				EvaluateSpritesAndRenderPixel();
	
			cycles.count += cycles.one;	
			NST_PPU_NEXT_PHASE( HActive1 );
		}
	
		void Ppu::HActive1()
		{
			if (io.enabled)
				io.pattern = FetchName();
	
			RenderPixel();
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HActive2 );
		}
	
		void Ppu::HActive2()
		{
			if (io.enabled)
				io.address = scroll.address;
	
			RenderPixel();
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HActive3 );
		}
	
		void Ppu::HActive3()
		{
			if (io.enabled)
			{
				tiles.attribute = FetchAttribute();
				scroll.ClockX();
	
				if (stage == 31)
					scroll.ClockY();
			}
	
			RenderPixel();
	
			cycles.count += cycles.one;	
			NST_PPU_NEXT_PHASE( HActive4 );
		}
	
		void Ppu::HActive4()
		{
			if (io.enabled)
			{
				io.address = io.pattern | 0x0;
	
				if (io.a12.InUse() && (regs.ctrl0 & Regs::CTRL0_BG_OFFSET))
					io.a12.Toggle( cycles.count );
			}
	
			RenderPixel();
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HActive5 );
		}
	
		void Ppu::HActive5()
		{
			if (io.enabled)
				tiles.pattern[0] = chrMem.FetchPattern( io.address );
	
			RenderPixel();
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HActive6 );
		}
	
		void Ppu::HActive6()
		{
			if (io.enabled)
				io.address = io.pattern | 0x8;
	
			RenderPixel();
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HActive7 );
		}
	
		void Ppu::HActive7()
		{
			const uint noSpHitOn255 = regs.status;
	
			RenderPixel();
	
			oam.clip = ~0U;
			tiles.clip = ~0U;
	
			if (io.enabled)
				tiles.pattern[1] = chrMem.FetchPattern( io.address );
	
			cycles.count += cycles.one;
	
			NST_ASSERT( stage < 32 );
	
			stage = (stage + 1) & 31;
	
			if (stage)
			{
				NST_PPU_NEXT_PHASE( HActive0 );
			}
			else
			{
				cycles.count += cycles.one;
				regs.status = noSpHitOn255;
				
				NST_PPU_NEXT_PHASE( HBlank );
			}
		}
	
		void Ppu::HBlank()
		{
			NST_ASSERT( stage == 0 );

			spHook.Execute();

			oam.loaded = oam.buffer;
			oam.visible = oam.output;
	
			if (io.enabled)
				scroll.ResetX();
	
			cycles.count += cycles.four - cycles.one;
			NST_PPU_NEXT_PHASE( HBlankSp );
		}
	
		void Ppu::HBlankSp()
		{
        hell:
	
			if (io.enabled)
			{
				if (oam.loaded == oam.evaluated)
				{
					if (io.a12.InUse() && (regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16)))
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
				if (io.enabled)
				{
					// extended +9 sprites
	
					while (oam.loaded != oam.evaluated) 
						LoadSprite(); 
				}
	
				cycles.count += cycles.four;
				NST_PPU_NEXT_PHASE( HBlankBg );
			}
		}
	
		void Ppu::HBlankBg()
		{
			NST_ASSERT( stage == 0 );

			bgHook.Execute();

			tiles.index = 0;
	
			if (io.enabled)
				io.address = scroll.address;
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HBlankBg1 );
		}
	
		void Ppu::HBlankBg0()
		{
			if (io.enabled)
				io.address = scroll.address;
	
			tiles.Load();
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HBlankBg1 );
		}
	
		void Ppu::HBlankBg1()
		{
			if (io.enabled)
				io.pattern = FetchName();
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HBlankBg2 );
		}
	
		void Ppu::HBlankBg2()
		{
			if (io.enabled)
				io.address = scroll.address;
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HBlankBg3 );
		}
	
		void Ppu::HBlankBg3()
		{
			if (io.enabled)
			{
				tiles.attribute = FetchAttribute();
				scroll.ClockX();
			}
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HBlankBg4 );
		}
	
		void Ppu::HBlankBg4()
		{
			if (io.enabled)
			{
				io.address = io.pattern | 0x0;
	
				if (io.a12.InUse() && (regs.ctrl0 & Regs::CTRL0_BG_OFFSET))
					io.a12.Toggle( cycles.count );
			}
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HBlankBg5 );
		}
	
		void Ppu::HBlankBg5()
		{
			if (io.enabled)
				tiles.pattern[0] = chrMem.FetchPattern( io.address );
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HBlankBg6 );
		}
	
		void Ppu::HBlankBg6()
		{
			if (io.enabled)
				io.address = io.pattern | 0x8;
	
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( HBlankBg7 );
		}
	
		void Ppu::HBlankBg7()
		{
			NST_ASSERT( stage < 2 && scanline < 240 );
	
			if (io.enabled)
				tiles.pattern[1] = chrMem.FetchPattern( io.address );
	
			if (stage ^= 1)
			{
				cycles.count += cycles.one;
				NST_PPU_NEXT_PHASE( HBlankBg0 );
			}
			else if (++scanline != 240)
			{
				cycles.count += cycles.six;

				oam.clip = (regs.ctrl1 & Regs::CTRL1_SP_NO_CLIPPING) ? ~0U : 0U;			
				tiles.clip = (regs.ctrl1 & Regs::CTRL1_BG_NO_CLIPPING) ? ~0U : 0U;

				if (scanline != 0)
				{
					NST_PPU_NEXT_PHASE( HActive0 );
				}
				else
				{
					output.index = 0;

					if (regs.ctrl1 & regs.frame)
					{
						output.burstPhase = (output.burstPhase + 2) % 3;
						cycles.count -= cycles.one;			
						cpu.SetupFrame( MC_DIV_NTSC * CC_FRAME_1_NTSC );
					}
					else if (cpu.GetMode() == MODE_NTSC)
					{
						output.burstPhase = (output.burstPhase + 1) % 3;
					}
					
					NST_PPU_NEXT_PHASE( HActive0 );
				}
			}
			else
			{
				output.index = ~0U;
				cycles.count = cpu.GetMasterClockFrameCycles() - cycles.one;
				NST_PPU_NEXT_PHASE( VBlankIn );
			}
		}

		void Ppu::VBlankIn()
		{
			regs.status |= Regs::STATUS_VBLANKING;
			cycles.count += cycles.one;
			NST_PPU_NEXT_PHASE( VBlank );
		}

		void Ppu::VBlank()
		{
			NST_ASSERT( scanline != SCANLINE_VBLANK && oam.address < Oam::SIZE );
	
			NST_VERIFY_MSG( regs.status & Regs::STATUS_VBLANKING, "Ppu $2002/VBlank conflict!" );

			regs.status = (regs.status & 0xFF) | ((regs.status >> 1) & Regs::STATUS_VBLANK);
			scanline = SCANLINE_VBLANK;
			oam.address = 0x00;

			if (cycles.spriteOverflow != NES_CYCLE_MAX)
			{
				cycles.spriteOverflow = NES_CYCLE_MAX;
				regs.status |= Regs::STATUS_SP_OVERFLOW;
			}

			cycles.count += cycles.one * 2;
			NST_PPU_NEXT_PHASE( VBlankOut );
		}

		void Ppu::VBlankOut()
		{
			cycles.count = NES_CYCLE_MAX;
			phase = &Ppu::HDummy;

			if (regs.ctrl0 & regs.status & Regs::CTRL0_NMI)
				cpu.DoNMI( cpu.GetMasterClockFrameCycles() );
		}

		void Ppu::HDummy()
		{
			NST_ASSERT( scanline == SCANLINE_VBLANK );
	
			regs.status = 0;
			scanline = SCANLINE_HDUMMY;
			cycles.count += cycles.four;
	
			NST_PPU_NEXT_PHASE( HDummyBg );
		}
	
		void Ppu::HDummyBg()
		{
			NST_ASSERT( scanline == SCANLINE_HDUMMY );
	
        hell:
	
			if (io.a12.InUse() && (regs.ctrl0 & Regs::CTRL0_BG_OFFSET) && io.enabled)
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
				NST_PPU_NEXT_PHASE( HDummySp );
			}
		}
	
		void Ppu::HDummySp()
		{
			NST_ASSERT( scanline == SCANLINE_HDUMMY );
	
        hell:
	
			if (io.a12.InUse() && (regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16)) && io.enabled)
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
					NST_PPU_NEXT_PHASE( HDummyScroll );
				}
			}
			else
			{
				cycles.count += cycles.four;
				NST_PPU_NEXT_PHASE( HBlankBg );
			}		
		}
	
		void Ppu::HDummyScroll()
		{
			NST_ASSERT( scanline == SCANLINE_HDUMMY );
	
			if (io.enabled)
				scroll.address = scroll.latch;
	
			cycles.count += cycles.four;
			NST_PPU_NEXT_PHASE( HDummySp );
		}
	
		void Ppu::WarmUp()
		{
			NST_ASSERT( stage && phase == &Ppu::WarmUp );
	
			cycles.count = NES_CYCLE_MAX;
	
			if (!--stage)
			{
				regs.status |= Regs::STATUS_VBLANK;
				oam.address = 0x00;
				phase = &Ppu::HDummy;
			}
		}
	}
}
