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

#ifndef NST_IO_WAVE_H
#define NST_IO_WAVE_H

#pragma once

#include "resource/resource.h"
#include "NstString.hpp"
#include <Windows.h>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#include <MMSystem.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace Nestopia
{
	namespace Io
	{
		class Wave : Sealed
		{
		public:

			Wave();
			~Wave();

			enum Exception
			{
				ERR_OPEN = IDS_WAVE_ERR_OPEN,
				ERR_WRITE = IDS_WAVE_ERR_WRITE,
				ERR_FINALIZE = IDS_WAVE_ERR_FINALIZE
			};

			void Open(const GenericString,const WAVEFORMATEX&);
			void Write(const void*,uint);
			void Close();

		private:

			enum
			{
				OPEN_FLAGS = MMIO_ALLOCBUF|MMIO_READWRITE|MMIO_CREATE|MMIO_EXCLUSIVE
			};

			void Abort();

			void CreateChunk(MMCKINFO&,FOURCC,uint=0) const;
			void AscendChunk(MMCKINFO&) const;

			template<typename T> 
			void WriteChunk(const T&,int=sizeof(T)) const;

			HMMIO handle;
			MMCKINFO chunkRiff;
			MMCKINFO chunkData;
			MMIOINFO output;
			Path fileName;

		public:

			ibool IsOpen() const
			{
				return handle != NULL;
			}

			const Path& GetName() const
			{
				return fileName;
			}
		};
	}
}

#endif
