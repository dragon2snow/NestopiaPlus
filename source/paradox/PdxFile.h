//////////////////////////////////////////////////////////////////////////////////////////////
//
// Paradox Library - general purpose C++ utilities
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Paradox Library.
// 
// Paradox Library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Paradox Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Paradox Library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PDX_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#ifndef PDXFILE_H
#define PDXFILE_H

#include "PdxString.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// Binary File class
//////////////////////////////////////////////////////////////////////////////////////////////

class PDXFILE : PDX_NON_COPYABLE_CLASS
{
public:

	typedef CHAR VALUE;
	typedef PDXARRAY<CHAR> BUFFER;
	typedef BUFFER::ITERATOR ITERATOR;
	typedef BUFFER::CONSTITERATOR CONSTITERATOR;

	enum MODE {INPUT,OUTPUT,APPEND};
	enum POSITION {BEGIN,CURRENT,END};

	explicit PDXFILE(const MODE=APPEND);

	PDXFILE(const CHAR* const,const MODE=APPEND);	
	PDXFILE(const PDXSTRING&,const MODE=APPEND);	

	~PDXFILE();

	PDXRESULT Open(const CHAR* const,const MODE);
	PDXRESULT Open(const CHAR* const);
	PDXRESULT Open(const MODE);
	PDXRESULT Open();
	PDXRESULT Close();
	PDXRESULT Flush();
	PDXRESULT ChangeName(const CHAR* const);

	PDXRESULT Open(const PDXSTRING& s,const MODE m)
	{ return Open(s.String(),m); }

	PDXRESULT Open(const PDXSTRING& s)
	{ return Open(s.String()); }

	template<class T> 
	inline PDXFILE& operator << (const T& t)
	{ Write(t); return *this; }

	template<class T>
	friend inline T& operator << (T& t,PDXFILE& file)
	{ file.Read(t); return t; }

	template<class T> 
	inline T& operator >> (T& t)
	{ Read(t); return t; }

	template<class T> T Read();
	template<class T> T Peek();

	template<class T> BOOL Read(T&);
	template<class T> BOOL Peek(T&);

	template<class T> VOID Write(const T&);

	template<class I,class J> PDX_NO_INLINE BOOL Read(I,const J&);
	template<class I,class J> PDX_NO_INLINE VOID Write(I,const J&);

	inline TSIZE Position() const
	{ return pos; }

	inline const PDXSTRING& Name() const
	{ return name; }

	const PDXSTRING& FileName();
	const PDXSTRING& FilePath();
	const PDXSTRING& FileExtension();
	
	TSIZE Seek(const POSITION,const LONG=0);

	inline BOOL Readable(const TSIZE length) const
	{ return (pos + length) <= buffer.Size(); }

    inline BOOL IsOpen() const 
	{ return open; }

	inline MODE Mode() const
	{ return mode; }

	inline TSIZE Size() const
	{ return buffer.Size(); }

	inline BOOL IsEmpty() const
	{ return buffer.Size() == 0; }

	VOID Abort()
	{ open=FALSE; Close(); }

	inline BOOL Eof() const
	{ return pos == buffer.Size(); }

	inline VOID Defrag()
	{ buffer.Defrag(); }

	inline ITERATOR Begin()						       { return buffer.Begin(); }
	inline CONSTITERATOR Begin() const                 { return buffer.Begin(); }

	inline ITERATOR At(const TSIZE i)                  { return buffer.At(i); }
	inline CONSTITERATOR At(const TSIZE i) const       { return buffer.At(i); }

	inline ITERATOR Offset(const TSIZE i=0)            { return buffer.At(pos+i); }
	inline CONSTITERATOR Offset(const TSIZE i=0) const { return buffer.At(pos+i); }

	inline ITERATOR End()                              { return buffer.End(); }
	inline CONSTITERATOR end() const                   { return buffer.End(); }

	class TEXTPROXY
	{
	public:

		friend class PDXFILE;

		PDXSTRING& Read(PDXSTRING&);
		const CHAR* Read(const CHAR*&);
		const CHAR* Read();

		VOID Write(const PDXSTRING& s)
		{ Write(s.Begin(),s.End()); }

		VOID Write(const CHAR* const s)
		{ PDX_ASSERT(s); Write(s,&s[strlen(s)]); }

