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

#include "NstWindowDialog.hpp"
#include "NstDialogFind.hpp"

namespace Nestopia
{
	using namespace Window;

	struct Finder::Handlers
	{
		static const MsgHandler::Entry<Finder> messages[];
		static const MsgHandler::HookEntry<Finder> hooks[];
	};

	const MsgHandler::Entry<Finder> Finder::Handlers::messages[] =
	{
		{ ::RegisterWindowMessage(FINDMSGSTRING), &Finder::OnMsg }
	};

	const MsgHandler::HookEntry<Finder> Finder::Handlers::hooks[] =
	{
		{ WM_DESTROY, &Finder::OnDestroy }
	};

	Finder::Finder(Custom& p)
	: parent( p )
	{
		p.Messages().Add( this, Handlers::messages, Handlers::hooks );
		findReplace.lStructSize = sizeof(findReplace);
		findReplace.lpstrFindWhat = NULL;
		findReplace.wFindWhatLen = BUFFER_SIZE;
	}

	void Finder::Close()
	{
		if (window)
		{
			Dialog::UnregisterModeless( window );
			window.Destroy();
		}
	}

	Finder::~Finder()
	{
		Close();

		delete [] findReplace.lpstrFindWhat;
		findReplace.lpstrFindWhat = NULL;
	}

	void Finder::Open(const Callback& c,const uint flags)
	{
		if (Handlers::messages[0].key && window == NULL)
		{
			callback = c;

			if (findReplace.lpstrFindWhat == NULL)
			{
				findReplace.lpstrFindWhat = new tchar [BUFFER_SIZE+1];
				findReplace.lpstrFindWhat[0] = '\0';
			}

			findReplace.Flags = flags & (DOWN|WHOLEWORD|MATCHCASE);
			findReplace.hwndOwner = parent;

			if (window = ::FindText( &findReplace ))
				Dialog::RegisterModeless( window );
		}
	}

	void Finder::OnDestroy(Param&)
	{
		Close();
	}

	ibool Finder::OnMsg(Param&)
	{
		if (findReplace.Flags & FR_DIALOGTERM)
		{
			Close();
		}
		else if (findReplace.Flags & FR_FINDNEXT)
		{
			callback( findReplace.lpstrFindWhat, findReplace.Flags & (FR_WHOLEWORD|FR_MATCHCASE|FR_DOWN) );
		}

		return true;
	}
}
