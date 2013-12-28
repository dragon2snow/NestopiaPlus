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
#include "../NstMachine.hpp"
#include "../NstImage.hpp"
#include "../NstState.hpp"
#include "NstApiMachine.hpp"

namespace Nes
{
	namespace Api
	{
		uint Machine::Is(uint a) const throw()
		{
			return emulator.Is( a );
		}

		bool Machine::Is(uint a,uint b) const throw()
		{
			return emulator.Is( a, b );
		}

		bool Machine::IsLocked() const throw()
		{
			return emulator.tracker.IsLocked();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Result Machine::Load(std::istream& stream,uint type)
		{
			Result result;

			const bool on = Is(ON);

			try
			{
				result = emulator.Load( &stream, type );
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

			if (on)
				Power( true );

			return result;
		}

		Result Machine::Load(std::istream& stream) throw()
		{
			return Load( stream, Core::Image::UNKNOWN );
		}

		Result Machine::LoadCartridge(std::istream& stream) throw()
		{
			return Load( stream, Core::Image::CARTRIDGE );
		}

		Result Machine::LoadDisk(std::istream& stream) throw()
		{
			return Load( stream, Core::Image::DISK );
		}

		Result Machine::LoadSound(std::istream& stream) throw()
		{
			return Load( stream, Core::Image::SOUND );
		}

		Result Machine::Unload() throw()
		{
			if (!Is(IMAGE))
				return RESULT_NOP;

			if (emulator.Unload())
				return RESULT_OK;
			else
				return RESULT_WARN_SAVEDATA_LOST;
		}

		Result Machine::Power(const bool on) throw()
		{
			if (on == bool(Is(ON)))
				return RESULT_NOP;

			if (on)
			{
				try
				{
					emulator.Reset( true );
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
			else
			{
				if (!emulator.PowerOff())
					return RESULT_WARN_SAVEDATA_LOST;
			}

			return RESULT_OK;
		}

		Result Machine::Reset(const bool hard) throw()
		{
			if (!Is(ON) || IsLocked())
				return RESULT_ERR_NOT_READY;

			try
			{
				emulator.Reset( hard );
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

			return RESULT_OK;
		}

		Machine::Mode Machine::GetMode() const throw()
		{
			return static_cast<Mode>(Is(NTSC|PAL));
		}

		Machine::Mode Machine::GetDesiredMode() const throw()
		{
			return (!emulator.image || emulator.image->GetMode() == Core::MODE_NTSC) ? NTSC : PAL;
		}

		Result Machine::SetMode(const Mode mode) throw()
		{
			if (mode == GetMode())
				return RESULT_NOP;

			emulator.SetMode( mode == PAL ? Core::MODE_PAL : Core::MODE_NTSC );

			return RESULT_OK;
		}

		Result Machine::LoadState(std::istream& stream) throw()
		{
			if (!Is(GAME,ON) || IsLocked())
				return RESULT_ERR_NOT_READY;

			try
			{
				emulator.tracker.Flush();
				Core::State::Loader loader( &stream, true );

				if (emulator.LoadState( loader ))
					return RESULT_OK;
				else
					return RESULT_ERR_INVALID_CRC;
			}
			catch (Result result)
			{
				return result;
			}
			catch (std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		Result Machine::SaveState(std::ostream& stream,Compression compression) const throw()
		{
			if (!Is(GAME,ON))
				return RESULT_ERR_NOT_READY;

			try
			{
				Core::State::Saver saver( &stream, compression != NO_COMPRESSION, false );
				emulator.SaveState( saver );
			}
			catch (Result result)
			{
				return result;
			}
			catch (std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
