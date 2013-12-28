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

#define application APPLICATION::GetSingleton()

#define NST_WM_CMDLINE 1
#define NST_CLASS_NAME "Nestopia Window"
#define NST_WINDOW_NAME "Nestopia"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#include "../paradox/PdxSingleton.h"
#include "../NstNes.h"
#include "NstStatusBar.h"

class TIMERMANAGER;
class GRAPHICMANAGER;
class SOUNDMANAGER;
class INPUTMANAGER;
class FILEMANAGER;
class FDSMANAGER;  
class GAMEGENIEMANAGER;
class SAVESTATEMANAGER;
class MOVIEMANAGER;
class VSDIPSWITCHMANAGER;
class PREFERENCES;
class LOGFILE;
class ROMINFO;
class HELPMANAGER; 
class USERINPUTMANAGER;
class CONFIGFILE;

////////////////////////////////////////////////////////////////////////////////////////
// window class
////////////////////////////////////////////////////////////////////////////////////////

class APPLICATION : public PDXSINGLETON<APPLICATION>
{
public:

	APPLICATION(HINSTANCE,const CHAR* const,const INT);
	~APPLICATION();

	INT Run();

	HWND  GetHWnd() const;
	HMENU GetMenu() const;
	HINSTANCE GetInstance() const;

	BOOL IsActive()   const;
	BOOL IsWindowed() const;
	BOOL IsMenuSet()  const;
	BOOL IsRunning()  const;
	BOOL IsPassive()  const;
						
	NES::MACHINE& GetNes();

	PDX_NO_INLINE VOID RefreshCursor(const BOOL=FALSE);
	PDX_NO_INLINE VOID UpdateWindowSizes(const UINT,const UINT);

	SAVESTATEMANAGER& GetSaveStateManager  ();
	FILEMANAGER&      GetFileManager       ();     
	GAMEGENIEMANAGER& GetGameGenieManager  ();
	GRAPHICMANAGER&   GetGraphicManager    ();  
	USERINPUTMANAGER& GetUserInputManager  ();
	SOUNDMANAGER&     GetSoundManager      ();
	PREFERENCES&      GetPreferences       ();
	MOVIEMANAGER&     GetMovieManager      ();
	TIMERMANAGER&     GetTimerManager      ();
	STATUSBAR&        GetStatusBar         ();

	NES::MODE GetNesMode() const;

	PDX_NO_INLINE VOID OnLoadStateSlot (const UINT);
	PDX_NO_INLINE VOID OnSaveStateSlot (const UINT);

	enum
	{
		TIMER_ID_SCREEN_MSG = 1,
		TIMER_ID_AUTO_SAVE
	};

	template<class T>                         
	VOID StartScreenMsg(const UINT,const T&);

	template<class T,class U>
	VOID StartScreenMsg(const UINT,const T&,const U&);

	template<class T,class U,class V>
	VOID StartScreenMsg(const UINT,const T&,const U&,const V&);

	template<class T,class U,class V,class W> 
	VOID StartScreenMsg(const UINT,const T&,const U&,const V&,const W&);

private:

	enum FILETYPE
	{
		FILE_ALL,
		FILE_NSP,
		FILE_INPUT
	};

	static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
	LRESULT MsgProc(const HWND,const UINT,const WPARAM,const LPARAM);

	VOID ExecuteImage();
	PDX_NO_INLINE VOID ExecuteNsf();

	PDX_NO_INLINE VOID SwitchScreen();
	PDX_NO_INLINE VOID PushWindow();
	PDX_NO_INLINE VOID PopWindow();
	PDX_NO_INLINE VOID UpdateRecentFiles();
	PDX_NO_INLINE VOID UpdateWindowItems();
	PDX_NO_INLINE VOID UpdateSoundRecorderMenu();
	PDX_NO_INLINE VOID UpdateFdsMenu();
	PDX_NO_INLINE VOID DisplayMsg();
	PDX_NO_INLINE VOID OutputNsfInfo();
	PDX_NO_INLINE VOID DisplayFPS(BOOL=TRUE);
	PDX_NO_INLINE BOOL ChangeMenuText(const ULONG,CHAR* const) const;
	PDX_NO_INLINE VOID SetScreenMsg(const UINT,const BOOL);
	
	enum CFG_OP
	{
		CFG_LOAD,
		CFG_SAVE
	};

