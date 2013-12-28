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
#include "NstState.hpp"
#include "NstPrpBarcodeReader.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Peripherals
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			BarcodeReader::BarcodeReader()
			{
				Reset();
			}

			void BarcodeReader::Reset()
			{
				stream = data;
				std::memset( data, END, MAX_DATA_LENGTH );
			}

			void BarcodeReader::SaveState(State::Saver& state) const
			{
				if (IsTransferring())
				{
					state.Begin( AsciiId<'P','T','R'>::V ).Write8( stream - data ).End();
					state.Begin( AsciiId<'D','A','T'>::V ).Compress( data ).End();
				}
			}

			void BarcodeReader::LoadState(State::Loader& state,const dword chunk)
			{
				switch (chunk)
				{
					case AsciiId<'P','T','R'>::V:

						stream = data + (state.Read8() & (MAX_DATA_LENGTH-1));
						break;

					case AsciiId<'D','A','T'>::V:

						state.Uncompress( data );
						data[MAX_DATA_LENGTH-1] = END;
						break;
				}
			}

			bool BarcodeReader::Transfer(cstring const string,const uint length)
			{
				Reset();
				return (string && length && SubTransfer( string, length, data ));
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}
