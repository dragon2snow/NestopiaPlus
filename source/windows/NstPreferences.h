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

	PREFERENCES();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);

	inline BOOL PriorityControl()        const { return prioritycontrol;    }
	inline BOOL EmulateImmediately()     const { return emulateimmediately; }
	inline BOOL RunInBackground()        const { return background;         }
	inline BOOL StartUpFullScreen()      const { return fullscreen;         }
	inline BOOL PowerOffOnClose()        const { return closepoweroff;      }
	inline BOOL ConfirmExit()		     const { return confirmexit;        }
	inline BOOL ConfirmReset()           const { return confirmreset;       }
	inline BOOL SaveSettings()           const { return savesettings;       }
	inline BOOL SaveLauncher()           const { return savelauncher;       }
	inline BOOL SaveLogFile()            const { return savelogfile;        }
	inline BOOL NoWarnings()             const { return nowarnings;         }
	inline BOOL ShowWarnings()           const { return !nowarnings;        }
	inline BOOL UseDatabase()            const { return usedatabase;        }
	inline BOOL AllowMultipleInstances() const { return multipleinstances;  }
	
	inline INT GetDefaultPriority() const
	{ return DefaultPriority; }

private:

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);
	VOID CloseDialog();

	static BOOL ReadRegKey();

	VOID Reset();
	VOID UpdateDialog(HWND);
	VOID SetContext(HWND=NULL);
	
	static VOID DeleteKey(HKEY,const PDXSTRING&);

	BOOL emulateimmediately;
	BOOL background;
	BOOL fullscreen;
	BOOL nowarnings;    
	BOOL closepoweroff;
	BOOL confirmexit;
	BOOL confirmreset;
	BOOL prioritycontrol;
	BOOL usedatabase;
	BOOL multipleinstances;
	BOOL savesettings;
	BOOL savelauncher;
	BOOL savelogfile;

	enum
	{
		FILE_NES,
		FILE_UNF,
		FILE_FDS,
		FILE_NSF,
		FILE_NSP,
		NUM_FILE_TYPES
	};

	struct ASSOCIATION
	{
		ASSOCIATION()
		: enabled(FALSE)
		{}

		VOID Set(const CHAR* const ext,const CHAR* const dsc,const UINT ico)
		{
			extension = ext;
			desc = dsc;
			icon = ico;
		}

		BOOL enabled;
		PDXSTRING extension;
		PDXSTRING desc;
		UINT icon;
	};

	VOID RegisterFile      (const ASSOCIATION&,const BOOL);
	VOID UnregisterFile    (const ASSOCIATION&);
	VOID UpdateAssociation (ASSOCIATION&);

	ASSOCIATION associations[NUM_FILE_TYPES];

	BOOL FilesUpdated;

	INT DefaultPriority;
};

#endif

