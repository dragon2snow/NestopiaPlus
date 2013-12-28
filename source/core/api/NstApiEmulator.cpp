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

#include <new>
#include "../NstCore.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiMachine.hpp"
#include "NstApiVideo.hpp"
#include "NstApiMovie.hpp"
#include "NstApiUser.hpp"
#include "../NstVideoPalette.hpp"
#include "../NstVideoRenderer.hpp"
#include "../NstImage.hpp"
#include "../input/NstInpDevice.hpp"
#include "../input/NstInpPad.hpp"
#include "../NstCartridge.hpp"
#include "../NstCheats.hpp"
#include "../NstNsf.hpp"
#include "../NstIoAdapter.hpp"
#include "../NstMovie.hpp"
#include "../NstRewinder.hpp"
#include "../NstImageDatabase.hpp"
  
namespace Nes
{
	namespace Api
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		Emulator::Emulator()
		: 
		state         (Machine::NTSC),
		frame         (0),
		extPort       (new Core::Input::AdapterTwo( new Core::Input::Pad(0), new Core::Input::Pad(1) )),
		expPort       (new Core::Input::Device),
		image         (NULL),
		movie         (NULL),
		renderer      (*new Core::Video::Renderer),
		palette       (*new Core::Video::Palette),
		ppu           (cpu),
		rewinder      (NULL),
		rewinderSound (true),
		cheats        (NULL),
		imageDatabase (NULL)
		{
			ppu.SetScreen( renderer.GetScreen() );
			renderer.SetPalette( palette.GetColors() );
		}
	
		Emulator::~Emulator()
		{
			Unload();

			delete rewinder;
			delete imageDatabase;
			delete cheats;
			delete &renderer;
			delete &palette;
			delete expPort;
	
			for (uint ports=extPort->NumPorts(), i=0; i < ports; ++i)
				delete extPort->GetDevice(i);
	
			delete extPort;
		}

		bool Emulator::GoodSaveTime() const
		{
			return image && (!movie || rewinder);
		}

		Result Emulator::Load(Core::StdStream stream,uint type)
		{
			NST_ASSERT( image == NULL );

			Core::Image::Context context
			(
				image,
				(Core::Image::Type) type,
				cpu,
				ppu,
				stream,
				imageDatabase
			);

			return Core::Image::Load( context );
		}
	
		void Emulator::Unload()
		{
			Core::Movie::Destroy( movie );

			frame = 0;

			if (image)
			{
				image->Flush();
				Core::Image::Unload( image );
			}
		}

		Result Emulator::PowerOn()
		{
			return Reset( true );	
		}
	
		void Emulator::PowerOff()
		{
			frame = 0;

			if (rewinder)
				rewinder->Reset();

			ppu.ClearScreen();
			cpu.GetApu().ClearBuffers();

			if (GoodSaveTime())
				image->Flush();
		}
	
