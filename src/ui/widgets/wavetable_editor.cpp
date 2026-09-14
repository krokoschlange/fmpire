#include "wavetable_editor.h"
#include "scroll_container.h"

#include "button.h"
#include "defines.h"
#include "harmonic_editor.h"
#include "int_editor.h"
#include "selector.h"
#include "spectrum_view.h"
#include "state_manager.h"
#include "utils.h"
#include "waveform_editor.h"
#include "waveform_selector.h"
#include "waveform_tools.h"
#include "wavetable_creator.h"

namespace fmpire
{

WavetableEditor::WavetableEditor(Widget* parent, StateManager& state_mgr) :
	GridContainer(parent),
	Selector::Callback(),
	wavetable(nullptr),
	selected_oscillator(0),
	state_manager(state_mgr)
{
	state_manager.set_wavetable_editor(this);

	add_row(2, 0, 0, 50, 0);
	add_row(1, 0, 0, 25, 0);
	add_row(1, 0, 0, 25, 0);
	add_row(2, 0, 0, 25, 0);
	add_row(12, 0, 0, 100, 0);
	add_row(8, 0, 0, 50, 0);
	add_row(2, 0, 0, 50, 0);

	add_column(2, 0, 0, 25, 0);
	add_column(2, 0, 0, 25, 0);
	add_column(12, 0, 0, 100, 0);
	add_column(4, 0, 0, 50, 0);

	oscillator_selector = new IntEditor(this);
	oscillator_selector->set_limits(1, FMPIRE_OSC_COUNT);
	oscillator_selector->set_default_value(1);
	oscillator_selector->set_label("OSC");
	oscillator_selector->set_tooltip("Oscillator");
	oscillator_selector->set_callback(this);
	put(oscillator_selector, 0, 0, 1, 2);

	spectrum_view = new SpectrumView(this);
	put(spectrum_view, 0, 2, 2, 1);

	waveform_editor = new WaveformEditor(this, spectrum_view, state_mgr);
	waveform_editor->set_callback(this);
	put(waveform_editor, 2, 2, 3, 1);

	part_selector = new IntEditor(this);
	part_selector->set_limits(-1, std::numeric_limits<int>::max());
	part_selector->set_label("Part #");
	part_selector->set_callback(this);
	put(part_selector, 3, 0, 1, 2);

	part_editor = new WaveformPartEditor(this);
	part_editor->set_callback(waveform_editor);
	put(part_editor, 4, 0, 1, 2);

	waveform_scoll = new ScrollContainer(this);
	waveform_scoll->set_scroll_mode(ScrollContainer::VERTICAL);

	waveform_selector = new WaveformSelector(waveform_scoll);
	waveform_selector->set_callback(this);

	put(waveform_scoll, 0, 3, 6, 1);

	waveform_add_button = new Button(this);
	waveform_add_button->set_text("+");
	waveform_add_button->set_callback(this);
	put(waveform_add_button, 6, 3);

	tool_selector = new Selector(this);
	tool_selector->add_option("Free");
	tool_selector->add_option("Line");
	tool_selector->add_option("Half Sine");
	tool_selector->add_option("Quarter Sine");
	tool_selector->set_callback(this);
	put(tool_selector, 1, 0, 1, 2);
	tool_selector->select(0, true);

	grid_x_editor = new IntEditor(this);
	grid_x_editor->set_default_value(0);
	grid_x_editor->set_limits(0, std::numeric_limits<int>::max());
	grid_x_editor->set_callback(this);
	grid_x_editor->set_label("Grid X");
	put(grid_x_editor, 2, 0);

	grid_y_editor = new IntEditor(this);
	grid_y_editor->set_default_value(0);
	grid_y_editor->set_limits(0, std::numeric_limits<int>::max());
	grid_y_editor->set_callback(this);
	grid_y_editor->set_label("Grid Y");
	put(grid_y_editor, 2, 1);

	harmonic_scroll = new ScrollContainer(this);
	harmonic_scroll->set_scroll_mode(ScrollContainer::HORIZONTAL);

	harmonic_editor = new HarmonicEditor(harmonic_scroll);
	harmonic_editor->set_callback(waveform_editor);

	put(harmonic_scroll, 5, 2);

	select_oscillator(0);
}

WavetableEditor::~WavetableEditor() noexcept
{
}

void WavetableEditor::select_oscillator(const size_t osc)
{
	selected_oscillator = osc;
	oscillator_selector->set_value(osc + 1);
	wavetable = state_manager.get_wavetable_creator(osc);
	waveform_editor->set_osc_index(osc);
	waveform_editor->set_waveform(wavetable->get_waveform(0));
	spectrum_view->set_waveform(wavetable->get_waveform(0));
	waveform_selector->set_wavetable(state_manager.get_wavetable(osc));
	repaint();
}

void WavetableEditor::on_selected(Selector* const selector,
								  const int index,
								  const std::string& option)
{
	if (selector == tool_selector)
	{
		waveform_editor->select_tool((WaveformToolType) index);
	}
}

void WavetableEditor::on_value_changed(IntEditor* const int_editor,
									   const int value)
{
	if (int_editor == oscillator_selector)
	{
		select_oscillator(value - 1);
	}
	else if (int_editor == grid_x_editor)
	{
		waveform_editor->set_grid(value, grid_y_editor->get_value());
	}
	else if (int_editor == grid_y_editor)
	{
		waveform_editor->set_grid(grid_x_editor->get_value(), value);
	}
	else if (int_editor == part_selector)
	{
		waveform_editor->select(
			wavetable->get_waveform(waveform_selector->get_selected())
				->get_part_by_idx(value));
	}
}

void WavetableEditor::on_press(Button* const button)
{
	if (button == waveform_add_button)
	{
		if (wavetable)
		{
			wavetable->add_waveform();
			uint32_t width, height;
			wavetable->get_size(width, height);

			std::string state;
			height -= 1;
			state += encode_base64(reinterpret_cast<uint8_t*>(&height),
								   sizeof(height));
			state += wavetable->get_waveform(height)->get_state();

			state_manager.set_state(KEY_OSC_PREFIX
										+ std::to_string(selected_oscillator)
										+ "/" KEY_OSC_WAVETABLE KEY_WT_INSERT,
									state);
			state_manager.on_wavetable_edited(selected_oscillator);

			waveform_selector->select_waveform(height);

			repaint();
		}
	}
}

void WavetableEditor::on_select(WaveformSelector* const selector,
								const size_t selected)
{
	waveform_editor->set_waveform(wavetable->get_waveform(selected));
	spectrum_view->set_waveform(wavetable->get_waveform(selected));
}

void WavetableEditor::on_waveform_moved(WaveformSelector* const selector,
										const size_t start,
										const size_t end)
{
	Ref<Waveform> waveform = wavetable->get_waveform(start);

	wavetable->remove_waveform(start);
	wavetable->insert_waveform(end, waveform);

	state_manager.on_wavetable_edited(selected_oscillator);

	uint32_t idx = start;
	std::string state =
		encode_base64(reinterpret_cast<uint8_t*>(&idx), sizeof(idx));
	state_manager.set_state(KEY_OSC_PREFIX + std::to_string(selected_oscillator)
								+ "/" KEY_OSC_WAVETABLE KEY_WT_REMOVE,
							state);

	idx = end;
	state = encode_base64(reinterpret_cast<uint8_t*>(&idx), sizeof(idx));
	state += waveform->get_state();
	state_manager.set_state(KEY_OSC_PREFIX + std::to_string(selected_oscillator)
								+ "/" KEY_OSC_WAVETABLE KEY_WT_INSERT,
							state);

	repaint();
}

void WavetableEditor::remove_waveform(WaveformSelector* const selector,
									  const size_t waveform)
{
	wavetable->remove_waveform(waveform);
	state_manager.on_wavetable_edited(selected_oscillator);

	uint32_t idx = waveform;
	std::string state =
		encode_base64(reinterpret_cast<uint8_t*>(&idx), sizeof(idx));
	state_manager.set_state(KEY_OSC_PREFIX + std::to_string(selected_oscillator)
								+ "/" KEY_OSC_WAVETABLE KEY_WT_REMOVE,
							state);

	repaint();
}

void WavetableEditor::on_waveform_part_selected(WaveformEditor* const editor,
												WaveformPart* const part)
{
	int part_idx = -1;
	for (uint32_t i = 0; true; i++)
	{
		WaveformPart* p =
			wavetable->get_waveform(waveform_selector->get_selected())
				->get_part_by_idx(i);
		if (p == nullptr)
		{
			break;
		}
		if (p == part)
		{
			part_idx = i;
			break;
		}
	}
	part_selector->set_value(part_idx);
	part_editor->set_part(part);

	if (part && part->get_type() == WaveformPart::Type::HARMONIC)
	{
		harmonic_editor->set_part((HarmonicsWaveformPart*) part);
	}
	else
	{
		harmonic_editor->set_part(nullptr);
	}
}

void WavetableEditor::on_waveform_edited(WaveformEditor* const editor,
										 Waveform* const wf,
										 const bool is_done)
{
}

void WavetableEditor::onDisplay()
{
	GridContainer::onDisplay();

	if (wavetable == nullptr)
	{
		return;
	}

	uint32_t wt_width, wt_height;
	wavetable->get_size(wt_width, wt_height);
	waveform_scoll->set_scroll_area(0, 40 * wt_height);

	uint32_t harmonic_count = 128;
	if (waveform_editor->get_selected_part()
		&& waveform_editor->get_selected_part()->get_type()
			   == WaveformPart::Type::HARMONIC)
	{
		harmonic_count =
			((HarmonicsWaveformPart*) waveform_editor->get_selected_part())
				->get_harmonics()
				.size();
	}
	harmonic_scroll->set_scroll_area(HarmonicEditor::box_width * harmonic_count,
									 0);
}

} // namespace fmpire
