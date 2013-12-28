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

#include <algorithm>
#include "NstIoFile.hpp"
#include "NstDialogLauncher.hpp"

namespace Nestopia
{
	using namespace Window;

	Launcher::List::Files::Strings::Strings(const uint reserve)
	: container( NST_MAX(2,reserve) )
	{
		container[0] = '-';
		container[1] = '\0';

		container.Resize( 2 );
	}

	Launcher::List::Files::Strings::Index Launcher::List::Files::Strings::operator << (const String::Generic string)
	{
		const uint index = container.Size();

		container.Append( string, string.Size() + 1 );
		container.Back() = '\0';

		return index;
	}

	int Launcher::List::Files::Strings::Find(const String::Generic needle) const
	{
		cstring it = container.Begin();
		cstring const end = container.End();
		NST_ASSERT( *(end-1) == '\0' );

		do
		{
			const String::Generic string( it );

			if (needle == string)
				return it - container;

			it += string.Size() + 1;
		}
		while (it != end);

		return -1;
	}

	uint Launcher::List::Files::Strings::Count() const
	{
		return std::count( container.Begin(), container.End(), '\0' );
	}

	void Launcher::List::Files::Strings::Clear()
	{
		container.Resize( 2 );
		container.Defrag();
	}

	ibool Launcher::List::Files::Strings::Import(const Io::File& file,const uint size)
	{
		NST_ASSERT( size );

		if (size < 2)
			return FALSE;

		container.Resize( size );
		file.Read( container, size );

		return container[0] == '-' && container[1] == '\0' && container.Back() == '\0';
	}

	void Launcher::List::Files::Strings::Export(const Io::File& file) const
	{
		NST_ASSERT( container.Size() >= 2 );
		file.Stream() << container;
	}
}
