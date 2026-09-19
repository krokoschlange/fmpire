#ifndef WAVEFORM_BULK_EDITOR_H_INCLUDED
#define WAVEFORM_BULK_EDITOR_H_INCLUDED

#include "button.h"
#include "grid_container.h"
#include "int_editor.h"
#include "selector.h"
#include "text_entry.h"

namespace fmpire
{
class Label;
class StateManager;

class WaveformBulkEditor :
	public GridContainer,
	public Selector::Callback,
	public Button::Callback
{
public:
	WaveformBulkEditor(Widget* parent, StateManager& state_mgr);
	virtual ~WaveformBulkEditor() noexcept;

	virtual void on_selected(Selector* const selector,
							 const int index,
							 const std::string& option) override;

	virtual void on_press(Button* const button) override;
	virtual void on_release(Button* const button) override {}

	struct Callback
	{
		virtual void on_bulk_math(uint32_t start,
								  uint32_t amount,
								  const std::string& function) = 0;

		// One of amount/width is derived from the file when negative,
		// matching the "Count" / "Width" mode toggle.
		virtual void on_bulk_wav(uint32_t start,
								 int amount,
								 int width,
								 const std::string& filepath) = 0;

		virtual void on_bulk_crossfade(uint32_t start, uint32_t amount) = 0;
		virtual void on_bulk_spectral(uint32_t start,
									  uint32_t amount,
									  bool zero_all,
									  bool zero_fundamental) = 0;
	};

	void set_callback(Callback* const cb) { callback = cb; }

private:
	StateManager& state_manager;
	Callback* callback;

	Ref<Selector> op_selector;

	Ref<GridContainer> math_group;
	Ref<Label> math_function_label;
	Ref<TextEntry> math_function_editor;
	Ref<IntEditor> math_start_editor;
	Ref<IntEditor> math_amount_editor;

	Ref<GridContainer> wav_group;
	Ref<TextEntry> wav_file_editor;
	Ref<Button> wav_browse_button;
	Ref<IntEditor> wav_start_editor;
	Ref<Selector> wav_amount_mode_selector;
	Ref<IntEditor> wav_amount_editor;

	Ref<GridContainer> morph_group;
	Ref<IntEditor> morph_start_editor;
	Ref<IntEditor> morph_amount_editor;
	Ref<Selector> morph_type_selector;

	Ref<Button> add_button;

	// Selector has no getter for the current selection, so track it here.
	int selected_op;
	bool wav_amount_is_count;
	int selected_morph_type;

	void update_group_visibility();
	void update_wav_amount_label();
};

} // namespace fmpire

#endif // WAVEFORM_BULK_EDITOR_H_INCLUDED
