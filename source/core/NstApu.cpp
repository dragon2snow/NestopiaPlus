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
#include "NstCpu.hpp"
#include "NstState.hpp"
#include "api/NstApiSound.hpp"
#include "NstSoundRenderer.inl"

namespace Nes
{
	namespace Core
	{
		const dword Apu::Cycles::frame[2] =
		{
			Cpu::MC_DIV_NTSC * 29830UL,
			Cpu::MC_DIV_PAL * 33254UL
		};

		const dword Apu::Cycles::frameClocks[2][2][4] =
		{
			{
				{
					Cpu::MC_DIV_NTSC * (7459UL - 1),
					Cpu::MC_DIV_NTSC * 7456UL,
					Cpu::MC_DIV_NTSC * 7458UL,
					Cpu::MC_DIV_NTSC * 7458UL
				},
				{
					Cpu::MC_DIV_NTSC * 7458UL,
					Cpu::MC_DIV_NTSC * 7456UL,
					Cpu::MC_DIV_NTSC * 7458UL,
					Cpu::MC_DIV_NTSC * (7458UL + 7452)
				}
			},
			{
				{
					Cpu::MC_DIV_PAL * (8315UL - 1),
					Cpu::MC_DIV_PAL * 8314UL,
					Cpu::MC_DIV_PAL * 8312UL,
					Cpu::MC_DIV_PAL * 8314UL
				},
				{
					Cpu::MC_DIV_PAL * 8314UL,
					Cpu::MC_DIV_PAL * 8314UL,
					Cpu::MC_DIV_PAL * 8312UL,
					Cpu::MC_DIV_PAL * (8314UL + 8312)
				}
			}
		};

		const byte Apu::LengthCounter::lut[32] =
		{
			0x0A, 0xFE, 0x14, 0x02,
			0x28, 0x04, 0x50, 0x06,
			0xA0, 0x08, 0x3C, 0x0A,
			0x0E, 0x0C, 0x1A, 0x0E,
			0x0C, 0x10, 0x18, 0x12,
			0x30, 0x14, 0x60, 0x16,
			0xC0, 0x18, 0x48, 0x1A,
			0x10, 0x1C, 0x20, 0x1E
		};

		const word Apu::Noise::lut[2][16] =
		{
			{
				0x004, 0x008, 0x010, 0x020,
				0x040, 0x060, 0x080, 0x0A0,
				0x0CA, 0x0FE, 0x17C, 0x1FC,
				0x2FA, 0x3F8, 0x7F2, 0xFE4
			},
			{
				0x004, 0x007, 0x00E, 0x01E,
				0x03C, 0x058, 0x076, 0x094,
				0x0BC, 0x0EC, 0x162, 0x1D8,
				0x2C4, 0x3B0, 0x762, 0xEC2
			}
		};

		const word Apu::Dmc::lut[2][16] =
		{
			{
				0x1AC * Cpu::MC_DIV_NTSC,
				0x17C * Cpu::MC_DIV_NTSC,
				0x154 * Cpu::MC_DIV_NTSC,
				0x140 * Cpu::MC_DIV_NTSC,
				0x11E * Cpu::MC_DIV_NTSC,
				0x0FE * Cpu::MC_DIV_NTSC,
				0x0E2 * Cpu::MC_DIV_NTSC,
				0x0D6 * Cpu::MC_DIV_NTSC,
				0x0BE * Cpu::MC_DIV_NTSC,
				0x0A0 * Cpu::MC_DIV_NTSC,
				0x08E * Cpu::MC_DIV_NTSC,
				0x080 * Cpu::MC_DIV_NTSC,
				0x06A * Cpu::MC_DIV_NTSC,
				0x054 * Cpu::MC_DIV_NTSC,
				0x048 * Cpu::MC_DIV_NTSC,
				0x036 * Cpu::MC_DIV_NTSC
			},
			{
				0x18E * Cpu::MC_DIV_PAL,
				0x162 * Cpu::MC_DIV_PAL,
				0x13C * Cpu::MC_DIV_PAL,
				0x12A * Cpu::MC_DIV_PAL,
				0x114 * Cpu::MC_DIV_PAL,
				0x0EC * Cpu::MC_DIV_PAL,
				0x0D2 * Cpu::MC_DIV_PAL,
				0x0C6 * Cpu::MC_DIV_PAL,
				0x0B0 * Cpu::MC_DIV_PAL,
				0x094 * Cpu::MC_DIV_PAL,
				0x084 * Cpu::MC_DIV_PAL,
				0x076 * Cpu::MC_DIV_PAL,
				0x062 * Cpu::MC_DIV_PAL,
				0x04E * Cpu::MC_DIV_PAL,
				0x042 * Cpu::MC_DIV_PAL,
				0x032 * Cpu::MC_DIV_PAL
			}
		};

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Apu::Context::Context()
		: rate(44100), bits(16), speed(0), transpose(false), stereo(false), audible(true)
		{
			for (uint i=0; i < MAX_CHANNELS; ++i)
				volumes[i] = DEFAULT_VOLUME;
		}

		Apu::Cycles::Cycles()
		: fixed(1), rateCounter(0), frameCounter(0), extCounter(Cpu::CYCLE_MAX)
		{
			Update( 44100, 0, MODE_NTSC );
			Reset( MODE_NTSC );
		}

		void Apu::Cycles::Update(dword sampleRate,const uint speed,const Mode mode)
		{
			frameCounter /= fixed;
			rateCounter /= fixed;

			if (extCounter != Cpu::CYCLE_MAX)
				extCounter /= fixed;

			uint multiplier = 0;

			if (mode == MODE_NTSC)
			{
				if (speed)
					sampleRate = sampleRate * FPS_NTSC / speed;

				while (++multiplier < 512 && qword(Cpu::MC_NTSC) * multiplier % sampleRate);

				rate = qword(Cpu::MC_NTSC) * multiplier / sampleRate;
				fixed = Cpu::CLK_NTSC_DIV * multiplier;
			}
			else
			{
				if (speed)
					sampleRate = sampleRate * FPS_PAL / speed;

				while (++multiplier < 512 && qword(Cpu::MC_PAL) * multiplier % sampleRate);

				rate = qword(Cpu::MC_PAL) * multiplier / sampleRate;
				fixed = Cpu::CLK_PAL_DIV * multiplier;
			}

			frameCounter *= fixed;
			rateCounter *= fixed;

			if (extCounter != Cpu::CYCLE_MAX)
				extCounter *= fixed;
		}

		void Apu::Cycles::Reset(const Mode mode)
		{
			rateCounter = 0;
			frameDivider = 0;
			frameIrqClock = Cpu::CYCLE_MAX;
			frameIrqRepeat = 0;
			extCounter = Cpu::CYCLE_MAX;
			dmcClock = Dmc::GetResetFrequency( mode );
			frameCounter = frame[mode] * fixed;
		}

		void Apu::DcBlocker::Reset()
		{
			acc = 0;
			prev = 0;
			next = 0;
		}

		void Apu::Envelope::Reset()
		{
			output = 0;
			reset = false;
			reg = DECAY_DISABLE;
			count = 1;
			volume = 0x0;
		}

		Apu::Dmc::Dmc()
		: curSample(0), linSample(0), outputVolume(0) {}

