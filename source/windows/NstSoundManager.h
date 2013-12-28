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

	SOUNDMANAGER(const INT,const UINT);

	PDXRESULT Clear();
	PDXRESULT Lock(NES::IO::SFX&);
	PDXRESULT Unlock();	
	PDXRESULT EnablePAL(const BOOL);

	inline NES::IO::SFX* GetFormat()
	{ return &format; }

	inline VOID StartSoundRecordDialog() { SoundRecorder.StartDialog(); }
	inline VOID ResetSoundRecording()	 { SoundRecorder.Reset();       }
	inline VOID StartSoundRecording()    { SoundRecorder.Start();       }
	inline VOID StopSoundRecording()     { SoundRecorder.Stop();        }

	inline BOOL IsSoundRecorderRecording()    const { return SoundRecorder.IsRecording(); }
	inline BOOL IsSoundRecordingFilePresent() const { return SoundRecorder.FilePresent(); }

private:

	PDXRESULT Create  (PDXFILE* const);
	PDXRESULT Destroy (PDXFILE* const);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	VOID Reset();
	VOID OnEnable(HWND);
	VOID UpdateDialog(HWND);
	BOOL UpdateSoundParameters();
	VOID ResetSoundParameters();

	BOOL enabled;
	BOOL pal;

	UINT SelectedAdapter;
	UINT SelectedSampleRate;
	UINT SelectedSampleBits;
	UINT SelectedLatency;
	INT  SelectedVolume;
	
	BOOL ChangedDevice;
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

    #pragma pack(push,1)

	struct HEADER
	{
		enum SAMPLERATE
		{
			SAMPLERATE_11025,
			SAMPLERATE_22050,
			SAMPLERATE_44100,
			SAMPLERATE_48000,
			SAMPLERATE_96000,
			SAMPLERATE_192000
		};

		enum SAMPLEBITS
		{
			SAMPLEBITS_8,
			SAMPLEBITS_16
		};

		GUID guid;

		U8  enabled    : 1;
		U8  SampleRate : 4;
		U8  SampleBits : 1;
		U8  latency    : 4;
		I16 volume;
		U8  square1    : 1;
		U8  square2    : 1;
		U8  triangle   : 1;
		U8  noise      : 1;
		U8  dpcm       : 1;
		U8  external   : 1;
	};

    #pragma pack(pop)
};

#endif
