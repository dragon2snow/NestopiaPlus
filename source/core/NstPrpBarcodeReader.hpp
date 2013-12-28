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

#ifndef NST_PRP_BARCODEREADER_H
#define NST_PRP_BARCODEREADER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace State
		{
			class Saver;
			class Loader;
		}

		namespace Peripherals
		{
			class NST_NO_VTABLE BarcodeReader
			{
			public:

				enum
				{
					MIN_DIGITS = 8,
					MAX_DIGITS = 13
				};

				bool Transfer(cstring,uint);
				virtual bool IsDigitsSupported(uint) const = 0;

			protected:

				BarcodeReader();

				enum
				{
					MAX_DATA_LENGTH = 0x100,
					END = 0xFF
				};

				void Reset();
				void SaveState(State::Saver&) const;
				void LoadState(State::Loader&,dword);

			private:

				virtual bool SubTransfer(cstring,uint,u8*) = 0;

				const u8* stream;
				u8 data[MAX_DATA_LENGTH];

			protected:

				virtual ~BarcodeReader() {}

				uint Fetch()
				{
					uint data = *stream;
					stream += (data != END);
					return data;
				}

				uint Latch() const
				{
					return (stream != data) ? *(stream-1) : 0x00;
				}

			public:

				bool IsTransferring() const
				{
					return *stream != END;
				}			
			};
		}
	}
}

#endif
