////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstDialogPaths.hpp"
#include "NstDialogBrowse.hpp"

namespace Nestopia
{
	using namespace Window;

	inline Paths::Settings::Flags::Flags()
	: Collection::BitSet
	(
		( 1U << USE_LAST_IMAGE_DIR      ) |
		( 1U << USE_LAST_SCRIPT_DIR     ) |
		( 1U << AUTO_IMPORT_STATE_SLOTS ) |
		( 1U << AUTO_EXPORT_STATE_SLOTS ) |
		( 1U << COMPRESS_STATES         )
	) 
	{}

	Paths::Settings::Settings()
	: screenShotFormat(SCREENSHOT_PNG) {}

	const Paths::Lut Paths::Lut::dirs[NUM_DIRS] =
	{
		{ DIR_IMAGE,	  IDC_PATHS_IMAGE,		 "files path image"    	     },
		{ DIR_DATA,       IDC_PATHS_BATTERY,	 "files path cartridge data" },
		{ DIR_STATE,	  IDC_PATHS_NST,	     "files path state"          },
		{ DIR_SCRIPT,	  IDC_PATHS_NSP,		 "files path script"         },
		{ DIR_IPS,		  IDC_PATHS_IPS,		 "files path ips"		     },
		{ DIR_SCREENSHOT, IDC_PATHS_SCREENSHOTS, "files path screenshot"     }	
	};

	const Paths::Lut Paths::Lut::flags[NUM_FLAGS] =
	{
		{ USE_LAST_IMAGE_DIR,	   IDC_PATHS_IMAGE_LAST,      "files use last image path"  },
		{ USE_LAST_SCRIPT_DIR,	   IDC_PATHS_NSP_LAST,        "files use last script path" },
		{ READONLY_CARTRIDGE,      IDC_PATHS_BATTERY_PROTECT, "files readonly cartridge"   },
		{ AUTO_IMPORT_STATE_SLOTS, IDC_PATHS_NST_AUTO_IMPORT, "files auto import states"   },
		{ AUTO_EXPORT_STATE_SLOTS, IDC_PATHS_NST_AUTO_EXPORT, "files auto export states"   },
		{ IPS_AUTO_PATCH,		   IDC_PATHS_IPS_AUTO_APPLY,  "files auto ips patching"	   },
		{ COMPRESS_STATES,		   IDC_PATHS_NST_COMPRESS,    "files compress states"	   }
	};

	const Paths::Lut Paths::Lut::screenShots[NUM_SCREENSHOTS] =
	{
		{ SCREENSHOT_PNG,  IDC_PATHS_SCREENSHOTS_PNG,  "png" },
		{ SCREENSHOT_JPEG, IDC_PATHS_SCREENSHOTS_JPEG, "jpg" },
		{ SCREENSHOT_BMP,  IDC_PATHS_SCREENSHOTS_BMP,  "bmp" }
	};

	struct Paths::Handlers
	{
		static const MsgHandler::Entry<Paths> messages[];
		static const MsgHandler::Entry<Paths> commands[];
	};

