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
		: type(t) 
		{
		}

		Image::~Image() 
		{
		}

		Result Image::Load(Context& context)
		{
			NST_ASSERT( context.image == NULL );

			Unload( context.image );

			try
			{
				switch (Stream::In(context.stream).Peek32())
				{
					case 0x1A53454EUL: // ines
					case 0x46494E55UL: // unif
				
						if (context.type == CARTRIDGE || context.type == UNKNOWN)
						{
							Result result = RESULT_OK;
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
				
					default: throw RESULT_ERR_INVALID_FILE;
				}
			}
			catch (Result result)
			{
				Unload( context.image );
				return result;
			}
			catch (std::bad_alloc&)
			{
				Unload( context.image );
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				Unload( context.image );
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		void Image::Unload(Image*& image)
		{
			delete image;
			image = NULL;
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
