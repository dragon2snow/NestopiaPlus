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

#ifndef NST_DIALOG_VIDEO_H
#define NST_DIALOG_VIDEO_H

#pragma once

#include "NstWindowRect.hpp"
#include "NstWindowDialog.hpp"
#include "NstDirect2D.hpp"
#include "../core/api/NstApiVideo.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Video
		{
		public:

			typedef DirectX::Direct2D::Adapter Adapter;
			typedef DirectX::Direct2D::Adapters Adapters;
			typedef DirectX::Direct2D::Mode Mode;
			typedef Adapter::Modes Modes;

			enum
			{
				NES_WIDTH      = Nes::Video::Output::WIDTH,
				NES_HEIGHT     = Nes::Video::Output::HEIGHT,
				NES_CLIP	   = 8,
				NTSC_WIDTH	   = Nes::Video::Output::NTSC_WIDTH,
				NTSC_HEIGHT	   = Nes::Video::Output::NTSC_HEIGHT,
				NTSC_CLIP	   = 8,
				DEFAULT_WIDTH  = 640,
				DEFAULT_HEIGHT = 480,
				DEFAULT_BPP    = 16,
				SCREEN_MATCHED = 8,
				SCREEN_STRETCHED = INT_MAX
			};

			Video(Managers::Emulator&,const Adapters&,const Managers::Paths&,const Configuration&);
			~Video();

			void Save(Configuration&) const;
			void LoadGamePalette(const Path&);
			void UnloadGamePalette();
			void SavePalette(Path&) const;

			void GetRenderState(Nes::Video::RenderState&,Rect&,Nes::Machine::Mode,const Window::Generic) const;
			const Rect GetNesRect(const Nes::Machine::Mode) const;
			ibool PutTextureInVideoMemory() const;
			Modes::const_iterator GetDialogMode() const;

		private:

			struct Handlers;

			uint GetFullscreenScaleMethod() const;
			void UpdateFullscreenScaleMethod(uint);

			enum FilterType
			{
				FILTER_NONE,
				FILTER_SCANLINES,
				FILTER_NTSC,
				FILTER_2XSAI,
				FILTER_SCALEX,
				FILTER_HQX,
				NUM_FILTERS
			};

			struct Filter
			{
				Filter();

				enum
				{
					SCANLINES_NONE = 0,
					SCANLINES_BRIGHT,
					SCANLINES_DARK,
					TYPE_2XSAI = 0,
					TYPE_SUPER2XSAI,
					TYPE_SUPEREAGLE,
					TYPE_SCALE2X = 2,
					TYPE_SCALE3X = 3,
					TYPE_HQ2X = 2,
					TYPE_HQ3X = 3
				};

				uchar attributes[3];
				bool bilinear;
			};

			struct Settings
			{
				Settings();

				struct Rects
				{
					Rect ntsc;
					Rect pal;
				};

				enum TexMem
				{
					TEXMEM_AUTO,
					TEXMEM_VIDMEM,
					TEXMEM_SYSMEM
				};

				Adapters::const_iterator adapter;
				TexMem texMem;
				Modes::const_iterator mode;
				Filter* filter;
				Filter filters[NUM_FILTERS];
				Rects rects;
				uint fullscreenScale;
				Path palette;
				Path lockedPalette;
				Nes::Video::Palette::Mode lockedMode;
				ibool autoHz;
			};

			ibool OnInitDialog        (Param&);
			ibool OnHScroll           (Param&);
			ibool OnInitFilterDialog  (Param&);
			ibool OnCmdDevice         (Param&);
			ibool OnCmdMode           (Param&);
			ibool OnCmdFilter         (Param&);
			ibool OnCmdFilterSettings (Param&);
			ibool OnCmdTexMem         (Param&);
			ibool OnCmdBitDepth       (Param&);
			ibool OnCmdRam            (Param&);
			ibool OnCmdColorsReset    (Param&);
			ibool OnCmdPalType        (Param&);
			ibool OnCmdPalBrowse      (Param&);
			ibool OnCmdPalClear       (Param&);
			ibool OnCmdPalEditor      (Param&);
			ibool OnCmdAutoHz         (Param&);
			ibool OnCmdDefault        (Param&);
			ibool OnCmdOk             (Param&);
			ibool OnCmdFilterDefault  (Param&);
			ibool OnCmdFilterCancel   (Param&);
			ibool OnCmdFilterOk       (Param&);

			void UpdateDevice(Mode);
			void UpdateResolutions(Mode);
			void UpdateFilters();
			void UpdateTexMem() const;
			void UpdateRects() const;
			void UpdateColors() const;
			void UpdatePalette() const;
			void ImportPalette(Path&,Managers::Paths::Alert);
			void ValidateRects();

			void ResetRects();
			void ResetColors();

			Modes::const_iterator GetDefaultMode() const;

			static void UpdateScreen(HWND);

			Settings settings;
			const Adapters& adapters;
			Nes::Video nes;
			Dialog dialog;
			const Managers::Paths& paths;

		public:

			void Open()
			{
				dialog.Open();
			}

			const Rect& GetInputRect(const Nes::Machine::Mode mode) const
			{
				return (mode == Nes::Machine::NTSC ? settings.rects.ntsc : settings.rects.pal);														
			}
  
			Modes::const_iterator GetMode() const
			{
				return settings.mode;
			}

			Adapters::const_iterator GetAdapter() const
			{
				return settings.adapter;
			}

			Adapter::Filter GetDirect2dFilter() const
			{
				if (settings.filter->bilinear && (settings.adapter->filters & Adapter::FILTER_BILINEAR))
					return Adapter::FILTER_BILINEAR;
				else
					return Adapter::FILTER_NONE;
			}

			uint GetFullscreenScale() const
			{
				return settings.fullscreenScale;
			}

			void SetFullscreenScale(uint scale)
			{
				settings.fullscreenScale = scale;
			}

			ibool UseAutoFrequency() const
			{
				return settings.autoHz;
			}
		};
	}
}

#endif
