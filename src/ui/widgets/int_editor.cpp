#include "int_editor.h"

#include "double_click.h"
#include "draw_operations.h"

namespace fmpire
{

IntEditor::IntEditor(Widget* parent) :
	FMpireWidget(parent),
	value(0),
	default_value(0),
	stored_value(0),
	min_value(0),
	max_value(10),
	dragging(false),
	scroll_value(0),
	scroll_speed(0.1),
	hover_state(MouseState::NONE),
	press_state(MouseState::NONE),
	callback(nullptr)
{
}

IntEditor::~IntEditor() noexcept
{
}

void IntEditor::set_value(const int val, const bool emit_callback)
{
	value = std::clamp(val, min_value, max_value);
	if (callback && emit_callback)
	{
		callback->on_value_changed(this, value);
	}
	repaint();
}

void IntEditor::set_default_value(const int val)
{
	default_value = val;
}

void IntEditor::set_limits(const int min, const int max)
{
	min_value = min;
	max_value = max;
	set_value(value);
}

void IntEditor::set_tooltip(const std::string& text)
{
	tooltip = text;
}

void IntEditor::set_label(const std::string& text)
{
	label = text;
	repaint();
}

void IntEditor::set_callback(Callback* const cb)
{
	callback = cb;
}

void IntEditor::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();

	float radius = theme->corner_radius;
	float line_width = theme->line_thin;
	theme->background.setFor(context);

	float box_top = 0.05 * getHeight();
	float box_bottom = 0.75 * getHeight();
	float box_center = (box_top + box_bottom) / 2;
	float box_height = box_bottom - box_top;

	switch (hover_state)
	{
	case MouseState::REDUCE:
		fill_rounded_box(context,
						 0,
						 box_top,
						 getWidth() * 0.2,
						 box_height,
						 radius,
						 line_width,
						 Corner::LEFT);
		break;
	case MouseState::CENTER:
		fill_rounded_box(context,
						 getWidth() * 0.2 - line_width,
						 box_top,
						 getWidth() * 0.6 + line_width * 2,
						 box_height,
						 radius,
						 line_width,
						 Corner::NONE);
		break;
	case MouseState::INCREASE:
		fill_rounded_box(context,
						 getWidth() * 0.8,
						 box_top,
						 getWidth() * 0.2,
						 box_height,
						 radius,
						 line_width,
						 Corner::RIGHT);
		break;
	default:
		break;
	}
	theme->foreground.setFor(context);
	draw_rounded_box(context,
					 0,
					 box_top,
					 getWidth(),
					 box_height,
					 radius,
					 line_width);
	draw_rounded_box(context,
					 0,
					 box_top,
					 getWidth() * 0.2,
					 box_height,
					 radius,
					 line_width,
					 Corner::LEFT);
	draw_rounded_box(context,
					 getWidth() * 0.8,
					 box_top,
					 getWidth() * 0.2,
					 box_height,
					 radius,
					 line_width,
					 Corner::RIGHT);

	theme->foreground.setFor(context);
	switch (press_state)
	{
	case MouseState::REDUCE:
		fill_rounded_box(context,
						 0,
						 box_top,
						 getWidth() * 0.2,
						 box_height,
						 radius,
						 line_width,
						 Corner::LEFT);
		break;
	case MouseState::CENTER:
		fill_rounded_box(context,
						 getWidth() * 0.2 - line_width,
						 box_top,
						 getWidth() * 0.6 + line_width * 2,
						 box_height,
						 radius,
						 line_width,
						 Corner::NONE);
		break;
	case MouseState::INCREASE:
		fill_rounded_box(context,
						 getWidth() * 0.8,
						 box_top,
						 getWidth() * 0.2,
						 box_height,
						 radius,
						 line_width,
						 Corner::RIGHT);
		break;
	default:
		break;
	}

	float text_size = box_height * 0.5;

	Color(255, 255, 255).setFor(context);
	draw_text(context,
			  std::to_string(value).c_str(),
			  theme->font.c_str(),
			  text_size,
			  Anchor::CENTER,
			  getWidth() / 2,
			  box_center,
			  true);
	draw_text(context,
			  "-",
			  theme->font.c_str(),
			  text_size,
			  Anchor::CENTER,
			  getWidth() * 0.1,
			  box_center,
			  true);
	draw_text(context,
			  "+",
			  theme->font.c_str(),
			  text_size,
			  Anchor::CENTER,
			  getWidth() * 0.9,
			  box_center,
			  true);
	draw_text(context,
			  label.c_str(),
			  theme->font.c_str(),
			  getHeight() * 0.2,
			  Anchor::CENTER,
			  getWidth() * 0.5,
			  getHeight() * 0.9,
			  true);
}

bool IntEditor::onMouse(const MouseEvent& event)
{
	if (event.button == 1 && !event.press)
	{
		press_state = MouseState::NONE;
		repaint();
		if (dragging)
		{
			dragging = false;
			return true;
		}
	}
	if (event.button == 1 && event.press && contains(event.pos))
	{
		if (event.pos.getX() < getWidth() * 0.2)
		{
			set_value(value - 1, true);
			press_state = MouseState::REDUCE;
		}
		else if (event.pos.getX() > getWidth() * 0.8)
		{
			set_value(value + 1, true);
			press_state = MouseState::INCREASE;
		}
		else
		{
			if (DoubleClick::is_double_click(event.button, event.time))
			{
				if (value == default_value)
				{
					set_value(stored_value, true);
				}
				else
				{
					stored_value = value;
					set_value(default_value, true);
				}
			}
			else
			{
				dragging = true;
				press_state = MouseState::CENTER;
				last_mouse_pos = event.pos;
			}
		}
		repaint();
		return true;
	}
	return false;
}

bool IntEditor::onMotion(const MotionEvent& event)
{
	if (contains(event.pos))
	{
		show_tooltip(tooltip,
					 event.absolutePos.getX(),
					 event.absolutePos.getY(),
					 false);
		if (event.pos.getX() < getWidth() * 0.2)
		{
			hover_state = MouseState::REDUCE;
			repaint();
		}
		else if (event.pos.getX() > getWidth() * 0.8)
		{
			hover_state = MouseState::INCREASE;
			repaint();
		}
		else
		{
			hover_state = MouseState::CENTER;
			repaint();
		}
	}
	else
	{
		hover_state = MouseState::NONE;
		repaint();
	}
	if (!dragging)
	{
		return false;
	}
	float diff = last_mouse_pos.getY() - event.pos.getY();

	scroll_value += diff * scroll_speed;
	set_value(scroll_value, true);

	last_mouse_pos = event.pos;
	return true;
}

} // namespace fmpire
