#include "button.h"

#include "draw_operations.h"
#include "fmpire_window.h"

namespace fmpire
{
Button::Button(Widget* parent) :
	FMpireWidget(parent),
	state(State::NORMAL),
	is_toggle(false),
	text(""),
	callback(nullptr),
	is_drawing_normal_bg(false)
{
}

Button::~Button() noexcept
{
}

void Button::set_toggle(const bool toggle)
{
	is_toggle = toggle;
}

void Button::set_text(const std::string& txt)
{
	text = txt;
	repaint();
}

void Button::set_enabled(const bool enabled)
{
	if (state == State::DISABLED && enabled)
	{
		state = State::NORMAL;
		repaint();
	}
	else if (state != State::DISABLED && !enabled)
	{
		state = State::DISABLED;
		repaint();
	}
}

void Button::set_pressed(const bool pressed)
{
	if (pressed)
	{
		state = State::PRESS;
		repaint();
	}
	else if (!pressed && state == State::PRESS)
	{
		state = State::NORMAL;
		repaint();
	}
}

void Button::set_callback(Callback* cb)
{
	callback = cb;
}

void Button::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	bool draw_bg = false;
	switch (state)
	{
	case State::DISABLED:
	case State::NORMAL:
		if (is_drawing_normal_bg)
		{
			draw_bg = true;
			theme->background.setFor(context);
		}
		break;
	case State::HOVER:
		draw_bg = true;
		theme->foreground.setFor(context);
		break;
	case State::PRESS:
		draw_bg = true;
		theme->highlight.setFor(context);
		break;
	}
	float line_width = theme->line_thin;
	if (draw_bg)
	{
		fill_rounded_box(context,
						 0,
						 0,
						 getWidth(),
						 getHeight(),
						 theme->corner_radius,
						 line_width);
	}

	switch (state)
	{
	case State::DISABLED:
		theme->background.setFor(context);
		break;
	case State::NORMAL:
		theme->foreground.setFor(context);
		break;
	case State::HOVER:
		theme->highlight.setFor(context);
		break;
	case State::PRESS:
		theme->highlight.setFor(context);
		break;
	}

	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 line_width);

	Color(255, 255, 255).setFor(context);
	const float text_size = scaled_font_size(font_role::BODY, window->get_ui_scale());
	draw_text_clipped(context,
					  text.c_str(),
					  theme->font.c_str(),
					  text_size,
					  Anchor::CENTER,
					  getWidth() / 2,
					  getHeight() / 2,
					  0,
					  0,
					  getWidth(),
					  getHeight());
}

bool Button::onMouse(const MouseEvent& event)
{
	if (state == State::DISABLED)
	{
		return false;
	}
	if (contains_clipped(event.pos) && event.button == 1 && event.press)
	{
		bool is_press = true;
		if (state != State::PRESS)
		{
			state = State::PRESS;
		}
		else if (is_toggle)
		{
			state = State::NORMAL;
			is_press = false;
		}

		repaint();

		if (callback && is_press)
		{
			callback->on_press(this);
		}
		else if (callback && !is_press)
		{
			callback->on_release(this);
		}
		return true;
	}
	else if (state == State::PRESS && event.button == 1 && !event.press
			 && !is_toggle)
	{
		state = State::NORMAL;
		repaint();
		if (callback)
		{
			callback->on_release(this);
		}
		return true;
	}
	return false;
}

bool Button::onMotion(const MotionEvent& event)
{
	if (!is_toggle && state == State::PRESS)
	{
		return true;
	}
	if (contains_clipped(event.pos))
	{
		if (state == State::NORMAL)
		{
			state = State::HOVER;
			repaint();
		}
		return true;
	}
	if (!contains_clipped(event.pos) && state == State::HOVER)
	{
		state = State::NORMAL;
		repaint();
	}
	return false;
}


} // namespace fmpire
