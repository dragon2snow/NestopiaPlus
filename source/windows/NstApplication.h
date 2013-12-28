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
#define application APPLICATION::GetSingleton()

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#include "../paradox/PdxSingleton.h"
#include "../paradox/PdxFile.h"
#include "resource/resource.h"
#include "NstGraphicManager.h"
#include "NstFileManager.h"
#include "NstTimerManager.h"
#include "NstLogFileManager.h"
#include "NstSoundManager.h"
#include "NstInputManager.h"
#include "NstFdsManager.h"
#include "NstSaveStateManager.h"
#include "NstMovieManager.h"
#include "NstGameGenieManager.h"
#include "NstVsDipSwitchManager.h"
#include "NstPreferences.h"
#include "NstRomInfo.h"
#include "NstHelpManager.h"
#include "NstUserInputManager.h"
#include "NstConfigFile.h"

////////////////////////////////////////////////////////////////////////////////////////
// window class
////////////////////////////////////////////////////////////////////////////////////////

class APPLICATION : public PDXSINGLETON<APPLICATION>
{
public:

	APPLICATION(HINSTANCE,const CHAR* const,const INT) throw(const CHAR*);
	~APPLICATION();

	INT Run();

	HWND  GetHWnd() const;
	HMENU GetMenu() const;

	BOOL IsActive()   const;
	BOOL IsWindowed() const;
	BOOL IsMenuSet()  const;

	NES::MACHINE& GetNes();

	PDX_NO_INLINE VOID RefreshCursor(const BOOL=FALSE);
	PDX_NO_INLINE VOID UpdateWindowSizes(const UINT,const UINT);

	SAVESTATEMANAGER& GetSaveStateManager();
	FILEMANAGER&      GetFileManager();     
	GAMEGENIEMANAGER& GetGameGenieManager();
	GRAPHICMANAGER&   GetGraphicManager();  
	SOUNDMANAGER&     GetSoundManager();
	PREFERENCES&      GetPreferences();
	MOVIEMANAGER&     GetMovieManager();
	TIMERMANAGER&     GetTimerManager();
	LOGFILEMANAGER&   LogFile();

	NES::MODE GetNesMode() const;

	PDX_NO_INLINE PDXRESULT OnWarning       (const CHAR* const);
	PDX_NO_INLINE BOOL      OnQuestion      (const CHAR* const,const CHAR* const);
	PDX_NO_INLINE BOOL      OnUserInput	    (const CHAR* const,const CHAR* const,PDXSTRING&);
	PDX_NO_INLINE VOID      OnLoadStateSlot (const UINT);
	PDX_NO_INLINE VOID      OnSaveStateSlot (const UINT);

	enum
	{
		TIMER_ID_SCREEN_MSG = 1,
		TIMER_ID_AUTO_SAVE
	};

	template<class T>                         
	PDX_NO_INLINE VOID StartScreenMsg(const UINT,const T&);

	template<class T,class U>
	PDX_NO_INLINE VOID StartScreenMsg(const UINT,const T&,const U&);

	template<class T,class U,class V>
	PDX_NO_INLINE VOID StartScreenMsg(const UINT,const T&,const U&,const V&);

	template<class T,class U,class V,class W> 
	PDX_NO_INLINE VOID StartScreenMsg(const UINT,const T&,const U&,const V&,const W&);

private:

	enum FILETYPE
	{
		FILE_ALL,
		FILE_NSP
	};

	static HWND Init(HINSTANCE);

	static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
	LRESULT MsgProc(const HWND,const UINT,const WPARAM,const LPARAM);

	VOID ExecuteFrame();

	PDX_NO_INLINE VOID SwitchScreen();
	PDX_NO_INLINE VOID PushWindow();
	PDX_NO_INLINE VOID PopWindow();
	PDX_NO_INLINE VOID UpdateRecentFiles();
	PDX_NO_INLINE VOID UpdateWindowItems();
	PDX_NO_INLINE VOID UpdateSoundRecorderMenu();
	PDX_NO_INLINE VOID UpdateFdsMenu();
	PDX_NO_INLINE VOID ApplyWindowSizing();
	PDX_NO_INLINE VOID OutputScreenMsg();
	PDX_NO_INLINE VOID OutputNsfInfo();
	PDX_NO_INLINE BOOL ChangeMenuText(const ULONG,CHAR* const) const;
	
