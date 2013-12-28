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

#ifndef NST_SOUNDMANAGER_H
#define NST_SOUNDMANAGER_H

#include "NstDirectX.h"
#include "NstManager.h"

class WAVEFILE;
class PDXFILE;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class SOUNDMANAGER : public DIRECTSOUND, public MANAGER
{
public:

	SOUNDMANAGER(const INT);

	PDXRESULT Clear();
	PDXRESULT Lock(NES::IO::SFX&);
	PDXRESULT Unlock();	
	PDXRESULT SetRefreshRate(const BOOL,const UINT);

	inline NES::IO::SFX* GetFormat()
	{ return &format; }

	inline VOID StartSoundRecordDialog() { SoundRecorder.StartDialog(); }
	inline VOID ResetSoundRecording()	 { SoundRecorder.Reset();       }
	inline VOID StartSoundRecording()    { SoundRecorder.Start();       }
	inline VOID StopSoundRecording()     { SoundRecorder.Stop();        }

	inline BOOL IsSoundRecorderRecording()    const { return SoundRecorder.IsRecording(); }
	inline BOOL IsSoundRecordingFilePresent() const { return SoundRecorder.FilePresent(); }

private:

	PDXRESULT Create  (CONFIGFILE* const);
	PDXRESULT Destroy (CONFIGFILE* const);

	PDXRESULT CreateDevice(GUID);

	VOID SetSampleRate(const DWORD=44100);
	VOID SetSampleBits(const UINT=16);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	VOID Reset();
	VOID OnEnable(HWND);
	VOID UpdateDialog(HWND);
	VOID UpdateRefreshRate();
	
	PDXRESULT UpdateDirectSound(const GUID* const=NULL);

	BOOL enabled;
	UINT SelectedAdapter;
	UINT SelectedSampleRate;
	UINT SelectedSampleBits;
	UINT SelectedLatency;
	INT  SelectedVolume;
	UINT RefreshRate;
	BOOL IsPAL;
	BOOL ChangedSampleParameters;

	NES::IO::SFX format;
	NES::IO::SFX::CONTEXT context;

	class SOUNDRECORDER : public MANAGER
	{
	public:

		SOUNDRECORDER();
		~SOUNDRECORDER();

		inline BOOL IsRecording() const
		{ return recording; }

		VOID Start (const BOOL=TRUE);
		VOID Stop  (const BOOL=TRUE);
		VOID Reset (const BOOL=TRUE);
		VOID Close ();
		
		inline BOOL FilePresent() const
		{ return file.Length() ? TRUE : FALSE; }

		PDXRESULT Write(const VOID* const,const TSIZE);
		
		VOID NotifySize();
		VOID SetWaveFormat(const WAVEFORMATEX&);

		BOOL DialogProc (HWND,UINT,WPARAM,LPARAM);

	private:

		VOID OnBrowse (HWND);
		VOID OnClear  (HWND);
		VOID OnOk     (HWND);

		BOOL recording;
		ULONG WrittenBytes;
		ULONG NextSize;
		PDXSTRING file;
		WAVEFILE* WaveFile;
		WAVEFORMATEX WaveFormat;
		BOOL KeepGoing;
	};

	SOUNDRECORDER SoundRecorder;
};

#endif
