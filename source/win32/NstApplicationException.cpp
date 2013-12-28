////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#include <new>
#include <cstring>
#include "resource/resource.h"
#include "NstApplicationException.hpp"
#include <Windows.h>

namespace Nestopia
{
	using Application::Exception;

	class Exception::Message
	{
		Message()
		: id(0), allocated(FALSE), next(NULL) {}

		Message(uint i,tstring const backup)
		: id(i), string(backup), allocated(FALSE), next(NULL)
		{
			tchar buffer[MAX_MSG_LENGTH];	

			if (uint length = ::LoadString( ::GetModuleHandle(NULL), i, buffer, MAX_MSG_LENGTH-1 ))
			{
				if (tchar* const block = new (std::nothrow) tchar [++length])
				{
					string = block;
					allocated = TRUE;
					std::memcpy( block, buffer, length * sizeof(tchar) );
				}
			}
		}

		~Message()
		{
			delete next;

			if (allocated)
				delete [] string;
		}

		const uint id;
		tstring string;
		ibool allocated;
		Message* next;

		static Message list;

	public:

		static tstring GetString(const uint,tstring);
	};

	Exception::Message Exception::Message::list;

	tstring Exception::Message::GetString(const uint id,tstring const backup)
	{
		Message* node = &list;

		while (node->next)
		{
			node = node->next;

			if (node->id == id)
				return node->string;
		}

		node->next = new Message( id, backup );

		return node->next->string;
	}

	tstring Exception::GetMessageText() const
	{
		if (msg && *msg)
			return msg;

		if (type == CRITICAL)
		{
			return Message::GetString
			( 
				msgId != NO_ID ? msgId : IDS_ERR_GENERIC, 
				_T("Unhandled error! Call the Ghostbusters!")
			);
		}

		tstring warning = _T("Unknown warning, just click OK to continue.");

		if (msgId != NO_ID)
			warning = Message::GetString( msgId, warning );

		return warning;
	}

	void Exception::Issue(const Place final) const
	{
		if (final && type == UNSTABLE)
			return;

		tstring message = GetMessageText();

		uint flags;
		tchar buffer[MAX_MSG_LENGTH * 2 + 8];

		switch (type)
		{
			case WARNING:  
	
				flags = MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST;
				break;
	
			case UNSTABLE: 
			{
				flags = MB_YESNO|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST;
	
				tstring const unstable = Message::GetString
				( 
					IDS_UNSTABLE, _T("application is unstable! Sure you want to continue?")
				);
	
				::_tcscpy( buffer, message );
				::_tcscat( buffer, _T("\r\n\r\n") );
				::_tcscat( buffer, unstable );
	
				message = buffer;
				break;
			}
	
			default:
	
				flags = MB_OK|MB_ICONERROR|MB_SETFOREGROUND|MB_TOPMOST;
				break;
		}

		tstring const caption = Message::GetString
		(
			type == CRITICAL ? IDS_TITLE_ERROR : IDS_TITLE_WARNING,
			type == CRITICAL ? _T("Nestopia Error") : _T("Nestopia Warning")
		);

		const int result = ::MessageBox( ::GetActiveWindow(), message, caption, flags );

		if (!final && (type == CRITICAL || (type == UNSTABLE && result == IDNO)))
			throw QUIT_FAILURE;
	}
}