		VOID Write(const INT a,  const UINT b=PDXSTRING::DEC) { Write(PDXSTRING(a,b)); }
		VOID Write(const UINT a, const UINT b=PDXSTRING::DEC) { Write(PDXSTRING(a,b)); }
		VOID Write(const LONG a, const UINT b=PDXSTRING::DEC) { Write(PDXSTRING(a,b)); }
		VOID Write(const ULONG a,const UINT b=PDXSTRING::DEC) { Write(PDXSTRING(a,b)); }
		VOID Write(const FLOAT a,const UINT b=PDXSTRING::DIG) { Write(PDXSTRING(a,b)); }

		VOID Write(const CHAR* const,const CHAR* const);

		template<class T> 
		inline TEXTPROXY& operator << (const T& t)
		{ Write(t); return *this; }

		template<class T>
		friend inline T& operator << (T& t,TEXTPROXY& file)
		{ file.Read(t); return t; }

		template<class T> 
		inline T& operator >> (T& t)
		{ Read(t); return t; }

	private:

		inline TEXTPROXY(PDXFILE* const f)
		: file(*f) {}

		PDXFILE& file;
	};

	friend class TEXTPROXY;

	inline TEXTPROXY Text()
	{ return TEXTPROXY(this); }

	inline BUFFER& Buffer()
	{ return buffer; }

	VOID Hook(BUFFER&,const MODE=INPUT,const CHAR* const=NULL);
	VOID UnHook();

private:

    VOID Reserve(const TSIZE);
	VOID Grow(const TSIZE);
	
	PDXRESULT ReadBuffer();
	PDXRESULT WriteBuffer() const;

	enum {CACHE=4096};

	BUFFER buffer;
	TSIZE  pos;
	MODE   mode;
	BOOL   open;

	PDXSTRING name;
	PDXSTRING filename;
	PDXSTRING filepath;
	PDXSTRING fileextension;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Read a range of values
//////////////////////////////////////////////////////////////////////////////////////////////

template<class I,class J> 
BOOL PDXFILE::Read(I first,const J& last)
{
	PDX_ASSERT(open);

	if (pos + ((last-first) * sizeof(*first)) > buffer.Size())
		return FALSE;

	for (I iterator(first); iterator != last; ++iterator, pos += sizeof(*iterator))
		memcpy(&(*iterator),buffer.At(pos),sizeof(*iterator));

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Write a range of values
//////////////////////////////////////////////////////////////////////////////////////////////

template<class I,class J> 
VOID PDXFILE::Write(I first,const J& last)
{
	PDX_ASSERT(open && (mode==OUTPUT||mode==APPEND));

    Grow((last-first) * sizeof(*first));

	for (; first != last; ++first, pos += sizeof(*first))
		memcpy(buffer.At(pos),&(*first),sizeof(*first));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Read any type
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
T PDXFILE::Read()
{
	PDX_ASSERT(open);
	PDX_ASSERT_MSG(pos + sizeof(T) <= buffer.Size(),"File size overflow!");
	CHAR value[sizeof(T)];
	memcpy(value,buffer.At(pos),sizeof(T));
	pos += sizeof(T);
	return *PDX_CAST(T*,value);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Peek any type
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
T PDXFILE::Peek()
{
	PDX_ASSERT(open);
	PDX_ASSERT_MSG(pos + sizeof(T) <= buffer.Size(),"File size overflow!");
	CHAR value[sizeof(T)];
	memcpy(value,buffer.At(pos),sizeof(T));
	return *PDX_CAST(T*,value);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Read any type
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
BOOL PDXFILE::Read(T& value)
{
	PDX_ASSERT(open);

	if (pos + sizeof(T) > buffer.Size())
		return FALSE;

	memcpy(&value,buffer.At(pos),sizeof(T));
	pos += sizeof(T);
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Read any type without advancing the position
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
BOOL PDXFILE::Peek(T& value)
{
	PDX_ASSERT(open);
	
	if (pos + sizeof(T) > buffer.Size(),"File size overflow!")
		return FALSE;

	memcpy(&value,buffer.At(pos),sizeof(T));
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Write any type
//////////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID PDXFILE::Write(const T& value)
{
	PDX_ASSERT(open && (mode==OUTPUT||mode==APPEND));
	Grow(sizeof(T));
	memcpy(buffer.At(pos),&value,sizeof(T));
	pos += sizeof(T);
}

#include "PdxFile.inl"

#endif

