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

#ifndef NST_CPU_H
#error Do not include NstApu.h directly!
#endif

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Output;
			class Buffer;
		}

		namespace State
		{
			class Saver;
			class Loader;
		}

		class Cpu;

		class Apu
		{
		public:

			Apu(Cpu*);
			~Apu();

			typedef iword Sample;

			enum
			{
				CHANNEL_SQUARE1,
				CHANNEL_SQUARE2,
				CHANNEL_TRIANGLE,
				CHANNEL_NOISE,
				CHANNEL_DPCM,
				CHANNEL_FDS,
				CHANNEL_MMC5,
				CHANNEL_VRC6,
				CHANNEL_VRC7,
				CHANNEL_N106,
				CHANNEL_S5B,
				MAX_CHANNELS
			};

			enum
			{
				FRAME_CLOCK_NTSC = 14915,
				FRAME_CLOCK_PAL  = 16626,
				OUTPUT_MIN       =-32768L,
				OUTPUT_MAX       = 32767L,
				OUTPUT_MUL       = 256,
				OUTPUT_DECAY     = OUTPUT_MUL / 4 - 1,
				DEFAULT_VOLUME   = 85
			};

			void   Reset(bool);
			void   ClearBuffers();
			void   SetMode(Mode);
			Cycle  Clock(Cycle);
			void   BeginFrame(Sound::Output*);
			void   EndFrame();
			void   Poke_4017(uint);
			uint   GetLatency() const;
			Result SetSampleRate(dword);
			Result SetSampleBits(uint);
			Result SetSpeed(uint);
			Result SetVolume(uint,uint);
			uint   GetVolume(uint) const;
			void   SetAutoTranspose(bool);
			void   EnableStereo(bool);

			inline void Update();

			class Channel
			{
			public:

				typedef Apu::Sample Sample;

				enum
				{
					MAX_CHANNELS = Apu::MAX_CHANNELS,
					DEFAULT_VOLUME = Apu::DEFAULT_VOLUME
				};

				virtual void Reset() = 0;
				virtual Sample GetSample() = 0;

				void SetContext(Cycle,Cycle,Mode,const u8 (&)[MAX_CHANNELS]);

			protected:

				Channel();

				Mode mode;
				uint fixed;
				ibool active;
				Cycle rate;
				uint outputVolume;

			private:

				virtual void UpdateContext(uint,const u8 (&)[MAX_CHANNELS]) = 0;

			public:

				virtual Cycle Clock()
				{
					return 0;
				}

				uint GetOutputVolume() const
				{
					return outputVolume;
				}
			};

			void HookChannel(Channel*);
			void SaveState(State::Saver&);
			void LoadState(State::Loader&);

			struct DcBlocker
			{
				void Reset();
				Sample Apply(Sample);

				enum
				{
					POLE = 3 // ~0.9999
				};

				iword prev;
				iword next;
				iword acc;
			};

			class LengthCounter
			{
			public:

				void LoadState(State::Loader&);
				void SaveState(State::Saver&) const;

			private:

				enum
				{
					SHIFT_COUNT = 3
				};

				ibool enabled;
				uint count;

				static const uchar lut[32];

			public:

				LengthCounter()
				: enabled(false), count(0) {}

				void Reset()
				{
					enabled = false;
					count = 0;
				}

				void Enable(ibool state)
				{
					enabled = state;

					if (!state)
						count = 0;
				}

				void Write(uint data)
				{
					NST_ASSERT( (data >> SHIFT_COUNT) < sizeof(lut) );

					if (enabled)
						count = lut[data >> SHIFT_COUNT];
				}

				uint GetCount() const
				{
					return count;
				}

				bool Clock()
				{
					return count && !--count;
				}
			};

			class Envelope
			{
			public:

				void Reset();
				void SetOutputVolume(uint);
				void LoadState(State::Loader&);
				void SaveState(State::Saver&) const;

				void Clock();
				void Write(uint);

			private:

				enum
				{
					DECAY_RATE    = b00001111,
					DECAY_DISABLE = b00010000,
					DECAY_LOOP    = b00100000
				};

				enum
				{
					SAVE_0_COUNT         = b00001111,
					SAVE_1_VOLUME        = b00001111,
					SAVE_1_RESET         = b10000000,
					SAVE_1_RESET_SHIFT   = 7,
					SAVE_2_DECAY_RATE    = b00001111,
					SAVE_2_DECAY_DISABLE = b00010000,
					SAVE_2_DECAY_LOOP    = b00100000
				};

				ibool reset;
				uint  count;
				uint  rate;
				ibool loop;
				uint  volume;
				ibool disabled;
				dword output;
				uint  outputVolume;

			public:

				Envelope()
				: outputVolume(OUTPUT_MUL)
				{
					Reset();
				}

				uint Loop() const
				{
					return loop;
				}

				dword Volume() const
				{
					return output;
				}

				void ResetClock()
				{
					reset = true;
				}
			};

		private:

			typedef void (Apu::*Updater)(Cycle);

			inline void Update(Cycle);
			inline void UpdateLatency();

			inline bool NoFrameClockCollision() const;
			void CalculateOscillatorClock(Cycle&,Cycle&) const;

			enum
			{
				RESET_CYCLES = 2048,
				NUM_SQUARES = 2
			};

			enum
			{
				ENABLE_SQUARE1  = 0x01,
				ENABLE_SQUARE2  = 0x02,
				ENABLE_TRIANGLE = 0x04,
				ENABLE_NOISE    = 0x08,
				ENABLE_DMC      = 0x10
			};

			enum
			{
				STATUS_NO_FRAME_IRQ     = b01000000,
				STATUS_SEQUENCE_5_STEP  = b10000000,
				STATUS_FRAME_IRQ_ENABLE = 0,
				STATUS_BITS             = STATUS_NO_FRAME_IRQ|STATUS_SEQUENCE_5_STEP
			};

			enum
			{
				NLN_VOL   = 192,
				NLN_SQ_F  = 900,
				NLN_SQ_0  = 9552UL * OUTPUT_MUL * NLN_VOL * (NLN_SQ_F/100),
				NLN_SQ_1  = 8128UL * OUTPUT_MUL * NLN_SQ_F,
				NLN_SQ_2  = NLN_SQ_F * 100UL,
				NLN_TND_F = 500,
				NLN_TND_0 = 16367UL * OUTPUT_MUL * NLN_VOL * (NLN_TND_F/100),
				NLN_TND_1 = 24329UL * OUTPUT_MUL * NLN_TND_F,
				NLN_TND_2 = NLN_TND_F * 100UL
			};

			NES_DECL_POKE( 4000 )
			NES_DECL_POKE( 4001 )
			NES_DECL_POKE( 4002 )
			NES_DECL_POKE( 4003 )
			NES_DECL_POKE( 4004 )
			NES_DECL_POKE( 4005 )
			NES_DECL_POKE( 4006 )
			NES_DECL_POKE( 4007 )
			NES_DECL_POKE( 4008 )
			NES_DECL_POKE( 400A )
			NES_DECL_POKE( 400B )
			NES_DECL_POKE( 400C )
			NES_DECL_POKE( 400E )
			NES_DECL_POKE( 400F )
			NES_DECL_POKE( 4010 )
			NES_DECL_POKE( 4011 )
			NES_DECL_POKE( 4012 )
			NES_DECL_POKE( 4013 )
			NES_DECL_POKE( 4015 )
			NES_DECL_PEEK( 4015 )
			NES_DECL_PEEK( 4xxx )

			Sample GetSample();

			void SyncOff (Cycle);
			void SyncOn  (Cycle);

			Cycle ClockOscillators();
			void ClockDmc(Cycle);
			void ClockFrameIRQ();

			template<typename T>
			void UpdateBuffer(T);

			void UpdateSettings();

			struct Cycles
			{
				Cycles();

				void Update(dword,uint,Mode);
				void Reset(Mode);

				uint  fixed;
				Cycle rate;
				Cycle rateCounter;
				Cycle frameCounter;
				Cycle extCounter;
				uint  frameDivider;
				uint  frameIrqRepeat;
				Cycle frameIrqClock;
				Cycle dmcClock;

				static const Cycle frame[2];
				static const Cycle frameClocks[2][2][4];
			};

			class Oscillator
			{
			protected:

				Oscillator();

				void Reset();
				void SetContext(Cycle,Cycle);

				ibool active;
				iword timer;
				Cycle rate;
				Cycle frequency;
				dword amp;
				uint fixed;

			public:

				inline void ClearAmp();
			};

			class Square : public Oscillator
			{
			public:

				void Reset();
				void SetContext(dword,uint,uint);

				NST_FORCE_INLINE void WriteReg0(uint);
				NST_FORCE_INLINE void WriteReg1(uint);
				NST_FORCE_INLINE void WriteReg2(uint);
				NST_FORCE_INLINE void WriteReg3(uint,bool);

				inline void Toggle(uint);
				dword GetSample();

				void ClockEnvelope();
				void ClockSweep(uint);

				inline uint GetLengthCounter() const;

				void LoadState(State::Loader&);
				void SaveState(State::Saver&) const;

			private:

				inline bool CanOutput() const;
				bool UpdateFrequency();

				enum
				{
					MIN_FRQ = 0x008,
					MAX_FRQ = 0x7FF
				};

				enum
				{
					REG0_DUTY_SHIFT       = 6,
					REG1_SWEEP_SHIFT      = b00000111,
					REG1_SWEEP_DECREASE   = b00001000,
					REG1_SWEEP_RATE       = b01110000,
					REG1_SWEEP_RATE_SHIFT = 4,
					REG1_SWEEP_ENABLED    = b10000000,
					REG2_WAVELENGTH_LOW   = b11111111,
					REG3_WAVELENGTH_HIGH  = b00000111
				};

				enum
				{
					SAVE_1_WAVELENGTH_LOW  = b11111111,
					SAVE_1_WAVELENGTH_HIGH = b00000111,
					SAVE_1_DUTY            = b01111000,
					SAVE_2_SWEEP_RATE      = b00000111,
					SAVE_2_SWEEP_ENABLE    = b00001000,
					SAVE_2_SWEEP_COUNT     = b01110000,
					SAVE_2_SWEEP_RELOAD    = b10000000,
					SAVE_3_SWEEP_SHIFT     = b00000111,
					SAVE_3_SWEEP_DECREASE  = b00001000
				};

				uint step;
				uint duty;
				LengthCounter lengthCounter;
				Envelope envelope;
				uint sweepCount;
				uint sweepRate;
				uint sweepShift;
				ibool sweepReload;
				uint sweepNegate;
				uint waveLength;
				ibool validFrequency;
			};

			class Triangle : public Oscillator
			{
			public:

				void Reset();
				void SetContext(dword,uint,uint);

				NST_FORCE_INLINE void WriteReg0(uint);
				NST_FORCE_INLINE void WriteReg2(uint);
				NST_FORCE_INLINE void WriteReg3(uint,bool);
				NST_FORCE_INLINE void Toggle(uint);
				NST_FORCE_INLINE dword GetSample();

				void ClockLinearCounter();
				void ClockLengthCounter();

				inline uint GetLengthCounter() const;

				void LoadState(State::Loader&);
				void SaveState(State::Saver&) const;

			private:

				inline bool CanOutput() const;

				enum
				{
					MIN_FRQ = 2 + 1,
					STEP_CHECK = 0x00, // >= 0x1F is technically correct but will produce clicks/pops
					STATUS_COUNTING = 0,
					STATUS_RELOAD
				};

				enum
				{
					REG0_LINEAR_COUNTER_LOAD  = b01111111,
					REG0_LINEAR_COUNTER_START = b10000000,
					REG2_WAVE_LENGTH_LOW      = b11111111,
					REG3_WAVE_LENGTH_HIGH     = b00000111
				};

				enum
				{
					SAVE_0_WAVELENGTH_LOW  = b11111111,
					SAVE_1_WAVELENGTH_HIGH = b00000111,
					SAVE_2_LINEAR_COUNT    = b01111111,
					SAVE_2_LINEAR_STATUS   = b10000000,
					SAVE_3_LINEAR_CTRL     = b11111111
				};

				uint step;
				uint status;
				uint linearCtrl;
				uint linearCounter;
				uint waveLength;
				uint outputVolume;
				LengthCounter lengthCounter;
			};

			class Noise : public Oscillator
			{
			public:

				void Reset();
				void SetContext(dword,uint,uint,Mode);

				NST_FORCE_INLINE void WriteReg0(uint);
				NST_FORCE_INLINE void WriteReg2(uint);
				NST_FORCE_INLINE void WriteReg3(uint,bool);
				NST_FORCE_INLINE void Toggle(uint);
				NST_FORCE_INLINE dword GetSample();

				void ClockEnvelope();
				void ClockLengthCounter();

				inline uint GetLengthCounter() const;

				void LoadState(State::Loader&);
				void SaveState(State::Saver&) const;

			private:

				inline bool CanOutput() const;

				enum
				{
					PULSE = 2
				};

				enum
				{
					REG2_SAMPLE_RATE = b00001111,
					REG2_93BIT_MODE  = b10000000
				};

				enum
				{
					SAVE_WAVELENGTH = b00001111,
					SAVE_93BIT_MODE = b00010000
				};

				uint bits;
				uint shifter;
				Mode mode;
				LengthCounter lengthCounter;
				Envelope envelope;

				static const u16 lut[2][16];
			};

			class Dmc
			{
			public:

				Dmc();

				void Reset(Cpu&);
				void SetContext(uint,Mode);
				NST_FORCE_INLINE void Toggle(uint,Cpu&);

				NST_FORCE_INLINE void WriteReg0(uint,Cpu&);
				NST_FORCE_INLINE void WriteReg1(uint);
				NST_FORCE_INLINE void WriteReg2(uint);
				NST_FORCE_INLINE void WriteReg3(uint);
				NST_FORCE_INLINE dword GetSample();
				NST_FORCE_INLINE void Clock(Cpu&);

				inline uint CheckSample() const;
				inline void ClearAmp();
				inline uint SetSample(uint);
				inline Cycle GetFrequency() const;
				inline uint GetLengthCounter() const;

				void LoadState(State::Loader&,Cpu&,Cycle&);
				void SaveState(State::Saver&,const Cpu&,Cycle) const;

				static Cycle GetResetFrequency(Mode);

			private:

				void OutputBuffer();
				void DoDMA(Cpu&);

				enum
				{
					DMA_CYCLES      = 4,
					REG0_FREQUENCY  = b00001111,
					REG0_LOOP       = b01000000,
					REG0_IRQ_ENABLE = b10000000,
					INP_STEP        = 8
				};

				enum
				{
					SAVE_2_FREQUENCY    = b00001111,
					SAVE_2_LOOP         = b00010000,
					SAVE_2_IRQ          = b00100000,
					SAVE_2_ENABLED      = b01000000,
					SAVE_3_LOAD_ADDRESS = b11111111,
					SAVE_4_LOAD_LENGTH  = b11111111,
					SAVE_5_ADDRESS_LOW  = b11111111,
					SAVE_6_ADDRESS_HIGH = b01111111,
					SAVE_6_BUFFERED     = b10000000,
					SAVE_7_LENGTH       = b11111111,
					SAVE_8_DMA_BUFFER   = b11111111,
					SAVE_9_SHIFTER      = b00000111,
					SAVE_10_OUT_BUFFER  = b11111111,
					SAVE_11_DAC         = b01111111
				};

				struct Out
				{
					uint dac;
					uint shifter;
					uint buffer;
				};

				struct Dma
				{
					uint lengthCounter;
					ibool buffered;
					uint address;
					uint buffer;
				};

				Mode mode;
				ibool loop;
				uint loadedAddress;
				uint loadedLengthCount;
				uint active;
				Cycle frequency;
				Out out;
				Dma dma;
				uint curSample;
				uint linSample;
				uint outputVolume;

				static const Cycle lut[2][16];
			};

			struct Context
			{
				Context();

				dword rate;
				u8 bits;
				u8 speed;
				bool transpose;
				bool stereo;
				bool audible;
				u8 volumes[MAX_CHANNELS];
			};

			Sound::Output* stream;
			uint ctrl;
			Updater updater;
			Cpu& cpu;
			Cycles cycles;
			Mode mode;
			Square square[NUM_SQUARES];
			Triangle triangle;
			Noise noise;
			Dmc dmc;
			Channel* extChannel;
			DcBlocker dcBlocker;
			Sound::Buffer& buffer;
			Context context;

		public:

			dword GetSampleRate() const
			{
				return context.rate;
			}

			uint GetSampleBits() const
			{
				return context.bits;
			}

			uint GetSpeed() const
			{
				return context.speed;
			}

			bool IsAutoTransposing() const
			{
				return context.transpose;
			}

			bool InStereo() const
			{
				return context.stereo;
			}

			bool IsAudible() const
			{
				return context.audible;
			}

			void ReleaseChannel()
			{
				extChannel = NULL;
			}

			void SetExternalClock(Cycle clock)
			{
				NST_ASSERT( extChannel );
				cycles.extCounter = clock;
			}
		};
	}
}
