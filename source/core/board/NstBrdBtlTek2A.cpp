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

#include "../NstMapper.hpp"
#include "../NstClock.hpp"
#include "NstBrdBtlTek2A.hpp"
	 
namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			uint BtlTek2A::GetDefaultDipSwitch(dword pRomCrc)
			{
				switch (pRomCrc)
				{
             		case 0x734B8FD2UL: // Shin Samurai Spirits 2
					case 0xA242E696UL: // Power Rangers 3
					case 0xB8B2596AUL: // Power Rangers 4
		
						return DIPSWITCH_MIRROR;
				}
		
				return 0;
			}
		
			BtlTek2A::Regs::Regs()
			{
				Reset();
			}
		
			BtlTek2A::Banks::Banks()
			{
				Reset();
			}
		
			BtlTek2A::BtlTek2A(Context& c,const DefaultDipSwitch d)
			: 
			Mapper    (c,WRAM_NONE),
			irq       (c.cpu,c.ppu,0,Clock::A12<Irq>::IRQ_DELAY),
			dipswitch 
			(
             	d == DEFAULT_DIPSWITCH_DETECT ? GetDefaultDipSwitch(c.pRomCrc) : 
             	d == DEFAULT_DIPSWITCH_EXT_MIRRORING ? DIPSWITCH_MIRROR : 0
			)
			{}
		
			void BtlTek2A::Regs::Reset()
			{
				mul[0] = 0;
				mul[1] = 0;
				tmp = 0;
				banking = 0;
				mirroring = 0;
				name = 0;
			}
		
			void BtlTek2A::Banks::Reset()
			{
				for (uint i=0; i < 4; ++i)
					prg[i] = 0;
		
				for (uint i=0; i < 8; ++i)
					chr[i] = 0;
		
				for (uint i=0; i < 4; ++i)
					name[i] = 0;
		
				secondary[0] = 0xFFFFU;
				secondary[1] = 0x0000U;
		
				prg6 = NULL;
			}
		
			void BtlTek2A::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					enabled = false;
					mode = 0;
					prescaler = 0;
					scale = 0xFF;
					count = 0;
					flip = 0;
				}
			}
		
			void BtlTek2A::SubReset(const bool hard)
			{
				irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

				for (uint i=0x5000U; i < 0x5800U; i += 0x4)
					Map( i, &BtlTek2A::Peek_5000 );
		
				for (uint i=0x5800U; i < 0x6000U; i += 0x4)
				{
					cpu.Map( i + 0x0 ).Set( &regs, &BtlTek2A::Regs::Peek_5800, &BtlTek2A::Regs::Poke_5800 );
					cpu.Map( i + 0x1 ).Set( &regs, &BtlTek2A::Regs::Peek_5801, &BtlTek2A::Regs::Poke_5801 );
					cpu.Map( i + 0x3 ).Set( &regs, &BtlTek2A::Regs::Peek_5803, &BtlTek2A::Regs::Poke_5803 );
				}
		
				Map( 0x6000U, 0x7FFFU, &BtlTek2A::Peek_6000 );
				Map( 0x8000U, 0x8FFFU, &BtlTek2A::Poke_8000 );
				Map( 0x9000U, 0x9FFFU, &BtlTek2A::Poke_9000 );
				Map( 0xA000U, 0xAFFFU, &BtlTek2A::Poke_A000 );
		
				for (uint i=0x0000U; i < 0x1000U; i += 0x8)
				{
					Map( 0xB000U + i, 0xB003U + i, &BtlTek2A::Poke_B000 );
					Map( 0xB004U + i, 0xB007U + i, &BtlTek2A::Poke_B004 );
		
					Map( 0xC000U + i, &BtlTek2A::Poke_C000 );
					Map( 0xC001U + i, &BtlTek2A::Poke_C001 );
					Map( 0xC002U + i, &BtlTek2A::Poke_C002 );
					Map( 0xC003U + i, &BtlTek2A::Poke_C003 );
					Map( 0xC004U + i, &BtlTek2A::Poke_C004 );
					Map( 0xC005U + i, &BtlTek2A::Poke_C005 );
					Map( 0xC006U + i, &BtlTek2A::Poke_C006 );
				}
		
				for (uint i=0x0000U; i < 0x1000U; i += 0x4)
				{
					Map( 0xD000U + i, &BtlTek2A::Poke_D000 );
					Map( 0xD001U + i, &BtlTek2A::Poke_D001 );
					Map( 0xD002U + i, &BtlTek2A::Poke_D002 );
					Map( 0xD003U + i, &BtlTek2A::Poke_D003 );
				}
		
				if (hard)
				{
					regs.Reset();
					banks.Reset();			
				}
				
				UpdatePrg();
				UpdateChr();
				UpdateName();
			}
		
			void BtlTek2A::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
     	 				case NES_STATE_CHUNK_ID('R','E','G','\0'):
						{
							const State::Loader::Data<38> data( state );
		
							regs.banking       = data[0];
							regs.mirroring     = data[1];
							regs.name	       = data[2];
							regs.mul[0]	       = data[3];
							regs.mul[1]	       = data[4];
							regs.tmp		   = data[5];
							banks.prg[0]	   = data[6];
							banks.prg[1]	   = data[7];
							banks.prg[2]	   = data[8];
							banks.prg[3]	   = data[9];
							banks.chr[0]	   = data[10] | (data[11] << 8);
							banks.chr[1]	   = data[12] | (data[13] << 8);
							banks.chr[2]	   = data[14] | (data[15] << 8);
							banks.chr[3]	   = data[16] | (data[17] << 8);
							banks.chr[4]	   = data[18] | (data[19] << 8);
							banks.chr[5]	   = data[20] | (data[21] << 8);
							banks.chr[6]	   = data[22] | (data[23] << 8);
							banks.chr[7]	   = data[24] | (data[25] << 8);
							banks.name[0]	   = data[26] | (data[27] << 8);
							banks.name[1]	   = data[28] | (data[29] << 8);
							banks.name[2]	   = data[30] | (data[31] << 8);
							banks.name[3]	   = data[32] | (data[33] << 8);
							banks.secondary[0] = data[34] | (data[35] << 8);
							banks.secondary[1] = data[36] | (data[37] << 8);
		
							UpdatePrg();
		
							break;
						}
		
						case NES_STATE_CHUNK_ID('I','R','Q','\0'):
						{
							u8 data[5];
							state.Read( data );
		
							irq.unit.enabled   = data[0] & 0x1;
							irq.unit.mode      = data[1];
							irq.unit.prescaler = data[2];
							irq.unit.count     = data[3];
							irq.unit.flip      = data[4];
		
							irq.unit.scale = (irq.unit.mode & Irq::MODE_SCALE_3BIT) ? 0x7 : 0xFF;					
							irq.EnableLine( irq.unit.IsEnabled() );
		
							break;
						}
					}
		
					state.End();
				}
			}
		
			void BtlTek2A::SaveState(State::Saver& state) const
			{
				{
					const u8 data[38] =
					{						
						regs.banking,
						regs.mirroring,
						regs.name,
						regs.mul[0],
						regs.mul[1],
						regs.tmp,
						banks.prg[0],
						banks.prg[1],
						banks.prg[2],
						banks.prg[3],
						banks.chr[0] & 0xFF,
						banks.chr[0] >> 8,
						banks.chr[1] & 0xFF,
						banks.chr[1] >> 8,
						banks.chr[2] & 0xFF,
						banks.chr[2] >> 8,
						banks.chr[3] & 0xFF,
						banks.chr[3] >> 8,
						banks.chr[4] & 0xFF,
						banks.chr[4] >> 8,
						banks.chr[5] & 0xFF,
						banks.chr[5] >> 8,
						banks.chr[6] & 0xFF,
						banks.chr[6] >> 8,
						banks.chr[7] & 0xFF,
						banks.chr[7] >> 8,
						banks.name[0] & 0xFF,
						banks.name[0] >> 8,
						banks.name[1] & 0xFF,
						banks.name[1] >> 8,
						banks.name[2] & 0xFF,
						banks.name[2] >> 8,
						banks.name[3] & 0xFF,
						banks.name[3] >> 8,
						banks.secondary[0] & 0xFF,
						banks.secondary[0] >> 8,
						banks.secondary[1] & 0xFF,
						banks.secondary[1] >> 8
					};
		
					state.Begin('R','E','G','\0').Write( data ).End();
				}

				{
					const u8 data[5] =
					{
						irq.unit.enabled != 0,
						irq.unit.mode,
						irq.unit.prescaler,
						irq.unit.count,
						irq.unit.flip
					};
		
					state.Begin('I','R','Q','\0').Write( data ).End();
				}
			}
		
			cstring BtlTek2A::GetDipSwitchName(const uint i) const
			{ 
				return i == 0 ? "Game Selection" : i == 1 ? "Extended Mirroring" : NULL;
			}
		
			cstring BtlTek2A::GetDipSwitchValueName(const uint i,const uint j) const
			{
             	if (i == 0) 
					return j == 0 ? "1" : j == 1 ? "2" : j == 2 ? "3" : j == 3 ? "4" : NULL;
		
				if (i == 1)
              		return j == 0 ? "off" : j == 1 ? "on" : NULL;
					
				return NULL;
			}
		
			int BtlTek2A::GetDipSwitchValue(const uint i) const
			{
				if (i == 0)
					return dipswitch >> 6;
		
				if (i == 1)
              		return dipswitch & DIPSWITCH_MIRROR;
		
				return -1;
			}
		
			Result BtlTek2A::SetDipSwitchValue(const uint i,const uint j)
			{
             	if (i == 0)
				{
					if (j < 4)
					{
						dipswitch = (dipswitch & ~uint(DIPSWITCH_GAME)) | (j << 6);
						return RESULT_OK;
					}
				}
				else if (i == 1)
				{
					if (j < 2)
					{
						dipswitch = (dipswitch & ~uint(DIPSWITCH_MIRROR) | j);
						return RESULT_OK;
					}
				}
		
				return RESULT_ERR_INVALID_PARAM;
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		  
			inline ibool BtlTek2A::Irq::Signal()
			{
				NST_ASSERT
				( 
					enabled && (mode & MODE_SOURCE) == MODE_PPU_A12 &&
					(
						(mode & MODE_COUNT_ENABLE) == MODE_COUNT_DOWN ||
						(mode & MODE_COUNT_ENABLE) == MODE_COUNT_UP
					)
				);
		
				if (mode & MODE_COUNT_DOWN)
					return (--prescaler & scale) == scale && (count-- & 0xFF) == 0x00;
				else
					return (++prescaler & scale) == 0x00 && (++count & 0xFF) == 0x00;
			}
		
			ibool BtlTek2A::Irq::IsEnabled() const
			{
				const bool counting = enabled &&
				( 
					(mode & MODE_COUNT_ENABLE) == MODE_COUNT_DOWN || 
					(mode & MODE_COUNT_ENABLE) == MODE_COUNT_UP
				);
		
				return counting && (mode & MODE_SOURCE) == MODE_PPU_A12;
			}
		
			uint BtlTek2A::Banks::Unscramble(const uint bank)
			{
				return
				(
				    ((bank & 0x01) << 6) |
					((bank & 0x02) << 4) |
					((bank & 0x04) << 2) |
					((bank & 0x10) >> 2) |
					((bank & 0x20) >> 4) |
					((bank & 0x40) >> 6)
				);
			}
		
			NES_PEEK(BtlTek2A,5000) 
			{ 
				return (dipswitch & DIPSWITCH_GAME) | ((address >> 8) & ~uint(DIPSWITCH_GAME)); 
			}
		
			NES_POKE(BtlTek2A::Regs,5800) { mul[0] = data; }
			NES_POKE(BtlTek2A::Regs,5801) { mul[1] = data; }
			NES_POKE(BtlTek2A::Regs,5803) { tmp = data;  }
		
			NES_PEEK(BtlTek2A::Regs,5800) { return (mul[0] * mul[1]) & 0xFF; }
			NES_PEEK(BtlTek2A::Regs,5801) { return (mul[0] * mul[1]) >> 8;   }
			NES_PEEK(BtlTek2A::Regs,5803) { return tmp; }
		
			NES_PEEK(BtlTek2A,6000) 
			{ 
				return banks.prg6 ? banks.prg6[address - 0x6000U] : (address >> 8); 
			}
		
			NES_POKE(BtlTek2A,8000) 
			{
				address &= 0x3;
		
				if (banks.prg[address] != data)
				{
					banks.prg[address] = data; 
					UpdatePrg(); 
				}
			}
		  
			NES_POKE(BtlTek2A,9000) 
			{ 
				address &= 0x7;
				data |= banks.chr[address] & 0xFF00U;
		
				if (banks.chr[address] != data)
				{
					banks.chr[address] = data; 
					UpdateChr();  
				}
			}
		
			NES_POKE(BtlTek2A,A000) 
			{ 
				address &= 0x7;
				data = (data << 8) | (banks.chr[address] & 0x00FFU);
		
				if (banks.chr[address] != data)
				{
					banks.chr[address] = data; 
					UpdateChr();  
				}
			}
		
			NES_POKE(BtlTek2A,B000) 
			{ 
				address &= 0x3;
				data |= banks.name[address] & 0xFF00U;
		
				if (banks.name[address] != data)
				{
					banks.name[address] = data;
					UpdateName(); 
				}
			}
		
			NES_POKE(BtlTek2A,B004) 
			{ 
				address &= 0x3;
				data = (data << 8) | (banks.name[address] & 0x00FFU); 
				
				if (banks.name[address] != data)
				{
					banks.name[address] = data;
					UpdateName(); 
				}
			}
		
			NES_POKE(BtlTek2A,C000)
			{
				data &= Irq::TOGGLE;
		
				if (irq.unit.enabled != data)
				{
					irq.Update();
		
					irq.unit.enabled = data;
		
					if (!data)
						cpu.ClearIRQ();
		
					irq.EnableLine( irq.unit.IsEnabled() );
				}
			}
		
			NES_POKE(BtlTek2A,C001)
			{
				if (irq.unit.mode != data)
				{
					irq.Update();
		
					NST_VERIFY
					( 
						(data & Irq::MODE_SCALE_ADJUST) == 0 &&
						(
             	 			(data & Irq::MODE_SOURCE) == Irq::MODE_PPU_A12 ||
             	 			(data & Irq::MODE_SOURCE) == Irq::MODE_CPU_WRITE
						)
					);
		
					irq.unit.mode = data;
					irq.unit.scale = (data & Irq::MODE_SCALE_3BIT) ? 0x7 : 0xFF;
					irq.EnableLine( irq.unit.IsEnabled() );
				}
			}
		
			NES_POKE(BtlTek2A,C002) 
			{
				if (irq.unit.enabled)
				{
					irq.Update();
		
					irq.unit.enabled = false;
					cpu.ClearIRQ();
					irq.EnableLine( irq.unit.IsEnabled() );
				}
			}
		
			NES_POKE(BtlTek2A,C003) 
			{
				if (!irq.unit.enabled)
				{
					irq.Update();
		
					irq.unit.enabled = true;
					irq.EnableLine( irq.unit.IsEnabled() );
				}
			}
		
			NES_POKE(BtlTek2A,C004) 
			{
				irq.Update();
				irq.unit.prescaler = data ^ irq.unit.flip;
			}
									  
			NES_POKE(BtlTek2A,C005) 
			{
				irq.Update();
				irq.unit.count = data ^ irq.unit.flip;
			}
		
			NES_POKE(BtlTek2A,C006) 
			{
				irq.unit.flip = data;
			}
		
			NES_POKE(BtlTek2A,D000) 
			{
				if (regs.banking != data)
				{
					regs.banking = data;
					UpdatePrg();
					UpdateChr();
					UpdateName();
				}
			}
		  
			NES_POKE(BtlTek2A,D001) 
			{
				data &= Regs::CTRL1_MIRRORING;
		
				if (regs.mirroring != data)
				{
					regs.mirroring = data;
					UpdateName();
				}
			}
		
			NES_POKE(BtlTek2A,D002) 
			{
				if (regs.name != data)
				{
					regs.name = data;
					UpdateName();
				}
			}
		
			NES_POKE(BtlTek2A,D003) 
			{
				if (data & Regs::CTRL3_NO_SECONDARY)
				{
					banks.secondary[0] = 0xFFFFU;
					banks.secondary[1] = 0x0000U;
				}
				else
				{
					banks.secondary[0] = 0x00FFU;
					banks.secondary[1] = (data & Regs::CTRL3_SECONDARY) << 8;
				}
		
				UpdateChr();
				UpdateName();
			}
		
			void BtlTek2A::UpdatePrg()
			{
				banks.prg6 = NULL;
		
				uint prg3 = banks.prg[3];
		
				switch (regs.banking & Regs::CTRL0_PRG_MODE)
				{
					case Regs::CTRL0_PRG_SWAP_32K:
				
						if (regs.banking & Regs::CTRL0_PRG6_ENABLE)
							banks.prg6 = prg.Source().Mem( (prg3 << 2) | 0x3 );
		
						prg.SwapBank<NES_32K,0x0000U>( (regs.banking & Regs::CTRL0_PRG_NOT_LAST) ? prg3 : 0x7F );
						break;
				
					case Regs::CTRL0_PRG_SWAP_16K:
		
						if (regs.banking & Regs::CTRL0_PRG6_ENABLE)
							banks.prg6 = prg.Source().Mem( (prg3 << 1) | 0x1 );
		
						prg.SwapBanks<NES_16K,0x0000U>
						( 
					    	banks.prg[1], 
							(regs.banking & Regs::CTRL0_PRG_NOT_LAST) ? prg3 : 0x7F 
						);
						break;
		
					case Regs::CTRL0_PRG_SWAP_8K_R:
		
						prg3 = banks.Unscramble( prg3 );
		
					case Regs::CTRL0_PRG_SWAP_8K:
		
						if (regs.banking & Regs::CTRL0_PRG6_ENABLE)
							banks.prg6 = prg.Source().Mem( prg3 );
			
						prg.SwapBanks<NES_8K,0x0000U>
						( 
            			   	banks.prg[0],
							banks.prg[1],
							banks.prg[2],
							(regs.banking & Regs::CTRL0_PRG_NOT_LAST) ? prg3 : 0x7F
						);
						break;
				}
			}
		
			void BtlTek2A::UpdateChr() const
			{
				ppu.Update();
		
				switch (regs.banking & Regs::CTRL0_CHR_MODE)
				{
					case Regs::CTRL0_CHR_SWAP_8K:
				
						chr.SwapBank<NES_8K,0x0000U>
						( 
					     	(banks.chr[0] & banks.secondary[0]) | banks.secondary[1] 
						);
						break;
				
					case Regs::CTRL0_CHR_SWAP_4K:
				
						chr.SwapBanks<NES_4K,0x0000U>
						( 
					     	(banks.chr[0] & banks.secondary[0]) | banks.secondary[1],
							(banks.chr[4] & banks.secondary[0]) | banks.secondary[1]
						);
						break;
				
					case Regs::CTRL0_CHR_SWAP_2K:
				
						chr.SwapBanks<NES_2K,0x0000U>
						( 
					     	(banks.chr[0] & banks.secondary[0]) | banks.secondary[1],
							(banks.chr[2] & banks.secondary[0]) | banks.secondary[1],
							(banks.chr[4] & banks.secondary[0]) | banks.secondary[1],
							(banks.chr[6] & banks.secondary[0]) | banks.secondary[1]
						);
						break;
				
					case Regs::CTRL0_CHR_SWAP_1K:
				
						chr.SwapBanks<NES_1K,0x0000U>
						( 
					     	(banks.chr[0] & banks.secondary[0]) | banks.secondary[1], 
							(banks.chr[1] & banks.secondary[0]) | banks.secondary[1],
							(banks.chr[2] & banks.secondary[0]) | banks.secondary[1],
							(banks.chr[3] & banks.secondary[0]) | banks.secondary[1]
						);
		
						chr.SwapBanks<NES_1K,0x1000U>
						( 
					       	(banks.chr[4] & banks.secondary[0]) | banks.secondary[1], 
							(banks.chr[5] & banks.secondary[0]) | banks.secondary[1],
							(banks.chr[6] & banks.secondary[0]) | banks.secondary[1],
							(banks.chr[7] & banks.secondary[0]) | banks.secondary[1]
						);
						break;
				}
			}
		
			void BtlTek2A::UpdateName() const
			{
				ppu.Update();
		
				if ((regs.banking & Regs::CTRL0_CHR_NAMETABLES) && (dipswitch & 0x1))
				{
					for (uint i=0; i < 4; ++i)
					{
						const uint selection = 
						(
					       	(regs.banking & Regs::CTRL0_CHR_ROM_NAMETABLES) || 
							((banks.name[i] ^ regs.name) & Regs::CTRL2_NAME_USE_RAM)
						);
		
						nmt.Source(selection).SwapBank<NES_1K>
						( 
					    	i * 0x0400, 
							(banks.name[i] & banks.secondary[0]) | banks.secondary[1]
						);
					}
				}
				else
				{
					static const uchar lut[4] =
					{
						Ppu::NMT_VERTICAL,
						Ppu::NMT_HORIZONTAL,
						Ppu::NMT_ZERO,
						Ppu::NMT_ONE
					};
		
					ppu.SetMirroring( lut[regs.mirroring] );
				}
			}

			void BtlTek2A::VSync()
			{
				irq.VSync();
			}
		}
	}
}
