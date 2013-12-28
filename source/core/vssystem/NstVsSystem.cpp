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

#include "../NstLog.hpp"
#include "../NstState.hpp"
#include "../NstCpu.hpp"
#include "../NstPpu.hpp"
#include "NstVsSystem.hpp"
#include "NstVsRbiBaseball.hpp"
#include "NstVsTkoBoxing.hpp"
#include "NstVsSuperXevious.hpp"

namespace Nes
{
	namespace Core
	{
		const u8 VsSystem::colorMaps[4][64] =
		{
			{
				0x35, 0x23, 0x16, 0x22, 0x1C, 0x09, 0xFF, 0x15,
				0x20, 0x00, 0x27, 0x05, 0x04, 0x27, 0x08, 0x30,
				0x21, 0xFF, 0xFF, 0x29, 0x3C, 0x32, 0x36, 0x12,
				0xFF, 0x2B, 0x0F, 0xFF, 0x20, 0x10, 0x24, 0x01,
				0xFF, 0x31, 0xFF, 0x2A, 0x2C, 0x0C, 0xFF, 0xFF,
				0xFF, 0x07, 0x34, 0x06, 0x13, 0x02, 0x26, 0x0F,
				0xFF, 0x19, 0x10, 0x0A, 0x39, 0xFF, 0x37, 0x17,
				0xFF, 0x11, 0x09, 0xFF, 0x39, 0x25, 0x18, 0xFF
			},
			{
				0x0F, 0x27, 0x18, 0xFF, 0x3A, 0x25, 0xFF, 0x31,
				0x16, 0x13, 0x38, 0x34, 0x20, 0x23, 0xFF, 0x0B,
				0xFF, 0x21, 0x06, 0xFF, 0x1B, 0x29, 0xFF, 0x22,
				0xFF, 0x24, 0xFF, 0x2B, 0xFF, 0x08, 0xFF, 0x03,
				0xFF, 0x36, 0x26, 0x33, 0x11, 0xFF, 0x10, 0x02,
				0x14, 0xFF, 0x00, 0x09, 0x12, 0x0F, 0x37, 0x30,
				0xFF, 0xFF, 0x2A, 0x17, 0x0C, 0x01, 0x15, 0x19,
				0xFF, 0x2C, 0x07, 0x37, 0xFF, 0x05, 0x0A, 0x00 
			},
			{
				0x03, 0xFF, 0xFF, 0x10, 0x1A, 0x30, 0x31, 0x09,
				0x01, 0x0F, 0x36, 0x08, 0x15, 0xFF, 0xFF, 0x30,
				0x22, 0x1C, 0xFF, 0x12, 0x19, 0x18, 0x17, 0x1B,
				0x00, 0xFF, 0xFF, 0x02, 0x16, 0x06, 0xFF, 0x35,
				0x23, 0xFF, 0x0F, 0x37, 0xFF, 0x27, 0x26, 0x30,
				0x29, 0xFF, 0x21, 0x24, 0x11, 0xFF, 0x0F, 0xFF,
				0x2C, 0xFF, 0xFF, 0xFF, 0x07, 0x2A, 0x28, 0xFF,
				0x0A, 0xFF, 0x32, 0x37, 0x13, 0xFF, 0xFF, 0x0C
			},
			{
				0x18, 0xFF, 0x1C, 0x28, 0xFF, 0xFF, 0x01, 0x17,
				0x10, 0x0F, 0x2A, 0x0F, 0x36, 0x37, 0x1A, 0xFF,
				0x25, 0xFF, 0x12, 0xFF, 0x0F, 0xFF, 0xFF, 0x26,
				0xFF, 0xFF, 0x22, 0x19, 0xFF, 0x0F, 0x3A, 0x21,
				0x05, 0x0A, 0x07, 0x01, 0x13, 0xFF, 0x00, 0x15,
				0x0C, 0xFF, 0x11, 0xFF, 0xFF, 0x38, 0xFF, 0xFF,
				0xFF, 0xFF, 0x08, 0x16, 0xFF, 0xFF, 0x30, 0x3C,
				0x0F, 0x27, 0xFF, 0x31, 0x29, 0xFF, 0x30, 0x09 
			}
		};

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		class VsSystem::Dip
		{
			class Proxy;

			struct Setting
			{
				uint data;
				cstring name;
			};

			Setting* settings;
			uint size;
			uint selection;
			uint mask;
			cstring name;

		public:

			class Value
			{
				friend class Dip;
				friend class Proxy;

				cstring const name;
				const uint data;
				const uint selection;

			public:

				Value(cstring n,uint d,uint s=0)
				: name(n), data(d), selection(s) {}
			};

		private:

			class Proxy
			{
				Dip& dip;
				const uint index;

			public:

				Proxy(Dip& d,uint i)
				: dip(d), index(i) {}

				void operator = (const Value& value)
				{
					dip.settings[index].data = value.data;
					dip.settings[index].name = value.name;
				}

				operator uint() const
				{
					return dip.settings[index].data;
				}

				cstring Name() const
				{
					return dip.settings[index].name;
				}
			};

		public:

			Dip()
			: settings(NULL) {}

			~Dip()
			{
				delete [] settings;
			}

			void operator = (const Value& value)
			{
				NST_ASSERT( settings == NULL && value.data && value.selection < value.data );

				name = value.name;
				size = value.data;
				selection = value.selection;
				settings = new Setting [size];
			}

			uint Size() const
			{
				return size;
			}

			void Select(uint i)
			{
				NST_ASSERT( i < size );
				selection = i;
			}

			uint Selection() const
			{
				return selection;
			}

			Proxy operator [] (uint i)
			{
				NST_ASSERT( i < size );
				return Proxy(*this,i);
			}

			cstring Name() const
			{
				return name;
			}
		};

		VsSystem::VsDipSwitches::VsDipSwitches(Dip*& old,uint n)
		: table(old), size(n) 
		{
			old = NULL;
			regs[1] = regs[0] = 0x00;

			for (uint i=0; i < n; ++i)
			{
				regs[0] |= (table[i][table[i].Selection()] & DIPSWITCH_4016_MASK) << DIPSWITCH_4016_SHIFT;
				regs[1] |= (table[i][table[i].Selection()] & DIPSWITCH_4017_MASK) << DIPSWITCH_4017_SHIFT;
			}
		}

