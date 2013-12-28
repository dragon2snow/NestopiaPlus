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

#ifndef NST_MANAGER_PATHS_H
#define NST_MANAGER_PATHS_H

#pragma once

#include "language/resource.h"
#include "NstObjectHeap.hpp"
#include "NstCollectionBitSet.hpp"
#include "NstManagerEmulator.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File;
		class Archive;
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

			struct File
			{
				typedef Collection::Buffer Data;
				typedef HeapString Text;
				typedef Collection::BitSet Types;

				enum Type
				{
					NONE    = 0x0000,
					INES    = 0x0001,
					UNIF    = 0x0002,
					FDS     = 0x0004,
					NSF     = 0x0008,
					BATTERY = 0x0010,
					TAPE    = 0x0020,
					STATE   = 0x0040,
					SLOTS   = 0x0080,
					IPS     = 0x0100,
					MOVIE   = 0x0200,
					SCRIPT  = 0x0400,
					ROM     = 0x0800,
					PALETTE = 0x1000,
					WAVE    = 0x2000,
					AVI     = 0x4000,
					ARCHIVE = 0x8000
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
					FILEID_RAR     = 0x21726152,
					FILEID_7Z      = 0xAFBC7A37,
					FILEID_FDS_RAW = 0x494E2A01
				};

				Type type;
				Path name;
				Data data;

				File()
				: type(NONE) {}
			};

			enum Method
			{
				DONT_SUGGEST,
				SUGGEST
			};

			enum Checking
			{
				CHECK_FILE,
				DONT_CHECK_FILE
			};

			enum Alert
			{
				QUIETLY,
				NOISY,
				STICKY
			};

			void Save(Configuration&) const;

			void FixFile(File::Type,Path&) const;
			ibool FindFile(Path&) const;
			ibool LocateFile(Path&,File::Types) const;

			Path GetIpsPath(const Path&,File::Type) const;
			Path GetSavePath(const Path&,File::Type) const;
			Path GetScreenShotPath() const;
			Path GetSamplesPath() const;

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

			Path BrowseLoad
			(
				File::Types,
				GenericString = GenericString(),
				Checking=CHECK_FILE
			)   const;

			Path BrowseSave
			(
				File::Types,
				Method=DONT_SUGGEST,
				GenericString = GenericString()
			)   const;

			File::Type Load
			(
				File&,
				File::Types,
				GenericString = GenericString(),
				Alert=NOISY
			)   const;

			ibool Save
			(
				const void*,
				uint,
				File::Type,
				Path,
				Alert=NOISY
			)   const;

			const Path GetDefaultDirectory(File::Types) const;

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

			static tstring GetDefaultExtension(File::Types);

			void UpdateSettings();
			void UpdateRecentDirectory(const Path&,File::Types) const;

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
				const Io::Archive&,
				Path&,
				File::Data*,
				const GenericString&,
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
