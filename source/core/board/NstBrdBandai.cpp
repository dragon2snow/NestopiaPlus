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

#include <cstring>
#include "../NstMapper.hpp"
#include "../NstClock.hpp"
#include "../NstPrpBarcodeReader.hpp"
#include "../NstFile.hpp"
#include "NstBrdBandai.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Bandai::Eeprom
			{
			public:

				template<uint N>
				class X24C0X;

				explicit Eeprom(Type);

				void Load();
				void Save() const;

				Pointer< X24C0X<128> > x24C01;
				Pointer< X24C0X<256> > x24C02;

			private:

				File file;
			};

			template<uint N>
			class Bandai::Eeprom::X24C0X
			{
			public:

				enum
				{
					SIZE = N
				};

				void Reset();
				void Set(uint,uint);
				void LoadState(State::Loader&);
				void SaveState(State::Saver&,dword) const;

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

				struct
				{
					uint scl;
					uint sda;
				}   line;

				Mode mode;
				Mode next;

				struct
				{
					uint bit;
					uint address;
					uint data;
				}   latch;

				ibool rw;
				uint output;
				byte mem[SIZE];

			public:

				X24C0X()
				{
					std::memset( mem, 0, SIZE );
				}

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

				void Load(const byte* NST_RESTRICT in)
				{
					std::memcpy( mem, in, SIZE );
				}

				void Save(byte* NST_RESTRICT out) const
				{
					std::memcpy( out, mem, SIZE );
				}
			};

			template<uint N>
			void Bandai::Eeprom::X24C0X<N>::Reset()
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
			void Bandai::Eeprom::X24C0X<N>::SaveState(State::Saver& state,const dword baseChunk) const
			{
				state.Begin( baseChunk );

				const byte data[6] =
				{
					line.scl | line.sda,
					uint(mode << 0) | uint(next << 4),
					latch.address,
					latch.data,
					latch.bit,
					output | (rw ? 0x80 : 0x00)
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				state.Begin( AsciiId<'R','A','M'>::V ).Compress( mem ).End();

				state.End();
			}

			template<uint N>
			void Bandai::Eeprom::X24C0X<N>::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'R','E','G'>::V:
						{
							State::Loader::Data<6> data( state );

							line.scl = data[0] & 0x20;
							line.sda = data[0] & 0x40;

							if ((data[1] & 0xF) < MODE_MAX)
								mode = static_cast<Mode>(data[1] & 0xF);

							if ((data[1] >> 4) < MODE_MAX)
								next = static_cast<Mode>(data[1] >> 4);

							latch.address = data[2] & (SIZE-1);
							latch.data = data[3];
							latch.bit = NST_MAX(data[4],8);

							rw = data[5] & 0x80;
							output = data[5] & 0x10;
							break;
						}

						case AsciiId<'R','A','M'>::V:

							state.Uncompress( mem );
							break;
					}

					state.End();
				}
			}

			template<>
			void Bandai::Eeprom::X24C0X<128>::Start()
			{
				mode = MODE_ADDRESS;
				latch.bit = 0;
				latch.address = 0;
				output = 0x10;
			}

			template<>
			void Bandai::Eeprom::X24C0X<256>::Start()
			{
				mode = MODE_DATA;
				latch.bit = 0;
				output = 0x10;
			}

			template<uint N>
			void Bandai::Eeprom::X24C0X<N>::Stop()
			{
				mode = MODE_IDLE;
				output = 0x10;
			}

			template<>
			void Bandai::Eeprom::X24C0X<128>::Rise(const uint bit)
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
							output = (latch.data & 1U << latch.bit++) ? 0x10 : 0x00;

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
			void Bandai::Eeprom::X24C0X<128>::Fall()
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
			void Bandai::Eeprom::X24C0X<256>::Rise(const uint bit)
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
							output = (latch.data & 1U << (7-latch.bit++)) ? 0x10 : 0x00;

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
			void Bandai::Eeprom::X24C0X<256>::Fall()
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
			void Bandai::Eeprom::X24C0X<N>::Set(const uint scl,const uint sda)
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

			class Bandai::DatachJointSystem : public Peripherals::BarcodeReader
			{
			public:

				explicit DatachJointSystem(Cpu&);

				void Reset();
				void SaveState(State::Saver&,dword) const;
				void LoadState(State::Loader&);

			private:

				enum
				{
					CC_INTERVAL = 1000
				};

				bool SubTransfer(cstring,uint,byte*);

				NES_DECL_HOOK( Transfer );

				Cpu& cpu;
				Cycle cycles;
				uint data;

				bool IsDigitsSupported(uint count) const
				{
					return count == MIN_DIGITS || count == MAX_DIGITS;
				}

			public:

				void VSync()
				{
					if (cycles != Cpu::CYCLE_MAX)
					{
						if (cycles >= cpu.GetFrameCycles())
							cycles -= cpu.GetFrameCycles();
						else
							cycles = 0;
					}
				}

				uint GetData() const
				{
					return data;
				}
			};

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Bandai::DatachJointSystem::DatachJointSystem(Cpu& c)
			: cpu(c), cycles(Cpu::CYCLE_MAX), data(0x00) {}

			Bandai::Eeprom::Eeprom(const Type t)
			:
			x24C01 (t == TYPE_DATACH || t == TYPE_LZ93D50_E2401 ? new X24C0X<128> : NULL),
			x24C02 (t == TYPE_DATACH || t == TYPE_LZ93D50_E2402 ? new X24C0X<256> : NULL)
			{
			}

			Bandai::Bandai(Context& c,const Type t)
			:
			Mapper (c,t == TYPE_WRAM ? CROM_MAX_256K|NMT_VERTICAL : CROM_MAX_256K|WRAM_DEFAULT|NMT_VERTICAL),
			irq    (c.cpu),
			datach (t == TYPE_DATACH ? new DatachJointSystem(c.cpu) : NULL),
			eeprom (t == TYPE_DATACH || t == TYPE_LZ93D50_E2401 || t == TYPE_LZ93D50_E2402 ? new Eeprom(t) : NULL),
			type   (t)
			{
				if (eeprom)
					eeprom->Load();
			}

			Bandai::~Bandai()
			{
			}

			Bandai::Device Bandai::QueryDevice(DeviceType devType)
			{
				if (devType == DEVICE_BARCODE_READER && datach)
				{
					return &*datach;
				}
				else
				{
					return Mapper::QueryDevice( devType );
				}
			}

			void Bandai::Eeprom::Load()
			{
				byte buffer[256+128];
				std::memset( buffer, 0, sizeof(buffer) );

				file.Load( File::LOAD_EEPROM, buffer, (x24C02 ? 256 : 0) + (x24C01 ? 128 : 0) );

				if (x24C02)
					x24C02->Load( buffer );

				if (x24C01)
					x24C01->Load( buffer + (x24C02 ? 256 : 0) );
			}

			void Bandai::Eeprom::Save() const
			{
				byte buffer[256+128];

				if (x24C02)
					x24C02->Save( buffer );

				if (x24C01)
					x24C01->Save( buffer + (x24C02 ? 256 : 0) );

				file.Save( File::SAVE_EEPROM, buffer, (x24C02 ? 256 : 0) + (x24C01 ? 128 : 0) );
			}

			void Bandai::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					latch = 0;
					count = 0;
				}
			}

			void Bandai::DatachJointSystem::Reset()
			{
				BarcodeReader::Reset();
				cycles = Cpu::CYCLE_MAX;
				data = 0x00;
				cpu.AddHook( Hook(this,&DatachJointSystem::Hook_Transfer) );
			}

			void Bandai::SubReset(const bool hard)
			{
				irq.Reset( hard, hard ? false : irq.Connected() );

				if (datach)
					datach->Reset();

				if (eeprom)
				{
					if (eeprom->x24C01)
						eeprom->x24C01->Reset();

					if (eeprom->x24C02)
						eeprom->x24C02->Reset();
				}

				switch (type)
				{
					case TYPE_WRAM:

						for (dword i=0x8000; i <= 0xFFFF; i += 0x10)
						{
							Map( i + 0x0, &Bandai::Poke_8000_B );
							Map( i + 0x8, &Bandai::Poke_8008_B );
							Map( i + 0x9, NMT_SWAP_VH01        );
							Map( i + 0xA, &Bandai::Poke_800A   );
							Map( i + 0xB, &Bandai::Poke_800B   );
							Map( i + 0xC, &Bandai::Poke_800C   );
						}
						break;

					case TYPE_DATACH:

						for (uint i=0x6000; i <= 0x7FFF; i += 0x100)
							Map( i, &Bandai::Peek_6000_C );

						for (dword i=0x6000; i <= 0xFFFF; i += 0x10)
						{
							Map( uint(i) + 0x0, uint(i) + 0x7, &Bandai::Poke_8000_C );
							Map( uint(i) + 0x8,                PRG_SWAP_16K_0       );
							Map( uint(i) + 0x9,                NMT_SWAP_VH01        );
							Map( uint(i) + 0xA,                &Bandai::Poke_800A   );
							Map( uint(i) + 0xB,                &Bandai::Poke_800B   );
							Map( uint(i) + 0xC,                &Bandai::Poke_800C   );
							Map( uint(i) + 0xD,                &Bandai::Poke_800D_C );
						}
						break;

					default:

						if (eeprom)
						{
							for (uint i=0x6000; i <= 0x7FFF; i += 0x100)
								Map( i, eeprom->x24C01 ? &Bandai::Peek_6000_A1 : &Bandai::Peek_6000_A2 );
						}

						for (dword i=0x6000; i <= 0xFFFF; i += 0x10)
						{
							Map( i + 0x0, CHR_SWAP_1K_0      );
							Map( i + 0x1, CHR_SWAP_1K_1      );
							Map( i + 0x2, CHR_SWAP_1K_2      );
							Map( i + 0x3, CHR_SWAP_1K_3      );
							Map( i + 0x4, CHR_SWAP_1K_4      );
							Map( i + 0x5, CHR_SWAP_1K_5      );
							Map( i + 0x6, CHR_SWAP_1K_6      );
							Map( i + 0x7, CHR_SWAP_1K_7      );
							Map( i + 0x8, PRG_SWAP_16K_0     );
							Map( i + 0x9, NMT_SWAP_VH01      );
							Map( i + 0xA, &Bandai::Poke_800A );
							Map( i + 0xB, &Bandai::Poke_800B );
							Map( i + 0xC, &Bandai::Poke_800C );

							if (eeprom)
								Map( i + 0xD, eeprom->x24C01 ? &Bandai::Poke_800D_A1 : &Bandai::Poke_800D_A2 );
						}
						break;
				}
			}

			void Bandai::BaseLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'B','A','N'>::V) );

				if (baseChunk == AsciiId<'B','A','N'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'I','R','Q'>::V:
							{
								State::Loader::Data<5> data( state );

								irq.Connect( data[0] & 0x1 );
								irq.unit.latch = data[1] | data[2] << 8;
								irq.unit.count = data[3] | data[4] << 8;
								break;
							}

							case AsciiId<'B','R','C'>::V:

								NST_VERIFY( datach );

								if (datach)
									datach->LoadState( state );

								break;

							case AsciiId<'C','0','1'>::V:

								NST_VERIFY( eeprom && eeprom->x24C01 );

								if (eeprom && eeprom->x24C01)
									eeprom->x24C01->LoadState( state );

								break;

							case AsciiId<'C','0','2'>::V:

								NST_VERIFY( eeprom && eeprom->x24C02 );

								if (eeprom && eeprom->x24C02)
									eeprom->x24C02->LoadState( state );

								break;
						}

						state.End();
					}
				}
			}

			void Bandai::DatachJointSystem::LoadState(State::Loader& state)
			{
				BarcodeReader::Reset();
				cycles = Cpu::CYCLE_MAX;

				while (const dword chunk = state.Begin())
				{
					if (chunk == AsciiId<'C','Y','C'>::V)
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

					cycles = cpu.GetCycles() + cpu.GetClock() * cycles;
				}
				else
				{
					cycles = Cpu::CYCLE_MAX;
					data = 0x00;
				}
			}

			void Bandai::BaseSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'B','A','N'>::V );

				const byte data[5] =
				{
					irq.Connected() ? 0x1 : 0x0,
					irq.unit.latch & 0xFF,
					irq.unit.latch >> 8,
					irq.unit.count & 0xFF,
					irq.unit.count >> 8
				};

				state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();

				if (datach && datach->IsTransferring())
					datach->SaveState( state, AsciiId<'B','R','C'>::V );

				if (eeprom)
				{
					if (eeprom->x24C01)
						eeprom->x24C01->SaveState( state, AsciiId<'C','0','1'>::V );

					if (eeprom->x24C02)
						eeprom->x24C02->SaveState( state, AsciiId<'C','0','2'>::V );
				}

				state.End();
			}

			void Bandai::DatachJointSystem::SaveState(State::Saver& state,const dword baseChunk) const
			{
				NST_ASSERT( IsTransferring() && cycles != Cpu::CYCLE_MAX );

				state.Begin( baseChunk );

				uint next;

				if (cycles > cpu.GetCycles())
					next = (cycles - cpu.GetCycles()) / (cpu.GetRegion() == Region::NTSC ? Clocks::RP2A03_CC : Clocks::RP2A07_CC);
				else
					next = 0;

				state.Begin( AsciiId<'C','Y','C'>::V ).Write16( next ).End();
				BarcodeReader::SaveState( state );

				state.End();
			}

			bool Bandai::DatachJointSystem::SubTransfer(cstring const string,uint length,byte* NST_RESTRICT output)
			{
				if (length != MAX_DIGITS && length != MIN_DIGITS)
					return false;

				static const byte prefixParityType[10][6] =
				{
					{8,8,8,8,8,8}, {8,8,0,8,0,0},
					{8,8,0,0,8,0}, {8,8,0,0,0,8},
					{8,0,8,8,0,0}, {8,0,0,8,8,0},
					{8,0,0,0,8,8}, {8,0,8,0,8,0},
					{8,0,8,0,0,8}, {8,0,0,8,0,8}
				};

				static const byte dataLeftOdd[10][7] =
				{
					{8,8,8,0,0,8,0}, {8,8,0,0,8,8,0},
					{8,8,0,8,8,0,0}, {8,0,0,0,0,8,0},
					{8,0,8,8,8,0,0}, {8,0,0,8,8,8,0},
					{8,0,8,0,0,0,0}, {8,0,0,0,8,0,0},
					{8,0,0,8,0,0,0}, {8,8,8,0,8,0,0}
				};

				static const byte dataLeftEven[10][7] =
				{
					{8,0,8,8,0,0,0}, {8,0,0,8,8,0,0},
					{8,8,0,0,8,0,0}, {8,0,8,8,8,8,0},
					{8,8,0,0,0,8,0}, {8,0,0,0,8,8,0},
					{8,8,8,8,0,8,0}, {8,8,0,8,8,8,0},
					{8,8,8,0,8,8,0}, {8,8,0,8,0,0,0}
				};

				static const byte dataRight[10][7] =
				{
					{0,0,0,8,8,0,8}, {0,0,8,8,0,0,8},
					{0,0,8,0,0,8,8}, {0,8,8,8,8,0,8},
					{0,8,0,0,0,8,8}, {0,8,8,0,0,0,8},
					{0,8,0,8,8,8,8}, {0,8,8,8,0,8,8},
					{0,8,8,0,8,8,8}, {0,0,0,8,0,8,8}
				};

				byte code[16];

				for (uint i=0; i < length; ++i)
				{
					if (string[i] >= '0' && string[i] <= '9')
						code[i] = string[i] - '0';
					else
						return false;
				}

				for (uint i=0; i < 1+32; ++i)
					*output++ = 8;

				*output++ = 0;
				*output++ = 8;
				*output++ = 0;

				uint sum = 0;

				if (length == MAX_DIGITS)
				{
					for (uint i=0; i < 6; ++i)
					{
						if (prefixParityType[code[0]][i])
						{
							for (uint j=0; j < 7; ++j)
								*output++ = dataLeftOdd[code[i+1]][j];
						}
						else
						{
							for (uint j=0; j < 7; ++j)
								*output++ = dataLeftEven[code[i+1]][j];
						}
					}

					*output++ = 8;
					*output++ = 0;
					*output++ = 8;
					*output++ = 0;
					*output++ = 8;

					for (uint i=7; i < 12; ++i)
					{
						for (uint j=0; j < 7; ++j)
							*output++ = dataRight[code[i]][j];
					}

					for (uint i=0; i < 12; ++i)
						sum += (i & 1) ? (code[i] * 3) : (code[i] * 1);
				}
				else
				{
					for (uint i=0; i < 4; ++i)
					{
						for (uint j=0; j < 7; ++j)
							*output++ = dataLeftOdd[code[i]][j];
					}

					*output++ = 8;
					*output++ = 0;
					*output++ = 8;
					*output++ = 0;
					*output++ = 8;

					for (uint i=4; i < 7; ++i)
					{
						for (uint j=0; j < 7; ++j)
							*output++ = dataRight[code[i]][j];
					}

					for (uint i=0; i < 7; ++i)
						sum += (i & 1) ? (code[i] * 1) : (code[i] * 3);
				}

				sum = (10 - (sum % 10)) % 10;

				for (uint i=0; i < 7; ++i)
					*output++ = dataRight[sum][i];

				*output++ = 0;
				*output++ = 8;
				*output++ = 0;

				for (uint i=0; i < 32; ++i)
					*output++ = 8;

				cycles = cpu.GetCycles() + cpu.GetClock() * CC_INTERVAL;

				return true;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_HOOK(Bandai::DatachJointSystem,Transfer)
			{
				while (cycles <= cpu.GetCycles())
				{
					data = Fetch();

					if (data != END)
					{
						cycles += cpu.GetClock() * CC_INTERVAL;
					}
					else
					{
						data = 0x00;
						cycles = Cpu::CYCLE_MAX;
						break;
					}
				}
			}

			NES_PEEK(Bandai,6000_A1)
			{
				return eeprom->x24C01->Read();
			}

			NES_PEEK(Bandai,6000_A2)
			{
				return eeprom->x24C02->Read();
			}

			NES_PEEK(Bandai,6000_C)
			{
				return datach->GetData() | (eeprom->x24C01->Read() & eeprom->x24C02->Read());
			}

			NES_POKE_D(Bandai,8000_B)
			{
				data = data << 4 & 0x10;
				prg.SwapBanks<SIZE_16K,0x0000>( data | (prg.GetBank<SIZE_16K,0x0000>() & 0x0F), data | 0xF );
			}

			NES_POKE_D(Bandai,8008_B)
			{
				prg.SwapBank<SIZE_16K,0x0000>( (prg.GetBank<SIZE_16K,0x0000>() & 0x10) | (data & 0x0F) );
			}

			NES_POKE_AD(Bandai,8000_C)
			{
				if (!chr.Source().Writable())
					chr.SwapBank<SIZE_1K>( (address & 0x7) << 10, data );

				eeprom->x24C01->SetScl( data << 2 & 0x20 );
			}

			NES_POKE_D(Bandai,800A)
			{
				irq.Update();
				irq.unit.count = irq.unit.latch + 1;
				irq.Connect( data & 0x1 );
				irq.ClearIRQ();
			}

			NES_POKE_D(Bandai,800B)
			{
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0xFF00) | data << 0;
			}

			NES_POKE_D(Bandai,800C)
			{
				irq.Update();
				irq.unit.latch = (irq.unit.latch & 0x00FF) | data << 8;
			}

			NES_POKE_D(Bandai,800D_A1)
			{
				eeprom->x24C01->Set( data & 0x20, data & 0x40 );
			}

			NES_POKE_D(Bandai,800D_A2)
			{
				eeprom->x24C02->Set( data & 0x20, data & 0x40 );
			}

			NES_POKE_D(Bandai,800D_C)
			{
				eeprom->x24C02->Set( data & 0x20, data & 0x40 );
				eeprom->x24C01->SetSda( data & 0x40 );
			}

			bool Bandai::Irq::Clock()
			{
				count = (count - 1U) & 0xFFFF;
				return count == 0;
			}

			void Bandai::Sync(Event event,Input::Controllers*)
			{
				if (event == EVENT_END_FRAME)
				{
					irq.VSync();

					if (datach)
						datach->VSync();
				}
				else if (event == EVENT_POWER_OFF)
				{
					if (eeprom)
						eeprom->Save();
				}
			}
		}
	}
}
