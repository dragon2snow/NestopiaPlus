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

#ifndef NST_APPLICATION_H
#define NST_APPLICATION_H

#define NST_MAX_PATH PDX_MAX(MAX_PATH,512)

#include "NstGraphicManager.h"
#include "NstFileManager.h"
#include "NstTimer.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class SOUNDMANAGER;
class INPUTMANAGER;
class GAMEGENIEMANAGER;
class SAVESTATEMANAGER;
class MOVIEMANAGER;
class VSDIPSWITCHMANAGER;
class FDSMANAGER;
class PREFERENCES;
class ROMINFO;
class LOGFILEMANAGER;
class HELPMANAGER;
class USERINPUTMANAGER;

////////////////////////////////////////////////////////////////////////////////////////
// window class
////////////////////////////////////////////////////////////////////////////////////////

class APPLICATION
{
public:

	APPLICATION();
	~APPLICATION();

	PDXRESULT Init(HINSTANCE,const CHAR* const,const INT);

	INT Loop();

	HINSTANCE GetInstance() const;
	HWND      GetHWnd()     const;
	HMENU     GetMenu()     const;

	BOOL IsActive()   const;
	BOOL IsWindowed() const;
	BOOL IsMenuSet()  const;

	VOID ResetTimer();

	const RECT& NesRect() const;

	SAVESTATEMANAGER& GetSaveStateManager();
	FILEMANAGER&      GetFileManager();     
	GAMEGENIEMANAGER& GetGameGenieManager();
	GRAPHICMANAGER&   GetGraphicManager();  
	SOUNDMANAGER&     GetSoundManager();
	PREFERENCES&      GetPreferences();
	MOVIEMANAGER&     GetMovieManager();
	TIMERMANAGER&     GetTimerManager();

	NES::MODE GetNesMode() const;

	PDXRESULT BeginDialogMode();
	PDXRESULT EndDialogMode();

	VOID UpdateWindowSizes(const UINT,const UINT);

	VOID StartScreenMsg(const CHAR* const,const UINT);
	VOID LogOutput(const CHAR* const) const;
	VOID LogSeparator() const;

	PDXRESULT   OnError     (const CHAR* const);
	PDXRESULT   OnWarning   (const CHAR* const);
	BOOL        OnQuestion  (const CHAR* const,const CHAR* const);
	BOOL        OnUserInput	(const CHAR* const,const CHAR* const,PDXSTRING&);

	enum
	{
		TIMER_ID_SCREEN_MSG = 1,
		TIMER_ID_AUTO_SAVE
	};

private:

	enum FILETYPE
	{
		FILE_ALL,
		FILE_NSP
	};

	static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
	LRESULT MsgProc(const HWND,const UINT,const WPARAM,const LPARAM);

	VOID ExecuteFrame();

	PDXRESULT SwitchScreen();
	PDXRESULT PushWindow();
	PDXRESULT PopWindow();
	
	BOOL ChangeMenuText(const ULONG,CHAR* const) const;
	VOID UpdateRecentFiles();
	VOID UpdateWindowsTitle();
	VOID UpdateWindowItems();
	VOID UpdateWindowRect(RECT&,const RECT&);
	VOID UpdateWindowRect(RECT&);
	VOID UpdateControllerPorts();
	VOID UpdateDynamicMenuItems();
	VOID UpdateSoundRecorderMenu();
	VOID UpdateMovieMenu();
	VOID UpdateFdsMenu();
	VOID UpdateNsf();
	VOID ResetSaveSlots(const BOOL=FALSE);
	VOID ApplyWindowSizing();

	PDX_NO_INLINE VOID OutputScreenMsg();
	PDX_NO_INLINE VOID OutputNsfInfo();

