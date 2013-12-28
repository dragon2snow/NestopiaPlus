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

inline HINSTANCE APPLICATION::GetInstance()	const
{
	return hInstance;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL APPLICATION::IsRunning() const
{
	return active && nes.IsOn() && !nes.IsPaused() && (nes.IsImage() || nes.IsNsf());
}

inline BOOL APPLICATION::IsPassive() const
{
	return nes.IsOff() || nes.IsPaused() || nes.IsNsf();
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

inline SAVESTATEMANAGER& APPLICATION::GetSaveStateManager () { return *SaveStateManager;   }
inline FILEMANAGER&      APPLICATION::GetFileManager      () { return *FileManager;        }
inline GAMEGENIEMANAGER& APPLICATION::GetGameGenieManager () { return *GameGenieManager;   }
inline GRAPHICMANAGER&   APPLICATION::GetGraphicManager   () { return *GraphicManager;     }
inline PREFERENCES&      APPLICATION::GetPreferences      () { return *preferences;        }
inline MOVIEMANAGER&     APPLICATION::GetMovieManager     () { return *MovieManager;       }
inline SOUNDMANAGER&     APPLICATION::GetSoundManager     () { return *SoundManager;       }
inline TIMERMANAGER&     APPLICATION::GetTimerManager     () { return *TimerManager;       }
inline USERINPUTMANAGER& APPLICATION::GetUserInputManager () { return *UserInputManager;    }

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
	const BOOL PrevMsg = bool(ScreenMsg.Length());
	ScreenMsg = t;
	SetScreenMsg( duration, PrevMsg );
}

template<class T,class U> 
VOID APPLICATION::StartScreenMsg(const UINT duration,const T& t,const U& u)
{
	const BOOL PrevMsg = bool(ScreenMsg.Length());
	ScreenMsg = t; 
	ScreenMsg << u;
	SetScreenMsg( duration, PrevMsg );
}

template<class T,class U,class V> 
VOID APPLICATION::StartScreenMsg(const UINT duration,const T& t,const U& u,const V& v)
{
	const BOOL PrevMsg = bool(ScreenMsg.Length());
	ScreenMsg = t; 
	ScreenMsg << u << v;
	SetScreenMsg( duration, PrevMsg );
}

template<class T,class U,class V,class W> 
VOID APPLICATION::StartScreenMsg(const UINT duration,const T& t,const U& u,const V& v,const W& w)
{
	const BOOL PrevMsg = bool(ScreenMsg.Length());
	ScreenMsg = t; 
	ScreenMsg << u << v << w;
	SetScreenMsg( duration, PrevMsg );
}
