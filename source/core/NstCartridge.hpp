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

#ifndef NST_CARTRIDGE_H
#define NST_CARTRIDGE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstMemory.hpp"
#include "NstImage.hpp"
#include "api/NstApiCartridge.hpp"
#include "NstMd5.hpp"

namespace Nes
{
	namespace Core
	{
		class Mapper;
		class Cpu;
		class Ppu;
		class ImageDatabase;
		class VsSystem;
		class TurboFile;
		class DataRecorder;

		class Cartridge : public Image
		{
		public:

			Cartridge(Context&,Result&);
			~Cartridge();

			typedef Api::Cartridge::Info Info;

			void Reset(bool);
			void LoadState(State::Loader&);
			void SaveState(State::Saver&) const;
			void BeginFrame(const Api::Input&,Input::Controllers*);
			void VSync();
			
			Mode GetMode() const;
			uint GetDesiredController(uint) const;						
			ExternalDevice QueryExternalDevice(ExternalDeviceType);

			static const void* SearchDatabase(const ImageDatabase&,const void*,ulong,ulong);

		private:

			void Destroy();
			void DetectControllers();
			void DetectVS();
			void DetectTurboFile(Context&);
			bool DetectEncryption();

			bool InitInfo(const ImageDatabase*);
			void ResetWRam();

			void LoadBattery();
			Result SaveBattery() const;

			Mapper* mapper;
			VsSystem* vs;
			TurboFile* turboFile;
			DataRecorder* dataRecorder;
			LinearMemory pRom;
			LinearMemory cRom;
			LinearMemory wRam;
			Info info;
			ibool wRamAuto;
			mutable Md5::Key batteryCheckSum;

		public:

			Result Flush() const
			{
				return SaveBattery();
			}

			const Info& GetInfo() const
			{ 
				return info; 
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
