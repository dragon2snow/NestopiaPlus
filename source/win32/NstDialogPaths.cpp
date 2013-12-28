////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#include "NstResourceString.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogPaths.hpp"
#include "NstDialogBrowse.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_PATHS_BATTERY     - IDC_PATHS_IMAGE == IDC_PATHS_BATTERY_BROWSE     - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_NSP         - IDC_PATHS_IMAGE == IDC_PATHS_NSP_BROWSE         - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_NST         - IDC_PATHS_IMAGE == IDC_PATHS_NST_BROWSE         - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_SAMPLES     - IDC_PATHS_IMAGE == IDC_PATHS_SAMPLES_BROWSE     - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_IPS         - IDC_PATHS_IMAGE == IDC_PATHS_IPS_BROWSE         - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_SCREENSHOTS - IDC_PATHS_IMAGE == IDC_PATHS_SCREENSHOTS_BROWSE - IDC_PATHS_IMAGE_BROWSE
		);

		struct Paths::Lut
		{
			struct A
			{
				ushort type;
				ushort dlg;
				cstring cfg;
				tstring def;
			};

			struct B
			{
				ushort type;
				ushort dlg;
				cstring cfg;
			};

			struct C
			{
				ushort type;
				ushort dlg;
				tstring cfg;
			};

			static const A dirs[NUM_DIRS];
			static const B flags[NUM_FLAGS];
			static const C screenShots[NUM_SCREENSHOTS];
		};

		const Paths::Lut::A Paths::Lut::dirs[NUM_DIRS] =
		{
			{ DIR_IMAGE,      IDC_PATHS_IMAGE,       "files path image",      _T(""              ) },
			{ DIR_SAVE,       IDC_PATHS_BATTERY,     "files path save",       _T("save\\"        ) },
			{ DIR_STATE,      IDC_PATHS_NST,         "files path state",      _T("states\\"      ) },
			{ DIR_SCRIPT,     IDC_PATHS_NSP,         "files path script",     _T("scripts\\"     ) },
			{ DIR_SAMPLES,    IDC_PATHS_SAMPLES,     "files path samples",    _T("samples\\"     ) },
			{ DIR_IPS,        IDC_PATHS_IPS,         "files path ips",        _T("ips\\"         ) },
			{ DIR_SCREENSHOT, IDC_PATHS_SCREENSHOTS, "files path screenshot", _T("screenshots\\" ) }
		};

		const Paths::Lut::B Paths::Lut::flags[NUM_FLAGS] =
		{
			{ USE_LAST_IMAGE_DIR,      IDC_PATHS_IMAGE_LAST,      "files use last image path"  },
			{ USE_LAST_SCRIPT_DIR,     IDC_PATHS_NSP_LAST,        "files use last script path" },
			{ READONLY_CARTRIDGE,      IDC_PATHS_BATTERY_PROTECT, "files readonly cartridge"   },
			{ AUTO_IMPORT_STATE_SLOTS, IDC_PATHS_NST_AUTO_IMPORT, "files auto import states"   },
			{ AUTO_EXPORT_STATE_SLOTS, IDC_PATHS_NST_AUTO_EXPORT, "files auto export states"   },
			{ IPS_AUTO_PATCH,          IDC_PATHS_IPS_AUTO_APPLY,  "files auto ips patching"    },
			{ COMPRESS_STATES,         IDC_PATHS_NST_COMPRESS,    "files compress states"      }
		};

		const Paths::Lut::C Paths::Lut::screenShots[NUM_SCREENSHOTS] =
		{
			{ SCREENSHOT_PNG,  IDC_PATHS_SCREENSHOTS_PNG,  _T( "png" ) },
			{ SCREENSHOT_JPEG, IDC_PATHS_SCREENSHOTS_JPEG, _T( "jpg" ) },
			{ SCREENSHOT_BMP,  IDC_PATHS_SCREENSHOTS_BMP,  _T( "bmp" ) }
		};

		struct Paths::Handlers
		{
			static const MsgHandler::Entry<Paths> messages[];
			static const MsgHandler::Entry<Paths> commands[];
		};

		const MsgHandler::Entry<Paths> Paths::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Paths::OnInitDialog }
		};

		const MsgHandler::Entry<Paths> Paths::Handlers::commands[] =
		{
			{ IDC_PATHS_IMAGE_BROWSE,       &Paths::OnCmdBrowse      },
			{ IDC_PATHS_BATTERY_BROWSE,     &Paths::OnCmdBrowse      },
			{ IDC_PATHS_NST_BROWSE,         &Paths::OnCmdBrowse      },
			{ IDC_PATHS_NSP_BROWSE,         &Paths::OnCmdBrowse      },
			{ IDC_PATHS_SAMPLES_BROWSE,     &Paths::OnCmdBrowse      },
			{ IDC_PATHS_IPS_BROWSE,         &Paths::OnCmdBrowse      },
			{ IDC_PATHS_SCREENSHOTS_BROWSE, &Paths::OnCmdBrowse      },
			{ IDC_PATHS_IMAGE_LAST,         &Paths::OnCmdLastVisited },
			{ IDC_PATHS_NSP_LAST,           &Paths::OnCmdLastVisited },
			{ IDC_PATHS_DEFAULT,            &Paths::OnCmdDefault     },
			{ IDOK,                         &Paths::OnCmdOk          }
		};

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

		Paths::Paths(const Configuration& cfg)
		: dialog(IDD_PATHS,this,Handlers::messages,Handlers::commands)
		{
			for (uint i=0; i < NUM_DIRS; ++i)
			{
				settings.dirs[Lut::dirs[i].type] = cfg[Lut::dirs[i].cfg];
				UpdateDirectory( i );
			}

			for (uint i=0; i < NUM_FLAGS; ++i)
			{
				if (cfg[Lut::flags[i].cfg] == Configuration::YES)
				{
					settings.flags[Lut::flags[i].type] = true;
				}
				else if (cfg[Lut::flags[i].cfg] == Configuration::NO)
				{
					settings.flags[Lut::flags[i].type] = false;
				}
			}

			const GenericString format( cfg["files screenshot format"] );

			if (format.Length())
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
				cfg[Lut::dirs[i].cfg].Quote() = settings.dirs[Lut::dirs[i].type];

			for (uint i=0; i < NUM_FLAGS; ++i)
				cfg[Lut::flags[i].cfg].YesNo() = settings.flags[Lut::flags[i].type];

			cfg["files screenshot format"] = Lut::screenShots[settings.screenShotFormat].cfg;
		}

		GenericString Paths::GetScreenShotExtension() const
		{
			switch (settings.screenShotFormat)
			{
				case SCREENSHOT_JPEG: return _T("jpg");
				case SCREENSHOT_BMP:  return _T("bmp");
				default:              return _T("png");
			}
		}

		void Paths::UpdateDirectory(const uint i)
		{
			const Path def( Application::Instance::GetExePath(Lut::dirs[i].def) );

			Path& dir = settings.dirs[Lut::dirs[i].type];

			bool useDef;

			if (dir.Length())
			{
				dir.MakePretty( true );
				useDef = (dir == def);
			}
			else
			{
				dir = def;
				useDef = true;
			}

			if (::GetFileAttributes( dir.Ptr() ) == INVALID_FILE_ATTRIBUTES)
			{
				if (useDef || User::Confirm( Resource::String(IDS_FILE_ASK_CREATE_DIR).Invoke(dir) ))
				{
					if (!::CreateDirectory( dir.Ptr(), NULL ))
					{
						if (useDef)
							dir = Application::Instance::GetExePath().Directory();
						else
							User::Fail( IDS_FILE_ERR_CREATE_DIR );
					}
				}
			}
		}

		void Paths::Update(const ibool reset) const
		{
			for (uint i=0; i < NUM_DIRS; ++i)
				dialog.Edit( Lut::dirs[i].dlg ) << (reset ? Application::Instance::GetExePath(Lut::dirs[i].def).Ptr() : settings.dirs[Lut::dirs[i].type].Ptr());

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

			UpdateLastVisited();
		}

		void Paths::UpdateLastVisited() const
		{
			ibool unchecked = dialog.CheckBox( IDC_PATHS_IMAGE_LAST ).Unchecked();
			dialog.Control( IDC_PATHS_IMAGE ).Enable( unchecked );
			dialog.Control( IDC_PATHS_IMAGE_BROWSE ).Enable( unchecked );

			unchecked = dialog.CheckBox( IDC_PATHS_NSP_LAST ).Unchecked();
			dialog.Control( IDC_PATHS_NSP ).Enable( unchecked );
			dialog.Control( IDC_PATHS_NSP_BROWSE ).Enable( unchecked );
		}

		ibool Paths::OnInitDialog(Param&)
		{
			Update( false );
			return true;
		}

		ibool Paths::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
				Update( true );

			return true;
		}

		ibool Paths::OnCmdBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = IDC_PATHS_IMAGE + (param.Button().GetId() - IDC_PATHS_IMAGE_BROWSE);
				dialog.Edit( id ).Try() << Browser::SelectDirectory().Ptr();
			}

			return true;
		}

		ibool Paths::OnCmdLastVisited(Param& param)
		{
			if (param.Button().Clicked())
				UpdateLastVisited();

			return true;
		}

		ibool Paths::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				for (uint i=0; i < NUM_DIRS; ++i)
				{
					dialog.Edit( Lut::dirs[i].dlg ) >> settings.dirs[Lut::dirs[i].type];
					UpdateDirectory( i );
				}

				for (uint i=0; i < NUM_FLAGS; ++i)
					settings.flags[Lut::flags[i].type] = dialog.CheckBox( Lut::flags[i].dlg ).Checked();

				for (uint i=0; i < NUM_SCREENSHOTS; ++i)
				{
					if (dialog.RadioButton( Lut::screenShots[i].dlg ).Checked())
					{
						settings.screenShotFormat = (ScreenShotFormat) Lut::screenShots[i].type;
						break;
					}
				}

				dialog.Close();
			}

			return true;
		}
	}
}