	const MsgHandler::Entry<Paths> Paths::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Paths::OnInitDialog },
		{ WM_DESTROY,    &Paths::OnDestroy    }
	};

	const MsgHandler::Entry<Paths> Paths::Handlers::commands[] =
	{
		{ IDC_PATHS_IMAGE_BROWSE,       &Paths::OnCmdBrowse  },
		{ IDC_PATHS_BATTERY_BROWSE,     &Paths::OnCmdBrowse  },
		{ IDC_PATHS_NSP_BROWSE,         &Paths::OnCmdBrowse  },
		{ IDC_PATHS_NST_BROWSE,         &Paths::OnCmdBrowse  },
		{ IDC_PATHS_SCREENSHOTS_BROWSE, &Paths::OnCmdBrowse  },
		{ IDC_PATHS_IPS_BROWSE,         &Paths::OnCmdBrowse  },
		{ IDC_PATHS_DEFAULT,            &Paths::OnCmdDefault },
		{ IDC_PATHS_OK,                 &Paths::OnCmdOk      }
	};

	Paths::Paths(const Configuration& cfg)
	: dialog(IDD_PATHS,this,Handlers::messages,Handlers::commands)
	{
		settings.dirs[ DIR_IMAGE ][ DEFAULT ] = Application::Instance::GetPath().Directory();

		CreateFolder( settings.dirs[ DIR_DATA       ][ DEFAULT ], "cartridge\\"   );
		CreateFolder( settings.dirs[ DIR_STATE      ][ DEFAULT ], "states\\"      );
		CreateFolder( settings.dirs[ DIR_IPS        ][ DEFAULT ], "ips\\"         );
		CreateFolder( settings.dirs[ DIR_SCRIPT     ][ DEFAULT ], "scripts\\"     );
		CreateFolder( settings.dirs[ DIR_SCREENSHOT ][ DEFAULT ], "screenshots\\" );

		for (uint i=0; i < NUM_DIRS; ++i)
		{
			const uint type = Lut::dirs[i].type;

			settings.dirs[type][ACTIVE] = cfg[Lut::dirs[i].cfg];

			if (settings.dirs[type][ACTIVE].Size())
				settings.dirs[type][ACTIVE].Directory().Validate();
			else
				settings.dirs[type][ACTIVE] = settings.dirs[type][DEFAULT];
		}

		for (uint i=0; i < NUM_FLAGS; ++i)
		{
			if (cfg[Lut::flags[i].cfg] == Configuration::YES)
			{
				settings.flags[Lut::flags[i].type] = TRUE;
			}
			else if (cfg[Lut::flags[i].cfg] == Configuration::NO)
			{
				settings.flags[Lut::flags[i].type] = FALSE;
			}
		}

		const String::Heap& format = cfg["files screenshot format"];

		if (format.Size())
		{
			for (uint i=0; i < NUM_SCREENSHOTS; ++i)
			{
				if (format == Lut::screenShots[i].cfg)
				{
					settings.screenShotFormat = (ScreenShotFormat) Lut::screenShots[i].type;
					break;
				}
			}
		}
	}

	Paths::~Paths()
	{
	}

	void Paths::Save(Configuration& cfg) const
	{
		for (uint i=0; i < NUM_DIRS; ++i)
			cfg[Lut::dirs[i].cfg].Quote() = settings.dirs[Lut::dirs[i].type][ACTIVE];

		for (uint i=0; i < NUM_FLAGS; ++i)
			cfg[Lut::flags[i].cfg].YesNo() = settings.flags[Lut::flags[i].type];

		cfg["files screenshot format"] = Lut::screenShots[settings.screenShotFormat].cfg;
	}

	void Paths::CreateFolder(Directory& dir,const String::Generic name) const
	{
		NST_ASSERT( name.Size() && *name != '\\' && *name != '/' );

		dir = Application::Instance::GetPath().Directory();
		dir << name;

		if (!::CreateDirectory( dir, NULL ) && ::GetLastError() != ERROR_ALREADY_EXISTS)
			dir.Shrink( name.Size() );
	}

	String::Generic Paths::GetScreenShotExtension() const
	{
		switch (settings.screenShotFormat)
		{
			case SCREENSHOT_JPEG: return "jpg";
			case SCREENSHOT_BMP:  return "bmp";
			default:              return "png";
		}
	}

	void Paths::Update(const ibool reset)
	{
		for (uint i=0; i < NUM_DIRS; ++i)
			dialog.Edit( Lut::dirs[i].dlg ) << settings.dirs[Lut::dirs[i].type][reset ? DEFAULT : ACTIVE];

		Settings::Flags flags;

		if (!reset)
			flags = settings.flags;

		for (uint i=0; i < NUM_FLAGS; ++i)
			dialog.CheckBox( Lut::flags[i].dlg ).Check( flags[Lut::flags[i].type] ); 

		ScreenShotFormat screenShotFormat = SCREENSHOT_PNG;

		if (!reset)
			screenShotFormat = settings.screenShotFormat;

		for (uint i=0; i < NUM_SCREENSHOTS; ++i)
			dialog.RadioButton( Lut::screenShots[i].dlg ).Check( Lut::screenShots[i].type == screenShotFormat );
	}

	ibool Paths::OnInitDialog(Param&)
	{
		for (uint i=0; i < NUM_DIRS; ++i)
			dialog.Edit( Lut::dirs[i].dlg ).Limit( _MAX_PATH );

		Update( FALSE );
		return TRUE;
	}

	ibool Paths::OnCmdDefault(Param& param)
	{
		if (param.Button().IsClicked())
			Update( TRUE );

		return TRUE;
	}

	ibool Paths::OnCmdBrowse(Param& param)
	{
		NST_COMPILE_ASSERT
		(
			IDC_PATHS_BATTERY_BROWSE - IDC_PATHS_IMAGE_BROWSE     == IDC_PATHS_BATTERY - IDC_PATHS_IMAGE &&
			IDC_PATHS_NSP_BROWSE - IDC_PATHS_IMAGE_BROWSE         == IDC_PATHS_NSP - IDC_PATHS_IMAGE &&
			IDC_PATHS_NST_BROWSE - IDC_PATHS_IMAGE_BROWSE         == IDC_PATHS_NST - IDC_PATHS_IMAGE &&
			IDC_PATHS_IPS_BROWSE - IDC_PATHS_IMAGE_BROWSE         == IDC_PATHS_IPS - IDC_PATHS_IMAGE &&
			IDC_PATHS_SCREENSHOTS_BROWSE - IDC_PATHS_IMAGE_BROWSE == IDC_PATHS_SCREENSHOTS - IDC_PATHS_IMAGE
		);

		if (param.Button().IsClicked())
		{
			const uint id = IDC_PATHS_IMAGE + param.Button().GetId() - IDC_PATHS_IMAGE_BROWSE;
			dialog.Edit( id ).Try() << Browser::SelectDirectory();
		}

		return TRUE;
	}

	ibool Paths::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}

	ibool Paths::OnDestroy(Param&)
	{	
		for (uint i=0; i < NUM_DIRS; ++i)
		{
			const uint type = Lut::dirs[i].type;

			dialog.Edit( Lut::dirs[i].dlg ) >> settings.dirs[type][ACTIVE];

			if (settings.dirs[type][ACTIVE].Size())
				settings.dirs[type][ACTIVE].Directory().Validate();
			else
				settings.dirs[type][ACTIVE] = settings.dirs[type][DEFAULT];
		}

		for (uint i=0; i < NUM_FLAGS; ++i)
			settings.flags[Lut::flags[i].type] = dialog.CheckBox( Lut::flags[i].dlg ).IsChecked();  

		for (uint i=0; i < NUM_SCREENSHOTS; ++i)
		{
			if (dialog.RadioButton( Lut::screenShots[i].dlg ).IsChecked())
			{
				settings.screenShotFormat = (ScreenShotFormat) Lut::screenShots[i].type;
				break;
			}
		}

		return TRUE;
	}
}