	enum CFG_OP
	{
		CFG_LOAD,
		CFG_SAVE
	};

	static PDX_NO_INLINE BOOL InitConfigFile(CONFIGFILE&,const CFG_OP);

	VOID UpdateWindowRect(RECT&,const RECT&);
	VOID UpdateWindowRect(RECT&);
	VOID UpdateControllerPorts();
	VOID UpdateDynamicMenuItems();
	VOID UpdateMovieMenu();
	VOID UpdateNsf();
	VOID ResetSaveSlots(const BOOL=FALSE);

	PDX_NO_INLINE VOID OnOpen(const FILETYPE,const INT=-1);
	PDX_NO_INLINE VOID OnPort(const UINT);
	PDX_NO_INLINE VOID OnAutoSelectController();
	PDX_NO_INLINE VOID OnActive();
	PDX_NO_INLINE VOID OnInactive(const BOOL=FALSE);
	PDX_NO_INLINE BOOL OnCommand(const WPARAM);
	PDX_NO_INLINE VOID OnPower(const BOOL);
	PDX_NO_INLINE VOID OnReset(const BOOL);
	PDX_NO_INLINE UINT GetAspectRatio() const;
	PDX_NO_INLINE VOID OnNsfCommand(const NES::IO::NSF::OP);
	PDX_NO_INLINE VOID OnWindowSize(const UINT,UINT=0,UINT=0);
	PDX_NO_INLINE VOID OnHideMenu();
	PDX_NO_INLINE VOID OnShowMenu();

	VOID OnPaint();
	VOID OnMode(const UINT);
	VOID OnExitSizeMove();
	VOID OnRecent(const UINT);
	VOID OnLoadNsp();
	VOID OnSaveNsp();
	VOID OnLoadState();
	VOID OnSaveState();
	VOID OnSoundRecorder(const UINT);
	BOOL OnSysCommand(const WPARAM);
	VOID OnMouseMove(const LPARAM);
	VOID OnLeftMouseButtonDown(const LPARAM);
	VOID OnLeftMouseButtonUp();
	VOID OnRightMouseButtonDown();
	VOID OnMove(const LPARAM);
	VOID OnSize(const LPARAM);
	VOID OnActivate(const WPARAM);
	VOID OnClose();
	VOID OnCloseWindow();
	VOID OnSaveScreenShot();
	VOID OnFdsInsertDisk(const UINT);
	VOID OnFdsEjectDisk();
	VOID OnFdsSide(const UINT);
	VOID OnPause();
	VOID OnHelp(const UINT);

	BOOL AcceleratorEnabled;
	BOOL active;
	BOOL windowed;
	BOOL ready;
	BOOL AutoSelectController;
	UINT FrameSkips;
	
	HMENU        hMenu;
	HCURSOR      hCursor;
	HACCEL const hAccel;
	BOOL         UseZapper;
	BOOL         InBackground;
	BOOL         ExitSuccess;
	
	RECT rcWindow;
	RECT rcClient;
	RECT rcScreen;
	RECT rcDefWindow;
	RECT rcDefClient;
	RECT rcRestoreWindow;

	HWND const hWnd;

	NES::MACHINE nes;
	NES::MODE NesMode;
	UINT SelectPort[5];

	TIMERMANAGER       TimerManager;
	GRAPHICMANAGER     GraphicManager;
	SOUNDMANAGER       SoundManager;
	INPUTMANAGER       InputManager;
	FILEMANAGER        FileManager;
	FDSMANAGER         FdsManager;
	GAMEGENIEMANAGER   GameGenieManager;
	SAVESTATEMANAGER   SaveStateManager;
	MOVIEMANAGER       MovieManager;
	VSDIPSWITCHMANAGER VsDipSwitchManager;
	PREFERENCES        preferences;
	LOGFILEMANAGER     log;
	ROMINFO            RomInfo;
	HELPMANAGER        HelpManager;
	USERINPUTMANAGER   UserInputManager;

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

	static VOID CALLBACK OnScreenMsgEnd(HWND,UINT,UINT_PTR,DWORD);
	static BOOL CALLBACK HelpAboutDlgProc(HWND,UINT,WPARAM,LPARAM);
};

#include "NstApplication.inl"

#endif

