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

#include <cstring>
#include <new>
#include "../NstMapper.hpp"
#include "../NstSoundWave.hpp"
#include "../api/NstApiSound.hpp"
#include "NstMapper086.hpp"
	   
namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		class Mapper86::Sound : public SoundWave
		{
		public:

			static Sound* Create(Cpu&);

			void Reset();
			void LoadState(State::Loader&);
			void SaveState(State::Saver&) const;

		private:

			Sound(Cpu& c)
			: SoundWave(c) {}

			class Loader;
			friend class Loader;

			enum
			{
				NUM_SLOTS = 16
			};

			NST_COMPILE_ASSERT( NUM_SLOTS == Core::Sound::Loader::MOERO_PRO_YAKYUU_SAMPLES );

			NES_DECL_PEEK( 7000 )
			NES_DECL_POKE( 7000 )

			struct Slot
			{
				const u8* data;
				dword length;
				dword rate;

				Slot() : data(NULL) {}
				~Slot() { delete [] data; }
			};

			uint reg;
			Slot slots[NUM_SLOTS];
		};

		class Mapper86::Sound::Loader : public Core::Sound::Loader
		{		
			Cpu& cpu;
			Sound* sound;

		public:

			Loader(Cpu& c)
			: cpu(c), sound(NULL) {}

			~Loader()
			{
				delete sound;
			}

			Sound* Get()
			{
				if (Sound* const tmp = sound)
				{
					for (uint i=0; i < NUM_SLOTS; ++i)
					{
						if (tmp->slots[i].data)
						{
							sound = NULL;
							return tmp;
						}
					}
				}

				return NULL;
			}

			Result Load(uint slot,const u8* input,dword length,dword rate)
			{
				Result result;
				u8* data;

				if (slot >= NUM_SLOTS || (sound && sound->slots[slot].data))
				{
					return RESULT_ERR_INVALID_PARAM;
				}
				else if (NES_FAILED(result=CanDo( input, length, rate )))
				{
					return result;
				}
				else if (sound == NULL && NULL == (sound = new (std::nothrow) Sound( cpu )) || NULL == (data = new (std::nothrow) u8 [length]))
				{
					return RESULT_ERR_OUT_OF_MEMORY;
				}

				std::memcpy( data, input, length );

				sound->slots[slot].data = data;
				sound->slots[slot].length = length;
				sound->slots[slot].rate = rate;

				return RESULT_OK;
			}
		};

		Mapper86::Sound* Mapper86::Sound::Create(Cpu& cpu)
		{
			Loader loader( cpu );
			Core::Sound::Loader::loadCallback( Core::Sound::Loader::MOERO_PRO_YAKYUU, loader );
			return loader.Get();
		}

		Mapper86::Mapper86(Context& c)
		: 
		Mapper (c,WRAM_NONE),
		sound  (c.pRomCrc == 0xDB53A88DUL || c.pRomCrc == 0x93B9B15CUL ? Sound::Create(c.cpu) : NULL)
		{
		}

		Mapper86::~Mapper86()
		{
			delete sound;
		}

		void Mapper86::Sound::Reset()
		{
			reg = 0x10;
			cpu.Map( 0x7000U ).Set( this, &Sound::Peek_7000, &Sound::Poke_7000 );

			SoundWave::Reset();
		}

		void Mapper86::SubReset(bool)
		{
			Map( 0x6000U, &Mapper86::Poke_6000 );

			if (sound)
				sound->Reset();
		}

		void Mapper86::Sound::LoadState(State::Loader& state)
		{
			Stop();
			reg = 0x10;

			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					reg = state.Read8();

				state.End();
			}
		}

		void Mapper86::Sound::SaveState(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( reg ).End();
		}

		void Mapper86::SubSave(State::Saver& state) const
		{
			if (sound)
				sound->SaveState( State::Saver::Subset(state,'S','N','D','\0').Ref() );
		}
		
		void Mapper86::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('S','N','D','\0'))
				{
					NST_VERIFY( sound );

					if (sound)
						sound->LoadState( state );
				}

				state.End();
			}
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper86,6000)
		{
			ppu.Update();
	
			prg.SwapBank<SIZE_32K,0x0000U>( (data & 0x30) >> 4 );
			chr.SwapBank<SIZE_8K,0x0000U>( (data & 0x03) | ((data & 0x40) >> 4) );
		}

		NES_POKE(Mapper86::Sound,7000)
		{
			uint tmp = reg;
			reg = data;

			if ((tmp & 0x10) < (data & 0x10))
			{
				const Slot& slot = slots[data & 0x0F];

				if (slot.data)
					Play( slot.data, slot.length, slot.rate );
			}
		}

		NES_PEEK(Mapper86::Sound,7000)
		{
			return 0x70;
		}
	}
}
