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
	cursor_position(-1),
	scroll_offset(0.0f)
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

float TextEntry::get_text_advance(const std::wstring& text) const
{
	if (text.empty())
	{
		return 0.0f;
	}

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	float width, height, bearing_x, bearing_y;

	cairo_surface_t* temp_surface =
		cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t* temp_cairo = cairo_create(temp_surface);
	CairoGraphicsContext context;
	context.handle = temp_cairo;

	get_text_rect(context,
				  converter.to_bytes(text).c_str(),
				  theme->font.c_str(),
				  getHeight() * 0.75,
				  width,
				  height,
				  bearing_x,
				  bearing_y);

	cairo_destroy(temp_cairo);
	cairo_surface_destroy(temp_surface);

	return width + bearing_x;
}

void TextEntry::update_scroll()
{
	if (cursor_position < 0)
	{
		scroll_offset = 0.0f;
		return;
	}

	const float padding = 5.0f;
	const float visible_width = std::max(getWidth() - 2 * padding, 0.0f);
	const float cursor_x = get_text_advance(value.substr(0, cursor_position));

	if (cursor_x - scroll_offset < 0.0f)
	{
		scroll_offset = cursor_x;
	}
	else if (cursor_x - scroll_offset > visible_width)
	{
		scroll_offset = cursor_x - visible_width;
	}

	if (scroll_offset < 0.0f)
	{
		scroll_offset = 0.0f;
	}
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

	Color(255, 255, 255).setFor(context);
	draw_text(context,
			  get_text_utf8().c_str(),
			  theme->font.c_str(),
			  getHeight() * 0.75,
			  Anchor::LEFT_CENTER,
			  5 - scroll_offset,
			  getHeight() * 0.5);

	if (static_cast<FMpireUI*>(getTopLevelWidget())->get_focus() == this
		&& cursor_position >= 0)
	{
		const float cursor_x =
			5 - scroll_offset
			+ get_text_advance(value.substr(0, cursor_position));

		Color(1.0f, 1.0f, 1.0f).setFor(context);
		Line<float> line(cursor_x,
						 getHeight() * 0.125,
						 cursor_x,
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

		float text_click_pos = event.pos.getX() - 5 + scroll_offset;

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

		update_scroll();
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

	update_scroll();
	repaint();

	return true;
}

bool TextEntry::onKeyboard(const KeyboardEvent& event)
{
	if (static_cast<FMpireUI*>(getTopLevelWidget())->get_focus() != this)
	{
		return false;
	}

	if (!event.press)
	{
		return false;
	}

	switch (event.key)
	{
	case kKeyLeft:
		if (cursor_position > 0)
		{
			cursor_position--;
		}
		break;
	case kKeyRight:
		if (cursor_position >= 0 && cursor_position < (int) value.size())
		{
			cursor_position++;
		}
		break;
	case kKeyHome:
		if (cursor_position > 0)
		{
			cursor_position = 0;
		}
		break;
	case kKeyEnd:
		if (cursor_position >= 0)
		{
			cursor_position = (int) value.size();
		}
		break;
	case kKeyDelete:
		if (cursor_position >= 0 && cursor_position < (int) value.size())
		{
			value.erase(cursor_position, 1);

			if (callback)
			{
				callback->on_value_changed(this, value);
			}
		}
		break;
	default:
		return false;
	}

	update_scroll();
	repaint();

	return true;
}

} // namespace fmpire