		Apu::Apu(Cpu& c)
		:
		stream     (NULL),
		cpu        (c),
		mode       (MODE_NTSC),
		extChannel (NULL),
		buffer     (16),
		padding    (0)
		{
			UpdateSettings();
		}

		void Apu::Reset(const bool hard)
		{
			cpu.Map( 0x4000 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4000 );
			cpu.Map( 0x4001 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4001 );
			cpu.Map( 0x4002 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4002 );
			cpu.Map( 0x4003 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4003 );
			cpu.Map( 0x4004 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4000 );
			cpu.Map( 0x4005 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4001 );
			cpu.Map( 0x4006 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4002 );
			cpu.Map( 0x4007 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4003 );
			cpu.Map( 0x4008 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4008 );
			cpu.Map( 0x400A ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_400A );
			cpu.Map( 0x400B ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_400B );
			cpu.Map( 0x400C ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_400C );
			cpu.Map( 0x400E ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_400E );
			cpu.Map( 0x400F ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_400F );
			cpu.Map( 0x4010 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4010 );
			cpu.Map( 0x4011 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4011 );
			cpu.Map( 0x4012 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4012 );
			cpu.Map( 0x4013 ).Set( this, &Apu::Peek_4xxx, &Apu::Poke_4013 );
			cpu.Map( 0x4015 ).Set( this, &Apu::Peek_4015, &Apu::Poke_4015 );

			cycles.Reset( mode );
			dcBlocker.Reset();
			buffer.Reset( context.bits );

			if (hard)
				ctrl = STATUS_FRAME_IRQ_ENABLE;

			if (ctrl == STATUS_FRAME_IRQ_ENABLE)
				cycles.frameIrqClock = (cycles.frameCounter / cycles.fixed) - (mode == MODE_NTSC ? Cpu::MC_DIV_NTSC * 1 : Cpu::MC_DIV_PAL * 1);

			square[0].Reset();
			square[1].Reset();
			triangle.Reset();
			noise.Reset();
			dmc.Reset( cpu );

			if (extChannel)
				extChannel->Reset();

			BeginFrame( NULL );
		}

		void Apu::HookChannel(Channel* const channel)
		{
			NST_ASSERT( !extChannel );

			extChannel = channel;

			Cycle rate, fixed;
			CalculateOscillatorClock( rate, fixed );

			extChannel->SetContext( rate, fixed, mode, context.volumes );
		}

		void Apu::SetMode(const Mode m)
		{
			if (mode != m)
			{
				mode = m;
				UpdateSettings();
			}
		}

		Result Apu::SetSampleRate(const dword rate)
		{
			if (context.rate == rate)
				return RESULT_NOP;

			if (!rate)
				return RESULT_ERR_INVALID_PARAM;

			context.rate = rate;
			UpdateSettings();

			return RESULT_OK;
		}

		Result Apu::SetSampleBits(const uint bits)
		{
			if (context.bits == bits)
				return RESULT_NOP;

			if (bits != 8 && bits != 16)
				return RESULT_ERR_UNSUPPORTED;

			context.bits = bits;
			UpdateSettings();

			return RESULT_OK;
		}

		Result Apu::SetVolume(const uint channels,const uint volume)
		{
			if (volume > 100)
				return RESULT_ERR_INVALID_PARAM;

			bool updated = false;

			for (uint i=0; i < MAX_CHANNELS; ++i)
			{
				if (channels & 1U << i)
				{
					if (context.volumes[i] != volume)
					{
						context.volumes[i] = volume;
						updated = true;
					}
				}
			}

			if (!updated)
				return RESULT_NOP;

			UpdateSettings();

			return RESULT_OK;
		}

		uint Apu::GetVolume(const uint channel) const
		{
			for (uint i=0; i < MAX_CHANNELS; ++i)
			{
				if (channel & 1U << i)
					return context.volumes[i];
			}

			return 0;
		}

		Result Apu::SetSpeed(const uint speed)
		{
			if (context.speed == speed)
				return RESULT_NOP;

			if ((speed > 0 && speed < 30) || speed > 240)
				return RESULT_ERR_UNSUPPORTED;

			context.speed = speed;
			UpdateSettings();

			return RESULT_OK;
		}

		void Apu::SetAutoTranspose(const bool transpose)
		{
			if (context.transpose != transpose)
			{
				context.transpose = transpose;
				UpdateSettings();
			}
		}

		void Apu::EnableStereo(const bool enable)
		{
			if (context.stereo != enable)
			{
				context.stereo = enable;
				UpdateSettings();
			}
		}

		void Apu::CalculateOscillatorClock(Cycle& rate,Cycle& fixed) const
		{
			dword sampleRate = context.rate;
			uint i = 0;

			if (mode == MODE_NTSC)
			{
				if (context.transpose && context.speed)
					sampleRate = sampleRate * FPS_NTSC / context.speed;

				while (++i < 0x1000 && qword(Cpu::MC_NTSC) * (i+1) / sampleRate <= 0x7FFFF && qword(Cpu::MC_NTSC) * i % sampleRate);

				rate = qword(Cpu::MC_NTSC) * i / sampleRate;
				fixed = i * dword(Cpu::CLK_NTSC_DIV) * Cpu::MC_DIV_NTSC;
			}
			else
			{
				if (context.transpose && context.speed)
					sampleRate = sampleRate * FPS_PAL / context.speed;

				while (++i < 0x1000 && qword(Cpu::MC_PAL) * (i+1) / sampleRate <= 0x7FFFF && qword(Cpu::MC_PAL) * i % sampleRate);

				rate = qword(Cpu::MC_PAL) * i / sampleRate;
				fixed = i * dword(Cpu::CLK_PAL_DIV) * Cpu::MC_DIV_PAL;
			}
		}

