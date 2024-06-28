#include "selector.h"

#include "draw_operations.h"

namespace fmpire
{

Selector::Selector(Widget* parent) :
	FMpireWidget(parent),
	selected(0),
	hover(-1),
	text_y_scale(0.5),
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

void Selector::set_text_y_scale(const float scale)
{
	text_y_scale = scale;
	repaint();
}

void Selector::set_callback(Callback* cb)
{
	callback = cb;
}

void Selector::onDisplay()
{
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
	float font_size = getHeight() * text_y_scale;
	for (size_t opt = 0; opt < options.size(); opt++)
	{
		float current_size = fit_text(context,
									  options[opt].c_str(),
									  theme->font.c_str(),
									  box_width,
									  getHeight() * text_y_scale);
		font_size = std::min(font_size, current_size);
	}
	Color(255, 255, 255).setFor(context);
	for (size_t opt = 0; opt < options.size(); opt++)
	{
		float x = box_width * opt + box_width * 0.5;
		draw_text(context,
				  options[opt].c_str(),
				  theme->font.c_str(),
				  font_size,
				  Anchor::CENTER,
				  x,
				  getHeight() * 0.5);
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
	if (!contains(event.pos))
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
	if (contains(event.pos))
	{
		float box_width = (float) getWidth() / options.size();
		hover = event.pos.getX() / box_width;
		hover = std::min<int>(std::max<int>(hover, 0), options.size() - 1);
		repaint();
		return true;
	}
	else if (!contains(event.pos) && hover >= 0)
	{
		hover = -1;
		repaint();
		return true;
	}
	return false;
}


} // namespace fmpire
