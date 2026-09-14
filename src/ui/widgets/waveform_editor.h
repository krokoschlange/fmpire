#ifndef WAVEFORM_EDITOR_H_INCLUDED
#define WAVEFORM_EDITOR_H_INCLUDED

#include "fmpire_widget.h"

#include "button.h"
#include "ref_counted.h"
#include "spectrum_view.h"
#include "waveform.h"
#include "waveform_tools.h"
#include "Widget.hpp"

#include "harmonic_editor.h"
#include "waveform_part_editor.h"

namespace fmpire
{
class HarmonicEditor;
class StateManager;
class Waveform;
class WaveformTool;
class WaveformPart;

class WaveformEditor :
	public FMpireWidget,
	public Button::Callback,
	public WaveformPartEditor::Callback,
	public HarmonicEditor::Callback
{
public:
	WaveformEditor(Widget* parent,
				   SpectrumView* spect_view,
				   StateManager& state_mgr);
	virtual ~WaveformEditor() noexcept;

	void set_waveform(Waveform* wf)
	{
		if (wf == waveform)
		{
			return;
		}

		waveform = wf;
		select(nullptr, true);
		update_delete_button();
		spectrum_view->set_waveform(wf);
		repaint();
	}

	Waveform* get_waveform() const { return waveform; }

	void select_tool(WaveformToolType tool) { selected_tool = tool; };

	void set_osc_index(uint32_t idx) { osc_index = idx; }

	void set_grid(uint32_t x, uint32_t y)
	{
		grid_x = x;
		grid_y = y;
		repaint();
	}

	virtual void on_press(Button* const button) override;

	virtual void on_release(Button* const button) override {}

	WaveformPart* get_selected_part() const { return selection; }

	void select(WaveformPart* part, bool trigger_callback = false);

	virtual void on_part_edited(WaveformPartEditor* const editor,
								WaveformPart* const part) override;

	virtual void on_harmonics_edited(
		HarmonicEditor* const editor,
		HarmonicsWaveformPart* const part) override;

	struct Callback
	{
		virtual void on_waveform_part_selected(WaveformEditor* const editor,
											   WaveformPart* const part) = 0;

		virtual void on_waveform_edited(WaveformEditor* const editor,
										Waveform* const wf,
										const bool is_done) = 0;
	};

	void set_callback(Callback* const cb) { callback = cb; }

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;
	void onPositionChanged(const PositionChangedEvent& event) override;
	void onResize(const ResizeEvent& event) override;

private:
	Ref<SpectrumView> spectrum_view;
	Ref<HarmonicEditor> harmonic_editor;

	Ref<Waveform> waveform;

	Ref<WaveformPart> selection;

	WaveformToolType selected_tool;
	Ref<WaveformTool> active_tool;

	Ref<Button> delete_button;

	uint32_t osc_index;
	uint32_t grid_x;
	uint32_t grid_y;
	StateManager& state_manager;

	Callback* callback;

	void update_delete_button();
	void on_waveform_updated(bool update_dsp);
};


} // namespace fmpire

#endif // WAVEFORM_EDITOR_H_INCLUDED
