#ifndef WAVETABLE_EDITOR_H_INCLUDED
#define WAVETABLE_EDITOR_H_INCLUDED

#include "button.h"
#include "grid_container.h"
#include "int_editor.h"
#include "selector.h"
#include "waveform_editor.h"
#include "waveform_part_editor.h"
#include "waveform_selector.h"

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
	public WaveformEditor::Callback
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

protected:
	virtual void onDisplay() override;

private:
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

	WavetableCreator* wavetable;

	uint32_t selected_oscillator;
	StateManager& state_manager;
};

}; // namespace fmpire

#endif // WAVETABLE_EDITOR_H_INCLUDED
