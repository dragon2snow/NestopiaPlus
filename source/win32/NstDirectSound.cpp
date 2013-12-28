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

#include "NstDirectSound.hpp"
#include "NstApplicationException.hpp"
#include "NstWindowStruct.hpp"
#include "NstIoLog.hpp"

namespace Nestopia
{
	using DirectX::DirectSound;

	DirectSound::Settings::Settings(HWND h)
	: hWnd(h), deviceId(0), priority(FALSE) {}

	DirectSound::DirectSound(HWND const hWnd)
	: settings( hWnd )
	{
		Io::Log() << "DirectSound: initializing..\r\n";

		if (FAILED(::DirectSoundEnumerate( EnumAdapter, &adapters )) || adapters.empty())
			EnumAdapter( NULL, _T("Primary Sound Driver"), NULL, &adapters );
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

		return TRUE;
	}

	tstring DirectSound::Update
	(
     	const uint deviceId,
		const uint rate,
		const uint bits,
		const Channels channels,
		const uint speed,
		const uint latency
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

		return buffer.Update( *device, settings.priority, rate, bits, channels, speed, latency );
	}

	tstring DirectSound::UpdateSpeed(const uint speed,const uint latency)
	{
		NST_ASSERT( device != NULL );
		return buffer.UpdateSpeed( *device, settings.priority, speed, latency );
	}

	DirectSound::Buffer::Settings::Settings()
	: size(0) {}

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
		if (com)
		{
			com->Stop();
			com.Release();
		}
	}

	tstring DirectSound::Buffer::Create(IDirectSound8& device,const ibool priority)
	{
		Release();

		Window::Struct<DSBUFFERDESC> desc;

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
		desc.dwFlags = DSBCAPS_LOCSOFTWARE|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_GLOBALFOCUS;
		desc.dwBufferBytes = settings.size * 2;
		desc.lpwfxFormat = &waveFormat;

		{
			ComInterface<IDirectSoundBuffer> oldie;

			if (FAILED(device.CreateSoundBuffer( &desc, &oldie, NULL )))
				return _T("IDirectSound8::CreateSoundBuffer() failed!");

			if (FAILED(oldie->QueryInterface( IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&com) )))
				return _T("IDirectSoundBuffer::QueryInterface() failed!");
		}

		return NULL;
	}

	uint DirectSound::Buffer::CalculateSize(uint rate,uint block,uint speed,uint latency)
	{
		uint size = rate * block * latency / speed;

		if (size % block)
			size += block - size % block;

		return size;
	}

	tstring DirectSound::Buffer::Update
	(
		IDirectSound8& device,
		const ibool priority,
		const uint rate,
		const uint bits,		
		const Channels channels,
		const uint speed,
		const uint latency
	)
	{	
		const uint size = CalculateSize( rate, bits / 8 * channels, speed, latency );

		if 
		(
		    com &&
     		waveFormat.nSamplesPerSec == rate &&
     		waveFormat.wBitsPerSample == bits &&
     		waveFormat.nChannels == channels &&
			settings.size == size
		)
			return NULL;

		waveFormat.nSamplesPerSec = rate; 
		waveFormat.wBitsPerSample = (WORD) bits; 
		waveFormat.nChannels = (WORD) channels;
		waveFormat.nBlockAlign = (WORD) (waveFormat.wBitsPerSample / 8 * waveFormat.nChannels);
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

		settings.size = size;

		return Create( device, priority );
	}

	tstring DirectSound::Buffer::UpdateSpeed
	(
     	IDirectSound8& device,
		const ibool priority,
		const uint speed,
		const uint latency
	)
	{
		const uint size = CalculateSize( waveFormat.nSamplesPerSec, waveFormat.nBlockAlign, speed, latency );

		if (com == NULL || settings.size != size)
		{
			settings.size = size;
			return Create( device, priority );
		}

		return NULL;
	}

	void DirectSound::Buffer::StartStream()
	{
		if (com)
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

				if (FAILED(hResult))
				{
					if (hResult != DSERR_BUFFERLOST)
						com.Release();

					return;
				}
			}							
			
			void* data;
			DWORD size;

			if (SUCCEEDED(com->Lock( 0, 0, &data, &size, NULL, NULL, DSBLOCK_ENTIREBUFFER )))
			{
				std::memset( data, waveFormat.wBitsPerSample == 16 ? DC_OFFSET_16 : DC_OFFSET_8, size );
				com->Unlock( data, size, NULL, 0 );
			}

			writeOffset = 0;
			const HRESULT hResult = com->Play( 0, 0, DSBPLAY_LOOPING );

			if (FAILED(hResult) && hResult != DSERR_BUFFERLOST)
				com.Release();
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	ibool DirectSound::Buffer::LockStream(void*& data,uint& size)
	{
		NST_ASSERT( com != NULL );

		DWORD pos;
		HRESULT hResult = com->GetCurrentPosition( NULL, &pos );

		if (SUCCEEDED(hResult))
		{
			if (uint(pos >= settings.size) == writeOffset)
				return FALSE;
			
			hResult = com->Lock( writeOffset ? settings.size : 0, settings.size, &data, &pos, NULL, NULL, 0 );

			if (SUCCEEDED(hResult) && pos == settings.size)
			{
				NST_ASSERT( pos % waveFormat.nBlockAlign == 0 );

				size = pos / waveFormat.nBlockAlign;
				writeOffset ^= 1;

				return TRUE;
			}
		}

		com->Stop();
		NST_DEBUG_MSG("DirectSound::Buffer::Lock() failed!");

		return FALSE;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif	
}
