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

#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiVideo.hpp"
#include "../NstVideoPalette.hpp"
#include "../NstVideoRenderer.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			Output::Locker Output::lockCallback;
			Output::Unlocker Output::unlockCallback;
		}
	}

	namespace Api
	{
		NST_COMPILE_ASSERT
		(
			Video::Palette::INTERNAL == Core::Video::Palette::INTERNAL &&
			Video::Palette::CUSTOM   == Core::Video::Palette::CUSTOM   &&
			Video::Palette::EMULATED == Core::Video::Palette::EMULATE  
		);

		Video::Palette::UpdateCaller Video::Palette::updateCallback;
		
		void Video::EnableUnlimSprites(bool state)
		{
			emulator.ppu.EnableUnlimSprites( state );
		}
	
		bool Video::AreUnlimSpritesEnabled() const
		{
			return emulator.ppu.AreUnlimSpritesEnabled();
		}
	
		uint Video::GetDefaultBrightness() const
		{
			return Core::Video::Palette::DEFAULT_BRIGHTNESS;
		}
	
		uint Video::GetDefaultSaturation() const
		{
			return Core::Video::Palette::DEFAULT_SATURATION;
		}
	
		uint Video::GetDefaultHue() const
		{
			return Core::Video::Palette::DEFAULT_HUE;
		}
	
		uint Video::GetBrightness() const
		{
			return emulator.palette.GetBrightness();
		}
	
		uint Video::GetSaturation() const
		{
			return emulator.palette.GetSaturation();
		}
	
		uint Video::GetHue() const
		{
			return emulator.palette.GetHue();
		}
	
		Result Video::SetBrightness(uint value)
		{
			return emulator.palette.SetBrightness( value );
		}
	
		Result Video::SetSaturation(uint value)
		{
			return emulator.palette.SetSaturation( value );
		}
	
		Result Video::SetHue(uint value)
		{
			return emulator.palette.SetHue( value );
		}
		
		Result Video::SetRenderState(const RenderState& state)
		{
			emulator.ppu.EnableEmphasis( state.bits.count != 8 );
			return emulator.renderer.SetState( state );
		}
	
		Result Video::GetRenderState(RenderState& state) const
		{
			return emulator.renderer.GetState( state );
		}
	
		Result Video::Blit(Output& output)
		{
			if (emulator.palette.NeedUpdate())
			{
				Palette::Colors const colors = emulator.palette.GetColors();
				emulator.renderer.SetPalette( colors );
				Palette::updateCallback( colors );
			}
	
			if (emulator.renderer.IsReady())
			{
				emulator.renderer.Blit( output );
				return RESULT_OK;
			}

			return RESULT_ERR_NOT_READY;
		}

		Video::Palette Video::GetPalette() const
		{
			return emulator.palette;
		}

		Result Video::Palette::SetMode(Mode mode)
		{
			if (mode == INTERNAL || mode == CUSTOM || mode == EMULATED)
				return palette.SetType( (Core::Video::Palette::Type) mode );

			return RESULT_ERR_INVALID_PARAM;
		}

		Video::Palette::Mode Video::Palette::GetMode() const
		{
			return (Mode) palette.GetType();
		}

		Video::Palette::Mode Video::Palette::GetDefaultMode() const
		{
			return (Mode) Core::Video::Palette::DEFAULT_PALETTE_TYPE;
		}

		Result Video::Palette::SetCustom(Colors colors)
		{
			return palette.SetCustomColors( colors );
		}

		Video::Palette::Colors Video::Palette::GetColors() const
		{
			return palette.GetColorsNoChange();
		}
	}
}
