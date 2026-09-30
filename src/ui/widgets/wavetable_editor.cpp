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
#include "waveform_part.h"
#include "waveform_selector.h"
#include "waveform_tools.h"
#include "wavetable_creator.h"

#include <algorithm>
#include <sndfile.h>

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

	add_row(2, 0, 0, 0, 0);
	add_row(1, 0, 0, 0, 0);
	add_row(1, 0, 0, 0, 0);
	add_row(2, 0, 0, 0, 0);
	add_row(12, 0, 0, 0, 0);
	add_row(1, 0, 0, 36, 0); // grid-config toolbar, between canvas and harmonic editor
	add_row(8, 0, 0, 0, 0);
	add_row(2, 0, 0, 0, 0);

	add_column(2, 0, 0, 0, 0);
	add_column(2, 0, 0, 0, 0);
	add_column(12, 0, 0, 0, 0);
	add_column(4, 0, 0, 0, 0);

	left_column = new GridContainer(this);
	left_column->add_row(1, 0, 0, 0, 0);    // oscillator selector
	left_column->add_row(5, 8, 0, 0, 0);   // tools panel
	left_column->add_row(4, 8, 0, 0, 0); // part panel
	left_column->add_row(6, 8, 0, 0, 0);   // bulk-ops panel
	left_column->add_row(0, 8, 0, 0, 0);    // spacer
	left_column->add_column(1, 0, 0, 0, 0);
	put(left_column, 0, 0, 8, 2);

	oscillator_selector = new IntEditor(left_column);
	oscillator_selector->set_limits(1, FMPIRE_OSC_COUNT);
	oscillator_selector->set_default_value(1);
	oscillator_selector->set_label("OSC");
	oscillator_selector->set_label_position(IntEditor::LabelPosition::LEFT);
	oscillator_selector->set_tooltip("Oscillator");
	oscillator_selector->set_callback(this);
	left_column->put(oscillator_selector, 0, 0);

	tools_panel_border = new Border(left_column);
	tools_panel_grid = new GridContainer(tools_panel_border);
	tools_panel_grid->add_row(2, 0, 0, 0, 0);
	tools_panel_grid->add_row(1, 0, 0, 0, 0);
	tools_panel_grid->add_row(1, 0, 0, 0, 0);
	tools_panel_grid->add_row(1, 0, 0, 0, 0);
	tools_panel_grid->add_row(1, 0, 0, 0, 0);
	tools_panel_grid->add_column(1, 0, 0, 0, 0);
	tools_panel_grid->add_column(1, 0, 0, 0, 0);
	left_column->put(tools_panel_border, 1, 0);

	tool_selector = new Selector(tools_panel_grid);
	tool_selector->add_option("Free");
	tool_selector->add_option("Line");
	tool_selector->add_option("Half Sine");
	tool_selector->add_option("Quarter Sine");
	tool_selector->set_callback(this);
	tools_panel_grid->put(tool_selector, 0, 0, 1, 2);

	undo_button = new Button(tools_panel_grid);
	undo_button->set_text("Undo");
	undo_button->set_callback(this);
	tools_panel_grid->put(undo_button, 1, 0);

	redo_button = new Button(tools_panel_grid);
	redo_button->set_text("Redo");
	redo_button->set_callback(this);
	tools_panel_grid->put(redo_button, 1, 1);

	to_harmonics_button = new Button(tools_panel_grid);
	to_harmonics_button->set_text("To HARM");
	to_harmonics_button->set_callback(this);
	tools_panel_grid->put(to_harmonics_button, 2, 0);

	to_harmonics_hq_button = new Button(tools_panel_grid);
	to_harmonics_hq_button->set_text("To HARM HQ");
	to_harmonics_hq_button->set_callback(this);
	tools_panel_grid->put(to_harmonics_hq_button, 2, 1);

	to_samples_button = new Button(tools_panel_grid);
	to_samples_button->set_text("To SMPL");
	to_samples_button->set_callback(this);
	tools_panel_grid->put(to_samples_button, 3, 0, 1, 2);

	export_wav_button = new Button(tools_panel_grid);
	export_wav_button->set_text("Export WAV");
	export_wav_button->set_callback(this);
	tools_panel_grid->put(export_wav_button, 4, 0, 1, 2);

	part_panel_border = new Border(left_column);
	part_panel_grid = new GridContainer(part_panel_border);
	part_panel_grid->add_row(3, 2, 3, 0, 0);
	part_panel_grid->add_row(7, 0, 2, 0, 0);
	part_panel_grid->add_column(1, 2, 2, 0, 0);
	left_column->put(part_panel_border, 2, 0);

	part_selector = new IntEditor(part_panel_grid);
	part_selector->set_limits(-1, std::numeric_limits<int>::max());
	part_selector->set_label("Part #");
	part_selector->set_label_position(IntEditor::LabelPosition::LEFT);
	part_selector->set_callback(this);
	part_panel_grid->put(part_selector, 0, 0);

	part_editor = new WaveformPartEditor(part_panel_grid);
	part_panel_grid->put(part_editor, 1, 0);

	bulk_panel_border = new Border(left_column);
	bulk_editor = new WaveformBulkEditor(bulk_panel_border, state_mgr);
	bulk_editor->set_callback(this);
	left_column->put(bulk_panel_border, 3, 0);

	spectrum_view = new SpectrumView(this);
	put(spectrum_view, 0, 2, 2, 1);

	waveform_editor = new WaveformEditor(this, spectrum_view, state_mgr);
	waveform_editor->set_callback(this);
	put(waveform_editor, 2, 2, 3, 1);

	part_editor->set_callback(waveform_editor);
	tool_selector->select(0, true);

	waveform_scoll = new ScrollContainer(this);
	waveform_scoll->set_scroll_mode(ScrollContainer::VERTICAL);

	waveform_selector = new WaveformSelector(waveform_scoll);
	waveform_selector->set_callback(this);

	put(waveform_scoll, 0, 3, 7, 1);

	waveform_add_button = new Button(this);
	waveform_add_button->set_text("+");
	waveform_add_button->set_callback(this);
	put(waveform_add_button, 7, 3);

	grid_toolbar_border = new Border(this);
	grid_toolbar_grid = new GridContainer(grid_toolbar_border);
	grid_toolbar_grid->add_row(1, 0, 0, 0, 0);
	grid_toolbar_grid->add_column(1, 0, 0, 0, 0);
	grid_toolbar_grid->add_column(1, 0, 0, 0, 0);
	put(grid_toolbar_border, 5, 2);

	grid_x_editor = new IntEditor(grid_toolbar_grid);
	grid_x_editor->set_default_value(0);
	grid_x_editor->set_limits(0, std::numeric_limits<int>::max());
	grid_x_editor->set_callback(this);
	grid_x_editor->set_label("Grid X");
	grid_x_editor->set_label_position(IntEditor::LabelPosition::LEFT);
	grid_toolbar_grid->put(grid_x_editor, 0, 0);

	grid_y_editor = new IntEditor(grid_toolbar_grid);
	grid_y_editor->set_default_value(0);
	grid_y_editor->set_limits(0, std::numeric_limits<int>::max());
	grid_y_editor->set_callback(this);
	grid_y_editor->set_label("Grid Y");
	grid_y_editor->set_label_position(IntEditor::LabelPosition::LEFT);
	grid_toolbar_grid->put(grid_y_editor, 0, 1);

	harmonic_scroll = new ScrollContainer(this);
	harmonic_scroll->set_scroll_mode(ScrollContainer::HORIZONTAL);

	harmonic_editor = new HarmonicEditor(harmonic_scroll);
	harmonic_editor->set_callback(waveform_editor);

	put(harmonic_scroll, 6, 2);

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
	waveform_selector->set_wavetable(state_manager.get_wavetable(osc),
									 wavetable);
	waveform_editor->set_osc_index(osc);
	waveform_editor->set_waveform(wavetable->get_waveform(0));
	spectrum_view->set_waveform(wavetable->get_waveform(0));

	if (wavetable && history[osc].is_empty())
	{
		history[osc].reset(wavetable->get_state());
	}
	update_undo_redo_buttons();
	update_tool_states();

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

			push_history();

			repaint();
		}
	}
	else if (button == undo_button)
	{
		undo();
	}
	else if (button == redo_button)
	{
		redo();
	}
	else if (button == to_harmonics_button)
	{
		convert_to_harmonics(false);
	}
	else if (button == to_harmonics_hq_button)
	{
		convert_to_harmonics(true);
	}
	else if (button == to_samples_button)
	{
		convert_to_samples();
	}
	else if (button == export_wav_button)
	{
		export_wavetable();
	}
}

