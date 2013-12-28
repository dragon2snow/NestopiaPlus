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

#pragma once

#ifndef NST_DIRECTSOUND_H
#define NST_DIRECTSOUND_H

#include <Mmreg.h>
#include <dsound.h>

////////////////////////////////////////////////////////////////////////////////////////
// Direct Sound
////////////////////////////////////////////////////////////////////////////////////////

class DIRECTSOUND
{
public:

	VOID EnablePAL(const BOOL);
	BOOL Synchronize();

	VOID Halt()
	{
		if (secondary)
			secondary->Stop();
	}

protected:

	DIRECTSOUND();
	~DIRECTSOUND();

	VOID Initialize(HWND);
	VOID Destroy();

	PDX_NO_INLINE PDXRESULT Create(const GUID&);
	PDX_NO_INLINE PDXRESULT SetFormat(const DWORD,const DWORD,const UINT,const BOOL,const UINT,const BOOL);
	
	PDXRESULT Start();
	PDXRESULT Stop(NES::IO::SFX&);
	PDXRESULT Lock(NES::IO::SFX&);
	PDXRESULT Unlock();

	inline const WAVEFORMATEX& GetWaveFormat() const
	{ return WaveFormat; }

	inline TSIZE GetMaxBufferLength() const
	{ return (WaveFormat.wBitsPerSample == 16 ? NotifySize >> 1 : NotifySize); }

	inline const VOID* GetDSoundBuffer() const
	{ return locked.data; }

	inline TSIZE GetDSoundBufferSize() const
	{ return locked.size; }

	inline LPDIRECTSOUNDBUFFER8 GetBuffer()
	{ return secondary; }

	VOID SetVolume(const LONG);

	struct ADAPTER
	{
		ADAPTER()
		{ memset( &guid, 0x00, sizeof(guid) ); }

		typedef PDXARRAY<DWORD> SAMPLERATES;

		GUID guid;
		PDXSTRING name;
		SAMPLERATES SampleRates;
	};

	typedef PDXARRAY<ADAPTER> ADAPTERS;

	HWND hWnd;
	ADAPTERS adapters;

private:

	PDXRESULT OnError(const CHAR* const);

	static BOOL CALLBACK EnumAdapters(LPGUID,LPCSTR,LPCSTR,LPVOID);

	PDXRESULT Lock(const DWORD,const DWORD);
	VOID ClearBuffer();
	
	PDX_NO_INLINE PDXRESULT CreateBuffer(const UINT,const UINT,const BOOL,const BOOL);

	struct LOCKED
	{
		VOID* data;
		DWORD size;
	};

	LPDIRECTSOUND8 device;
	LPDIRECTSOUNDBUFFER8 secondary;
	WAVEFORMATEX WaveFormat;   
	LOCKED locked;
	
	DWORD NotifySize;
	DWORD BufferSize;
	DWORD LastOffset;
	DWORD LastPos;
	DWORD TimerRate;
	LONG  LastSample;

	struct DATA
	{
		DATA()
		:
		SampleRate (0),
		SampleBits (0),
		latency    (0),
		fps        (0),
		pal        (FALSE),
		volume     (0)
		{}

		DWORD SampleRate;
		DWORD SampleBits;
		UINT latency;
		UINT fps;
		BOOL pal;
		BOOL volume;
	};

	GUID guid;
	DATA data;
};

#endif
