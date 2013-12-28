////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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
		class Ips;

		class Cartridge::Unif
		{
		public:

			static void Load
			(
				StdStream,
				StdStream,
				Ram&,
				Ram&,
				FavoredSystem,
				Profile&,
				ProfileEx&,
				const ImageDatabase*
			);

		private:

			enum
			{
				HEADER_RESERVED_LENGTH = 24,
				MAX_ROM_SIZE = SIZE_16K * 0x1000UL
			};

			class Context
			{
			public:

				Context();

				bool operator () (uint,dword);

				enum System
				{
					SYSTEM_NTSC,
					SYSTEM_PAL,
					SYSTEM_BOTH
				};

				struct Rom
				{
					Rom();

					Ram data;
					dword truncated;
					char crc[12];
				};

				System system;
				Rom roms[2][16];

			private:

				byte chunks[80];
			};

			static bool NewChunk (byte&,dword);
			static cstring ChunkName (char (&)[5],dword);

			static void  ReadHeader     (StdStream);
			static void  ReadChunks     (StdStream,Ram&,Ram&,FavoredSystem,Profile&,ProfileEx&);
			static dword ReadString     (StdStream,cstring,Vector<char>*);
			static dword ReadName       (StdStream,std::wstring&);
			static dword ReadDumper     (StdStream);
			static dword ReadComment    (StdStream);
			static dword ReadSystem     (StdStream,Context&);
			static dword ReadMirroring  (StdStream,ProfileEx&);
			static dword ReadBattery    (ProfileEx&);
			static dword ReadBoard      (StdStream,Profile::Board&);
			static dword ReadController (StdStream,Api::Input::Type (&)[5]);
			static dword ReadChrRam     ();
			static dword ReadChecksum   (StdStream,uint,uint,Context::Rom&);
			static dword ReadRom        (StdStream,uint,uint,dword,Context::Rom*);
			static dword ReadUnknown    (dword);
		};
	}
}

#endif
