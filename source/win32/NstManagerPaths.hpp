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

#ifndef NST_MANAGER_PATHS_H
#define NST_MANAGER_PATHS_H

#pragma once

#include "resource/resource.h"
#include "NstObjectHeap.hpp"
#include "NstObjectRaw.hpp"
#include "NstCollectionBitSet.hpp"
#include "NstCollectionVector.hpp"
#include "NstManagerEmulator.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File;
		class Zip;
	}

	namespace Window
	{
		class Menu;
		class Paths;
	}

	namespace Managers
	{
		class Paths
		{
		public:

			Paths(Emulator&,const Configuration&,Window::Menu&);
			~Paths();

			typedef String::Path<false> Path;
			typedef String::Path<true> TmpPath;

			struct File
			{
				typedef Collection::Buffer Data;
				typedef Collection::BitSet Types;

				enum Type
				{
					NONE    = 0x0000,
					INES    = 0x0001,
					UNIF    = 0x0002,
					FDS     = 0x0004,
					NSF     = 0x0008,
					BATTERY = 0x0010,
					STATE   = 0x0020,
					SLOTS   = 0x0040,
					IPS     = 0x0080,
					MOVIE   = 0x0100,
					SCRIPT  = 0x0200,
					ROM     = 0x0400,
					PALETTE = 0x0800,
					WAVE    = 0x1000,
					ARCHIVE = 0x2000
				};

				enum
				{
					CARTRIDGE = INES|UNIF,
					GAME = CARTRIDGE|FDS,
					IMAGE = GAME|NSF
				};

				enum
				{
					FILEID_INES    = 0x1A53454E,
					FILEID_UNIF    = 0x46494E55,
					FILEID_FDS     = 0x1A534446,
					FILEID_NSF     = 0x4D53454E,
					FILEID_IPS     = 0x43544150,
					FILEID_NST     = 0x1A54534E,
					FILEID_NSV     = 0x1A56534E,
					FILEID_ZIP     = 0x04034B50,
					FILEID_FDS_RAW = 0x494E2A01,
					FILEID_WAV     = 0x46464952
				};

				Type type;
				Path name;
				Data data;

				File()
				: type(NONE) {}
			};

			enum Alert
			{
				QUIETLY,
				NOISY,
				STICKY
			};

			void Save(Configuration&) const;
			
			ibool FindFile(Path&) const;
			ibool LocateFile(Path&,File::Types) const;
			
			TmpPath GetIpsPath(const Path&,File::Type) const;
			TmpPath GetSavePath(const Path&,File::Type) const;
			TmpPath GetScreenShotPath() const;

			ibool SaveSlotExportingEnabled() const;
			ibool SaveSlotImportingEnabled() const;
			ibool UseStateCompression() const;

			ibool CheckFile
			(
		     	Path&,
				File::Types,
				Alert=QUIETLY,
				uint=IDS_TITLE_ERROR
			)   const;

			TmpPath BrowseLoad
			(
		     	File::Types,
				String::Generic=String::Generic()
			)   const;

			TmpPath BrowseSave
			(
		     	File::Types,
				String::Generic=String::Generic(),
				String::Generic=String::Generic()
			)   const;

			File::Type Load
			(
		    	File&,
				File::Types,
				String::Generic=String::Generic(),
				Alert=NOISY,
				uint=IDS_TITLE_ERROR
			)   const;

			ibool Save
			(
       			const Object::ConstRaw&,
				File::Type,
				String::Generic=String::Generic(),
				Alert=NOISY,
				uint=IDS_TITLE_ERROR
			)   const;

			const String::Generic GetDefaultDirectory(File::Types) const;

		private:

			class Filter;

			enum
			{
				RECENT_DIR_IMAGE,
				RECENT_DIR_SCRIPT,
				NUM_RECENT_DIRS
			};

			void OnMenu(uint);
			void OnEmuEvent(Emulator::Event);

			static cstring GetDefaultExtension(File::Types);
			
			void UpdateSettings();
			void UpdateRecentDirectory(const TmpPath&,File::Types) const;
			
			static File::Type CheckFile
			(
		     	File::Types,
				uint,
				uint
			);

			static File::Type LoadFromFile
			(
		     	Path&,
				File::Data*,
				File::Types
			);

			static File::Type LoadFromArchive
			(
		     	const Io::Zip&,
				Path&,
				File::Data*,
				const String::Generic&,
				File::Types
			);

			Emulator& emulator;
			const Window::Menu& menu;
			mutable Path recentDirs[NUM_RECENT_DIRS];
			Object::Heap<Window::Paths> dialog;

			static cstring const recentDirCfgNames[NUM_RECENT_DIRS];
		};
	}
}

#endif
