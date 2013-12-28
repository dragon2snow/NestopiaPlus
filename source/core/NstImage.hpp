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

#ifndef NST_IMAGE_H
#define NST_IMAGE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			class Controllers;
		}

		namespace State
		{
			class Loader;
			class Saver;
		}

		class ImageDatabase;
		class BarcodeReader;
		class Cpu;
		class Ppu;

		class NST_NO_VTABLE Image
		{
		public:

			enum Type
			{
				UNKNOWN   = 0x0,
				CARTRIDGE = 0x1,
				DISK      = 0x2,
				SOUND     = 0x4
			};

			typedef void* ExternalDevice;

			enum ExternalDeviceType
			{
				EXT_DIP_SWITCHES = 1,
				EXT_DATA_RECORDER,
				EXT_BARCODE_READER,
				EXT_TURBO_FILE
			};

			struct Context
			{
				Image*& image;
				const Type type;
				Cpu& cpu;
				Ppu& ppu;
				StdStream const stream;
				const ImageDatabase* const database;

				Context(Image*& i,Type t,Cpu& c,Ppu& p,StdStream s,const ImageDatabase* d=NULL)
				: image(i), type(t), cpu(c), ppu(p), stream(s), database(d)	{}
			};

			static Result Load(Context&);
			static void Unload(Image*&);

			virtual void Reset(bool) = 0;

			virtual Result Flush() const
			{ 
				return RESULT_OK; 
			}

			virtual void VSync() {}

			virtual void LoadState(State::Loader&) {}
			virtual void SaveState(State::Saver&) const {}
			virtual void BeginFrame(Input::Controllers*) {}
			virtual	uint GetDesiredController(uint) const;

			virtual Mode GetMode() const = 0;
			virtual void SetMode(Mode) {}

			virtual dword GetPrgCrc() const
			{
				return 0;
			}

			virtual ExternalDevice QueryExternalDevice(ExternalDeviceType)
			{
				return NULL;
			}

		protected:

			Image(Type);
			virtual ~Image();

		private:

			const Type type;

		public:

			Type GetType() const
			{
				return type;
			}
		};
	}
}

#endif
