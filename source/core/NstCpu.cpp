////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include "NstLog.hpp"
#include "NstSignedArithmetic.hpp"
#include "NstState.hpp"
#include "NstChip.hpp"
#include "NstHook.hpp"
#include "NstCpu.hpp"
#include "api/NstApiUser.hpp"

#ifdef _MSC_VER
#pragma warning( disable : 4355 )
#endif

namespace Nes
{
	namespace Core
	{
		dword Cpu::logged = 0;
	
		void (Cpu::* const Cpu::opcodes[NUM_OPCODES])() =
		{
			&Cpu::op0x00, &Cpu::op0x01, &Cpu::op0x02, &Cpu::op0x03, 
			&Cpu::op0x04, &Cpu::op0x05, &Cpu::op0x06, &Cpu::op0x07,
			&Cpu::op0x08, &Cpu::op0x09, &Cpu::op0x0A, &Cpu::op0x0B, 
			&Cpu::op0x0C, &Cpu::op0x0D, &Cpu::op0x0E, &Cpu::op0x0F,
			&Cpu::op0x10, &Cpu::op0x11, &Cpu::op0x12, &Cpu::op0x13, 
			&Cpu::op0x14, &Cpu::op0x15, &Cpu::op0x16, &Cpu::op0x17,
			&Cpu::op0x18, &Cpu::op0x19, &Cpu::op0x1A, &Cpu::op0x1B, 
			&Cpu::op0x1C, &Cpu::op0x1D, &Cpu::op0x1E, &Cpu::op0x1F,
			&Cpu::op0x20, &Cpu::op0x21, &Cpu::op0x22, &Cpu::op0x23, 
			&Cpu::op0x24, &Cpu::op0x25, &Cpu::op0x26, &Cpu::op0x27,
			&Cpu::op0x28, &Cpu::op0x29, &Cpu::op0x2A, &Cpu::op0x2B, 
			&Cpu::op0x2C, &Cpu::op0x2D, &Cpu::op0x2E, &Cpu::op0x2F,
			&Cpu::op0x30, &Cpu::op0x31, &Cpu::op0x32, &Cpu::op0x33, 
			&Cpu::op0x34, &Cpu::op0x35, &Cpu::op0x36, &Cpu::op0x37,
			&Cpu::op0x38, &Cpu::op0x39, &Cpu::op0x3A, &Cpu::op0x3B, 
			&Cpu::op0x3C, &Cpu::op0x3D, &Cpu::op0x3E, &Cpu::op0x3F,
			&Cpu::op0x40, &Cpu::op0x41, &Cpu::op0x42, &Cpu::op0x43, 
			&Cpu::op0x44, &Cpu::op0x45, &Cpu::op0x46, &Cpu::op0x47,
			&Cpu::op0x48, &Cpu::op0x49, &Cpu::op0x4A, &Cpu::op0x4B, 
			&Cpu::op0x4C, &Cpu::op0x4D, &Cpu::op0x4E, &Cpu::op0x4F,
			&Cpu::op0x50, &Cpu::op0x51, &Cpu::op0x52, &Cpu::op0x53, 
			&Cpu::op0x54, &Cpu::op0x55, &Cpu::op0x56, &Cpu::op0x57,
			&Cpu::op0x58, &Cpu::op0x59, &Cpu::op0x5A, &Cpu::op0x5B, 
			&Cpu::op0x5C, &Cpu::op0x5D, &Cpu::op0x5E, &Cpu::op0x5F,
			&Cpu::op0x60, &Cpu::op0x61, &Cpu::op0x62, &Cpu::op0x63, 
			&Cpu::op0x64, &Cpu::op0x65, &Cpu::op0x66, &Cpu::op0x67,
			&Cpu::op0x68, &Cpu::op0x69, &Cpu::op0x6A, &Cpu::op0x6B, 
			&Cpu::op0x6C, &Cpu::op0x6D, &Cpu::op0x6E, &Cpu::op0x6F,
			&Cpu::op0x70, &Cpu::op0x71, &Cpu::op0x72, &Cpu::op0x73, 
			&Cpu::op0x74, &Cpu::op0x75, &Cpu::op0x76, &Cpu::op0x77,
			&Cpu::op0x78, &Cpu::op0x79, &Cpu::op0x7A, &Cpu::op0x7B, 
			&Cpu::op0x7C, &Cpu::op0x7D, &Cpu::op0x7E, &Cpu::op0x7F,
			&Cpu::op0x80, &Cpu::op0x81, &Cpu::op0x82, &Cpu::op0x83, 
			&Cpu::op0x84, &Cpu::op0x85, &Cpu::op0x86, &Cpu::op0x87,
			&Cpu::op0x88, &Cpu::op0x89, &Cpu::op0x8A, &Cpu::op0x8B, 
			&Cpu::op0x8C, &Cpu::op0x8D, &Cpu::op0x8E, &Cpu::op0x8F,
			&Cpu::op0x90, &Cpu::op0x91, &Cpu::op0x92, &Cpu::op0x93, 
			&Cpu::op0x94, &Cpu::op0x95, &Cpu::op0x96, &Cpu::op0x97,
			&Cpu::op0x98, &Cpu::op0x99, &Cpu::op0x9A, &Cpu::op0x9B, 
			&Cpu::op0x9C, &Cpu::op0x9D, &Cpu::op0x9E, &Cpu::op0x9F,
			&Cpu::op0xA0, &Cpu::op0xA1, &Cpu::op0xA2, &Cpu::op0xA3, 
			&Cpu::op0xA4, &Cpu::op0xA5, &Cpu::op0xA6, &Cpu::op0xA7,
			&Cpu::op0xA8, &Cpu::op0xA9, &Cpu::op0xAA, &Cpu::op0xAB, 
			&Cpu::op0xAC, &Cpu::op0xAD, &Cpu::op0xAE, &Cpu::op0xAF,
			&Cpu::op0xB0, &Cpu::op0xB1, &Cpu::op0xB2, &Cpu::op0xB3, 
			&Cpu::op0xB4, &Cpu::op0xB5, &Cpu::op0xB6, &Cpu::op0xB7,
			&Cpu::op0xB8, &Cpu::op0xB9, &Cpu::op0xBA, &Cpu::op0xBB, 
			&Cpu::op0xBC, &Cpu::op0xBD, &Cpu::op0xBE, &Cpu::op0xBF,
			&Cpu::op0xC0, &Cpu::op0xC1, &Cpu::op0xC2, &Cpu::op0xC3, 
			&Cpu::op0xC4, &Cpu::op0xC5, &Cpu::op0xC6, &Cpu::op0xC7,
			&Cpu::op0xC8, &Cpu::op0xC9, &Cpu::op0xCA, &Cpu::op0xCB, 
			&Cpu::op0xCC, &Cpu::op0xCD, &Cpu::op0xCE, &Cpu::op0xCF,
			&Cpu::op0xD0, &Cpu::op0xD1, &Cpu::op0xD2, &Cpu::op0xD3, 
			&Cpu::op0xD4, &Cpu::op0xD5, &Cpu::op0xD6, &Cpu::op0xD7,
			&Cpu::op0xD8, &Cpu::op0xD9, &Cpu::op0xDA, &Cpu::op0xDB, 
			&Cpu::op0xDC, &Cpu::op0xDD, &Cpu::op0xDE, &Cpu::op0xDF,
			&Cpu::op0xE0, &Cpu::op0xE1, &Cpu::op0xE2, &Cpu::op0xE3, 
			&Cpu::op0xE4, &Cpu::op0xE5, &Cpu::op0xE6, &Cpu::op0xE7,
			&Cpu::op0xE8, &Cpu::op0xE9, &Cpu::op0xEA, &Cpu::op0xEB, 
			&Cpu::op0xEC, &Cpu::op0xED, &Cpu::op0xEE, &Cpu::op0xEF,
			&Cpu::op0xF0, &Cpu::op0xF1, &Cpu::op0xF2, &Cpu::op0xF3, 
			&Cpu::op0xF4, &Cpu::op0xF5, &Cpu::op0xF6, &Cpu::op0xF7,
			&Cpu::op0xF8, &Cpu::op0xF9, &Cpu::op0xFA, &Cpu::op0xFB, 
			&Cpu::op0xFC, &Cpu::op0xFD, &Cpu::op0xFE, &Cpu::op0xFF
		};

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("a", on)
        #endif
	
		inline uint Cpu::IoMap::Peek8(const uint address) const
		{
			NST_ASSERT( address < (NES_64K + OVERFLOW_SIZE) );
			return ports[address].Peek( address );
		}
	
