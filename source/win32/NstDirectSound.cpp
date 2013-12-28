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

#ifdef _MSC_VER
#pragma comment(lib,"dsound")
#endif

#include "NstIoLog.hpp"
#include "NstDirectSound.hpp"
#include <Shlwapi.h>

namespace Nestopia
{
	namespace DirectX
	{
		DirectSound::Settings::Settings(HWND h)
		: hWnd(h), deviceId(0), priority(false), buggyDriver(false) {}

		DirectSound::DirectSound(HWND const hWnd)
		: settings( hWnd )
		{
			Io::Log() << "DirectSound: initializing..\r\n";

			if (FAILED(::DirectSoundEnumerate( EnumAdapter, &adapters )) || adapters.empty())
				EnumAdapter( NULL, _T("Primary Sound Driver"), NULL, &adapters );

			for (Adapters::const_iterator it(adapters.begin()), end(adapters.end()); it != end; ++it)
			{
				if (::StrStrI( it->name.Ptr(), _T("E-DSP Wave") ))
				{
					settings.buggyDriver = true;
					Io::Log() << "DirectSound: warning, possibly buggy drivers!! activating stupid-mode..\r\n";
					break;
				}
			}
		}

		DirectSound::~DirectSound()
		{
			Destroy();
		}

		void DirectSound::Destroy()
		{
			buffer.Release();
			device.Release();
		}

		BOOL CALLBACK DirectSound::EnumAdapter(LPGUID guid,LPCTSTR desc,LPCTSTR,LPVOID context)
		{
			Io::Log() << "DirectSound: enumerating device - name: "
                      << (desc && *desc ? desc : _T("unknown"))
                      << ", GUID: "
                      << (guid ? System::Guid( *guid ).GetString() : _T("unspecified"))
                      << "\r\n";

			ComInterface<IDirectSound8> device;

			if (SUCCEEDED(::DirectSoundCreate8( guid, &device, NULL )))
			{
				static_cast<Adapters*>(context)->resize( static_cast<Adapters*>(context)->size() + 1 );
				Adapter& adapter = static_cast<Adapters*>(context)->back();

				if (guid)
					adapter.guid = *guid;

				adapter.name = desc;
			}
			else
			{
				Io::Log() << "DirectSound: DirectSoundCreate8() failed on this device, continuing enumeration..\r\n";
			}

			return true;
		}

		tstring DirectSound::Update
		(
			const uint deviceId,
			const uint rate,
			const uint bits,
			const Channels channels,
			const uint latency,
			const Pool pool,
			const ibool globalFocus
		)
		{
			NST_ASSERT( deviceId < adapters.size() );

			if (settings.deviceId != deviceId || device == NULL)
			{
				settings.deviceId = deviceId;

				Destroy();

				if (FAILED(::DirectSoundCreate8( &adapters[deviceId].guid, &device, NULL )))
					return _T("::DirectSoundCreate8() failed!");

				Io::Log() << "DirectSound: creating device #" << deviceId << "\r\n";

				settings.priority = SUCCEEDED(device->SetCooperativeLevel( settings.hWnd, DSSCL_PRIORITY ));

				if (!settings.priority)
				{
					Io::Log() << "DirectSound: warning, IDirectSound8::SetCooperativeLevel( DSSCL_PRIORITY ) failed! Retrying with DSSCL_NORMAL..\r\n";

					if (FAILED(device->SetCooperativeLevel( settings.hWnd, DSSCL_NORMAL )))
					{
						device.Release();
						return _T("IDirectSound8::SetCooperativeLevel() failed!");
					}
				}
			}

			if (tstring errMsg = buffer.Update( **device, settings.priority, rate, bits, channels, latency, pool, globalFocus ))
			{
				Destroy();
				return errMsg;
			}

			return NULL;
		}

		DirectSound::Buffer::Settings::Settings()
		: size(0), pool(POOL_HARDWARE), globalFocus(false) {}

		DirectSound::Buffer::Buffer()
		: writeOffset(0)
		{
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		}

		DirectSound::Buffer::~Buffer()
		{
			Release();
		}

		void DirectSound::Buffer::Release()
		{
			if (com != NULL)
			{
				com->Stop();
				com.Release();
			}
		}

