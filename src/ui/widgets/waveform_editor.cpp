#include "waveform_editor.h"

#include "button.h"
#include "defines.h"
#include "draw_operations.h"
#include "Geometry.hpp"
#include "harmonic_editor.h"
#include "ref_counted.h"
#include "state_manager.h"
#include "utils.h"
#include "waveform.h"
#include "waveform_part.h"
#include "waveform_tools.h"
#include <string>

namespace fmpire
{
WaveformEditor::WaveformEditor(Widget* parent,
							   SpectrumView* spec_view,
							   StateManager& state_mgr) :
	FMpireWidget(parent),
	waveform(nullptr),
	spectrum_view(spec_view),
	selection(nullptr),
	selected_tool(WaveformToolType::FREE),
	active_tool(nullptr),
	osc_index(0),
	grid_x(0),
	grid_y(0),
	state_manager(state_mgr)
{
}

WaveformEditor::~WaveformEditor() noexcept
{
}

void WaveformEditor::on_press(Button* const button)
{
	if (button == delete_button)
	{
		waveform->remove_part(selection);
		select(nullptr, true);
		on_waveform_updated(true);
		repaint();
	}
}

void WaveformEditor::select(WaveformPart* part, bool trigger_callback)
{
	selection = part;

	if (trigger_callback && callback)
	{
		callback->on_waveform_part_selected(this, part);
	}
	update_delete_button();
	repaint();
}

void WaveformEditor::onDisplay()
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
	Line<float> line(0, getHeight() / 2, getWidth(), getHeight() / 2);
	line.draw(context, theme->line_very_thin);

	for (uint32_t x = 0; x <= grid_x; x++)
	{
		float line_pos = (float) getWidth() / grid_x * x;
		line = Line<float>(line_pos, 0, line_pos, getHeight());
		line.draw(context, theme->line_very_thin);
	}

	for (uint32_t y = 0; y <= grid_y; y++)
	{
		float line_pos = (float) getHeight() / grid_y * y;
		line = Line<float>(0, line_pos, getWidth(), line_pos);
		line.draw(context, theme->line_very_thin);
	}

	std::vector<float> samples;

	if (waveform)
	{
		samples = waveform->sample_all();
	}
	if (samples.size() < 2)
	{
		samples = {0, 0};
	}

	if (selection)
	{
		Color fg_alpha = theme->foreground;
		fg_alpha.alpha = 0.5;
		fg_alpha.setFor(context, true);

		float start =
			(float) selection->get_start() / waveform->get_width() * getWidth();
		float end =
			(float) selection->get_end() / waveform->get_width() * getWidth();

		std::vector<std::pair<float, float>> highlights = {
			{start, end}
        };
		bool has_found_part = false;
		for (size_t i = 0; true; i++)
		{
			WaveformPart* part = waveform->get_part_by_idx(i);
			if (part == nullptr)
			{
				break;
			}
			else if (!has_found_part)
			{
				if (part == selection)
				{
					has_found_part = true;
				}
				continue;
			}

			float part_start =
				(float) part->get_start() / waveform->get_width() * getWidth();
			float part_end =
				(float) part->get_end() / waveform->get_width() * getWidth();

			for (size_t j = 0; j < highlights.size(); j++)
			{
				std::pair<float, float>& highlight = highlights[j];
				if (highlight.first < part_end && highlight.second > part_end)
				{
					float first = highlight.first;
					float second = highlight.second;
					highlight.first = part_end;
					if (first < part_start)
					{
						highlights.push_back({first, part_start});
					}
				}
				else if (highlight.first > part_start
						 && highlight.second < part_end)
				{
					highlights.erase(highlights.begin() + j);
					j--;
				}
				else if (highlight.first < part_start
						 && highlight.second > part_start)
				{
					highlight.second = part_start;
				}
			}
		}

		for (size_t i = 0; i < highlights.size(); i++)
		{
			Rectangle<float> selection_box(highlights[i].first,
										   0,
										   highlights[i].second
											   - highlights[i].first,
										   getHeight());
			selection_box.draw(context);
		}

		theme->highlight.setFor(context);
		Line<float> line(start, 0, start, getHeight());
		line.draw(context, theme->line_very_thin);

		line = Line<float>(end, 0, end, getHeight());
		line.draw(context, theme->line_very_thin);
	}

