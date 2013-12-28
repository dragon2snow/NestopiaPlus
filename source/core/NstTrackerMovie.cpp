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

#include <new>
#include "NstMachine.hpp"
#include "NstState.hpp"
#include "NstTrackerMovie.hpp"
#include "api/NstApiMovie.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Tracker::Movie::Recorder
		{
		public:

			void Start(bool,dword);
			void BeginFrame(dword,Machine&,EmuSaveState);
			uint WritePort(uint,uint);
			void Reset(bool);
			void EndFrame();
			void Stop();
			void Cut();

		private:

			void Flush();

			typedef Vector<byte> Buffer;

			struct Port
			{
				bool Unlock();
				NST_FORCE_INLINE void Flush(State::Saver&,uint);
				NST_FORCE_INLINE void Sync(uint);

				dword lock;
				Buffer input;
				Buffer recent;
				Buffer output;

				Port()
				: lock(0) {}
			};

			struct Saver : State::Saver
			{
				explicit Saver(StdStream s)
				: State::Saver(s,true,true) {}

				void WriteHeader(bool,dword);

				bool operator != (StdStream s) const
				{
					return stream != s;
				}
			};

			ibool good;
			dword frame;
			ibool cut;
			Port port[2];
			Saver state;

		public:

			explicit Recorder(StdStream stream)
			: good(true), frame(0), cut(false), state(stream) {}

			bool operator != (StdStream stream) const
			{
				return state != stream;
			}
		};

		class Tracker::Movie::Player
		{
		public:

			void Start(dword);
			bool BeginFrame(dword,Machine&,EmuLoadState,EmuReset);
			inline uint ReadPort(uint);
			void EndFrame();

		private:

			typedef Vector<byte> Buffer;

			struct Port
			{
				bool Load(State::Loader&);
				NST_FORCE_INLINE void Sync(uint);

				dword lock;
				dword next;
				dword offset;
				dword pos;
				Buffer output;
			};

			struct Loader : State::Loader
			{
				explicit Loader(StdStream s)
				: State::Loader(s,false) {}

				void ReadHeader(dword);

				bool Eof()
				{
					return stream.Eof();
				}

				bool operator != (StdStream s) const
				{
					return stream != s;
				}
			};

			ibool good;

			struct
			{
				uint port;
				dword clip;
				dword reset;
				dword wait;
			}   frame;

			Port port[2];
			Loader state;

		public:

			explicit Player(StdStream stream)
			: state(stream) {}

			bool operator != (StdStream stream) const
			{
				return state != stream;
			}

			void Stop()
			{
			}
		};

		Tracker::Movie::Movie(Machine& e,EmuReset r,EmuLoadState l,EmuSaveState s,Cpu& c,dword crc)
		:
		cpu            (c),
		status         (STOPPED),
		player         (NULL),
		recorder       (NULL),
		emulator       (e),
		emuSaveState   (s),
		emuLoadState   (l),
		emuReset       (r),
		callbackEnable (false),
		prgCrc         (crc)
		{
		}

		Tracker::Movie::~Movie()
		{
			Close();
		}

		void Tracker::Movie::Close()
		{
			Stop();

			delete player;
			player = NULL;

			delete recorder;
			recorder = NULL;
		}

		bool Tracker::Movie::Record(StdStream const stream,const bool end,const bool callback)
		{
			NST_ASSERT( stream );

			if (status == PLAYING)
				throw RESULT_ERR_NOT_READY;

			callbackEnable = callback;

			if (player)
			{
				Close();
			}
			else if (recorder)
			{
				if (*recorder != stream)
				{
					Close();
				}
				else if (status == RECORDING)
				{
					return false;
				}
			}

			if (recorder == NULL)
				recorder = new Recorder( stream );

			try
			{
				recorder->Start( end, prgCrc );
			}
			catch (...)
			{
				delete recorder;
				recorder = NULL;
				throw;
			}

			status = RECORDING;
			SaveCpuPorts();

			if (callbackEnable)
				Api::Movie::stateCallback( Api::Movie::RECORDING );

			return true;
		}

		bool Tracker::Movie::Play(StdStream const stream,const bool callback)
		{
			NST_ASSERT( stream );

			if (status == RECORDING)
				throw RESULT_ERR_NOT_READY;

			callbackEnable = callback;

			if (recorder)
			{
				Close();
			}
			else if (player)
			{
				if (*player != stream)
				{
					Close();
				}
				else if (status == PLAYING)
				{
					return false;
				}
			}

			if (player == NULL)
				player = new Player( stream );

			try
			{
				player->Start( prgCrc );
			}
			catch (...)
			{
				delete player;
				player = NULL;
				throw;
			}

			status = PLAYING;
			SaveCpuPorts();

			if (callbackEnable)
				Api::Movie::stateCallback( Api::Movie::PLAYING );

			return true;
		}

		bool Tracker::Movie::Stop(Result result)
		{
			if (status != STOPPED)
			{
				cpu.Unlink( 0x4016, this, status == PLAYING ? &Movie::Peek_4016_Play : &Movie::Peek_4016_Record, &Movie::Poke_4016 );
				cpu.Unlink( 0x4017, this, status == PLAYING ? &Movie::Peek_4017_Play : &Movie::Peek_4017_Record, &Movie::Poke_4017 );

				status = STOPPED;

				if (NES_SUCCEEDED(result))
				{
					try
					{
						if (recorder)
						{
							recorder->Stop();
						}
						else if (player)
						{
							player->Stop();
						}
					}
					catch (Result r)
					{
						result = r;
					}
					catch (const std::bad_alloc&)
					{
						result = RESULT_ERR_OUT_OF_MEMORY;
					}
					catch (...)
					{
						result = RESULT_ERR_GENERIC;
					}

					if (NES_SUCCEEDED(result) && callbackEnable)
						Api::Movie::stateCallback( recorder ? Api::Movie::STOPPED_RECORDING : Api::Movie::STOPPED_PLAYING );
				}
			}

			if (NES_SUCCEEDED(result))
				return true;

			delete recorder;
			recorder = NULL;

			delete player;
			player = NULL;

			if (callbackEnable)
			{
				Api::Movie::State state;

				switch (result)
				{
					case RESULT_ERR_OUT_OF_MEMORY:

						state = Api::Movie::ERR_OUT_OF_MEMORY;
						break;

					case RESULT_ERR_INVALID_FILE:
					case RESULT_ERR_CORRUPT_FILE:

						state = Api::Movie::ERR_CORRUPT_FILE;
						break;

					case RESULT_ERR_UNSUPPORTED_GAME:

						state = Api::Movie::ERR_UNSUPPORTED_IMAGE;
						break;

					default:

						state = Api::Movie::ERR_GENERIC;
						break;
				}

				Api::Movie::stateCallback( state );
			}

			return false;
		}

		void Tracker::Movie::Stop()
		{
			Stop( RESULT_OK );
		}

		void Tracker::Movie::Cut()
		{
			if (status == RECORDING)
				recorder->Cut();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		bool Tracker::Movie::BeginFrame(const dword frame)
		{
			try
			{
				if (status == RECORDING)
				{
					recorder->BeginFrame( frame, emulator, emuSaveState );
				}
				else if (status == PLAYING)
				{
					if (!player->BeginFrame( frame, emulator, emuLoadState, emuReset ))
						Stop();
				}

				return true;
			}
			catch (Result result)
			{
				return Stop( result );
			}
			catch (const std::bad_alloc&)
			{
				return Stop( RESULT_ERR_OUT_OF_MEMORY );
			}
			catch (...)
			{
				return Stop( RESULT_ERR_GENERIC );
			}
		}

		bool Tracker::Movie::EndFrame()
		{
			try
			{
				if (status == RECORDING)
				{
					recorder->EndFrame();
				}
				else if (status == PLAYING)
				{
					player->EndFrame();
				}

				return true;
			}
			catch (Result result)
			{
				return Stop( result );
			}
			catch (const std::bad_alloc&)
			{
				return Stop( RESULT_ERR_OUT_OF_MEMORY );
			}
			catch (...)
			{
				return Stop( RESULT_ERR_GENERIC );
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		bool Tracker::Movie::Reset(const bool hard)
		{
			if (status != STOPPED)
			{
				SaveCpuPorts();

				if (status == RECORDING)
				{
					try
					{
						recorder->Reset( hard );
					}
					catch (Result result)
					{
						return Stop( result );
					}
					catch (const std::bad_alloc&)
					{
						return Stop( RESULT_ERR_OUT_OF_MEMORY );
					}
					catch (...)
					{
						return Stop( RESULT_ERR_GENERIC );
					}
				}
			}

			return true;
		}

		void Tracker::Movie::SaveCpuPorts()
		{
			NST_ASSERT( status != STOPPED );

			const bool recording = (status == RECORDING);

			ports[0] = cpu.Link( 0x4016, Cpu::LEVEL_HIGHEST, this, recording ? &Movie::Peek_4016_Record : &Movie::Peek_4016_Play, &Movie::Poke_4016 );
			ports[1] = cpu.Link( 0x4017, Cpu::LEVEL_HIGHEST, this, recording ? &Movie::Peek_4017_Record : &Movie::Peek_4017_Play, &Movie::Poke_4017 );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NST_FORCE_INLINE void Tracker::Movie::Recorder::Port::Flush(State::Saver& state,const uint index)
		{
			Unlock();
			recent.Clear();

			if (output.Size())
			{
				state.Begin( AsciiId<'P','T','0'>::R(0,0,index) ).Write32( output.Size() - 1 ).Compress( output.Begin(), output.Size() ).End();
				output.Clear();
			}
		}

		void Tracker::Movie::Recorder::Flush()
		{
			for (uint i=0; i < 2; ++i)
				port[i].Flush( state, i );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Tracker::Movie::Recorder::Saver::WriteHeader(const bool end,const dword prgCrc)
		{
			const byte header[16] =
			{
				Ascii<'N'>::V,
				Ascii<'S'>::V,
				Ascii<'V'>::V,
				0x1A,
				VERSION,
				prgCrc >>  0 & 0xFF,
				prgCrc >>  8 & 0xFF,
				prgCrc >> 16 & 0xFF,
				prgCrc >> 24 & 0xFF,
				0,
				0,
				0,
				0,
				0,
				0,
				0
			};

			stream.Write( header );

			if (end)
				stream.SeekEnd();
		}

		void Tracker::Movie::Recorder::Start(const bool end,const dword prgCrc)
		{
			cut = false;
			frame = NO_FRAME;
			state.WriteHeader( end, prgCrc );
		}

		void Tracker::Movie::Recorder::Stop()
		{
			NST_VERIFY( bool(frame) >= bool(port[0].output.Size() || port[1].output.Size()) );

			Flush();

			if (frame && frame != NO_FRAME)
				state.Begin( AsciiId<'W','A','I'>::V ).Write32( frame ).End();
		}

		void Tracker::Movie::Recorder::Cut()
		{
			Stop();
			cut = true;
		}

		void Tracker::Movie::Recorder::Reset(const bool hard)
		{
			if (frame)
			{
				Flush();
				state.Begin( AsciiId<'R','E','S'>::V ).Write32( frame != NO_FRAME ? frame : 0 ).Write8( hard ).End();
				frame = 0;
			}
		}

		void Tracker::Movie::Player::Loader::ReadHeader(const dword prgCrc)
		{
			if (stream.Read32() != (AsciiId<'N','S','V'>::V | 0x1AUL << 24))
				throw RESULT_ERR_INVALID_FILE;

			if (stream.Read8() != VERSION)
				throw RESULT_ERR_UNSUPPORTED_FILE_VERSION;

			const dword crc = stream.Read32();

			if
			(
				crc && prgCrc && crc != prgCrc &&
				Api::User::questionCallback( Api::User::QUESTION_NSV_PRG_CRC_FAIL_CONTINUE ) == Api::User::ANSWER_NO
			)
				throw RESULT_ERR_INVALID_CRC;

			stream.Seek( 7 );
		}

		void Tracker::Movie::Player::Start(const dword prgCrc)
		{
			good = false;

			frame.port = 0;
			frame.clip = NO_FRAME;
			frame.reset = NO_FRAME;
			frame.wait = NO_FRAME;

			for (uint i=0; i < 2; ++i)
			{
				port[i].pos = 0;
				port[i].offset = 0;
				port[i].lock = 0;
				port[i].next = 0;
				port[i].output.Clear();
			}

			state.ReadHeader( prgCrc );

			good = true;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		bool Tracker::Movie::Player::Port::Load(State::Loader& state)
		{
			if (pos != output.Size())
				return false;

			pos = 0;
			offset = 0;

			const dword size = state.Read32() + 1;

			if (size <= 0x02000000UL)
			{
				output.Resize( size );
				state.Uncompress( output.Begin(), size );
				return true;
			}
			else
			{
				throw RESULT_ERR_CORRUPT_FILE;
			}
		}

		bool Tracker::Movie::Recorder::Port::Unlock()
		{
			if (dword count = lock)
			{
				lock = 0;

				output.Expand( --count < LOCK_SIZE_BIT ? 1 : 4 );

				if (count < LOCK_SIZE_BIT)
				{
					output.Back() = (LOCK_BIT|LOCK_SIZE_BIT) | count;
				}
				else
				{
					byte* const ptr = output.End() - 4;

					ptr[0] = count >> 24 & 0xFF | LOCK_BIT;
					ptr[1] = count >> 16 & 0xFF;
					ptr[2] = count >>  8 & 0xFF;
					ptr[3] = count >>  0 & 0xFF;
				}

				return true;
			}

			return false;
		}

		NST_FORCE_INLINE void Tracker::Movie::Recorder::Port::Sync(const uint index)
		{
			NST_ASSERT( index <= 1 );

			if (input.Size())
			{
				if (input.Size() <= MAX_FRAME_READS)
				{
					if (recent == input)
					{
						++lock;
					}
					else
					{
						if (!Unlock() && index && recent.Size())
							output.Append( 0x00 ); // $4016 can afford one bit while $4017 can't

						recent = input;
						output += input;
					}
				}
				else
				{
					throw RESULT_ERR_UNSUPPORTED_GAME;
				}
			}
		}

		NST_FORCE_INLINE void Tracker::Movie::Player::Port::Sync(uint index)
		{
			NST_ASSERT( index <= 1 );

			if (pos > offset)
			{
				if (lock)
				{
					if (--lock)
					{
						pos = offset;
					}
					else
					{
						pos = offset = next;
					}
				}
				else if (pos < output.Size())
				{
					const uint ctrl = output[pos];

					if (ctrl & LOCK_BIT)
					{
						next = pos;
						pos = offset;

						if (ctrl & LOCK_SIZE_BIT)
						{
							next += 1;
							lock = (ctrl & ~uint(LOCK_BIT|LOCK_SIZE_BIT)) + 1;
						}
						else
						{
							next += 4;

							if (next <= output.Size())
							{
								lock = 1 +
								(
									dword(ctrl & ~uint(LOCK_BIT)) << 24 |
									dword(output[next-3])         << 16 |
									dword(output[next-2])         <<  8 |
									dword(output[next-1])         <<  0
								);
							}
							else
							{
								throw RESULT_ERR_CORRUPT_FILE;
							}
						}
					}
					else
					{
						pos += index; // $4016 can afford one bit while $4017 can't
						offset = pos;
					}
				}
			}
		}

		void Tracker::Movie::Recorder::BeginFrame(const dword emuFrame,Machine& emulator,EmuSaveState emuSaveState)
		{
			NST_VERIFY( frame <= emuFrame || frame == NO_FRAME );

			for (uint i=0; i < 2; ++i)
				port[i].input.Clear();

			if (frame == emuFrame && !cut)
				return;

			cut = false;

			Flush();

			state.Begin( AsciiId<'C','L','P'>::V );
			state.Write32( frame != NO_FRAME ? frame : 0 );
			(emulator.*emuSaveState)( state );
			state.End();

			frame = emuFrame;
		}

		bool Tracker::Movie::Player::BeginFrame
		(
			const dword emuFrame,
			Machine& emulator,
			EmuLoadState emuLoadState,
			EmuReset emuReset
		)
		{
			for (;;)
			{
				if (frame.port)
				{
					NST_ASSERT( frame.port <= 2 );

					if (port[frame.port - 1].Load( state ))
					{
						frame.port = 0;
						state.End();
					}
					else
					{
						break;
					}
				}
				else if (frame.clip != NO_FRAME)
				{
					if (frame.clip <= emuFrame)
					{
						frame.clip = NO_FRAME;
						(emulator.*emuLoadState)( state );
						state.End();
					}
					else
					{
						break;
					}
				}
				else if (frame.reset != NO_FRAME)
				{
					if (frame.reset <= emuFrame)
					{
						frame.reset = NO_FRAME;
						(emulator.*emuReset)( state.Read8() & 0x1 );
						state.End();
					}
					else
					{
						break;
					}
				}
				else if (frame.wait != NO_FRAME)
				{
					if (frame.wait <= emuFrame)
					{
						frame.wait = NO_FRAME;
						state.End();
					}
					else
					{
						break;
					}
				}

				if (!state.Eof())
				{
					switch (state.Begin())
					{
						case AsciiId<'P','T','0'>::V:

							frame.port = 1;
							break;

						case AsciiId<'P','T','1'>::V:

							frame.port = 2;
							break;

						case AsciiId<'C','L','P'>::V:

							frame.clip = state.Read32() & VALID_FRAME;
							break;

						case AsciiId<'R','E','S'>::V:

							frame.reset = state.Read32() & VALID_FRAME;
							break;

						case AsciiId<'W','A','I'>::V:

							frame.wait = state.Read32() & VALID_FRAME;
							break;

						default:

							return false;
					}
				}
				else
				{
					return port[0].pos < port[0].output.Size() || port[1].pos < port[1].output.Size();
				}
			}

			return true;
		}

		void Tracker::Movie::Recorder::EndFrame()
		{
			if (good)
			{
				++frame;

				for (uint i=0; i < 2; ++i)
					port[i].Sync( i );
			}
			else
			{
				throw RESULT_ERR_OUT_OF_MEMORY;
			}
		}

		void Tracker::Movie::Player::EndFrame()
		{
			if (good)
			{
				for (uint i=0; i < 2; ++i)
					port[i].Sync( i );
			}
			else
			{
				throw RESULT_ERR_CORRUPT_FILE;
			}
		}

		uint Tracker::Movie::Recorder::WritePort(const uint index,const uint data)
		{
			NST_ASSERT( index || !(data & LOCK_BIT) );

			if (good)
			{
				try
				{
					port[index].input.Append( data );
				}
				catch (...)
				{
					good = false;
				}
			}

			return data;
		}

		inline uint Tracker::Movie::Player::ReadPort(const uint index)
		{
			if (good)
			{
				if (port[index].pos < port[index].output.Size())
					return port[index].output[port[index].pos++];

				good = false;
			}

			return OPEN_BUS;
		}

		NES_PEEK(Tracker::Movie,4016_Record)
		{
			return recorder->WritePort( 0, ports[0]->Peek( 0x4016 ) );
		}

		NES_PEEK(Tracker::Movie,4016_Play)
		{
			return player->ReadPort( 0 );
		}

		NES_PEEK(Tracker::Movie,4017_Record)
		{
			return recorder->WritePort( 1, ports[1]->Peek( 0x4017 ) );
		}

		NES_PEEK(Tracker::Movie,4017_Play)
		{
			return player->ReadPort( 1 );
		}

		NES_POKE(Tracker::Movie,4016)
		{
			ports[0]->Poke( address, data );
		}

		NES_POKE(Tracker::Movie,4017)
		{
			ports[1]->Poke( address, data );
		}
	}
}
