#include "fmpire_knob.h"

#include "utils.h"

#include "arc.h"
#include "fmpire_theme.h"

#include "Color.hpp"
#include "double_click.h"

namespace fmpire
{

FMpireKnob::FMpireKnob(Widget* parentWidget) :
	SubWidget(parentWidget),
	value(0.5),
	default_value(0),
	stored_value(0),
	drag_speed(0.005),
	dragging(false),
	callback(nullptr)
{
}

FMpireKnob::~FMpireKnob() noexcept
{
}

void FMpireKnob::set_value(float new_value, bool emit_callback)
{
	value = new_value;
	if (emit_callback && callback)
	{
		callback->value_changed(this, value);
	}
	repaint();
}

void FMpireKnob::set_callback(Callback* cb)
{
	callback = cb;
}

bool FMpireKnob::onMouse(const MouseEvent & event)
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
		if (callback)
		{
			callback->drag_started(this);
		}
	}
	else if (event.button == 1 && !event.press && dragging)
	{
		dragging = false;
		if (callback)
		{
			callback->drag_ended(this);
		}
		return true;
	}
	return false;
}

bool FMpireKnob::onMotion(const MotionEvent& event)
{
	if (!dragging)
	{
		return false;
	}
	
	float diff = last_mouse_pos.getY() - event.pos.getY();
	
	value += diff * drag_speed;
	value = std::clamp(value, 0.f, 1.f);
	set_value(value, true);
	
	last_mouse_pos = event.pos;
	return true;
}

void FMpireKnob::onDisplay()
{
	const GraphicsContext& context(getGraphicsContext());
	
	float radius = std::min(getWidth(), getHeight()) * 0.75 * 0.5;
	
	Arc<float> base(getWidth() / 2, getHeight() / 2, radius, 45, 315);
	theme->background.setFor(context);
	base.draw(context, radius * 0.07);
	
	Arc<float> slider(getWidth() / 2, getHeight() / 2, radius, 45, lerp(45, 315, value));
	theme->highlight.setFor(context);
	slider.draw(context, radius * 0.4);
}






}
