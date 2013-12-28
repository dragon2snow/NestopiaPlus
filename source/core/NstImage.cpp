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

#include <new>
#include "NstStream.hpp"
#include "NstCartridge.hpp"
#include "NstFds.hpp"
#include "NstNsf.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		Image::Image(Type t)
		: type(t) {}

		Result Image::Load(Context& context)
		{
			NST_ASSERT( context.image == NULL );

			Result result;

			try
			{
				switch (Stream::In(context.stream).Peek32())
				{
					case 0x1A53454EUL: // ines
					case 0x46494E55UL: // unif

						if (context.type == CARTRIDGE || context.type == UNKNOWN)
						{
							result = RESULT_OK;
							context.image = new Cartridge (context,result);
							return result;
						}
						break;

					case 0x1A534446UL: // fds
					case 0x494E2A01UL: //

						if (context.type == DISK || context.type == UNKNOWN)
							context.image = new Fds (context);

						break;

					case 0x4D53454EUL:

						if (context.type == SOUND || context.type == UNKNOWN)
							context.image = new Nsf (context);

						break;
				}

				return context.image ? RESULT_OK : RESULT_ERR_INVALID_FILE;
			}
			catch (Result r)
			{
				result = r;
			}
			catch (const std::bad_alloc&)
			{
				result = RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				result = RESULT_ERR_GENERIC;
			}

			Unload( context.image );

			return result;
		}

		void Image::Unload(Image*& image)
		{
			delete image;
			image = NULL;
		}

		uint Image::GetDesiredController(uint port) const
		{
			switch (port)
			{
				case Api::Input::PORT_1: return Api::Input::PAD1;
				case Api::Input::PORT_2: return Api::Input::PAD2;
				default: return Api::Input::UNCONNECTED;
			}
		}

		uint Image::GetDesiredAdapter() const
		{
			return Api::Input::ADAPTER_NES;
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
