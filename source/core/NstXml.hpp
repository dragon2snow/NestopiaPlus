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

#ifndef NST_XML_H
#define NST_XML_H

#include <cstring>
#include <iosfwd>

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Xml
		{
			class BaseNode
			{
				static wchar_t ParseReference(wcstring&,wcstring);
				static wchar_t* SetValue(wchar_t* NST_RESTRICT,wcstring,wcstring,bool);
				static wchar_t* SetType(wchar_t* NST_RESTRICT,wcstring,wcstring,bool);

			public:

				struct Attribute
				{
					Attribute(wcstring,wcstring,wcstring,wcstring,bool=true);
					~Attribute();

					wcstring const type;
					wcstring const value;
					Attribute* next;
				};

				BaseNode(wcstring,wcstring,bool=true);
				~BaseNode();

				void SetValue(wcstring,wcstring,bool=true);
				void AddAttribute(wcstring,wcstring,wcstring,wcstring,bool=true);

				wcstring const type;
				wcstring value;
				Attribute* attribute;
				BaseNode* child;
				BaseNode* sibling;
			};

			static bool IsEqual(wcstring,wcstring);
			static bool IsEqualNonCase(wcstring,wcstring);
			static long ToSigned(wcstring,uint,wcstring*);
			static ulong ToUnsigned(wcstring,uint,wcstring*);

		public:

			Xml();
			~Xml();

			class Node;

			class Attribute : public ImplicitBool<Attribute>
			{
			public:

				long GetSignedValue(uint=10) const;
				long GetSignedValue(uint,wcstring&) const;

				ulong GetUnsignedValue(uint=10) const;
				ulong GetUnsignedValue(uint,wcstring&) const;

			private:

				friend class Node;

				BaseNode::Attribute* attribute;

				Attribute(BaseNode::Attribute* a)
				: attribute(a) {}

			public:

				bool operator ! () const
				{
					return !attribute;
				}

				wcstring GetType() const
				{
					return attribute ? attribute->type : L"";
				}

				wcstring GetValue() const
				{
					return attribute ? attribute->value : L"";
				}

				Attribute GetNext() const
				{
					return attribute ? attribute->next : NULL;
				}

				bool IsType(wcstring type) const
				{
					return IsEqual( GetType(), type ? type : L"" );
				}

				bool IsValue(wcstring value) const
				{
					return IsEqualNonCase( GetValue(), value ? value : L"" );
				}
			};

			class Node : public ImplicitBool<Node>
			{
				friend class Xml;

			public:

				dword NumChildren(wcstring=NULL) const;
				dword NumAttributes() const;

				Attribute GetAttribute(dword) const;
				Attribute GetAttribute(wcstring) const;

				Node GetChild(dword) const;
				Node GetChild(wcstring) const;

				Node AddChild(wcstring,wcstring=NULL);
				Node AddSibling(wcstring,wcstring=NULL);
				Attribute AddAttribute(wcstring,wcstring);

				long GetSignedValue(uint=10) const;
				long GetSignedValue(uint,wcstring&) const;

				ulong GetUnsignedValue(uint=10) const;
				ulong GetUnsignedValue(uint,wcstring&) const;

			private:

				BaseNode* Add(wcstring,wcstring,BaseNode**) const;

				BaseNode* node;

				Node(BaseNode* n)
				: node(n) {}

			public:

				Node()
				: node(NULL) {}

				bool operator ! () const
				{
					return !node;
				}

				bool HasChildren() const
				{
					return node && node->child;
				}

				bool HasNextSibling() const
				{
					return node && node->sibling;
				}

				bool HasAttributes() const
				{
					return node && node->attribute;
				}

				Attribute GetFirstAttribute() const
				{
					return node ? node->attribute : NULL;
				}

				Node GetFirstChild() const
				{
					return node ? node->child : NULL;
				}

				Node GetNextSibling() const
				{
					return node ? node->sibling : NULL;
				}

				wcstring GetType() const
				{
					return node ? node->type : L"";
				}

				wcstring GetValue() const
				{
					return node ? node->value : L"";
				}

				bool IsType(wcstring type) const
				{
					return IsEqual( GetType(), type ? type : L"" );
				}

				bool IsValue(wcstring value) const
				{
					return IsEqualNonCase( GetValue(), value ? value : L"" );
				}
			};

			struct Format
			{
				Format();

				cstring tab;
				cstring newline;
				cstring valueDelimiter;
				bool xmlHeader;
				bool byteOrderMark;
			};

			Node Create(wcstring);
			Node Read(wcstring);
			Node Read(std::istream&);
			void Write(Node,std::ostream&,const Format& = Format()) const;
			void Destroy();

		private:

			enum Tag
			{
				TAG_XML,
				TAG_COMMENT,
				TAG_INSTRUCTION,
				TAG_OPEN,
				TAG_OPEN_CLOSE,
				TAG_CLOSE
			};

			class Input
			{
				const char* const stream;
				const dword size;
				dword pos;

				static char* Init(std::istream&,dword&);
				static inline wchar_t WordToWchar(dword);

			public:

				Input(std::istream&,dword=0);
				~Input();

				inline dword Size() const;

				inline byte ToByte(dword) const;
				inline wchar_t FromUTF16LE(dword) const;
				inline wchar_t FromUTF16BE(dword) const;
				inline wchar_t FromChar(dword) const;

				wchar_t ReadUTF8();
				inline void SetReadPointer(dword);
			};

			class Output
			{
				std::ostream& stream;

				void Write(cstring,uint);

			public:

				const Format format;

				Output(std::ostream&,const Format&);

				class Type
				{
					friend class Output;
					wcstring string;

				public:

					inline Type(wcstring);
				};

				class Value
				{
					friend class Output;
					wcstring string;

				public:

					inline Value(wcstring);
				};

				const Output& operator << (char) const;
				const Output& operator << (uchar) const;
				const Output& operator << (wchar_t) const;
				const Output& operator << (Type) const;
				const Output& operator << (Value) const;

				inline const Output& operator << (cstring) const;

				template<uint N>
				inline const Output& operator << (const char (&)[N]) const;
			};

			static bool IsVoid(wchar_t);
			static Tag CheckTag(wcstring);

			static wcstring SkipVoid(wcstring);
			static wcstring RewindVoid(wcstring,wcstring=NULL);
			static wcstring ReadTag(wcstring,BaseNode*&);
			static wcstring ReadValue(wcstring,BaseNode&);
			static wcstring ReadNode(wcstring,Tag,BaseNode*&);
			static void WriteNode(Node,const Output&,uint);

			BaseNode* root;

		public:

			Node GetRoot() const
			{
				return root;
			}
		};
	}
}

#endif
