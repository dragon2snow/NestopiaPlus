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

#ifndef NST_APPLICATION_EXCEPTION_H
#define NST_APPLICATION_EXCEPTION_H

#pragma once

#include <cstdlib>
#include "NstMain.hpp"

namespace Nestopia
{
	namespace Application
	{
		class Exception
		{
		public:

			enum Type
			{
				CRITICAL,
				WARNING,
				UNSTABLE
			};

			enum Place
			{
				PRESENT,
				FINAL
			};

			enum ExitCode
			{
				QUIT_SUCCESS = EXIT_SUCCESS,
				QUIT_FAILURE = EXIT_FAILURE
			};

			void Issue(Place=PRESENT) const;
			tstring GetMessageText() const;

		private:

			class Message;

			enum 
			{
				NO_ID = INT_MAX,
				MAX_MSG_LENGTH = 1024
			};

			const uint msgId;
			tstring const msg;
			const Type type;

		public:

			explicit Exception(uint i,Type t=CRITICAL)
			: msgId(i), msg(NULL), type(t) {}

			explicit Exception(tstring m,Type t=CRITICAL)
			: msgId(NO_ID), msg(m), type(t) {}

			Type GetType() const
			{
				return type;
			}
		};
	}
}

#endif
