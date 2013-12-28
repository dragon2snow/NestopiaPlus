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

#include <dsound.h>
#include <MMSystem.h>
#include "../paradox/PdxArray.h"

////////////////////////////////////////////////////////////////////////////////////////
// Direct Sound
////////////////////////////////////////////////////////////////////////////////////////

class DIRECTSOUND
{
public:

	PDXRESULT Stop();
	PDXRESULT Start();

	VOID EnablePAL(const BOOL);

protected:

	DIRECTSOUND();
	~DIRECTSOUND();

	PDXRESULT Clear();
	PDXRESULT Lock(NES::IO::SFX&);
	PDXRESULT Unlock();	
	PDXRESULT Initialize(HWND);
	PDXRESULT Destroy();
	PDXRESULT Create(const GUID* const);
	PDXRESULT SetFormat(const DWORD,const DWORD,const UINT,const UINT,const BOOL);

	inline const WAVEFORMATEX& GetWaveFormat() const
	{ return WaveFormat; }

	inline TSIZE GetMaxBufferLength() const
	{ return (WaveFormat.wBitsPerSample == 16 ? NotifySize >> 1 : NotifySize); }

	inline const VOID* GetDSoundBuffer() const
	{ return locked.data; }

	inline TSIZE GetDSoundBufferSize() const
	{ return locked.size; }

	VOID SetVolume(const LONG);

	struct ADAPTER
	{
		typedef PDXARRAY<DWORD> SAMPLERATES;

		GUID guid;
		PDXSTRING name;
		BOOL SampleBits16;
		BOOL SampleBits8;
		SAMPLERATES SampleRates;
	};

	typedef PDXARRAY<ADAPTER> ADAPTERS;

	HWND hWnd;
	ADAPTERS adapters;

private:

	static PDXRESULT Error(const CHAR* const);

	static BOOL CALLBACK EnumAdapters(LPGUID,LPCSTR,LPCSTR,LPVOID);

	PDXRESULT UnlockSecondary();
	PDXRESULT LockSecondary(const DWORD,const DWORD);
	PDXRESULT ClearBuffer();
	PDXRESULT RestoreBuffer(BOOL* const=NULL);
	PDXRESULT CreateBuffer(const UINT,const UINT,const BOOL);

	struct LOCKED
	{
		VOID* data;
		DWORD size;
	};

	LPDIRECTSOUND8 device;
	LPDIRECTSOUNDBUFFER8 secondary;
	WAVEFORMATEX WaveFormat;   
	LOCKED locked;
	
	DWORD NotifyOffset;
	DWORD NotifySize;
	DWORD BufferSize;
	DWORD LastOffset;
};

#endif
