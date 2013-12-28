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

#include <vector>
#include "NstState.hpp"
#include "NstCpu.hpp"
#include "NstCrc32.hpp"
#include "NstTurboFile.hpp"
#include "api/NstApiUser.hpp"
   
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		TurboFile::TurboFile(Cpu& c)
		: cpu(c)
		{
			std::vector<u8> data;
			Api::User::fileIoCallback( Api::User::FILE_LOAD_TURBOFILE, data );
			const ulong size = NST_MIN(SIZE,data.size());

			if (size)
				std::memcpy( ram, &data.front(), size );

			std::memset( ram + size, 0, SIZE - size );
			crc = Crc32::Compute( ram, SIZE );
		}

		TurboFile::~TurboFile()
		{
			if (crc != Crc32::Compute( ram, SIZE ))
			{
				try
				{
					std::vector<u8> data( ram, ram + SIZE );
					Api::User::fileIoCallback( Api::User::FILE_SAVE_TURBOFILE, data );
				}
				catch (...)
				{
				}
			}
		}

		void TurboFile::Reset()
		{
			pos = 0x00;
			bit = 0x01;
			old = 0x00;
			out = 0x00;

			p4016 = cpu.Map( 0x4016U );
			p4017 = cpu.Map( 0x4017U );

			cpu.Map( 0x4016U ).Set( this, &TurboFile::Peek_4016, &TurboFile::Poke_4016 );
			cpu.Map( 0x4017U ).Set( this, &TurboFile::Peek_4017, &TurboFile::Poke_4017 );
		}

		void TurboFile::SaveState(State::Saver& state) const
		{
			uint count;
			for (count=0; bit && bit != (1U << count); ++count);

			const u8 data[3] =
			{
				pos & 0xFF,
				pos >> 8,
				count | (old << 1) | (out << 2)
			};

			state.Begin('R','E','G','\0').Write( data ).End();
			state.Begin('R','A','M','\0').Compress( ram ).End();
		}

		void TurboFile::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('R','E','G','\0'):
					{
						const State::Loader::Data<3> data( state );
				
						pos = data[0] | ((data[1] & 0x1F) << 8);
						bit = 1U << (data[2] & 0x7);
						old = (data[2] >> 1) & WRITE_BIT;
						out = (data[2] >> 2) & READ_BIT;
				
						break;
					}
				
					case NES_STATE_CHUNK_ID('R','A','M','\0'):
				
						state.Uncompress( ram );
						break;				
				}

				state.End();
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		NES_POKE(TurboFile,4016)
		{
			if (!(data & NO_RESET))
			{
				pos = 0x00;
				bit = 0x01;
			}

			const uint advance = old;
			old = data & WRITE_ENABLE;

			if (old)
			{
				ram[pos] = (ram[pos] & ~bit) | (bit * (data & WRITE_BIT));
			}
			else if (advance)
			{
				if (bit != 0x80)
				{
					bit <<= 1;
				}
				else
				{
					bit = 0x01;
					pos = (pos + 1) & (SIZE-1);
				}
			}

			out = (ram[pos] & bit) ? READ_BIT : 0;

			p4016.Poke( address, data );
		}

		NES_PEEK(TurboFile,4016)
		{
			return p4016.Peek( address );
		}

		NES_POKE(TurboFile,4017)
		{
			p4017.Poke( address, data );
		}

		NES_PEEK(TurboFile,4017)
		{
			return p4017.Peek( address ) | out;
		}
	}
}