		tstring DirectSound::Buffer::Create(IDirectSound8& device,const bool priority)
		{
			Release();

			Object::Pod<DSBUFFERDESC> desc;

			desc.dwSize = sizeof(desc);
			desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
			desc.lpwfxFormat = NULL;

			if (priority)
			{
				ComInterface<IDirectSoundBuffer> primary;

				if (FAILED(device.CreateSoundBuffer( &desc, &primary, NULL )) || FAILED(primary->SetFormat( &waveFormat )))
				{
					static bool logged = false;

					if (logged == false)
					{
						logged = true;
						Io::Log() << "DirectSound: warning, couldn't set the sample format for the primary sound buffer!\r\n";
					}
				}
			}

			NST_ASSERT( settings.size % waveFormat.nBlockAlign == 0 );

			desc.Clear();
			desc.dwSize = sizeof(desc);
			desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;

			if (settings.globalFocus)
				desc.dwFlags |= DSBCAPS_GLOBALFOCUS;

			if (settings.pool == POOL_SYSTEM)
				desc.dwFlags |= DSBCAPS_LOCSOFTWARE;

			desc.dwBufferBytes = settings.size;
			desc.lpwfxFormat = &waveFormat;

			ComInterface<IDirectSoundBuffer> oldCom;

			if (FAILED(device.CreateSoundBuffer( &desc, &oldCom, NULL )))
			{
				if (!(desc.dwFlags & DSBCAPS_LOCSOFTWARE))
				{
					desc.dwFlags |= DSBCAPS_LOCSOFTWARE;

					static bool logged = false;

					if (logged == false)
					{
						logged = true;
						Io::Log() << "DirectSound: warning, couldn't create the sound buffer! Retrying with software buffers..\r\n";
					}

					if (FAILED(device.CreateSoundBuffer( &desc, &oldCom, NULL )))
						return _T("IDirectSound8::CreateSoundBuffer() failed! Sound will be disabled!");
				}
			}

			if (FAILED(oldCom->QueryInterface( IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&com) )))
				return _T("IDirectSoundBuffer::QueryInterface() failed! Sound will be disabled!");

			return NULL;
		}

		tstring DirectSound::Buffer::Update
		(
			IDirectSound8& device,
			const bool priority,
			const uint rate,
			const uint bits,
			const Channels channels,
			const uint latency,
			const Pool pool,
			const bool globalFocus
		)
		{
			const uint size = rate * latency / 1000 * (bits / 8 * channels);

			if
			(
				com != NULL &&
				waveFormat.nSamplesPerSec == rate &&
				waveFormat.wBitsPerSample == bits &&
				waveFormat.nChannels == channels &&
				settings.size == size &&
				settings.pool == pool &&
				bool(settings.globalFocus) == globalFocus
			)
				return NULL;

			waveFormat.nSamplesPerSec = rate;
			waveFormat.wBitsPerSample = bits;
			waveFormat.nChannels = channels;
			waveFormat.nBlockAlign = waveFormat.wBitsPerSample / 8 * waveFormat.nChannels;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

			settings.size = size;
			settings.pool = pool;
			settings.globalFocus = globalFocus;

			return Create( device, priority );
		}

		void DirectSound::Buffer::StartStream()
		{
			if (com != NULL)
			{
				DWORD status;

				if (FAILED(com->GetStatus( &status )))
				{
					com.Release();
					return;
				}

				if ((status & (DSBSTATUS_BUFFERLOST|DSBSTATUS_LOOPING|DSBSTATUS_PLAYING)) == (DSBSTATUS_LOOPING|DSBSTATUS_PLAYING))
					return;

				if (status & DSBSTATUS_PLAYING)
					com->Stop();

				if (status & DSBSTATUS_BUFFERLOST)
				{
					const HRESULT hResult = com->Restore();

					if (FAILED(hResult) && hResult != DSERR_BUFFERLOST)
						com.Release();

					return;
				}

				void* data;
				DWORD size;

				if (SUCCEEDED(com->Lock( 0, 0, &data, &size, NULL, NULL, DSBLOCK_ENTIREBUFFER )))
				{
					std::memset( data, waveFormat.wBitsPerSample == 16 ? DC_OFFSET_16 : DC_OFFSET_8, size );
					com->Unlock( data, size, NULL, 0 );
				}

				writeOffset = 0;
				com->SetCurrentPosition( 0 );
				const HRESULT hResult = com->Play( 0, 0, DSBPLAY_LOOPING );

				if (FAILED(hResult) && hResult != DSERR_BUFFERLOST)
					com.Release();
			}
		}

		void DirectSound::Buffer::StopStream(IDirectSound8* const device,const bool priority)
		{
			if (com != NULL)
			{
				if (device)
					Create( *device, priority );
				else
					com->Stop();
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		ibool DirectSound::Buffer::LockStream(void** data,uint* size)
		{
			DWORD pos;

			if (SUCCEEDED(com->GetCurrentPosition( &pos, NULL )))
			{
				pos = (pos > writeOffset ? pos - writeOffset : pos + settings.size - writeOffset);

				DWORD bytes[2];

				if (SUCCEEDED(com->Lock( writeOffset, pos, data+0, bytes+0, data+1, bytes+1, 0 )))
				{
					writeOffset = (writeOffset + pos) % settings.size;

					size[0] = bytes[0] / waveFormat.nBlockAlign;
					size[1] = bytes[1] / waveFormat.nBlockAlign;

					return true;
				}
			}

			com->Stop();

			NST_DEBUG_MSG("DirectSound::Buffer::Lock() failed!");

			return false;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
