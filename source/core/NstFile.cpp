////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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
#include <new>
#include "NstStream.hpp"
#include "NstVector.hpp"
#include "NstChecksum.hpp"
#include "NstFile.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		File::File()
		: checksum( *new Checksum )
		{
		}

		File::~File()
		{
			delete &checksum;
		}

		void File::Load(const byte* data,dword size) const
		{
			NST_ASSERT( data && size );

			checksum.Clear();
			checksum.Compute( data, size );
		}

		void File::Load(Type type,byte* data,dword size) const
		{
			const LoadBlock loadBlock = {data,size};
			Load( type, &loadBlock, 1 );
		}

		void File::Load(const Type type,const LoadBlock* const loadBlock,const uint loadBlockCount) const
		{
			class Loader : public Api::User::File
			{
				const Action action;
				const LoadBlock* const loadBlock;
				const uint loadBlockCount;

				Action GetAction() const throw()
				{
					return action;
				}

				ulong GetMaxSize() const throw()
				{
					dword maxsize = 0;

					for (const LoadBlock* NST_RESTRICT it=loadBlock, *const end=loadBlock+loadBlockCount; it != end; ++it)
						maxsize += it->size;

					return maxsize;
				}

				Result SetContent(const void* data,ulong filesize) throw()
				{
					if (!data || !filesize)
						return RESULT_ERR_INVALID_PARAM;

					const byte* NST_RESTRICT filedata = static_cast<const byte*>(data);

					for (const LoadBlock* NST_RESTRICT it=loadBlock, *const end=loadBlock+loadBlockCount; it != end; ++it)
					{
						if (const dword size = NST_MIN(filesize,it->size))
						{
							std::memcpy( it->data, filedata, size );
							filedata += size;
							filesize -= size;
						}
					}

					return RESULT_OK;
				}

				Result SetContent(std::istream& stdStream) throw()
				{
					try
					{
						Stream::In stream( &stdStream );

						if (ulong length = stream.Length())
						{
							for (const LoadBlock* NST_RESTRICT it=loadBlock, *const end=loadBlock+loadBlockCount; it != end; ++it)
							{
								if (const dword size = NST_MIN(length,it->size))
								{
									stream.Read( it->data, size );
									length -= size;
								}
							}
						}
						else
						{
							return RESULT_ERR_INVALID_PARAM;
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

					return RESULT_OK;
				}

			public:

				Loader(Type t,const LoadBlock* l,uint c)
				:
				action
				(
					t == EEPROM    ? LOAD_EEPROM :
					t == TAPE      ? LOAD_TAPE :
					t == TURBOFILE ? LOAD_TURBOFILE :
                                     LOAD_BATTERY
				),
				loadBlock(l),
				loadBlockCount(c)
				{
				}
			};

			{
				Loader loader( type, loadBlock, loadBlockCount );
				Api::User::fileIoCallback( loader );
			}

			checksum.Clear();

			for (const LoadBlock* NST_RESTRICT it=loadBlock, *const end=loadBlock+loadBlockCount; it != end; ++it)
				checksum.Compute( it->data, it->size );
		}

		void File::Load(const Type type,Vector<byte>& buffer,const dword maxsize) const
		{
			NST_ASSERT( maxsize && type != DISK );

			class Loader : public Api::User::File
			{
				const Action action;
				Vector<byte>& buffer;
				const dword maxsize;

				Action GetAction() const throw()
				{
					return action;
				}

				ulong GetMaxSize() const throw()
				{
					return maxsize;
				}

				Result SetContent(const void* filedata,ulong filesize) throw()
				{
					if (!filedata || !filesize)
						return RESULT_ERR_INVALID_PARAM;

					try
					{
						buffer.Assign( static_cast<const byte*>(filedata), NST_MIN(filesize,maxsize) );
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

				Result SetContent(std::istream& stdStream) throw()
				{
					try
					{
						Stream::In stream( &stdStream );

						if (const ulong length = stream.Length())
						{
							buffer.Resize( NST_MIN(length,maxsize) );

							try
							{
								stream.Read( buffer.Begin(), buffer.Size() );
							}
							catch (...)
							{
								buffer.Destroy();
								throw;
							}
						}
						else
						{
							return RESULT_ERR_INVALID_PARAM;
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

					return RESULT_OK;
				}

			public:

				Loader(Type t,Vector<byte>& b,dword m)
				:
				action
				(
					t == EEPROM    ? LOAD_EEPROM :
					t == TAPE      ? LOAD_TAPE :
					t == TURBOFILE ? LOAD_TURBOFILE :
                                     LOAD_BATTERY
				),
				buffer  (b),
				maxsize (m)
				{
				}
			};

			{
				Loader loader( type, buffer, maxsize );
				Api::User::fileIoCallback( loader );
			}

			if (buffer.Size())
				Load( buffer.Begin(), buffer.Size() );
		}

		void File::Save(Type type,const byte* data,dword size) const
		{
			const SaveBlock saveBlock = {data,size};
			Save( type, &saveBlock, 1 );
		}

		void File::Save(const Type type,const SaveBlock* const saveBlock,const uint saveBlockCount) const
		{
			NST_ASSERT( saveBlock && saveBlockCount );

			Checksum recentChecksum;

			for (const SaveBlock *NST_RESTRICT it=saveBlock, *const end=saveBlock+saveBlockCount; it != end; ++it)
				recentChecksum.Compute( it->data, it->size );

			if (recentChecksum != checksum)
			{
				class Saver : public Api::User::File
				{
					const Action action;
					const SaveBlock* const saveBlock;
					const uint saveBlockCount;
					mutable Vector<byte> buffer;

					Action GetAction() const throw()
					{
						return action;
					}

					ulong GetMaxSize() const throw()
					{
						dword size = 0;

						for (const SaveBlock* NST_RESTRICT it=saveBlock, *const end=saveBlock+saveBlockCount; it != end; ++it)
							size += it->size;

						return size;
					}

					Result GetContent(const void*& filedata,ulong& filesize) const throw()
					{
						if (saveBlockCount <= 1)
						{
							filedata = saveBlock->data;
							filesize = saveBlock->size;
						}
						else
						{
							if (!buffer.Size())
							{
								try
								{
									buffer.Resize( Saver::GetMaxSize() );
								}
								catch (...)
								{
									filedata = NULL;
									filesize = 0;
									return RESULT_ERR_OUT_OF_MEMORY;
								}

								dword offset = 0;

								for (const SaveBlock* NST_RESTRICT it=saveBlock, *const end=saveBlock+saveBlockCount; it != end; ++it)
								{
									std::memcpy( &buffer[offset], it->data, it->size );
									offset += it->size;
								}
							}

							filedata = buffer.Begin();
							filesize = buffer.Size();
						}

						return RESULT_OK;
					}

					Result GetContent(std::ostream& stdStream) const throw()
					{
						try
						{
							Stream::Out stream( &stdStream );

							for (const SaveBlock* NST_RESTRICT it=saveBlock, *const end=saveBlock+saveBlockCount; it != end; ++it)
							{
								if (it->size)
									stream.Write( it->data, it->size );
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

						return RESULT_OK;
					}

				public:

					Saver(Type t,const SaveBlock* s,uint c)
					:
					action
					(
						t == EEPROM    ? SAVE_EEPROM :
						t == TAPE      ? SAVE_TAPE :
						t == TURBOFILE ? SAVE_TURBOFILE :
                                         SAVE_BATTERY
					),
					saveBlock      (s),
					saveBlockCount (c)
					{
					}
				};

				Saver saver( type, saveBlock, saveBlockCount );
				Api::User::fileIoCallback( saver );
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
