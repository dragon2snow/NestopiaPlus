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

#include "NstObjectPod.hpp"
#include "NstCollectionVector.hpp"
#include "NstApplicationException.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstSystemAccelerator.hpp"

namespace Nestopia
{
	using System::Accelerator;

	struct Accelerator::CopyTable
	{
		enum { STACK_SIZE = 96 };

		uint size;
		ACCEL stack[STACK_SIZE];
		Collection::Vector<ACCEL> heap;
	};

	void Accelerator::Clear()
	{
		if (handle)
		{
			::DestroyAcceleratorTable( handle );
			handle = NULL;
		}
	}

	uint Accelerator::Size() const
	{
		if (handle)
		{
			int size = ::CopyAcceleratorTable( handle, NULL, 0 );

			if (size > 0) 
				return size;
		}

		return 0;
	}

	ibool Accelerator::Get(Entries& entries) const
	{
		entries.Resize( Size() );

		if (entries.Size())
		{
			if (::CopyAcceleratorTable( handle, entries, entries.Size() ) != (int) entries.Size())
				throw Application::Exception("CopyAcceleratorTable() failed!");

			return TRUE;
		}

		return FALSE;
	}

	void Accelerator::Set(const ACCEL* const entries,const uint size)
	{
		NST_ASSERT( bool(entries) >= bool(size) );

		Clear();

		if (size)
		{
			Collection::Vector<ACCEL> actualEntries( size );
			uint actualSize = 0;

			for (uint i=0; i < size; ++i)
			{
				if (entries[i].fVirt && entries[i].key && entries[i].cmd)
					actualEntries[actualSize++] = entries[i];
			}

			if (actualSize && (handle = ::CreateAcceleratorTable( actualEntries, actualSize )) == 0)
				throw Application::Exception("CreateAcceleratorTable() failed!");
		}
	}

	Accelerator::KeyName Accelerator::GetKeyName(const ACCEL& accel)
	{
		KeyName name;
		String::Generic shortCut;

		switch (accel.fVirt & ~FVIRTKEY)
		{
			case FALT:				   shortCut = "Alt+";            break;
			case FSHIFT:			   shortCut = "Shift+";          break;
			case FCONTROL:			   shortCut = "Ctrl+";           break;
			case FCONTROL|FALT:        shortCut = "Ctrl+Alt+";       break;
			case FCONTROL|FSHIFT:      shortCut = "Ctrl+Shift+";     break;
			case FALT|FSHIFT:		   shortCut = "Alt+Shift+";      break;
			case FCONTROL|FALT|FSHIFT: shortCut = "Ctrl+Alt+Shift+"; break;
		}

		if (shortCut.Size())
			name = shortCut;

		name << Keyboard::VikName( accel.key );

		if (name.Size() > shortCut.Size() + 1)
			name( shortCut.Size() + 1 ).MakeLowerCase();

		return name;
	}

	ACCEL* Accelerator::Copy(CopyTable& table) const
	{
		if (handle && (table.size = Size()) != 0)
		{
			ACCEL* ptr;

			if (table.size < CopyTable::STACK_SIZE)
			{
				ptr = table.stack;
			}
			else
			{
				table.heap.Resize( table.size + 1 );
				ptr = table.heap;
			}

			if (table.size == (uint) ::CopyAcceleratorTable( handle, ptr, table.size ))
				return ptr;
		}

		return NULL;
	}

	ACCEL Accelerator::operator [] (const uint cmd) const
	{
		Object::Pod<ACCEL> entry;

		CopyTable table;

		if (const ACCEL* it = Copy( table ))
		{
			for (const ACCEL* const end = it + table.size; it != end; ++it)
			{
				if (cmd == it->cmd)
				{
					static_cast<ACCEL&>(entry) = *it;
					break;
				}
			}
		}

		return entry;
	}
}
