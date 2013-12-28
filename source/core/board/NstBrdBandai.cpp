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

#include <cstring>
#include "../NstMapper.hpp"
#include "../NstClock.hpp"
#include "../NstPrpBarcodeReader.hpp"
#include "../api/NstApiUser.hpp"
#include "NstBrdBandai.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			template<uint N>
			class Bandai::E24C0X
			{
			public:

				E24C0X();

				enum
				{
					SIZE = N
				};

				void Reset();
				void Set(uint,uint);
				void Load(const u8*);
				bool Save(u8*) const;
				void LoadState(State::Loader&);
				void SaveState(State::Saver&) const;

			private:

				void Start();
				void Stop();
				void Rise(uint);
				void Fall();

				enum Mode
				{
					MODE_IDLE,
					MODE_DATA,
					MODE_ADDRESS,
					MODE_READ,
					MODE_WRITE,
					MODE_ACK,
					MODE_NOT_ACK,
					MODE_ACK_WAIT,
					MODE_MAX
				};

				struct Line
				{
					uint scl;
					uint sda;
				};

				struct Latch
				{
					uint bit;
					uint address;
					uint data;
				};

				Line line;
				Mode mode;
				Mode next;
				Latch latch;
				ibool rw;
				uint output;
				u8 mem[SIZE];
				u8 backup[SIZE];

			public:

				void SetScl(uint scl)
				{
					Set( scl, line.sda );
				}

				void SetSda(uint sda)
				{
					Set( line.scl, sda );
				}

				uint Read() const
				{
					return output;
				}
			};

			template<uint N>
			Bandai::E24C0X<N>::E24C0X()
			{
				std::memset( mem, 0, SIZE );
				std::memset( backup, 0, SIZE );
			}

			template<uint N>
			void Bandai::E24C0X<N>::Reset()
			{
				line.scl      = 0;
				line.sda      = 0;
				mode          = MODE_IDLE;
				next          = MODE_IDLE;
				latch.bit     = 0;
				latch.address = 0;
				latch.data    = 0;
				rw            = false;
				output        = 0x10;
			}

			template<uint N>
			void Bandai::E24C0X<N>::Load(const u8* file)
			{
				std::memcpy( mem, file, SIZE );
				std::memcpy( backup, mem, SIZE );
			}

			template<uint N>
			bool Bandai::E24C0X<N>::Save(u8* file) const
			{
				std::memcpy( file, mem, SIZE );
				return std::memcmp( mem, backup, SIZE ) != 0;
			}

			template<uint N>
			void Bandai::E24C0X<N>::SaveState(State::Saver& state) const
			{
				const u8 data[6] =
				{
					line.scl | line.sda,
					(mode << 0) | (next << 4),
					latch.address,
					latch.data,
					latch.bit,
					output | (rw ? 0x80 : 0x00)
				};

				state.Begin('R','E','G','\0').Write( data ).End();
				state.Begin('R','A','M','\0').Compress( mem ).End();
			}

			template<uint N>
			void Bandai::E24C0X<N>::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case NES_STATE_CHUNK_ID('R','E','G','\0'):
						{
							const State::Loader::Data<6> data( state );

							line.scl = data[0] & 0x20;
							line.sda = data[0] & 0x40;

							if ((data[1] & 0xF) < MODE_MAX)
								mode = (Mode) (data[1] & 0xF);

							if ((data[1] >> 4) < MODE_MAX)
								next = (Mode) (data[1] >> 4);

							latch.address = data[2] & (SIZE-1);
							latch.data = data[3];
							latch.bit = NST_MAX(data[4],8);

							rw = data[5] & 0x80;
							output = data[5] & 0x10;
							break;
						}

						case NES_STATE_CHUNK_ID('R','A','M','\0'):

							state.Uncompress( mem );
							break;
					}

					state.End();
				}
			}

			template<>
			void Bandai::E24C0X<128>::Start()
			{
				mode = MODE_ADDRESS;
				latch.bit = 0;
				latch.address = 0;
				output = 0x10;
			}

			template<>
			void Bandai::E24C0X<256>::Start()
			{
				mode = MODE_DATA;
				latch.bit = 0;
				output = 0x10;
			}

			template<uint N>
			void Bandai::E24C0X<N>::Stop()
			{
				mode = MODE_IDLE;
				output = 0x10;
			}

			template<>
			void Bandai::E24C0X<128>::Rise(const uint bit)
			{
				NST_ASSERT( bit <= 1 );

				switch (mode)
				{
					case MODE_ADDRESS:

						if (latch.bit < 7)
						{
							latch.address &= ~(1U << latch.bit);
							latch.address |= bit << latch.bit++;
						}
						else if (latch.bit < 8)
						{
							latch.bit = 8;

							if (bit)
							{
								next = MODE_READ;
								latch.data = mem[latch.address];
							}
							else
							{
								next = MODE_WRITE;
							}
						}
						break;

					case MODE_ACK:

						output = 0x00;
						break;

					case MODE_READ:

						if (latch.bit < 8)
							output = (latch.data & (1U << latch.bit++)) ? 0x10 : 0x00;

						break;

					case MODE_WRITE:

						if (latch.bit < 8)
						{
							latch.data &= ~(1U << latch.bit);
							latch.data |= bit << latch.bit++;
						}
						break;

					case MODE_ACK_WAIT:

						if (bit == 0)
							next = MODE_IDLE;

						break;
				}
			}

			template<>
			void Bandai::E24C0X<128>::Fall()
			{
				switch (mode)
				{
					case MODE_ADDRESS:

						if (latch.bit == 8)
						{
							mode = MODE_ACK;
							output = 0x10;
						}
						break;

					case MODE_ACK:

						mode = next;
						latch.bit = 0;
						output = 0x10;
						break;

					case MODE_READ:

						if (latch.bit == 8)
						{
							mode = MODE_ACK_WAIT;
							latch.address = (latch.address+1) & 0x7F;
						}
						break;

					case MODE_WRITE:

						if (latch.bit == 8)
						{
							mode = MODE_ACK;
							next = MODE_IDLE;
							mem[latch.address] = latch.data;
							latch.address = (latch.address+1) & 0x7F;
						}
						break;
				}
			}

			template<>
			void Bandai::E24C0X<256>::Rise(const uint bit)
			{
				NST_ASSERT( bit <= 1 );

				switch (mode)
				{
					case MODE_DATA:

						if (latch.bit < 8)
						{
							latch.data &= ~(1U << (7-latch.bit));
							latch.data |= bit << (7-latch.bit++);
						}
						break;

					case MODE_ADDRESS:

						if (latch.bit < 8)
						{
							latch.address &= ~(1U << (7-latch.bit));
							latch.address |= bit << (7-latch.bit++);
						}
						break;

					case MODE_READ:

						if (latch.bit < 8)
							output = (latch.data & (1U << (7-latch.bit++))) ? 0x10 : 0x00;

						break;

					case MODE_WRITE:

						if (latch.bit < 8)
						{
							latch.data &= ~(1U << (7-latch.bit));
							latch.data |= bit << (7-latch.bit++);
						}
						break;

					case MODE_NOT_ACK:

						output = 0x10;
						break;

					case MODE_ACK:

						output = 0x00;
						break;

					case MODE_ACK_WAIT:

						if (bit == 0)
						{
							next = MODE_READ;
							latch.data = mem[latch.address];
						}
						break;
				}
			}

			template<>
			void Bandai::E24C0X<256>::Fall()
			{
				switch (mode)
				{
					case MODE_DATA:

						if (latch.bit == 8)
						{
							if ((latch.data & 0xA0) == 0xA0)
							{
								latch.bit = 0;
								mode = MODE_ACK;
								rw = latch.data & 0x01;
								output = 0x10;

								if (rw)
								{
									next = MODE_READ;
									latch.data = mem[latch.address];
								}
								else
								{
									next = MODE_ADDRESS;
								}
							}
							else
							{
								mode = MODE_NOT_ACK;
								next = MODE_IDLE;
								output = 0x10;
							}
						}
						break;

					case MODE_ADDRESS:

						if (latch.bit == 8)
						{
							latch.bit = 0;
							mode = MODE_ACK;
							next = (rw ? MODE_IDLE : MODE_WRITE);
							output = 0x10;
						}
						break;

					case MODE_READ:

						if (latch.bit == 8)
						{
							mode = MODE_ACK_WAIT;
							latch.address = (latch.address+1) & 0xFF;
						}
						break;

					case MODE_WRITE:

						if (latch.bit == 8)
						{
							latch.bit = 0;
							mode = MODE_ACK;
							next = MODE_WRITE;
							mem[latch.address] = latch.data;
							latch.address = (latch.address+1) & 0xFF;
						}
						break;

					case MODE_NOT_ACK:

						mode = MODE_IDLE;
						latch.bit = 0;
						output = 0x10;
						break;

					case MODE_ACK:
					case MODE_ACK_WAIT:

						mode = next;
						latch.bit = 0;
						output = 0x10;
						break;
				}
			}

			template<uint N>
			void Bandai::E24C0X<N>::Set(const uint scl,const uint sda)
			{
				if (line.scl && sda < line.sda)
				{
					Start();
				}
				else if (line.scl && sda > line.sda)
				{
					Stop();
				}
				else if (scl > line.scl)
				{
					Rise( sda >> 6 );
				}
				else if (scl < line.scl)
				{
					Fall();
				}

				line.scl = scl;
				line.sda = sda;
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			class Bandai::DatachJointSystem : public Peripherals::BarcodeReader
			{
				enum
				{
					CC_INTERVAL = 1000
				};

				bool SubTransfer(cstring,uint,u8*);

				NES_DECL_HOOK( Transfer )

				Cpu& cpu;
				Cycle cycles;
				uint data;

				bool IsDigitsSupported(uint count) const
				{
					return count == MIN_DIGITS || count == MAX_DIGITS;
				}

			public:

				DatachJointSystem(Cpu& c)
				: cpu(c), cycles(NES_CYCLE_MAX), data(0x00) {}

				void Reset()
				{
					BarcodeReader::Reset();
					cycles = NES_CYCLE_MAX;
					data = 0x00;
					cpu.AddHook( Hook(this,&DatachJointSystem::Hook_Transfer) );
				}

				void SaveState(State::Saver& state) const
				{
					NST_ASSERT( IsTransferring() && cycles != NES_CYCLE_MAX );

					uint next;

					if (cycles > cpu.GetMasterClockCycles())
						next = (cycles - cpu.GetMasterClockCycles()) / (cpu.GetMode() == MODE_NTSC ? Cpu::MC_DIV_NTSC : Cpu::MC_DIV_PAL);
					else
						next = 0;

					state.Begin('C','Y','C','\0').Write16( next ).End();
					BarcodeReader::SaveState( state );
				}

				void LoadState(State::Loader& state)
				{
					BarcodeReader::Reset();
					cycles = NES_CYCLE_MAX;

					while (const dword chunk = state.Begin())
					{
						if (chunk == NES_STATE_CHUNK_ID('C','Y','C','\0'))
							cycles = state.Read16();
						else
							BarcodeReader::LoadState( state, chunk );

						state.End();
					}

					if (IsTransferring())
					{
						data = Latch();

						if (cycles > CC_INTERVAL)
							cycles = CC_INTERVAL;

						cycles = cpu.GetMasterClockCycles() + cpu.GetMasterClockCycle(1) * cycles;
					}
					else
					{
						cycles = NES_CYCLE_MAX;
						data = 0x00;
					}
				}

				void VSync()
				{
					if (cycles != NES_CYCLE_MAX)
					{
						if (cycles >= cpu.GetMasterClockFrameCycles())
							cycles -= cpu.GetMasterClockFrameCycles();
						else
							cycles = 0;
					}
				}

				uint GetData() const
				{
					return data;
				}
			};

			bool Bandai::HasEEPROM(const uint size,const Type type,const dword crc)
			{
				if (type == TYPE_A)
				{
					switch (crc)
					{
						case 0x2E991109UL: // Dragon Ball Z Gaiden - Saiya Jin Zetsumetsu Keikaku
						case 0x9C3457E0UL: // -||-
						case 0xE49FC53EUL: // Dragon Ball Z 2 - Gekishin Freeza!!
						case 0x4DC67A94UL: // -||-
						case 0x4710C295UL: // -||-
						case 0xEEC199D4UL: // -||-
						case 0x7B1730A9UL: // -||-
						case 0x1D6F27F7UL: // -||-
						case 0x1582FEE0UL: // -||- redump
						case 0x09499F4DUL: // Dragon Ball Z 3 - Ressen Jinzou Ningen
						case 0xBD064C82UL: // -||-
						case 0xC196AC58UL: // -||-
						case 0x170250DEUL: // Rokudenashi Blues
						case 0x73AC76DBUL: // SD Gundam Gaiden - Knight Gundam Monogatari 2 - Hikari no Kishi
						case 0x81A15EB8UL: // SD Gundam Gaiden - Knight Gundam Monogatari 3 - Densetsu no Kishi Dan
							return (size == 256);
					}

					return (size == 128);
				}
				else
				{
					return (type == TYPE_C);
				}
			}

			Bandai::Bandai(Context& c,const Type t)
			:
			Mapper (c,t == TYPE_B ? WRAM_AUTO : WRAM_NONE),
			irq    (c.cpu),
			datach (t == TYPE_C ? new DatachJointSystem(c.cpu) : NULL),
			e24C01 (HasEEPROM( 128, t, c.prgCrc ) ? new E24C0X<128> : NULL),
			e24C02 (HasEEPROM( 256, t, c.prgCrc ) ? new E24C0X<256> : NULL),
			type   (t)
			{
				if (e24C01 || e24C02)
				{
					std::vector<u8> data;
					Api::User::fileIoCallback( Api::User::FILE_LOAD_EEPROM, data );

					if (const ulong size = data.size())
					{
						ulong pos = 0;

						if (e24C02 && size >= pos + 256)
						{
							e24C02->Load( &data.front() );
							pos += 256;
						}

						if (e24C01 && size >= pos + 128)
						{
							e24C01->Load( &data.front() + pos );
							pos += 128;
						}
					}
				}
			}

			Bandai::~Bandai()
			{
				if (e24C01 || e24C02)
				{
					try
					{
						std::vector<u8> data;
						data.resize( (e24C01 ? 128 : 0) + (e24C02 ? 256 : 0) );

						bool altered = false;

						if (e24C02 && e24C02->Save( &data.front() ))
							altered = true;

						if (e24C01 && e24C01->Save( &data.front() + (e24C02 ? 256 : 0) ))
							altered = true;

						if (altered)
							Api::User::fileIoCallback( Api::User::FILE_SAVE_EEPROM, data );
					}
					catch (...)
					{
					}

					delete e24C01;
					delete e24C02;
				}

				delete datach;
			}

			Bandai::Device Bandai::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_BARCODE_READER && datach)
					return datach;
				else
					return Mapper::QueryDevice( type );
			}

			void Bandai::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					latch = 0;
					count = 0;
				}
			}

			void Bandai::SubReset(const bool hard)
			{
				irq.Reset( hard, hard ? false : irq.IsLineEnabled() );

				if (datach)
					datach->Reset();

				if (e24C01)
					e24C01->Reset();

				if (e24C02)
					e24C02->Reset();

				switch (type)
				{
					case TYPE_A:

						for (dword i=0x6000U; i <= 0x7FFFU; i += 0x100)
							Map( i, e24C01 ? &Bandai::Peek_6000_A1 : &Bandai::Peek_6000_A2 );

						for (dword i=0x6000U; i <= 0xFFFFU; i += 0x10)
						{
							Map( i + 0x0, CHR_SWAP_1K_0      );
							Map( i + 0x1, CHR_SWAP_1K_1      );
							Map( i + 0x2, CHR_SWAP_1K_2      );
							Map( i + 0x3, CHR_SWAP_1K_3      );
							Map( i + 0x4, CHR_SWAP_1K_4      );
							Map( i + 0x5, CHR_SWAP_1K_5      );
							Map( i + 0x6, CHR_SWAP_1K_6      );
							Map( i + 0x7, CHR_SWAP_1K_7      );
							Map( i + 0x8, PRG_SWAP_16K       );
							Map( i + 0x9, NMT_SWAP_VH01      );
							Map( i + 0xA, &Bandai::Poke_800A );
							Map( i + 0xB, &Bandai::Poke_800B );
							Map( i + 0xC, &Bandai::Poke_800C );
							Map( i + 0xD, e24C01 ? &Bandai::Poke_800D_A1 : &Bandai::Poke_800D_A2 );
						}
						break;

					case TYPE_B:

						for (dword i=0x8000U; i <= 0xFFFFU; i += 0x10)
						{
							Map( i + 0x0, &Bandai::Poke_8000_B );
							Map( i + 0x8, &Bandai::Poke_8008_B );
							Map( i + 0x9, NMT_SWAP_VH01        );
							Map( i + 0xA, &Bandai::Poke_800A   );
							Map( i + 0xB, &Bandai::Poke_800B   );
							Map( i + 0xC, &Bandai::Poke_800C   );
						}
						break;

					case TYPE_C:

						for (dword i=0x6000U; i <= 0x7FFFU; i += 0x100)
							Map( i, &Bandai::Peek_6000_C );

						for (dword i=0x6000U; i <= 0xFFFFU; i += 0x10)
						{
							Map( i + 0x0, i + 0x7, &Bandai::Poke_8000_C );
							Map( i + 0x8,          PRG_SWAP_16K         );
							Map( i + 0x9,          NMT_SWAP_VH01        );
							Map( i + 0xA,          &Bandai::Poke_800A   );
							Map( i + 0xB,          &Bandai::Poke_800B   );
							Map( i + 0xC,          &Bandai::Poke_800C   );
							Map( i + 0xD,          &Bandai::Poke_800D_C );
						}
						break;
				}
			}

			void Bandai::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('B','A','N','\0') );

				if (id == NES_STATE_CHUNK_ID('B','A','N','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case NES_STATE_CHUNK_ID('I','R','Q','\0'):
							{
								const State::Loader::Data<5> data( state );

								irq.EnableLine( data[0] & 0x1 );
								irq.unit.latch = data[1] | (data[2] << 8);
								irq.unit.count = data[3] | (data[4] << 8);
								break;
							}

							case NES_STATE_CHUNK_ID('B','R','C','\0'):

								if (datach)
									datach->LoadState( State::Loader::Subset(state).Ref() );

								break;

							case NES_STATE_CHUNK_ID('C','0','1','\0'):

								if (e24C01)
									e24C01->LoadState( State::Loader::Subset(state).Ref() );

								break;

							case NES_STATE_CHUNK_ID('C','0','2','\0'):

								if (e24C02)
									e24C02->LoadState( State::Loader::Subset(state).Ref() );

								break;
						}

						state.End();
					}
				}
			}

			void Bandai::BaseSave(State::Saver& state) const
			{
				state.Begin('B','A','N','\0');

				const u8 data[5] =
				{
					irq.IsLineEnabled() ? 0x1 : 0x0,
					irq.unit.latch & 0xFF,
					irq.unit.latch >> 8,
					irq.unit.count & 0xFF,
					irq.unit.count >> 8
				};

				state.Begin('I','R','Q','\0').Write( data ).End();

				if (datach && datach->IsTransferring())
					datach->SaveState( State::Saver::Subset(state,'B','R','C','\0').Ref() );

				if (e24C01)
					e24C01->SaveState( State::Saver::Subset(state,'C','0','1','\0').Ref() );

				if (e24C02)
					e24C02->SaveState( State::Saver::Subset(state,'C','0','2','\0').Ref() );

				state.End();
			}

			bool Bandai::DatachJointSystem::SubTransfer(cstring const string,uint length,u8* NST_RESTRICT stream)
			{
				if (length != MAX_DIGITS && length != MIN_DIGITS)
					return false;

				static const u8 prefixParityType[10][6] =
				{
					{8,8,8,8,8,8}, {8,8,0,8,0,0},
					{8,8,0,0,8,0}, {8,8,0,0,0,8},
					{8,0,8,8,0,0}, {8,0,0,8,8,0},
					{8,0,0,0,8,8}, {8,0,8,0,8,0},
					{8,0,8,0,0,8}, {8,0,0,8,0,8}
				};

				static const u8 dataLeftOdd[10][7] =
				{
					{8,8,8,0,0,8,0}, {8,8,0,0,8,8,0},
					{8,8,0,8,8,0,0}, {8,0,0,0,0,8,0},
					{8,0,8,8,8,0,0}, {8,0,0,8,8,8,0},
					{8,0,8,0,0,0,0}, {8,0,0,0,8,0,0},
					{8,0,0,8,0,0,0}, {8,8,8,0,8,0,0}
				};

				static const u8 dataLeftEven[10][7] =
				{
					{8,0,8,8,0,0,0}, {8,0,0,8,8,0,0},
					{8,8,0,0,8,0,0}, {8,0,8,8,8,8,0},
					{8,8,0,0,0,8,0}, {8,0,0,0,8,8,0},
					{8,8,8,8,0,8,0}, {8,8,0,8,8,8,0},
					{8,8,8,0,8,8,0}, {8,8,0,8,0,0,0}
				};

				static const u8 dataRight[10][7] =
				{
					{0,0,0,8,8,0,8}, {0,0,8,8,0,0,8},
					{0,0,8,0,0,8,8}, {0,8,8,8,8,0,8},
					{0,8,0,0,0,8,8}, {0,8,8,0,0,0,8},
					{0,8,0,8,8,8,8}, {0,8,8,8,0,8,8},
					{0,8,8,0,8,8,8}, {0,0,0,8,0,8,8}
				};

				u8 code[16];

				for (uint i=0; i < length; ++i)
				{
					if (string[i] >= '0' && string[i] <= '9')
						code[i] = string[i] - '0';
					else
						return false;
				}

				for (uint i=0; i < 1+32; ++i)
					*stream++ = 8;

				*stream++ = 0;
				*stream++ = 8;
				*stream++ = 0;

				uint sum = 0;

				if (length == MAX_DIGITS)
				{
					for (uint i=0; i < 6; ++i)
					{
						if (prefixParityType[code[0]][i])
						{
							for (uint j=0; j < 7; ++j)
								*stream++ = dataLeftOdd[code[i+1]][j];
						}
						else
						{
							for (uint j=0; j < 7; ++j)
								*stream++ = dataLeftEven[code[i+1]][j];
						}
					}

					*stream++ = 8;
					*stream++ = 0;
					*stream++ = 8;
					*stream++ = 0;
					*stream++ = 8;

					for (uint i=7; i < 12; ++i)
					{
						for (uint j=0; j < 7; ++j)
							*stream++ = dataRight[code[i]][j];
					}

					for (uint i=0; i < 12; ++i)
						sum += (i & 1) ? (code[i] * 3) : (code[i] * 1);
				}
				else
				{
					for (uint i=0; i < 4; ++i)
					{
						for (uint j=0; j < 7; ++j)
							*stream++ = dataLeftOdd[code[i]][j];
					}

					*stream++ = 8;
					*stream++ = 0;
					*stream++ = 8;
					*stream++ = 0;
					*stream++ = 8;

					for (uint i=4; i < 7; ++i)
					{
						for (uint j=0; j < 7; ++j)
							*stream++ = dataRight[code[i]][j];
					}

					for (uint i=0; i < 7; ++i)
						sum += (i & 1) ? (code[i] * 1) : (code[i] * 3);
				}

				sum = (10 - (sum % 10)) % 10;

				for (uint i=0; i < 7; ++i)
					*stream++ = dataRight[sum][i];

				*stream++ = 0;
				*stream++ = 8;
				*stream++ = 0;

				for (uint i=0; i < 32; ++i)
					*stream++ = 8;

				cycles = cpu.GetMasterClockCycles() + cpu.GetMasterClockCycle(1) * CC_INTERVAL;

				return true;
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_HOOK(Bandai::DatachJointSystem,Transfer)
			{
				while (cycles <= cpu.GetMasterClockCycles())
				{
					data = Fetch();

					if (data != END)
					{
						cycles += cpu.GetMasterClockCycle(1) * CC_INTERVAL;
					}
					else
					{
						data = 0x00;
						cycles = NES_CYCLE_MAX;
						break;
					}
				}
			}

			NES_PEEK(Bandai,6000_A1)
			{
				return e24C01->Read();
			}

			NES_PEEK(Bandai,6000_A2)
			{
				return e24C02->Read();
			}

			NES_PEEK(Bandai,6000_C)
			{
				return datach->GetData() | (e24C01->Read() & e24C02->Read());
			}

			NES_POKE(Bandai,8000_B)
			{
				data = (data & 0x1) << 4;
				prg.SwapBanks<SIZE_16K,0x0000U>( data | (prg.GetBank<SIZE_16K,0x0000U>() & 0x0F), data | 0xF );
			}

			NES_POKE(Bandai,8008_B)
			{
				prg.SwapBank<SIZE_16K,0x0000U>( (prg.GetBank<SIZE_16K,0x0000U>() & 0x10) | (data & 0x0F) );
			}

			NES_POKE(Bandai,8000_C)
			{
				if (!chr.Source().Writable())
					chr.SwapBank<SIZE_1K>( (address & 0x7) << 10, data );

				e24C01->SetScl( (data << 2) & 0x20 );
			}

			NES_POKE(Bandai,800A)
			{
				irq.Update();
				irq.unit.count = irq.unit.latch + 1;
				irq.EnableLine( data & 0x1 );
				irq.ClearIRQ();
			}

			NES_POKE(Bandai,800B)
			{
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0xFF00U) | data;
			}

			NES_POKE(Bandai,800C)
			{
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0x00FFU) | (data << 8);
			}

			NES_POKE(Bandai,800D_A1)
			{
				e24C01->Set( data & 0x20, data & 0x40 );
			}

			NES_POKE(Bandai,800D_A2)
			{
				e24C02->Set( data & 0x20, data & 0x40 );
			}

			NES_POKE(Bandai,800D_C)
			{
				e24C02->Set( data & 0x20, data & 0x40 );
				e24C01->SetSda( data & 0x40 );
			}

			ibool Bandai::Irq::Signal()
			{
				count = (count - 1) & 0xFFFFU;
				return count == 0;
			}

			void Bandai::VSync()
			{
				irq.VSync();

				if (datach)
					datach->VSync();
			}
		}
	}
}
