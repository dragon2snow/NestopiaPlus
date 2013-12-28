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
		Video::Palette::UpdateCaller Video::Palette::updateCallback;

		void Video::EnableUnlimSprites(bool state)
		{
			emulator.ppu.EnableUnlimSprites( state );
		}

		bool Video::AreUnlimSpritesEnabled() const
		{
			return emulator.ppu.AreUnlimSpritesEnabled();
		}

		int Video::GetBrightness() const
		{
			return emulator.renderer.GetBrightness();
		}

		int Video::GetSaturation() const
		{
			return emulator.renderer.GetSaturation();
		}

		int Video::GetContrast() const
		{
			return emulator.renderer.GetContrast();
		}

		int Video::GetSharpness() const
		{
			return emulator.renderer.GetSharpness();
		}

		int Video::GetColorResolution() const
		{
			return emulator.renderer.GetColorResolution();
		}

		int Video::GetColorBleed() const
		{
			return emulator.renderer.GetColorBleed();
		}

		int Video::GetColorArtifacts() const
		{
			return emulator.renderer.GetColorArtifacts();
		}

		int Video::GetColorFringing() const
		{
			return emulator.renderer.GetColorFringing();
		}

		int Video::GetHue() const
		{
			return emulator.renderer.GetHue();
		}

		Result Video::SetBrightness(int value)
		{
			return emulator.renderer.SetBrightness( value );
		}

		Result Video::SetSaturation(int value)
		{
			return emulator.renderer.SetSaturation( value );
		}

		Result Video::SetContrast(int value)
		{
			return emulator.renderer.SetContrast( value );
		}

		Result Video::SetSharpness(int value)
		{
			return emulator.renderer.SetSharpness( value );
		}

		Result Video::SetColorResolution(int value)
		{
			return emulator.renderer.SetColorResolution( value );
		}

		Result Video::SetColorBleed(int value)
		{
			return emulator.renderer.SetColorBleed( value );
		}

		Result Video::SetColorArtifacts(int value)
		{
			return emulator.renderer.SetColorArtifacts( value );
		}

		Result Video::SetColorFringing(int value)
		{
			return emulator.renderer.SetColorFringing( value );
		}

		Result Video::SetHue(int value)
		{
			return emulator.renderer.SetHue( value );
		}

		void Video::EnableFieldMerging(bool state)
		{
			emulator.renderer.EnableFieldMerging( state );
		}

		bool Video::IsFieldMergingEnabled() const
		{
			return emulator.renderer.IsFieldMergingEnabled();
		}

		Result Video::SetRenderState(const RenderState& state)
		{
			emulator.ppu.EnableEmphasis( state.bits.count != 8 );
			const Result result = emulator.renderer.SetState( state );

			if (NES_SUCCEEDED(result))
				emulator.UpdateColorMode();

			return result;
		}

		Result Video::GetRenderState(RenderState& state) const
		{
			return emulator.renderer.GetState( state );
		}

		Result Video::Blit(Output& output)
		{
			if (emulator.renderer.IsReady())
			{
				emulator.renderer.Blit( output, emulator.ppu.GetBurstPhase() );
				return RESULT_OK;
			}

			return RESULT_ERR_NOT_READY;
		}

		Video::Decoder::Decoder(DecoderPreset preset)
		{
			switch (preset)
			{
				case DECODER_CONSUMER:

					axes[0].angle = 105;
					axes[0].gain  = 0.78f;
					axes[1].angle = 236;
					axes[1].gain  = 0.33f;
					axes[2].angle = 0;
					axes[2].gain  = 1.0f;
					boostYellow   = false;
					break;

				case DECODER_ALTERNATIVE:

					axes[0].angle = 105;
					axes[0].gain  = 0.570f;
					axes[1].angle = 251;
					axes[1].gain  = 0.351f;
					axes[2].angle = 15;
					axes[2].gain  = 1.015f;
					boostYellow   = true;
					break;

				default:

					axes[0].angle = 105;
					axes[0].gain  = 0.570f;
					axes[1].angle = 251;
					axes[1].gain  = 0.351f;
					axes[2].angle = 15;
					axes[2].gain  = 1.015f;
					boostYellow   = false;
					break;
			}
		}

		bool Video::Decoder::operator == (const Decoder& decoder) const
		{
			for (uint i=0; i < NUM_AXES; ++i)
			{
				if (axes[i].angle != decoder.axes[i].angle || axes[i].gain != decoder.axes[i].gain)
					return false;
			}

			if (boostYellow != decoder.boostYellow)
				return false;

			return true;
		}

		Result Video::SetDecoder(const Decoder& decoder)
		{
			return emulator.renderer.SetDecoder( decoder );
		}

		const Video::Decoder& Video::GetDecoder() const
		{
			return emulator.renderer.GetDecoder();
		}

		Result Video::SetPaletteMode(const Palette::Mode paletteMode)
		{
			return emulator.UpdateColorMode
			(
				paletteMode == Palette::MODE_RGB    ? Emulator::COLORMODE_RGB :
				paletteMode == Palette::MODE_CUSTOM ? Emulator::COLORMODE_CUSTOM :
                                                      Emulator::COLORMODE_YUV
			);
		}

		Video::Palette::Mode Video::GetPaletteMode() const
		{
			switch (emulator.renderer.GetPaletteType())
			{
				case Core::Video::Renderer::PALETTE_YUV:
					return Palette::MODE_YUV;

				case Core::Video::Renderer::PALETTE_CUSTOM:
					return Palette::MODE_CUSTOM;
			}

			return Palette::MODE_RGB;
		}

		Video::Palette::Mode Video::GetDefaultPaletteMode() const
		{
			NST_COMPILE_ASSERT( Core::Video::Renderer::DEFAULT_PALETTE == Core::Video::Renderer::PALETTE_YUV );
			return Palette::MODE_YUV;
		}

		Result Video::SetCustomPalette(Palette::Colors colors)
		{
			return emulator.renderer.LoadCustomPalette( colors );
		}

		void Video::ResetCustomPalette()
		{
			return emulator.renderer.ResetCustomPalette();
		}

		Video::Palette::Colors Video::GetPaletteColors() const
		{
			return emulator.renderer.GetPalette();
		}
	}
}
