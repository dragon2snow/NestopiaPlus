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

#include "resource/resource.h"
#include "NstApplication.h"
#include "NstSoundManager.h"
#include "../paradox/PdxFile.h"
#include <WindowsX.h>
#include <CommCtrl.h>
#include "NstWaveFile.h"

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

SOUNDMANAGER::SOUNDMANAGER(const INT id,const UINT chunk)
: 
MANAGER (id,chunk), 
pal     (FALSE) 
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::Create(PDXFILE* const file)
{	
	PDX_TRY(DIRECTSOUND::Initialize( MANAGER::hWnd ));
	PDX_TRY(SoundRecorder.Init( MANAGER::hWnd, MANAGER::hInstance ));

	nes->GetAudioContext( context );

	format.device = PDX_CAST(VOID*,this);

	if (file && file->Read<U8>() == 0xDA)
	{
		ChangedDevice = TRUE;
		ChangedSampleParameters = TRUE;

		HEADER header;
		file->Read(header);
		
		SelectedAdapter = 0;

		for (UINT i=0; i < adapters.Size(); ++i)
		{
			if (!memcmp(&header.guid,&adapters[i].guid,sizeof(header.guid)))
			{
				SelectedAdapter = i;
				break;
			}
		}

		UINT SampleRate;

		switch (header.SampleRate)
		{
			case HEADER::SAMPLERATE_11025:  SampleRate = 11025;  break;
			case HEADER::SAMPLERATE_22050:  SampleRate = 22050;  break;
			case HEADER::SAMPLERATE_44100:  SampleRate = 44100;  break;
			case HEADER::SAMPLERATE_48000:  SampleRate = 48000;  break;
			case HEADER::SAMPLERATE_96000:  SampleRate = 96000;  break;
			case HEADER::SAMPLERATE_192000: SampleRate = 192000; break;
		}

		SelectedSampleRate = 0;

		const ADAPTER& adapter = adapters[SelectedAdapter];

		for (UINT i=0; i < adapter.SampleRates.Size(); ++i)
		{
			if (adapter.SampleRates[i] == SampleRate)
			{
				SelectedSampleRate = i;
				break;
			}
		}

		switch (header.SampleBits)
		{
     		case HEADER::SAMPLEBITS_8:  SelectedSampleBits = adapter.SampleBits8  ? 8 : 16; break;
			case HEADER::SAMPLEBITS_16: SelectedSampleBits = adapter.SampleBits16 ? 16 : 8; break;
		}

		enabled			 = header.enabled; 
		SelectedLatency	 = header.latency; 
		SelectedVolume	 = header.volume;  
		context.square1	 = header.square1; 
		context.square2	 = header.square2; 
		context.triangle = header.triangle;
		context.noise	 = header.noise;   
		context.dpcm	 = header.dpcm;
		context.external = header.external;
	}
	else
	{
		Reset();
	}

	UpdateSoundParameters();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::Destroy(PDXFILE* const file)
{
	if (file)
	{
		const U8 good = adapters.Size() > SelectedAdapter ? 0xDA : 0x00;

		file->Write(good);

		if (good)
		{
			HEADER header;

			switch (adapters[SelectedAdapter].SampleRates[SelectedSampleRate])
			{
	     		case 11025:  header.SampleRate = HEADER::SAMPLERATE_11025;  break;
	     		case 22050:  header.SampleRate = HEADER::SAMPLERATE_22050;  break;
	     		case 44100:  header.SampleRate = HEADER::SAMPLERATE_44100;  break;
				case 48000:  header.SampleRate = HEADER::SAMPLERATE_48000;  break;
	     		case 96000:  header.SampleRate = HEADER::SAMPLERATE_96000;  break;
	     		case 192000: header.SampleRate = HEADER::SAMPLERATE_192000; break;
			}

			switch (SelectedSampleBits)
			{
	     		case 8:	 header.SampleBits = HEADER::SAMPLEBITS_8;  break;
	     		case 16: header.SampleBits = HEADER::SAMPLEBITS_16; break;
			}

			header.enabled  = enabled ? 1 : 0;
			header.latency  = SelectedLatency;
			header.volume   = SelectedVolume;
			header.square1  = context.square1  ? 1 : 0;
			header.square2  = context.square2  ? 1 : 0;
			header.triangle = context.triangle ? 1 : 0;
			header.noise    = context.noise    ? 1 : 0;
			header.dpcm     = context.dpcm     ? 1 : 0;
			header.external = context.external ? 1 : 0;
			
			memcpy( &header.guid, &adapters[SelectedAdapter].guid, sizeof(header.guid) );

			file->Write( header );
		}
	}

	SoundRecorder.Stop( FALSE );

	return DIRECTSOUND::Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::Reset()
{
	ChangedDevice = TRUE;

	SelectedAdapter = 0;
	SelectedLatency = 3;
	SelectedVolume = 0;

	context.enabled  = TRUE;
	context.square1  = TRUE;
	context.square2  = TRUE;
	context.triangle = TRUE;
	context.noise    = TRUE;
	context.dpcm     = TRUE;
	context.external = TRUE;

	if (enabled = adapters.Size())
		ResetSoundParameters();	

	SoundRecorder.SetWaveFormat( GetWaveFormat() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::Clear()
{
	return DIRECTSOUND::Clear();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::Lock(NES::IO::SFX& sfx)
{
	return DIRECTSOUND::Lock(sfx);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::Unlock()
{
	if (SoundRecorder.IsRecording())
	{
		PDXRESULT r1 = SoundRecorder.Write( GetDSoundBuffer(), GetDSoundBufferSize() );
		PDXRESULT r2 = DIRECTSOUND::Unlock();

		if (PDX_SUCCEEDED(r2))
		{
			SoundRecorder.NotifySize();
		}
		else
		{
			SoundRecorder.Close();
		}

		return r2;
	}

	return DIRECTSOUND::Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT SOUNDMANAGER::EnablePAL(const BOOL p)
{
	if (pal != p)
	{
		pal = p;
		ChangedSampleParameters = TRUE;
		return UpdateSoundParameters() ? PDX_OK : PDX_FAILURE;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::ResetSoundParameters()
{
	PDX_ASSERT(adapters.Size());

	ChangedSampleParameters = TRUE;
	SelectedSampleRate = 0;

	const DIRECTSOUND::ADAPTER& adapter = adapters[SelectedAdapter];

	for (UINT i=0; i < adapter.SampleRates.Size(); ++i)
	{
		if (adapter.SampleRates[i] == 44100)
		{
			SelectedSampleRate = i;
			break;
		}
	}

	SelectedSampleBits = adapter.SampleBits16 ? 16 : 8;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL SOUNDMANAGER::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
    	case WM_INITDIALOG:

			SoundRecorder.Stop( FALSE );
			UpdateDialog( hDlg );
     		return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_SOUND_ENABLE:
				{
					const bool checked = IsDlgButtonChecked(hDlg,IDC_SOUND_ENABLE) == BST_CHECKED ? true : false;

					if (checked != bool(enabled))
					{
						enabled = checked;
						OnEnable(hDlg);
					}

					return TRUE;
				}

				case IDC_SOUND_DEVICE:
     				
					if(HIWORD(wParam) == CBN_SELCHANGE)
					{
						ChangedDevice = TRUE;
						SelectedAdapter = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_SOUND_DEVICE));
						ResetSoundParameters();
       					UpdateDialog(hDlg);
					}	     
					return TRUE;

				case IDC_SOUND_SAMPLE_RATE:

					if(HIWORD(wParam) == CBN_SELCHANGE)
					{
						ChangedSampleParameters = TRUE;
						SelectedSampleRate = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_SOUND_SAMPLE_RATE));
					}
					return TRUE;

				case IDC_SOUND_8_BIT:

					ChangedSampleParameters = TRUE;
					SelectedSampleBits = 8;
					return TRUE;

				case IDC_SOUND_16_BIT:

					ChangedSampleParameters = TRUE;
					SelectedSampleBits = 16;
					return TRUE;

				case IDC_SOUND_SQUARE1:  
					
					context.square1 = IsDlgButtonChecked( hDlg, IDC_SOUND_SQUARE1) == BST_CHECKED ? TRUE : FALSE;  
					return TRUE;

				case IDC_SOUND_SQUARE2:  

					context.square2 = IsDlgButtonChecked( hDlg, IDC_SOUND_SQUARE2) == BST_CHECKED ? TRUE : FALSE;  
					return TRUE;

				case IDC_SOUND_TRIANGLE: 

					context.triangle = IsDlgButtonChecked( hDlg, IDC_SOUND_TRIANGLE) == BST_CHECKED ? TRUE : FALSE; 
					return TRUE;

				case IDC_SOUND_NOISE:  

					context.noise = IsDlgButtonChecked( hDlg, IDC_SOUND_NOISE) == BST_CHECKED ? TRUE : FALSE;    
					return TRUE;

				case IDC_SOUND_DPCM:  

					context.dpcm = IsDlgButtonChecked( hDlg, IDC_SOUND_DPCM) == BST_CHECKED ? TRUE : FALSE;     
					return TRUE;

				case IDC_SOUND_EXTERNAL: 

					context.external = IsDlgButtonChecked( hDlg, IDC_SOUND_EXTERNAL) == BST_CHECKED ? TRUE : FALSE; 
					return TRUE;

				case IDC_SOUND_DEFAULT:

					Reset();
					UpdateDialog(hDlg);
					return TRUE;

				case IDC_SOUND_OK:

					EndDialog(hDlg,0);
					return TRUE;
			}
			return TRUE;

		case WM_VSCROLL:
		{
			const INT volume = DSBVOLUME_MIN - (INT) SendMessage
			(
				GetDlgItem(hDlg,IDC_SOUND_VOLUME),
				TBM_GETPOS,
				WPARAM(0),
				LPARAM(0)
			);

			if (SelectedVolume != volume)
			{
				SelectedVolume = volume;
				ChangedSampleParameters = TRUE;
			}

			return TRUE;
		}

		case WM_HSCROLL:
		{
			const UINT latency = (UINT) SendMessage
			(
				GetDlgItem(hDlg,IDC_SOUND_LATENCY),
				TBM_GETPOS,
				WPARAM(0),
				LPARAM(0)
			);

			if (SelectedLatency != latency)
			{
				SelectedLatency = latency;
				ChangedSampleParameters = TRUE;
			}

			return TRUE;
		}

		case WM_CLOSE:

			EndDialog(hDlg,0);
			return TRUE;

		case WM_DESTROY:

       		return UpdateSoundParameters();
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::OnEnable(HWND hDlg)
{
	ChangedDevice = ChangedSampleParameters = TRUE;

	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_DEVICE      ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_SAMPLE_RATE ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_8_BIT       ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_16_BIT      ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_VOLUME      ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_LATENCY     ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_SQUARE1     ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_SQUARE2     ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_TRIANGLE    ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_NOISE       ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_DPCM        ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_EXTERNAL    ), enabled );
	EnableWindow( GetDlgItem( hDlg, IDC_SOUND_DEFAULT     ), enabled );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL SOUNDMANAGER::UpdateSoundParameters()
{
	context.enabled = enabled;

	const BOOL NeedVolume = (SelectedVolume != DSBVOLUME_MAX);

	if (enabled)
	{
		const DIRECTSOUND::ADAPTER& adapter = adapters[SelectedAdapter];

		if (ChangedDevice)
		{
			ChangedDevice = FALSE;

			if (PDX_FAILED(DIRECTSOUND::Create(&adapter.guid)))
			{
				enabled = FALSE;
				context.enabled = FALSE;
				nes->SetAudioContext( context );
				return FALSE;
			}
		}

		if (ChangedSampleParameters)
		{
			ChangedSampleParameters = FALSE;

			context.SampleRate = adapter.SampleRates[SelectedSampleRate];
			context.SampleBits = SelectedSampleBits;

			const UINT fps = (pal ? NES_FPS_PAL : NES_FPS_NTSC);

			if (PDX_FAILED(SetFormat(context.SampleRate,context.SampleBits,SelectedLatency,fps,NeedVolume)))
			{
				enabled = FALSE;
				context.enabled = FALSE;
				nes->SetAudioContext( context );
				return FALSE;
			}
		}
	}

	context.MaxBufferLength	= GetMaxBufferLength();
	nes->SetAudioContext(context);

	if (NeedVolume)
		SetVolume(SelectedVolume);
	
	SoundRecorder.SetWaveFormat( GetWaveFormat() );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::UpdateDialog(HWND hDlg)
{
	if (!adapters.Size())
	{
		CheckDlgButton(hDlg,IDC_SOUND_ENABLE,FALSE);
		EnableWindow(GetDlgItem(hDlg,IDC_SOUND_ENABLE),FALSE);
		OnEnable(hDlg);
		return;
	}

	{
		HWND item = GetDlgItem(hDlg,IDC_SOUND_DEVICE);

		ComboBox_ResetContent(item);

		for (UINT i=0; i < adapters.Size(); ++i)
			ComboBox_AddString(item,adapters[i].name.Begin());

		ComboBox_SetCurSel(item,SelectedAdapter);
	}

	{
		HWND item = GetDlgItem(hDlg,IDC_SOUND_SAMPLE_RATE);

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
		const BOOL Has8 = adapters[SelectedAdapter].SampleBits8;
		const BOOL Has16 = adapters[SelectedAdapter].SampleBits16;

		EnableWindow( GetDlgItem(hDlg,IDC_SOUND_8_BIT), Has8 );
		EnableWindow( GetDlgItem(hDlg,IDC_SOUND_16_BIT), Has16 );

		if (!Has8 && SelectedSampleBits == 8) SelectedSampleBits = 16;
		else if (!Has16 && SelectedSampleBits == 16) SelectedSampleBits = 8;

		const INT button = (SelectedSampleBits == 16) ? IDC_SOUND_16_BIT : IDC_SOUND_8_BIT;
		CheckRadioButton( hDlg, IDC_SOUND_8_BIT, IDC_SOUND_16_BIT, button );
	}

	{
		CheckDlgButton( hDlg, IDC_SOUND_SQUARE1,  context.square1  );
		CheckDlgButton( hDlg, IDC_SOUND_SQUARE2,  context.square2  );
		CheckDlgButton( hDlg, IDC_SOUND_TRIANGLE, context.triangle );
		CheckDlgButton( hDlg, IDC_SOUND_NOISE,    context.noise    );
		CheckDlgButton( hDlg, IDC_SOUND_DPCM,     context.dpcm     );
		CheckDlgButton( hDlg, IDC_SOUND_EXTERNAL, context.external );
	}

	{
		CheckDlgButton( hDlg, IDC_SOUND_ENABLE, enabled );
	}

	{
		HWND item = GetDlgItem( hDlg, IDC_SOUND_LATENCY );

		SendMessage( item, TBM_SETRANGE, WPARAM(FALSE), LPARAM(MAKELONG(1,10)) );
		SendMessage( item, TBM_SETPOS, WPARAM(TRUE), LPARAM(SelectedLatency) );
	}

	{
		HWND item = GetDlgItem( hDlg, IDC_SOUND_VOLUME );

		SendMessage( item, TBM_SETRANGE, WPARAM(FALSE), LPARAM(MAKELONG(DSBVOLUME_MIN,DSBVOLUME_MAX)) );
		SendMessage( item, TBM_SETPOS, WPARAM(TRUE), LPARAM(DSBVOLUME_MIN - SelectedVolume) );
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
			KeepGoing = application.OnQuestion
			(
				"Sound Recorder",
				"You've wasted 100MB of harddrive space on the wave file " 
				"recording so far. Are you sure you want to continue?"
			);

			if (!KeepGoing)
			{
				Close();
				return;
			}
		}
		else
		{
			PDXSTRING msg(NextSize / NST_1MB);
			msg += " MB written to wave file..";

			application.StartScreenMsg( msg, 1500 );
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
			
			if (PDX_FAILED(WaveFile->Open( file, WaveFormat )))
			{
				Close();
				file.Clear();
				return;
			}
			
			if (msg)
				application.StartScreenMsg( "Sound recording started..", 1500 );
		}
		else
		{
			if (msg)
				application.StartScreenMsg( "Sound recording resumed..", 1500 );
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
			application.StartScreenMsg( "Sound recording stopped..", 1500 );
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
		application.StartScreenMsg( "Sound recording reset..", 1500 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::OnBrowse(HWND hDlg)
{
	PDXSTRING file;
	file.Buffer().Resize( NST_MAX_PATH );
	file.Buffer().Front() = '\0';

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = hWnd;
	ofn.hInstance       = application.GetHInstance();
	ofn.lpstrFilter     = "Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file.Begin();
	ofn.lpstrTitle      = "Choose Wave File";
	ofn.nMaxFile        = NST_MAX_PATH;
	ofn.Flags           = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

	if (GetSaveFileName(&ofn))
	{
		file.Validate();

		if (file.Length() && file.GetFileExtension().IsEmpty())
			file += ".wav";

		SetDlgItemText( hDlg, IDC_SOUND_RECORD_FILE, file );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::OnClear(HWND hDlg)
{
	SetDlgItemText( hDlg, IDC_SOUND_RECORD_FILE, "" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID SOUNDMANAGER::SOUNDRECORDER::OnOk(HWND hDlg)
{
	PDXSTRING name;
	
	name.Buffer().Resize( NST_MAX_PATH );	
	GetDlgItemText( hDlg, IDC_SOUND_RECORD_FILE, name.Begin(), NST_MAX_PATH );
	
	name.Validate();
	
	if (name.Length() && name.GetFileExtension().IsEmpty())
		name += ".wav";

	if (file != name)
	{
		file = name;
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

			SetDlgItemText( hDlg, IDC_SOUND_RECORD_FILE, file );
			return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
     			case IDC_SOUND_RECORD_BROWSE: OnBrowse( hDlg );     return TRUE;
				case IDC_SOUND_RECORD_CLEAR:  OnClear( hDlg );      return TRUE;
    			case IDC_SOUND_RECORD_OK:	  OnOk( hDlg );
	       		case IDC_SOUND_RECORD_CANCEL: EndDialog( hDlg, 0 ); return TRUE;
			}
			return FALSE;

		case WM_CLOSE:

			EndDialog( hDlg, 0 );
			return TRUE;
	}

	return FALSE;
}
