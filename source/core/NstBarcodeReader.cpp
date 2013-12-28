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
#include "NstState.hpp"
#include "NstBarcodeReader.hpp"
   
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
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
				state.Begin('P','T','R','\0').Write8( stream - data ).End();
				state.Begin('D','A','T','\0').Compress( data ).End();
			}
		}

		void BarcodeReader::LoadState(State::Loader& state,const dword chunk)
		{
			switch (chunk)
			{
     			case NES_STATE_CHUNK_ID('P','T','R','\0'):

					stream = data + (state.Read8() & (MAX_DATA_LENGTH-1));
					break;

				case NES_STATE_CHUNK_ID('D','A','T','\0'):
				
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

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	}
}