		VsSystem::VsDipSwitches::~VsDipSwitches()
		{
			delete [] table;
		}

		inline void VsSystem::VsDipSwitches::Reset()
		{
			regs[0] &= ~uint(COIN);
		}

		inline uint VsSystem::VsDipSwitches::Reg(uint i) const
		{
			return regs[i];
		}

		uint VsSystem::VsDipSwitches::NumDips() const
		{ 
			return size; 
		}

		uint VsSystem::VsDipSwitches::NumValues(uint dip) const
		{ 
			NST_ASSERT( dip < size );
			return table[dip].Size(); 
		}

		cstring VsSystem::VsDipSwitches::GetDipName(uint dip) const
		{ 
			NST_ASSERT( dip < size );
			return table[dip].Name(); 
		}

		cstring VsSystem::VsDipSwitches::GetValueName(uint dip,uint value) const
		{ 
			NST_ASSERT( dip < size && value < table[dip].Size() );
			return table[dip][value].Name();
		}

		uint VsSystem::VsDipSwitches::GetValue(uint dip) const
		{ 
			NST_ASSERT( dip < size );
			return table[dip].Selection(); 
		}

		bool VsSystem::VsDipSwitches::SetValue(uint dip,uint value)
		{ 
			NST_ASSERT( dip < size && value < table[dip].Size() );

			const uint old = table[dip].Selection();

			if (old != value)
			{
				regs[0] &= ~((table[dip][old] & DIPSWITCH_4016_MASK) << DIPSWITCH_4016_SHIFT);
				regs[1] &= ~((table[dip][old] & DIPSWITCH_4017_MASK) << DIPSWITCH_4017_SHIFT);

				table[dip].Select( value );

				regs[0] |= (table[dip][value] & DIPSWITCH_4016_MASK) << DIPSWITCH_4016_SHIFT;
				regs[1] |= (table[dip][value] & DIPSWITCH_4017_MASK) << DIPSWITCH_4017_SHIFT;

				return true;
			}

			return false;
		}

		void VsSystem::VsDipSwitches::BeginFrame(Input::Controllers* const input)
		{
			regs[0] &= ~uint(COIN);

			if (input && Input::Controllers::VsSystem::callback( input->vsSystem ))
				regs[0] |= input->vsSystem.insertCoin & COIN;
		}				  

		struct VsSystem::Context
		{
			Dip* dips;
			uint numDips;
			Cpu& cpu;
			Ppu& ppu;
			uint securityColor;
			uint securityPpu;
			ibool swapPorts;

			Context(Cpu& c,Ppu& p)
			: 
			dips          (NULL),
			numDips       (0),
			cpu           (c),
			ppu           (p),
			securityColor (UINT_MAX),
			securityPpu   (0),
			swapPorts     (false)
			{}

			void SetDips(uint n)
			{
				numDips = n;
				dips = new Dip [n];
			}
		};

		VsSystem* VsSystem::Create(Cpu& cpu,Ppu& ppu,const dword pRomCrc)
		{
			switch (pRomCrc)
			{
				// VS. Dual-System Games are unsupported
	
				case 0xB90497AAUL: // Tennis
				case 0x008A9C16UL: // Wrecking Crew 
				case 0xAD407F52UL: // Balloon Fight
				case 0x18A93B7BUL: // Mahjong (J)
				case 0x13A91937UL: // Baseball 
				case 0xF5DEBF88UL: // Baseball 
				case 0xF64D7252UL: // Baseball 
				case 0x968A6E9DUL: // Baseball
				case 0xF42DAB14UL: // Ice Climber
				case 0x7D6B764FUL: // Ice Climber
		
					Log::Flush( "VsSystem: error, Dual-System games are not supported" NST_LINEBREAK );
					throw RESULT_ERR_UNSUPPORTED_VSSYSTEM;
			}
		
			Context context( cpu, ppu );

			try
			{
				// Credits to the MAME people for the DIP switch info.
			
				switch (pRomCrc)
				{
					case 0xEB2DBA63UL: // TKO Boxing
					case 0x9818F656UL:
				
						context.SetDips(7);
						context.dips[0]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit", 0x01 );
						context.dips[0][2] = Dip::Value( "2 Coins / 1 Credit", 0x02 );
						context.dips[0][3] = Dip::Value( "3 Coins / 1 Credit", 0x03 );
						context.dips[1]    = Dip::Value( "unknown",            2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x04 );
						context.dips[2]    = Dip::Value( "unknown",            2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x08 );
						context.dips[3]    = Dip::Value( "unknown",            2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x10 );
						context.dips[4]    = Dip::Value( "Palette Color",      2, 1 );
						context.dips[4][0] = Dip::Value( "Black",              0x00 );
						context.dips[4][1] = Dip::Value( "White",              0x20 );
						context.dips[5]    = Dip::Value( "unknown",            2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x40 );
						context.dips[6]    = Dip::Value( "unknown",            2, 0 );
						context.dips[6][0] = Dip::Value( "Off",                0x00 );
						context.dips[6][1] = Dip::Value( "On",                 0x80 );
					
						return new VsTkoBoxing( context );
			
					case 0x135ADF7CUL: // RBI Baseball
				
						context.SetDips(4);
						context.dips[0]    = Dip::Value( "Coinage",                4, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit",     0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit",     0x01 );
						context.dips[0][2] = Dip::Value( "2 Coins / 1 Credit",     0x02 );
						context.dips[0][3] = Dip::Value( "3 Coins / 1 Credit",     0x03 );
						context.dips[1]    = Dip::Value( "Max. 1p/in, 2p/in, Min", 4, 1 );
						context.dips[1][0] = Dip::Value( "2, 1, 3",                0x04 );
						context.dips[1][1] = Dip::Value( "2, 2, 4",                0x0C );
						context.dips[1][2] = Dip::Value( "3, 2, 6",                0x00 );
						context.dips[1][3] = Dip::Value( "4, 3, 7",                0x08 );
						context.dips[2]    = Dip::Value( "Demo Sounds",            2, 1 );
						context.dips[2][0] = Dip::Value( "Off",                    0x00 );
						context.dips[2][1] = Dip::Value( "On",                     0x10 );
						context.dips[3]    = Dip::Value( "Color Palette",          5, 0 );
						context.dips[3][0] = Dip::Value( "Normal",                 0x80 );
						context.dips[3][1] = Dip::Value( "Wrong 1",                0x00 );
						context.dips[3][2] = Dip::Value( "Wrong 2",                0x40 );
						context.dips[3][3] = Dip::Value( "Wrong 3",                0x20 );
						context.dips[3][4] = Dip::Value( "Wrong 4",                0xC0 );
			
						context.securityColor = 2;
				
						return new VsRbiBaseball( context );
				
				 	case 0xED588F00UL: // Duck Hunt
					
						context.SetDips(4);
				     	context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit",  0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit",  0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	        0x07 );
						context.dips[1]    = Dip::Value( "Difficulty",          4, 1 );
						context.dips[1][0] = Dip::Value( "Easy",                0x00 );
						context.dips[1][1] = Dip::Value( "Normal",              0x08 );
						context.dips[1][2] = Dip::Value( "Hard",                0x10 );
						context.dips[1][3] = Dip::Value( "Very Hard",           0x18 );
						context.dips[2]    = Dip::Value( "Misses per Game",     2, 1 );
						context.dips[2][0] = Dip::Value( "3",                   0x00 );
						context.dips[2][1] = Dip::Value( "5",                   0x20 );
						context.dips[3]    = Dip::Value( "Bonus Life",          4, 0 );
						context.dips[3][0] = Dip::Value( "30000",               0x00 );
						context.dips[3][1] = Dip::Value( "50000",	            0x40 );
						context.dips[3][2] = Dip::Value( "80000",	            0x80 );
						context.dips[3][3] = Dip::Value( "100000",	            0xC0 );
			
						break;
				
					case 0x16D3F469UL: // Ninja Jajamaru Kun (J)
				     
						context.SetDips(5);
				    	context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit",  0x02 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credit",  0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	        0x07 );
						context.dips[1]    = Dip::Value( "Lives",               3, 0 );
						context.dips[1][0] = Dip::Value( "3",                   0x00 );
						context.dips[1][1] = Dip::Value( "4",                   0x10 );
						context.dips[1][2] = Dip::Value( "5",                   0x08 );
						context.dips[2]    = Dip::Value( "unknown",             2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                 0x00 );
						context.dips[2][1] = Dip::Value( "On",                  0x20 );
						context.dips[3]    = Dip::Value( "unknown",             2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                 0x00 );
						context.dips[3][1] = Dip::Value( "On",                  0x40 );
						context.dips[4]    = Dip::Value( "Demo Sounds",         2, 1 );
						context.dips[4][0] = Dip::Value( "Off",                 0x00 );
						context.dips[4][1] = Dip::Value( "On",                  0x80 );
			
						context.securityPpu = 0x1B;			
						break;
				
				 	case 0x8850924BUL: // Tetris
				
						context.SetDips(6);
				    	context.dips[0]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit", 0x02 );
						context.dips[0][2] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][3] = Dip::Value( "3 Coins / 1 Credit", 0x03 );
						context.dips[1]    = Dip::Value( "unknown",            2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x04 );
						context.dips[2]    = Dip::Value( "unknown",            2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x08 );
						context.dips[3]    = Dip::Value( "unknown",            2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x10 );
						context.dips[4]    = Dip::Value( "Palette Color",      3, 2 );
						context.dips[4][0] = Dip::Value( "Black",              0x40 );
						context.dips[4][1] = Dip::Value( "Green",              0x20 );
						context.dips[4][2] = Dip::Value( "Grey",               0x60 );
						context.dips[5]    = Dip::Value( "unknown",            2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x80 );
				
