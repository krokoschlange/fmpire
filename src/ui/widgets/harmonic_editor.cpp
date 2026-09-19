#include "harmonic_editor.h"
#include "Base.hpp"
#include "double_click.h"
#include "draw_operations.h"
#include "waveform_part.h"

namespace fmpire
{

HarmonicEditor::HarmonicEditor(Widget* parent) :
	FMpireWidget(parent),
	callback(nullptr),
	is_pressed(false),
	dragging_phase(false)
{
}

HarmonicEditor::~HarmonicEditor() noexcept
{
}

void HarmonicEditor::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	if (part != nullptr && part->get_harmonics().empty())
	{
		part->get_harmonics().resize(128, {0.0f, 0.0f});
	}

	uint32_t box_count = part != nullptr ? part->get_harmonics().size() : 128;

	float h = getHeight();
	float amp_h = h * 0.8f;
	float phase_h = h * 0.2f;

	for (uint32_t i = 0; i < box_count; i++)
	{
		float x = i * box_width;
		float amp = 0.0f;
		float phase = 0.0f;
		if (part != nullptr && i < part->get_harmonics().size())
		{
			amp = part->get_harmonics()[i].amplitude;
			phase = part->get_harmonics()[i].phase;
		}

		// Amplitude bar
		float bar_h = amp_h * amp;
		if (bar_h > 0.0f)
		{
			theme->foreground.setFor(context);
			Rectangle<float> amp_rect(x + 2,
									  amp_h - bar_h,
									  box_width - 4,
									  bar_h);
			amp_rect.draw(context);
		}

		// Divider
		Line<float> div(x, amp_h, x + box_width, amp_h);
		theme->background.setFor(context);
		div.draw(context, theme->line_thin);

		// Phase dot
		theme->foreground.setFor(context);
		float phase_y = amp_h + phase_h * (1.0f - phase);
		Rectangle<float> phase_dot(x + box_width * 0.5f - 2, phase_y - 2, 4, 4);
		phase_dot.draw(context);

		// Divider between harmonics
		Line<float> v_div(x, 0, x, h);
		theme->background.setFor(context);
		v_div.draw(context);

		if (i % 8 == 0)
		{
			draw_text(context,
					  std::to_string(i).c_str(),
					  theme->font.c_str(),
					  box_width * 0.5,
					  Anchor::CENTER,
					  x + box_width * 0.5,
					  amp_h * 0.1);
		}
	}
}

void HarmonicEditor::apply_drag_position(const Point<double>& pos)
{
	int harmonic_idx = (int) (pos.getX() / box_width);
	if (harmonic_idx < 0 || harmonic_idx >= (int) part->get_harmonics().size())
	{
		return;
	}

	float h = getHeight();
	float amp_h = h * 0.8f;
	float y = pos.getY();

	if (!dragging_phase)
	{
		y = std::clamp(y, 0.0f, amp_h);
		float amp = 1.0f - (y / amp_h);
		part->get_harmonics()[harmonic_idx].amplitude =
			std::clamp(amp, 0.0f, 1.0f);
	}
	else
	{
		y = std::clamp(y, amp_h, h);
		float phase = 1.0f - ((y - amp_h) / (h - amp_h));
		part->get_harmonics()[harmonic_idx].phase = std::clamp(phase, 0.0f, 1.0f);
	}

	if (callback)
	{
		callback->on_harmonics_edited(this, part, false);
	}
	repaint();
}

bool HarmonicEditor::onMouse(const MouseEvent& event)
{
	if (part == nullptr)
	{
		return false;
	}
	if (event.button == 1 && event.press && contains_clipped(event.pos))
	{
		int harmonic_idx = (int) (event.pos.getX() / box_width);
		if (harmonic_idx < 0
			|| harmonic_idx >= (int) part->get_harmonics().size())
		{
			return true;
		}

		float h = getHeight();
		float amp_h = h * 0.8f;
		dragging_phase = event.pos.getY() >= amp_h;

		if (DoubleClick::is_double_click(event.button, event.time))
		{
			is_pressed = false;

			HarmonicsWaveformPart::Harmonic& harmonic =
				part->get_harmonics()[harmonic_idx];
			if (dragging_phase)
			{
				if (stored_phase.size() != part->get_harmonics().size())
				{
					stored_phase.resize(part->get_harmonics().size(), 0.0f);
				}
				if (harmonic.phase != 0.0f)
				{
					stored_phase[harmonic_idx] = harmonic.phase;
					harmonic.phase = 0.0f;
				}
				else
				{
					harmonic.phase = stored_phase[harmonic_idx];
				}
			}
			else
			{
				if (stored_amplitude.size() != part->get_harmonics().size())
				{
					stored_amplitude.resize(part->get_harmonics().size(), 0.0f);
				}
				if (harmonic.amplitude != 0.0f)
				{
					stored_amplitude[harmonic_idx] = harmonic.amplitude;
					harmonic.amplitude = 0.0f;
				}
				else
				{
					harmonic.amplitude = stored_amplitude[harmonic_idx];
				}
			}

			if (callback)
			{
				callback->on_harmonics_edited(this, part, true);
			}
			repaint();
			return true;
		}

		is_pressed = true;
		apply_drag_position(event.pos);
		return true;
	}
	else if (event.button == 1 && !event.press && is_pressed)
	{
		is_pressed = false;
		if (callback && part)
		{
			callback->on_harmonics_edited(this, part, true);
		}
		return true;
	}
	return false;
}

bool HarmonicEditor::onMotion(const MotionEvent& event)
{
	if (part == nullptr)
	{
		return false;
	}
	if (!is_pressed)
	{
		return false;
	}

	apply_drag_position(event.pos);
	return true;
}

} // namespace fmpire
