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

#include <algorithm>
#include "NstIoFile.hpp"
#include "NstDialogLauncher.hpp"

namespace Nestopia
{
	namespace Window
	{
		Launcher::List::Files::Strings::Strings(const uint reserve)
		{
			container.Reserve( 2 + reserve );
			container.Assign( _T("-\0"), 2 );
		}

		int Launcher::List::Files::Strings::Find(const GenericString needle) const
		{
			tstring it = container.Ptr();
			tstring const end = it + container.Length();
			NST_ASSERT( *(end-1) == '\0' );

			do
			{
				const GenericString string( it );

				if (needle == string)
					return it - container.Ptr();

				it += string.Length() + 1;
			}
			while (it != end);

			return -1;
		}

		uint Launcher::List::Files::Strings::Count() const
		{
			return std::count( container.Ptr(), container.Ptr() + container.Length(), _T('\0') );
		}

		void Launcher::List::Files::Strings::Clear()
		{
			container.ShrinkTo( 2 );
			container.Defrag();
		}

		ibool Launcher::List::Files::Strings::Import(const Io::File& file,const uint size,const ibool utf16)
		{
			NST_VERIFY( size >= 2 );

			if (size >= 2)
			{
				if (utf16)
				{
					container.Resize( size / 2 );
					file.Read( container.Ptr(), container.Length() * sizeof(tchar) );
				}
				else
				{
					String::Heap<char> tmp;
					tmp.Resize( size );
					file.Read( tmp.Ptr(), tmp.Length() );
					container = tmp;
				}

				if (container[0] == '-' && container[1] == '\0')
					return true;
			}

			return false;
		}

		void Launcher::List::Files::Strings::Export(const Io::File& file) const
		{
			NST_ASSERT( container.Length() >= 2 );

			if (IsUTF16())
			{
				file.Write( container.Ptr(), container.Length() * sizeof(tchar) );
			}
			else
			{
				const String::Heap<char> tmp( container );
				file.Write( tmp.Ptr(), tmp.Length() );
			}
		}
	}
}