						context.swapPorts = true;
						break;
				
				 	case 0x8C0C2DF5UL: // Top Gun
				
						context.SetDips(5);
				     	context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit",  0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit",  0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	        0x07 );
						context.dips[1]    = Dip::Value( "Lives per Coin",      2, 0 );
						context.dips[1][0] = Dip::Value( "3 - 12 Max",          0x00 );
						context.dips[1][1] = Dip::Value( "2 - 9 Max",	        0x08 );
						context.dips[2]    = Dip::Value( "Bonus",               4, 0 );
						context.dips[2][0] = Dip::Value( "30k and every 50k",   0x00 );
						context.dips[2][1] = Dip::Value( "50k and every 100k",  0x20 );
						context.dips[2][2] = Dip::Value( "100k and every 150k", 0x10 );
						context.dips[2][3] = Dip::Value( "200k and every 200k", 0x30 );
						context.dips[3]    = Dip::Value( "Difficulty",          2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",              0x00 );
						context.dips[3][1] = Dip::Value( "Hard",                0x40 );
						context.dips[4]    = Dip::Value( "Demo Sounds",         2, 1 );
						context.dips[4][0] = Dip::Value( "Off",                 0x00 );
						context.dips[4][1] = Dip::Value( "On",                  0x80 );
			
						context.securityPpu = 0x1B;				
						break;
				
				 	case 0x70901B25UL: // Slalom
				
						context.SetDips(5);
				   		context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit",  0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit",  0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit",  0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit",  0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit",  0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit",  0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit",  0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",  	        0x07 );
						context.dips[1]    = Dip::Value( "Freestyle Points",    2, 0 );
						context.dips[1][0] = Dip::Value( "Left / Right",        0x00 );
						context.dips[1][1] = Dip::Value( "Hold Time",           0x08 );
						context.dips[2]    = Dip::Value( "Difficulty",          4, 1 );
						context.dips[2][0] = Dip::Value( "Easy",                0x00 );
						context.dips[2][1] = Dip::Value( "Normal",              0x10 );
						context.dips[2][2] = Dip::Value( "Hard",                0x20 );
						context.dips[2][3] = Dip::Value( "Hardest",             0x30 );
						context.dips[3]    = Dip::Value( "Allow Continue",      2, 1 );
						context.dips[3][0] = Dip::Value( "No",                  0x40 );
						context.dips[3][1] = Dip::Value( "Yes",                 0x00 );
						context.dips[4]    = Dip::Value( "Inverted input",      2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                 0x00 );
						context.dips[4][1] = Dip::Value( "On",                  0x80 );
			
						context.securityColor = 1;				
						break;
				
				 	case 0xCF36261EUL: // Sky Kid
				
