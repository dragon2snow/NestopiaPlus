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

#include <vector>
#include "NstState.hpp"
#include "NstCpu.hpp"
#include "NstHook.hpp"
#include "NstPrpDataRecorder.hpp"
#include "api/NstApiUser.hpp"
   
namespace Nes
{
	namespace Core
	{
		namespace Peripherals
		{
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("s", on)
            #endif
		
			NST_COMPILE_ASSERT( MODE_NTSC == 0 && MODE_PAL == 1 );
		
			const Cycle DataRecorder::clocks[2][2] =
			{
				{ Cpu::CLK_NTSC_DIV * 16U, Cpu::MC_NTSC * 16U / dword(CLOCK) },
				{ Cpu::CLK_PAL_DIV * 320U, qword(Cpu::MC_PAL) * 320U / dword(CLOCK) }
			};
		
			DataRecorder::DataRecorder(Cpu& c)
			: cpu(c), cycles(NES_CYCLE_MAX), status(STOPPED), loaded(false) {}
		
			void DataRecorder::Load()
			{
				NST_ASSERT( !loaded );
		
				loaded = true;
		
				std::vector<u8> data;
				Api::User::fileIoCallback( Api::User::FILE_LOAD_TAPE, data );
		
				if (ulong size = data.size())
				{
					if (size > SIZE_4096K)
						size = SIZE_4096K;
		
					stream.Resize( size );
					std::memcpy( stream.Begin(), &data.front(), size );
					checksum = Checksum::Md5::Compute( stream.Begin(), size );
				}
			}
		
			DataRecorder::~DataRecorder()
			{
				Stop();
		
				if (stream.Size() && checksum != Checksum::Md5::Compute( stream.Begin(), stream.Size() ))
				{
					try
					{
						std::vector<u8> data( stream.Begin(), stream.End() );
						Api::User::fileIoCallback( Api::User::FILE_SAVE_TAPE, data );
					}
					catch (...)
					{
					}
				}
			}
		
			void DataRecorder::SaveState(State::Saver& state) const
			{
				if (stream.Size())
				{
					if (status != STOPPED)
					{
						const dword p = (status == PLAYING ? pos : 0);
						Cycle c = cycles / clocks[cpu.GetMode()][0];
		
						if (c > cpu.GetMasterClockCycles())
							c -= cpu.GetMasterClockCycles();
						else
							c = 0;
		
						c /= cpu.GetMasterClockCycle(1);
		
						const u8 data[] =
						{
							status,
							in,
							out,
							(p >>  0) & 0xFF,
							(p >>  8) & 0xFF,
							(p >> 16) & 0xFF,
							(p >> 24) & 0xFF,
							(c >>  0) & 0xFF,
							(c >>  8) & 0xFF,
							(c >> 16) & 0xFF,
							(c >> 24) & 0xFF
						};
		
						state.Begin('R','E','G','\0').Write( data ).End();
					}
		
					state.Begin('D','A','T','\0').Write32( stream.Size() ).Compress( stream.Begin(), stream.Size() ).End();
				}
			}
		
			void DataRecorder::LoadState(State::Loader& state)
			{
				Stop();						
				pos = 0;
				stream.Destroy();
		
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case NES_STATE_CHUNK_ID('R','E','G','\0'):
						{
							loaded = true;
							const State::Loader::Data<11> data( state );
					
							status = (data[0] == 1 ? PLAYING : data[0] == 2 ? RECORDING : STOPPED);
							in = data[1] & 0x2;
							out = data[2];
		
							if (status == PLAYING)
								pos = data[3] | (data[4] << 8) | (data[5] << 16) | (data[6] << 24);
		
							if (status != STOPPED)
							{
								cycles  = data[7] | (data[8] << 8) | (data[9] << 16) | (data[10] << 24);
								cycles *= cpu.GetMasterClockCycle(1) * clocks[cpu.GetMode()][0];
								cycles += cpu.GetMasterClockCycles() * clocks[cpu.GetMode()][0];
							}
		
							break;
						}
					
						case NES_STATE_CHUNK_ID('D','A','T','\0'):
		
							loaded = true;
		
							if (const dword size = state.Read32())
							{
								if (size <= SIZE_4096K)
								{
									stream.Resize( size );
									state.Uncompress( stream.Begin(), size );
								}
							}
							break;				
					}
		
