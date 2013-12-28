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
#include "NstApiCheats.hpp"
#include "NstApiMachine.hpp"
#include "../NstCheats.hpp"

namespace Nes
{
	namespace Api
	{
		Result Cheats::GameGenieEncode(const Code& code,char (&characters)[9])
		{ 
			if (code.address < 0x8000U)
				return RESULT_ERR_INVALID_PARAM;

			const u8 codes[8] =
			{
				((code.value   >>  0) & (0x1|0x2|0x4)) | ((code.value   >> 4) & 0x8),
				((code.value   >>  4) & (0x1|0x2|0x4)) | ((code.address >> 4) & 0x8),
				((code.address >>  4) & (0x1|0x2|0x4)) | ((code.useCompare ? 0x8 : 0x0)),
				((code.address >> 12) & (0x1|0x2|0x4)) | ((code.address >> 0) & 0x8),
				((code.address >>  0) & (0x1|0x2|0x4)) | ((code.address >> 8) & 0x8),
				((code.address >>  8) & (0x1|0x2|0x4)) | ((code.useCompare ? code.compare : code.value) & 0x8),
				(code.useCompare ? (((code.compare >> 0) & (0x1|0x2|0x4)) | ((code.compare >> 4) & 0x8)) : 0),
				(code.useCompare ? (((code.compare >> 5) & (0x1|0x2|0x4)) | ((code.value   >> 0) & 0x8)) : 0)
			};

			uint i = code.useCompare ? 8 : 6;

			characters[i--] = '\0';

			do
			{
				static const char lut[] =
				{
					'A','P','Z','L','G','I','T','Y',
					'E','O','X','U','K','S','V','N'
				};

				characters[i] = lut[codes[i]];
			}
			while (i--);

			return RESULT_OK;
		}
	
		Result Cheats::GameGenieDecode(cstring const characters,Code& code)
		{ 
			if (characters == NULL)
				return RESULT_ERR_INVALID_PARAM;

			u8 codes[8];

			uint length = 6;

			for (uint i=0; i < length; ++i)
			{
				switch (characters[i])
				{
					case 'A': case 'a': codes[i] = 0x0; break;
					case 'P': case 'p': codes[i] = 0x1; break;
					case 'Z': case 'z': codes[i] = 0x2; break;
					case 'L': case 'l': codes[i] = 0x3; break;
					case 'G': case 'g': codes[i] = 0x4; break;
					case 'I': case 'i': codes[i] = 0x5; break;
					case 'T': case 't': codes[i] = 0x6; break;
					case 'Y': case 'y': codes[i] = 0x7; break;
					case 'E': case 'e': codes[i] = 0x8; break;
					case 'O': case 'o': codes[i] = 0x9; break;
					case 'X': case 'x': codes[i] = 0xA; break;
					case 'U': case 'u': codes[i] = 0xB; break;
					case 'K': case 'k': codes[i] = 0xC; break;
					case 'S': case 's': codes[i] = 0xD; break;
					case 'V': case 'v': codes[i] = 0xE; break;
					case 'N': case 'n': codes[i] = 0xF; break;

					default: return RESULT_ERR_INVALID_PARAM;
				}
	
				if ((i == 2) && (codes[2] & 0x8))
					length = 8;
			}

			code.address = 0x8000U |
			(
				( ( codes[4] & 0x1 ) << 0x0 ) | 
				( ( codes[4] & 0x2 ) << 0x0 ) |
				( ( codes[4] & 0x4 ) << 0x0 ) |
				( ( codes[3] & 0x8 ) << 0x0 ) |
				( ( codes[2] & 0x1 ) << 0x4 ) |
				( ( codes[2] & 0x2 ) << 0x4 ) |
				( ( codes[2] & 0x4 ) << 0x4 ) |
				( ( codes[1] & 0x8 ) << 0x4 ) |
				( ( codes[5] & 0x1 ) << 0x8 ) |
				( ( codes[5] & 0x2 ) << 0x8 ) |
				( ( codes[5] & 0x4 ) << 0x8 ) |
				( ( codes[4] & 0x8 ) << 0x8 ) |
				( ( codes[3] & 0x1 ) << 0xC ) |
				( ( codes[3] & 0x2 ) << 0xC ) |
				( ( codes[3] & 0x4 ) << 0xC )
			);

			code.value =
			(
				( ( codes[0] & 0x1 ) << 0x0 ) |
				( ( codes[0] & 0x2 ) << 0x0 ) |
				( ( codes[0] & 0x4 ) << 0x0 ) |
				( ( codes[1] & 0x1 ) << 0x4 ) |
				( ( codes[1] & 0x2 ) << 0x4 ) |
				( ( codes[1] & 0x4 ) << 0x4 ) |
				( ( codes[0] & 0x8 ) << 0x4 ) 
			);

			if (length == 8)
			{
				code.useCompare = true;
				code.value |= codes[7] & 0x8; 
				code.compare =
				(
					( ( codes[6] & 0x1 ) << 0x0 ) |
					( ( codes[6] & 0x2 ) << 0x0 ) |
					( ( codes[6] & 0x4 ) << 0x0 ) |
					( ( codes[5] & 0x8 ) << 0x0 ) |
					( ( codes[7] & 0x1 ) << 0x4 ) |
					( ( codes[7] & 0x2 ) << 0x4 ) |
					( ( codes[7] & 0x4 ) << 0x4 ) |
					( ( codes[6] & 0x8 ) << 0x4 ) 
				);
			}
			else
			{
				code.useCompare = false;
				code.value |= codes[5] & 0x8;
				code.compare = 0x00;
			}

			return RESULT_OK;
		}

