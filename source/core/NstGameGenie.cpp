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

#include "NstCore.hpp"
#include "NstCpu.hpp"
#include "NstGameGenie.hpp"

#define NES_PACKED_DATA       0x000000FFUL
#define NES_PACKED_COMPARE    0x0000FF00UL
#define NES_PACKED_ADDRESS    0x7FFF0000UL
#define NES_PACKED_USECOMPARE 0x80000000UL

#define NES_PACKED_SHIFT_DATA        0
#define NES_PACKED_SHIFT_COMPARE     8
#define NES_PACKED_SHIFT_ADDRESS    16
#define NES_PACKED_SHIFT_USECOMPARE 31

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		GameGenie::Code::Code()
		: 
		data       (0x00),
		compare    (0x00),
		address    (0x0000),
		useCompare (0x00)
		{}
	
		void GameGenie::Code::operator = (const Code& code)
		{
			data       = code.data;
			compare    = code.compare;
			address    = code.address;
			useCompare = code.useCompare;
		}
	
		Result GameGenie::Encode(const dword packed,char (&characters)[9])
		{
			return Code().Encode( packed, characters );
		}
	
		Result GameGenie::Decode(cstring const characters,ulong& packed)
		{
			return Code().Decode( characters, &packed );
		}
	
		Result GameGenie::Pack(const uint address,const uint data,const uint compare,const bool useCompare,ulong& packed)
		{
			if ((address < 0x8000U) || (address > 0xFFFFU) || (data > 0xFFU) || (compare > 0xFFU && useCompare))
				return RESULT_ERR_INVALID_PARAM;
	
			packed =
			(
				( dword( data              ) << NES_PACKED_SHIFT_DATA    ) | 
				( dword( compare           ) << NES_PACKED_SHIFT_COMPARE ) | 
				( dword( address - 0x8000U ) << NES_PACKED_SHIFT_ADDRESS ) |
				( useCompare ? NES_PACKED_USECOMPARE : 0 )
			);
	
			return RESULT_OK;
		}
	
		Result GameGenie::Unpack(const dword packed,uint& address,uint& data,uint& compare,bool& useCompare)
		{
			data       = (( packed & NES_PACKED_DATA       ) >> NES_PACKED_SHIFT_DATA       );
			compare    = (( packed & NES_PACKED_COMPARE    ) >> NES_PACKED_SHIFT_COMPARE    );
			address    = (( packed & NES_PACKED_ADDRESS    ) >> NES_PACKED_SHIFT_ADDRESS    ) + 0x8000U;
			useCompare = (( packed & NES_PACKED_USECOMPARE ) >> NES_PACKED_SHIFT_USECOMPARE );
	
			return RESULT_OK;
		}
	
		Result GameGenie::AddCode(const dword packed)
		{
			Code tmp;
	
			const Result result = tmp.Encode( packed );
	
			if (NES_FAILED(result))
				return result;
	
			NST_ASSERT(tmp.Address() < 0x8000U);
	
			if (tmp.Address() >= 0x8000U)
				return RESULT_ERR_INVALID_PARAM;
	
			Code& code = codes[0x8000U + tmp.Address()];
			code = tmp;
	
			Map( code, true );
	
			return RESULT_OK;
		}
	
		Result GameGenie::DeleteCode(const dword packed)
		{
			Code tmp;
	
			const Result result = tmp.Encode( packed );
	
			if (NES_FAILED(result))
				return result;
	
			NST_ASSERT(tmp.Address() < 0x8000U);
	
			if (tmp.Address() >= 0x8000U)
				return RESULT_ERR_INVALID_PARAM;
	
			Codes::iterator it( codes.find(0x8000U + tmp.Address()) );
	
			if (it != codes.end())
			{
				Map( it->second, false );
				codes.erase( it->first );
			}
	
			return RESULT_OK;
		}
	
		uint GameGenie::NumCodes() const
		{
			return codes.size();
		}
	
		dword GameGenie::GetCode(const uint index) const
		{
			Codes::const_iterator it( codes.begin() );
	
			for (uint i=0; i < index; ++i)
				++it;
	
			return it->second.Packed();
		}
	
		dword GameGenie::Code::Packed() const
		{
			ulong packed;
			GameGenie::Pack( address, data, compare, useCompare, packed );
			return packed;
		}
	
		Result GameGenie::Code::Decode(cstring const characters,ulong* const packed)
		{
			uint codes[8];
	
			switch (DecodeCharacters( characters, codes ))
			{
				case 0: Decode6( codes ); break;
				case 1: Decode8( codes ); break;
				default: return RESULT_ERR_INVALID_PARAM;
			}
	
			if (packed)
			{
				*packed = 
				(
					( dword( data    ) << NES_PACKED_SHIFT_DATA	   ) |
					( dword( compare ) << NES_PACKED_SHIFT_COMPARE ) |
					( dword( address ) << NES_PACKED_SHIFT_ADDRESS ) |
					( useCompare ? NES_PACKED_USECOMPARE : 0   )
				);
			}
	
			return RESULT_OK;
		}
	
		Result GameGenie::Code::Encode(const dword packed,char* const characters)
		{
			data       = ( packed & NES_PACKED_DATA       ) >> NES_PACKED_SHIFT_DATA;
			compare    = ( packed & NES_PACKED_COMPARE    ) >> NES_PACKED_SHIFT_COMPARE;
			address    = ( packed & NES_PACKED_ADDRESS    ) >> NES_PACKED_SHIFT_ADDRESS;
			useCompare = ( packed & NES_PACKED_USECOMPARE ) >> NES_PACKED_SHIFT_USECOMPARE;
	
			if (characters)
			{
				uint codes[8] = {0,0,0,0,0,0,0,0};
	
				switch (useCompare)
				{
					case 0:  Encode6( codes ); break;
					default: Encode8( codes ); break;
				}
	
				EncodeCharacters( codes, characters );
			}
	
			return RESULT_OK;
		}
	
		int GameGenie::Code::DecodeCharacters(cstring const characters,uint* const codes) const
		{
			NST_ASSERT( characters && codes );
	
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
					default: return -1;
				}
	
				if ((i == 2) && (codes[2] & 0x8))
					length = 8;
			}
	
			return length == 8 ? 1 : (length == 6 ? 0 : -1);
		}
	
		void GameGenie::Code::EncodeCharacters(const uint* const codes,char* characters) const
		{
			NST_ASSERT( codes && characters );
	
			const uint length = (useCompare ? 8 : 6);
	
			for (uint i=0; i < length; ++i)
			{
				switch (codes[i])
				{
					case 0x0: *characters++ = 'A'; continue;
					case 0x1: *characters++ = 'P'; continue;
					case 0x2: *characters++ = 'Z'; continue;
					case 0x3: *characters++ = 'L'; continue;
					case 0x4: *characters++ = 'G'; continue;
					case 0x5: *characters++ = 'I'; continue;
					case 0x6: *characters++ = 'T'; continue;
					case 0x7: *characters++ = 'Y'; continue;
					case 0x8: *characters++ = 'E'; continue;
					case 0x9: *characters++ = 'O'; continue;
					case 0xA: *characters++ = 'X'; continue;
					case 0xB: *characters++ = 'U'; continue;
					case 0xC: *characters++ = 'K'; continue;
					case 0xD: *characters++ = 'S'; continue;
					case 0xE: *characters++ = 'V'; continue;
					case 0xF: *characters++ = 'N'; continue;
				}
	
				NST_DEBUG_MSG("GameGenie::Code::EncodeCharacters() internal error!");
			}
	
			*characters = '\0';
		}
	
		void GameGenie::Code::DecodeAddress(const uint* const codes)
		{
			address = 
			(
				((codes[4] & 0x1) << 0x0) | 
				((codes[4] & 0x2) << 0x0) |
				((codes[4] & 0x4) << 0x0) |
				((codes[3] & 0x8) << 0x0) |
				((codes[2] & 0x1) << 0x4) |
				((codes[2] & 0x2) << 0x4) |
				((codes[2] & 0x4) << 0x4) |
				((codes[1] & 0x8) << 0x4) |
				((codes[5] & 0x1) << 0x8) |
				((codes[5] & 0x2) << 0x8) |
				((codes[5] & 0x4) << 0x8) |
				((codes[4] & 0x8) << 0x8) |
				((codes[3] & 0x1) << 0xC) |
				((codes[3] & 0x2) << 0xC) |
				((codes[3] & 0x4) << 0xC)
			);
		}
	
		void GameGenie::Code::EncodeAddress(uint* const codes) const
		{
			codes[4] |= (address & 0x0001U) >> 0x0;
			codes[4] |= (address & 0x0002U) >> 0x0;
			codes[4] |= (address & 0x0004U) >> 0x0;
			codes[3] |= (address & 0x0008U) >> 0x0;
			codes[2] |= (address & 0x0010U) >> 0x4;
			codes[2] |= (address & 0x0020U) >> 0x4;
			codes[2] |= (address & 0x0040U) >> 0x4;
			codes[1] |= (address & 0x0080U) >> 0x4;
			codes[5] |= (address & 0x0100U) >> 0x8;
			codes[5] |= (address & 0x0200U) >> 0x8;
			codes[5] |= (address & 0x0400U) >> 0x8;
			codes[4] |= (address & 0x0800U) >> 0x8;
			codes[3] |= (address & 0x1000U) >> 0xC;
			codes[3] |= (address & 0x2000U) >> 0xC;
			codes[3] |= (address & 0x4000U) >> 0xC;
		}
	
		void GameGenie::Code::Decode6(const uint* const codes)
		{
			DecodeAddress( codes );
	
			data =
			(
				((codes[0] & 0x1) << 0x0) |
				((codes[0] & 0x2) << 0x0) |
				((codes[0] & 0x4) << 0x0) |
				((codes[5] & 0x8) << 0x0) |
				((codes[1] & 0x1) << 0x4) |
				((codes[1] & 0x2) << 0x4) |
				((codes[1] & 0x4) << 0x4) |
				((codes[0] & 0x8) << 0x4) 
			);
	
			compare = 0x00;
			useCompare = 0x00;
		}
	
		void GameGenie::Code::Encode6(uint* const codes) const
		{
			EncodeAddress( codes );
	
			codes[0] |= (data & 0x01) >> 0x0;
			codes[0] |= (data & 0x02) >> 0x0;
			codes[0] |= (data & 0x04) >> 0x0;
			codes[5] |= (data & 0x08) >> 0x0;
			codes[1] |= (data & 0x10) >> 0x4;
			codes[1] |= (data & 0x20) >> 0x4;
			codes[1] |= (data & 0x40) >> 0x4;
			codes[0] |= (data & 0x80) >> 0x4;
		}
	
		void GameGenie::Code::Decode8(const uint* const codes)
		{
			DecodeAddress( codes );
	
			data =
			(
				((codes[0] & 0x1) << 0x0) |
				((codes[0] & 0x2) << 0x0) |
				((codes[0] & 0x4) << 0x0) |
				((codes[7] & 0x8) << 0x0) |
				((codes[1] & 0x1) << 0x4) |
				((codes[1] & 0x2) << 0x4) |
				((codes[1] & 0x4) << 0x4) |
				((codes[0] & 0x8) << 0x4) 
			);
	
			compare =
			(
				((codes[6] & 0x1) << 0x0) |
				((codes[6] & 0x2) << 0x0) |
				((codes[6] & 0x4) << 0x0) |
				((codes[5] & 0x8) << 0x0) |
				((codes[7] & 0x1) << 0x4) |
				((codes[7] & 0x2) << 0x4) |
				((codes[7] & 0x4) << 0x4) |
				((codes[6] & 0x8) << 0x4)  
			);
	
			useCompare = 0x01;
		}
	
		void GameGenie::Code::Encode8(uint* const codes) const
		{
			EncodeAddress( codes );
	
			codes[2] |= 0x8;
	
			codes[0] |= (data & 0x01) >> 0x0;
			codes[0] |= (data & 0x02) >> 0x0;
			codes[0] |= (data & 0x04) >> 0x0;
			codes[7] |= (data & 0x08) >> 0x0;
			codes[1] |= (data & 0x10) >> 0x4;
			codes[1] |= (data & 0x20) >> 0x4;
			codes[1] |= (data & 0x40) >> 0x4;
			codes[0] |= (data & 0x80) >> 0x4; 
	
			codes[6] |= (compare & 0x01) >> 0x0;
			codes[6] |= (compare & 0x02) >> 0x0;
			codes[6] |= (compare & 0x04) >> 0x0;
			codes[5] |= (compare & 0x08) >> 0x0;
			codes[7] |= (compare & 0x10) >> 0x4;
			codes[7] |= (compare & 0x20) >> 0x4;
			codes[7] |= (compare & 0x40) >> 0x4;
			codes[6] |= (compare & 0x80) >> 0x4;  
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		inline uint GameGenie::Code::Peek(const uint address)
		{
			NST_ASSERT( (0x8000U + this->address) == address );
	
			if (useCompare)
			{
				const uint value = port.Peek( address );
	
				if (value != compare)
					return value;
			}
	
			return data;
		}
	
		inline void GameGenie::Code::Poke(const uint address,const uint value)
		{
			NST_ASSERT( (0x8000U + this->address) == address );
			port.Poke( address, value );
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
		GameGenie::GameGenie(Cpu& c)
		: cpu(c) {}
	
		GameGenie::~GameGenie()
		{
			ClearCodes();
		}
	
		void GameGenie::ClearCodes()
		{
			for (Codes::iterator it(codes.begin()); it != codes.end(); ++it)
			{
				if (cpu.Map( it->first ).SameComponent( this ))
					cpu.Map( it->first ) = it->second.GetPort();
			}
		}
	
		void GameGenie::Reset()
		{
			for (Codes::iterator it(codes.begin()); it != codes.end(); ++it)
				Map( it->second, true );
		}
	
		void GameGenie::Map(Code& code,const bool enable)
		{
			const uint address = 0x8000U + code.Address();
	
			if (enable)
			{
				if (!cpu.Map( address ).SameComponent( this ))
				{
					code.SetPort( cpu.Map( address ) );
					cpu.Map( address ).Set( this, &GameGenie::Peek_wizard, &GameGenie::Poke_wizard );
				}
			}
			else
			{
				if (cpu.Map( address ).SameComponent( this ))
					cpu.Map( address ) = code.GetPort();
			}
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_PEEK(GameGenie,wizard)
		{
			NST_ASSERT(address >= 0x8000U);
			return codes.find( address )->second.Peek( address );
		}
	
		NES_POKE(GameGenie,wizard)
		{
			NST_ASSERT(address >= 0x8000U);
			codes.find( address )->second.Poke( address, data );
		}
	}
}
