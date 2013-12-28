////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#ifndef NST_DIALOG_VIDEOFILTERS_H
#define NST_DIALOG_VIDEOFILTERS_H

#pragma once

#include "NstWindowDialog.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerEmulator.hpp"

namespace Nestopia
{
	namespace Window
	{
		class VideoFilters
		{
		public:

			enum Type
			{
				TYPE_NONE,
				TYPE_SCANLINES,
				TYPE_NTSC,
				TYPE_2XSAI,
				TYPE_SCALEX,
				TYPE_HQX,
				NUM_TYPES
			};

			enum
			{
				ATR_SCANLINES_MAX = 100,
				ATR_FIELDMERGING_AUTO = 0,
				ATR_FIELDMERGING_ON,
				ATR_FIELDMERGING_OFF,
				ATR_CONTRAST_MIN = -5,
				ATR_CONTRAST_MAX = +5,
				ATR_SHARPNESS_MIN = -5,
				ATR_SHARPNESS_MAX = +5,
				ATR_2XSAI = 0,
				ATR_SUPER2XSAI,
				ATR_SUPEREAGLE,
				ATR_SCALE2X = 0,
				ATR_SCALE3X,
				ATR_HQ2X = 0,
				ATR_HQ3X
			};

			enum
			{
				ATR_BILINEAR = 0,
				ATR_TYPE = 1,
				ATR_SCANLINES = 1,
				ATR_NO_WIDESCREEN = 2,
				ATR_FIELDMERGING = 3,
				ATR_CONTRAST = 4,
				ATR_SHARPNESS = 5
			};

			struct Settings
			{
				i8 attributes[8];

				void Reset()
				{
					std::memset( attributes, 0, sizeof(attributes) );
				}

				Settings()
				{
					Reset();
				}
			};

			VideoFilters(uint,Settings&,uint,bool);

			static Type Load(const Configuration&,Settings (&)[NUM_TYPES],uint,bool);
			static void Save(Configuration&,const Settings (&)[NUM_TYPES],Type);

			enum
			{
				MAX_2X_SIZE   = NST_MAX(Nes::Video::Output::WIDTH*2,Nes::Video::Output::HEIGHT*2),
				MAX_3X_SIZE   = NST_MAX(Nes::Video::Output::WIDTH*3,Nes::Video::Output::HEIGHT*3),
				MAX_NTSC_SIZE = NST_MAX(Nes::Video::Output::NTSC_WIDTH,Nes::Video::Output::NTSC_HEIGHT)
			};

		private:

			struct Handlers;

			ibool OnInitDialog    (Param&);
			ibool OnHScroll       (Param&);
			ibool OnCmdOk         (Param&);
			ibool OnCmdCancel     (Param&);
			ibool OnCmdDefault    (Param&);
			ibool OnCmdNtscFields (Param&);

			Settings& settings;
			const uint maxScreenSize;
			const ibool canDoBilinear;
			Dialog dialog;
		};
	}
}

#endif
