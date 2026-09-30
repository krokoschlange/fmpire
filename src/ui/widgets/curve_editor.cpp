#include "curve_editor.h"

#include "arc.h"
#include "double_click.h"
#include "draw_operations.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace fmpire
{

namespace
{
constexpr float reference_size = 280.0f;
constexpr float base_padding = 14.0f;
constexpr float base_point_radius = 9.0f;
constexpr float base_handle_radius = 9.0f;
} // namespace

CurveEditor::CurveEditor(Widget* parent, ModulationModel& modulation_model) :
	FMpireWidget(parent),
	model(modulation_model),
	modulator_id(ModulationModel::npos),
	is_envelope(true),
	grid_x(0),
	grid_y(0),
	drag_point(Curve::npos),
	drag_handle(Curve::npos),
	hover_point(Curve::npos),
	hover_handle(Curve::npos)
{
	model.add_listener(this);
	on_modulation_changed();
}

CurveEditor::~CurveEditor() noexcept
{
	model.remove_listener(this);
}

void CurveEditor::set_grid(const uint32_t x, const uint32_t y)
{
	grid_x = x;
	grid_y = y;
	repaint();
}

void CurveEditor::on_modulation_changed()
{
	const size_t selected = model.get_selected_modulator();
	if (selected != modulator_id)
	{
		drag_point = Curve::npos;
		drag_handle = Curve::npos;
		hover_point = Curve::npos;
		hover_handle = Curve::npos;
	}

	modulator_id = selected;
	if (model.has_modulator(modulator_id))
	{
		const ModulationModel::ModulatorEntry& entry =
			model.get_modulator(modulator_id);
		curve = entry.curve;
		is_envelope = entry.settings.type == ModulatorType::ENVELOPE;
	}
	else
	{
		curve = Curve();
	}
	repaint();
}

void CurveEditor::on_playhead_changed(const size_t id)
{
	if (id == modulator_id)
	{
		repaint();
	}
}

float CurveEditor::ui_scale() const
{
	return std::clamp(std::min(getWidth(), getHeight()) / reference_size,
					  0.5f,
					  4.0f);
}

float CurveEditor::padding() const
{
	return base_padding * ui_scale();
}

float CurveEditor::point_radius() const
{
	return base_point_radius * ui_scale();
}

float CurveEditor::handle_radius() const
{
	return base_handle_radius * ui_scale();
}

Point<float> CurveEditor::to_pixels(const float x, const float y) const
{
	const float width = std::max(getWidth() - 2.0f * padding(), 1.0f);
	const float height = std::max(getHeight() - 2.0f * padding(), 1.0f);
	return Point<float>(padding() + x * width, padding() + (1.0f - y) * height);
}

Point<float> CurveEditor::from_pixels(const float px, const float py) const
{
	const float width = std::max(getWidth() - 2.0f * padding(), 1.0f);
	const float height = std::max(getHeight() - 2.0f * padding(), 1.0f);
	return Point<float>(std::clamp((px - padding()) / width, 0.0f, 1.0f),
						std::clamp(1.0f - (py - padding()) / height, 0.0f, 1.0f));
}

float CurveEditor::snap(const float value, const uint32_t divisions) const
{
	if (divisions == 0)
	{
		return value;
	}
	return std::round(value * divisions) / divisions;
}

size_t CurveEditor::find_point(const float px, const float py) const
{
	size_t best = Curve::npos;
	float best_distance = point_radius() * point_radius();
	for (size_t index = 0; index < curve.size(); index++)
	{
		const Point<float> pos =
			to_pixels(curve.point(index).x, curve.point(index).y);
		const float dx = pos.getX() - px;
		const float dy = pos.getY() - py;
		const float distance = dx * dx + dy * dy;
		if (distance <= best_distance)
		{
			best = index;
			best_distance = distance;
		}
	}
	return best;
}

size_t CurveEditor::find_handle(const float px, const float py) const
{
	for (size_t index = 0; index + 1 < curve.size(); index++)
	{
		float x, y;
		curve.segment_handle(index, x, y);
		const Point<float> pos = to_pixels(x, y);
		const float dx = pos.getX() - px;
		const float dy = pos.getY() - py;
		if (dx * dx + dy * dy <= handle_radius() * handle_radius())
		{
			return index;
		}
	}
	return Curve::npos;
}

void CurveEditor::send()
{
	if (model.has_modulator(modulator_id))
	{
		model.set_modulator_curve(modulator_id, curve);
	}
}

void CurveEditor::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	clip_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 0);

	theme->background.setFor(context);
	for (uint32_t x = 0; x <= grid_x; x++)
	{
		if (grid_x == 0)
		{
			break;
		}
		const float px = to_pixels((float) x / grid_x, 0).getX();
		Line<float> line(px, padding(), px, getHeight() - padding());
		line.draw(context, theme->line_very_thin);
	}
	for (uint32_t y = 0; y <= grid_y; y++)
	{
		if (grid_y == 0)
		{
			break;
		}
		const float py = to_pixels(0, (float) y / grid_y).getY();
		Line<float> line(padding(), py, getWidth() - padding(), py);
		line.draw(context, theme->line_very_thin);
	}

	if (!model.has_modulator(modulator_id))
	{
		Color(255, 255, 255).setFor(context);
		draw_text(context,
				  "Add an envelope or LFO",
				  theme->font.c_str(),
				  getHeight() * 0.06f,
				  Anchor::CENTER,
				  getWidth() * 0.5f,
				  getHeight() * 0.5f);
	}
	else
	{
		if (is_envelope)
		{
			const float sustain_x = curve.point(curve.get_sustain_index()).x;
			const float px = to_pixels(sustain_x, 0).getX();
			theme->foreground.setFor(context);
			Line<float> line(px, padding(), px, getHeight() - padding());
			line.draw(context, theme->line_very_thin);
		}

		const size_t sample_count =
			std::max<size_t>((size_t) std::max(getWidth() - 2.0f * padding(), 2.0f),
							 2);
		std::vector<Point<float>> line(sample_count);
		for (size_t i = 0; i < sample_count; i++)
		{
			const float pos = (float) i / (sample_count - 1);
			line[i] = to_pixels(pos, curve.sample(pos));
		}
		theme->highlight.setFor(context);
		draw_line_string(context, line, theme->line_thin);

		// where the newest voice currently is
		const float playhead = model.get_playhead(modulator_id);
		if (playhead >= 0.0f)
		{
			const float px = to_pixels(std::min(playhead, 1.0f), 0).getX();
			Color(255, 150, 40).setFor(context);
			Line<float> playhead_line(px, padding(), px, getHeight() - padding());
			playhead_line.draw(context, theme->line_thin);
		}

		for (size_t index = 0; index + 1 < curve.size(); index++)
		{
			float x, y;
			curve.segment_handle(index, x, y);
			const Point<float> pos = to_pixels(x, y);
			const bool is_active = index == drag_handle || index == hover_handle;
			const float radius = handle_radius() * (is_active ? 0.7f : 0.55f);

			// (Circle::drawOutline leaves the last segment open)
			Circle<float> disc(pos, radius, 32);
			theme->foreground.setFor(context);
			disc.draw(context);
			theme->highlight.setFor(context);
			Arc<float> ring(pos, radius, 0, 360, 64);
			ring.draw(context, theme->line_thin);
		}

		theme->highlight.setFor(context);
		for (size_t index = 0; index < curve.size(); index++)
		{
			const Point<float> pos =
				to_pixels(curve.point(index).x, curve.point(index).y);
			const bool is_active = index == drag_point || index == hover_point;
			const float half = point_radius() * (is_active ? 0.8f : 0.6f);
			Rectangle<float> box(pos.getX() - half,
								 pos.getY() - half,
								 half * 2.0f,
								 half * 2.0f);
			box.draw(context);
		}
	}

	theme->foreground.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 theme->line_very_thin);
}

