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

#ifndef NST_APPLICATION_MAIN_H
#define NST_APPLICATION_MAIN_H

#pragma once

#include "NstObjectHeap.hpp"
#include "NstApplicationInstance.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerLogfile.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerLauncher.hpp"
#include "NstManagerMachine.hpp"
#include "NstManagerNetplay.hpp"
#include "NstManagerFds.hpp"
#include "NstManagerTapeRecorder.hpp"
#include "NstManagerDipSwitches.hpp"
#include "NstManagerBarcodeReader.hpp"
#include "NstManagerNsf.hpp"
#include "NstManagerMovie.hpp"
#include "NstManagerSaveStates.hpp"
#include "NstManagerCheats.hpp"
#include "NstManagerRecentFiles.hpp"
#include "NstManagerRecentDirs.hpp"
#include "NstManagerImageInfo.hpp"
#include "NstManagerHelp.hpp"
#include "NstManagerFiles.hpp"
#include "NstManagerInesHeader.hpp"
#include "NstWindowMain.hpp"

namespace Nestopia
{
	namespace Application
	{
		class Main
		{
		public:

			Main(int);
			~Main();

			int Run();

		private:

			void Save();
			void Exit();

			ibool FirstUnloadOnExit();
			ibool IsOkToExit() const;

			ibool OnWinClose           (Window::Param&);
			ibool OnWinQueryEndSession (Window::Param&);

			void OnCmdFileExit(uint);   

			Instance instance;	
			Managers::Emulator emulator;
			Window::Menu menu;
			const Managers::Preferences preferences;
			const Managers::Logfile logfile;
			const Managers::Paths paths;
			const Managers::RecentFiles recentFiles;
			const Managers::RecentDirs recentDirs;
			Window::Main window;
			const Managers::Machine machine;
			const Managers::Netplay netplay;			 
			const Managers::Launcher launcher;		 
			const Managers::Fds fds;
			const Managers::TapeRecorder tapeRecorder;
			const Managers::DipSwitches dipSwitches;
			const Managers::BarcodeReader barcodeReader;
			const Managers::Nsf nsf;					 
			Managers::Movie movie;				 
			const Managers::Cheats cheats;			 
			const Managers::SaveStates saveStates;
			const Managers::ImageInfo imageInfo;
			const Managers::Help help;
			const Managers::InesHeader inesHeader;
			const Managers::Files files;
		};
	}
}

#endif