		void Apu::UpdateSettings()
		{
			cycles.Update( context.rate, context.speed, mode );
			dcBlocker.Reset();
			buffer.Reset( context.bits );

			Cycle rate, fixed;
			CalculateOscillatorClock( rate, fixed );

			square[0].SetContext ( rate, fixed, (context.volumes[ CHANNEL_SQUARE1  ] * uint(OUTPUT_MUL) + DEFAULT_VOLUME/2) / DEFAULT_VOLUME       );
			square[1].SetContext ( rate, fixed, (context.volumes[ CHANNEL_SQUARE2  ] * uint(OUTPUT_MUL) + DEFAULT_VOLUME/2) / DEFAULT_VOLUME       );
			triangle.SetContext  ( rate, fixed, (context.volumes[ CHANNEL_TRIANGLE ] * uint(OUTPUT_MUL) + DEFAULT_VOLUME/2) / DEFAULT_VOLUME       );
			noise.SetContext     ( rate, fixed, (context.volumes[ CHANNEL_NOISE    ] * uint(OUTPUT_MUL) + DEFAULT_VOLUME/2) / DEFAULT_VOLUME, mode );
			dmc.SetContext       (              (context.volumes[ CHANNEL_DPCM     ] * uint(OUTPUT_MUL) + DEFAULT_VOLUME/2) / DEFAULT_VOLUME, mode );

			context.audible =
			(
				uint(context.volumes[ CHANNEL_SQUARE1  ]) |
				uint(context.volumes[ CHANNEL_SQUARE2  ]) |
				uint(context.volumes[ CHANNEL_TRIANGLE ]) |
				uint(context.volumes[ CHANNEL_NOISE    ]) |
				uint(context.volumes[ CHANNEL_DPCM     ])
			);

			if (extChannel)
			{
				extChannel->SetContext( rate, fixed, mode, context.volumes );

				if (extChannel->GetOutputVolume())
					context.audible = true;
			}

			BeginFrame( stream );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Apu::BeginFrame(Sound::Output* const s)
		{
			stream = s;

			if (stream && context.audible)
				updater = &Apu::SyncOn;
			else
				updater = &Apu::SyncOff;
		}

		inline bool Apu::NoFrameClockCollision() const
		{
			return cycles.frameCounter != cpu.GetMasterClockCycles() * cycles.fixed;
		}

		inline void Apu::Update(const Cycle count)
		{
			(*this.*updater)( count * cycles.fixed );
		}

		inline void Apu::UpdateLatency()
		{
			(*this.*updater)( (cpu.GetMasterClockCycles() + cpu.GetMasterClockCycle(1)) * cycles.fixed );
		}

		void Apu::EndFrame()
		{
			if (stream && context.audible && Sound::Output::lockCallback( *stream ))
			{
				if (context.bits == 16)
				{
					if (!context.stereo)
						FlushSound<iword,false>();
					else
						FlushSound<iword,true>();
				}
				else
				{
					if (!context.stereo)
						FlushSound<byte,false>();
					else
						FlushSound<byte,true>();
				}

				Sound::Output::unlockCallback( *stream );
			}

			Update();

			Clock( cpu.GetMasterClockCycles() );

			const Cycle frame = cpu.GetMasterClockFrameCycles();
			const Cycle fixed = frame * cycles.fixed;

			NST_VERIFY
			(
				cycles.rateCounter >= fixed &&
				cycles.frameCounter >= fixed &&
				cycles.extCounter >= fixed &&
				cycles.frameIrqClock >= frame &&
				cycles.dmcClock >= frame
			);

			cycles.rateCounter -= fixed;
			cycles.frameCounter -= fixed;

			if (cycles.extCounter != Cpu::CYCLE_MAX)
				cycles.extCounter -= fixed;

			if (cycles.frameIrqClock != Cpu::CYCLE_MAX)
				cycles.frameIrqClock -= frame;

			cycles.dmcClock -= frame;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		inline void Apu::Oscillator::ClearAmp()
		{
			amp = 0;
		}

		inline void Apu::Dmc::ClearAmp()
		{
			curSample = 0;
			linSample = 0;
		}

		void Apu::ClearBuffers()
		{
			square[0].ClearAmp();
			square[1].ClearAmp();
			triangle.ClearAmp();
			noise.ClearAmp();
			dmc.ClearAmp();

			dcBlocker.Reset();
			buffer.Reset( context.bits, false );
		}

		Apu::Envelope::Envelope()
		: outputVolume(OUTPUT_MUL)
		{
			Reset();
		}

		Apu::Channel::Channel()
		:
		mode         (MODE_NTSC),
		fixed        (1),
		active       (false),
		rate         (1),
		outputVolume (DEFAULT_VOLUME)
		{}

		Apu::Oscillator::Oscillator()
		:
		active    (false),
		timer     (0),
		rate      (1),
		frequency (1),
		amp       (0),
		fixed     (1)
		{}

		void Apu::Channel::Reset()
		{
			active = false;
		}

		void Apu::Oscillator::Reset()
		{
			active = false;
			timer = RESET_CYCLES * fixed;
			frequency = fixed;
			amp = 0;
		}

		void Apu::Channel::SetContext(const Cycle r,const Cycle f,const Mode m,const byte (&volumes)[MAX_CHANNELS])
		{
			const uint old = fixed;

			fixed = f;
			rate = r;
			mode = m;

			UpdateContext( old, volumes );
		}

		void Apu::Oscillator::SetContext(dword r,uint f)
		{
			frequency = (frequency / fixed) * f;
			timer = (timer / fixed) * f;
			fixed = f;
			rate = r;
		}

		void Apu::Dmc::SetContext(uint v,Mode m)
		{
			mode = m;

			if (outputVolume)
				linSample /= outputVolume;

			if (outputVolume)
				curSample /= outputVolume;

			linSample *= v;
			curSample *= v;
			outputVolume = v;

			if (!v)
				active = false;
		}

		void Apu::Square::Reset()
		{
			Oscillator::Reset();

			step = 0;
			duty = 0;
			frequency = fixed * 2;

			envelope.Reset();
			lengthCounter.Reset();

			sweepRate = 0;
			sweepShift = 0;
			sweepCount = 1;
			sweepReload = false;
			sweepIncrease = ~0U;

			waveLength = 0;
			validFrequency = false;
		}

		void Apu::Noise::Reset()
		{
			Oscillator::Reset();

			envelope.Reset();
			lengthCounter.Reset();

			bits = 1;
			shifter = 13;
			frequency = lut[mode][0] * dword(fixed);
		}

		void Apu::Triangle::Reset()
		{
			Oscillator::Reset();

			lengthCounter.Reset();

			step = 0x7;
			status = STATUS_COUNTING;
			linearCtrl = 0;
			linearCounter = 0;
			waveLength = 0;
		}

		void Apu::Dmc::Reset(Cpu& cpu)
		{
			active            = false;
			frequency         = GetResetFrequency( mode );
			curSample         = 0;
			linSample         = 0;
			loop              = false;
			loadedLengthCount = 1;
			loadedAddress     = 0xC000;
			out.dac           = 0;
			out.shifter       = 1;
			out.buffer        = 0x00;
			dma.lengthCounter = 0;
			dma.buffered      = false;
			dma.address       = 0xC000;
			dma.buffer        = 0x00;

			cpu.SetLine( Cpu::IRQ_DMC, false );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Apu::Sample Apu::DcBlocker::Apply(Sample sample)
		{
			acc  -= prev;
			prev  = signed_shl(sample,15);
			acc  += prev - next * POLE;
			next  = signed_shr(acc,15);
			return next;
		}

		void Apu::Envelope::Clock()
		{
			if (!reset)
			{
				if (--count)
					return;

				if (volume | reg & uint(DECAY_LOOP))
					volume = (volume - 1U) & 0xF;
			}
			else
			{
				reset = false;
				volume = 0xF;
			}

			count = (reg & uint(DECAY_RATE)) + 1;

			if (!(reg & uint(DECAY_DISABLE)))
				output = volume * outputVolume;
		}

		inline void Apu::Envelope::UpdateOutput()
		{
			output = ((reg & uint(DECAY_DISABLE)) ? (reg & uint(DECAY_RATE)) : volume) * outputVolume;
		}

		void Apu::Envelope::Write(const uint data)
		{
			reg = data;
			UpdateOutput();
		}

		inline void Apu::Square::Toggle(const uint state)
		{
			lengthCounter.Enable( state );

			if (!state)
				active = false;
		}

		NST_FORCE_INLINE void Apu::Triangle::Toggle(const uint state)
		{
			lengthCounter.Enable( state );

			if (!state)
				active = false;
		}

		NST_FORCE_INLINE void Apu::Noise::Toggle(const uint state)
		{
			lengthCounter.Enable( state );

			if (!state)
				active = false;
		}

		NST_FORCE_INLINE void Apu::Dmc::Toggle(const uint enable,Cpu& cpu)
		{
			cpu.ClearIRQ( Cpu::IRQ_DMC );

			if (!enable)
			{
				dma.lengthCounter = 0;
			}
			else if (!dma.lengthCounter)
			{
				dma.lengthCounter = loadedLengthCount;
				dma.address = loadedAddress;

				if (!dma.buffered)
					DoDMA( cpu );
			}
		}

		inline bool Apu::Square::CanOutput() const
		{
			return lengthCounter.GetCount() && envelope.Volume() && validFrequency;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::Square::SetContext(dword r,uint f,uint v)
		{
			Oscillator::SetContext( r, f );
			envelope.SetOutputVolume( v );
			active = CanOutput();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		bool Apu::Square::UpdateFrequency()
		{
			frequency = (waveLength + 1) * fixed * 2;

			if (waveLength >= MIN_FRQ && waveLength + (sweepIncrease & waveLength >> sweepShift) <= MAX_FRQ)
			{
				validFrequency = true;
				return lengthCounter.GetCount() && envelope.Volume();
			}

			validFrequency = false;

			return false;
		}

		NST_FORCE_INLINE void Apu::Square::WriteReg0(const uint data)
		{
			envelope.Write( data );
			active = CanOutput();
			duty = data >> REG0_DUTY_SHIFT;
		}

		NST_FORCE_INLINE void Apu::Square::WriteReg1(const uint data)
		{
			sweepIncrease = (data & REG1_SWEEP_DECREASE) ? 0U : ~0U;
			sweepShift = data & REG1_SWEEP_SHIFT;
			sweepRate = 0;

			if ((data & (REG1_SWEEP_ENABLED|REG1_SWEEP_SHIFT)) > REG1_SWEEP_ENABLED)
			{
				sweepRate = ((data & REG1_SWEEP_RATE) >> REG1_SWEEP_RATE_SHIFT) + 1;
				sweepReload = true;
			}

			active = UpdateFrequency();
		}

		NST_FORCE_INLINE void Apu::Square::WriteReg2(const uint data)
		{
			waveLength &= uint(REG3_WAVELENGTH_HIGH) << 8;
			waveLength |= data;

			active = UpdateFrequency();
		}

		NST_FORCE_INLINE void Apu::Square::WriteReg3(const uint data,const bool noFrameCollision)
		{
			step = 0;

			envelope.ResetClock();

			if (!lengthCounter.GetCount() || noFrameCollision)
				lengthCounter.Write( data );

			waveLength &= REG2_WAVELENGTH_LOW;
			waveLength |= (data & REG3_WAVELENGTH_HIGH) << 8;

			active = UpdateFrequency();
		}

		void Apu::Square::ClockEnvelope()
		{
			envelope.Clock();
			active = CanOutput();
		}

		void Apu::Square::ClockSweep(const uint complement)
		{
			if (!envelope.Loop() && lengthCounter.Clock())
				active = false;

			if (sweepRate && !--sweepCount)
			{
				sweepCount = sweepRate;

				if (waveLength >= MIN_FRQ)
				{
					const uint shifted = waveLength >> sweepShift;

					if (!sweepIncrease)
					{
						waveLength += complement - shifted;
						active = UpdateFrequency();
					}
					else if (waveLength + shifted <= MAX_FRQ)
					{
						waveLength += shifted;
						active = UpdateFrequency();
					}
				}
			}

			if (sweepReload)
			{
				sweepReload = false;
				sweepCount = sweepRate;
			}
		}

		inline bool Apu::Triangle::CanOutput() const
		{
			return lengthCounter.GetCount() && linearCounter && waveLength >= MIN_FRQ && outputVolume;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::Triangle::SetContext(dword r,uint f,uint v)
		{
			Oscillator::SetContext( r, f );
			outputVolume = v;
			active = CanOutput();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NST_FORCE_INLINE void Apu::Triangle::WriteReg0(const uint data)
		{
			linearCtrl = data;
		}

		NST_FORCE_INLINE void Apu::Triangle::WriteReg2(const uint data)
		{
			waveLength &= uint(REG3_WAVE_LENGTH_HIGH) << 8;
			waveLength |= data;
			frequency = (waveLength + 1) * fixed;

			active = CanOutput();
		}

		NST_FORCE_INLINE void Apu::Triangle::WriteReg3(const uint data,const bool noFrameCollision)
		{
			waveLength &= REG2_WAVE_LENGTH_LOW;
			waveLength |= (data & REG3_WAVE_LENGTH_HIGH) << 8;
			frequency = (waveLength + 1) * fixed;
			status = STATUS_RELOAD;

			if (!lengthCounter.GetCount() || noFrameCollision)
				lengthCounter.Write( data );

			active = CanOutput();
		}

		void Apu::Triangle::ClockLinearCounter()
		{
			if (status == STATUS_COUNTING)
			{
				if (linearCounter && !--linearCounter)
					active = false;
			}
			else
			{
				if (!(linearCtrl & REG0_LINEAR_COUNTER_START))
					status = STATUS_COUNTING;

				linearCounter = linearCtrl & REG0_LINEAR_COUNTER_LOAD;
				active = CanOutput();
			}
		}

		void Apu::Triangle::ClockLengthCounter()
		{
			if (!(linearCtrl & REG0_LINEAR_COUNTER_START) && lengthCounter.Clock())
				active = false;
		}

		inline bool Apu::Noise::CanOutput() const
		{
			return lengthCounter.GetCount() && envelope.Volume();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::Noise::SetContext(dword r,uint f,uint v,Mode m)
		{
			Oscillator::SetContext( r, f );
			mode = m;
			envelope.SetOutputVolume( v );
			active = CanOutput();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NST_FORCE_INLINE void Apu::Noise::WriteReg0(const uint data)
		{
			envelope.Write( data );
			active = CanOutput();
		}

		NST_FORCE_INLINE void Apu::Noise::WriteReg2(const uint data)
		{
			frequency = lut[mode][data & REG2_SAMPLE_RATE] * dword(fixed);
			shifter = (data & REG2_93BIT_MODE) ? 8 : 13;
		}

		NST_FORCE_INLINE void Apu::Noise::WriteReg3(const uint data,const bool noFrameCollision)
		{
			envelope.ResetClock();

			if (!lengthCounter.GetCount() || noFrameCollision)
				lengthCounter.Write( data );

			active = CanOutput();
		}

		void Apu::Noise::ClockEnvelope()
		{
			envelope.Clock();
			active = CanOutput();
		}

		void Apu::Noise::ClockLengthCounter()
		{
			if (!envelope.Loop() && lengthCounter.Clock())
				active = false;
		}

		dword Apu::Square::GetSample()
		{
			NST_VERIFY( bool(active) == CanOutput() && timer >= 0 );

			dword sum = timer;
			timer -= idword(rate);

			if (active)
			{
				static const byte steps[4][8] =
				{
					{0x1F,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F},
					{0x1F,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F},
					{0x1F,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F},
					{0x00,0x1F,0x1F,0x00,0x00,0x00,0x00,0x00}
				};

				if (timer >= 0)
				{
					amp = dword(envelope.Volume()) >> steps[duty][step];
				}
				else
				{
					sum >>= steps[duty][step];

					do
					{
						sum += NST_MIN(dword(-timer),frequency) >> steps[duty][step = (step + 1) & 0x7];
						timer += idword(frequency);
					}
					while (timer < 0);

					NST_VERIFY( !envelope.Volume() || sum <= 0xFFFFFFFF / envelope.Volume() + rate/2 );
					amp = (sum * envelope.Volume() + rate/2) / rate;
				}
			}
			else
			{
				if (timer < 0)
				{
					const uint count = (dword(-timer) + frequency - 1) / frequency;
					step = (step + count) & 0x7;
					timer += idword(count * frequency);
				}

				if (amp < OUTPUT_DECAY)
				{
					return 0;
				}
				else
				{
					amp -= OUTPUT_DECAY;
				}
			}

			return amp;
		}

		NST_FORCE_INLINE dword Apu::Triangle::GetSample()
		{
			NST_VERIFY( bool(active) == CanOutput() && timer >= 0 );

			if (active)
			{
				static const byte steps[32] =
				{
					0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,
					0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
					0xF,0xE,0xD,0xC,0xB,0xA,0x9,0x8,
					0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0
				};

				dword sum = timer;
				timer -= idword(rate);

				if (timer >= 0)
				{
					amp = steps[step] * outputVolume * 3;
				}
				else
				{
					sum *= steps[step];

					do
					{
						sum += NST_MIN(dword(-timer),frequency) * steps[step = (step + 1) & 0x1F];
						timer += idword(frequency);
					}
					while (timer < 0);

					NST_VERIFY( !outputVolume || sum <= 0xFFFFFFFF / outputVolume + rate/2 );
					amp = (sum * outputVolume + rate/2) / rate * 3;
				}
			}
			else if (amp < OUTPUT_DECAY)
			{
				return 0;
			}
			else
			{
				amp -= OUTPUT_DECAY;
				step &= STEP_CHECK;
			}

			return amp;
		}

		NST_FORCE_INLINE dword Apu::Noise::GetSample()
		{
			NST_VERIFY( bool(active) == CanOutput() && timer >= 0 );

			dword sum = timer;
			timer -= idword(rate);

			if (active)
			{
				if (timer >= 0)
				{
					if (!(bits & 0x4000))
						return envelope.Volume() * 2;
				}
				else
				{
					if (bits & 0x4000)
						sum = 0;

					do
					{
						bits = (bits << 1) | ((bits >> 14 ^ bits >> shifter) & 0x1);

						if (!(bits & 0x4000))
							sum += NST_MIN(dword(-timer),frequency);

						timer += idword(frequency);
					}
					while (timer < 0);

					NST_VERIFY( !envelope.Volume() || sum <= 0xFFFFFFFF / envelope.Volume() + rate/2 );
					return (sum * envelope.Volume() + rate/2) / rate * 2;
				}
			}
			else while (timer < 0)
			{
				bits = (bits << 1) | ((bits >> 14 ^ bits >> shifter) & 0x1);
				timer += idword(frequency);
			}

			return 0;
		}

		NST_FORCE_INLINE dword Apu::Dmc::GetSample()
		{
			if (curSample != linSample)
			{
				const uint step = outputVolume * INP_STEP;

				if (curSample + step - linSample <= step*2)
				{
					linSample = curSample;
				}
				else if (curSample > linSample)
				{
					linSample += step;
				}
				else
				{
					linSample -= step;
				}
			}

			return linSample;
		}

		inline uint Apu::Dmc::CheckSample() const
		{
			return curSample;
		}

		void Apu::Dmc::DoDMA(Cpu& cpu)
		{
			NST_VERIFY( !dma.buffered );

			dma.buffer = cpu.Peek( dma.address );
			cpu.StealCycles( cpu.GetMasterClockCycle(DMA_CYCLES) );
			dma.address = 0x8000 | ((dma.address + 1) & 0x7FFF);
			dma.buffered = true;

			NST_VERIFY( dma.lengthCounter );

			if (--dma.lengthCounter)
				return;

			// sample finished playing

			if (loop)
			{
				// reload counters
				dma.address = loadedAddress;
				dma.lengthCounter = loadedLengthCount;
			}
			else if (cpu.IsLine(Cpu::IRQ_DMC))
			{
				cpu.DoIRQ( Cpu::IRQ_DMC );
			}
		}

		NST_FORCE_INLINE void Apu::Dmc::WriteReg0(const uint data,Cpu& cpu)
		{
			loop = data & REG0_LOOP;
			cpu.SetLine( Cpu::IRQ_DMC, data & REG0_IRQ_ENABLE );
			frequency = lut[mode][data & REG0_FREQUENCY];
		}

		void Apu::Dmc::OutputBuffer()
		{
			const uint next = out.dac + ((out.buffer & 0x1U) << 2) - 2;
			out.buffer >>= 1;

			if (next <= 0x7F)
			{
				out.dac = next;
				curSample = next * outputVolume;
			}
		}

		NST_FORCE_INLINE void Apu::Dmc::WriteReg1(const uint data)
		{
			out.dac = data & 0x7F;
			curSample = out.dac * outputVolume;
		}

		NST_FORCE_INLINE void Apu::Dmc::WriteReg2(const uint data)
		{
			loadedAddress = 0xC000 + (data << 6);
		}

		NST_FORCE_INLINE void Apu::Dmc::WriteReg3(const uint data)
		{
			loadedLengthCount = (data << 4) + 1;
		}

		NST_FORCE_INLINE void Apu::Dmc::Clock(Cpu& cpu)
		{
			if (active)
				OutputBuffer();

			if (!--out.shifter)
			{
				out.shifter = 8;
				active = dma.buffered;

				if (active)
				{
					// fetch next sample from the DMA unit
					// and start a new output sequence

					active = outputVolume;
					dma.buffered = false;
					out.buffer = dma.buffer;

					if (dma.lengthCounter)
						DoDMA( cpu );
				}
			}
		}

		Cycle Apu::ClockOscillators()
		{
			NST_COMPILE_ASSERT( STATUS_SEQUENCE_5_STEP == 0x80 );

			square[0].ClockEnvelope();
			square[1].ClockEnvelope();

			triangle.ClockLinearCounter();
			noise.ClockEnvelope();

			if (cycles.frameDivider & 0x1)
			{
				square[0].ClockSweep( ~0U );
				square[1].ClockSweep(  0U );

				triangle.ClockLengthCounter();
				noise.ClockLengthCounter();
			}

			cycles.frameDivider = (cycles.frameDivider + 1) & 0x3;

			return Cycles::frameClocks[mode][ctrl >> 7][cycles.frameDivider] * cycles.fixed;
		}

		inline uint Apu::Dmc::SetSample(const uint value)
		{
			const uint old = curSample;
			curSample = value;
			return old;
		}

		inline Cycle Apu::Dmc::GetFrequency() const
		{
			return frequency;
		}

		Cycle Apu::Dmc::GetResetFrequency(Mode mode)
		{
			return lut[mode][0];
		}

		void Apu::ClockDmc(const Cycle target)
		{
			NST_VERIFY( cycles.dmcClock <= target );

			do
			{
				uint sample = dmc.CheckSample();

				dmc.Clock( cpu );

				if (sample != dmc.CheckSample())
				{
					// The operation changed the sample output value of the
					// DMC. Rewind and update the rest of the channels.

					sample = dmc.SetSample( sample );
					Update( cycles.dmcClock );
					dmc.SetSample( sample );
				}

				cycles.dmcClock += dmc.GetFrequency();
			}
			while (cycles.dmcClock <= target);
		}

		NST_NO_INLINE void Apu::ClockFrameIRQ()
		{
			NST_VERIFY( ctrl == STATUS_FRAME_IRQ_ENABLE );

			cpu.DoIRQ( Cpu::IRQ_FRAME, cycles.frameIrqClock );

			if (++cycles.frameIrqRepeat < 3)
			{
				cycles.frameIrqClock += cpu.GetMasterClockCycle(1);
			}
			else
			{
				cycles.frameIrqRepeat = 0;
				cycles.frameIrqClock += Cycles::frame[mode] - cpu.GetMasterClockCycle(2);
			}
		}

		Apu::Sample Apu::GetSample()
		{
			dword dac[2];

			const Sample sample = dcBlocker.Apply
			(
				(0 != (dac[0] = square[0].GetSample() + square[1].GetSample()) ? NLN_SQ_0 / (NLN_SQ_1 / dac[0] + NLN_SQ_2) : 0) +
				(0 != (dac[1] = triangle.GetSample() + noise.GetSample() + dmc.GetSample()) ? NLN_TND_0 / (NLN_TND_1 / dac[1] + NLN_TND_2) : 0)
			) + (extChannel ? extChannel->GetSample() : 0);

			return (sample <= OUTPUT_MAX) ? (sample >= OUTPUT_MIN) ? sample : OUTPUT_MIN : OUTPUT_MAX;
		}

		void Apu::SyncOn(const Cycle target)
		{
			while (cycles.rateCounter < target)
			{
				buffer << GetSample();

				if (cycles.frameCounter <= cycles.rateCounter)
					cycles.frameCounter += ClockOscillators();

				while (cycles.extCounter <= cycles.rateCounter)
					cycles.extCounter += extChannel->Clock();

				cycles.rateCounter += cycles.rate;
			}

			if (cycles.frameCounter < target)
				cycles.frameCounter += ClockOscillators();

			while (cycles.extCounter < target)
				cycles.extCounter += extChannel->Clock();

			NST_VERIFY( cycles.frameCounter >= target && cycles.extCounter >= target );
		}

		void Apu::SyncOff(const Cycle target)
		{
			cycles.rateCounter = target;

			while (cycles.frameCounter < target)
				cycles.frameCounter += ClockOscillators();

			while (cycles.extCounter < target)
				cycles.extCounter += extChannel->Clock();
		}

		inline uint Apu::Square::GetLengthCounter() const
		{
			return lengthCounter.GetCount();
		}

		inline uint Apu::Triangle::GetLengthCounter() const
		{
			return lengthCounter.GetCount();
		}

		inline uint Apu::Noise::GetLengthCounter() const
		{
			return lengthCounter.GetCount();
		}

		inline uint Apu::Dmc::GetLengthCounter() const
		{
			return dma.lengthCounter;
		}

		NES_POKE(Apu,4000)
		{
			UpdateLatency();
			square[address >> 2 & 0x1].WriteReg0( data );
		}

		NES_POKE(Apu,4001)
		{
			Update();
			square[address >> 2 & 0x1].WriteReg1( data );
		}

		NES_POKE(Apu,4002)
		{
			Update();
			square[address >> 2 & 0x1].WriteReg2( data );
		}

		NES_POKE(Apu,4003)
		{
			Update();

			const bool safe = NoFrameClockCollision();

			if (!safe)
				UpdateLatency();

			square[address >> 2 & 0x1].WriteReg3( data, safe );
		}

		NES_POKE(Apu,4008)
		{
			Update();
			triangle.WriteReg0( data );
		}

		NES_POKE(Apu,400A)
		{
			Update();
			triangle.WriteReg2( data );
		}

		NES_POKE(Apu,400B)
		{
			Update();

			const bool safe = NoFrameClockCollision();

			if (!safe)
				UpdateLatency();

			triangle.WriteReg3( data, safe );
		}

		NES_POKE(Apu,400C)
		{
			UpdateLatency();
			noise.WriteReg0( data );
		}

		NES_POKE(Apu,400E)
		{
			Update();
			noise.WriteReg2( data );
		}

		NES_POKE(Apu,400F)
		{
			Update();

			const bool safe = NoFrameClockCollision();

			if (!safe)
				UpdateLatency();

			noise.WriteReg3( data, safe );
		}

		NES_POKE(Apu,4010)
		{
			dmc.WriteReg0( data, cpu );
		}

		NES_POKE(Apu,4011)
		{
			Update();
			dmc.WriteReg1( data );
		}

		NES_POKE(Apu,4012)
		{
			dmc.WriteReg2( data );
		}

		NES_POKE(Apu,4013)
		{
			dmc.WriteReg3( data );
		}

		NES_POKE(Apu,4015)
		{
			Update();

			square[0].Toggle ( data & ENABLE_SQUARE1  );
			square[1].Toggle ( data & ENABLE_SQUARE2  );
			triangle.Toggle  ( data & ENABLE_TRIANGLE );
			noise.Toggle     ( data & ENABLE_NOISE    );
			dmc.Toggle       ( data & ENABLE_DMC, cpu );
		}

		NES_PEEK(Apu,4015)
		{
			NST_COMPILE_ASSERT( Cpu::IRQ_FRAME == 0x40 && Cpu::IRQ_DMC == 0x80 );

			if (cycles.frameCounter < cpu.GetMasterClockCycles() * cycles.fixed)
				Update(); // update only if channels want to be clocked

			while (cycles.frameIrqClock <= cpu.GetMasterClockCycles())
				ClockFrameIRQ();

			const uint data = (cpu.GetIRQ() & (Cpu::IRQ_FRAME|Cpu::IRQ_DMC)) |
			(
				uint( square[0].GetLengthCounter() ? ENABLE_SQUARE1  : 0 ) |
				uint( square[1].GetLengthCounter() ? ENABLE_SQUARE2  : 0 ) |
				uint( triangle.GetLengthCounter()  ? ENABLE_TRIANGLE : 0 ) |
				uint( noise.GetLengthCounter()     ? ENABLE_NOISE    : 0 ) |
				uint( dmc.GetLengthCounter()       ? ENABLE_DMC      : 0 )
			);

			cpu.ClearIRQ( Cpu::IRQ_FRAME );

			return data;
		}

		void Apu::Poke_4017(const uint data)
		{
			NST_COMPILE_ASSERT( MODE_NTSC == 0 && MODE_PAL == 1 );

			Cycle delay = cpu.GetMasterClockCycles();

			if (cpu.IsOddCycle())
				delay += cpu.GetMasterClockCycle(1);

			(*this.*updater)( delay * cycles.fixed );

			while (cycles.frameIrqClock <= delay)
				ClockFrameIRQ();

			delay += cpu.GetMasterClockCycle(1);

			cycles.frameCounter = (delay + Cycles::frameClocks[mode][data >> 7][0]) * cycles.fixed;
			cycles.frameDivider = 0;

			ctrl = data & STATUS_BITS;

			if (ctrl)
			{
				cycles.frameIrqClock = Cpu::CYCLE_MAX;

				if (ctrl & STATUS_NO_FRAME_IRQ)
					cpu.ClearIRQ( Cpu::IRQ_FRAME );

				if (ctrl & STATUS_SEQUENCE_5_STEP)
				{
					square[0].ClockEnvelope();
					square[1].ClockEnvelope();

					square[0].ClockSweep( ~0U );
					square[1].ClockSweep(  0U );

					triangle.ClockLinearCounter();
					noise.ClockEnvelope();

					triangle.ClockLengthCounter();
					noise.ClockLengthCounter();
				}
			}
			else
			{
				cycles.frameIrqRepeat = 0;
				cycles.frameIrqClock = delay + Cycles::frame[mode];

				NST_VERIFY( cycles.frameIrqClock >= cpu.GetMasterClockFrameCycles() );
			}
		}

		NES_PEEK(Apu,4xxx)
		{
			return 0x40;
		}

		template<typename T,bool STEREO>
		void Apu::FlushSound()
		{
			for (uint i=0; i < 2; ++i)
			{
				if (stream->length[i])
				{
					Sound::Buffer::Block block( stream->length[i] );
					buffer >> block;

					Sound::Buffer::Renderer<T,STEREO> output( stream->samples[i], stream->length[i], buffer.history );

					if (output << block)
					{
						const Cycle target = cpu.GetMasterClockCycles() * cycles.fixed;

						while (cycles.rateCounter < target && output)
						{
							output << GetSample();

							if (cycles.frameCounter <= cycles.rateCounter)
								cycles.frameCounter += ClockOscillators();

							while (cycles.extCounter <= cycles.rateCounter)
								cycles.extCounter += extChannel->Clock();

							cycles.rateCounter += cycles.rate;
						}

						if (output)
						{
							if (cycles.frameCounter < target)
								cycles.frameCounter += ClockOscillators();

							while (cycles.extCounter < target)
								cycles.extCounter += extChannel->Clock();

							NST_VERIFY( cycles.frameCounter >= target && cycles.extCounter >= target );

							do
							{
								output << GetSample();
							}
							while (output);
						}
					}
				}
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Apu::Envelope::SetOutputVolume(uint v)
		{
			outputVolume = v;
			UpdateOutput();
		}

		void Apu::SaveState(State::Saver& state,const dword id) const
		{
			state.Begin( id );

			{
				byte data[4];

				data[0] = ctrl;

				Cycle clock = cycles.frameCounter / cycles.fixed;

				NST_VERIFY( clock >= cpu.GetMasterClockCycles() );

				if (clock > cpu.GetMasterClockCycles())
					clock = (clock - cpu.GetMasterClockCycles()) / (mode == MODE_NTSC ? Cpu::MC_DIV_NTSC : Cpu::MC_DIV_PAL);
				else
					clock = 0;

				data[1] = clock & 0xFF;
				data[2] = clock >> 8;
				data[3] = cycles.frameDivider;

				state.Begin( AsciiId<'F','R','M'>::V ).Write( data ).End();
			}

			square[0].SaveState( state, AsciiId<'S','Q','0'>::V );
			square[1].SaveState( state, AsciiId<'S','Q','1'>::V );
			triangle.SaveState( state, AsciiId<'T','R','I'>::V );
			noise.SaveState( state, AsciiId<'N','O','I'>::V );
			dmc.SaveState( state, AsciiId<'D','M','C'>::V, cpu, cycles.dmcClock );

			state.End();
		}

		void Apu::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'F','R','M'>::V:
					{
						State::Loader::Data<4> data( state );

						ctrl = data[0] & STATUS_BITS;
						cycles.rateCounter = cpu.GetMasterClockCycles() * cycles.fixed;

						cycles.frameCounter = cycles.fixed *
						(
							cpu.GetMasterClockCycles() +
							(
								(data[1] | data[2] << 8) *
								(mode == MODE_NTSC ? Cpu::MC_DIV_NTSC : Cpu::MC_DIV_PAL)
							)
						);

						cycles.frameDivider = data[3] & 0x3;

						if (ctrl == STATUS_FRAME_IRQ_ENABLE)
						{
							cycles.frameIrqClock = (mode == MODE_NTSC ? dword(FRAME_CLOCK_NTSC) * Cpu::MC_DIV_NTSC / 2 : dword(FRAME_CLOCK_PAL) * Cpu::MC_DIV_PAL / 2);
							cycles.frameIrqClock = cycles.frameIrqClock * (3 - cycles.frameDivider) + (cycles.frameCounter / cycles.fixed);
						}
						else
						{
							cycles.frameIrqClock = Cpu::CYCLE_MAX;
						}

						break;
					}

					case AsciiId<'S','Q','0'>::V:

						square[0].LoadState( state );
						break;

					case AsciiId<'S','Q','1'>::V:

						square[1].LoadState( state );
						break;

					case AsciiId<'T','R','I'>::V:

						triangle.LoadState( state );
						break;

					case AsciiId<'N','O','I'>::V:

						noise.LoadState( state );
						break;

					case AsciiId<'D','M','C'>::V:

						dmc.LoadState( state, cpu, cycles.dmcClock );
						break;
				}

				state.End();
			}
		}

		void Apu::Envelope::SaveState(State::Saver& state,const dword id) const
		{
			NST_VERIFY( count );

			byte data[3] =
			{
				count - 1,
				volume,
				reg & uint(DECAY_RATE)
			};

			if ( reset                     ) data[1] |= uint(SAVE_1_RESET);
			if ( reg & uint(DECAY_DISABLE) ) data[2] |= uint(SAVE_2_DECAY_DISABLE);
			if ( reg & uint(DECAY_LOOP)    ) data[2] |= uint(SAVE_2_DECAY_LOOP);

			state.Begin( id ).Write( data ).End();
		}

		void Apu::Envelope::LoadState(State::Loader& state)
		{
			State::Loader::Data<3> data( state );

			count  = (data[0] & SAVE_0_COUNT) + 1;
			volume = data[1] & SAVE_1_VOLUME;
			reset  = data[1] >> SAVE_1_RESET_SHIFT;
			reg    = data[2] & DECAY_RATE;

			if (data[2] & SAVE_2_DECAY_DISABLE)
				reg |= DECAY_DISABLE;

			if (data[2] & SAVE_2_DECAY_LOOP)
				reg |= DECAY_LOOP;

			UpdateOutput();
		}

		void Apu::LengthCounter::LoadState(State::Loader& state)
		{
			const uint data = state.Read8();
			enabled = (data != 0xFF);
			count = (enabled ? data : 0);
		}

		void Apu::LengthCounter::SaveState(State::Saver& state,const dword id) const
		{
			NST_VERIFY( count < 0xFF );
			state.Begin( id ).Write8( enabled ? count : 0xFF ).End();
		}

		void Apu::Square::SaveState(State::Saver& state,const dword id) const
		{
			state.Begin( id );

			{
				byte data[4];

				data[0] = waveLength & 0xFF;
				data[1] = (waveLength >> 8) | (duty ? duty << (2+3) : 2U << 3); // for version compatibility
				data[2] = (sweepCount - 1) << 4;

				if (sweepRate)
					data[2] |= uint(SAVE_2_SWEEP_ENABLE) | (sweepRate - 1);

				if (sweepReload)
					data[2] |= uint(SAVE_2_SWEEP_RELOAD);

				data[3] = sweepShift;

				if (!sweepIncrease)
					data[3] |= uint(SAVE_3_SWEEP_DECREASE);

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			lengthCounter.SaveState( state, AsciiId<'L','E','N'>::V );
			envelope.SaveState( state, AsciiId<'E','N','V'>::V );

			state.End();
		}

		void Apu::Square::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<4> data( state );

						waveLength = data[0] | (data[1] & SAVE_1_WAVELENGTH_HIGH) << 8;

						// for version compatibility
						switch (data[1] & SAVE_1_DUTY)
						{
							case 4U  << 3: duty = 1; break;
							case 8U  << 3: duty = 2; break;
							case 12U << 3: duty = 3; break;
							default:       duty = 0; break;
						}

						if (data[2] & SAVE_2_SWEEP_ENABLE)
							sweepRate = (data[2] & SAVE_2_SWEEP_RATE) + 1;
						else
							sweepRate = 0;

						sweepCount = ((data[2] & SAVE_2_SWEEP_COUNT) >> 4) + 1;
						sweepReload = bool(data[2] & SAVE_2_SWEEP_RELOAD);
						sweepShift = data[3] & SAVE_3_SWEEP_SHIFT;
						sweepIncrease = (data[3] & SAVE_3_SWEEP_DECREASE) ? 0U : ~0U;
						break;
					}

					case AsciiId<'L','E','N'>::V:

						lengthCounter.LoadState( state );
						break;

					case AsciiId<'E','N','V'>::V:

						envelope.LoadState( state );
						break;
				}

				state.End();
			}

			step = 0;
			timer = 0;
			active = UpdateFrequency();
		}

		void Apu::Triangle::SaveState(State::Saver& state,const dword id) const
		{
			state.Begin( id );

			{
				const byte data[4] =
				{
					waveLength & 0xFF,
					waveLength >> 8,
					linearCounter | (status << 7),
					linearCtrl
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			lengthCounter.SaveState( state, AsciiId<'L','E','N'>::V );

			state.End();
		}

		void Apu::Triangle::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<4> data( state );

						waveLength = data[0] | (data[1] & SAVE_1_WAVELENGTH_HIGH) << 8;
						linearCounter = data[2] & SAVE_2_LINEAR_COUNT;
						status = data[2] >> 7;
						linearCtrl = data[3];

						frequency = (waveLength + 1) * fixed;
						break;
					}

					case AsciiId<'L','E','N'>::V:

						lengthCounter.LoadState( state );
						break;
				}

				state.End();
			}

			timer = 0;
			step = 0;
			active = CanOutput();
		}

		void Apu::Noise::SaveState(State::Saver& state,const dword id) const
		{
			state.Begin( id );

			{
				uint data = frequency / fixed;

				for (uint i=0; i < 16; ++i)
				{
					if (data == lut[mode][i])
					{
						data = i;
						break;
					}
				}

				if (shifter == 8)
					data |= SAVE_93BIT_MODE;

				state.Begin( AsciiId<'R','E','G'>::V ).Write8( data ).End();
			}

			lengthCounter.SaveState( state, AsciiId<'L','E','N'>::V );
			envelope.SaveState( state, AsciiId<'E','N','V'>::V );

			state.End();
		}

		void Apu::Noise::LoadState(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						const uint data = state.Read8();

						frequency = lut[mode][data & SAVE_WAVELENGTH] * dword(fixed);
						shifter = (data & SAVE_93BIT_MODE) ? 8 : 13;
						break;
					}

					case AsciiId<'L','E','N'>::V:

						lengthCounter.LoadState( state );
						break;

					case AsciiId<'E','N','V'>::V:

						envelope.LoadState( state );
						break;
				}

				state.End();
			}

			timer = 0;
			bits = 1;
			active = CanOutput();
		}