		inline uint Cpu::IoMap::Peek16(const uint address) const
		{
			NST_ASSERT( (address + 1) < (NES_64K + OVERFLOW_SIZE) );
			return u8(ports[address].Peek( address )) | (u8(ports[address + 1].Peek( address + 1 )) << 8);
		}
	
		inline void Cpu::IoMap::Poke8(const uint address,const uint data) const
		{
			NST_ASSERT( address < (NES_64K + OVERFLOW_SIZE) );
			ports[address].Poke( address, data );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #pragma optimize("s", on)
        #endif
	
		template<typename T,typename U>
		Cpu::IoMap::IoMap(Cpu* cpu,T peek,U poke)
		: Io::Map<NES_64K>( cpu, peek, poke ) {}
	
		Cpu::Cpu()
		:
		map        (*new IoMap( this, &Cpu::Peek_Overflow, &Cpu::Poke_Overflow )),
		frameClock (0),
		mode       (MODE_NTSC),
		apu        (this)
		{
			Boot();
		}
	
		Cpu::Cycles::Cycles()
		: count(0)
		{
			for (uint i=0; i < 8; ++i) 
				clock[i] = (i+1) * MC_DIV_NTSC;
		}
	
		Cpu::~Cpu()
		{
			delete &map;
		}
	
		void Cpu::Boot()
		{
			map( 0x0000U, 0x1FFFU ).Set( &ram, &Cpu::Ram::Peek_Ram, &Cpu::Ram::Poke_Ram );
			map( 0x2000U, 0xFFFFU ).Set( this, &Cpu::Peek_Nop,      &Cpu::Poke_Nop      );
			map( 0xFFFCU          ).Set( this, &Cpu::Peek_Jam_1,    &Cpu::Poke_Nop      );
			map( 0xFFFDU          ).Set( this, &Cpu::Peek_Jam_2,    &Cpu::Poke_Nop      );

			hooks.Clear();
		}
	
		void Cpu::SetMode(const Mode m)
		{
			if (mode != m)
			{
				mode = m;
				ticks = (ticks / ulong(m == MODE_NTSC ? MC_DIV_NTSC : MC_DIV_PAL)) * ulong(m == MODE_NTSC ? MC_DIV_NTSC : MC_DIV_PAL);
				cycles.Update( m );
				apu.SetMode( m );
			}
		}
	
		void Cpu::AddHook(const Hook& hook)
		{
			for (uint i=0; i < hooks.Size(); ++i)
				if (hooks[i] == hook)
					return;

			hooks << hook;
		}
	
		void Cpu::Cycles::Update(const Mode mode)
		{
			const uint divider = (mode == MODE_NTSC ? MC_DIV_NTSC : MC_DIV_PAL);
			count = (count / divider) * divider;
	
			for (uint i=0; i < 8; ++i) 
				clock[i] = (i + 1) * divider;
		}
	
		void Cpu::Cycles::Reset()
		{
			count = clock[RESET_CYCLES-1];
			round = 0;
		}
	
		void Cpu::Interrupt::Reset()
		{
			nmiClock = NES_CYCLE_MAX;
			irqClock = NES_CYCLE_MAX;
			source = IRQ_FRAME|IRQ_EXT;
			low = 0;
		}
	
		void Cpu::Interrupt::Jam()
		{
			nmiClock = NES_CYCLE_MAX;
			irqClock = NES_CYCLE_MAX;
			low = 0;
		}
	
		void Cpu::Ram::Clear()
		{
			std::memset( mem, 0x00, sizeof(mem) );
		}
	
		void Cpu::Ram::Reset()
		{
			std::memset( mem + 0x000, 0xFF, 0x3F0 );
			std::memset( mem + 0x3F0, 0x00, 0x010 );
			std::memset( mem + 0x400, 0xFF, 0x1F0 );
			std::memset( mem + 0x5F0, 0x00, 0x010 );
			std::memset( mem + 0x600, 0xFF, 0x200 );
											  
			mem[0x08] = 0xF7;
			mem[0x09] = 0xEF;
			mem[0x0A] = 0xDF;
			mem[0x0F] = 0xBF;
		}
	
		void Cpu::Reset(const bool hard)
		{
			if (hard)
			{
				ram.Reset();
				a = x = y = 0x00;
				sp = 0xFD;
				flags.nz = 0 ^ 1;
				flags.d = flags.v = flags.c = 0;
			}
			else
			{
				sp = (sp - 3) & 0xFF;
			}
	
			cycles.Reset();
			interrupt.Reset();
	
			flags.i  = Flags::I;
			pc       = RESET_VECTOR;
			jammed   = false;
			ticks    = 0;
			logged   = 0;
	
			ResetLog();
	
			apu.Reset( hard );
	
			pc = map.Peek16( pc );
		}
	
		void Cpu::ResetLog()
		{
			Log() << "Cpu: reset" NST_LINEBREAK "Cpu: PC: "
				  << Log::Hex( (u16) pc ) << ", SP: "
				  << Log::Hex( (u8)  sp ) << NST_LINEBREAK "Cpu: A: "
				  << Log::Hex( (u8)   a ) << ", X: "
				  << Log::Hex( (u8)   x ) << ", Y: "
				  << Log::Hex( (u8)   y ) << NST_LINEBREAK "Cpu: C:"  
				  << (flags.c             ? '1' : '0') << ", Z:"
				  << ((flags.nz & 0xFF)   ? '0' : '1') << ", I:"
				  << (flags.i             ? '1' : '0') << ", D:"
				  << (flags.d             ? '1' : '0') << ", V:"      
				  << (flags.v             ? '1' : '0') << ", N:"
				  << ((flags.nz & 0x180U) ? "1" NST_LINEBREAK : "0"  NST_LINEBREAK);
		}
	
		void Cpu::SaveState(State::Saver& state) const
		{
			{
				const u8 data[7] =
				{
					pc & 0xFF,
					pc >> 8,
					sp,
					a,
					x,
					y,
					flags.Pack()
				};

				state.Begin('R','E','G','\0').Write( data ).End();
			}
	
			state.Begin('R','A','M','\0').Compress( ram.mem ).End();
	
			{
				const u8 data[5] =
				{
					((interrupt.nmiClock != NES_CYCLE_MAX) ? SAVE_INT_NMI   : 0) |
					((interrupt.low & IRQ_FRAME)           ? SAVE_INT_FRAME : 0) |
					((interrupt.low & IRQ_DMC)             ? SAVE_INT_DMC   : 0) |
					((interrupt.low & IRQ_EXT)             ? SAVE_INT_EXT   : 0) |
					(jammed                                ? SAVE_JAMMED    : 0) |
					(mode == MODE_PAL                      ? SAVE_PAL       : 0), 
					(cycles.count >>  0) & 0xFF,
					(cycles.count >>  8) & 0xFF,
					(cycles.count >> 16) & 0xFF,
					(cycles.count >> 24) 
				};

				state.Begin('F','R','M','\0').Write( data ).End();
			}
		}
	
		void Cpu::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'): 
					{
						const State::Loader::Data<7> data( state );

						pc = data[0] | (data[1] << 8);
						sp = data[2];
						a  = data[3];
						x  = data[4];
						y  = data[5];

						flags.Unpack( data[6] );
						break;
					}
	
					case NES_STATE_CHUNK_ID('R','A','M','\0'):
	
						state.Uncompress( ram.mem );
						break;

					case NES_STATE_CHUNK_ID('F','R','M','\0'): 
					{
						const State::Loader::Data<5> data( state );

						if (data[0] & SAVE_INT_NMI)
							interrupt.nmiClock = cycles.clock[0];
						else
							interrupt.nmiClock = NES_CYCLE_MAX;

						interrupt.low = 0;
						interrupt.irqClock = NES_CYCLE_MAX;

						if (data[0] & (SAVE_INT_FRAME|SAVE_INT_DMC|SAVE_INT_EXT))
						{
							interrupt.low = 
							(
       							((data[0] & SAVE_INT_FRAME) ? IRQ_FRAME : 0) |
								((data[0] & SAVE_INT_DMC)   ? IRQ_DMC   : 0) |
								((data[0] & SAVE_INT_EXT)   ? IRQ_EXT   : 0)
							);

							if (!flags.i)
								interrupt.irqClock = 0;
						}

						cycles.count = data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24);

						if (bool(mode == MODE_PAL) != bool(data[0] & SAVE_PAL))
							cycles.Update( mode );

						if (cycles.count >= frameClock)
							cycles.count = 0;

						jammed = data[0] & SAVE_JAMMED;

						if (jammed)
							interrupt.Jam();

						break;
					}
				}
	
