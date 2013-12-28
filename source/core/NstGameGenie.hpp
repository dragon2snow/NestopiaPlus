////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#ifndef NST_GAMEGENIE_H
#define NST_GAMEGENIE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <map>

namespace Nes
{
	namespace Core
	{
		class Cpu;

		class GameGenie
		{
		public:

			GameGenie(Cpu&);
			~GameGenie();

			void Reset();
			void ClearCodes();

			static Result Encode (dword,char (&)[9]);
			static Result Decode (cstring,ulong&);
			static Result Pack   (uint,uint,uint,bool,ulong&);
			static Result Unpack (dword,uint&,uint&,uint&,bool&);

			Result AddCode    (dword);
			Result DeleteCode (dword);

			uint NumCodes() const;

			dword GetCode(uint) const;	

		private:

			class Code
			{
			public:

				Code();

				void operator = (const Code&);

				Result Decode(cstring,ulong* = NULL);
				Result Encode(dword,char* = NULL);

				inline uint Peek(uint);
				inline void Poke(uint,uint);

				dword Packed() const;

			private:

				int  DecodeCharacters(cstring,uint*) const;
				void EncodeCharacters(const uint*,char*) const;

				void Decode6(const uint*);
				void Encode6(uint*) const;

				void Decode8(const uint*);
				void Encode8(uint*) const;

				void DecodeAddress(const uint*);
				void EncodeAddress(uint*) const;

				Io::Port port;

				uchar  data;
				uchar  compare;
				ushort address;
				uchar  useCompare;

			public:

				void SetPort(const Io::Port& p)
				{ 
					port = p; 
				}

				const Io::Port& GetPort() const
				{ 
					return port; 
				}

				uint Address() const 
				{ 
					return address; 
				}
			};

			typedef std::map<uint,Code> Codes;

			void Map(Code&,bool);

			NES_DECL_PEEK( wizard )
			NES_DECL_POKE( wizard )

			Codes codes;
			Cpu& cpu;
		};
	}
}

#endif
