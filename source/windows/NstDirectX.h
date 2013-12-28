////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#pragma once

#ifndef NST_DIRECTX_H
#define NST_DIRECTX_H

#include <Windows.h>
#include "../paradox/PdxLibrary.h"
#include "../NstNes.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

namespace DIRECTX
{
	template<class T> ULONG Release(T&,const BOOL=FALSE);
	template<class T> VOID InitStruct(T&);

	template<class T>
	ULONG Release(T& com,const BOOL last)
	{
		ULONG RefCount = 0;

		if (com)
		{
			RefCount = com->Release();

			if (last && RefCount)
			{
				while (com->Release());
				PDX_DEBUG_BREAK_MSG("One of the DirectX COM interfaces was not properly released!");
			}

			com = NULL;
		}

		return RefCount;
	}

	template<class T>
	VOID InitStruct(T& desc)
	{
		PDXMemZero( desc );
		desc.dwSize = sizeof(desc);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#include "NstDirectDraw.h"
#include "NstDirectSound.h"
#include "NstDirectInput.h"

#endif
