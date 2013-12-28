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

#ifndef NST_RAM_H
#define NST_RAM_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Ram
		{
		public:

			Ram();
			~Ram();

			void Set(dword,u8* = NULL);
			void Set(bool,bool,dword,u8* = NULL);
			void Destroy();
			void Fill(uint);
			void Mirror(dword);

		private:

			u8* mem;
			dword mask;
			dword size;
			u8 writable;
			u8 readable;
			u8 internal;
			const u8 pad;

		public:

			dword Size() const
			{
				return size;
			}

			dword Masking() const
			{
				return mask;
			}

			ibool Empty() const
			{
				return size == 0;
			}

			ibool Readable() const
			{
				return readable;
			}

			ibool Writable() const
			{
				return writable;
			}

			ibool Internal() const
			{
				return internal;
			}

			void ReadEnable(bool r)
			{
				readable = r;
			}

			void WriteEnable(bool w)
			{
				writable = w;
			}

			u8* Mem(dword offset=0)
			{
				return mem + (offset & mask);
			}

			const u8* Mem(dword offset=0) const
			{
				return mem + (offset & mask);
			}

			u8& operator [] (dword i)
			{
				return mem[i];
			}

			const u8& operator [] (dword i) const
			{
				return mem[i];
			}
		};
	}
}

#endif
