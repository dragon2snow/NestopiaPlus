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

#ifndef NST_PREFERENCES_H
#define NST_PREFERENCES_H

#include "NstManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class PREFERENCES : public MANAGER
{
public:

	PREFERENCES(const INT a)
	: MANAGER(a) {}

	inline BOOL HighPriority()         const { return highpriority;       }
	inline BOOL EmulateImmediately()   const { return emulateimmediately; }
	inline BOOL RunInBackground()      const { return background;         }
	inline BOOL RunNsfInBackground()   const { return backgroundnsf;      }
	inline BOOL StartUpFullScreen()    const { return fullscreen;         }
	inline BOOL PowerOffOnClose()      const { return closepoweroff;      }
	inline BOOL HideMenuInFullScreen() const { return hidemenu;           }
	inline BOOL ConfirmExit()		   const { return confirmexit;        }
	inline BOOL LogFileEnabled()       const { return uselogfile;         }
	inline BOOL NoWarnings()           const { return nowarnings;         }
	inline BOOL ShowWarnings()         const { return !nowarnings;        }

private:

	PDXRESULT Create  (CONFIGFILE* const);
	PDXRESULT Destroy (CONFIGFILE* const);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	VOID Reset();
	VOID UpdateDialog(HWND);
	VOID SetContext(HWND=NULL);

   #pragma pack(push,1)

	struct HEADER
	{
		U8 emulateimmediately : 1;
		U8 backgroundnsf      : 1;
		U8 background         : 1;
		U8 fullscreen         : 1;
		U8 nowarnings         : 1;
		U8 closepoweroff      : 1;
		U8 hidemenu           : 1;
		U8 confirmexit        : 1;
		U8 logfile            : 1;
		U8 highpriority       : 1;
	};

   #pragma pack(pop)

	BOOL emulateimmediately;
	BOOL background;
	BOOL backgroundnsf;
	BOOL fullscreen;
	BOOL nowarnings;    
	BOOL closepoweroff;
	BOOL hidemenu;
	BOOL confirmexit;
	BOOL uselogfile;
	BOOL highpriority;

	INT DefPriority;
};

#endif