		void Apu::Dmc::SaveState(State::Saver& state,const dword id,const Cpu& cpu,Cycle dmcClock) const
		{
			NST_VERIFY( dmcClock >= cpu.GetMasterClockCycles() );

			if (dmcClock > cpu.GetMasterClockCycles())
				dmcClock = (dmcClock - cpu.GetMasterClockCycles()) / (mode == MODE_NTSC ? Cpu::MC_DIV_NTSC : Cpu::MC_DIV_PAL);
			else
				dmcClock = 0;

			NST_VERIFY( dmcClock <= 0x1FFF );

			byte data[12] =
			{
				dmcClock & 0xFF,
				dmcClock >> 16,
				(loop ? uint(SAVE_2_LOOP) : 0U) | (cpu.IsLine(Cpu::IRQ_DMC) ? uint(SAVE_2_IRQ) : 0U) | (dma.lengthCounter ? uint(SAVE_2_ENABLED) : 0U),
				(loadedAddress - 0xC000) >> 6,
				(loadedLengthCount - 1) >> 4,
				(dma.address >> 0 & SAVE_5_ADDRESS_LOW),
				(dma.address >> 8 & SAVE_6_ADDRESS_HIGH) | (dma.buffered ? SAVE_6_BUFFERED : 0),
				dma.lengthCounter ? (dma.lengthCounter - 1) >> 4 : 0,
				dma.buffer,
				8 - out.shifter,
				out.buffer,
				out.dac
			};

			for (uint i=0; i < 16; ++i)
			{
				if (frequency == lut[mode][i])
				{
					data[2] |= i;
					break;
				}
			}

			state.Begin( id ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
		}

		void Apu::Dmc::LoadState(State::Loader& state,Cpu& cpu,Cycle& dmcClock)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<12> data( state );

						dmcClock = data[0] | data[1] << 8;
						dmcClock = cpu.GetMasterClockCycles() + (mode == MODE_NTSC ? dmcClock * Cpu::MC_DIV_NTSC : dmcClock * Cpu::MC_DIV_PAL);

						cpu.SetLine( Cpu::IRQ_DMC, data[2] & SAVE_2_IRQ );

						frequency         = lut[mode][data[2] & SAVE_2_FREQUENCY];
						loop              = data[2] & SAVE_2_LOOP;
						loadedAddress     = 0xC000 + (data[3] << 6);
						loadedLengthCount = (data[4] << 4) + 1;
						dma.address       = 0x8000 + (data[5] | (data[6] & SAVE_6_ADDRESS_HIGH) << 8);
						dma.buffered      = data[6] & SAVE_6_BUFFERED;
						dma.lengthCounter = (data[2] & SAVE_2_ENABLED) ? (data[7] << 4) + 1 : 0;
						dma.buffer        = data[8];
						out.shifter       = 8 - (data[9] & SAVE_9_SHIFTER);
						out.buffer        = data[10];
						out.dac           = data[11] & SAVE_11_DAC;

						curSample = out.dac * outputVolume;
						linSample = curSample;
						active = dma.buffered && outputVolume;
						break;
					}
				}

				state.End();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
