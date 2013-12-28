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

#include "NstDialogVideoDecoder.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		IDC_VIDEO_DECODER_GY_VALUE == IDC_VIDEO_DECODER_RY_VALUE + 1 &&
		IDC_VIDEO_DECODER_BY_VALUE == IDC_VIDEO_DECODER_RY_VALUE + 2
	);

	NST_COMPILE_ASSERT
	(
		IDC_VIDEO_DECODER_GY_NUM == IDC_VIDEO_DECODER_RY_NUM + 1 &&
		IDC_VIDEO_DECODER_BY_NUM == IDC_VIDEO_DECODER_RY_NUM + 2
	);

	NST_COMPILE_ASSERT
	(
		IDC_VIDEO_DECODER_GY_GAIN == IDC_VIDEO_DECODER_RY_GAIN + 1 &&
		IDC_VIDEO_DECODER_BY_GAIN == IDC_VIDEO_DECODER_RY_GAIN + 2
	);

	using namespace Window;

	struct VideoDecoder::Handlers
	{
		static const MsgHandler::Entry<VideoDecoder> messages[];
		static const MsgHandler::Entry<VideoDecoder> commands[];
	};

	const MsgHandler::Entry<VideoDecoder> VideoDecoder::Handlers::messages[] =
	{
		{ WM_INITDIALOG,  &VideoDecoder::OnInitDialog },
		{ WM_HSCROLL,     &VideoDecoder::OnHScroll    }
	};

	const MsgHandler::Entry<VideoDecoder> VideoDecoder::Handlers::commands[] =
	{
		{ IDC_VIDEO_DECODER_RY_GAIN,      &VideoDecoder::OnCmdGain        },
		{ IDC_VIDEO_DECODER_GY_GAIN,      &VideoDecoder::OnCmdGain        },
		{ IDC_VIDEO_DECODER_BY_GAIN,      &VideoDecoder::OnCmdGain        },
		{ IDC_VIDEO_DECODER_BOOST_YELLOW, &VideoDecoder::OnCmdBoostYellow },
		{ IDC_VIDEO_DECODER_CANONICAL ,   &VideoDecoder::OnCmdPreset      },
		{ IDC_VIDEO_DECODER_CONSUMER,     &VideoDecoder::OnCmdPreset      },
		{ IDC_VIDEO_DECODER_ALTERNATIVE,  &VideoDecoder::OnCmdPreset      },
		{ IDC_VIDEO_DECODER_OK,           &VideoDecoder::OnCmdOk          },
		{ IDC_VIDEO_DECODER_CANCEL,       &VideoDecoder::OnCmdCancel      }
	};

	VideoDecoder::VideoDecoder(Nes::Video e,ibool ntsc)
	:
	dialog          (IDD_VIDEO_DECODER,this,Handlers::messages,Handlers::commands),
	nes             (e),
	usingNtscFilter (ntsc),
	final           (e.GetDecoder())
	{
	}

	VideoDecoder::~VideoDecoder()
	{
		nes.SetDecoder( final );
	}

	ibool VideoDecoder::OnInitDialog(Param&)
	{
		for (uint i=0; i < 3; ++i)
		{
			dialog.Slider( IDC_VIDEO_DECODER_RY_VALUE+i ).SetRange( 0, 60 );
			dialog.Edit( IDC_VIDEO_DECODER_RY_GAIN+i ).Limit( 5 );
		}

		dialog.Control( IDC_VIDEO_DECODER_BOOST_YELLOW ).Enable( !usingNtscFilter );
		dialog.Control( IDC_VIDEO_DECODER_ALTERNATIVE ).Enable( !usingNtscFilter );

		Update();

		return true;
	}

	ibool VideoDecoder::OnHScroll(Param& param)
	{
		for (uint i=0; i < 3; ++i)
		{
			if (param.Slider().IsControl( IDC_VIDEO_DECODER_RY_VALUE+i ))
			{
				Nes::Api::Video::Decoder decoder( nes.GetDecoder() );

				static const ushort offsets[3] = {60, 200, 330};
				uint angle = dialog.Slider( IDC_VIDEO_DECODER_RY_VALUE+i ).Position() + offsets[i];

				if (angle >= 360)
					angle -= 360;

				if (decoder.axes[i].angle != angle)
				{
					decoder.axes[i].angle = angle;
					nes.SetDecoder( decoder );
					dialog.Edit( IDC_VIDEO_DECODER_RY_NUM+i ) << angle;
					Redraw();
				}

				return true;
			}
		}

		return true;
	}

	ibool VideoDecoder::OnCmdGain(Param& param)
	{
		if (param.Edit().Changed())
		{
			String::Heap<char> string;
			dialog.Edit( param.Edit().GetId() ) >> string;

			float gain = std::atof( string.Ptr() );
			gain = NST_CLAMP(gain,0.0f,2.0f);

			Nes::Api::Video::Decoder decoder( nes.GetDecoder() );
			decoder.axes[param.Edit().GetId() - IDC_VIDEO_DECODER_RY_GAIN].gain = gain;

			if (nes.SetDecoder( decoder ) != Nes::RESULT_NOP)
				Redraw();
		}

		return true;
	}

	ibool VideoDecoder::OnCmdBoostYellow(Param& param)
	{
		if (param.Button().IsClicked())
		{
			Nes::Api::Video::Decoder decoder( nes.GetDecoder() );
			decoder.boostYellow = dialog.CheckBox(IDC_VIDEO_DECODER_BOOST_YELLOW).IsChecked();
			nes.SetDecoder( decoder );
			Redraw();
		}

		return true;
	}

	ibool VideoDecoder::OnCmdPreset(Param& param)
	{
		if (param.Button().IsClicked())
		{
			Nes::Api::Video::DecoderPreset preset;

			switch (param.Button().GetId())
			{
				case IDC_VIDEO_DECODER_CONSUMER:    preset = Nes::Api::Video::DECODER_CONSUMER;    break;
				case IDC_VIDEO_DECODER_ALTERNATIVE: preset = Nes::Api::Video::DECODER_ALTERNATIVE; break;
				default:                            preset = Nes::Api::Video::DECODER_CANONICAL;   break;
			}

			nes.SetDecoder( preset );
			Update();
			Redraw();
		}

		return true;
	}

	ibool VideoDecoder::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return true;
	}

	ibool VideoDecoder::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			final = nes.GetDecoder();
			dialog.Close();
		}

		return true;
	}

	void VideoDecoder::Update() const
	{
		Nes::Api::Video::Decoder decoder( nes.GetDecoder() );

		for (uint i=0; i < 3; ++i)
		{
			int angle = decoder.axes[i].angle;

			dialog.Edit( IDC_VIDEO_DECODER_RY_NUM+i ) << uint(angle);
			static const short offsets[3] = {60, 200, 330};

			angle -= offsets[i];

			if (angle < 0)
				angle += 360;

			dialog.Slider( IDC_VIDEO_DECODER_RY_VALUE+i ).Position() = uint(angle);

			tchar buffer[32];
			::_stprintf( buffer, _T("%.3f"), decoder.axes[i].gain );

			dialog.Edit( IDC_VIDEO_DECODER_RY_GAIN+i ).Text() << buffer;
		}

		dialog.CheckBox( IDC_VIDEO_DECODER_BOOST_YELLOW ).Check( decoder.boostYellow );
	}

	void VideoDecoder::Redraw() const
	{
		::RedrawWindow( ::GetParent(::GetParent( dialog )), NULL, NULL, RDW_INVALIDATE );
	}
}

