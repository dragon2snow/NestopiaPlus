////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include "NstRam.hpp"
#include "NstImage.hpp"
#include "NstFile.hpp"
#include "api/NstApiCartridge.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper;

		namespace Peripherals
		{
			class TurboFile;
			class DataRecorder;
		}

		class Cartridge : public Image
		{
		public:

			explicit Cartridge(Context&);

			void BeginFrame(const Api::Input&,Input::Controllers*);

			typedef Api::Cartridge::Info Info;

			class Ines;
			class Unif;

		private:

			~Cartridge();

			class VsSystem;

			void Reset(bool);
			bool PowerOff();
			void LoadState(State::Loader&);
			void SaveState(State::Saver&,dword) const;
			void Destroy();
			void VSync();

			Region::Type GetRegion() const;
			uint GetDesiredController(uint) const;
			uint GetDesiredAdapter() const;
			void DetectControllers(uint);
			ExternalDevice QueryExternalDevice(ExternalDeviceType);
			Revision::Ppu QueryPpu(bool);

			class Wrk : public Ram
			{
				enum
				{
					GARBAGE = 0x00,
					MIN_BATTERY_SIZE = SIZE_1K
				};

				File file;

			public:

				void Reset(const Info&,bool);
				void Load(const Info&);
				void Save(const Info&) const;
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
		};
	}
}

#endif