	PDX_NO_INLINE BOOL InitConfigFile(CONFIGFILE&,const CFG_OP);

	VOID UpdateControllerPorts();
	VOID UpdateDynamicMenuItems();
	VOID UpdateMovieMenu();
	VOID UpdateNsf();
	VOID ResetSaveSlots(const BOOL=FALSE);
	VOID GetScreenRect(RECT&) const;
	
	BOOL IsChecked(const UINT) const;

	INT GetMenuHeight() const;

	PDX_NO_INLINE VOID OnOpen(const FILETYPE,const VOID* const=NULL);
	PDX_NO_INLINE VOID OnPort(const UINT);
	PDX_NO_INLINE VOID OnAutoSelectController();
	PDX_NO_INLINE VOID OnActive();
	PDX_NO_INLINE VOID OnInactive(const BOOL=FALSE);
	PDX_NO_INLINE BOOL OnCommand(const WPARAM);
	PDX_NO_INLINE VOID OnPower(const BOOL);
	PDX_NO_INLINE VOID OnReset(const BOOL);
	PDX_NO_INLINE UINT GetAspectRatio() const;
	PDX_NO_INLINE VOID OnNsfCommand(const NES::IO::NSF::OP);
	PDX_NO_INLINE VOID OnWindowSize(const UINT,const BOOL=FALSE);
	PDX_NO_INLINE VOID OnToggleStatusBar();
	PDX_NO_INLINE VOID OnToggleMenu();
	PDX_NO_INLINE VOID OnToggleFPS();
	PDX_NO_INLINE VOID OnHideMenu();
	PDX_NO_INLINE VOID OnShowMenu();
	PDX_NO_INLINE BOOL OnCopyData(const LPARAM);
	PDX_NO_INLINE VOID OnTop();
	PDX_NO_INLINE VOID SetThreadPriority(INT);
	PDX_NO_INLINE VOID OnPaint(const BOOL=FALSE);

	VOID OnMode(const UINT);
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
	VOID OnUnlimitedSprites();
	VOID OnNsfInBackground();
	VOID OnSizeMove(const LPARAM);
	VOID OnActivate(const WPARAM);
	VOID OnClose();
	VOID OnCloseWindow();
	VOID OnSaveScreenShot();
	VOID OnFdsInsertDisk(const UINT);
	VOID OnFdsEjectDisk();
	VOID OnFdsSide(const UINT);
	VOID OnPause();
	VOID OnHelp(const UINT);

	INT GetDesiredPriority() const;

	INT ThreadPriority;

	BOOL AcceleratorEnabled;
	BOOL active;
	BOOL windowed;
	BOOL AutoSelectController;
	
	HMENU        hMenu;
	HCURSOR      hCursor;
	HACCEL const hAccel;
	BOOL         UseZapper;
	BOOL         ShowFPS;
	BOOL         InBackground;
	BOOL         WindowVisible;
	BOOL         ExitSuccess;
	
	RECT rcWindow;
	RECT rcScreen;
	RECT rcDesktop;
	RECT rcDesktopClient;

	HWND hWnd;
	HINSTANCE const hInstance;

	STATUSBAR*          StatusBar;
	TIMERMANAGER*       TimerManager;
	GRAPHICMANAGER*     GraphicManager;
	SOUNDMANAGER*       SoundManager;
	INPUTMANAGER*       InputManager;
	FILEMANAGER*        FileManager;
	FDSMANAGER*         FdsManager;
	GAMEGENIEMANAGER*   GameGenieManager;
	SAVESTATEMANAGER*   SaveStateManager;
	MOVIEMANAGER*       MovieManager;
	VSDIPSWITCHMANAGER* VsDipSwitchManager;
	PREFERENCES*        preferences;
	LOGFILE*            log;
	ROMINFO*            RomInfo;
	HELPMANAGER*        HelpManager;
	USERINPUTMANAGER*   UserInputManager;

	PDXSTRING ScreenMsg;

	NES::MODE NesMode;
	UINT SelectPort[5];

	NES::MACHINE nes;

	struct NSFINFO
	{
		VOID Clear()
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

	static VOID CALLBACK OnScreenMsgEnd(HWND,UINT,UINT_PTR,DWORD);
	static BOOL CALLBACK HelpAboutDlgProc(HWND,UINT,WPARAM,LPARAM);
};

#include "NstApplication.inl"

#endif