void WavetableEditor::on_select(WaveformSelector* const selector,
								const size_t selected)
{
	waveform_editor->set_waveform(wavetable->get_waveform(selected));
	spectrum_view->set_waveform(wavetable->get_waveform(selected));
	update_tool_states();
}

void WavetableEditor::on_waveform_moved(WaveformSelector* const selector,
										const size_t start,
										const size_t end)
{
	Ref<Waveform> waveform = wavetable->get_waveform(start);

	wavetable->remove_waveform(start);
	wavetable->insert_waveform(end, waveform);
	const bool baked = wavetable->bake_orphaned_interpolated();
	// refreshes the waveform indices and the interpolated waveforms
	wavetable->update();

	state_manager.on_wavetable_edited(selected_oscillator);

	if (baked)
	{
		// the DSP can't know the content the baked waveform had
		send_all_waveforms();
	}
	else
	{
		uint32_t idx = start;
		std::string state =
			encode_base64(reinterpret_cast<uint8_t*>(&idx), sizeof(idx));
		state_manager.set_state(KEY_OSC_PREFIX
									+ std::to_string(selected_oscillator)
									+ "/" KEY_OSC_WAVETABLE KEY_WT_REMOVE,
								state);

		idx = end;
		state = encode_base64(reinterpret_cast<uint8_t*>(&idx), sizeof(idx));
		state += waveform->get_state();
		state_manager.set_state(KEY_OSC_PREFIX
									+ std::to_string(selected_oscillator)
									+ "/" KEY_OSC_WAVETABLE KEY_WT_INSERT,
								state);
	}

	push_history();

	repaint();
}

