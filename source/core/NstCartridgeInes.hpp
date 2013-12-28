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

#ifndef NST_CARTRIDGE_INES_H
#define NST_CARTRIDGE_INES_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Cartridge::Ines
		{
		public:

			Ines
			(
				StdStream,
				Ram&,
				Ram&,
				Ram&,
				Api::Cartridge::Info&,
				const ImageDatabase*,
				ImageDatabase::Handle&
			);

			enum
			{
				TRAINER_BEGIN  = 0x1000U,
				TRAINER_END    = 0x1200U,
				TRAINER_LENGTH = 0x0200U
			};

			static Result ReadHeader(Api::Cartridge::Setup&,const void*,ulong);
			static Result WriteHeader(const Api::Cartridge::Setup&,void*,ulong);

			static ImageDatabase::Handle SearchDatabase(const ImageDatabase&,const u8*,ulong);

		private:

			enum
			{
				VS_MAPPER_99 = 99,
				VS_MAPPER_151 = 151
			};

			void Import();
			void TryDatabase();

			Result result;

			Stream::In stream;
			Log log;

			Ram& prg;
			Ram& chr;
			Ram& wrk;

			Api::Cartridge::Info& info;

			dword prgSkip;
			dword chrSkip;

			const ImageDatabase* const database;
			ImageDatabase::Handle& databaseHandle;

		public:

			Result GetResult() const
			{
				return result;
			}
		};
	}
}

#endif
