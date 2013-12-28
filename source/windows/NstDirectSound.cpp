////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#include "NstDirectX.h"
#include "NstApplication.h"

#pragma comment(lib,"dsound")

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DIRECTSOUND::EnumAdapters(LPGUID guid,LPCSTR desc,LPCSTR,LPVOID context)
{
	LPDIRECTSOUND8 device;

	if (FAILED(DirectSoundCreate8(guid,&device,NULL)))
		return TRUE;

	DSCAPS caps;
	DIRECTX::InitStruct(caps);

	if (FAILED(device->GetCaps(&caps)))
	{
		device->Release();
		return TRUE;
	}

	device->Release();

	if (!(caps.dwFlags & DSCAPS_SECONDARY16BIT) && !((caps.dwFlags & DSCAPS_SECONDARY8BIT)))
		 return TRUE;

	PDX_CAST(ADAPTERS*,context)->Grow();
	ADAPTER& adapter = PDX_CAST(ADAPTERS*,context)->Back();

	if (guid) memcpy( &adapter.guid, guid, sizeof(GUID) );
	else      memset( &adapter.guid, 0x00, sizeof(GUID) );

	adapter.name = desc;

	if (adapter.name.IsEmpty())
		adapter.name = "unknown";

	adapter.SampleBits8 = bool(caps.dwFlags & DSCAPS_SECONDARY8BIT);
	adapter.SampleBits16 = bool(caps.dwFlags & DSCAPS_SECONDARY16BIT);

	const UINT rates[] = {11025,22050,44100,48000,96000,192000};

	for (UINT i=0; i < sizeof(rates) / sizeof(*rates); ++i)
	{
		if (rates[i] <= caps.dwMaxSecondarySampleRate && rates[i] >= caps.dwMinSecondarySampleRate)
			adapter.SampleRates.InsertBack( rates[i] );
	}

	if (!adapter.SampleRates.Size())
		PDX_CAST(ADAPTERS*,context)->EraseBack();

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTSOUND::DIRECTSOUND()
:
hWnd          (NULL),
device        (NULL),
secondary     (NULL),
NotifyOffset  (0),
LastOffset    (0)
{
	locked.data = NULL;
	locked.size = 0;

	PDXMemZero( WaveFormat );
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTSOUND::~DIRECTSOUND()
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Error(const CHAR* const msg)
{
	application.LogOutput(PDXSTRING("DIRECTSOUND: ") + msg);
	application.OnError( msg );

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
// initializer
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Initialize(HWND h)
{
	PDX_ASSERT(adapters.IsEmpty() && h);
	
	hWnd = h;

	application.LogOutput("DIRECTSOUND: Initializing");

	if (FAILED(DirectSoundEnumerate(EnumAdapters,PDX_CAST(LPVOID,&adapters))))
		return Error("DirectSoundEnumerate() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Create(const GUID* const guid)
{
	PDX_ASSERT(guid && hWnd);

	if (PDX_FAILED(Destroy()))
		return PDX_FAILURE;//Error("Failed to release the DirectSound interface!");

	PDX_ASSERT(!device);

	if (FAILED(DirectSoundCreate8(guid,&device,NULL)))
	{
		Destroy();
		return PDX_FAILURE;//application.OnWarning("DirectSoundCreate8() failed!");
	}

	if (FAILED(device->SetCooperativeLevel(hWnd,DSSCL_PRIORITY)))
	{
		Destroy();
		return PDX_FAILURE;//Error("IDirectSound8::SetCooperativeLevel() failed!");
	}

	LastOffset = 0;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::CreateBuffer(const UINT latency,const UINT fps,const BOOL volume)
{
	PDX_ASSERT(latency && latency <= 10);

	const DOUBLE rate = (fps == NES_FPS_NTSC ? NES_FPS_NTSC : NES_FPS_REAL_PAL);

	NotifySize = (WaveFormat.nSamplesPerSec * latency * WaveFormat.nBlockAlign) / rate;
	NotifySize -= NotifySize % WaveFormat.nBlockAlign;
	BufferSize  = NotifySize * 2;
	LastOffset  = 0;

	{
		PDXSTRING log;

		log  = "DIRECTSOUND: creating ";
		log += BufferSize;
		log += " byte sound buffer";
		application.LogOutput( log );

		log  = "DIRECTSOUND: sample rate - ";
		log += WaveFormat.nSamplesPerSec;
		application.LogOutput( log );

		log  = "DIRECTSOUND: sample bits - ";
		log += WaveFormat.wBitsPerSample;
		application.LogOutput( log );
	}

	DSBUFFERDESC desc;
	DIRECTX::InitStruct(desc);

	desc.dwFlags         = DSBCAPS_LOCSOFTWARE|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_GLOBALFOCUS;
	desc.dwBufferBytes   = BufferSize;
	desc.guid3DAlgorithm = GUID_NULL;
	desc.lpwfxFormat     = &WaveFormat;

	if (volume)
		desc.dwFlags |= DSBCAPS_CTRLVOLUME;

	if (secondary)
	{
		secondary->Stop();
		secondary->Release();
	}

	LPDIRECTSOUNDBUFFER oldfart;

	if (FAILED(device->CreateSoundBuffer(&desc,&oldfart,NULL)))
		return Error("IDirectSound8::CreateSoundBuffer() failed!");

	if (FAILED(oldfart->QueryInterface(IID_IDirectSoundBuffer8,PDX_CAST_PTR(LPVOID,secondary))))
	{
		oldfart->Release();
		return Error("IDirectSoundBuffer::QueryInterface() failed!");
	}

	oldfart->Release();

	PDX_TRY(ClearBuffer());

	if (FAILED(secondary->Play(0,0,DSBPLAY_LOOPING)))
		return Error("IDirectSoundBuffer8::Play() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTSOUND::SetVolume(const LONG volume)
{
	PDX_ASSERT(volume <= DSBVOLUME_MAX && volume >= DSBVOLUME_MIN);

	if (secondary)
		secondary->SetVolume(volume);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::RestoreBuffer(BOOL* const restored)
{
	DWORD status;

	if (FAILED(secondary->GetStatus(&status)))
		return Error("IDirectSoundBuffer8::GetStatus() failed!");

	if (status & DSBSTATUS_BUFFERLOST)
	{
		while (secondary->Restore() == DSERR_BUFFERLOST)
			Sleep(10);

		if (restored)
			*restored = TRUE;
	}
	else 
	{
		if (restored)
			*restored = FALSE;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::LockSecondary(const DWORD offset,const DWORD size)
{
	VOID* skip1;
	DWORD skip2;

	if (FAILED(secondary->Lock(offset,size,&locked.data,&locked.size,&skip1,&skip2,0)))
		return Error("IDirectSoundBuffer8::Lock() failed!");

	if (skip1)
		return Error("Bogus error in IDirectSoundBuffer8::Lock(), call the ghostbusters!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::UnlockSecondary()
{
	if (FAILED(secondary->Unlock(locked.data,locked.size,NULL,0)))
		return Error("IDirectSoundBuffer8::Unlock() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::ClearBuffer()
{
	PDX_TRY(RestoreBuffer());
	PDX_TRY(LockSecondary(0,BufferSize));

	memset( locked.data, WaveFormat.wBitsPerSample == 8 ? 0x80 : 0x00, locked.size );

	PDX_TRY(UnlockSecondary());

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// set format
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::SetFormat(const DWORD SampleRate,const DWORD SampleBits,const UINT latency,const UINT fps,const BOOL volume)
{
	PDXMemZero( WaveFormat );

	WaveFormat.wFormatTag      = WAVE_FORMAT_PCM; 
	WaveFormat.nChannels       = 1; 
	WaveFormat.nSamplesPerSec  = SampleRate; 
	WaveFormat.wBitsPerSample  = SampleBits; 
	WaveFormat.nBlockAlign     = WaveFormat.wBitsPerSample / 8;
	WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;

	return CreateBuffer(latency,fps,volume);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Lock(NES::IO::SFX& SoundOut)
{
	if (!secondary)
		return PDX_BUSY;

	PDX_TRY(RestoreBuffer());

	DWORD WritePos;

	if (FAILED(secondary->GetCurrentPosition(NULL,&WritePos)))
		return Error("IDirectSoundBuffer8::GetCurrentPosition() failed!");

	const DWORD offset = (WritePos >= NotifySize) ? NotifySize : 0;

	if (offset == LastOffset)
		return PDX_BUSY;

	const DWORD NewOffset = LastOffset;
	LastOffset = offset;

	PDX_TRY(LockSecondary(NewOffset,NotifySize));

	SoundOut.samples = PDX_CAST(VOID*,locked.data);
	SoundOut.length  = locked.size;

	if (WaveFormat.wBitsPerSample == 16)
		SoundOut.length >>= 1;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// destroyer
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Unlock()
{
	return UnlockSecondary();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Stop()
{
	if (!secondary || FAILED(secondary->Stop()) || PDX_FAILED(ClearBuffer()))
		return PDX_FAILURE;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Start()
{
	return (!secondary || FAILED(secondary->Play(0,0,DSBPLAY_LOOPING))) ? PDX_FAILURE : PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Clear()
{
	if (PDX_FAILED(Stop()) || PDX_FAILED(Start()))
		return PDX_FAILURE;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// destroyer
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Destroy()
{
	if (secondary)
	{
		secondary->Stop();
		DIRECTX::Release(secondary,TRUE);
	}

	DIRECTX::Release(device,TRUE);

	return PDX_OK;
}