void WavetableEditor::remove_waveform(WaveformSelector* const selector,
									  const size_t waveform)
{
	wavetable->remove_waveform(waveform);
	// an interpolated waveform that is now at the table edge lost its anchor
	// and becomes a normal waveform (the new anchor of its neighbours)
	const bool baked = wavetable->bake_orphaned_interpolated();
	// refreshes the waveform indices and the interpolated waveforms
	wavetable->update();
	state_manager.on_wavetable_edited(selected_oscillator);

	if (baked)
	{
		// the DSP can't know the content the baked waveform had
		send_all_waveforms();
	}
	else
	{
		uint32_t idx = waveform;
		std::string state =
			encode_base64(reinterpret_cast<uint8_t*>(&idx), sizeof(idx));
		state_manager.set_state(KEY_OSC_PREFIX
									+ std::to_string(selected_oscillator)
									+ "/" KEY_OSC_WAVETABLE KEY_WT_REMOVE,
								state);
	}

	push_history();

	repaint();
}

void WavetableEditor::send_all_waveforms()
{
	state_manager.set_state(KEY_OSC_PREFIX + std::to_string(selected_oscillator)
								+ "/" KEY_OSC_WAVETABLE KEY_WT_ALL,
							wavetable->get_state());
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
	if (is_done)
	{
		push_history();
	}
	// the waveform may just have been made editable
	update_tool_states();
	// the harmonics may have been changed from outside the harmonic editor
	harmonic_editor->repaint();
}

void WavetableEditor::undo()
{
	if (!wavetable || !history[selected_oscillator].undo_possible())
	{
		return;
	}
	apply_history_snapshot(history[selected_oscillator].undo());
}

void WavetableEditor::redo()
{
	if (!wavetable || !history[selected_oscillator].redo_possible())
	{
		return;
	}
	apply_history_snapshot(history[selected_oscillator].redo());
}

void WavetableEditor::push_history()
{
	if (!wavetable)
	{
		return;
	}
	history[selected_oscillator].push(wavetable->get_state());
	update_undo_redo_buttons();
}

void WavetableEditor::apply_state(const std::string& snapshot)
{
	std::string_view view = snapshot;
	wavetable->set_state(KEY_WT_ALL, view);

	state_manager.set_state(KEY_OSC_PREFIX + std::to_string(selected_oscillator)
								+ "/" KEY_OSC_WAVETABLE KEY_WT_ALL,
							snapshot);
	state_manager.on_wavetable_edited(selected_oscillator);
}

void WavetableEditor::refresh_editor_view()
{
	int part_idx = part_selector->get_value();

	waveform_selector->select_waveform(waveform_selector->get_selected());

	Waveform* wf = wavetable->get_waveform(waveform_selector->get_selected());
	waveform_editor->set_waveform(wf);
	spectrum_view->set_waveform(wf);

	WaveformPart* part =
		(wf && part_idx >= 0) ? wf->get_part_by_idx(part_idx) : nullptr;
	waveform_editor->select(part, true);
	update_tool_states();

	repaint();
}

void WavetableEditor::apply_history_snapshot(const std::string& snapshot)
{
	apply_state(snapshot);
	refresh_editor_view();
	update_undo_redo_buttons();
}