					state.End();
				}
		
				if (status != STOPPED)
				{
					if (stream.Size() && pos < stream.Size() && cycles <= clocks[cpu.GetMode()][1] * 2)
						Prepare();
					else
						Stop();
				}
			}
		
			Result DataRecorder::Record()
			{
				if (status == RECORDING)
					return RESULT_NOP;
		
				if (status == PLAYING)
					return RESULT_ERR_NOT_READY;
		
				loaded = true;
				status = RECORDING;
				out = 0;
				cycles = 0;			
				stream.Destroy();			
				
				Prepare();
				
				Api::User::eventCallback( Api::User::EVENT_TAPE_RECORDING, NULL );
		
				return RESULT_OK;
			}
		
			Result DataRecorder::Play()
			{
				if (status == PLAYING)
					return RESULT_NOP;
				
				if (status == RECORDING)
					return RESULT_ERR_NOT_READY;
		
				if (!loaded)
					Load();
		
				if (!stream.Size())
					return RESULT_ERR_NOT_READY;
		
				status = PLAYING;
				pos = 0;
				in = 0;
				cycles = 0;
		
				Prepare();
				
				Api::User::eventCallback( Api::User::EVENT_TAPE_PLAYING, NULL );
		
				return RESULT_OK;
			}
		
			void DataRecorder::Prepare()
			{
				p4016 = cpu.Link( 0x4016, Cpu::LEVEL_LOW, this, &DataRecorder::Peek_4016, &DataRecorder::Poke_4016 );
				cpu.AddHook( Hook(this,&DataRecorder::Hook_Tape) );
			}
		
			void DataRecorder::Stop()
			{
				if (status != STOPPED)
				{
					status = STOPPED;
					cycles = NES_CYCLE_MAX - 1;
					cpu.Unlink( 0x4016, this, &DataRecorder::Peek_4016, &DataRecorder::Poke_4016 );
					Api::User::eventCallback( Api::User::EVENT_TAPE_STOPPED, NULL );
				}
			}
		
			void DataRecorder::VSync()
			{
				if (cycles != NES_CYCLE_MAX)
				{
					if (cycles != NES_CYCLE_MAX-1)
					{
						const Cycle frame = cpu.GetMasterClockFrameCycles() * clocks[cpu.GetMode()][0];
		
						if (cycles > frame)
							cycles -= frame;
						else
							cycles = 0;
					}
					else
					{
						cycles = NES_CYCLE_MAX;
						cpu.RemoveHook( Hook(this,&DataRecorder::Hook_Tape) );
					}
				}
			}
		
            #ifdef NST_PRAGMA_OPTIMIZE
            #pragma optimize("", on)
            #endif
		
			NES_HOOK(DataRecorder,Tape)
			{
				for (const Cycle next = cpu.GetMasterClockCycles() * clocks[cpu.GetMode()][0]; cycles < next; cycles += clocks[cpu.GetMode()][1])
				{
					if (status == PLAYING)
					{
						if (pos < stream.Size())
						{
							const uint data = stream[pos++];
		
							if (data >= 0x8C)
							{
								in = 0x2;
							}
							else if (data <= 0x74)
							{
								in = 0x0;
							}
						}
						else
						{
							Stop();
							break;
						}
					}
					else
					{
						NST_ASSERT( status == RECORDING );
		
						if (stream.Size() < SIZE_4096K)
						{
							stream << ((out & 0x7) == 0x7 ? 0x90 : 0x70);
						}
						else
						{
							Stop();
							break;
						}
					}
				}
			}
		
			NES_POKE(DataRecorder,4016)
			{
				out = data;
				p4016->Poke( address, data );
			}
		
			NES_PEEK(DataRecorder,4016)
			{
				return p4016->Peek( address ) | in;
			}
		}
	}
}
