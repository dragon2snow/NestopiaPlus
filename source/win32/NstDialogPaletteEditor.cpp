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

#include <utility>
#include "NstDialogPaletteEditor.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		IDC_PALETTE_EDITOR_G_SLIDER == IDC_PALETTE_EDITOR_R_SLIDER + 1 &&
		IDC_PALETTE_EDITOR_B_SLIDER == IDC_PALETTE_EDITOR_R_SLIDER + 2
	);

	NST_COMPILE_ASSERT
	(
		IDC_PALETTE_EDITOR_G_VALUE == IDC_PALETTE_EDITOR_R_VALUE + 1 &&
		IDC_PALETTE_EDITOR_B_VALUE == IDC_PALETTE_EDITOR_R_VALUE + 2
	);

	using namespace Window;

	struct PaletteEditor::Handlers
	{
		static const MsgHandler::Entry<PaletteEditor> messages[];
		static const MsgHandler::Entry<PaletteEditor> commands[];
	};

	const MsgHandler::Entry<PaletteEditor> PaletteEditor::Handlers::messages[] =
	{
		{ WM_INITDIALOG,  &PaletteEditor::OnInitDialog  },
		{ WM_PAINT,       &PaletteEditor::OnPaint       },
		{ WM_LBUTTONDOWN, &PaletteEditor::OnLButtonDown },
		{ WM_HSCROLL,     &PaletteEditor::OnHScroll     }
	};

	const MsgHandler::Entry<PaletteEditor> PaletteEditor::Handlers::commands[] =
	{
		{ IDC_PALETTE_EDITOR_UNDO,   &PaletteEditor::OnCmdUndo   },
		{ IDC_PALETTE_EDITOR_REDO,   &PaletteEditor::OnCmdRedo   },
		{ IDC_PALETTE_EDITOR_RESET,  &PaletteEditor::OnCmdReset  },
		{ IDC_PALETTE_EDITOR_SAVE,   &PaletteEditor::OnCmdSave   },
		{ IDC_PALETTE_EDITOR_CANCEL, &PaletteEditor::OnCmdCancel },
		{ IDC_PALETTE_EDITOR_CUSTOM, &PaletteEditor::OnCmdMode   },
		{ IDC_PALETTE_EDITOR_YUV,    &PaletteEditor::OnCmdMode   },
		{ IDC_PALETTE_EDITOR_RGB,    &PaletteEditor::OnCmdMode   }
	};

	PaletteEditor::Settings::Settings(Nes::Video emulator)
	{
		mode = emulator.GetPalette().GetMode();
		brightness = emulator.GetBrightness();
		saturation = emulator.GetSaturation();
		hue = emulator.GetHue();

		emulator.GetPalette().SetMode( Nes::Video::Palette::MODE_CUSTOM );
		emulator.SetBrightness( 128 );
		emulator.SetSaturation( 128 );
		emulator.SetHue( 128 );

		std::memcpy( palette, emulator.GetPalette().GetColors(), 64 * 3 );
	}

	void PaletteEditor::Settings::Restore(Nes::Video emulator) const
	{
		emulator.GetPalette().SetMode( mode );
		emulator.SetBrightness( brightness );
		emulator.SetSaturation( saturation );
		emulator.SetHue( hue );
		emulator.GetPalette().SetCustom( palette );
	}

	PaletteEditor::History::History()
	{
		Reset();
	}

	void PaletteEditor::History::Reset()
	{
		pos = 0;
		data[0][0] = STOP;
		data[LENGTH-1][0] = STOP;
	}

	void PaletteEditor::History::Add(uint index,uint color)
	{
		data[pos][0] = index;
		data[pos][1] = color;
		pos = (pos+1) & (LENGTH-1);
		data[pos][0] = STOP;
	}

	ibool PaletteEditor::History::CanUndo() const
	{
		return data[(pos-1) & (LENGTH-1)][0] != STOP;
	}

	ibool PaletteEditor::History::CanRedo() const
	{
		return data[pos][0] != STOP;
	}

	uint PaletteEditor::History::Undo(Nes::Video emulator)
	{
		NST_ASSERT( CanUndo() );

		u8 palette[64][3];
		std::memcpy( palette, emulator.GetPalette().GetColors(), 64*3 );

		pos = (pos-1) & (LENGTH-1);
		const uint index = data[pos][0];
		std::swap( palette[index / 3][index % 3], data[pos][1] );

		emulator.GetPalette().SetCustom( palette );

		return index / 3;
	}

	uint PaletteEditor::History::Redo(Nes::Video emulator)
	{
		NST_ASSERT( CanRedo() );

		u8 palette[64][3];
		std::memcpy( palette, emulator.GetPalette().GetColors(), 64*3 );

		const uint index = data[pos][0];
		std::swap( palette[index / 3][index % 3], data[pos][1] );
		pos = (pos+1) & (LENGTH-1);

		emulator.GetPalette().SetCustom( palette );

		return index / 3;
	}

	PaletteEditor::PaletteEditor(Nes::Video& e,const Managers::Paths& p,const Path& d)
	:
	dialog         (IDD_PALETTE_EDITOR,this,Handlers::messages,Handlers::commands),
	emulator       (e),
	colorSelect    (0),
	paths          (p),
	path           (d),
	sliderDragging (false),
	settings       (e)
	{
	}

	PaletteEditor::~PaletteEditor()
	{
		settings.Restore( emulator );
	}

	ibool PaletteEditor::OnInitDialog(Param&)
	{
		for (uint i=0; i < 3; ++i)
			dialog.Slider( IDC_PALETTE_EDITOR_R_SLIDER+i ).SetRange( 0, 255 );

		dialog.RadioButton( IDC_PALETTE_EDITOR_CUSTOM ).Check();
		UpdateMode( true );

		return true;
	}

	ibool PaletteEditor::OnPaint(Param& param)
	{
		if (HDC const hdc = ::GetDC( param.hWnd ))
		{
			class Bitmap : BITMAPINFO
			{
				RGBQUAD nesColor;
				u8 pixels[BMP_COLOR_WIDTH * BMP_COLOR_HEIGHT];

			public:

				Bitmap()
				{
					std::memset( this, 0, sizeof(*this) );

					bmiHeader.biSize = sizeof(bmiHeader);
					bmiHeader.biWidth = BMP_COLOR_WIDTH;
					bmiHeader.biHeight = BMP_COLOR_HEIGHT;
					bmiHeader.biPlanes = 1;
					bmiHeader.biBitCount = 8;
					bmiHeader.biCompression = BI_RGB;

					for (uint y=BMP_COLOR_WIDTH+1; y < BMP_COLOR_WIDTH * (BMP_COLOR_HEIGHT-2); y += BMP_COLOR_WIDTH)
						std::memset( pixels + y, 1, BMP_COLOR_WIDTH-2 );
				}

				void Draw(HDC const hdc,const u8 (*NST_RESTRICT palette)[3],const uint selected)
				{
					const RGBQUAD selectColors[2] =
					{
						{BMP_COLOR_UNSELECT,BMP_COLOR_UNSELECT,BMP_COLOR_UNSELECT,0},
						{BMP_COLOR_SELECT,BMP_COLOR_SELECT,BMP_COLOR_SELECT,0}
					};

					for (uint y=0; y < BMP_COLUMNS; ++y)
					{
						for (uint x=0; x < BMP_ROWS; ++x)
						{
							const uint index = y * BMP_ROWS + x;

							*bmiColors = selectColors[selected == index];

							nesColor.rgbRed   = palette[index][0];
							nesColor.rgbGreen = palette[index][1];
							nesColor.rgbBlue  = palette[index][2];

							::SetDIBitsToDevice
							(
								hdc,
								BMP_START_X + x * BMP_COLOR_WIDTH,
								BMP_START_Y + y * BMP_COLOR_HEIGHT,
								BMP_COLOR_WIDTH,
								BMP_COLOR_HEIGHT,
								0,
								0,
								0,
								BMP_COLOR_HEIGHT,
								&pixels,
								this,
								DIB_RGB_COLORS
							);
						}
					}
				}
			};

			Bitmap bitmap;
			bitmap.Draw( hdc, emulator.GetPalette().GetColors(), colorSelect );

			::ReleaseDC( param.hWnd, hdc );
			::RedrawWindow( ::GetParent(::GetParent( param.hWnd )), NULL, NULL, RDW_INVALIDATE );
		}

		return false;
	}

	ibool PaletteEditor::OnLButtonDown(Param& param)
	{
		const Point point( LOWORD(param.lParam), HIWORD(param.lParam) );

		if (point.x >= BMP_START_X && point.x < BMP_END_X && point.y >= BMP_START_Y && point.y < BMP_END_Y)
		{
			colorSelect = ((point.y-BMP_START_Y) / BMP_COLOR_HEIGHT * BMP_ROWS) + ((point.x-BMP_START_X) / BMP_COLOR_WIDTH);
			UpdateColor();
			dialog.Redraw();
			return true;
		}

		return true;
	}

	ibool PaletteEditor::OnHScroll(Param& param)
	{
		uint index;

		if (param.Slider().IsControl( IDC_PALETTE_EDITOR_R_SLIDER ))
		{
			index = 0;
		}
		else if (param.Slider().IsControl( IDC_PALETTE_EDITOR_G_SLIDER ))
		{
			index = 1;
		}
		else if (param.Slider().IsControl( IDC_PALETTE_EDITOR_B_SLIDER ))
		{
			index = 2;
		}
		else
		{
			return false;
		}

		const uint color = dialog.Slider( IDC_PALETTE_EDITOR_R_SLIDER+index ).Position();
		const u8 (*NST_RESTRICT colors)[3] = emulator.GetPalette().GetColors();

		if (color != colors[colorSelect][index])
		{
			if (!sliderDragging)
			{
				sliderDragging = true;
				history.Add( colorSelect * 3 + index, colors[colorSelect][index] );
			}

			dialog.Control( IDC_PALETTE_EDITOR_R_VALUE+index ).Text() << color;

			{
				u8 custom[64][3];
				std::memcpy( custom, colors, 64 * 3 );
				custom[colorSelect][index] = color;
				emulator.GetPalette().SetCustom( custom );
			}

			dialog.Redraw();
		}

		if (sliderDragging && param.Slider().Released())
		{
			sliderDragging = false;
			dialog.Control( IDC_PALETTE_EDITOR_REDO ).Disable();
			dialog.Control( IDC_PALETTE_EDITOR_UNDO ).Enable();
		}

		return true;
	}

	ibool PaletteEditor::OnCmdUndo(Param& param)
	{
		if (param.Button().IsClicked())
		{
			colorSelect = history.Undo( emulator );
			dialog.Control( IDC_PALETTE_EDITOR_REDO ).Enable();
			dialog.Control( IDC_PALETTE_EDITOR_UNDO ).Enable( history.CanUndo() );
			UpdateColor();
			dialog.Redraw();
		}

		return true;
	}

	ibool PaletteEditor::OnCmdRedo(Param& param)
	{
		if (param.Button().IsClicked())
		{
			colorSelect = history.Redo( emulator );
			dialog.Control( IDC_PALETTE_EDITOR_UNDO ).Enable();
			dialog.Control( IDC_PALETTE_EDITOR_REDO ).Enable( history.CanRedo() );
			UpdateColor();
			dialog.Redraw();
		}

		return true;
	}

	ibool PaletteEditor::OnCmdReset(Param& param)
	{
		if (param.Button().IsClicked())
		{
			history.Reset();
			emulator.GetPalette().SetCustom( settings.palette );
			UpdateMode( true );
		}

		return true;
	}

	ibool PaletteEditor::OnCmdSave(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const Path tmp( paths.BrowseSave( Managers::Paths::File::PALETTE, Managers::Paths::SUGGEST, path ) );

			if (tmp.Length())
			{
				paths.Save( emulator.GetPalette().GetColors(), 64 * 3, Managers::Paths::File::PALETTE, tmp );
				path = tmp;
				dialog.Close();
			}
		}

		return true;
	}

	ibool PaletteEditor::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return true;
	}

	ibool PaletteEditor::OnCmdMode(Param& param)
	{
		if (param.Button().IsClicked())
			UpdateMode();

		return true;
	}

	void PaletteEditor::UpdateMode(const ibool forceUpdate)
	{
		Nes::Video::Palette::Mode mode;

		if (dialog.RadioButton(IDC_PALETTE_EDITOR_CUSTOM).IsChecked())
		{
			mode = Nes::Video::Palette::MODE_CUSTOM;
		}
		else if (dialog.RadioButton(IDC_PALETTE_EDITOR_YUV).IsChecked())
		{
			mode = Nes::Video::Palette::MODE_YUV;
		}
		else
		{
			mode = Nes::Video::Palette::MODE_RGB;
		}

		if (emulator.GetPalette().SetMode( mode ) != Nes::RESULT_NOP || forceUpdate)
		{
			const ibool custom = (mode == Nes::Video::Palette::MODE_CUSTOM);

			for (uint i=0; i < 3; ++i)
				dialog.Slider( IDC_PALETTE_EDITOR_R_SLIDER+i ).Enable( custom );

			dialog.Control( IDC_PALETTE_EDITOR_UNDO ).Enable( custom && history.CanUndo() );
			dialog.Control( IDC_PALETTE_EDITOR_REDO ).Enable( custom && history.CanRedo() );
			dialog.Control( IDC_PALETTE_EDITOR_RESET ).Enable( custom );

			UpdateColor();
			dialog.Redraw();
		}
	}

	void PaletteEditor::UpdateColor()
	{
		const u8 (&colors)[3] = emulator.GetPalette().GetColors()[colorSelect];

		for (uint i=0; i < 3; ++i)
		{
			const uint color = colors[i];
			dialog.Slider( IDC_PALETTE_EDITOR_R_SLIDER+i ).Position() = color;
			dialog.Control( IDC_PALETTE_EDITOR_R_VALUE+i ).Text() << color;
		}
	}
}