bool CurveEditor::onMouse(const MouseEvent& event)
{
	if (FMpireWidget::onMouse(event))
	{
		return true;
	}

	if (!event.press)
	{
		if (event.button == 1
			&& (drag_point != Curve::npos || drag_handle != Curve::npos))
		{
			drag_point = Curve::npos;
			drag_handle = Curve::npos;
			repaint();
			return true;
		}
		return false;
	}

	if (!model.has_modulator(modulator_id) || !contains_clipped(event.pos))
	{
		return false;
	}

	const float px = event.pos.getX();
	const float py = event.pos.getY();
	const size_t hit_point = find_point(px, py);

	if (event.button == 2)
	{
		if (hit_point != Curve::npos)
		{
			curve.remove_point(hit_point);
			send();
		}
		return true;
	}
	if (event.button != 1)
	{
		return false;
	}

	if (hit_point != Curve::npos)
	{
		if (DoubleClick::is_double_click(event.button, event.time))
		{
			curve.remove_point(hit_point);
			send();
		}
		else
		{
			drag_point = hit_point;
			repaint();
		}
		return true;
	}

	const size_t hit_handle = find_handle(px, py);
	if (hit_handle != Curve::npos)
	{
		drag_handle = hit_handle;
		return true;
	}

	const Point<float> pos = from_pixels(px, py);
	const size_t index =
		curve.add_point(snap(pos.getX(), grid_x), snap(pos.getY(), grid_y));
	if (index != Curve::npos)
	{
		drag_point = index;
		send();
	}
	return true;
}

bool CurveEditor::onMotion(const MotionEvent& event)
{
	if (FMpireWidget::onMotion(event))
	{
		return true;
	}

	size_t hovered_point = Curve::npos;
	size_t hovered_handle = Curve::npos;
	if (drag_point == Curve::npos && drag_handle == Curve::npos
		&& contains_clipped(event.pos))
	{
		hovered_point = find_point(event.pos.getX(), event.pos.getY());
		if (hovered_point == Curve::npos)
		{
			hovered_handle = find_handle(event.pos.getX(), event.pos.getY());
		}
	}
	if (hovered_point != hover_point || hovered_handle != hover_handle)
	{
		hover_point = hovered_point;
		hover_handle = hovered_handle;
		repaint();
	}

	if (drag_point != Curve::npos)
	{
		const Point<float> pos = from_pixels(event.pos.getX(), event.pos.getY());
		curve.move_point(drag_point,
						 snap(pos.getX(), grid_x),
						 snap(pos.getY(), grid_y));
		send();
		return true;
	}
	if (drag_handle != Curve::npos)
	{
		const Point<float> pos = from_pixels(event.pos.getX(), event.pos.getY());
		curve.set_segment_handle(drag_handle, pos.getX(), pos.getY());
		send();
		return true;
	}
	return false;
}

} // namespace fmpire