void WavetableEditor::update_undo_redo_buttons()
{
	undo_button->set_enabled(history[selected_oscillator].undo_possible());
	redo_button->set_enabled(history[selected_oscillator].redo_possible());
}

void WavetableEditor::update_tool_states()
{
	// interpolated waveforms are derived from their neighbours and can't be
	// changed until they are made editable
	const Waveform* wf =
		wavetable ? wavetable->get_waveform(waveform_selector->get_selected())
				  : nullptr;
	const bool editable = wf && !wf->is_interpolated();

	to_harmonics_button->set_enabled(editable);
	to_harmonics_hq_button->set_enabled(editable);
	to_samples_button->set_enabled(editable);
}

void WavetableEditor::convert_to_harmonics(const bool high_quality)
{
	if (!wavetable)
	{
		return;
	}

	Waveform* wf = wavetable->get_waveform(waveform_selector->get_selected());
	if (!wf || wf->is_interpolated())
	{
		return;
	}

	Ref<HarmonicsWaveformPart> part = static_cast<HarmonicsWaveformPart*>(
		WaveformPart::create(WaveformPart::Type::HARMONIC));
	part->set_start(0);
	part->set_end(wf->get_width());
	part->get_harmonics() = analyze_harmonics(wf->sample_all(), high_quality);

	replace_waveform_parts(wf, part);
}

void WavetableEditor::convert_to_samples()
{
	if (!wavetable)
	{
		return;
	}

	Waveform* wf = wavetable->get_waveform(waveform_selector->get_selected());
	if (!wf || wf->is_interpolated())
	{
		return;
	}

	Ref<SamplesWaveformPart> part = static_cast<SamplesWaveformPart*>(
		WaveformPart::create(WaveformPart::Type::SAMPLES));
	part->set_start(0);
	part->set_end(wf->get_width());
	part->get_samples() = wf->sample_all();

	replace_waveform_parts(wf, part);
}

void WavetableEditor::replace_waveform_parts(Waveform* const wf,
											 WaveformPart* const part)
{
	while (wf->get_part_by_idx(0))
	{
		wf->remove_part(static_cast<size_t>(0));
	}
	wf->insert_part(part);

	apply_state(wavetable->get_state());

	// the new part is the only one
	part_selector->set_value(0);
	refresh_editor_view();
	push_history();
}

void WavetableEditor::export_wavetable()
{
	if (!wavetable)
	{
		return;
	}

	uint32_t width, height;
	wavetable->get_size(width, height);
	if (width == 0 || height == 0)
	{
		return;
	}

	// Snapshot the current wavetable content up front: the dialog is
	// answered asynchronously and the user may switch oscillators (or edit
	// the wavetable) before that happens.
	std::vector<float> samples = wavetable->create_wavetable();

	const std::string default_name =
		"osc" + std::to_string(selected_oscillator + 1) + "_wavetable.wav";

	state_manager.open_file_browser(
		[samples = std::move(samples), width](const char* filename)
		{
			if (!filename)
			{
				return;
			}

			std::string path = filename;
			if (path.size() < 4
				|| path.compare(path.size() - 4, 4, ".wav") != 0)
			{
				path += ".wav";
			}

			SF_INFO info = {};
			// one cycle per "sample" of a synth-style sample rate keeps each
			// waveform exactly `width` samples long, as wavetable-importing
			// hosts (and this plugin's own WAV bulk-import) expect
			info.samplerate = (int) width;
			info.channels = 1;
			info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

			SNDFILE* file = sf_open(path.c_str(), SFM_WRITE, &info);
			if (!file)
			{
				return;
			}

			sf_writef_float(file, samples.data(), (sf_count_t) samples.size());
			sf_close(file);
		},
		true,
		default_name.c_str());
}

void WavetableEditor::apply_bulk_insert(
	uint32_t start,
	const std::vector<Ref<Waveform>>& new_waveforms)
{
	if (!wavetable || new_waveforms.empty())
	{
		return;
	}

	uint32_t width, height;
	wavetable->get_size(width, height);
	start = std::min(start, height);

	for (size_t i = 0; i < new_waveforms.size(); i++)
	{
		wavetable->insert_waveform(start + i, new_waveforms[i]);
	}
	wavetable->update();

	apply_state(wavetable->get_state());
	refresh_editor_view();
	push_history();
}

