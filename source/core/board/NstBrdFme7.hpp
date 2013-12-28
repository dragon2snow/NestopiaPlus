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

#ifndef NST_BOARDS_FME7_H
#define NST_BOARDS_FME7_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Fme7 : public Mapper
			{
			public:

				class Sound : public Apu::Channel
				{
				public:

					Sound(Cpu&,bool=true);
					~Sound();

					void Poke_E000(uint);

					void LoadState(State::Loader&);
					void SaveState(State::Saver&) const;

				protected:

					void Reset();
					void UpdateContext(uint,const u8 (&)[MAX_CHANNELS]);
					Sample GetSample();

				private:

					enum
					{
						NUM_SQUARES = 3
					};

					class Envelope
					{
					public:

						Envelope();

						void Reset(uint);
						void UpdateContext(uint);
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&,uint);

						void WriteReg0(uint,uint);
						void WriteReg1(uint,uint);
						void WriteReg2(uint);

						NST_FORCE_INLINE dword Clock(Cycle);

					private:

						void UpdateFrequency(uint);

						u8    holding;
						u8    hold;
						u8    alternate;
						u8    attack;
						iword timer;
						dword frequency;
						uint  count;
						uint  volume;
						uint  length;
					};

					class Noise
					{
					public:

						Noise();

						void Reset(uint);
						void UpdateContext(uint);
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&,uint);

						void WriteReg(uint,uint);

						NST_FORCE_INLINE dword Clock(Cycle);

					private:

						void UpdateFrequency(uint);

						iword timer;
						dword frequency;
						dword rng;
						dword dc;
						uint length;
					};

					class Square
					{
					public:

						Square();

						void Reset(uint);
						void UpdateContext(uint);
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&,uint);

						void WriteReg0(uint,uint);
						void WriteReg1(uint,uint);
						void WriteReg2(uint);
						void WriteReg3(uint);

						NST_FORCE_INLINE dword GetSample(Cycle,uint,uint);

					private:

						void UpdateFrequency(uint);

						iword timer;
						dword frequency;
						uint  status;
						uint  ctrl;
						uint  volume;
						dword dc;
						uint  length;
					};

					Apu& apu;
					uint regSelect;
					ibool active;
					Envelope envelope;
					Noise noise;
					Square squares[NUM_SQUARES];
					Apu::DcBlocker dcBlocker;
					const ibool hooked;

					static const u16 levels[32];

				public:

					void Poke_C000(uint data)
					{
						regSelect = data;
					}
				};

			protected:

				Fme7(Context&);
				~Fme7();

			private:

				class BarcodeWorld;

				enum
				{
					ATR_BARCODE_READER = 1
				};

				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				Device QueryDevice(DeviceType);
				void VSync();

				NES_DECL_POKE( 8000  )
				NES_DECL_POKE( A000  )
				NES_DECL_POKE( C000  )
				NES_DECL_POKE( E000  )

				struct Irq
				{
					void Reset(bool);
					ibool Signal();

					uint count;
					ibool enabled;
				};

				uint command;
				Clock::M2<Irq> irq;
				Sound sound;
				BarcodeWorld* const barcodeWorld;
			};
		}
	}
}

#endif
