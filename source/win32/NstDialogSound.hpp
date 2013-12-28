////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#ifndef NST_DIALOG_SOUND_H
#define NST_DIALOG_SOUND_H

#pragma once

#include "NstCollectionBitSet.hpp"
#include "NstWindowDialog.hpp"
#include "NstDirectSound.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Sound
		{
			typedef DirectX::DirectSound::Adapter Adapter;
			typedef DirectX::DirectSound::Adapters Adapters;

		public:

			Sound(const Adapters&,const Managers::Paths&,const Configuration&);
			~Sound();

			void Save(Configuration&) const;

			enum
			{
				VOLUME_MAX = DirectX::DirectSound::VOLUME_MAX,
				LATENCY_MAX = DirectX::DirectSound::LATENCY_MAX
			};

			enum
			{
				DEFAULT_BITS = 16,
				DEFAULT_RATE = 44100,
				DEFAULT_LATENCY = 2,
				DEFAULT_VOLUME = VOLUME_MAX,
				DEFAULT_MONO = 0
			};

		private:

			enum
			{
				CHANNEL_SQUARE1, 
				CHANNEL_SQUARE2, 
				CHANNEL_TRIANGLE,
				CHANNEL_NOISE,   
				CHANNEL_DPCM,    
				CHANNEL_EXTERNAL
			};

			struct Handlers;

			struct Settings
			{
				Settings();

				ibool enabled;
				uint adapter;
				uint volume;
				uint rate;
				uint bits;
				uint latency;
				ibool stereo;
				ibool pitch;
				Collection::BitSet channels;
			};

			uint GetDefaultAdapter() const;

			ibool OnInitDialog (Param&);
			ibool OnDestroy    (Param&);
			ibool OnCmdEnable  (Param&);
			ibool OnCmdBits    (Param&);
			ibool OnCmdOutput  (Param&);
			ibool OnCmdDefault (Param&);
			ibool OnCmdOk      (Param&);

			void Enable(ibool);

			const Adapters& adapters;
			Settings settings;
			Dialog dialog;

			struct ChannelLut
			{
				cstring cfg;
				ushort ctrl;
				ushort index;
			};

			static const ChannelLut channelLut[];

		public:

			class Recorder
			{
			public:

				explicit Recorder(const Managers::Paths&);

			private:

				struct Handlers;

				ibool OnInitDialog (Param&);
				ibool OnCmdBrowse  (Param&);
				ibool OnCmdClear   (Param&);
				ibool OnCmdOk      (Param&);
				ibool OnCmdCancel  (Param&);

				Dialog dialog;
				const Managers::Paths& paths;
				Path waveFile;

			public:

				void Open()
				{
					dialog.Open();
				}

				const Path& WaveFile() const
				{
					return waveFile;
				}
			};

		private:

			Recorder recorder;

		public:

			void Open()
			{
				dialog.Open();
			}

			Recorder& GetRecorder()
			{
				return recorder;
			}

			ibool IsSoundEnabled() const
			{
				return settings.enabled;
			}

			uint GetAdapter() const
			{
				return settings.adapter;
			}

			uint GetVolume() const
			{
				return settings.volume;
			}

			uint GetRate() const
			{
				return settings.rate;
			}

			uint GetBits() const
			{
				return settings.bits;
			}

			uint GetLatency() const
			{
				return settings.latency;
			}

			uint GetChannels() const
			{
				return settings.channels.Word();
			}

			ibool IsStereo() const
			{
				return settings.stereo;
			}

			ibool IsPitchAdjust() const
			{
				return settings.pitch;
			}
		};
	}
}

#endif
