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

inline HWND APPLICATION::GetHWnd() const
{ 
	return hWnd; 
}

inline HMENU APPLICATION::GetMenu() const
{ 
	return hMenu ? hMenu : ::GetMenu(hWnd); 
}

inline NES::MACHINE& APPLICATION::GetNes()
{
	return nes;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL APPLICATION::IsActive()   const { return active;        }
inline BOOL APPLICATION::IsWindowed() const { return windowed;      }
inline BOOL APPLICATION::IsMenuSet()  const { return hMenu == NULL; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline SAVESTATEMANAGER& APPLICATION::GetSaveStateManager() { return SaveStateManager;   }
inline FILEMANAGER&      APPLICATION::GetFileManager()      { return FileManager;        }
inline GAMEGENIEMANAGER& APPLICATION::GetGameGenieManager() { return GameGenieManager;   }
inline GRAPHICMANAGER&   APPLICATION::GetGraphicManager()   { return GraphicManager;     }
inline PREFERENCES&      APPLICATION::GetPreferences()      { return preferences;        }
inline MOVIEMANAGER&     APPLICATION::GetMovieManager()     { return MovieManager;       }
inline SOUNDMANAGER&     APPLICATION::GetSoundManager()     { return SoundManager;       }
inline TIMERMANAGER&     APPLICATION::GetTimerManager()     { return TimerManager;       }
inline LOGFILEMANAGER&   APPLICATION::LogFile()             { return log;                }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline NES::MODE APPLICATION::GetNesMode() const
{ 
	return NesMode; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID APPLICATION::StartScreenMsg(const UINT duration,const T& t)
{
	ScreenMsg = t;
	SetTimer( hWnd, TIMER_ID_SCREEN_MSG, duration, OnScreenMsgEnd );
}

template<class T,class U> 
VOID APPLICATION::StartScreenMsg(const UINT duration,const T& t,const U& u)
{
	ScreenMsg  = t;
	ScreenMsg += u;

	SetTimer( hWnd, TIMER_ID_SCREEN_MSG, duration, OnScreenMsgEnd );
}

template<class T,class U,class V> 
VOID APPLICATION::StartScreenMsg(const UINT duration,const T& t,const U& u,const V& v)
{
	ScreenMsg  = t;
	ScreenMsg += u;
	ScreenMsg += v;

	SetTimer( hWnd, TIMER_ID_SCREEN_MSG, duration, OnScreenMsgEnd );
}

template<class T,class U,class V,class W> 
VOID APPLICATION::StartScreenMsg(const UINT duration,const T& t,const U& u,const V& v,const W& w)
{
	ScreenMsg  = t;
	ScreenMsg += u;
	ScreenMsg += v;
	ScreenMsg += w;

	SetTimer( hWnd, TIMER_ID_SCREEN_MSG, duration, OnScreenMsgEnd );
}
