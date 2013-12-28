////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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
#include "NstManager.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File;
		class Archive;
	}

	namespace Window
	{
		class Paths;
	}

	namespace Managers
	{
		class Paths : Manager
		{
		public:

			Paths(Emulator&,const Configuration&,Window::Menu&);
			~Paths();

			struct File
			{
				File();
				~File();

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
					ID_INES    = NST_FOURCC('N','E','S',0x1A),
					ID_UNIF    = NST_FOURCC('U','N','I','F'),
					ID_FDS     = NST_FOURCC('F','D','S',0x1A),
					ID_NSF     = NST_FOURCC('N','E','S','M'),
					ID_IPS     = NST_FOURCC('P','A','T','C'),
					ID_NST     = NST_FOURCC('N','S','T',0x1A),
					ID_NSV     = NST_FOURCC('N','S','V',0x1A),
					ID_ZIP     = NST_FOURCC('P','K',0x03,0x04),
					ID_RAR     = NST_FOURCC('R','a','r','!'),
					ID_7Z      = NST_FOURCC('7','z',0xBC,0xAF),
					ID_FDS_RAW = NST_FOURCC(0x01,0x2A,0x4E,0x49)
				};

				Type type;
				Path name;
				Data data;
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
			bool FindFile(Path&) const;
			bool LocateFile(Path&,File::Types) const;

			Path GetIpsPath(const Path&,File::Type) const;
			Path GetSavePath(const Path&,File::Type) const;
			Path GetScreenShotPath() const;
			Path GetSamplesPath() const;

			bool SaveSlotExportingEnabled() const;
			bool SaveSlotImportingEnabled() const;
			bool UseStateCompression() const;

			bool CheckFile
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

			bool Save
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

			mutable Path recentDirs[NUM_RECENT_DIRS];
			Object::Heap<Window::Paths> dialog;

			static cstring const recentDirCfgNames[NUM_RECENT_DIRS];
		};
	}
}

#endif
