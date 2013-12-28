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

#ifndef NST_DIALOG_INESHEADER_H
#define NST_DIALOG_INESHEADER_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class InesHeader
		{
		public:

			InesHeader(const Nes::Cartridge::Database&,const Managers::Paths&);

			void Open(const Path&);

		private:

			struct Handlers;

            #pragma pack(push,1)
		
			struct Header
			{
				enum
				{
					SIGNATURE  = 0x1A53454E,
					VERTICAL   = 0x0001,
					BATTERY    = 0x0002,
					TRAINER    = 0x0004,
					FOURSCREEN = 0x0008,
					MAPPER_LO  = 0x00F0,
					VS         = 0x0100,
					MAPPER_HI  = 0xF000,
					PAL        = 0x1
				};
		
				u32 signature;
				u8  num16kPRomBanks;
				u8  num8kCRomBanks;
				u16 flags;
				u8  num8kWRamBanks;
				u8  pal;
				u8  reserved[6];
			};
		
            #pragma pack(pop)
		
			NST_COMPILE_ASSERT( sizeof(Header) == 16 );

			static uint Import(const Path&,Collection::Buffer&);
			static uint Export(const Path&,const Collection::Buffer&);			
			static ibool IsBad(const Header&,ulong);

			ibool Save(Header&) const;
			void UpdateOriginal() const;
			void UpdateDetect() const;

			ibool OnInitDialog  (Param&);
			ibool OnCmdOriginal (Param&);
			ibool OnCmdDetect   (Param&);
			ibool OnCmdCancel   (Param&);
			ibool OnCmdSave     (Param&);

			Dialog dialog;
			const Nes::Cartridge::Database database;
			Nes::Cartridge::Database::Entry dbEntry;
			Header header;
			ulong imageSize;
			const Path* path;
			const Managers::Paths& paths;
		};
	}
}

#endif
