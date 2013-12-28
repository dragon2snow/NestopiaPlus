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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <MMSystem.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include "../paradox/PdxFile.h"
#include "NstSoundManager.h"
#include "NstWaveFile.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NES::IO::SFX::Lock()
{
	PDX_ASSERT(device);
	return PDX_CAST(SOUNDMANAGER*,device)->Lock(*this);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NES::IO::SFX::Unlock()
{
	PDX_ASSERT(device);
	return PDX_CAST(SOUNDMANAGER*,device)->Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NES::IO::SFX::Clear()
{
	PDX_ASSERT(device);
	return PDX_CAST(SOUNDMANAGER*,device)->Clear();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SOUNDMANAGER::SOUNDMANAGER()
: 
MANAGER            (IDD_SOUND), 
enabled            (FALSE),
SelectedAdapter    (0),
SelectedSampleRate (0),
SelectedSampleBits (0),
SelectedLatency    (3),
SelectedVolume     (DSBVOLUME_MAX),
RefreshRate        (60),
IsPAL              (FALSE)
{
	format.device = PDX_CAST(VOID*,this);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::Create(CONFIGFILE* const ConfigFile)
{	
	DIRECTSOUND::Initialize( MANAGER::hWnd );

	nes.GetAudioContext( context );

	GUID guid;
	GUID* pGuid = NULL;

	if (adapters.IsEmpty())
	{
		Disable();
	}
	else if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		SetSampleRate( file["sound sample rate"].ToUlong() );
		SetSampleBits( file["sound sample bits"].ToUlong() );

		enabled			 = ( file[ "sound enabled"      ] == "no"  ? FALSE : TRUE );
		context.square1	 = ( file[ "sound apu square 1" ] == "off" ? FALSE : TRUE );
		context.square2	 = ( file[ "sound apu square 2" ] == "off" ? FALSE : TRUE );
		context.triangle = ( file[ "sound apu triangle" ] == "off" ? FALSE : TRUE );
		context.noise	 = ( file[ "sound apu noise"    ] == "off" ? FALSE : TRUE );
		context.dpcm	 = ( file[ "sound apu dpcm"     ] == "off" ? FALSE : TRUE );
		context.external = ( file[ "sound apu external" ] == "off" ? FALSE : TRUE );

		const PDXSTRING* string;

		ULONG volume;
		string = &file[ "sound buffers" ]; SelectedLatency = string->Length() ? string->ToUlong() : 3;
		string = &file[ "sound volume"  ]; volume          = string->Length() ? string->ToUlong() : 100;

		SelectedLatency = PDX_MIN(SelectedLatency,10);
		SelectedVolume = LONG(PDX_MIN(volume,100) * 100) + DSBVOLUME_MIN;

		guid = UTILITIES::ToGUID( file["sound device"].String() );
		pGuid = &guid;
	}
	else
	{
		Reset();
	}

	UpdateDirectSound( pGuid );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::Disable()
{
	enabled = FALSE;
	context.enabled = FALSE;
	SelectedAdapter = 0;
	SoundRecorder.Close();
	nes.SetAudioContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	SoundRecorder.Close();

	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		if (adapters.Size() > SelectedAdapter)
		{
			file[ "sound device"      ] = UTILITIES::FromGUID(adapters[SelectedAdapter].guid).Quoted();
			file[ "sound sample rate" ] = adapters[SelectedAdapter].SampleRates[SelectedSampleRate];
		}
		else
		{
			file[ "sound device"      ];
			file[ "sound sample rate" ];
		}

		file[ "sound enabled"      ] = (enabled ? "yes" : "no");
		file[ "sound sample bits"  ] = SelectedSampleBits;
		file[ "sound buffers"      ] = SelectedLatency;
		file[ "sound volume"       ] = ((SelectedVolume - DSBVOLUME_MIN) / 100);
		file[ "sound apu square 1" ] = (context.square1  ? "on" : "off");
		file[ "sound apu square 2" ] = (context.square2  ? "on" : "off");
		file[ "sound apu triangle" ] = (context.triangle ? "on" : "off");
		file[ "sound apu noise"    ] = (context.noise    ? "on" : "off");
		file[ "sound apu dpcm"     ] = (context.dpcm     ? "on" : "off");
		file[ "sound apu external" ] = (context.external ? "on" : "off");
	}

	enabled = FALSE;

	DIRECTSOUND::Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::Stop()
{
	nes.ResetAudioBuffer();

	if (enabled && PDX_FAILED(DIRECTSOUND::Stop(format)))
		Disable();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::Start()
{
	if (enabled && PDX_FAILED(DIRECTSOUND::Start()))
		Disable();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::Clear()
{
	nes.ResetAudioBuffer();

	if (enabled && (PDX_FAILED(DIRECTSOUND::Stop(format)) || PDX_FAILED(DIRECTSOUND::Start())))
	{
		Disable();
		return PDX_FAILURE;
	}
  
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::CreateDevice(GUID guid)
{
	for (UINT i=0; i < adapters.Size(); ++i)
	{
		if (PDXMemCompare(guid,adapters[i].guid))
		{
			if (PDX_SUCCEEDED(DIRECTSOUND::Create(guid)))
			{
				SelectedAdapter = i;
				return PDX_OK;
			}
			break;
		}
	}

	PDXMemZero( guid );

	for (UINT i=0; i < adapters.Size(); ++i)
	{
		if (PDXMemCompare(guid,adapters[i].guid))
		{
			if (PDX_SUCCEEDED(DIRECTSOUND::Create(guid)))
			{
				SelectedAdapter = i;
				return PDX_OK;
			}
			break;
		}
	}

	for (UINT i=0; i < adapters.Size(); ++i)
	{
		if (PDX_SUCCEEDED(DIRECTSOUND::Create(adapters[i].guid)))
		{
			SelectedAdapter = i;
			return PDX_OK;
		}
	}

	DIRECTSOUND::Destroy();

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SetSampleRate(const DWORD rate)
{
	SelectedSampleRate = 0;

	if (SelectedAdapter < adapters.Size())
	{
		const PDXARRAY<DWORD>& rates = adapters[SelectedAdapter].SampleRates;
		const DWORD* pos;

		if ((pos = PDX::Find( rates.Begin(), rates.End(), rate )) == rates.End())
			pos = PDX::Find( rates.Begin(), rates.End(), 44100 );

		if (pos != rates.End())
			SelectedSampleRate = pos - rates.Begin();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SetSampleBits(const UINT bits)
{
	SelectedSampleBits = (bits == 8 ? 8 : 16);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::Reset()
{
	enabled = bool(adapters.Size());

	SelectedAdapter = 0;
	SelectedLatency = 3;
	SelectedVolume = DSBVOLUME_MAX;

	context.enabled  = TRUE;
	context.square1  = TRUE;
	context.square2  = TRUE;
	context.triangle = TRUE;
	context.noise    = TRUE;
	context.dpcm     = TRUE;
	context.external = TRUE;

	SetSampleRate();
	SetSampleBits();

	SoundRecorder.Close();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::Lock(NES::IO::SFX& sfx)
{
	if (enabled)
	{
		const PDXRESULT result = DIRECTSOUND::Lock(sfx);

		switch (result)
		{
     		case PDX_OK: return PDX_OK;
     		case PDX_BUSY: return PDX_BUSY;
		}

		Disable();
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::Unlock()
{
	if (enabled)
	{
		if (SoundRecorder.IsRecording())
		{
			const PDXRESULT recorded = SoundRecorder.Write( GetDSoundBuffer(), GetDSoundBufferSize() );

			if (PDX_SUCCEEDED(DIRECTSOUND::Unlock()))
			{
				if (PDX_SUCCEEDED(recorded))
					SoundRecorder.NotifySize();

				return PDX_OK;
			}

			Disable();

			return PDX_FAILURE;
		}

		if (PDX_SUCCEEDED(DIRECTSOUND::Unlock()))
		{
			if (nes.GetAudioLatency() > GetMaxBufferLength())
				nes.ResetAudioBuffer();

			return PDX_OK;
		}

		Disable();
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SetRefreshRate(const BOOL pal,const UINT rate)
{
	if ((RefreshRate != rate || bool(IsPAL) != bool(pal)) && SelectedAdapter < adapters.Size())
	{
		IsPAL = pal;
		RefreshRate = rate;
		UpdateDirectSound();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::UpdateRefreshRate()
{
	if (SelectedAdapter < adapters.Size())
	{
		const DOUBLE SampleRate = DOUBLE(adapters[SelectedAdapter].SampleRates[SelectedSampleRate]);
		context.SampleRate = ULONG(SampleRate / (DOUBLE(RefreshRate) / (IsPAL ? 50.0 : 60.0)));
		nes.SetAudioContext( context );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL SOUNDMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
    	case WM_INITDIALOG:

			SoundRecorder.Close();
			UpdateDialog( hDlg );
     		return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_SOUND_ENABLE:
				{
					const bool checked = ::IsDlgButtonChecked(hDlg,IDC_SOUND_ENABLE) == BST_CHECKED;

					if (bool(enabled) != checked)
					{
						enabled = checked;
						OnEnable(hDlg);
					}
					return TRUE;
				}

				case IDC_SOUND_DEVICE:
     				
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						SelectedAdapter = ComboBox_GetCurSel(::GetDlgItem(hDlg,IDC_SOUND_DEVICE));
						SetSampleRate();
						SetSampleBits();
       					UpdateDialog(hDlg);
					}	     
					return TRUE;

				case IDC_SOUND_SAMPLE_RATE:

					if (HIWORD(wParam) == CBN_SELCHANGE)
						SelectedSampleRate = ComboBox_GetCurSel(::GetDlgItem(hDlg,IDC_SOUND_SAMPLE_RATE));

					return TRUE;

				case IDC_SOUND_8_BIT:  SelectedSampleBits = 8;  return TRUE;
				case IDC_SOUND_16_BIT: SelectedSampleBits = 16; return TRUE;

				case IDC_SOUND_SQUARE1:  context.square1  = (::IsDlgButtonChecked( hDlg, IDC_SOUND_SQUARE1  ) == BST_CHECKED); return TRUE;
				case IDC_SOUND_SQUARE2:  context.square2  = (::IsDlgButtonChecked( hDlg, IDC_SOUND_SQUARE2  ) == BST_CHECKED); return TRUE;
				case IDC_SOUND_TRIANGLE: context.triangle = (::IsDlgButtonChecked( hDlg, IDC_SOUND_TRIANGLE ) == BST_CHECKED); return TRUE;
				case IDC_SOUND_NOISE:    context.noise    = (::IsDlgButtonChecked( hDlg, IDC_SOUND_NOISE    ) == BST_CHECKED); return TRUE;
				case IDC_SOUND_DPCM:     context.dpcm     = (::IsDlgButtonChecked( hDlg, IDC_SOUND_DPCM     ) == BST_CHECKED); return TRUE;
				case IDC_SOUND_EXTERNAL: context.external = (::IsDlgButtonChecked( hDlg, IDC_SOUND_EXTERNAL ) == BST_CHECKED); return TRUE;

				case IDC_SOUND_DEFAULT:

					Reset();
					UpdateDialog(hDlg);
					return TRUE;

				case IDC_SOUND_OK:

					::EndDialog(hDlg,0);
					return TRUE;
			}
			return TRUE;

		case WM_VSCROLL:
		{
			const INT volume = DSBVOLUME_MIN - (INT) ::SendMessage
			(
				::GetDlgItem(hDlg,IDC_SOUND_VOLUME),
				TBM_GETPOS,
				WPARAM(0),
				LPARAM(0)
			);

			SelectedVolume = volume;
			return TRUE;
		}

		case WM_HSCROLL:
		{
			const UINT latency = (UINT) ::SendMessage
			(
				::GetDlgItem(hDlg,IDC_SOUND_LATENCY),
				TBM_GETPOS,
				WPARAM(0),
				LPARAM(0)
			);

			SelectedLatency = latency;
			return TRUE;
		}

		case WM_CLOSE:

			::EndDialog(hDlg,0);
			return TRUE;

		case WM_DESTROY:

       		UpdateDirectSound();
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::OnEnable(HWND hDlg)
{
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_DEVICE      ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_SAMPLE_RATE ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_8_BIT       ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_16_BIT      ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_VOLUME      ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_LATENCY     ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_SQUARE1     ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_SQUARE2     ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_TRIANGLE    ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_NOISE       ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_DPCM        ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_EXTERNAL    ), enabled );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_SOUND_DEFAULT     ), enabled );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::UpdateDirectSound(const GUID* const guid)
{
	context.enabled = enabled;

	const BOOL NeedVolume = (SelectedVolume != DSBVOLUME_MAX);

	if (enabled && SelectedAdapter < adapters.Size())
	{
		const UINT prev = SelectedAdapter;

		if (PDX_FAILED(CreateDevice(guid ? *guid : adapters[SelectedAdapter].guid)))
		{
			Disable();
			return;
		}

		const ADAPTER& adapter = adapters[SelectedAdapter];

		if (prev != SelectedAdapter)
		{
			SetSampleRate( adapters[prev].SampleRates[SelectedSampleRate] );
			SetSampleBits( SelectedSampleBits );
		}

		context.SampleBits = SelectedSampleBits;

		if (PDX_FAILED(SetFormat(adapter.SampleRates[SelectedSampleRate],context.SampleBits,SelectedLatency,IsPAL,RefreshRate,NeedVolume)))
		{
			Disable();
			return;
		}
	}
	else
	{
		DIRECTSOUND::Destroy();
	}

	UpdateRefreshRate();

	if (NeedVolume)
		SetVolume(SelectedVolume);
	
	SoundRecorder.SetWaveFormat( GetWaveFormat() );
	nes.SetAudioContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::UpdateDialog(HWND hDlg)
{
	if (SelectedAdapter >= adapters.Size())
		SelectedAdapter = 0;

	if (adapters.IsEmpty())
	{
		::CheckDlgButton(hDlg,IDC_SOUND_ENABLE,BST_UNCHECKED);
		::EnableWindow(::GetDlgItem(hDlg,IDC_SOUND_ENABLE),FALSE);
		OnEnable(hDlg);
		return;
	}

	{
		HWND item = ::GetDlgItem(hDlg,IDC_SOUND_DEVICE);

		ComboBox_ResetContent(item);

		for (UINT i=0; i < adapters.Size(); ++i)
			ComboBox_AddString(item,adapters[i].name.Begin());

		ComboBox_SetCurSel(item,SelectedAdapter);
	}

	{
		HWND item = ::GetDlgItem(hDlg,IDC_SOUND_SAMPLE_RATE);

		ComboBox_ResetContent(item);

		const DIRECTSOUND::ADAPTER::SAMPLERATES& SampleRates = adapters[SelectedAdapter].SampleRates;

		PDXSTRING string;

		for (UINT i=0; i < SampleRates.Size(); ++i)
		{
			string.Set(SampleRates[i]);
			ComboBox_AddString(item,string.Begin());
		}

		ComboBox_SetCurSel(item,SelectedSampleRate);
	}

	{
		const INT button = (SelectedSampleBits == 16) ? IDC_SOUND_16_BIT : IDC_SOUND_8_BIT;
		::CheckRadioButton( hDlg, IDC_SOUND_8_BIT, IDC_SOUND_16_BIT, button );
	}

	{
		::CheckDlgButton( hDlg, IDC_SOUND_SQUARE1,  context.square1  );
		::CheckDlgButton( hDlg, IDC_SOUND_SQUARE2,  context.square2  );
		::CheckDlgButton( hDlg, IDC_SOUND_TRIANGLE, context.triangle );
		::CheckDlgButton( hDlg, IDC_SOUND_NOISE,    context.noise    );
		::CheckDlgButton( hDlg, IDC_SOUND_DPCM,     context.dpcm     );
		::CheckDlgButton( hDlg, IDC_SOUND_EXTERNAL, context.external );
	}

	{
		::CheckDlgButton( hDlg, IDC_SOUND_ENABLE, enabled );
	}

	{
		HWND item = ::GetDlgItem( hDlg, IDC_SOUND_LATENCY );

		::SendMessage( item, TBM_SETRANGE, WPARAM(FALSE), LPARAM(MAKELONG(1,10)) );
		::SendMessage( item, TBM_SETPOS, WPARAM(TRUE), LPARAM(SelectedLatency) );
	}

	{
		HWND item = ::GetDlgItem( hDlg, IDC_SOUND_VOLUME );

		::SendMessage( item, TBM_SETRANGE, WPARAM(FALSE), LPARAM(MAKELONG(DSBVOLUME_MIN,DSBVOLUME_MAX)) );
		::SendMessage( item, TBM_SETPOS, WPARAM(TRUE), LPARAM(DSBVOLUME_MIN - SelectedVolume) );
	}

	if (!enabled)
		OnEnable( hDlg );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_1MB 0x100000UL

SOUNDMANAGER::SOUNDRECORDER::SOUNDRECORDER()
: 
MANAGER      (IDD_SOUND_RECORD),
WrittenBytes (0),
NextSize     (NST_1MB),
recording    (FALSE),
KeepGoing    (FALSE),
WaveFile     (NULL) 
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

SOUNDMANAGER::SOUNDRECORDER::~SOUNDRECORDER()
{
	delete WaveFile;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::SetWaveFormat(const WAVEFORMATEX& wf)
{
	WaveFormat = wf; 
	Reset( FALSE );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::NotifySize()
{
	if (WrittenBytes >= NextSize)
	{
		if (WrittenBytes >= NST_1MB * 100 && !KeepGoing)
		{
			KeepGoing = UI::MsgQuestion
			(
				IDS_SOUNDRECORD_WASTED_SPACE_1_2,
				IDS_SOUNDRECORD_WASTED_SPACE_2_2
			);

			if (!KeepGoing)
			{
				Close();
				return;
			}
		}
		else
		{
			application.StartScreenMsg
			( 
     			1500,
			    (NextSize / NST_1MB),
     			" MB written to wave file.." 
			);
		}

		while (NextSize < WrittenBytes)
			NextSize += NST_1MB;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::SOUNDRECORDER::Write(const VOID* const data,const TSIZE size)
{
	PDX_ASSERT( recording && WaveFile && size );

	if (PDX_SUCCEEDED(WaveFile->Write( data, size )))
	{
		WrittenBytes += size;
		return PDX_OK;
	}

	Close();

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::Close()
{
	WrittenBytes = 0;
	NextSize = NST_1MB;
	KeepGoing = FALSE;
	recording = FALSE;
	delete WaveFile;
	WaveFile = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::Start(const BOOL msg)
{
	if (!recording && file.Length())
	{
		if (!WaveFile)
		{
			WaveFile = new WAVEFILE;
			
			WrittenBytes = 0;
			NextSize = NST_1MB;
			KeepGoing = FALSE;
			
			if (PDX_FAILED(WaveFile->Open( file.String(), WaveFormat )))
			{
				Close();
				file.Clear();
				return;
			}
			
			if (msg)
				application.StartScreenMsg( 1500, "Sound recording started.." );
		}
		else
		{
			if (msg)
				application.StartScreenMsg( 1500, "Sound recording resumed.." );
		}

		recording = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::Stop(const BOOL msg)
{
	if (recording)
	{
		recording = FALSE;

		if (msg)
			application.StartScreenMsg( 1500, "Sound recording stopped.." );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::Reset(const BOOL msg)
{
	delete WaveFile;
	WaveFile = NULL;

	WrittenBytes = 0;
	NextSize = NST_1MB;
	KeepGoing = FALSE;

	if (recording)
	{
		recording = FALSE;
		Start( FALSE );
	}

	if (msg)
		application.StartScreenMsg( 1500, "Sound recording reset.." );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::OnBrowse(HWND hDlg)
{
	PDXSTRING filename;

	const BOOL succeeded = UTILITIES::BrowseSaveFile
	(
	    filename,
		hDlg,
		IDS_FILE_SAVE_WAVE,
   		"WAVE Files (*.wav)\0"
		"*.wav\0"
		"All Files (*.*)\0"
		"*.*\0",
		NULL,
		"wav"
	);

	if (succeeded)
		::SetDlgItemText( hDlg, IDC_SOUND_CAPTURE_FILE, filename.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::OnClear(HWND hDlg)
{
	::SetDlgItemText( hDlg, IDC_SOUND_CAPTURE_FILE, "" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::OnOk(HWND hDlg)
{
	PDXSTRING filename;	
	MANAGER::GetDlgItemText( hDlg, IDC_SOUND_CAPTURE_FILE, filename );

	if (filename.Length() && filename.GetFileExtension().IsEmpty())
		filename += ".wav";

	if (file != filename)
	{
		file = filename;
		Reset( FALSE );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL SOUNDMANAGER::SOUNDRECORDER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			::SetDlgItemText( hDlg, IDC_SOUND_CAPTURE_FILE, file.String() );
			return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
     			case IDC_SOUND_CAPTURE_BROWSE: OnBrowse( hDlg );     return TRUE;
				case IDC_SOUND_CAPTURE_CLEAR:  OnClear( hDlg );      return TRUE;
    			case IDC_SOUND_CAPTURE_OK:	   OnOk( hDlg );
	       		case IDC_SOUND_CAPTURE_CANCEL: ::EndDialog( hDlg, 0 ); return TRUE;
			}
			return FALSE;

		case WM_CLOSE:

			::EndDialog( hDlg, 0 );
			return TRUE;
	}

	return FALSE;
}
