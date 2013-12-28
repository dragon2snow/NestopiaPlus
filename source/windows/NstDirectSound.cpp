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
// constructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTSOUND::DIRECTSOUND()
:
hWnd       (NULL),
device     (NULL),
secondary  (NULL),
LastOffset (0),
TimerRate  (44100 / 60),
LastPos    (0),
LastSample (0)
{
	locked.data = NULL;
	locked.size = 0;

	PDXMemZero( guid );
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
// destroyer
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTSOUND::Destroy()
{
	if (secondary)
	{
		secondary->Stop();
		DIRECTX::Release(secondary,TRUE);
	}

	DIRECTX::Release(device,TRUE);
	PDXMemZero( guid );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::OnError(const CHAR* const msg)
{
	for (UINT i=0; i < adapters.Size(); ++i)
	{
		if (PDXMemCompare(guid,adapters[i].guid))
		{
			adapters.Erase( adapters.At(i) );
			break;
		}
	}

	Destroy();

	PDXSTRING m(msg);
	m += " Sound will be disabled for this device!";

	application.LogFile().Output("DIRECTSOUND: ",m);
	application.OnWarning(m.String());
	
	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DIRECTSOUND::EnumAdapters(LPGUID guid,LPCSTR desc,LPCSTR,LPVOID context)
{
	LPDIRECTSOUND8 device = NULL;

	application.LogFile().Output
	(
       	"DIRECTSOUND: enumerating device, guid: ",
		(guid ? CONFIGFILE::FromGUID(*guid) : "default"),
		", name: ",
		(desc ? desc : "unknown") 
	);

	if (FAILED(DirectSoundCreate8(guid,&device,NULL)) || !device)
	{
		application.LogFile().Output
		(
	     	"DIRECTSOUND: DirectSoundCreate8() on this device failed, continuing enumeration.."
		);
		return TRUE;
	}

	DSCAPS caps;
	DIRECTX::InitStruct(caps);

	if (FAILED(device->GetCaps(&caps)))
	{
		application.LogFile().Output
		(
       		"DIRECTSOUND: IDirectSound8::GetCaps() on this device failed, continuing enumeration.."
		);		
		device->Release();		
		return TRUE;
	}

	device->Release();

	if (!(caps.dwFlags & (DSCAPS_SECONDARY16BIT|DSCAPS_SECONDARY8BIT)))
	{
		application.LogFile().Output
		(
	     	"DIRECTSOUND: this device lacks both 8 and 16 bit sample support and can't be"
			"used, continuing enumeration.."
		);
		return TRUE;
	}

	PDX_CAST(ADAPTERS*,context)->Grow();
	ADAPTER& adapter = PDX_CAST(ADAPTERS*,context)->Back();

	if (guid) PDXMemCopy( adapter.guid, *guid );
	else      PDXMemZero( adapter.guid        );

	adapter.name = desc;

	if (adapter.name.IsEmpty())
		adapter.name = "unknown";

	adapter.SampleBits8 = bool(caps.dwFlags & DSCAPS_SECONDARY8BIT);
	adapter.SampleBits16 = bool(caps.dwFlags & DSCAPS_SECONDARY16BIT);

	application.LogFile().Output
	(
	    adapter.SampleBits8 ?
		"DIRECTSOUND: 8 bit samples are supported by this device" :
		"DIRECTSOUND: 8 bit samples are NOT supported by this device"
	);

	application.LogFile().Output
	(
		adapter.SampleBits16 ?
		"DIRECTSOUND: 16 bit samples are supported by this device" :
		"DIRECTSOUND: 16 bit samples are NOT supported by this device"
	);

	const UINT rates[5] = {11025,22050,44100,48000,96000};

	for (UINT i=0; i < 5; ++i)
	{
		if (rates[i] <= caps.dwMaxSecondarySampleRate && rates[i] >= caps.dwMinSecondarySampleRate)
		{
			adapter.SampleRates.InsertBack( rates[i] );

			application.LogFile().Output
			(
     			"DIRECTSOUND: sample rate ",
				rates[i],
				" is supported by this device"
			);
		}
		else
		{
			application.LogFile().Output
			(
     			"DIRECTSOUND: sample rate ",
				rates[i],
				" is NOT supported by this device"
			);
		}
	}

	if (adapter.SampleRates.IsEmpty())
	{
		PDX_CAST(ADAPTERS*,context)->EraseBack();
		
		application.LogFile().Output
		(
		    "DIRECTSOUND: No valid sample rate was found for this device, continuing enumeration.."
		);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// initializer
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTSOUND::Initialize(HWND h)
{
	PDX_ASSERT(adapters.IsEmpty() && h);
	
	hWnd = h;

	if (!hWnd || adapters.Size())
		throw ("Internal error in DIRECTSOUND::Initialize()!");

	if (FAILED(DirectSoundEnumerate(EnumAdapters,PDX_CAST(LPVOID,&adapters))) || adapters.IsEmpty())
	{
		application.LogFile().Output("DIRECTSOUND: sound enumeration failed or no adapters was found, retrying with default..");
		EnumAdapters(NULL,"Primary Sound Driver","",PDX_CAST(LPVOID,&adapters));

		if (adapters.IsEmpty())
			OnError("No sound device could be found!");
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Create(const GUID& g)
{
	PDX_ASSERT(hWnd);

	if (device && PDXMemCompare(guid,g))
		return PDX_OK;

	Destroy();
	guid = g;

	if (FAILED(DirectSoundCreate8(&guid,&device,NULL)))
		return OnError("DirectSoundCreate8() failed!");

	application.LogFile().Output
	( 
     	"DIRECTSOUND: creating device - guid: ",
		CONFIGFILE::FromGUID(guid)
	);

	if (FAILED(device->SetCooperativeLevel(hWnd,DSSCL_PRIORITY)))
		return OnError("IDirectSound8::SetCooperativeLevel() failed!");

	LastOffset = 0;
	LastPos = 0;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::CreateBuffer(const UINT latency,const UINT rate,const BOOL pal,const BOOL volume)
{
	PDX_ASSERT(latency && latency <= 10);

	const DOUBLE multiplier = DOUBLE(rate) / (pal ? 50.0 : 60.0);

	TimerRate   = DWORD(DOUBLE(WaveFormat.nSamplesPerSec / 60.0) * multiplier);
	NotifySize  = DWORD(DOUBLE(WaveFormat.nSamplesPerSec * latency * WaveFormat.nBlockAlign) / 60.0 * multiplier);
	NotifySize -= NotifySize % WaveFormat.nBlockAlign;
	BufferSize  = NotifySize * 2;
	LastOffset  = 0;

	application.LogFile().Output
	(
       	"DIRECTSOUND: creating ",
    	BufferSize,
     	" byte sound buffer"
	);

	application.LogFile().Output
	(
     	"DIRECTSOUND: sample rate - ",
     	WaveFormat.nSamplesPerSec
	);

	application.LogFile().Output
	(
    	"DIRECTSOUND: sample bits - ",
     	WaveFormat.wBitsPerSample
	);

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
		DIRECTX::Release(secondary,TRUE);
	}

	LPDIRECTSOUNDBUFFER oldfart = NULL;

	if (FAILED(device->CreateSoundBuffer(&desc,&oldfart,NULL)) || !oldfart)
		return OnError("IDirectSound8::CreateSoundBuffer() failed!");

	if (FAILED(oldfart->QueryInterface(IID_IDirectSoundBuffer8,PDX_CAST_PTR(LPVOID,secondary))) || !secondary)
	{
		oldfart->Release();
		return OnError("IDirectSoundBuffer::QueryInterface() failed!");
	}

	oldfart->Release();

	ClearBuffer();

	LastOffset = 0;
	LastPos = 0;
	LastSample = 0;

	if (FAILED(secondary->Play(0,0,DSBPLAY_LOOPING)))
		return OnError("IDirectSoundBuffer8::Play() failed!");

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

PDXRESULT DIRECTSOUND::Lock(const DWORD offset,const DWORD size)
{
	PDX_ASSERT(secondary);

	VOID* skip1;
	DWORD skip2;

	HRESULT hResult;

	if (FAILED(hResult=secondary->Lock(offset,size,&locked.data,&locked.size,&skip1,&skip2,0)))
	{
		if (hResult == DSERR_BUFFERLOST)
		{
			Sleep(50);

			for (ULONG i=0; secondary->Restore() == DSERR_BUFFERLOST && i < 0xFFFFUL; ++i)
				Sleep(50);
		}

		if (FAILED(secondary->Lock(offset,size,&locked.data,&locked.size,&skip1,&skip2,0)))
			return OnError("IDirectSoundBuffer8::Lock() failed!");
	}

	if (locked.size != size)
		return OnError("Driver error in IDirectSoundBuffer8::Lock()!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Unlock()
{
	PDX_ASSERT(secondary);

	LastSample = 0;

	if (locked.size)
	{
		if (WaveFormat.wBitsPerSample == 16)
			LastSample = PDX_CAST(const I16*,locked.data)[(locked.size >> 1)-1];
		else
			LastSample = PDX_CAST(const U8*,locked.data)[(locked.size >> 0)-1];
	}

	if (FAILED(secondary->Unlock(locked.data,locked.size,NULL,0)))
		return OnError("IDirectSoundBuffer8::Unlock() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#ifdef NST_SYNCHRONIZE_SOUND

BOOL DIRECTSOUND::UpdateRefresh()
{
	if (secondary)
	{
		DWORD status = 0;

		if (secondary->GetStatus(&status) == DS_OK && (status & DSBSTATUS_PLAYING))
		{
			DWORD CurrentPos;

			for (;;)
			{
				if (secondary->GetCurrentPosition(&CurrentPos,NULL) != DS_OK)
					return FALSE;

				const DWORD offset = CurrentPos + (LastPos < CurrentPos ? 0 : BufferSize);

				if ((offset - LastPos) >= TimerRate)
				{
					LastPos += TimerRate;

					if (LastPos + TimerRate < offset)
					{
						LastPos = CurrentPos;
					}
					else if (LastPos >= BufferSize)
					{
						LastPos -= BufferSize;
					}

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTSOUND::ClearBuffer()
{
	PDX_ASSERT(secondary);

	LPVOID ptr[2];
	DWORD size[2];

	if (secondary->Lock(0,0,&ptr[0],&size[0],&ptr[1],&size[1],DSBLOCK_ENTIREBUFFER) == DS_OK)
	{
		const INT silence = (WaveFormat.wBitsPerSample == 8 ? 0x80 : 0x00);
		
		memset( ptr[0], silence, sizeof(BYTE) * size[0] );
		memset( ptr[1], silence, sizeof(BYTE) * size[1] );
		
		secondary->Unlock(ptr[0],size[0],ptr[1],size[1]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// set format
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::SetFormat(const DWORD SampleRate,const DWORD SampleBits,const UINT latency,const BOOL pal,const UINT fps,const BOOL volume)
{
	const BOOL set = 
	(
     	secondary && 
		data.SampleRate == SampleRate && 
		data.SampleBits == SampleBits && 
		data.latency == latency && 
		data.fps == fps && 
		data.pal == pal &&
		data.volume == volume
	);

	if (set)
		return PDX_OK;

	data.SampleRate = SampleRate;
	data.SampleBits = SampleBits;
	data.latency    = latency;
	data.fps        = fps;
	data.pal        = pal;
	data.volume     = volume;

	PDXMemZero( WaveFormat );

	WaveFormat.wFormatTag      = WAVE_FORMAT_PCM; 
	WaveFormat.nChannels       = 1; 
	WaveFormat.nSamplesPerSec  = SampleRate; 
	WaveFormat.wBitsPerSample  = SampleBits; 
	WaveFormat.nBlockAlign     = WaveFormat.wBitsPerSample / 8;
	WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;

	{
		DSBUFFERDESC desc;
		DIRECTX::InitStruct(desc);

		desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

		LPDIRECTSOUNDBUFFER primary = NULL;

		if (device->CreateSoundBuffer(&desc,&primary,NULL) == DS_OK && primary)
		{
			primary->SetFormat(&WaveFormat);
			primary->Release();
		}
	}

	return CreateBuffer(latency,fps,pal,volume);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Lock(NES::IO::SFX& SoundOut)
{
	PDX_ASSERT( secondary );

	if (!secondary)
		return PDX_BUSY;

	DWORD WritePos;

	if (FAILED(secondary->GetCurrentPosition(NULL,&WritePos)))
		return OnError("IDirectSoundBuffer8::GetCurrentPosition() failed!");

	const DWORD offset = (WritePos >= NotifySize) ? NotifySize : 0;

	if (offset == LastOffset)
		return PDX_BUSY;

	const DWORD NewOffset = LastOffset;
	LastOffset = offset;

	PDX_TRY(Lock(NewOffset,NotifySize));

	SoundOut.samples = PDX_CAST(VOID*,locked.data);
	SoundOut.length  = locked.size;

	if (WaveFormat.wBitsPerSample == 16)
		SoundOut.length >>= 1;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Stop(NES::IO::SFX& SoundOut)
{
	if (!secondary)
		return PDX_OK;

	DWORD status = 0;

	const BOOL fade =
	(
    	LastSample && 
		data.latency <= 4 &&
     	secondary->GetStatus(&status) == DS_OK && 
		(status & DSBSTATUS_PLAYING)
	);

	if (fade)
	{
		for (;;)
		{
			const PDXRESULT result = Lock(SoundOut);

			if (result == PDX_BUSY)
				continue;

			if (PDX_FAILED(result))
				return PDX_FAILURE;

			break;
		}

		if (WaveFormat.wBitsPerSample == 16)
		{
			const LONG fade = LONG(ceil(FLOAT(LastSample) / FLOAT(SoundOut.length)));
			I16* const output = (I16*) SoundOut.samples;
			
			UINT p=0;

			if (LastSample < 0) for (LONG i=LastSample; p < SoundOut.length && i < 0; ++p, i -= fade) output[p] = i;
			else                for (LONG i=LastSample; p < SoundOut.length && i > 0; ++p, i -= fade) output[p] = i;

			memset( output + p, 0x00, sizeof(I16) * (SoundOut.length - p) );
		}
		else
		{
			LastSample = PDX_CLAMP(LastSample,0x00,0xFF);

			const UINT fade = UINT(ceil(FLOAT(LastSample) / FLOAT(SoundOut.length)));
			U8* const output = (U8*) SoundOut.samples;

			UINT p=0;

			if (LastSample < 0x80) for (LONG i=LastSample; p < SoundOut.length && i < 0x80; ++p, i += fade) output[p] = i;
			else                   for (LONG i=LastSample; p < SoundOut.length && i > 0x80; ++p, i -= fade) output[p] = i;

			memset( output + p, 0x80, sizeof(U8) * (SoundOut.length - p) );
		}

		Unlock();

		for (UINT i=0; i < 2; ++i)
		{
			for (;;)
			{
				const PDXRESULT result = Lock(SoundOut);

				if (result == PDX_BUSY)
					continue;

				if (PDX_FAILED(result))
					return PDX_FAILURE;

				break;
			}	

			if (WaveFormat.wBitsPerSample == 16) memset( SoundOut.samples, 0x00, SoundOut.length * sizeof(I16) );
			else                                 memset( SoundOut.samples, 0x80, SoundOut.length * sizeof(U8)  );

			Unlock();
		}
	}

	LastSample = 0;
	LastOffset = 0;
	LastPos = 0;

	secondary->Stop();
	secondary->SetCurrentPosition(0);

	if (!fade)
		ClearBuffer();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTSOUND::Start()
{
	return (!secondary || secondary->Play(0,0,DSBPLAY_LOOPING) == DS_OK) ? PDX_OK : PDX_FAILURE;
}