						context.SetDips(5);
				     	context.dips[0]    = Dip::Value( "unknown",            2, 0 );
						context.dips[0][0] = Dip::Value( "Off",                0x00 );
						context.dips[0][1] = Dip::Value( "On",                 0x01 );
						context.dips[1]    = Dip::Value( "unknown",            2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x02 );
						context.dips[2]    = Dip::Value( "Lives",              2, 0 );
						context.dips[2][0] = Dip::Value( "2",                  0x00 );
						context.dips[2][1] = Dip::Value( "3",                  0x04 );
						context.dips[3]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[3][0] = Dip::Value( "1 Coins / 1 Credit", 0x00 );
						context.dips[3][1] = Dip::Value( "1 Coins / 2 Credit", 0x08 );
						context.dips[3][2] = Dip::Value( "2 Coins / 1 Credit", 0x10 );
						context.dips[3][3] = Dip::Value( "3 Coins / 1 Credit", 0x18 );
						context.dips[4]    = Dip::Value( "Color Palette",      5, 0 );
						context.dips[4][0] = Dip::Value( "Normal",             0x20 );
						context.dips[4][1] = Dip::Value( "Wrong 1",            0x00 );
						context.dips[4][2] = Dip::Value( "Wrong 2",            0x40 );
						context.dips[4][3] = Dip::Value( "Wrong 3",            0x80 );
						context.dips[4][4] = Dip::Value( "Wrong 4",            0xC0 );
				
						break;
				
				   	case 0xE1AA8214UL: // Star Luster
				
						context.SetDips(6);
				     	context.dips[0]    = Dip::Value( "Coinage",            4, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit", 0x02 );
						context.dips[0][2] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][3] = Dip::Value( "3 Coins / 1 Credit", 0x03 );
						context.dips[1]    = Dip::Value( "unknown",            2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x04 );
						context.dips[2]    = Dip::Value( "unknown",            2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x08 );
						context.dips[3]    = Dip::Value( "unknown",            2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x10 );
						context.dips[4]    = Dip::Value( "Palette Color",      3, 0 );
						context.dips[4][0] = Dip::Value( "Black",              0x40 );
						context.dips[4][1] = Dip::Value( "Green",              0x20 );
						context.dips[4][2] = Dip::Value( "Grey",               0x60 );
						context.dips[5]    = Dip::Value( "unknown",            2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                0x00 );
						context.dips[5][1] = Dip::Value( "On",                 0x80 );
				
						break;
				
				 	case 0xD5D7EAC4UL: // Dr. Mario
				
						context.SetDips(5);
				 		context.dips[0]    = Dip::Value( "Drop Rate Increases After", 4, 0 );
						context.dips[0][0] = Dip::Value( "7 Pills",                   0x00 );
						context.dips[0][1] = Dip::Value( "8 Pills",                   0x01 );
						context.dips[0][2] = Dip::Value( "9 Pills",                   0x02 );
						context.dips[0][3] = Dip::Value( "10 Pills",                  0x03 );
						context.dips[1]    = Dip::Value( "Virus Level",               4, 0 );
						context.dips[1][0] = Dip::Value( "1",                         0x00 );
						context.dips[1][1] = Dip::Value( "3",                         0x04 );
						context.dips[1][2] = Dip::Value( "5",                         0x08 );
						context.dips[1][3] = Dip::Value( "7",                         0x0C );
						context.dips[2]    = Dip::Value( "Drop Speed Up",             4, 0 );
						context.dips[2][0] = Dip::Value( "Slow",                      0x00 );
						context.dips[2][1] = Dip::Value( "Medium",                    0x10 );
						context.dips[2][2] = Dip::Value( "Fast",                      0x20 );
						context.dips[2][3] = Dip::Value( "Fastest",                   0x30 );
						context.dips[3]    = Dip::Value( "Free Play",                 2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                       0x00 );
						context.dips[3][1] = Dip::Value( "On",                        0x40 );
						context.dips[4]    = Dip::Value( "Demo Sounds",               2, 1 );
						context.dips[4][0] = Dip::Value( "Off",                       0x00 );
						context.dips[4][1] = Dip::Value( "On",                        0x80 );
			
						context.securityColor = 2;
						break;
				
				 	case 0xFFBEF374UL: // Castlevania
				
