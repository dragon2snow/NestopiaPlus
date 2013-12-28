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

#include "NstInpDevice.hpp"
#include "../NstPpu.hpp"
#include "NstInpLightGun.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			const u8 LightGun::lightMap[64] =
			{
				0x66,0x1C,0x1F,0x2A,0x31,0x2D,0x27,0x26,
				0x29,0x2E,0x34,0x32,0x30,0x00,0x00,0x00,
				0xB9,0x52,0x52,0x51,0x53,0x4F,0x54,0x59,
				0x5E,0x5A,0x5A,0x5E,0x5D,0x00,0x00,0x00,
				0xFF,0xA4,0xA0,0xA2,0xA2,0x9D,0xA0,0xA6,
				0xA9,0xA8,0xA6,0xA6,0xAB,0x60,0x00,0x00,
				0xFF,0xD8,0xD5,0xD1,0xCE,0xCE,0xCF,0xD5,
				0xDD,0xD5,0xD1,0xD0,0xD6,0xBD,0x00,0x00
			};

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			LightGun::LightGun(Ppu& p)
			: 
			Device (Api::Input::ZAPPER),
			patch  (PATCH_NORMAL),
			ppu    (p)
			{
				LightGun::Reset();
			}
		
			void LightGun::Reset()
			{
				shifter = 1;
				stream = 0x10;
			}
		
			void LightGun::Initialize(const dword pRomCrc)
			{
				switch (pRomCrc)
				{
					case 0xED588F00UL: // VS.Duck Hunt
					case 0xFF5135A3UL: // VS.Hogan's Alley
					case 0x17AE56BEUL: // VS.Freedom Force
			
						patch = PATCH_VS;
						break;
			
					default:
			
						patch = PATCH_NORMAL;
						break;
				}
			}
		
			void LightGun::SaveState(State::Saver& state,const uchar id) const
			{
				const u8 data[2] = 
				{ 
					(patch == PATCH_VS) ? shifter ? 0x1 : 0x3 : 0x0, 
					(patch == PATCH_VS) ? stream : 0x00
				};

				state.Begin('L','G',id,'\0').Write( data ).End();
			}

			void LightGun::LoadState(State::Loader& state,const dword id)
			{
				if (id == NES_STATE_CHUNK_ID('L','G','\0','\0'))
				{
					const State::Loader::Data<2> data( state );

					if (data[0] & 0x1)
					{
						shifter = (~data[0] & 0x2) >> 1;
						stream = data[1];
					}
				}
			}

            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif

			void LightGun::BeginFrame(Controllers* i)
			{
				input = i;
				pos = ~0U;
				fire = 0;
			}

			uint LightGun::Poll()
			{
				if (input)
				{
					Controllers::Zapper& zapper = input->zapper;
					input = NULL;

					if (Controllers::Zapper::callback( zapper ))
					{
						fire = (zapper.fire ? patch == PATCH_NORMAL ? 0x10 : 0x80 : 0x00);

						if (zapper.y < Ppu::HEIGHT && zapper.x < Ppu::WIDTH)
							pos = zapper.y * Ppu::WIDTH + zapper.x;
					}
				}

				if (pos < Ppu::WIDTH * Ppu::HEIGHT)
				{
					ppu.Update();

					const uint cycle = ppu.GetPixelCycles();

					if (pos < cycle && pos >= cycle - PHOSPHOR_DECAY)
						return lightMap[ppu.GetPixel( pos ) & 0x3F];
				}

				return 0;
			}
		
			void LightGun::Poke(const uint data)
			{
				if (patch == PATCH_VS)
				{
					shifter = ~data & 0x1;
					stream = 0x10 | fire | (Poll() >= 0x40 ? 0x40 : 0x00);
				}
			}
		
			uint LightGun::Peek(uint)
			{
				if (patch == PATCH_NORMAL)
				{
					return fire | (Poll() >= 0x40 ? 0x0 : 0x8);
				}
				else
				{
					const uint data = stream;
					stream >>= shifter;
					return data & 0x1;
				}
			}
		}
	}
}
