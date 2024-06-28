#include "knob.h"

#include "utils.h"

#include "arc.h"
#include "theme.h"

#include "Color.hpp"
#include "double_click.h"
#include "draw_operations.h"

namespace fmpire
{

Knob::Knob(Widget* parentWidget) :
	FMpireWidget(parentWidget),
	value(0.5),
	default_value(0),
	stored_value(0.5),
	label_scale(0.2),
	drag_speed(0.005),
	dragging(false),
	callback(nullptr)
{
}

Knob::~Knob() noexcept
{
}

void Knob::set_value(float new_value, bool emit_callback)
{
	value = new_value;

	if (emit_callback && callback)
	{
		callback->value_changed(this, value);
	}

	repaint();
}

void Knob::set_default_value(const float val)
{
	default_value = val;
}

void Knob::set_label(const std::string& text)
{
	label = text;
	repaint();
}

void Knob::set_label_scale(const float scale)
{
	label_scale = scale;
	repaint();
}

void Knob::set_tooltip(const std::string& txt,
					   const float offset,
					   const float mult,
					   const std::string unit)
{
	tooltip = txt;
	tooltip_value_offset = offset;
	tooltip_value_mult = mult;
	tooltip_unit = unit;
}

void Knob::set_callback(Callback* cb)
{
	callback = cb;
}

bool Knob::onMouse(const MouseEvent& event)
{
	if (event.button == 1 && event.press && contains(event.pos))
	{
		if (DoubleClick::is_double_click(event.button, event.time))
		{
			if (d_isEqual(default_value, value))
			{
				set_value(stored_value, true);
			}
			else
			{
				stored_value = value;
				set_value(default_value, true);
			}

			dragging = false;

			if (callback)
			{
				callback->drag_ended(this);
			}
			return true;
		}

		dragging = true;
		last_mouse_pos = event.pos;

		show_tooltip(create_tooltip_string(),
					 event.absolutePos.getX(),
					 event.absolutePos.getY(),
					 true);
		if (callback)
		{
			callback->drag_started(this);
		}
	}
	else if (event.button == 1 && !event.press && dragging)
	{
		dragging = false;

		unpin_tooltip();
		if (callback)
		{
			callback->drag_ended(this);
		}

		return true;
	}

	return false;
}

bool Knob::onMotion(const MotionEvent& event)
{
	if (contains(event.pos))
	{
		show_tooltip(create_tooltip_string(),
					 event.absolutePos.getX(),
					 event.absolutePos.getY(),
					 dragging);
	}
	if (!dragging)
	{
		return false;
	}
	update_tooltip(create_tooltip_string());

	float diff = last_mouse_pos.getY() - event.pos.getY();

	value += diff * drag_speed;
	value = std::clamp(value, 0.f, 1.f);
	set_value(value, true);

	last_mouse_pos = event.pos;
	return true;
}

void Knob::onDisplay()
{
	const GraphicsContext& context(getGraphicsContext());

	float radius = std::min(getWidth(), getHeight()) * 0.75 * 0.5;

	Arc<float> base(getWidth() / 2, getHeight() / 2, radius, 45, 315);
	theme->background.setFor(context);
	base.draw(context, radius * 0.07);

	Arc<float> slider(getWidth() / 2,
					  getHeight() / 2,
					  radius,
					  45,
					  lerp(45, 315, value));
	theme->highlight.setFor(context);
	slider.draw(context, radius * 0.4);

	Color(255, 255, 255).setFor(context);
	draw_text(context,
			  label.c_str(),
			  theme->font.c_str(),
			  getHeight() * label_scale,
			  Anchor::CENTER,
			  getWidth() / 2,
			  getHeight() * (1.0 - label_scale * 0.5));
}

std::string Knob::create_tooltip_string()
{
	return tooltip + ": "
		 + std::to_string(value * tooltip_value_mult + tooltip_value_offset)
		 + " " + tooltip_unit;
}

} // namespace fmpire