		Result Cheats::ProActionRockyEncode(const Code& code,char (&characters)[9])
		{
			if (code.address < 0x8000U || !code.useCompare)
				return RESULT_ERR_INVALID_PARAM;

			const dword input = (code.address & 0x7FFFU) | (code.compare << 16) | (code.value << 24);
			dword output = 0;

			for (u32 i=31, key=0xFCBDD274UL; i--; key <<= 1)
			{
				static const uchar scrambled[] =
				{
					3,  13, 14,  1,  6,  9,  5,  0,
					12,  7,  2,  8, 10, 11,  4, 19,
					21, 23, 22, 20, 17, 16, 18, 29,
					31, 24, 26, 25, 30, 27, 28
				};

				const uint ctrl = (input >> scrambled[i]) & 0x1;
				output |= ((key >> 31) ^ ctrl) << (i+1);

				if (ctrl)
					key ^= 0xB8309722UL;
			}

			characters[8] = '\0';

			for (uint i=0; i < 8; ++i)
			{
				const int value = (output >> (i * 4)) & 0xF;
				characters[i ^ 7] = (value >= 0xA) ? (value - 0xA + 'A') : (value + '0'); 
			}

			return RESULT_OK;
		}

		Result Cheats::ProActionRockyDecode(cstring characters,Code& code)
		{
			if (characters == NULL)
				return RESULT_ERR_INVALID_PARAM;

			dword input=0, output=0;

			for (uint i=0; i < 8; ++i)
			{
				dword num; 
				const int character = characters[i ^ 7];

				if (character >= '0' && character <= '9')
				{
					num = character - '0';
				}
				else if (character >= 'A' && character <= 'F')
				{
					num = character - 'A' + 0xA;
				}
				else if (character >= 'a' && character <= 'f')
				{
					num = character - 'a' + 0xA;
				}
				else
				{
					return RESULT_ERR_INVALID_PARAM;
				}

				input |= num << (i * 4);
			}

			for (dword i=31, key=0xFCBDD274UL; i--; input <<= 1, key <<= 1)
			{
				if ((key ^ input) & 0x80000000UL)
				{
					static const uchar scrambled[] =
					{
						3,  13, 14,  1,  6,  9,  5,  0,
						12,  7,  2,  8, 10, 11,  4, 19,
						21, 23, 22, 20, 17, 16, 18, 29,
						31, 24, 26, 25, 30, 27, 28
					};

					output |= 1UL << scrambled[i];
					key ^= 0xB8309722UL;
				}
			}

			code.address    = (output & 0x7FFFU) | 0x8000U;
			code.compare    = (output >> 16) & 0xFF;
			code.value      = (output >> 24) & 0xFF;
			code.useCompare = true;

			return RESULT_OK;
		}

		Result Cheats::SetCode(const Code& code)
		{ 
			try
			{
				if (emulator.cheats == NULL)
					emulator.cheats = new Core::Cheats( emulator.cpu );

				return emulator.cheats->SetCode
				( 
			     	code.address, 
					code.value, 
					code.compare, 
					code.useCompare, 
					emulator.Is(Machine::GAME)
				);
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
	
		Result Cheats::DeleteCode(const dword index)
		{ 
			if (emulator.cheats)
			{
				const Result result = emulator.cheats->DeleteCode( index );

				if (NES_SUCCEEDED(result) && emulator.cheats->NumCodes() == 0)
					ClearCodes();

				return result;
			}
			else
			{
				return RESULT_ERR_INVALID_PARAM; 
			}
		}
	
		dword Cheats::NumCodes() const
		{ 
			return emulator.cheats ? emulator.cheats->NumCodes() : 0; 
		}
	
		Result Cheats::GetCode(dword index,Code& code) const
		{ 
			if (emulator.cheats)
				return emulator.cheats->GetCode( index, &code.address, &code.value, &code.compare, &code.useCompare );
			else
				return RESULT_ERR_INVALID_PARAM;
		}

		Result Cheats::GetCode(dword index,u16* address,u8* value,u8* compare,bool* useCompare) const
		{ 
			if (emulator.cheats)
				return emulator.cheats->GetCode( index, address, value, compare, useCompare );
			else
				return RESULT_ERR_INVALID_PARAM;
		}

		Result Cheats::ClearCodes()
		{
			if (emulator.cheats)
			{
				delete emulator.cheats;
				emulator.cheats = NULL;

				return RESULT_OK; 
			}
			else
			{
				return RESULT_NOP;
			}
		}

		Cheats::Ram Cheats::GetRam() const
		{
			return emulator.cpu.GetSystemRam();
		}
	}
}