				state.End();
			}
		}
	
		void Cpu::TryLogMsg(cstring const msg,const uint length,const uint which)
		{
			NST_DEBUG_MSG( msg );
	
			if (!(logged & which))
			{
				logged |= which;
				Log::Flush( msg, length );
			}
		}
	
		template<size_t N>
		inline void Cpu::LogMsg(const char (&c)[N],const uint e)
		{
			TryLogMsg( c, N-1, e );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #pragma optimize("w", on)
        #endif
	
		NES_PEEK(Cpu::Ram,Ram)
		{
			return mem[address & 0x7FF];
		}
	
		NES_POKE(Cpu::Ram,Ram)
		{
			mem[address & 0x7FF] = data;
		}
	
		NES_PEEK(Cpu,Nop)
		{ 
			return address >> 8; 
		}
	
		NES_POKE(Cpu,Nop)
		{ 
		}
	
		NES_PEEK(Cpu,Overflow)
		{
			pc &= 0xFFFFU;
			return ram.mem[address & 0x7FF];
		}
	
		NES_POKE(Cpu,Overflow)
		{
			pc &= 0xFFFFU;
			ram.mem[address & 0x7FF] = data;
		}
	
		NES_PEEK(Cpu,Jam_1)
		{
			pc = (pc - 1U) & 0xFFFFU;
			return 0xFC;
		}
	
		NES_PEEK(Cpu,Jam_2)
		{
			return 0xFF;
		}
	
		uint Cpu::Flags::Pack() const
		{
			NST_ASSERT( (i == 0 || i == I) && (c == 0 || c == C) && (d == 0 || d == D) );
	
			return
			(
				((nz | (nz >> 1)) & N) |
				((nz & 0xFF) ? 0 : Z) |
				c |
				(v ? V : 0) |
				i |
				d |
				R
			);
		}
	
		void Cpu::Flags::Unpack(const uint f)
		{
			nz = (~f & Z) | ((f & N) << 1);
			c =  f & C;
			v =  f & V;	
			i =  f & I;
			d =  f & D;
		}
	
		inline void Cpu::Cycles::NextRound(const Cycle next)
		{
			if (round > next)
				round = next;
		}
	
		inline uint Cpu::Interrupt::Clock(const Cycle cycle)
		{
			if (cycle >= nmiClock)
			{
				nmiClock = NES_CYCLE_MAX;
				return NMI_VECTOR;
			}
			else if (cycle >= irqClock)
			{
				irqClock = NES_CYCLE_MAX;
				return IRQ_VECTOR;
			}
			else
			{
				return 0;
			}
		}
	
		inline void Cpu::Interrupt::EndFrame(const Cycle frameClock)
		{
			if (nmiClock != NES_CYCLE_MAX)
			{
				NST_VERIFY( nmiClock >= frameClock );
				nmiClock -= frameClock;
			}
	
			if (irqClock != NES_CYCLE_MAX)
				irqClock = irqClock > frameClock ? irqClock - frameClock : 0;
		}
	
		inline uint Cpu::FetchZpg16(const uint address) const
		{
			return ram.page.zero[address & 0xFF] | (ram.page.zero[(address+1) & 0xFF] << 8);
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("a", on)
        #endif
	
		inline uint Cpu::FetchPc8()
		{
			const uint data = map.Peek8( pc );
			++pc;
			return data;
		}
	
		inline uint Cpu::FetchPc16()
		{
			const uint data = map.Peek16( pc );
			pc += 2;
			return data;
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #pragma optimize("w", on)
        #endif
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Immediate addressing
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline uint Cpu::Imm_R() 
		{
			const uint data = FetchPc8();
			cycles.count += cycles.clock[1];
			return data;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Absolute addressing
		////////////////////////////////////////////////////////////////////////////////////////
	
		uint Cpu::Abs_R() 
		{
			uint data = FetchPc16();
			cycles.count += cycles.clock[2];
			
			data = map.Peek8( data );
			cycles.count += cycles.clock[0];		
	
			return data;
		}
	
		uint Cpu::Abs_RW(uint& data) 
		{
			const uint address = FetchPc16();
			cycles.count += cycles.clock[2];
	
			data = map.Peek8( address );
			cycles.count += cycles.clock[0];
	
			map.Poke8( address, data );
			cycles.count += cycles.clock[0];
	
			return address;
		}
	
		inline uint Cpu::Abs_W() 
		{
			const uint address = FetchPc16();
			cycles.count += cycles.clock[2];
			return address;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Zero page addressing
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline uint Cpu::Zpg_R() 
		{
			const uint address = FetchPc8();
			cycles.count += cycles.clock[2];
			return ram.page.zero[address];
		}
	
		inline uint Cpu::Zpg_RW(uint& data)
		{
			const uint address = FetchPc8();
			cycles.count += cycles.clock[4];
			data = ram.page.zero[address];
			return address;
		}
	
		inline uint Cpu::Zpg_W()
		{
			const uint address = FetchPc8();
			cycles.count += cycles.clock[2];
			return address;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Zero page indexed addressing
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline uint Cpu::ZpgReg_R(uint indexed) 
		{
			indexed = (indexed + FetchPc8()) & 0xFF;
			cycles.count += cycles.clock[3];
			return ram.page.zero[indexed];
		}
	
		inline uint Cpu::ZpgReg_RW(uint& data,uint indexed)
		{
			indexed = (indexed + FetchPc8()) & 0xFF;
			cycles.count += cycles.clock[5];
			data = ram.page.zero[indexed];
			return indexed;
		}
	
		inline uint Cpu::ZpgReg_W(uint indexed) 
		{
			indexed = (indexed + FetchPc8()) & 0xFF;
			cycles.count += cycles.clock[3];
			return indexed;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Zero page indexed addressing (X && Y)
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline uint Cpu::ZpgX_R()            { return ZpgReg_R( x );        }
		inline uint Cpu::ZpgX_RW(uint& data) { return ZpgReg_RW( data, x ); }
		inline uint Cpu::ZpgX_W()            { return ZpgReg_W( x );        }
		inline uint Cpu::ZpgY_R()            { return ZpgReg_R( y );        }
		inline uint Cpu::ZpgY_RW(uint& data) { return ZpgReg_RW( data, y ); }
		inline uint Cpu::ZpgY_W()            { return ZpgReg_W( y );        }
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Absolute indexed addressing
		////////////////////////////////////////////////////////////////////////////////////////
	
		uint Cpu::AbsReg_R(uint indexed)
		{
			uint data = pc;
			indexed += map.Peek8( data );
			data = (map.Peek8( data + 1 ) << 8) + indexed;
			cycles.count += cycles.clock[2];
	
			if (indexed & 0x100)
			{
				map.Peek8( data - 0x100 );
				cycles.count += cycles.clock[0];
			}
	
			data = map.Peek8( data );
			pc += 2;
			cycles.count += cycles.clock[0];
	
			return data;
		}
	
		uint Cpu::AbsReg_RW(uint& data,uint indexed)
		{
			uint address = pc;
			indexed += map.Peek8( address );
			address = (map.Peek8( address + 1 ) << 8) + indexed;
	
			map.Peek8( address - (indexed & 0x100) );
			pc += 2;
			cycles.count += cycles.clock[3];
	
			data = map.Peek8( address );
			cycles.count += cycles.clock[0];
	
			map.Poke8( address, data );
			cycles.count += cycles.clock[0];
	
			return address;
		}
	
		NST_FORCE_INLINE uint Cpu::AbsReg_W(uint indexed)
		{
			uint address = pc;
			indexed += map.Peek8( address );
			address = (map.Peek8( address + 1 ) << 8) + indexed;
	
			map.Peek8( address - (indexed & 0x100) );
			pc += 2;
			cycles.count += cycles.clock[3];
	
			return address;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Absolute indexed addressing (X && Y)
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline uint Cpu::AbsX_R()            { return AbsReg_R( x );        }
		inline uint Cpu::AbsY_R()            { return AbsReg_R( y );        }
		inline uint Cpu::AbsX_RW(uint& data) { return AbsReg_RW( data, x ); }										 
		inline uint Cpu::AbsY_RW(uint& data) { return AbsReg_RW( data, y ); }
		NST_FORCE_INLINE uint Cpu::AbsX_W()  { return AbsReg_W( x );        }
		NST_FORCE_INLINE uint Cpu::AbsY_W()  { return AbsReg_W( y );        }
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Indexed indirect addressing
		////////////////////////////////////////////////////////////////////////////////////////
	
		uint Cpu::IndX_R() 
		{
			uint data = FetchPc8() + x;
			cycles.count += cycles.clock[4];
			data = FetchZpg16( data );
	
			data = map.Peek8( data );
			cycles.count += cycles.clock[0];
	
			return data;
		}
	
		inline uint Cpu::IndX_RW(uint& data) 
		{
			uint address = FetchPc8() + x;
			cycles.count += cycles.clock[4];
			address = FetchZpg16( address );
			
			data = map.Peek8( address );
			cycles.count += cycles.clock[0];
	
			map.Poke8( address, data );
			cycles.count += cycles.clock[0];
	
			return address;
		}
	
		inline uint Cpu::IndX_W() 
		{
			const uint address = FetchPc8() + x;
			cycles.count += cycles.clock[4];
			return FetchZpg16( address );
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// Indirect indexed addressing
		////////////////////////////////////////////////////////////////////////////////////////
	
		uint Cpu::IndY_R() 
		{
			uint data = FetchPc8();
			cycles.count += cycles.clock[3];
	
			const uint indexed = ram.page.zero[data] + y;
			data = (ram.page.zero[(data + 1) & 0xFF] << 8) + indexed;
	
			if (indexed & 0x100)
			{
				map.Peek8( data - 0x100 );
				cycles.count += cycles.clock[0];
			}
	
			data = map.Peek8( data );
			cycles.count += cycles.clock[0];
	
			return data;
		}
	
		inline uint Cpu::IndY_RW(uint& data)
		{
			uint address = FetchPc8();
			cycles.count += cycles.clock[4];
	
			const uint indexed = ram.page.zero[address] + y;
			address = (ram.page.zero[(address + 1) & 0xFF] << 8) + indexed;
			map.Peek8( address - (indexed & 0x100) );
	
			data = map.Peek8( address );
			cycles.count += cycles.clock[0];
	
			map.Poke8( address, data );
			cycles.count += cycles.clock[0];
	
			return address;
		}
	
		NST_FORCE_INLINE uint Cpu::IndY_W() 
		{
			uint address = FetchPc8();
			cycles.count += cycles.clock[4];
	
			const uint indexed = ram.page.zero[address] + y;
			address = (ram.page.zero[(address + 1) & 0xFF] << 8) + indexed;
	
			map.Peek8( address - (indexed & 0x100) );
	
			return address;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// relative addressing
		////////////////////////////////////////////////////////////////////////////////////////
										
		template<bool STATE>
		inline void Cpu::Branch(uint tmp)
		{
			if ((!!tmp) == STATE)
			{
				tmp = pc;
				pc = (++tmp + sign_cast<i8>(map.Peek8( pc ))) & 0xFFFFU;
				cycles.count += cycles.clock[2 + (((tmp ^ pc) >> 8) & 1)];
			}
			else
			{
				++pc;
				cycles.count += cycles.clock[1];
			}
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// store data
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline void Cpu::StoreMem(const uint address,const uint data)
		{
			map.Poke8( address, data );
			cycles.count += cycles.clock[0];
		}
	
		inline void Cpu::StoreZpg(const uint address,const uint data)
		{
			ram.page.zero[address] = data;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// stack management
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline void Cpu::Push8(const uint data)
		{
			NST_ASSERT( sp <= 0xFF );
	
			const uint p = sp;
			sp = (sp - 1) & 0xFF;
			
			ram.page.stack[p] = data;
		}
	
		inline void Cpu::Push16(const uint data)
		{
			NST_ASSERT( sp <= 0xFF );
	
			const uint p0 = sp;
			const uint p1 = (p0 - 1) & 0xFF;
			sp = (p1 - 1) & 0xFF;
			
			ram.page.stack[p1] = data & 0xFF;
			ram.page.stack[p0] = data >> 8;
		}
	
		inline uint Cpu::Pull8()
		{
			sp = (sp + 1) & 0xFF;
			
			return ram.page.stack[sp];
		}
	
		inline uint Cpu::Pull16()
		{
			const uint p0 = (sp + 1) & 0xFF; 
			const uint p1 = (p0 + 1) & 0xFF;
			sp = p1;
	
			return ram.page.stack[p0] | (ram.page.stack[p1] << 8);
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// load instructions
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline void Cpu::Lda(const uint data) { flags.nz = a = data; }	      
		inline void Cpu::Ldx(const uint data) { flags.nz = x = data; }
		inline void Cpu::Ldy(const uint data) { flags.nz = y = data; }
	
		////////////////////////////////////////////////////////////////////////////////////////
		// store instructions
		////////////////////////////////////////////////////////////////////////////////////////
	
		NST_FORCE_INLINE uint Cpu::Sta() const { return a; }
		NST_FORCE_INLINE uint Cpu::Stx() const { return x; }
		NST_FORCE_INLINE uint Cpu::Sty() const { return y; }  
	
		////////////////////////////////////////////////////////////////////////////////////////
		// transfer instructions
		////////////////////////////////////////////////////////////////////////////////////////
	
		NST_FORCE_INLINE void Cpu::Tax() 
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = x = a; 
		}   
	
		NST_FORCE_INLINE void Cpu::Tay()
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = y = a; 
		}
	
		NST_FORCE_INLINE void Cpu::Txa() 
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = a = x; 
		}
	
		NST_FORCE_INLINE void Cpu::Tya() 
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = a = y; 
		}           
	
		////////////////////////////////////////////////////////////////////////////////////////
		// flow control instructions
		////////////////////////////////////////////////////////////////////////////////////////
	
		NST_FORCE_INLINE void Cpu::JmpAbs() 
		{
			pc = map.Peek16( pc );
			cycles.count += cycles.clock[JMP_ABS_CYCLES-1];
		}       
	
		NST_FORCE_INLINE void Cpu::JmpInd() 
		{
			// 6502 trap, can't cross between pages
	
			const uint pos = map.Peek16( pc );
			pc = u8(map.Peek8( pos )) | (u8(map.Peek8( (pos & 0xFF00U) | ((pos + 1) & 0x00FFU) )) << 8);
	
			cycles.count += cycles.clock[JMP_IND_CYCLES-1];
		}       
	
		NST_FORCE_INLINE void Cpu::Jsr() 
		{
			// 6502 trap, return address pushed on the stack is 
			// one byte prior to the next instruction
	
			Push16( pc + 1 );
			pc = map.Peek16( pc );
			cycles.count += cycles.clock[JSR_CYCLES-1];
		}           
	
		NST_FORCE_INLINE void Cpu::Rts() 
		{
			pc = Pull16() + 1;
			cycles.count += cycles.clock[RTS_CYCLES-1];
		}           
	
		NST_FORCE_INLINE void Cpu::Rti() 
		{
			cycles.count += cycles.clock[RTI_CYCLES-1];
	
			{
				const uint packed = Pull8();
				pc = Pull16();
				flags.Unpack( packed );
			}
	
			if (!interrupt.low || flags.i)
				interrupt.irqClock = NES_CYCLE_MAX;
			else
				cycles.round = interrupt.irqClock = 0;
		}   
	
		NST_FORCE_INLINE void Cpu::Bne() { Branch< true  >( flags.nz & 0xFF  ); }
		NST_FORCE_INLINE void Cpu::Beq() { Branch< false >( flags.nz & 0xFF  ); }
		NST_FORCE_INLINE void Cpu::Bmi() { Branch< true  >( flags.nz & 0x180 ); }
		NST_FORCE_INLINE void Cpu::Bpl() { Branch< false >( flags.nz & 0x180 ); }
		NST_FORCE_INLINE void Cpu::Bcs() { Branch< true  >( flags.c          ); }
		NST_FORCE_INLINE void Cpu::Bcc() { Branch< false >( flags.c          ); }
		NST_FORCE_INLINE void Cpu::Bvs() { Branch< true  >( flags.v          ); }
		NST_FORCE_INLINE void Cpu::Bvc() { Branch< false >( flags.v          ); }
	
		////////////////////////////////////////////////////////////////////////////////////////
		// math operations
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline void Cpu::Adc(const uint data) 
		{
			NST_ASSERT( flags.c <= 1 );
	
			// N2A03 has no BCD mode
	
			const uint tmp = a + data + flags.c;
			flags.v = (~(a ^ data)) & (a ^ tmp) & 0x80;
			flags.nz = a = tmp & 0xFF;
			flags.c = (tmp >> 8) & 0x1;
		}
	
		inline void Cpu::Sbc(const uint data) 
		{
			NST_ASSERT( flags.c <= 1 );
	
			// the N2A03 has no BCD mode
	
			const uint tmp = a - data - (flags.c ^ 0x1);
			flags.v = (a ^ data) & (a ^ tmp) & 0x80;
			flags.nz = a = tmp & 0xFF;
			flags.c = (~tmp >> 8) & 0x1;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// logical operations
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline void Cpu::And(const uint data) 
		{
			flags.nz = a = a & data; 
		}
	
		inline void Cpu::Ora(const uint data) 
		{ 
			flags.nz = a = a | data;
		}
		
		inline void Cpu::Eor(const uint data) 
		{
			flags.nz = a = a ^ data; 
		}
	
		inline void Cpu::Bit(const uint data) 
		{
			flags.nz = ((data & a) != 0) | ((data & Flags::N) << 1);
			flags.v = data & Flags::V;
		}	      
	
		inline void Cpu::Cmp(uint data) 
		{
			data = a - data;
			flags.nz = data & 0xFF;
			flags.c = (~data >> 8) & 0x1;
		}
	
		inline void Cpu::Cpx(uint data) 
		{
			data = x - data;
			flags.nz = data & 0xFF;
			flags.c = (~data >> 8) & 0x1;
		}
	
		inline void Cpu::Cpy(uint data) 
		{
			data = y - data;
			flags.nz = data & 0xFF;
			flags.c = (~data >> 8) & 0x1;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// shift operations
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline uint Cpu::Asl(const uint data) 
		{
			flags.c = data >> 7;
			return flags.nz = (data << 1) & 0xFF;
		}
	
		inline uint Cpu::Lsr(const uint data) 
		{
			flags.c = data & 0x01;
			return flags.nz = data >> 1;
		}
	
		inline uint Cpu::Rol(const uint data) 
		{
			NST_ASSERT( flags.c <= 1 );
	
			flags.nz = ((data << 1) & 0xFF) | flags.c;
			flags.c = data >> 7;
			
			return flags.nz;
		}
	
		inline uint Cpu::Ror(const uint data) 
		{
			NST_ASSERT( flags.c <= 1 );
	
			flags.nz = (data >> 1) | (flags.c << 7);
			flags.c = data & 0x01;
			
			return flags.nz;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// increment and decrement operations
		////////////////////////////////////////////////////////////////////////////////////////
	
		inline uint Cpu::Dec(const uint data) 
		{ 
			return flags.nz = (data - 1) & 0xFF; 
		}
	
		inline uint Cpu::Inc(const uint data) 
		{ 
			return flags.nz = (data + 1) & 0xFF; 
		} 
	
		NST_FORCE_INLINE void Cpu::Dex() 
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = x = (x - 1) & 0xFF; 
		}           
		
		NST_FORCE_INLINE void Cpu::Dey() 
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = y = (y - 1) & 0xFF; 
		}           
		
		NST_FORCE_INLINE void Cpu::Inx() 
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = x = (x + 1) & 0xFF; 
		}           
		
		NST_FORCE_INLINE void Cpu::Iny() 
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = y = (y + 1) & 0xFF; 
		}           
	
		////////////////////////////////////////////////////////////////////////////////////////
		// flags instructions
		////////////////////////////////////////////////////////////////////////////////////////
	
		NST_FORCE_INLINE void Cpu::Clc() 
		{ 
			cycles.count += cycles.clock[1];
			flags.c = 0; 
		}
	
		NST_FORCE_INLINE void Cpu::Sec() 
		{ 
			cycles.count += cycles.clock[1];
			flags.c = Flags::C;
		}
	
		NST_FORCE_INLINE void Cpu::Cld() 
		{ 
			cycles.count += cycles.clock[1];
			flags.d = 0; 
		}
	
		NST_FORCE_INLINE void Cpu::Sed() 
		{ 
			cycles.count += cycles.clock[1];
			flags.d = Flags::D; 
		}
	
		NST_FORCE_INLINE void Cpu::Clv() 
		{ 
			cycles.count += cycles.clock[1];
			flags.v = 0;
		}  
	
		NST_FORCE_INLINE void Cpu::Sei() 
		{
			cycles.count += cycles.clock[1];
	
			if (!flags.i)
			{
				flags.i = Flags::I;
				interrupt.irqClock = NES_CYCLE_MAX;
	
				// new I flag respect is delayed for one instruction
	
				if (interrupt.low)
					DoISR( IRQ_VECTOR );
			}
		}
	
		NST_FORCE_INLINE void Cpu::Cli() 
		{
			cycles.count += cycles.clock[1];
	
			if (flags.i)
			{
				flags.i = 0;
	
				NST_VERIFY( interrupt.irqClock == NES_CYCLE_MAX );
	
				// new I flag respect is delayed for one instruction
	
				if (interrupt.low)
				{
					interrupt.irqClock = cycles.count + 1;
					cycles.NextRound( interrupt.irqClock );
				}
			}
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// stack operations
		////////////////////////////////////////////////////////////////////////////////////////
	
		NST_FORCE_INLINE void Cpu::Pha() 
		{ 
			cycles.count += cycles.clock[PHA_CYCLES-1];
			Push8( a );
		}
	
		NST_FORCE_INLINE void Cpu::Php() 
		{ 
			// 6502 trap, B flag joins the club
	
			cycles.count += cycles.clock[PHP_CYCLES-1];
			Push8( flags.Pack() | Flags::B ); 
		}
	
		NST_FORCE_INLINE void Cpu::Pla() 
		{ 
			cycles.count += cycles.clock[PLA_CYCLES-1];
			flags.nz = a = Pull8(); 
		}
	
		NST_FORCE_INLINE void Cpu::Plp() 
		{
			cycles.count += cycles.clock[PLP_CYCLES-1];
	
			const uint i = flags.i;
			flags.Unpack( Pull8() );          
	
			// new I flag respect is delayed for one instruction
	
			if (interrupt.low)
			{
				if (i > flags.i)
				{
					interrupt.irqClock = cycles.count + 1;
					cycles.NextRound( interrupt.irqClock );
				}
				else if (i < flags.i)
				{
					interrupt.irqClock = NES_CYCLE_MAX;
					DoISR( IRQ_VECTOR );
				}
			}
		}
	
		NST_FORCE_INLINE void Cpu::Tsx() 
		{ 
			cycles.count += cycles.clock[1];
			flags.nz = x = sp;
		}
	
		NST_FORCE_INLINE void Cpu::Txs() 
		{ 
			cycles.count += cycles.clock[1];
			sp = x;
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// undocumented instructions, rarely used
		////////////////////////////////////////////////////////////////////////////////////////
	
		void Cpu::Anc(const uint data) 
		{
			flags.nz = a = a & data;
			flags.c = flags.nz >> 7;		
			LogMsg("Cpu: unofficial opcode executed - ANC" NST_LINEBREAK,1UL << 0);
		}
	
		NST_FORCE_INLINE void Cpu::Ane(const uint data) 
		{
			flags.nz = a = (a | 0xEE) & x & data;		
			LogMsg("Cpu: unofficial opcode executed - ANE" NST_LINEBREAK,1UL << 1);
		}
	
		NST_FORCE_INLINE void Cpu::Arr(const uint data) 
		{
			flags.nz = a = ((data & a) >> 1) | (flags.c << 7);
			flags.c = (a & 0x40) >> 6;
			flags.v = ((a >> 6) ^ (a >> 5)) & 0x1;
			LogMsg("Cpu: unofficial opcode executed - ARR" NST_LINEBREAK,1UL << 2);
		}
	
		NST_FORCE_INLINE void Cpu::Asr(const uint data) 
		{
			flags.c = data & a & 0x1;
			flags.nz = a = (data & a) >> 1;
			LogMsg("Cpu: unofficial opcode executed - ASR" NST_LINEBREAK,1UL << 3);
		}
	
		uint Cpu::Dcp(uint data) 
		{
			data = (data - 1) & 0xFF;
			Cmp( data );		
			LogMsg("Cpu: unofficial opcode executed - DCP" NST_LINEBREAK,1UL << 4);		
			return data;
		}
	
		uint Cpu::Isb(uint data) 
		{
			data = (data + 1) & 0xFF;
			Sbc( data );		
			LogMsg("Cpu: unofficial opcode executed - ISB" NST_LINEBREAK,1UL << 5);		
			return data;
		}
	
		NST_FORCE_INLINE void Cpu::Las(const uint data) 
		{
			flags.nz = a = x = sp = sp & data;		
			LogMsg("Cpu: unofficial opcode executed - LAS" NST_LINEBREAK,1UL << 6);
		}
	
		void Cpu::Lax(const uint data) 
		{ 
			flags.nz = x = a = data;		
			LogMsg("Cpu: unofficial opcode executed - LAX" NST_LINEBREAK,1UL << 7);
		}       
	
		NST_FORCE_INLINE void Cpu::Lxa(const uint data) 
		{
			flags.nz = x = a = a & data;		
			LogMsg("Cpu: unofficial opcode executed - LXA" NST_LINEBREAK,1UL << 8);
		}
	
		uint Cpu::Rla(uint data) 
		{
			const uint carry = flags.c;
			flags.c = data >> 7;
			data = ((data << 1) & 0xFF) | carry;
			flags.nz = a = a & data;		
			LogMsg("Cpu: unofficial opcode executed - RLA" NST_LINEBREAK,1UL << 9);		
			return data;
		}      
	
		uint Cpu::Rra(uint data) 
		{
			const uint carry = flags.c << 7;
			flags.c = data & 0x01;
			data = (data >> 1) | carry;
			Adc( data );		
			LogMsg("Cpu: unofficial opcode executed - RRA" NST_LINEBREAK,1UL << 10);
			return data;
		} 
	
		uint Cpu::Sax() const 
		{
			const uint data = a & x;
			LogMsg("Cpu: unofficial opcode executed - SAX" NST_LINEBREAK,1UL << 11);		
			return data;
		}  
	
		NST_FORCE_INLINE void Cpu::Sbx(uint data) 
		{
			data = (a & x) - data;
			flags.c = (data <= 0xFF);
			flags.nz = x = data & 0xFF;
			LogMsg("Cpu: unofficial opcode executed - SBX" NST_LINEBREAK,1UL << 12);
		}
	
		uint Cpu::Sha(uint address) const
		{
			address = a & x & ((address >> 8) + 1);
			LogMsg("Cpu: unofficial opcode executed - SHA" NST_LINEBREAK,1UL << 13);
			return address;
		}
	
		NST_FORCE_INLINE uint Cpu::Shs(uint address) 
		{
			sp = a & x;		
			address = sp & ((address >> 8) + 1);
			LogMsg("Cpu: unofficial opcode executed - SHS" NST_LINEBREAK,1UL << 14);
			return address;
		}
	
		NST_FORCE_INLINE uint Cpu::Shx(uint address) const
		{
			address = x & ((address >> 8) + 1);
			LogMsg("Cpu: unofficial opcode executed - SHX" NST_LINEBREAK,1UL << 15);		
			return address;
		}
	
		NST_FORCE_INLINE uint Cpu::Shy(uint address) const
		{
			address = y & ((address >> 8) + 1);
			LogMsg("Cpu: unofficial opcode executed - SHY" NST_LINEBREAK,1UL << 16);
			return address;
		}
	
		uint Cpu::Slo(uint data) 
		{
			flags.c = data >> 7;
			data = (data << 1) & 0xFF;
			flags.nz = a = a | data;
			LogMsg("Cpu: unofficial opcode executed - SLO" NST_LINEBREAK,1UL << 17);
			return data;
		}
	
		uint Cpu::Sre(uint data) 
		{
			flags.c = data & 0x01;
			data >>= 1;
			flags.nz = a = a ^ data;
			LogMsg("Cpu: unofficial opcode executed - SRE" NST_LINEBREAK,1UL << 18);
			return data;
		}
	
		void Cpu::Dop() const
		{ 
			LogMsg("Cpu: unofficial opcode executed - DOP" NST_LINEBREAK,1UL << 19); 
		}
	
		void Cpu::Top() const
		{ 
			LogMsg("Cpu: unofficial opcode executed - TOP" NST_LINEBREAK,1UL << 20); 
		}
	
		////////////////////////////////////////////////////////////////////////////////////////
		// interrupts
		////////////////////////////////////////////////////////////////////////////////////////
	
		void Cpu::DoISR(const uint vector)
		{
			if (!jammed)
			{
				Push16( pc );
				Push8( flags.Pack() );
				flags.i = Flags::I;
				pc = map.Peek16( vector );
				cycles.count += cycles.clock[INT_CYCLES-1];
			}
		}
	
		NST_FORCE_INLINE void Cpu::Brk() 
		{
			Push16( pc + 1 );
			Push8( flags.Pack() | Flags::B );
			flags.i = Flags::I;
	
			// 6502 trap, a pending NMI can get serviced right inside BRK if
			// it occurs before the flags get pushed on the stack
	
			const uint vector = interrupt.Clock( cycles.count + cycles.clock[2] );
			NST_VERIFY( vector != NMI_VECTOR );
	
			pc = map.Peek16( vector ? vector : IRQ_VECTOR );		
			cycles.count += cycles.clock[BRK_CYCLES-1];
	
			LogMsg("Cpu: opcode executed - BRK" NST_LINEBREAK,1UL << 21);
		}	
	
		void Cpu::Jam() 
		{
			// roll back and keep jamin'
	
			pc = (pc - 1U) & 0xFFFFU;
			cycles.count += cycles.clock[1];
	
			if (!jammed)
			{
				jammed = true;
				interrupt.Jam();
        		NST_DEBUG_MSG("6502 jam");
				Api::User::eventCallback( Api::User::EVENT_CPU_JAM );
				Log::Flush( "Cpu: jammed" NST_LINEBREAK );
			}
		}         
	
		////////////////////////////////////////////////////////////////////////////////////////
		// opcodes
		////////////////////////////////////////////////////////////////////////////////////////
	
        #define StoreZpgX(a_,d_) StoreZpg(a_,d_)
        #define StoreZpgY(a_,d_) StoreZpg(a_,d_)
        #define StoreAbs(a_,d_)  StoreMem(a_,d_)
        #define StoreAbsX(a_,d_) StoreMem(a_,d_)
        #define StoreAbsY(a_,d_) StoreMem(a_,d_)
        #define StoreIndX(a_,d_) StoreMem(a_,d_)
        #define StoreIndY(a_,d_) StoreMem(a_,d_)
	
        #define NES_I____(instr_,hex_)			      \
												      \
		void Cpu::op##hex_()					      \
		{										      \
			instr_();						          \
		}
	
        #define NES____C_(nop_,ticks_,hex_) 	      \
		                                              \
		void Cpu::op##hex_()					      \
		{										      \
			cycles.count += cycles.clock[ticks_ - 1]; \
		}
	
        #define NES_IR___(instr_,addr_,hex_)	      \
		                                              \
		void Cpu::op##hex_()					      \
		{										      \
        	instr_( addr_##_R() );    			      \
		}
	
        #define NES_I_W__(instr_,addr_,hex_)	      \
                                            	      \
		void Cpu::op##hex_()					      \
		{										      \
        	const uint dst = addr_##_W();    	      \
			Store##addr_( dst, instr_() );	          \
		}
	
        #define NES_IRW__(instr_,addr_,hex_)	      \
                                                      \
		void Cpu::op##hex_()					      \
		{										      \
			uint data;	    						  \
			const uint dst = addr_##_RW( data );	  \
			Store##addr_( dst, instr_(data) );  	  \
		}
	
        #define NES_IRA__(instr_,hex_)	              \
                                            	      \
		void Cpu::op##hex_()					      \
		{										      \
        	cycles.count += cycles.clock[1];          \
			a = instr_( a );					      \
		}
	
        #define NES_I_W_A(instr_,addr_,hex_)	      \
                                             	      \
		void Cpu::op##hex_()					      \
		{										      \
         	const uint dst = addr_##_W();		      \
			Store##addr_( dst, instr_(dst) );	      \
		}
	
        #define NES_IP_C_(instr_,ops_,ticks_,hex_)    \
		                                              \
		void Cpu::op##hex_()					      \
		{										      \
			pc += ops_;             			      \
			cycles.count += cycles.clock[ticks_ - 1]; \
			instr_();             			          \
		}
	
		// param 1 = instruction
		// param 2 = addressing mode
		// param 3 = cycles
		// param 4 = opcode
	
		NES_IR___( Adc, Imm,      0x69 )
		NES_IR___( Adc, Zpg,      0x65 )
		NES_IR___( Adc, ZpgX,     0x75 )
		NES_IR___( Adc, Abs,      0x6D )
		NES_IR___( Adc, AbsX,     0x7D )
		NES_IR___( Adc, AbsY,     0x79 )
		NES_IR___( Adc, IndX,     0x61 )
		NES_IR___( Adc, IndY,     0x71 )
		NES_IR___( And, Imm,      0x29 )   
		NES_IR___( And, Zpg,      0x25 )
		NES_IR___( And, ZpgX,     0x35 )
		NES_IR___( And, Abs,      0x2D )
		NES_IR___( And, AbsX,     0x3D )
		NES_IR___( And, AbsY,     0x39 )
		NES_IR___( And, IndX,     0x21 )
		NES_IR___( And, IndY,     0x31 )
		NES_IRA__( Asl,           0x0A )
		NES_IRW__( Asl, Zpg,      0x06 )
		NES_IRW__( Asl, ZpgX,     0x16 )
		NES_IRW__( Asl, Abs,      0x0E )
		NES_IRW__( Asl, AbsX,     0x1E )
		NES_I____( Bcc, 	      0x90 )
		NES_I____( Bcs, 	      0xB0 )
		NES_I____( Beq,  	      0xF0 )
		NES_IR___( Bit, Zpg,      0x24 )
		NES_IR___( Bit, Abs,      0x2C )
		NES_I____( Bmi, 	      0x30 )
		NES_I____( Bne, 	      0xD0 )
		NES_I____( Bpl, 	      0x10 )
		NES_I____( Brk,           0x00 )
		NES_I____( Bvc,  		  0x50 )
		NES_I____( Bvs,  	      0x70 )
		NES_I____( Clc,           0x18 )
		NES_I____( Cld,           0xD8 )
		NES_I____( Cli,           0x58 )
		NES_I____( Clv,           0xB8 )
		NES_IR___( Cmp, Imm,      0xC9 )
		NES_IR___( Cmp, Zpg,      0xC5 )
		NES_IR___( Cmp, ZpgX,     0xD5 )
		NES_IR___( Cmp, Abs,      0xCD )
		NES_IR___( Cmp, AbsX,     0xDD )
		NES_IR___( Cmp, AbsY,     0xD9 )
		NES_IR___( Cmp, IndX,     0xC1 )
		NES_IR___( Cmp, IndY,     0xD1 )
		NES_IR___( Cpx, Imm,      0xE0 )
		NES_IR___( Cpx, Zpg,      0xE4 )
		NES_IR___( Cpx, Abs,      0xEC )
		NES_IR___( Cpy, Imm,      0xC0 )
		NES_IR___( Cpy, Zpg,      0xC4 )
		NES_IR___( Cpy, Abs,      0xCC )
		NES_IRW__( Dec, Zpg,      0xC6 )
		NES_IRW__( Dec, ZpgX,     0xD6 )
		NES_IRW__( Dec, Abs,      0xCE )
		NES_IRW__( Dec, AbsX,     0xDE )
		NES_I____( Dex,           0xCA )
		NES_I____( Dey,           0x88 )
		NES_IR___( Eor, Imm,      0x49 )
		NES_IR___( Eor, Zpg,      0x45 )
		NES_IR___( Eor, ZpgX,     0x55 )
		NES_IR___( Eor, Abs,      0x4D )
		NES_IR___( Eor, AbsX,     0x5D )
		NES_IR___( Eor, AbsY,     0x59 )
		NES_IR___( Eor, IndX,     0x41 )
		NES_IR___( Eor, IndY,     0x51 )
		NES_IRW__( Inc, Zpg,      0xE6 )
		NES_IRW__( Inc, ZpgX,     0xF6 )
		NES_IRW__( Inc, Abs,      0xEE )
		NES_IRW__( Inc, AbsX,     0xFE )
		NES_I____( Inx,           0xE8 )
		NES_I____( Iny,           0xC8 )
		NES_I____( Jam,		      0x02 )
		NES_I____( Jam,		      0x12 )
		NES_I____( Jam,		      0x22 )
		NES_I____( Jam,		      0x32 )
		NES_I____( Jam,		      0x42 )
		NES_I____( Jam,		      0x52 )
		NES_I____( Jam,		      0x62 )
		NES_I____( Jam,		      0x72 )
		NES_I____( Jam,		      0x92 )
		NES_I____( Jam,		      0xB2 )
		NES_I____( Jam,		      0xD2 )
		NES_I____( Jam,		      0xF2 )
		NES_I____( JmpAbs,        0x4C )
		NES_I____( JmpInd,        0x6C )
		NES_I____( Jsr,           0x20 )
		NES_IR___( Lda, Imm,      0xA9 )
		NES_IR___( Lda, Zpg,      0xA5 )
		NES_IR___( Lda, ZpgX,     0xB5 )
		NES_IR___( Lda, Abs,      0xAD )
		NES_IR___( Lda, AbsX,     0xBD )
		NES_IR___( Lda, AbsY,     0xB9 )
		NES_IR___( Lda, IndX,     0xA1 )
		NES_IR___( Lda, IndY,     0xB1 )
		NES_IR___( Ldx, Imm,      0xA2 )
		NES_IR___( Ldx, Zpg,      0xA6 )
		NES_IR___( Ldx, ZpgY,     0xB6 )
		NES_IR___( Ldx, Abs,      0xAE )
		NES_IR___( Ldx, AbsY,     0xBE )
		NES_IR___( Ldy, Imm,      0xA0 )
		NES_IR___( Ldy, Zpg,      0xA4 )
		NES_IR___( Ldy, ZpgX,     0xB4 )
		NES_IR___( Ldy, Abs,      0xAC )
		NES_IR___( Ldy, AbsX,     0xBC )
		NES_IRA__( Lsr,           0x4A )
		NES_IRW__( Lsr, Zpg,      0x46 )
		NES_IRW__( Lsr, ZpgX,     0x56 )
		NES_IRW__( Lsr, Abs,      0x4E )
		NES_IRW__( Lsr, AbsX,     0x5E )
		NES____C_( Nop,        2, 0x1A )
		NES____C_( Nop,        2, 0x3A )
		NES____C_( Nop,        2, 0x5A )
		NES____C_( Nop,        2, 0x7A )
		NES____C_( Nop,        2, 0xDA )
		NES____C_( Nop,        2, 0xEA )
		NES____C_( Nop,        2, 0xFA )
		NES_IR___( Ora, Imm,      0x09 )
		NES_IR___( Ora, Zpg,      0x05 )
		NES_IR___( Ora, ZpgX,     0x15 )
		NES_IR___( Ora, Abs,      0x0D )
		NES_IR___( Ora, AbsX,     0x1D )
		NES_IR___( Ora, AbsY,     0x19 )
		NES_IR___( Ora, IndX,     0x01 )
		NES_IR___( Ora, IndY,     0x11 )
		NES_I____( Pha,           0x48 )
		NES_I____( Php,           0x08 )
		NES_I____( Pla,           0x68 )
		NES_I____( Plp,           0x28 )
		NES_IRA__( Rol,           0x2A )
		NES_IRW__( Rol, Zpg,      0x26 )
		NES_IRW__( Rol, ZpgX,     0x36 )
		NES_IRW__( Rol, Abs,      0x2E )
		NES_IRW__( Rol, AbsX,     0x3E )
		NES_IRA__( Ror,           0x6A )
		NES_IRW__( Ror, Zpg,      0x66 )
		NES_IRW__( Ror, ZpgX,     0x76 )
		NES_IRW__( Ror, Abs,      0x6E )
		NES_IRW__( Ror, AbsX,     0x7E )
		NES_I____( Rti,           0x40 )
		NES_I____( Rts,           0x60 )
		NES_IR___( Sbc, Imm,      0xE9 )
		NES_IR___( Sbc, Imm,      0xEB )
		NES_IR___( Sbc, Zpg,      0xE5 )
		NES_IR___( Sbc, ZpgX,     0xF5 )
		NES_IR___( Sbc, Abs,      0xED )
		NES_IR___( Sbc, AbsX,     0xFD )
		NES_IR___( Sbc, AbsY,     0xF9 )
		NES_IR___( Sbc, IndX,     0xE1 )
		NES_IR___( Sbc, IndY,     0xF1 )
		NES_I____( Sec,           0x38 )
		NES_I____( Sed,           0xF8 )
		NES_I____( Sei,           0x78 )
		NES_I_W__( Sta, Zpg,      0x85 )
		NES_I_W__( Sta, ZpgX,     0x95 )
		NES_I_W__( Sta, Abs,      0x8D )
		NES_I_W__( Sta, AbsX,     0x9D )
		NES_I_W__( Sta, AbsY,     0x99 )
		NES_I_W__( Sta, IndX,     0x81 )
		NES_I_W__( Sta, IndY,     0x91 )
		NES_I_W__( Stx, Zpg,      0x86 )
		NES_I_W__( Stx, ZpgY,     0x96 )
		NES_I_W__( Stx, Abs,      0x8E )
		NES_I_W__( Sty, Zpg,      0x84 )
		NES_I_W__( Sty, ZpgX,     0x94 )
		NES_I_W__( Sty, Abs,      0x8C )
		NES_I____( Tax,           0xAA )
		NES_I____( Tay,           0xA8 )
		NES_I____( Tsx,           0xBA )
		NES_I____( Txa,           0x8A )
		NES_I____( Txs,           0x9A )
		NES_I____( Tya,           0x98 )
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		// unofficial stuff
	
		NES_IR___( Anc, Imm,      0x0B )
		NES_IR___( Anc, Imm,      0x2B ) 
		NES_IR___( Ane, Imm,      0x8B )
		NES_IR___( Arr, Imm,      0x6B )	
		NES_IR___( Asr, Imm,      0x4B )
		NES_IRW__( Dcp, Zpg,      0xC7 )
		NES_IRW__( Dcp, ZpgX,     0xD7 )
		NES_IRW__( Dcp, IndX,     0xC3 )
		NES_IRW__( Dcp, IndY,     0xD3 )
		NES_IRW__( Dcp, Abs,      0xCF )
		NES_IRW__( Dcp, AbsX,     0xDF )
		NES_IRW__( Dcp, AbsY,     0xDB )
		NES_IP_C_( Dop, 1,     2, 0x80 )
		NES_IP_C_( Dop, 1,     2, 0x82 )
		NES_IP_C_( Dop, 1,     2, 0x89 )
		NES_IP_C_( Dop, 1,     2, 0xC2 )
		NES_IP_C_( Dop, 1,     2, 0xE2 )
		NES_IP_C_( Dop, 1,     3, 0x04 )
		NES_IP_C_( Dop, 1,     3, 0x44 )
		NES_IP_C_( Dop, 1,     3, 0x64 )
		NES_IP_C_( Dop, 1,     4, 0x14 )
		NES_IP_C_( Dop, 1,     4, 0x34 )
		NES_IP_C_( Dop, 1,     4, 0x54 )
		NES_IP_C_( Dop, 1,     4, 0x74 )
		NES_IP_C_( Dop, 1,     4, 0xD4 )
		NES_IP_C_( Dop, 1,     4, 0xF4 )
		NES_IRW__( Isb, Zpg,      0xE7 )
		NES_IRW__( Isb, ZpgX,     0xF7 )
		NES_IRW__( Isb, Abs,      0xEF )
		NES_IRW__( Isb, AbsX,     0xFF )
		NES_IRW__( Isb, AbsY,     0xFB )
		NES_IRW__( Isb, IndX,     0xE3 )
		NES_IRW__( Isb, IndY,     0xF3 )
		NES_IR___( Las, AbsY,     0xBB )
		NES_IR___( Lax, Zpg,      0xA7 )
		NES_IR___( Lax, ZpgY,     0xB7 )
		NES_IR___( Lax, Abs,      0xAF )
		NES_IR___( Lax, AbsY,     0xBF )
		NES_IR___( Lax, IndX,     0xA3 )
		NES_IR___( Lax, IndY,     0xB3 )
		NES_IR___( Lxa, Imm,      0xAB )
		NES_IRW__( Rla, Zpg,      0x27 )
		NES_IRW__( Rla, ZpgX,     0x37 )
		NES_IRW__( Rla, Abs,      0x2F )
		NES_IRW__( Rla, AbsX,     0x3F )
		NES_IRW__( Rla, AbsY,     0x3B )
		NES_IRW__( Rla, IndX,     0x23 )
		NES_IRW__( Rla, IndY,     0x33 )
		NES_IRW__( Rra, Zpg,      0x67 )
		NES_IRW__( Rra, ZpgX,     0x77 )
		NES_IRW__( Rra, Abs,      0x6F )
		NES_IRW__( Rra, AbsX,     0x7F )
		NES_IRW__( Rra, AbsY,     0x7B )
		NES_IRW__( Rra, IndX,     0x63 )
		NES_IRW__( Rra, IndY,     0x73 )
		NES_I_W__( Sax, Zpg,      0x87 )
		NES_I_W__( Sax, ZpgY,     0x97 )
		NES_I_W__( Sax, Abs,      0x8F )
		NES_I_W__( Sax, IndX,     0x83 )
		NES_IR___( Sbx, Imm,      0xCB )
		NES_I_W_A( Sha, AbsY,     0x9F )
		NES_I_W_A( Sha, IndY,     0x93 )
		NES_I_W_A( Shs, AbsY,     0x9B )
		NES_I_W_A( Shx, AbsY,     0x9E )
		NES_I_W_A( Shy, AbsX,     0x9C )
		NES_IRW__( Slo, Zpg,      0x07 )
		NES_IRW__( Slo, ZpgX,     0x17 )
		NES_IRW__( Slo, Abs,      0x0F )
		NES_IRW__( Slo, AbsX,     0x1F )
		NES_IRW__( Slo, AbsY,     0x1B )
		NES_IRW__( Slo, IndX,     0x03 )
		NES_IRW__( Slo, IndY,     0x13 )
		NES_IRW__( Sre, Zpg,      0x47 )
		NES_IRW__( Sre, ZpgX,     0x57 )
		NES_IRW__( Sre, Abs,      0x4F )
		NES_IRW__( Sre, AbsX,     0x5F )
		NES_IRW__( Sre, AbsY,     0x5B )
		NES_IRW__( Sre, IndX,     0x43 )
		NES_IRW__( Sre, IndY,     0x53 )
		NES_IP_C_( Top, 2,     4, 0x0C )
		NES_IP_C_( Top, 2,     5, 0x1C )
		NES_IP_C_( Top, 2,     5, 0x3C )
		NES_IP_C_( Top, 2,     5, 0x5C )
		NES_IP_C_( Top, 2,     5, 0x7C )
		NES_IP_C_( Top, 2,     5, 0xDC )
		NES_IP_C_( Top, 2,     5, 0xFC )
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
        #undef StoreZpgX
        #undef StoreZpgY
        #undef StoreAbs 
        #undef StoreAbsX
        #undef StoreAbsY
        #undef StoreIndX
        #undef StoreIndY
	
        #undef NES_I____
        #undef NES____C_
        #undef NES_IR___
        #undef NES_I_W__
        #undef NES_IRW__
        #undef NES_IRA__
        #undef NES_I_W_A
        #undef NES_IP_C_
	
		////////////////////////////////////////////////////////////////////////////////////////
		// main
		////////////////////////////////////////////////////////////////////////////////////////
	
		void Cpu::BeginFrame(Sound::Output* const sound)
		{
			NST_VERIFY( cycles.count < frameClock );

			apu.BeginFrame( sound );

			Clock();
		}

		void Cpu::EndFrame()
		{
			apu.EndFrame();

			NST_VERIFY( cycles.count >= frameClock );

			ticks += frameClock;
			cycles.count -= frameClock;
			interrupt.EndFrame( frameClock );
		}
	
		void Cpu::DoIRQ(const uint line,const Cycle cycle)
		{
			NST_VERIFY( interrupt.source & line );
	
			interrupt.low |= line;
	
			if (!flags.i && interrupt.irqClock == NES_CYCLE_MAX)
			{
				// give some time for the falling edge
				interrupt.irqClock = cycle + cycles.clock[1];
				cycles.NextRound( interrupt.irqClock );
			}
		}
	
		void Cpu::DoNMI(const Cycle cycle)
		{
			if (interrupt.nmiClock == NES_CYCLE_MAX)
			{
				// give some time for the falling edge
				interrupt.nmiClock = cycle + cycles.clock[1];
				cycles.NextRound( interrupt.nmiClock );
			}
		}
	
		void Cpu::Clock()
		{
			Cycle clock = apu.Clock( cycles.count );
	
			if (const uint vector = interrupt.Clock( cycles.count ))
				DoISR( vector );

			if (clock > interrupt.irqClock)
				clock = interrupt.irqClock;

			if (clock > interrupt.nmiClock)
				clock = interrupt.nmiClock;

			if (clock > frameClock)
				clock = frameClock;

			cycles.round = clock;
		}

		void Cpu::Run0()
		{				   
			do
			{
				do 
				{		
					(*this.*(opcodes[FetchPc8()]))();
				} 
				while (cycles.count < cycles.round);
	
				Clock();
			}
			while (cycles.count < frameClock);
		}
	
		void Cpu::Run1()
		{
			const Hook hook( hooks[0] );
	
			do
			{
				do 
				{
					(*this.*(opcodes[FetchPc8()]))();
					hook.Execute();
				} 
				while (cycles.count < cycles.round);
	
				Clock();
			}
			while (cycles.count < frameClock);
		}
	
		void Cpu::Run2()
		{
			const Hook* const begin = hooks.Begin();
			const Hook* const end = hooks.End();
	
			do
			{
				do 
				{		
					(*this.*(opcodes[FetchPc8()]))();
	
					const Hook* NST_RESTRICT hook = begin;
	
					do
					{
						hook->Execute();
					}
					while (++hook != end);
				} 
				while (cycles.count < cycles.round);
	
				Clock();
			}
			while (cycles.count < frameClock);
		}
		
		uint Cpu::Peek(const uint address)
		{ 
			return map.Peek8( address );
		}
	
		void Cpu::Poke(const uint address,const uint data)
		{
			return map.Poke8( address, data );
		}
	}
}
