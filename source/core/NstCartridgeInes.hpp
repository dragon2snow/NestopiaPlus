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

#ifndef NST_CARTRIDGE_INES_H
#define NST_CARTRIDGE_INES_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Log;

		class Cartridge::Ines
		{
			typedef Api::Cartridge Ref;

		public:

			Ines
			(
				StdStream,
				Ram&,
				Ram&,
				Ram&,
				Ref::Info&,
				const ImageDatabase*,
				ImageDatabase::Handle&
			);

			enum
			{
				TRAINER_BEGIN  = 0x1000,
				TRAINER_END    = 0x1200,
				TRAINER_LENGTH = 0x0200
			};

			static Result ReadHeader(Ref::Setup&,const byte*,ulong);
			static Result WriteHeader(const Ref::Setup&,byte*,ulong);

			static ImageDatabase::Handle SearchDatabase(const ImageDatabase&,const byte*,ulong);

		private:

			enum
			{
				VS_MAPPER_99 = 99,
				VS_MAPPER_151 = 151,
				MIN_CRC_SEARCH_STRIDE = NST_MIN(dword(SIZE_8K),dword(TRAINER_LENGTH)),
				MAX_CRC_SEARCH_LENGTH = TRAINER_LENGTH + SIZE_16K * 0xFFFUL + SIZE_8K * 0xFFFUL
			};

			Result Collect();
			dword  Process();
			dword  Repair(Log&);

			Result result;

			Stream::In stream;

			Ram& prg;
			Ram& chr;
			Ram& wrk;

			Ref::Info& info;

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
