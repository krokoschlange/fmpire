#include "text_entry.h"
#include "cairo.h"
#include "Cairo.hpp"
#include "draw_operations.h"
#include "fmpire_ui.h"
#include <codecvt>
#include <iostream>
#include <locale>

namespace fmpire
{

TextEntry::TextEntry(Widget* parent) :
	FMpireWidget(parent),
	cursor_position(-1)
{
}

TextEntry::~TextEntry() noexcept
{
}

void TextEntry::set_text(const std::string& text)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	value = converter.from_bytes(text.data());
}

std::string TextEntry::get_text_utf8() const
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(value);
}

void TextEntry::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	theme->background.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 theme->line_very_thin);

	draw_text(context,
			  get_text_utf8().c_str(),
			  theme->font.c_str(),
			  getHeight() * 0.75,
			  Anchor::BOTTOM_LEFT,
			  5,
			  getHeight() * 0.875);

	if (static_cast<FMpireUI*>(getTopLevelWidget())->get_focus() == this
		&& cursor_position >= 0)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		float text_width, text_height, bearing_x, bearing_y;
		get_text_rect(
			context,
			converter.to_bytes(value.substr(0, cursor_position)).c_str(),
			theme->font.c_str(),
			getHeight() * 0.75,
			text_width,
			text_height,
			bearing_x,
			bearing_y);

		Color(1.0f, 1.0f, 1.0f).setFor(context);
		Line<float> line(5 + text_width + bearing_x,
						 getHeight() * 0.125,
						 5 + text_width + bearing_x,
						 getHeight() * 0.875);
		line.draw(context, theme->line_thin);
	}
}

bool TextEntry::onMouse(const MouseEvent& event)
{
	if (event.press && contains_clipped(event.pos))
	{
		static_cast<FMpireUI*>(getTopLevelWidget())->set_focus(this);

		float text_width = 0.0f, text_height, bearing_x, bearing_y;
		cursor_position = 0;
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

		float text_click_pos = event.pos.getX() - 5;

		cairo_surface_t* temp_surface =
			cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
		cairo_t* temp_cairo = cairo_create(temp_surface);
		CairoGraphicsContext context;
		context.handle = temp_cairo;


		for (uint32_t i = 1; i <= value.size(); i++)
		{
			float prev_width = text_width;
			get_text_rect(context,
						  converter.to_bytes(value.substr(0, i)).c_str(),
						  theme->font.c_str(),
						  getHeight() * 0.75,
						  text_width,
						  text_height,
						  bearing_x,
						  bearing_y);

			text_width += bearing_x;

			if (text_width > text_click_pos)
			{
				if (text_width - text_click_pos < text_click_pos - prev_width)
				{
					cursor_position++;
				}
				break;
			}
			cursor_position++;
		}
		cairo_destroy(temp_cairo);
		cairo_surface_destroy(temp_surface);

		repaint();

		return true;
	}
	return false;
}

bool is_printable_character(uint32_t character)
{
	if (character <= 0x1F || character == 0x7F)
	{
		return false;
	}

	if (character >= 0x80 && character <= 0x9F)
	{
		return false;
	}

	if (character == 0xFEFF)
	{
		return false;
	}

	return true;
}

bool TextEntry::onCharacterInput(const CharacterInputEvent& event)
{
	if (static_cast<FMpireUI*>(getTopLevelWidget())->get_focus() != this)
	{
		return false;
	}

	switch (event.character)
	{
	case 0x8:
		if (cursor_position > 0 && cursor_position <= value.size())
		{
			value.erase(cursor_position - 1, 1);
			cursor_position--;

			if (callback)
			{
				callback->on_value_changed(this, value);
			}
		}
		break;
	case 0xFF:
		if (cursor_position >= 0 && cursor_position < value.size())
		{
			value.erase(cursor_position, 1);

			if (callback)
			{
				callback->on_value_changed(this, value);
			}
		}
		break;
	default:
	{
		if (is_printable_character(event.character))
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			std::wstring wstr = converter.from_bytes(event.string);
			uint32_t insert_pos =
				std::min<uint32_t>(std::max<int>(cursor_position, 0),
								   value.size());

			value.insert(insert_pos, wstr);
			cursor_position++;

			if (callback)
			{
				callback->on_value_changed(this, value);
			}
		}
		break;
	}
	}

	repaint();

	return true;
}

} // namespace fmpire
