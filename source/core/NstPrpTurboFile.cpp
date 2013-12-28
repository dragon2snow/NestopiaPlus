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
#include "NstCpu.hpp"
#include "NstState.hpp"
#include "NstFile.hpp"
#include "NstPrpTurboFile.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Peripherals
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			TurboFile::TurboFile(Cpu& c)
			: cpu(c)
			{
				std::memset( ram, 0, SIZE );
				file.Load( File::LOAD_TURBOFILE, ram, SIZE );
			}

			void TurboFile::PowerOff() const
			{
				file.Save( File::SAVE_TURBOFILE, ram, SIZE );
			}

			void TurboFile::Reset()
			{
				pos = 0x00;
				bit = 0x01;
				old = 0x00;
				out = 0x00;

				p4016 = cpu.Map( 0x4016 );
				p4017 = cpu.Map( 0x4017 );

				cpu.Map( 0x4016 ).Set( this, &TurboFile::Peek_4016, &TurboFile::Poke_4016 );
				cpu.Map( 0x4017 ).Set( this, &TurboFile::Peek_4017, &TurboFile::Poke_4017 );
			}

			void TurboFile::SaveState(State::Saver& state,const dword id) const
			{
				state.Begin( id );

				uint count;
				for (count=0; bit && bit != (1U << count); ++count);

				const byte data[3] =
				{
					pos & 0xFF,
					pos >> 8,
					count | (old << 1) | (out << 2)
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				state.Begin( AsciiId<'R','A','M'>::V ).Compress( ram ).End();

				state.End();
			}

			void TurboFile::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'R','E','G'>::V:
						{
							State::Loader::Data<3> data( state );

							pos = data[0] | (data[1] << 8 & 0x1F00);
							bit = 1U << (data[2] & 0x7);
							old = data[2] >> 1 & WRITE_BIT;
							out = data[2] >> 2 & READ_BIT;

							break;
						}

						case AsciiId<'R','A','M'>::V:

							state.Uncompress( ram );
							break;
					}

					state.End();
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
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
}
