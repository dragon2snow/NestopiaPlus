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

#ifndef NST_DIALOG_VIDEOFILTERS_H
#define NST_DIALOG_VIDEOFILTERS_H

#pragma once

#include "NstWindowDialog.hpp"

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
				ATR_FIELDMERGING_AUTO = 0,
				ATR_FIELDMERGING_ON,
				ATR_FIELDMERGING_OFF,
				ATR_2XSAI = 0,
				ATR_SUPER2XSAI,
				ATR_SUPEREAGLE,
				ATR_SCALEAX = 0,
				ATR_SCALE2X,
				ATR_SCALE3X,
				ATR_HQAX = 0,
				ATR_HQ2X,
				ATR_HQ3X,
				ATR_HQ4X
			};

			enum
			{
				ATR_BILINEAR = 0,
				ATR_TYPE,
				ATR_SCANLINES = 1,
				ATR_RESCALE_PIC,
				ATR_FIELDMERGING,
				ATR_NO_AUTO_TUNING
			};

			struct Settings
			{
				void Reset(Type);

				i8 attributes[8];
			};

			VideoFilters(Nes::Video,uint,Settings&,uint,bool,Nes::Video::Palette::Mode);

			static Type Load(const Configuration&,Settings (&)[NUM_TYPES],Nes::Video,uint,bool,Nes::Video::Palette::Mode);
			static void Save(Configuration&,const Settings (&)[NUM_TYPES],Nes::Video,Type);

			static void UpdateAutoModes(const Settings (&)[NUM_TYPES],Nes::Video,Nes::Video::Palette::Mode);

			enum
			{
				MAX_2X_SIZE   = NST_MAX(Nes::Video::Output::WIDTH*2,Nes::Video::Output::HEIGHT*2),
				MAX_3X_SIZE   = NST_MAX(Nes::Video::Output::WIDTH*3,Nes::Video::Output::HEIGHT*3),
				MAX_4X_SIZE   = NST_MAX(Nes::Video::Output::WIDTH*4,Nes::Video::Output::HEIGHT*4),
				MAX_NTSC_SIZE = NST_MAX(Nes::Video::Output::NTSC_WIDTH,Nes::Video::Output::NTSC_HEIGHT)
			};

		private:

			static void ResetAutoModes(Nes::Video,Nes::Video::Palette::Mode);

			struct Handlers;

			struct Backup
			{
				Backup(const Settings&,const Nes::Video);

				const Settings settings;
				const i8 sharpness;
				const i8 resolution;
				const i8 bleed;
				const i8 artifacts;
				const i8 fringing;
				bool restore;
			};

			ibool OnInitDialog    (Param&);
			ibool OnDestroy       (Param&);
			ibool OnHScroll       (Param&);
			ibool OnCmdOk         (Param&);
			ibool OnCmdDefault    (Param&);
			ibool OnCmdBilinear   (Param&);
			ibool OnCmdNtscTuning (Param&);
			ibool OnCmdNtscCable  (Param&);
			ibool OnCmd2xSaI      (Param&);
			ibool OnCmdScaleX     (Param&);
			ibool OnCmdHqX        (Param&);

			void UpdateScanlinesSlider() const;
			void UpdateNtscSliders() const;
			void UpdateNtscSlider(int,uint) const;
			void UpdateNtscTuning() const;

			Settings& settings;
			Backup backup;
			const uint maxScreenSize;
			const ibool canDoBilinear;
			const Nes::Video::Palette::Mode paletteMode;
			Nes::Video nes;
			Dialog dialog;

		public:

			void Open()
			{
				dialog.Open();
			}
		};
	}
}

#endif
