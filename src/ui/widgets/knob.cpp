#include "knob.h"

#include "utils.h"

#include "arc.h"
#include "theme.h"

#include "Color.hpp"
#include "double_click.h"
#include "draw_operations.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace fmpire
{

namespace
{

// Widens [down, up] (how far a route can move its target below/above the knob
// value) by what this route contributes.
void add_route_extent(const RouteSettings& route, float& down, float& up)
{
	if (route.bipolar)
	{
		down += std::fabs(route.amount);
		up += std::fabs(route.amount);
	}
	else if (route.amount >= 0.0f)
	{
		up += route.amount;
	}
	else
	{
		down -= route.amount;
	}
}

} // namespace

Knob::Knob(Widget* parentWidget) :
	FMpireWidget(parentWidget),
	value(0.5),
	default_value(0),
	stored_value(0.5),
	label_scale(0.2),
	drag_speed(0.005),
	dragging(false),
	callback(nullptr),
	mod_model(nullptr),
	mod_target(TargetType::OSC_VOLUME),
	mod_object(0),
	mod_dragging(false),
	mod_amount(0.0f)
{
}

Knob::~Knob() noexcept
{
	if (mod_model)
	{
		mod_model->remove_listener(this);
	}
}

void Knob::set_mod_target(ModulationModel& modulation_model,
						  const TargetType target,
						  const size_t target_object)
{
	if (mod_model != &modulation_model)
	{
		if (mod_model)
		{
			mod_model->remove_listener(this);
		}
		mod_model = &modulation_model;
		mod_model->add_listener(this);
	}
	mod_target = target;
	mod_object = target_object;
	repaint();
}

void Knob::on_modulation_changed()
{
	repaint();
}

void Knob::on_route_meter_changed(const size_t slot)
{
	const RouteSettings* route = mod_model->get_route(slot);
	if (route && route->target == mod_target && route->target_object == mod_object)
	{
		repaint();
	}
}

bool Knob::is_mod_mode() const
{
	return mod_model && mod_model->has_armed_source();
}

std::string Knob::create_mod_tooltip_string() const
{
	char text[48];
	std::snprintf(text, sizeof(text), "Mod amount: %.2f", mod_amount);
	return text;
}

bool Knob::on_mod_mouse(const MouseEvent& event)
{
	const SourceId armed = mod_model->get_armed_source();
	const size_t slot = mod_model->find_route(armed, mod_target, mod_object);

	if (event.button == 1 && event.press && contains_clipped(event.pos))
	{
		if (DoubleClick::is_double_click(event.button, event.time))
		{
			if (slot != ModulationModel::npos)
			{
				mod_model->remove_route(slot);
			}
			mod_dragging = false;
			unpin_tooltip();
			return true;
		}

		const RouteSettings* route = mod_model->get_route(slot);
		mod_amount = route ? route->amount : 0.0f;
		mod_dragging = true;
		last_mouse_pos = event.pos;
		show_tooltip(create_mod_tooltip_string(),
					 event.absolutePos.getX(),
					 event.absolutePos.getY(),
					 true);
		return true;
	}
	else if (event.button == 1 && !event.press && mod_dragging)
	{
		mod_dragging = false;
		unpin_tooltip();
		if (std::fabs(mod_amount) < 0.01f && slot != ModulationModel::npos)
		{
			mod_model->remove_route(slot);
		}
		return true;
	}
	else if (event.button == 2 && event.press && contains_clipped(event.pos))
	{
		const RouteSettings* route = mod_model->get_route(slot);
		if (route)
		{
			mod_model->set_route_bipolar(slot, !route->bipolar);
		}
		return true;
	}
	return false;
}

bool Knob::on_mod_motion(const MotionEvent& event)
{
	if (!mod_dragging)
	{
		return false;
	}
	if (!is_mod_mode())
	{
		// disarmed in the middle of a drag
		mod_dragging = false;
		unpin_tooltip();
		return false;
	}

	const float diff = last_mouse_pos.getY() - event.pos.getY();
	mod_amount = std::clamp(mod_amount + diff * drag_speed * 2.0f, -1.0f, 1.0f);

	// keep the route alive while dragging through zero; it is removed on
	// release if it ended up (almost) at zero
	float applied = mod_amount;
	if (std::fabs(applied) < 0.002f)
	{
		applied = mod_amount >= 0.0f ? 0.002f : -0.002f;
	}
	mod_model->set_route_amount(mod_model->get_armed_source(),
								mod_target,
								mod_object,
								applied);

	update_tooltip(create_mod_tooltip_string());
	last_mouse_pos = event.pos;
	return true;
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
	if (is_mod_mode() || mod_dragging)
	{
		return on_mod_mouse(event);
	}

	if (event.button == 1 && event.press && contains_clipped(event.pos))
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
	if (is_mod_mode() || mod_dragging)
	{
		return on_mod_motion(event);
	}

	if (contains_clipped(event.pos))
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

void Knob::draw_mod_range(const GraphicsContext& context,
						  const float radius,
						  const float down,
						  const float up,
						  const float width) const
{
	const float low = std::clamp(value - down, 0.0f, 1.0f);
	const float high = std::clamp(value + up, 0.0f, 1.0f);

	Color(255, 150, 40).setFor(context);
	Arc<float> range(getWidth() / 2,
					 getHeight() / 2,
					 radius * 1.25f,
					 lerp(45, 315, low),
					 lerp(45, 315, high));
	range.draw(context, width);
}

void Knob::onDisplay()
{
	clip();

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

	if (is_mod_mode())
	{
		const size_t slot = mod_model->find_route(mod_model->get_armed_source(),
												  mod_target,
												  mod_object);
		const RouteSettings* route = mod_model->get_route(slot);

		Color(255, 150, 40).setFor(context);
		Arc<float> ring(getWidth() / 2, getHeight() / 2, radius * 1.25f, 45, 315);
		ring.draw(context, radius * 0.05f);

		if (route)
		{
			float down = 0.0f;
			float up = 0.0f;
			add_route_extent(*route, down, up);
			draw_mod_range(context, radius, down, up, radius * 0.18f);
		}
	}
	else if (mod_model)
	{
		// outside of programming mode: the combined range of all routes, and
		// where the modulation currently is
		float down = 0.0f;
		float up = 0.0f;
		bool modulated = false;
		for (size_t slot = 0; slot < mod_model->route_slot_count(); slot++)
		{
			const RouteSettings* route = mod_model->get_route(slot);
			if (route && route->target == mod_target
				&& route->target_object == mod_object)
			{
				add_route_extent(*route, down, up);
				modulated = true;
			}
		}
		if (modulated)
		{
			draw_mod_range(context, radius, down, up, radius * 0.05f);
			const float live = mod_model->get_target_meter(mod_target, mod_object);
			if (std::fabs(live) > 0.001f)
			{
				draw_mod_range(context,
							   radius,
							   live < 0.0f ? -live : 0.0f,
							   live > 0.0f ? live : 0.0f,
							   radius * 0.18f);
			}
		}
	}

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