void WavetableEditor::on_bulk_math(uint32_t start,
								   uint32_t amount,
								   const std::string& function)
{
	if (!wavetable || amount == 0)
	{
		return;
	}

	uint32_t width, height;
	wavetable->get_size(width, height);

	std::vector<Ref<Waveform>> new_waveforms;
	for (uint32_t i = 0; i < amount; i++)
	{
		Ref<Waveform> wf = new Waveform();
		wf->remove_part(0);

		Ref<FunctionWaveformPart> part = static_cast<FunctionWaveformPart*>(
			WaveformPart::create(WaveformPart::Type::FUNCTION));
		part->set_start(0);
		part->set_end(width);
		part->set_function(function);

		wf->insert_part(part);
		new_waveforms.push_back(wf);
	}

	apply_bulk_insert(start, new_waveforms);
}

void WavetableEditor::on_bulk_wav(uint32_t start,
								  int amount,
								  int width_arg,
								  const std::string& filepath)
{
	if (!wavetable)
	{
		return;
	}

	SF_INFO info = {};
	SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &info);
	if (!file)
	{
		return;
	}

	std::vector<float> samples((size_t) info.frames * info.channels);
	sf_readf_float(file, samples.data(), info.frames);
	sf_close(file);

	uint32_t width, height;
	wavetable->get_size(width, height);

	int samples_per_wf;
	uint32_t frame_count;
	if (amount > 0)
	{
		samples_per_wf = (int) ((float) info.frames / amount);
		frame_count = amount;
	}
	else
	{
		samples_per_wf = width_arg > 0 ? width_arg : (int) width;
		frame_count = samples_per_wf > 0 ? info.frames / samples_per_wf : 0;
	}

	if (frame_count == 0 || samples_per_wf <= 0)
	{
		return;
	}

	std::vector<Ref<Waveform>> new_waveforms;
	for (uint32_t i = 0; i < frame_count; i++)
	{
		std::vector<float> wf_samples(width, 0.0f);
		for (uint32_t smpl = 0; smpl < width; smpl++)
		{
			float rel_pos = (float) smpl / width;
			float orig_pos = rel_pos * samples_per_wf;
			int total_pos =
				((int) orig_pos + samples_per_wf * (int) i) * info.channels;
			total_pos =
				std::min<int>(total_pos, info.channels * info.frames - 1);
			float smpl1 = samples[total_pos];
			total_pos =
				std::min<int>(total_pos + 1, info.channels * info.frames - 1);
			float smpl2 = samples[total_pos];
			wf_samples[smpl] =
				smpl1 + (orig_pos - (int) orig_pos) * (smpl2 - smpl1);
		}

		Ref<Waveform> wf = new Waveform();
		wf->remove_part(0);

		Ref<SamplesWaveformPart> part = static_cast<SamplesWaveformPart*>(
			WaveformPart::create(WaveformPart::Type::SAMPLES));
		part->set_start(0);
		part->set_end(width);
		part->get_samples() = wf_samples;

		wf->insert_part(part);
		new_waveforms.push_back(wf);
	}

	apply_bulk_insert(start, new_waveforms);
}

void WavetableEditor::insert_interpolated(const uint32_t start,
										  const uint32_t amount,
										  const InterpolationType type)
{
	if (!wavetable || amount == 0)
	{
		return;
	}

	uint32_t width, height;
	wavetable->get_size(width, height);
	if (height < 2 || start == 0 || start > height - 1)
	{
		return;
	}

	// The content is derived from the neighbouring waveforms by
	// WavetableCreator::update_interpolated() (called by apply_bulk_insert).
	std::vector<Ref<Waveform>> new_waveforms;
	for (uint32_t i = 0; i < amount; i++)
	{
		Ref<Waveform> wf = new Waveform();
		wf->set_interpolation(type);
		new_waveforms.push_back(wf);
	}

	apply_bulk_insert(start, new_waveforms);
}

void WavetableEditor::on_bulk_crossfade(uint32_t start, uint32_t amount)
{
	insert_interpolated(start, amount, InterpolationType::CROSSFADE);
}

void WavetableEditor::on_bulk_spectral(uint32_t start,
									   uint32_t amount,
									   bool zero_all,
									   bool zero_fundamental)
{
	InterpolationType type = InterpolationType::SPECTRAL;
	if (zero_all)
	{
		type = InterpolationType::SPECTRAL_ZERO_ALL;
	}
	else if (zero_fundamental)
	{
		type = InterpolationType::SPECTRAL_ZERO_FUNDAMENTAL;
	}
	insert_interpolated(start, amount, type);
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
