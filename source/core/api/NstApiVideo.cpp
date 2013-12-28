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

#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiVideo.hpp"
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
			Video::Palette::INTERNAL == Core::Video::Renderer::PALETTE_INTERNAL &&
			Video::Palette::CUSTOM   == Core::Video::Renderer::PALETTE_CUSTOM   &&
			Video::Palette::EMULATED == Core::Video::Renderer::PALETTE_EMULATE  
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
			return Core::Video::Renderer::DEFAULT_BRIGHTNESS;
		}
	
		uint Video::GetDefaultSaturation() const
		{
			return Core::Video::Renderer::DEFAULT_SATURATION;
		}
	
		uint Video::GetDefaultHue() const
		{
			return Core::Video::Renderer::DEFAULT_HUE;
		}
	
		uint Video::GetBrightness() const
		{
			return emulator.renderer.GetBrightness();
		}
	
		uint Video::GetSaturation() const
		{
			return emulator.renderer.GetSaturation();
		}
	
		uint Video::GetHue() const
		{
			return emulator.renderer.GetHue();
		}
	
		Result Video::SetBrightness(uint value)
		{
			return emulator.renderer.SetBrightness( value );
		}
	
		Result Video::SetSaturation(uint value)
		{
			return emulator.renderer.SetSaturation( value );
		}
	
		Result Video::SetHue(uint value)
		{
			return emulator.renderer.SetHue( value );
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
			if (emulator.renderer.IsReady())
			{
				emulator.renderer.Blit( output );
				return RESULT_OK;
			}

			return RESULT_ERR_NOT_READY;
		}

		Video::Palette Video::GetPalette() const
		{
			return emulator.renderer;
		}

		Result Video::Palette::SetMode(Mode mode)
		{
			if (mode == INTERNAL || mode == CUSTOM || mode == EMULATED)
				return renderer.SetPaletteType( (Core::Video::Renderer::PaletteType) mode );

			return RESULT_ERR_INVALID_PARAM;
		}

		Video::Palette::Mode Video::Palette::GetMode() const
		{
			return (Mode) renderer.GetPaletteType();
		}

		Video::Palette::Mode Video::Palette::GetDefaultMode() const
		{
			return (Mode) Core::Video::Renderer::DEFAULT_PALETTE;
		}

		Result Video::Palette::SetCustom(Colors colors)
		{
			return renderer.LoadCustomPalette( colors );
		}

		void Video::Palette::ResetCustom()
		{
			return renderer.ResetCustomPalette();
		}

		Video::Palette::Colors Video::Palette::GetColors() const
		{
			return renderer.GetPalette();
		}
	}
}
