#include "selector.h"

#include "draw_operations.h"
#include "fmpire_window.h"

namespace fmpire
{

Selector::Selector(Widget* parent) :
	FMpireWidget(parent),
	selected(0),
	hover(-1),
	font_role_size(font_role::BODY),
	callback(nullptr)
{
}

Selector::~Selector() noexcept
{
}

void Selector::add_option(std::string option)
{
	options.push_back(option);
	repaint();
}

void Selector::remove_option(std::string option)
{
	std::remove(options.begin(), options.end(), option);
	repaint();
}

void Selector::clear_options()
{
	options.clear();
	repaint();
}

void Selector::select(const int index, const bool emit_callback)
{
	selected = index;
	if (emit_callback && callback)
	{
		callback->on_selected(this, selected, options[selected]);
	}
	repaint();
}

void Selector::set_callback(Callback* cb)
{
	callback = cb;
}

void Selector::set_font_role(const float role_size)
{
	font_role_size = role_size;
	repaint();
}

void Selector::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	float line_width = theme->line_thin;
	float box_width = (float) getWidth() / options.size();

	if (hover >= 0 && options.size() > 0)
	{
		theme->background.setFor(context);
		float x0 = box_width * hover - line_width / 2;
		x0 = std::max<float>(x0, 0);
		float x1 = box_width * (hover + 1) + line_width / 2;
		x1 = std::min<float>(x1, getWidth());
		Corner corners = Corner::NONE;
		if (hover == 0)
		{
			corners = Corner::LEFT;
		}
		if (hover == options.size() - 1)
		{
			corners = Corner::RIGHT;
		}
		fill_rounded_box(context,
						 x0,
						 0,
						 x1 - x0,
						 getHeight(),
						 theme->corner_radius,
						 line_width,
						 corners);
	}

	if (selected >= 0 && options.size() > 0)
	{
		theme->foreground.setFor(context);
		float x0 = box_width * selected - line_width / 2;
		x0 = std::max<float>(x0, 0);
		float x1 = box_width * (selected + 1) + line_width / 2;
		x1 = std::min<float>(x1, getWidth());

		Corner corners = Corner::NONE;
		if (selected == 0)
		{
			corners = Corner::LEFT;
		}
		if (selected == options.size() - 1)
		{
			corners = Corner::RIGHT;
		}

		fill_rounded_box(context,
						 x0,
						 0,
						 x1 - x0,
						 getHeight(),
						 theme->corner_radius,
						 line_width,
						 corners);
	}
	const float font_size = scaled_font_size(font_role_size, window->get_ui_scale());
	Color(255, 255, 255).setFor(context);
	for (size_t opt = 0; opt < options.size(); opt++)
	{
		float x = box_width * opt + box_width * 0.5;
		draw_text_clipped(context,
						  options[opt].c_str(),
						  theme->font.c_str(),
						  font_size,
						  Anchor::CENTER,
						  x,
						  getHeight() * 0.5,
						  box_width * opt,
						  0,
						  box_width,
						  getHeight());
	}


	theme->foreground.setFor(context);
	for (size_t opt = 1; opt < options.size(); opt++)
	{
		float x0 = box_width * opt;
		Line<float> separator(x0, 0, x0, getHeight());
		separator.draw(context, line_width);
	}
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 line_width);

	if (hover >= 0 && options.size() > 0)
	{
		theme->foreground.setFor(context);
		float x0 = box_width * hover - line_width / 2;
		x0 = std::max<float>(x0, 0);
		float x1 = box_width * (hover + 1) + line_width / 2;
		x1 = std::min<float>(x1, getWidth());
		Corner corners = Corner::NONE;
		if (hover == 0)
		{
			corners = Corner::LEFT;
		}
		if (hover == options.size() - 1)
		{
			corners = Corner::RIGHT;
		}
		draw_rounded_box(context,
						 x0,
						 0,
						 x1 - x0,
						 getHeight(),
						 theme->corner_radius,
						 line_width,
						 corners);
	}

	if (selected >= 0 && options.size() > 0)
	{
		theme->highlight.setFor(context);
		float x0 = box_width * selected - line_width / 2;
		x0 = std::max<float>(x0, 0);
		float x1 = box_width * (selected + 1) + line_width / 2;
		x1 = std::min<float>(x1, getWidth());
		Corner corners = Corner::NONE;
		if (selected == 0)
		{
			corners = Corner::LEFT;
		}
		if (selected == options.size() - 1)
		{
			corners = Corner::RIGHT;
		}
		draw_rounded_box(context,
						 x0,
						 0,
						 x1 - x0,
						 getHeight(),
						 theme->corner_radius,
						 line_width,
						 corners);
	}
}

bool Selector::onMouse(const MouseEvent& event)
{
	if (!contains_clipped(event.pos))
	{
		return false;
	}
	if (event.press)
	{
		float box_width = (float) getWidth() / options.size();
		int index = event.pos.getX() / box_width;
		index = std::min<int>(std::max<int>(index, 0), options.size() - 1);
		select(index, true);
		return true;
	}
	return false;
}

bool Selector::onMotion(const MotionEvent& event)
{
	if (contains_clipped(event.pos))
	{
		float box_width = (float) getWidth() / options.size();
		hover = event.pos.getX() / box_width;
		hover = std::min<int>(std::max<int>(hover, 0), options.size() - 1);
		repaint();
		return true;
	}
	else if (!contains_clipped(event.pos) && hover >= 0)
	{
		hover = -1;
		repaint();
		return true;
	}
	return false;
}


} // namespace fmpire