						context.SetDips(4);
				 		context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit", 0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credit", 0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	       0x07 );
						context.dips[1]    = Dip::Value( "Lives",              2, 1 );
						context.dips[1][0] = Dip::Value( "2",                  0x08 );
						context.dips[1][1] = Dip::Value( "3",	               0x00 );
						context.dips[2]    = Dip::Value( "Bonus",              4, 0 );
						context.dips[2][0] = Dip::Value( "100k",               0x00 );
						context.dips[2][1] = Dip::Value( "200k",               0x20 );
						context.dips[2][2] = Dip::Value( "300k",               0x10 );
						context.dips[2][3] = Dip::Value( "400k",               0x30 );
						context.dips[3]    = Dip::Value( "Difficulty",         2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",             0x00 );
						context.dips[3][1] = Dip::Value( "Hard",               0x40 );
			
						context.securityColor = 1;
						context.swapPorts = true;
						break;
				
				   	case 0xE2C0A2BEUL: // Platoon
				
						context.SetDips(6);
				    	context.dips[0]    = Dip::Value( "unknown",            2, 0 );
						context.dips[0][0] = Dip::Value( "Off",                0x00 );
						context.dips[0][1] = Dip::Value( "On",                 0x01 );
						context.dips[1]    = Dip::Value( "unknown",            2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x02 );
						context.dips[2]    = Dip::Value( "Demo Sounds",        2, 1 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x04 );
						context.dips[3]    = Dip::Value( "unknown",            2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                0x00 );
						context.dips[3][1] = Dip::Value( "On",                 0x08 );
						context.dips[4]    = Dip::Value( "unknown",            2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x10 );
						context.dips[5]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[5][0] = Dip::Value( "1 Coins / 1 Credit", 0x00 );
						context.dips[5][1] = Dip::Value( "1 Coins / 2 Credit", 0x20 );
						context.dips[5][2] = Dip::Value( "1 Coins / 3 Credit", 0x40 );
						context.dips[5][3] = Dip::Value( "2 Coins / 1 Credit", 0x60 );
						context.dips[5][4] = Dip::Value( "3 Coins / 1 Credit", 0x80 );
						context.dips[5][5] = Dip::Value( "4 Coins / 1 Credit", 0xA0 );
						context.dips[5][6] = Dip::Value( "5 Coins / 1 Credit", 0xC0 );
						context.dips[5][7] = Dip::Value( "Free Play",	       0xE0 );
			
						context.securityColor = 0;
						context.swapPorts = true;
						break;
				
					case 0xCBE85490UL: // Excitebike
				 	case 0x29155E0CUL: // Excitebike (alt)
				
						context.SetDips(4);
				    	context.dips[0]    = Dip::Value( "Coinage",                  8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit",       0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit",       0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit",       0x02 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credit",       0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit",       0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit",       0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit",       0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	             0x07 );
						context.dips[1]    = Dip::Value( "Bonus",                    4, 0 );
						context.dips[1][0] = Dip::Value( "100k and Every 50k",       0x00 );
						context.dips[1][1] = Dip::Value( "Every 100k",               0x10 );
						context.dips[1][2] = Dip::Value( "100k Only",                0x08 );
						context.dips[1][3] = Dip::Value( "None",                     0x18 );
						context.dips[2]    = Dip::Value( "1st Half Qualifying Time", 2, 0 );
						context.dips[2][0] = Dip::Value( "Normal",                   0x00 );
						context.dips[2][1] = Dip::Value( "Hard",                     0x20 );
						context.dips[3]    = Dip::Value( "2nd Half Qualifying Time", 2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",                   0x00 );
						context.dips[3][1] = Dip::Value( "Hard",                     0x40 );
			
						context.securityColor = (pRomCrc == 0x29155E0CUL ? 3 : 2);
						context.swapPorts = true;
						break;
				
					case 0x07138C06UL: // Clu Clu Land
				
						context.SetDips(5);
				     	context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credit", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	       0x07 );
						context.dips[1]    = Dip::Value( "unknown",            2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x08 );
						context.dips[2]    = Dip::Value( "unknown",            2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x10 );
						context.dips[3]    = Dip::Value( "Lives",              4, 1 );
						context.dips[3][0] = Dip::Value( "2",                  0x60 );
						context.dips[3][1] = Dip::Value( "3",	               0x00 );
						context.dips[3][2] = Dip::Value( "4",                  0x40 );
						context.dips[3][3] = Dip::Value( "5",                  0x20 );
						context.dips[4]    = Dip::Value( "unknown",            2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                0x00 );
						context.dips[4][1] = Dip::Value( "On",                 0x80 );
			
						context.securityColor = 3;
						context.swapPorts = true;
						break;
				
					case 0x43A357EFUL: // Ice Climber
				
						context.SetDips(4);
				      	context.dips[0]    = Dip::Value( "Coinage",              8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit",   0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit",   0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit",   0x02 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credit",   0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit",   0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit",   0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit",   0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	         0x07 );
						context.dips[1]    = Dip::Value( "Lives",                4, 0 );
						context.dips[1][0] = Dip::Value( "3",                    0x00 );
						context.dips[1][1] = Dip::Value( "4",	                 0x10 );
						context.dips[1][2] = Dip::Value( "5",                    0x08 );
						context.dips[1][3] = Dip::Value( "7",	                 0x18 );
						context.dips[2]    = Dip::Value( "Difficulty",           2, 0 );
						context.dips[2][0] = Dip::Value( "Normal",               0x00 );
						context.dips[2][1] = Dip::Value( "Hard",                 0x20 );
						context.dips[3]    = Dip::Value( "Time before the bear", 2, 0 );
						context.dips[3][0] = Dip::Value( "Long",                 0x00 );
						context.dips[3][1] = Dip::Value( "Short",                0x40 );
			
						context.securityColor = 3;
						context.swapPorts = true;
						break;
				
					case 0x737DD1BFUL: // Super Mario Bros	
					case 0x4BF3972DUL: 
					case 0x8B60CC58UL:
					case 0x8192C804UL: 
				
						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit", 0x06 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit", 0x01 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credit", 0x05 );
						context.dips[0][4] = Dip::Value( "1 Coins / 5 Credit", 0x03 );
						context.dips[0][5] = Dip::Value( "2 Coins / 1 Credit", 0x04 );
						context.dips[0][6] = Dip::Value( "3 Coins / 1 Credit", 0x02 );
						context.dips[0][7] = Dip::Value( "Free Play",	       0x07 );
						context.dips[1]    = Dip::Value( "Lives",              2, 1 );
						context.dips[1][0] = Dip::Value( "2",                  0x08 );
						context.dips[1][1] = Dip::Value( "3",	               0x00 );
						context.dips[2]    = Dip::Value( "Bonus Life",         4, 0 );
						context.dips[2][0] = Dip::Value( "100",                0x00 );
						context.dips[2][1] = Dip::Value( "150",	               0x20 );
						context.dips[2][2] = Dip::Value( "200",	               0x10 );
						context.dips[2][3] = Dip::Value( "250",	               0x30 );
						context.dips[3]    = Dip::Value( "Timer",              2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",             0x00 );
						context.dips[3][1] = Dip::Value( "Fast",	           0x40 );
						context.dips[4]    = Dip::Value( "Continue Lives",     2, 0 );
						context.dips[4][0] = Dip::Value( "3",                  0x80 );
						context.dips[4][1] = Dip::Value( "4",	               0x00 );
			
						context.securityColor = 3;
						break;
				
					case 0xEC461DB9UL: // Pinball
					case 0xE528F651UL: // Pinball (alt)
				
						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",            8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credit", 0x01 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credit", 0x06 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credit", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credit", 0x04 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credit", 0x05 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credit", 0x03 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credit", 0x07 );
						context.dips[0][7] = Dip::Value( "Free Play",          0x00 );
						context.dips[1]    = Dip::Value( "unknown",            2, 0 );
						context.dips[1][0] = Dip::Value( "Off",                0x00 );
						context.dips[1][1] = Dip::Value( "On",                 0x08 );
						context.dips[2]    = Dip::Value( "unknown",            2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                0x00 );
						context.dips[2][1] = Dip::Value( "On",                 0x10 );
						context.dips[3]    = Dip::Value( "Balls",              4, 1 );
						context.dips[3][0] = Dip::Value( "2",                  0x60 );
						context.dips[3][1] = Dip::Value( "3",                  0x00 );
						context.dips[3][2] = Dip::Value( "4",                  0x40 );
						context.dips[3][3] = Dip::Value( "5",                  0x20 );
						context.dips[4]    = Dip::Value( "Ball Speed",         2, 0 );
						context.dips[4][0] = Dip::Value( "Normal",             0x00 );
						context.dips[4][1] = Dip::Value( "Fast",               0x80 );
			
						context.securityColor = (pRomCrc == 0xEC461DB9UL ? 0 : 3);
						break;
				
					case 0x0B65A917UL: // Mach Rider
					case 0x8A6A9848UL:
				
						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credits", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credits", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credits", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credits", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	        0x07 );
						context.dips[1]    = Dip::Value( "Time",				4, 0 );
						context.dips[1][0] = Dip::Value( "280",             	0x00 );
						context.dips[1][1] = Dip::Value( "250",	                0x10 );
						context.dips[1][2] = Dip::Value( "220",	                0x08 );
						context.dips[1][3] = Dip::Value( "200",	                0x18 );
						context.dips[2]    = Dip::Value( "unknown",             2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                 0x00 );
						context.dips[2][1] = Dip::Value( "On",                  0x20 );
						context.dips[3]    = Dip::Value( "unknown",             2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                 0x00 );
						context.dips[3][1] = Dip::Value( "On",                  0x40 );
						context.dips[4]    = Dip::Value( "unknown",             2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                 0x00 );
						context.dips[4][1] = Dip::Value( "On",                  0x80 );
			
						context.securityColor = 1;
						break;
				
					case 0x46914E3EUL: // Soccer
				
						context.SetDips(3);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credits", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credits", 0x01 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credits", 0x05 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credits", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	        0x07 );
						context.dips[1]    = Dip::Value( "Points Timer",        4, 2 );
						context.dips[1][0] = Dip::Value( "600 Pts",             0x00 );
						context.dips[1][1] = Dip::Value( "800 Pts",             0x10 );
						context.dips[1][2] = Dip::Value( "1000 Pts",            0x08 );
						context.dips[1][3] = Dip::Value( "1200 Pts",            0x18 );
						context.dips[2]    = Dip::Value( "Difficulty",          4, 1 );
						context.dips[2][0] = Dip::Value( "Easy",                0x00 );
						context.dips[2][1] = Dip::Value( "Normal",              0x40 );
						context.dips[2][2] = Dip::Value( "Hard",                0x20 );
						context.dips[2][3] = Dip::Value( "Very Hard",           0x60 );
																			  
						context.securityColor = 2;
						break;
				
					case 0x70433F2CUL: // Battle City
					case 0x8D15A6E6UL: // bad .nes
				
						context.SetDips(7);
						context.dips[0]    = Dip::Value( "Credits for 2 Players", 2, 1 );
						context.dips[0][0] = Dip::Value( "1",                     0x00 );
						context.dips[0][1] = Dip::Value( "2",                     0x01 );
						context.dips[1]    = Dip::Value( "Lives",                 2, 0 );
						context.dips[1][0] = Dip::Value( "3",                     0x00 );
						context.dips[1][1] = Dip::Value( "5",                     0x02 );
						context.dips[2]    = Dip::Value( "Demo Sounds",           2, 1 );
						context.dips[2][0] = Dip::Value( "Off",                   0x00 );
						context.dips[2][1] = Dip::Value( "On",                    0x04 );
						context.dips[3]    = Dip::Value( "unknown",               2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                   0x00 );
						context.dips[3][1] = Dip::Value( "On",                    0x08 );
						context.dips[4]    = Dip::Value( "unknown",               2, 0 );
						context.dips[4][0] = Dip::Value( "Off",                   0x00 );
						context.dips[4][1] = Dip::Value( "On",                    0x10 );
						context.dips[5]    = Dip::Value( "unknown",               2, 0 );
						context.dips[5][0] = Dip::Value( "Off",                   0x00 );
						context.dips[5][1] = Dip::Value( "On",                    0x20 );
						context.dips[6]    = Dip::Value( "Color Palette",         4, 0 );
						context.dips[6][0] = Dip::Value( "Normal",                0x80 );
						context.dips[6][1] = Dip::Value( "Wrong 1",               0x00 );
						context.dips[6][2] = Dip::Value( "Wrong 2",               0x40 );
						context.dips[6][3] = Dip::Value( "Wrong 3",               0xC0 );
			
						context.securityColor = 2;
						break;
				
					case 0xD99A2087UL: // Gradius
				
						context.SetDips(5);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credits", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credits", 0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credits", 0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credits", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",           0x07 );
						context.dips[1]    = Dip::Value( "Lives",               2, 0 );
						context.dips[1][0] = Dip::Value( "3",                   0x08 );
						context.dips[1][1] = Dip::Value( "4",                   0x00 );
						context.dips[2]    = Dip::Value( "Bonus",               4, 0 );
						context.dips[2][0] = Dip::Value( "100k",                0x00 );
						context.dips[2][1] = Dip::Value( "200k",	            0x20 );
						context.dips[2][2] = Dip::Value( "300k",	            0x10 );
						context.dips[2][3] = Dip::Value( "400k",	            0x30 );
						context.dips[3]    = Dip::Value( "Difficulty",          2, 0 );
						context.dips[3][0] = Dip::Value( "Normal",              0x00 );
						context.dips[3][1] = Dip::Value( "Hard",                0x40 );
						context.dips[4]    = Dip::Value( "Demo Sounds",         2, 1 );
						context.dips[4][0] = Dip::Value( "Off",                 0x00 );
						context.dips[4][1] = Dip::Value( "On",                  0x80 );
			
						context.securityColor = 0;
						break;
				
					case 0x1E438D52UL: // Goonies
				
						context.SetDips(6);
						context.dips[0]    = Dip::Value( "Coinage",             8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credits", 0x00 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credits", 0x04 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credits", 0x02 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "3 Coins / 1 Credits", 0x01 );
						context.dips[0][5] = Dip::Value( "4 Coins / 1 Credits", 0x05 );
						context.dips[0][6] = Dip::Value( "5 Coins / 1 Credits", 0x03 );
						context.dips[0][7] = Dip::Value( "Free Play",	        0x07 );
						context.dips[1]    = Dip::Value( "Lives",               2, 0 );
						context.dips[1][0] = Dip::Value( "3",                   0x00 );
						context.dips[1][1] = Dip::Value( "2",                   0x08 );
						context.dips[2]    = Dip::Value( "unknown",             2, 0 );
						context.dips[2][0] = Dip::Value( "Off",                 0x00 );
						context.dips[2][1] = Dip::Value( "On",                  0x10 );
						context.dips[3]    = Dip::Value( "unknown",             2, 0 );
						context.dips[3][0] = Dip::Value( "Off",                 0x00 );
						context.dips[3][1] = Dip::Value( "On",                  0x20 );
						context.dips[4]    = Dip::Value( "Timer",               2, 0 );
						context.dips[4][0] = Dip::Value( "Normal",              0x00 );
						context.dips[4][1] = Dip::Value( "Fast",                0x40 );
						context.dips[5]    = Dip::Value( "Demo Sounds",         2, 1 );
						context.dips[5][0] = Dip::Value( "Off",                 0x00 );
						context.dips[5][1] = Dip::Value( "On",                  0x80 );
			
						context.securityColor = 2;
						break;
				
					case 0xFF5135A3UL: // Hogan's Alley
				
						context.SetDips(4);
						context.dips[0]    = Dip::Value( "Coinage",             8, 4 );
						context.dips[0][0] = Dip::Value( "5 Coins / 1 Credits", 0x03 );
						context.dips[0][1] = Dip::Value( "4 Coins / 1 Credits", 0x05 );
						context.dips[0][2] = Dip::Value( "3 Coins / 1 Credits", 0x01 );
						context.dips[0][3] = Dip::Value( "2 Coins / 1 Credits", 0x06 );
						context.dips[0][4] = Dip::Value( "1 Coins / 1 Credits", 0x00 );
						context.dips[0][5] = Dip::Value( "1 Coins / 2 Credits", 0x04 );
						context.dips[0][6] = Dip::Value( "1 Coins / 3 Credits", 0x02 );
						context.dips[0][7] = Dip::Value( "Free Play",	        0x07 );
						context.dips[1]    = Dip::Value( "Difficulty",          4, 1 );
						context.dips[1][0] = Dip::Value( "Easy",                0x00 );
						context.dips[1][1] = Dip::Value( "Normal",              0x08 );
						context.dips[1][2] = Dip::Value( "Hard",                0x10 );
						context.dips[1][3] = Dip::Value( "Very Hard",           0x18 );
						context.dips[2]    = Dip::Value( "Misses per Game",     2, 1 );
						context.dips[2][0] = Dip::Value( "3",                   0x00 );
						context.dips[2][1] = Dip::Value( "5",                   0x20 );
						context.dips[3]    = Dip::Value( "Bonus Life",          4, 0 );
						context.dips[3][0] = Dip::Value( "30000",               0x00 );
						context.dips[3][1] = Dip::Value( "50000",	            0x40 );
						context.dips[3][2] = Dip::Value( "80000",	            0x80 );
						context.dips[3][3] = Dip::Value( "100000",	            0xC0 );
			
						context.securityColor = 0;				
						break;
				
					case 0x17AE56BEUL: // Freedom Force

						context.securityColor = 0;
						break;
				
					case 0xF9D3B0A3UL: // Super Xevious
					case 0x66BB838FUL: // Super Xevious
				
						context.SetDips(8);
			
						for (uint i=0; i < 8; ++i)
						{
							context.dips[i]	   = Dip::Value( "unknown", 2, 0    );
							context.dips[i][0] = Dip::Value( "off",     0x00    );
							context.dips[i][1] = Dip::Value( "on",      1U << i );
						}
				
						context.securityColor = 0;

						return new VsSuperXevious( context );
				
					case 0xCC2C4B5DUL: // Golf
					case 0x86167220UL: // Lady Golf
				
						context.SetDips(5);
				     	context.dips[0]    = Dip::Value( "Coinage",                 8, 0 );
						context.dips[0][0] = Dip::Value( "1 Coins / 1 Credits",     0x01 );
						context.dips[0][1] = Dip::Value( "1 Coins / 2 Credits",     0x06 );
						context.dips[0][2] = Dip::Value( "1 Coins / 3 Credits",     0x02 );
						context.dips[0][3] = Dip::Value( "1 Coins / 4 Credits",     0x04 );
						context.dips[0][4] = Dip::Value( "2 Coins / 1 Credits",     0x05 );
						context.dips[0][5] = Dip::Value( "3 Coins / 1 Credits",     0x03 );
						context.dips[0][6] = Dip::Value( "4 Coins / 1 Credits",     0x07 );
						context.dips[0][7] = Dip::Value( "Free Play",	            0x00 );
						context.dips[1]    = Dip::Value( "Hole Size",               2, 0 );
						context.dips[1][0] = Dip::Value( "Large",                   0x00 );
						context.dips[1][1] = Dip::Value( "Small",                   0x08 );
						context.dips[2]    = Dip::Value( "Points per Stroke",       2, 0 );
						context.dips[2][0] = Dip::Value( "Easier",                  0x00 );
						context.dips[2][1] = Dip::Value( "Harder",                  0x10 );
						context.dips[3]    = Dip::Value( "Starting Points",         4, 0 );
						context.dips[3][0] = Dip::Value( "10",                      0x00 );
						context.dips[3][1] = Dip::Value( "13",                      0x40 );
						context.dips[3][2] = Dip::Value( "16",                      0x20 );
						context.dips[3][3] = Dip::Value( "20",                      0x60 );
						context.dips[4]    = Dip::Value( "Difficulty Vs. Computer", 2, 0 );
						context.dips[4][0] = Dip::Value( "Easy",                    0x00 );
						context.dips[4][1] = Dip::Value( "Hard",                    0x80 );
			
						context.securityColor = 1;				
						context.swapPorts = true;
						break;
				
					case 0xB90497AAUL: // Tennis
				
						context.SetDips(6);
				    	context.dips[0]    = Dip::Value( "Difficulty Vs. Computer", 4, 1 );
						context.dips[0][0] = Dip::Value( "Easy",                    0x00 );
						context.dips[0][1] = Dip::Value( "Normal",                  0x02 );
						context.dips[0][2] = Dip::Value( "Hard",                    0x01 );
						context.dips[0][3] = Dip::Value( "Very Hard",               0x03 );
						context.dips[1]    = Dip::Value( "Difficulty Vs. Player",   4, 1 );
						context.dips[1][0] = Dip::Value( "Easy",                    0x00 );
						context.dips[1][1] = Dip::Value( "Normal",                  0x08 );
						context.dips[1][2] = Dip::Value( "Hard",                    0x04 );
						context.dips[1][3] = Dip::Value( "Very Hard",               0x0C );
						context.dips[2]    = Dip::Value( "Racket Size",             2, 0 );
						context.dips[2][0] = Dip::Value( "Large",                   0x00 );
						context.dips[2][1] = Dip::Value( "Small",                   0x10 );
						context.dips[3]    = Dip::Value( "Extra Score",             2, 0 );
						context.dips[3][0] = Dip::Value( "1 Set",                   0x00 );
						context.dips[3][1] = Dip::Value( "1 Game",                  0x20 );
						context.dips[4]    = Dip::Value( "Court Color",             2, 0 );
						context.dips[4][0] = Dip::Value( "Green",                   0x00 );
						context.dips[4][1] = Dip::Value( "Blue",                    0x40 );
						context.dips[5]    = Dip::Value( "Copyright",               2, 0 );
						context.dips[5][0] = Dip::Value( "Japan",                   0x00 );
						context.dips[5][1] = Dip::Value( "USA",                     0x80 );
						break;

					case 0xCA85E56DUL: // Mighty Bomb Jack

						context.securityPpu = 0x3D;
						break;
				}
			
				if (context.dips == NULL)
				{
					context.SetDips(8);

					for (uint i=0; i < 8; ++i)
					{
						context.dips[i]	   = Dip::Value( "unknown", 2, 0    );
						context.dips[i][0] = Dip::Value( "off",     0x00    );
						context.dips[i][1] = Dip::Value( "on",      1U << i );
					}
				}
			}
			catch (...)
			{
				delete [] context.dips;
				throw;
			}

			return new VsSystem( context );
		}
	
		void VsSystem::Destroy(VsSystem*& vs)
		{
			delete vs;
			vs = NULL;
		}

		VsSystem::VsSystem(Context& context)
		: 
		cpu           (context.cpu), 
		ppu           (context.ppu), 
		securityColor (context.securityColor < 4 ? colorMaps[context.securityColor] : NULL),
		dips          (context.dips,context.numDips),
		securityPpu   (context.securityPpu),
		swapPorts     (context.swapPorts)
		{
			if (securityColor)
				Log::Flush( "VsSystem: scrambled color table present" NST_LINEBREAK );
	
			if (securityPpu)
				Log::Flush( "VsSystem: ppu security check present" NST_LINEBREAK );

			if (swapPorts)
				Log::Flush( "VsSystem: controller I/O ports swapped" NST_LINEBREAK );
		}

		VsSystem::~VsSystem()
		{
		}

		void VsSystem::Reset(bool)
		{
			if (securityColor)
			{
				p2007 = cpu.Map( 0x2007U );
	
				for (uint i=0x2000U; i < 0x4000U; i += 0x8)
					cpu.Map( i + 0x7 ).Set( this, &VsSystem::Peek_2007, &VsSystem::Poke_2007 );
			}
	
			if (securityPpu)
			{
				const Io::Port p2000( cpu.Map( 0x2000U ) );
				const Io::Port p2001( cpu.Map( 0x2001U ) );
	
				p2002 = cpu.Map( 0x2002U );
	
				for (uint i=0x2000U; i < 0x4000U; i += 0x8)
				{
					cpu.Map( i + 0x0 ) = p2001;
					cpu.Map( i + 0x1 ) = p2000;
					cpu.Map( i + 0x2 ).Set( this, &VsSystem::Peek_2002, &VsSystem::Poke_2002 );
				}
			}
	
			dips.Reset();

			coin = 0;
			
			p4016 = cpu.Map( 0x4016U );
			p4017 = cpu.Map( 0x4017U );
	
			cpu.Map( 0x4016U ).Set( this, swapPorts ? &VsSystem::Peek_4016_Swap : &VsSystem::Peek_4016, &VsSystem::Poke_4016 );
			cpu.Map( 0x4017U ).Set( this, swapPorts ? &VsSystem::Peek_4017_Swap : &VsSystem::Peek_4017, &VsSystem::Poke_4017 );
			
			cpu.Map( 0x5000U, 0x5FFFU ).Set( this, &VsSystem::Peek_Nop, &VsSystem::Poke_Nop );
	
			Reset();
		}
	
		void VsSystem::SaveState(State::Saver& state) const
		{
			state.Write8( coin );
			SubSave( state );
		}
	
		void VsSystem::LoadState(State::Loader& state)
		{
			coin = state.Read8();
	
			while (const dword id = state.Begin())
			{
				SubLoad( state, id );
				state.End();
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_PEEK(VsSystem,Nop)
		{
			return 0x50;
		}

		NES_POKE(VsSystem,Nop)
		{
		}
	
		NES_PEEK(VsSystem,2002)
		{
			return (p2002.Peek( 0x2002 ) & 0xC0) | securityPpu;
		}
	
		NES_POKE(VsSystem,2002)
		{
			p2002.Poke( address, data );
		}
	
		NES_PEEK(VsSystem,2007)
		{
			return p2007.Peek( address );
		}
	
		NES_POKE(VsSystem,2007)
		{
			ppu.Update();
	
			if ((ppu.GetVRamAddress() & 0x3F00) == 0x3F00)
			{
				data &= 0x3F;
	
				if (securityColor[data] != 0xFF)
					data = securityColor[data];
			}
	
			p2007.Poke( 0x2007, data );
		}
	
		NES_PEEK(VsSystem,4016)
		{
			return (p4016.Peek( 0x4016 ) & ~uint(STATUS_4016_MASK)) | dips.Reg(0);
		}

		NES_PEEK(VsSystem,4016_Swap)
		{
			return (p4017.Peek( 0x4017 ) & ~uint(STATUS_4016_MASK)) | dips.Reg(0);
		}

		NES_POKE(VsSystem,4016)
		{
			p4016.Poke( address, data );
		}
	
		NES_PEEK(VsSystem,4017)
		{
			return (p4017.Peek( 0x4017 ) & ~uint(STATUS_4017_MASK)) | dips.Reg(1);
		}

		NES_PEEK(VsSystem,4017_Swap)
		{
			return (p4016.Peek( 0x4016 ) & ~uint(STATUS_4017_MASK)) | dips.Reg(1);
		}

		NES_POKE(VsSystem,4017)
		{
			p4017.Poke( address, data );
		}
	
		NES_PEEK(VsSystem,4020)
		{
			return coin;
		}
	
		NES_POKE(VsSystem,4020)
		{
			coin = data;
		}
	}
}
