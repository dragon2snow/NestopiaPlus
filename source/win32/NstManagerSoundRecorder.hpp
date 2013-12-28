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

#include "NstObjectPod.hpp"
#include "NstIoWave.hpp"

namespace Nestopia
{
	namespace Managers
	{
		class Sound::Recorder
		{
		public:

			Recorder(Window::Menu&,Window::Sound::Recorder&,Emulator&);
			~Recorder();

			void Enable(const WAVEFORMATEX&);
			void Disable();

			NST_NO_INLINE void Flush(const Nes::Sound::Output&);

		private:

			void UpdateMenu() const;
			void Close();

			void OnEmuEvent(Emulator::Event);
			ibool CanRecord() const;

			void OnCmdFile   (uint);
			void OnCmdRecord (uint);
			void OnCmdStop   (uint);
			void OnCmdRewind (uint);

			enum
			{
				ONE_MB = 0x100000,
				SMALL_SIZE = ONE_MB,
				BIG_SIZE = ONE_MB * 200
			};

			ibool recording;
			Io::Wave file;
			uint size;
			uint nextSmallSizeNotification;
			uint nextBigSizeNotification;
			Window::Sound::Recorder& dialog;
			const Window::Menu& menu;
			Emulator& emulator;
			Object::Pod<WAVEFORMATEX> waveFormat;

		public:

			ibool IsRecording() const
			{
				return recording;
			}
		};
	}
}
