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

#ifndef NST_CARTRIDGE_H
#define NST_CARTRIDGE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstMemory.hpp"
#include "NstImage.hpp"
#include "api/NstApiCartridge.hpp"

namespace Nes
{
	namespace Core
	{
		class Mapper;
		class Cpu;
		class Ppu;
		class ImageDatabase;
		class VsSystem;

		class Cartridge : public Image
		{
		public:

			Cartridge(Context&,Result&);
			~Cartridge();

			typedef Api::Cartridge::Info Info;

			void Reset(bool);

			void LoadState(State::Loader&);
			void SaveState(State::Saver&) const;

			void VSync();

			Mode GetMode() const;
			void BeginFrame(Input::Controllers*);

			static const void* SearchDatabase(const ImageDatabase&,const void*,ulong,ulong);

		private:

			void DetectVS();
			bool DetectEncryption();
			void DetectControllers();

			bool InitInfo(const ImageDatabase*);
			void ResetWRam();

			void LoadBattery();
			Result SaveBattery() const;

			Mapper* mapper;
			VsSystem* vs;
			LinearMemory pRom;
			LinearMemory cRom;
			LinearMemory wRam;
			Info info;
			ibool wRamAuto;
			mutable dword batteryCrc;

		public:

			Result Flush() const
			{
				return SaveBattery();
			}

			const Info& GetInfo() const
			{ 
				return info; 
			}

			Mapper& GetMapper()
			{
				NST_ASSERT( mapper );
				return *mapper; 
			}

			const Mapper& GetMapper() const
			{ 
				NST_ASSERT( mapper );
				return *mapper; 
			}

			VsSystem* GetVsSystem()
			{ 
				return vs; 
			}

			const VsSystem* GetVsSystem() const
			{ 
				return vs; 
			}

			dword GetPrgCrc() const
			{
				return info.pRomCrc;
			}

			bool IsVS() const
			{ 
				return vs != NULL; 
			}
		};
	}
}

#endif
