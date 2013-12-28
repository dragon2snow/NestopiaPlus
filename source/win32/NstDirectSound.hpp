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

#ifndef NST_DIRECTX_DIRECTSOUND_H
#define NST_DIRECTX_DIRECTSOUND_H

#pragma once

#include <vector>
#include "NstObjectPod.hpp"
#include "NstDirectX.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#include <MMSystem.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <dsound.h>

namespace Nestopia
{
	namespace DirectX
	{
		class DirectSound
		{
		public:

			explicit DirectSound(HWND);
			~DirectSound();

			typedef BaseAdapter Adapter;
			typedef std::vector<Adapter> Adapters;

			enum Channels
			{
				MONO = 1,
				STEREO
			};

			enum Pool
			{
				POOL_HARDWARE,
				POOL_SYSTEM
			};

			tstring Update(uint,uint,uint,Channels,uint,Pool,ibool);
			void    Destroy();

		private:

			static BOOL CALLBACK EnumAdapter(LPGUID,LPCTSTR,LPCTSTR,LPVOID);

			class Buffer
			{
			public:

				Buffer();
				~Buffer();

				tstring Update(IDirectSound8&,bool,uint,uint,Channels,uint,Pool,bool);
				void    StartStream();
				ibool   LockStream(void**,uint*);
				void    StopStream(IDirectSound8*,bool);
				void    Release();

			private:

				tstring Create(IDirectSound8&,bool);

				enum
				{
					DC_OFFSET_8 = 0x80,
					DC_OFFSET_16 = 0x0000
				};

				struct Settings
				{
					Settings();

					uint size;
					Pool pool;
					ibool globalFocus;
				};

				ComInterface<IDirectSoundBuffer8> com;
				uint writeOffset;
				Object::Pod<WAVEFORMATEX> waveFormat;
				Settings settings;

			public:

				const WAVEFORMATEX& GetWaveFormat() const
				{
					return waveFormat;
				}

				ibool GlobalFocus() const
				{
					return settings.globalFocus;
				}

				void UnlockStream(void** data,uint* size) const
				{
					NST_ASSERT( data && com != NULL );
					com->Unlock( data[0], size[0]*waveFormat.nBlockAlign, data[1], size[1]*waveFormat.nBlockAlign );
				}

				ibool Streaming() const
				{
					DWORD status;

					return
					(
						com != NULL && SUCCEEDED(com->GetStatus( &status )) &&
						(status & (DSBSTATUS_BUFFERLOST|DSBSTATUS_PLAYING|DSBSTATUS_LOOPING)) == (DSBSTATUS_PLAYING|DSBSTATUS_LOOPING)
					);
				}
			};

			struct Settings
			{
				Settings(HWND);

				HWND const hWnd;
				u16 deviceId;
				bool priority;
				bool buggyDriver;
			};

			ComInterface<IDirectSound8> device;
			Buffer buffer;
			Settings settings;
			Adapters adapters;

		public:

			ibool Streaming() const
			{
				return buffer.Streaming();
			}

			void StopStream()
			{
				buffer.StopStream( settings.buggyDriver ? *device : NULL, settings.priority );
			}

			void StartStream()
			{
				buffer.StartStream();
			}

			ibool LockStream(void** data,uint* size)
			{
				return buffer.LockStream( data, size );
			}

			void UnlockStream(void** data,uint* size) const
			{
				buffer.UnlockStream( data, size );
			}

			const Adapters& GetAdapters() const
			{
				return adapters;
			}

			const WAVEFORMATEX& GetWaveFormat() const
			{
				return buffer.GetWaveFormat();
			}

			ibool GlobalFocus() const
			{
				return buffer.GlobalFocus();
			}
		};
	}
}

#endif
