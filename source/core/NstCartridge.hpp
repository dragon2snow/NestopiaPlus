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

#ifndef NST_CARTRIDGE_H
#define NST_CARTRIDGE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstRam.hpp"
#include "NstImage.hpp"
#include "api/NstApiCartridge.hpp"
#include "NstChecksumMd5.hpp"

namespace Nes
{
	namespace Core
	{
		class Mapper;
		class Cpu;
		class Ppu;
		class ImageDatabase;
		class VsSystem;

		namespace Peripherals
		{
			class TurboFile;
			class DataRecorder;
		}

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
			uint GetDesiredAdapter() const;
			ExternalDevice QueryExternalDevice(ExternalDeviceType);
			PpuType QueryPpu(bool);

			static const void* SearchDatabase(const ImageDatabase&,const void*,ulong);

			class Ines;
			class Unif;

		private:

			void Destroy();
			void DetectControllers(uint);
			void ResetWrkRam(uint);

			void LoadBattery();
			Result SaveBattery(bool) const;

			Result Flush(bool,bool) const;

			struct Wrk : Ram
			{
				enum
				{
					MIN_BATTERY_SIZE = SIZE_1K,
					GARBAGE = 0xFF
				};

				mutable Checksum::Md5::Key ramCheckSum;
			};

			Mapper* mapper;
			VsSystem* vs;
			Peripherals::TurboFile* turboFile;
			Peripherals::DataRecorder* dataRecorder;
			Ram prg;
			Ram chr;
			Wrk wrk;
			Info info;

		public:

			const Info& GetInfo() const
			{
				return info;
			}

			dword GetPrgCrc() const
			{
				return info.prgCrc;
			}

			bool IsVS() const
			{
				return vs != NULL;
			}
		};
	}
}

#endif
