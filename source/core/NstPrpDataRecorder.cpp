////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include "NstCpu.hpp"
#include "NstHook.hpp"
#include "NstState.hpp"
#include "NstFile.hpp"
#include "NstPrpDataRecorder.hpp"
#include "api/NstApiTapeRecorder.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Peripherals
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			const dword DataRecorder::clocks[2][2] =
			{
				{ Clocks::NTSC_DIV * 16, Clocks::NTSC_CLK * (16UL/1) / (CLOCK/1)   },
				{ Clocks::PAL_DIV * 320, Clocks::PAL_CLK * (320UL/16) / (CLOCK/16) }
			};

			DataRecorder::DataRecorder(Cpu& c)
			: cpu(c), cycles(Cpu::CYCLE_MAX), status(STOPPED), loaded(false)
			{
				NST_COMPILE_ASSERT( Region::NTSC == 0 && Region::PAL == 1 );
			}

			DataRecorder::~DataRecorder()
			{
				Stop();
			}

			void DataRecorder::Reset()
			{
				Stop();
				cycles = Cpu::CYCLE_MAX;
			}

			void DataRecorder::PowerOff()
			{
				Reset();

				if (stream.Size())
					file.Save( File::SAVE_TAPE, stream.Begin(), stream.Size() );
			}

			void DataRecorder::SaveState(State::Saver& state,const dword baseChunk) const
			{
				if (stream.Size())
				{
					state.Begin( baseChunk );

					if (status != STOPPED)
					{
						const dword p = (status == PLAYING ? pos : 0);
						Cycle c = cycles / clocks[cpu.GetRegion()][0];

						if (c > cpu.GetCycles())
							c -= cpu.GetCycles();
						else
							c = 0;

						c /= cpu.GetClock();

						const byte data[] =
						{
							status,
							in,
							out,
							p >>  0 & 0xFF,
							p >>  8 & 0xFF,
							p >> 16 & 0xFF,
							p >> 24 & 0xFF,
							c >>  0 & 0xFF,
							c >>  8 & 0xFF,
							c >> 16 & 0xFF,
							c >> 24 & 0xFF
						};

						state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					}

					state.Begin( AsciiId<'D','A','T'>::V ).Write32( stream.Size() ).Compress( stream.Begin(), stream.Size() ).End();

					state.End();
				}
			}

			void DataRecorder::LoadState(State::Loader& state)
			{
				Stop();

				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'R','E','G'>::V:
						{
							loaded = true;
							State::Loader::Data<11> data( state );

							status = (data[0] == 1 ? PLAYING : data[0] == 2 ? RECORDING : STOPPED);
							in = data[1] & 0x2;
							out = data[2];

							if (status == PLAYING)
								pos = data[3] | data[4] << 8 | dword(data[5]) << 16 | dword(data[6]) << 24;

							if (status != STOPPED)
							{
								cycles  = data[7] | data[8] << 8 | dword(data[9]) << 16 | dword(data[10]) << 24;
								cycles *= cpu.GetClock() * clocks[cpu.GetRegion()][0];
								cycles += cpu.GetCycles() * clocks[cpu.GetRegion()][0];
							}

							break;
						}

						case AsciiId<'D','A','T'>::V:
						{
							loaded = true;
							const dword size = state.Read32();

							NST_VERIFY( size > 0 && size <= MAX_LENGTH );

							if (size > 0 && size <= MAX_LENGTH)
							{
								stream.Resize( size );
								state.Uncompress( stream.Begin(), size );
							}

							break;
						}
					}

					state.End();
				}

				if (status != STOPPED)
				{
					if (stream.Size() && pos < stream.Size() && cycles <= clocks[cpu.GetRegion()][1] * 2)
					{
						Start();
					}
					else
					{
						status = STOPPED;
						cycles = Cpu::CYCLE_MAX-1UL;
					}
				}
			}

			bool DataRecorder::Playable()
			{
				if (!loaded)
				{
					loaded = true;

					try
					{
						file.Load( File::LOAD_TAPE, stream, MAX_LENGTH );
					}
					catch (...)
					{
						stream.Destroy();
					}
				}

				return stream.Size();
			}

			Result DataRecorder::Record()
			{
				if (status == RECORDING)
					return RESULT_NOP;

				if (status == PLAYING)
					return RESULT_ERR_NOT_READY;

				loaded = true;
				status = RECORDING;
				in = 0;
				out = 0;
				cycles = 0;
				stream.Destroy();

				Start();

				return RESULT_OK;
			}

			Result DataRecorder::Play()
			{
				if (status == PLAYING)
					return RESULT_NOP;

				if (status == RECORDING || !Playable())
					return RESULT_ERR_NOT_READY;

				status = PLAYING;
				pos = 0;
				in = 0;
				out = 0;
				cycles = 0;

				Start();

				return RESULT_OK;
			}

			void DataRecorder::Start()
			{
				p4016 = cpu.Link( 0x4016, Cpu::LEVEL_LOW, this, &DataRecorder::Peek_4016, &DataRecorder::Poke_4016 );
				cpu.AddHook( Hook(this,&DataRecorder::Hook_Tape) );
				Api::TapeRecorder::eventCallback( status == PLAYING ? Api::TapeRecorder::EVENT_PLAYING : Api::TapeRecorder::EVENT_RECORDING );
			}

			Result DataRecorder::Stop()
			{
				if (status == STOPPED)
					return RESULT_NOP;

				status = STOPPED;
				cycles = Cpu::CYCLE_MAX-1UL;
				cpu.Unlink( 0x4016, this, &DataRecorder::Peek_4016, &DataRecorder::Poke_4016 );
				Api::TapeRecorder::eventCallback( Api::TapeRecorder::EVENT_STOPPED );

				return RESULT_OK;
			}

			void DataRecorder::VSync()
			{
				if (cycles == Cpu::CYCLE_MAX)
					return;

				if (cycles != Cpu::CYCLE_MAX-1UL)
				{
					const Cycle frame = cpu.GetFrameCycles() * clocks[cpu.GetRegion()][0];

					if (cycles > frame)
						cycles -= frame;
					else
						cycles = 0;
				}
				else
				{
					cycles = Cpu::CYCLE_MAX;
					cpu.RemoveHook( Hook(this,&DataRecorder::Hook_Tape) );
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_HOOK(DataRecorder,Tape)
			{
				for (const Cycle next = cpu.GetCycles() * clocks[cpu.GetRegion()][0]; cycles < next; cycles += clocks[cpu.GetRegion()][1])
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

						if (stream.Size() < MAX_LENGTH)
						{
							stream.Append( (out & 0x7) == 0x7 ? 0x90 : 0x70 );
						}
						else
						{
							Stop();
							break;
						}
					}
				}
			}

			NES_POKE_AD(DataRecorder,4016)
			{
				out = data;
				p4016->Poke( address, data );
			}

			NES_PEEK_A(DataRecorder,4016)
			{
				return p4016->Peek( address ) | in;
			}
		}
	}
}
