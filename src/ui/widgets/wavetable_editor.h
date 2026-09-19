#ifndef WAVETABLE_EDITOR_H_INCLUDED
#define WAVETABLE_EDITOR_H_INCLUDED

#include "border.h"
#include "button.h"
#include "defines.h"
#include "grid_container.h"
#include "int_editor.h"
#include "selector.h"
#include "waveform_bulk_editor.h"
#include "waveform_editor.h"
#include "waveform_part_editor.h"
#include "waveform_selector.h"
#include "wavetable_history.h"

#include <array>

namespace fmpire
{
class HarmonicEditor;
class ScrollContainer;
class SpectrumView;
class StateManager;
class WavetableCreator;

class WavetableEditor :
	public GridContainer,
	public Selector::Callback,
	public IntEditor::Callback,
	public Button::Callback,
	public WaveformSelector::Callback,
	public WaveformEditor::Callback,
	public WaveformBulkEditor::Callback
{
public:
	WavetableEditor(Widget* parent, StateManager& state_mgr);
	virtual ~WavetableEditor() noexcept;

	virtual void select_oscillator(const size_t osc);

	virtual void on_selected(Selector* const selector,
							 const int index,
							 const std::string& option) override;

	virtual void on_value_changed(IntEditor* const editor,
								  const int value) override;

	virtual void on_press(Button* const button) override;

	virtual void on_release(Button* const button) override {}

	virtual void on_select(WaveformSelector* const selector,
						   const size_t selected) override;
	virtual void on_waveform_moved(WaveformSelector* const selector,
								   const size_t start,
								   const size_t end) override;
	virtual void remove_waveform(WaveformSelector* const selector,
								 const size_t waveform) override;

	virtual void on_waveform_part_selected(WaveformEditor* const editor,
										   WaveformPart* const part) override;

	virtual void on_waveform_edited(WaveformEditor* const editor,
									Waveform* const wf,
									const bool is_done) override;

	virtual void on_bulk_math(uint32_t start,
							  uint32_t amount,
							  const std::string& function) override;
	virtual void on_bulk_wav(uint32_t start,
							 int amount,
							 int width,
							 const std::string& filepath) override;
	virtual void on_bulk_crossfade(uint32_t start, uint32_t amount) override;
	virtual void on_bulk_spectral(uint32_t start,
								  uint32_t amount,
								  bool zero_all,
								  bool zero_fundamental) override;

protected:
	virtual void onDisplay() override;

private:
	Ref<GridContainer> left_column;

	Ref<Border> tools_panel_border;
	Ref<GridContainer> tools_panel_grid;

	Ref<Border> part_panel_border;
	Ref<GridContainer> part_panel_grid;

	Ref<Border> bulk_panel_border;
	Ref<WaveformBulkEditor> bulk_editor;

	Ref<Border> grid_toolbar_border;
	Ref<GridContainer> grid_toolbar_grid;

	Ref<IntEditor> oscillator_selector;
	Ref<WaveformEditor> waveform_editor;
	Ref<ScrollContainer> waveform_scoll;
	Ref<WaveformSelector> waveform_selector;
	Ref<Button> waveform_add_button;
	Ref<SpectrumView> spectrum_view;
	Ref<IntEditor> part_selector;
	Ref<WaveformPartEditor> part_editor;

	Ref<ScrollContainer> harmonic_scroll;
	Ref<HarmonicEditor> harmonic_editor;

	Ref<Selector> tool_selector;
	Ref<IntEditor> grid_x_editor;
	Ref<IntEditor> grid_y_editor;

	Ref<Button> undo_button;
	Ref<Button> redo_button;

	WavetableCreator* wavetable;

	uint32_t selected_oscillator;
	StateManager& state_manager;

	std::array<WavetableHistory, FMPIRE_OSC_COUNT> history;

	void undo();
	void redo();

	void push_history();
	void apply_state(const std::string& snapshot);
	void refresh_editor_view();
	void apply_history_snapshot(const std::string& snapshot);
	void update_undo_redo_buttons();

	void apply_bulk_insert(uint32_t start,
						   const std::vector<Ref<Waveform>>& new_waveforms);
};

}; // namespace fmpire

#endif // WAVETABLE_EDITOR_H_INCLUDED