	theme->highlight.setFor(context);
	for (size_t sample = 1; sample < samples.size(); sample++)
	{
		float x0 = (float) (sample - 1) / (samples.size() - 1);
		float x1 = (float) sample / (samples.size() - 1);
		x0 *= getWidth();
		x1 *= getWidth();
		float y0 = getHeight() / 2 - samples[sample - 1] * getHeight() / 2;
		float y1 = getHeight() / 2 - samples[sample] * getHeight() / 2;
		line = Line<float>(x0, y0, x1, y1);
		line.draw(context, theme->line_thin);
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

bool WaveformEditor::onMouse(const MouseEvent& event)
{
	if (FMpireWidget::onMouse(event))
	{
		return true;
	}

	if (waveform == nullptr)
	{
		return false;
	}

	if (event.button == 1)
	{
		bool had_tool = active_tool != nullptr;
		if (active_tool)
		{
			active_tool->end();
			active_tool = nullptr;
		}

		if (event.press && contains_clipped(event.pos))
		{
			switch (selected_tool)
			{
			case WaveformToolType::FREE:
			{
				Ref<WaveformPart> existing_part = waveform->get_part(
					event.pos.getX() / getWidth() * waveform->get_width());
				active_tool = new FreeDrawTool(
					waveform,
					existing_part != nullptr
							&& existing_part->get_type()
								   == WaveformPart::Type::SAMPLES
						? static_ref_cast<SamplesWaveformPart>(existing_part)
						: nullptr);
				break;
			}
			case WaveformToolType::LINE:
				active_tool = new LineDrawTool(waveform);
				break;
			case WaveformToolType::HALF_SINE:
				active_tool = new HalfSineTool(waveform);
				break;
			case WaveformToolType::SINE_SLOPE:
				active_tool = new SineSlopeTool(waveform);
				break;
			default:
				break;
			}

			if (active_tool)
			{
				float x = event.pos.getX() / getWidth();
				float y = 1.0f - event.pos.getY() / getHeight();
				float snapped_x =
					grid_x > 0 ? std::round(x * grid_x) / grid_x : x;
				float snapped_y =
					grid_y > 0 ? std::round(y * grid_y) / grid_y : y;

				active_tool->start({x, y, snapped_x, snapped_y});

				on_waveform_updated(true, false);
			}
			return true;
		}
		else if (had_tool && !event.press)
		{
			on_waveform_updated(true);
			return true;
		}
	}
	else if (event.button == 2 && event.press && contains_clipped(event.pos))
	{
		size_t sample = event.pos.getX() / getWidth() * waveform->get_width();

		select(waveform->get_part(sample), true);
		update_delete_button();
		repaint();
	}

	return false;
}

bool WaveformEditor::onMotion(const MotionEvent& event)
{
	if (FMpireWidget::onMotion(event))
	{
		return true;
	}

	if (!contains_clipped(event.pos))
	{
		return false;
	}

	if (active_tool == nullptr)
	{
		return false;
	}

	float x = event.pos.getX() / getWidth();
	float y = 1.0f - event.pos.getY() / getHeight();
	float snapped_x = grid_x > 0 ? std::round(x * grid_x) / grid_x : x;
	float snapped_y = grid_y > 0 ? std::round(y * grid_y) / grid_y : y;

	active_tool->step({x, y, snapped_x, snapped_y});


	on_waveform_updated(false);
	return true;
}

void WaveformEditor::onPositionChanged(const PositionChangedEvent& event)
{
	update_delete_button();
}

void WaveformEditor::onResize(const ResizeEvent& event)
{
	update_delete_button();
}

void WaveformEditor::update_delete_button()
{
	if (selection == nullptr)
	{
		delete_button = nullptr;
		return;
	}
	else if (delete_button == nullptr)
	{
		delete_button = new Button(this);
		delete_button->set_text("—");
		delete_button->set_drawing_normal_bg(true);
		delete_button->set_callback(this);
	}

	float button_size = std::max(getHeight() * 0.05f, 10.0f);

	float selection_end =
		(float) selection->get_end() / waveform->get_width() * getWidth();

	float button_pos = std::min(std::max(5.0f, selection_end - 5 - button_size),
								getWidth() - 5 - button_size);

	delete_button->setAbsolutePos(getAbsoluteX() + button_pos,
								  getAbsoluteY() + 5);
	delete_button->setSize(button_size, button_size);
}

void WaveformEditor::on_waveform_updated(bool update_dsp, bool notify_callback)
{
	if (update_dsp)
	{
		uint32_t wf_index = waveform->get_index();

		std::string state;
		state += encode_base64(reinterpret_cast<const uint8_t*>(&wf_index),
							   sizeof(wf_index));
		state += waveform->get_state();
		state_manager.set_state(KEY_OSC_PREFIX + std::to_string(osc_index)
									+ "/" KEY_OSC_WAVETABLE KEY_WT_UPDATE,
								state);
	}
	state_manager.on_wavetable_edited(osc_index);

	update_delete_button();
	spectrum_view->set_waveform(waveform);

	if (callback && notify_callback)
	{
		callback->on_waveform_edited(this, waveform, update_dsp);
	}
}

void WaveformEditor::on_part_edited(WaveformPartEditor* const editor,
								   WaveformPart* const new_part)
{
	if (!waveform)
	{
		return;
	}

	WaveformPart* old_part = selection;
	if (old_part != new_part)
	{
		waveform->replace_part(old_part, new_part);
		select(new_part, true);
	}

	on_waveform_updated(true);
	repaint();
}

void WaveformEditor::on_harmonics_edited(HarmonicEditor* const editor,
										 HarmonicsWaveformPart* const part,
										 const bool is_done)
{
	part->update();
	on_waveform_updated(true, is_done);
	repaint();
}

} // namespace fmpire