		Result Emulator::Reset(const bool hard)
		{
			frame = 0;

			try
			{
				cpu.Boot();
	
				if (!(state & Machine::SOUND))
				{
					if (rewinder && rewinder->InBadState())
						rewinder->Reset();

					InitializeInputDevices();
	
					cpu.Map( 0x4016U ).Set( this, &Emulator::Peek_4016, &Emulator::Poke_4016 );
					cpu.Map( 0x4017U ).Set( this, &Emulator::Peek_4017, &Emulator::Poke_4017 );
	
					extPort->Reset();
					expPort->Reset();
	
					ppu.Reset( hard );
					
					if (image)
						image->Reset( hard );
					
					cpu.Reset( hard );

					if (cheats)
						cheats->Reset();
	
					if (movie && !movie->MachineReset( hard ))
						Core::Movie::Destroy( movie );

					return GoodSaveTime() ? image->Flush() : RESULT_OK;
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
	
		void Emulator::SetMode(const Core::Mode mode)
		{
			if (rewinder)
				rewinder->Reset();

			cpu.SetMode( mode );
			ppu.SetMode( mode );
	
			if (image)
				image->SetMode( mode );
		}
	
		void Emulator::InitializeInputDevices() const
		{
			if (image && image->GetType() == Core::Image::CARTRIDGE)
			{
				const dword crc = static_cast<const Core::Cartridge*>(image)->GetInfo().pRomCrc;
	
				extPort->Initialize( crc );
				expPort->Initialize( crc );
			}
		}
	
		Result Emulator::SaveState(Core::StdStream stream,bool compress)
		{
			if ((state & (Machine::GAME|Machine::ON)) > Machine::ON)
			{
				try
				{
					Core::State::Saver saver( stream, compress );
	
					saver.Begin('N','S','T',0x1A);
					{
						saver.Begin('N','F','O','\0').Write32( image->GetPrgCrc() ).Write32( frame ).End();
						
						cpu.SaveState( Core::State::Saver::Subset(saver,'C','P','U','\0').Ref() );
						ppu.SaveState( Core::State::Saver::Subset(saver,'P','P','U','\0').Ref() );
						cpu.GetApu().SaveState( Core::State::Saver::Subset(saver,'A','P','U','\0').Ref() );
						image->SaveState( Core::State::Saver::Subset(saver,'I','M','G','\0').Ref() );

						saver.Begin('P','R','T','\0');
						{
							if (extPort->NumPorts() == 4)
							{
								static_cast<const Core::Input::AdapterFour*>(extPort)->SaveState
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
	
		Result Emulator::LoadState(Core::StdStream stream,bool checkCrc)
		{
			if ((state & (Machine::GAME|Machine::ON)) <= Machine::ON)
				return RESULT_ERR_NOT_READY;

			try
			{
				Core::State::Loader loader( stream );
	
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
    							checkCrc && !(state & Machine::DISK) &&
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
							
							cpu.LoadState( Core::State::Loader::Subset(loader).Ref() ); 
							break;
	
						case NES_STATE_CHUNK_ID('P','P','U','\0'): 
							
							ppu.LoadState( Core::State::Loader::Subset(loader).Ref() ); 
							break;
	
						case NES_STATE_CHUNK_ID('A','P','U','\0'): 
							
							cpu.GetApu().LoadState( Core::State::Loader::Subset(loader).Ref() ); 
							break;
	
						case NES_STATE_CHUNK_ID('I','M','G','\0'): 
							
							image->LoadState( Core::State::Loader::Subset(loader).Ref() ); 
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
										static_cast<Core::Input::AdapterFour*>(extPort)->LoadState
										( 
									       	Core::State::Loader::Subset(loader).Ref() 
										);
									}
								}
								else switch (const uint index = ((subId >> 16) & 0xFF))
								{
									case '2':
									case '3':

										if (extPort->NumPorts() != 4)
											break;

									case '0':
									case '1':

										extPort->GetDevice( index - '0' )->LoadState
										( 
									     	Core::State::Loader::Subset(loader).Ref(), subId & 0xFF00FFFFUL 
										);
										break;

									case 'X':

										expPort->LoadState
										( 
									    	Core::State::Loader::Subset(loader).Ref(), subId & 0xFF00FFFFUL 
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

		Result Emulator::Execute
		(
        	Core::Video::Output* const video,
			Core::Sound::Output* const sound,
			Core::Input::Controllers* const input
		)		
		{
			if (rewinder == NULL || (state & Machine::SOUND))
				return ExecuteFrame( video, sound, input );
			else
				return rewinder->Execute( video, sound, input );
		}

		Result Emulator::ExecuteFrame
		(
        	Core::Video::Output* const video,
			Core::Sound::Output* const sound,
			Core::Input::Controllers* const input
		)
		{
			if (state & Machine::ON)
			{
				try
				{
					if (!(state & Machine::SOUND))
					{
						if (state & Machine::CARTRIDGE)
							static_cast<Core::Cartridge*>(image)->BeginFrame( input );

						if (movie && !movie->BeginFrame( frame, *this, &Emulator::SaveState, &Emulator::LoadState, &Emulator::Reset ))
							Core::Movie::Destroy( movie );
	
						extPort->BeginFrame( input );
						expPort->BeginFrame( input );
	
						ppu.BeginFrame( video != NULL || ppu.GetScreen() != renderer.GetScreen() );

						if (cheats)
							cheats->BeginFrame();

						cpu.BeginFrame( sound );
						cpu.ExecuteFrame();
						ppu.EndFrame();

						if (video)
						{
							if (palette.NeedUpdate())
							{
								Api::Video::Palette::Colors const colors = palette.GetColors();
								renderer.SetPalette( colors );
								Api::Video::Palette::updateCallback( colors );
							}

							renderer.Blit( *video );
						}

						cpu.EndFrame();

						if (image)
							image->VSync();

						if (movie && !movie->EndFrame())
							Core::Movie::Destroy( movie );

						++frame;
					}
					else
					{
						static_cast<Core::Nsf*>(image)->BeginFrame( NULL );
						
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
	
			return RESULT_ERR_NOT_READY;
		}
	
		NES_POKE(Emulator,4016)
		{
			extPort->Poke( data );
			expPort->Poke( data );
		}
	
		NES_PEEK(Emulator,4016)
		{
			return OPEN_BUS | extPort->Peek(0) | expPort->Peek(0);
		}
	
		NES_POKE(Emulator,4017)
		{
			cpu.GetApu().Poke_4017( data );
		}
	
		NES_PEEK(Emulator,4017)
		{
			return OPEN_BUS | extPort->Peek(1) | expPort->Peek(1);
		}
	}
}