	VOID OnPaint();
	VOID OnMode(const UINT);
	VOID OnPort(const UINT);
	VOID OnAutoSelectController();
	VOID OnExitSizeMove();
	VOID OnActive();
	VOID OnInactive(const BOOL=FALSE);
	VOID OnRecent(const UINT);
	VOID OnLoadScript();
	VOID OnSaveScript();
	VOID OnLoadState();
	VOID OnSaveState();
	VOID OnSaveStateSlot(const UINT);
	VOID OnLoadStateSlot(const UINT);
	VOID OnSoundRecorder(const UINT);
	BOOL OnCommand(const WPARAM);
	BOOL OnSysCommand(const WPARAM);
	VOID OnMouseMove(const LPARAM);
	VOID OnLeftMouseButtonDown(const LPARAM);
	VOID OnLeftMouseButtonUp();
	VOID OnRightMouseButtonDown();
	VOID OnMove(const LPARAM);
	VOID OnSize(const LPARAM);
	VOID OnActivate(const WPARAM);
	VOID OnOpen(const FILETYPE,const INT=-1);
	VOID OnReset(const BOOL);
	VOID OnPower(const BOOL);
	VOID OnClose();
	VOID OnCloseWindow();
	UINT GetAspectRatio() const;
	VOID OnFdsInsertDisk(const UINT);
	VOID OnFdsEjectDisk();
	VOID OnFdsSide(const UINT);
	VOID OnNsfCommand(const NES::IO::NSF::OP);
	VOID OnWindowSize(const UINT,UINT=0,UINT=0);
	VOID OnHideMenu();
	VOID OnShowMenu();
	VOID OnExit();
	VOID OnPause();

	BOOL AcceleratorEnabled;

	HWND hWnd;
	BOOL active;
	BOOL windowed;
	BOOL locked;
	BOOL ready;
	BOOL error;
	BOOL AutoSelectController;
	
	HINSTANCE hInstance;
	HMENU     hMenu;
	HCURSOR   hCursor;
	BOOL      UseZapper;
	BOOL      ScreenInvisible;
	BOOL      InBackground;
	
	HBRUSH hMenuWindowBrush;
	HBRUSH hMenuFullscreenBrush;

	RECT rcWindow;
	RECT rcClient;
	RECT rcScreen;
	RECT rcDefWindow;
	RECT rcDefClient;
	RECT rcRestoreWindow;

	TIMERMANAGER*       const TimerManager;
	GRAPHICMANAGER*     const GraphicManager;
	SOUNDMANAGER*       const SoundManager;
	INPUTMANAGER*       const InputManager;
	FILEMANAGER*        const FileManager;
	FDSMANAGER*         const FdsManager;
	GAMEGENIEMANAGER*   const GameGenieManager;
	SAVESTATEMANAGER*   const SaveStateManager;
	MOVIEMANAGER*       const MovieManager;
	VSDIPSWITCHMANAGER* const VsDipSwitchManager;
	PREFERENCES*        const preferences;
	LOGFILEMANAGER*     const LogFileManager;
	ROMINFO*            const RomInfo;
	HELPMANAGER*        const HelpManager;
	USERINPUTMANAGER*   const UserInputManager;

	NES::IO::INPUT NesInput;
	NES::MODE NesMode;

	UINT SelectPort[5];

	NES::MACHINE nes;

	struct NSFINFO
	{
		PDX_NO_INLINE VOID Clear()
		{
			name.Clear();
			artist.Clear();
			copyright.Clear();
			song.Clear();
		}

		PDXSTRING name;
		PDXSTRING artist;
		PDXSTRING copyright;
		PDXSTRING song;
	};

	NSFINFO NsfInfo;

	static PDXSTRING ScreenMsg;

	PDXSTRING CfgFileName;

	static VOID CALLBACK OnScreenMsgEnd(HWND,UINT,UINT_PTR,DWORD);
	static BOOL CALLBACK HelpAboutDlgProc(HWND,UINT,WPARAM,LPARAM);
};

extern APPLICATION application;

#include "NstApplication.inl"

#endif

