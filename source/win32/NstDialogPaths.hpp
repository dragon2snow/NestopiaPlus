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

#ifndef NST_DIALOG_PATHS_H
#define NST_DIALOG_PATHS_H

#pragma once

#include "NstCollectionBitSet.hpp"
#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Paths
		{
		public:

			typedef String::Path<false> Directory;

			enum Type
			{
				DIR_IMAGE,
				DIR_DATA,
				DIR_STATE,
				DIR_SCRIPT,
				DIR_IPS,
				DIR_SCREENSHOT
			};

			enum ScreenShotFormat
			{
				SCREENSHOT_PNG,
				SCREENSHOT_JPEG,
				SCREENSHOT_BMP
			};

			enum
			{
				NUM_DIRS = DIR_SCREENSHOT + 1,
				NUM_SCREENSHOTS = SCREENSHOT_BMP + 1
			};

			enum
			{
				USE_LAST_IMAGE_DIR,
				USE_LAST_SCRIPT_DIR,
				READONLY_CARTRIDGE,
				AUTO_IMPORT_STATE_SLOTS,
				AUTO_EXPORT_STATE_SLOTS,
				IPS_AUTO_PATCH,
				COMPRESS_STATES,
				NUM_FLAGS
			};

			explicit Paths(const Configuration&);
			~Paths();

			void Save(Configuration&) const;
			String::Generic GetScreenShotExtension() const;

		private:

			struct Handlers;

			enum
			{
				ACTIVE,
				DEFAULT
			};

			struct Settings
			{
				Settings();

				struct Flags : Collection::BitSet
				{
					inline Flags();
				};

				Flags flags;
				Directory dirs[NUM_DIRS][2];
				ScreenShotFormat screenShotFormat;
			};

			void Update(ibool);
			void CreateFolder(Directory&,String::Generic) const;

			ibool OnInitDialog (Param&);
			ibool OnCmdBrowse  (Param&);
			ibool OnCmdDefault (Param&);
			ibool OnCmdOk      (Param&);
			ibool OnDestroy    (Param&);

			Settings settings;
			Dialog dialog;

			struct Lut
			{
				ushort type;
				ushort dlg;
				cstring cfg;

				static const Lut dirs[NUM_DIRS];
				static const Lut flags[NUM_FLAGS];
				static const Lut screenShots[NUM_SCREENSHOTS];
			};

		public:

			void Open()
			{
				dialog.Open();
			}

			const Directory& GetDirectory(Type type) const
			{
				return settings.dirs[type][ACTIVE];
			}

			ibool GetSetting(uint flag) const
			{
				return settings.flags[flag];
			}

			ScreenShotFormat GetScreenShotFormat() const
			{
				return settings.screenShotFormat;
			}
		};
	}
}

#endif
