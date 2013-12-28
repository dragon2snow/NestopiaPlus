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

#include <new>
#include "NstMachine.hpp"
#include "NstImage.hpp"
#include "NstCartridge.hpp"
#include "NstCheats.hpp"
#include "NstNsf.hpp"
#include "NstImageDatabase.hpp"
#include "input/NstInpDevice.hpp"
#include "input/NstInpAdapter.hpp"
#include "input/NstInpPad.hpp"
#include "api/NstApiMachine.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Machine::Machine()
		:
		state         (Api::Machine::NTSC),
		frame         (0),
		extPort       (new Input::AdapterTwo( new Input::Pad(cpu,0), new Input::Pad(cpu,1) )),
		expPort       (new Input::Device( cpu )),
		image         (NULL),
		cheats        (NULL),
		ppu           (cpu),
		imageDatabase (NULL)
		{
		}

		Machine::~Machine()
		{
			Unload();

			delete imageDatabase;
			delete cheats;
			delete expPort;

			for (uint ports=extPort->NumPorts(), i=0; i < ports; ++i)
				delete extPort->GetDevice(i);

			delete extPort;
		}

		Result Machine::Load(StdStream stream,uint type)
		{
			NST_ASSERT( image == NULL );

			Image::Context context
			(
				image,
				(Image::Type) type,
				cpu,
				ppu,
				stream,
				imageDatabase
			);

			const Result result = Image::Load( context );

			if (NES_SUCCEEDED(result))
				UpdateColorMode();

			return result;
		}

		void Machine::Unload()
		{
			tracker.Unload();
			frame = 0;

			if (image)
			{
				image->Flush( false, false );
				Image::Unload( image );
				UpdateColorMode();
			}
		}

		Result Machine::UpdateColorMode()
		{
			return UpdateColorMode
			(
				renderer.GetPaletteType() == Video::Renderer::PALETTE_YUV    ? COLORMODE_YUV :
				renderer.GetPaletteType() == Video::Renderer::PALETTE_CUSTOM ? COLORMODE_CUSTOM :
                                                                               COLORMODE_RGB
			);
		}

		Result Machine::UpdateColorMode(const ColorMode colorMode)
		{
			const PpuType ppuType = image ? image->QueryPpu( colorMode == COLORMODE_YUV ) : PPU_RP2C02;
			Video::Renderer::PaletteType paletteType;

			switch (colorMode)
			{
				case COLORMODE_RGB:

					switch (ppuType)
					{
						case PPU_RP2C04_0001: paletteType = Video::Renderer::PALETTE_VS1;  break;
						case PPU_RP2C04_0002: paletteType = Video::Renderer::PALETTE_VS2;  break;
						case PPU_RP2C04_0003: paletteType = Video::Renderer::PALETTE_VS3;  break;
						case PPU_RP2C04_0004: paletteType = Video::Renderer::PALETTE_VS4;  break;
						default:              paletteType = Video::Renderer::PALETTE_PC10; break;
					}
					break;

				case COLORMODE_CUSTOM:

					paletteType = Video::Renderer::PALETTE_CUSTOM;
					break;

				default:

					paletteType = Video::Renderer::PALETTE_YUV;
					break;
			}

			return renderer.SetPaletteType( paletteType );
		}

		Result Machine::PowerOn()
		{
			return Reset( true );
		}

		void Machine::PowerOff()
		{
			frame = 0;

			ppu.ClearScreen();
			cpu.GetApu().ClearBuffers();

			if (image)
				image->Flush( false, tracker.MovieIsInserted() );
		}

		Result Machine::Reset(const bool hard)
		{
			frame = 0;

			try
			{
				cpu.Boot();

				if (!(state & Api::Machine::SOUND))
				{
					InitializeInputDevices();

					cpu.Map( 0x4016U ).Set( this, &Machine::Peek_4016, &Machine::Poke_4016 );
					cpu.Map( 0x4017U ).Set( this, &Machine::Peek_4017, &Machine::Poke_4017 );

					extPort->Reset();
					expPort->Reset();

					ppu.Reset( hard );

					if (image)
						image->Reset( hard );

					cpu.Reset( hard );

					if (cheats)
						cheats->Reset();

					tracker.Reset( hard );

					return image ? image->Flush( true, tracker.MovieIsInserted() ) : RESULT_OK;
				}
				else
				{
					image->Reset( true );
					cpu.Reset( true );
					return RESULT_OK;
				}
			}
			catch (Result result)
			{
				return result;
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		void Machine::SetMode(const Mode mode)
		{
			cpu.SetMode( mode );
			ppu.SetMode( mode );

			if (image)
				image->SetMode( mode );

			renderer.SetMode( mode );
		}

		void Machine::InitializeInputDevices() const
		{
			if (image && image->GetType() == Image::CARTRIDGE)
			{
				const bool arcade = (static_cast<const Cartridge*>(image)->GetInfo().setup.system == SYSTEM_VS);

				extPort->Initialize( arcade );
				expPort->Initialize( arcade );
			}
		}

		Result Machine::SaveState(StdStream stream,bool compress,bool internal)
		{
			if ((state & (Api::Machine::GAME|Api::Machine::ON)) > Api::Machine::ON)
			{
				try
				{
					State::Saver saver( stream, compress, internal );

					saver.Begin('N','S','T',0x1A);
					{
						saver.Begin('N','F','O','\0').Write32( image->GetPrgCrc() ).Write32( frame ).End();

						cpu.SaveState( State::Saver::Subset(saver,'C','P','U','\0').Ref() );
						ppu.SaveState( State::Saver::Subset(saver,'P','P','U','\0').Ref() );
						cpu.GetApu().SaveState( State::Saver::Subset(saver,'A','P','U','\0').Ref() );
						image->SaveState( State::Saver::Subset(saver,'I','M','G','\0').Ref() );

						saver.Begin('P','R','T','\0');
						{
							if (extPort->NumPorts() == 4)
							{
								static_cast<const Input::AdapterFour*>(extPort)->SaveState
								(
									saver, NES_STATE_CHUNK_ID('4','S','C','\0')
								);
							}

							for (uint i=0; i < extPort->NumPorts(); ++i)
								extPort->GetDevice( i )->SaveState( saver, '0' + i );

							expPort->SaveState( saver, 'X' );
						}
						saver.End();
					}
					saver.End();

					return RESULT_OK;
				}
				catch (Result result)
				{
					return result;
				}
				catch (const std::bad_alloc&)
				{
					return RESULT_ERR_OUT_OF_MEMORY;
				}
				catch (...)
				{
					return RESULT_ERR_GENERIC;
				}
			}

			return RESULT_ERR_NOT_READY;
		}

		Result Machine::LoadState(StdStream stream,bool checkCrc)
		{
			if ((state & (Api::Machine::GAME|Api::Machine::ON)) <= Api::Machine::ON)
				return RESULT_ERR_NOT_READY;

			try
			{
				State::Loader loader( stream );

				if (loader.Begin() != NES_STATE_CHUNK_ID('N','S','T',0x1A))
					return RESULT_ERR_INVALID_FILE;

				loader.DigIn();

				while (const dword id = loader.Begin())
				{
					switch (id)
					{
						case NES_STATE_CHUNK_ID('N','F','O','\0'):
						{
							const dword crc = loader.Read32();

							if
							(
								checkCrc && !(state & Api::Machine::DISK) &&
								crc && crc != image->GetPrgCrc() &&
								Api::User::questionCallback( Api::User::QUESTION_NST_PRG_CRC_FAIL_CONTINUE ) == Api::User::ANSWER_NO
							)
							{
								loader.End();
								loader.DigOut();
								return RESULT_ERR_INVALID_CRC;
							}

							frame = loader.Read32();
							break;
						}

						case NES_STATE_CHUNK_ID('C','P','U','\0'):

							cpu.LoadState( State::Loader::Subset(loader).Ref() );
							break;

						case NES_STATE_CHUNK_ID('P','P','U','\0'):

							ppu.LoadState( State::Loader::Subset(loader).Ref() );
							break;

						case NES_STATE_CHUNK_ID('A','P','U','\0'):

							cpu.GetApu().LoadState( State::Loader::Subset(loader).Ref() );
							break;

						case NES_STATE_CHUNK_ID('I','M','G','\0'):

							image->LoadState( State::Loader::Subset(loader).Ref() );
							break;

						case NES_STATE_CHUNK_ID('P','R','T','\0'):

							extPort->Reset();
							expPort->Reset();

							loader.DigIn();

							while (const dword subId = loader.Begin())
							{
								if (subId == NES_STATE_CHUNK_ID('4','S','C','\0'))
								{
									if (extPort->NumPorts() == 4)
									{
										static_cast<Input::AdapterFour*>(extPort)->LoadState
										(
											State::Loader::Subset(loader).Ref()
										);
									}
								}
								else switch (const uint index = (subId >> 16 & 0xFF))
								{
									case '2':
									case '3':

										if (extPort->NumPorts() != 4)
											break;

									case '0':
									case '1':

										extPort->GetDevice( index - '0' )->LoadState
										(
											State::Loader::Subset(loader).Ref(), subId & 0xFF00FFFFUL
										);
										break;

									case 'X':

										expPort->LoadState
										(
											State::Loader::Subset(loader).Ref(), subId & 0xFF00FFFFUL
										);
										break;
								}

								loader.End();
							}

							loader.DigOut();
							break;
					}

					loader.End();
				}

				loader.DigOut();

				return RESULT_OK;
			}
			catch (Result result)
			{
				return result;
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Result Machine::ExecuteFrame
		(
			Video::Output* const video,
			Sound::Output* const sound,
			Input::Controllers* const input
		)
		{
			NST_ASSERT( (state & (Api::Machine::IMAGE|Api::Machine::ON)) > Api::Machine::ON );

			try
			{
				if (!(state & Api::Machine::SOUND))
				{
					if (state & Api::Machine::CARTRIDGE)
						static_cast<Cartridge*>(image)->BeginFrame( Api::Input(*this), input );

					extPort->BeginFrame( input );
					expPort->BeginFrame( input );

					ppu.BeginFrame( video != NULL );

					if (cheats)
						cheats->BeginFrame();

					cpu.BeginFrame( sound );
					cpu.ExecuteFrame();
					ppu.EndFrame();

					if (video)
						renderer.Blit( *video, ppu.GetScreen(), ppu.GetBurstPhase() );

					cpu.EndFrame();

					if (image)
						image->VSync();

					++frame;
				}
				else
				{
					static_cast<Nsf*>(image)->BeginFrame();

					cpu.BeginFrame( sound );
					cpu.ExecuteFrame();
					cpu.EndFrame();

					image->VSync();
				}

				return RESULT_OK;
			}
			catch (...)
			{
				PowerOff();
				return RESULT_ERR_GENERIC;
			}
		}

		NES_POKE(Machine,4016)
		{
			extPort->Poke( data );
			expPort->Poke( data );
		}

		NES_PEEK(Machine,4016)
		{
			return OPEN_BUS | extPort->Peek(0) | expPort->Peek(0);
		}

		NES_POKE(Machine,4017)
		{
			cpu.GetApu().Poke_4017( data );
		}

		NES_PEEK(Machine,4017)
		{
			return OPEN_BUS | extPort->Peek(1) | expPort->Peek(1);
		}
	}
}
