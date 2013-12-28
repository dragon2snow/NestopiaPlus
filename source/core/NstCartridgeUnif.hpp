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

#ifndef NST_CARTRIDGE_UNIF_H
#define NST_CARTRIDGE_UNIF_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Cartridge::Unif
		{
			typedef Api::Cartridge Ref;

		public:

			Unif
			(
				StdStream,
				Ram&,
				Ram&,
				Ram&,
				Ref::Info&,
				const ImageDatabase*,
				ImageDatabase::Handle&
			);

		private:

			class Boards;

			void CopyRom();
			void CheckImageDatabase(const ImageDatabase*,ImageDatabase::Handle&);

			static bool NewChunk(byte&,dword);
			static cstring ChunkName(char (&)[5],dword);

			void  ReadHeader     ();
			void  ReadChunks     ();
			dword ReadName       ();
			dword ReadComment    ();
			dword ReadString     (cstring,Vector<char>*);
			dword ReadDumper     ();
			dword ReadSystem     ();
			dword ReadRomCrc     (uint,uint);
			dword ReadRomData    (uint,uint,dword);
			dword ReadBattery    ();
			dword ReadMapper     ();
			dword ReadMirroring  ();
			dword ReadController ();
			dword ReadChrRam     () const;
			dword ReadUnknown    (dword) const;

			enum
			{
				HEADER_RESERVED_LENGTH = 24,
				MAX_ROM_SIZE = SIZE_16K * 0x1000UL
			};

			struct Rom
			{
				Rom();

				dword crc;
				dword truncated;
				Ram rom;
			};

			struct Dump
			{
				enum
				{
					NAME_LENGTH = 100,
					AGENT_LENGTH = 100,
					LENGTH = NAME_LENGTH + 4 + AGENT_LENGTH
				};

				char name[NAME_LENGTH];
				byte day;
				byte month;
				word year;
				char agent[AGENT_LENGTH];
			};

			Stream::In stream;
			Ram& prg;
			Ram& chr;
			Ref::Info& info;
			Rom roms[2][16];
			Result result;
			bool knownBoard;

		public:

			Result GetResult() const
			{
				return result;
			}
		};
	}
}

#endif
