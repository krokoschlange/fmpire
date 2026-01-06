#ifndef WAVEFORM_PART_EDITOR_H_INCLUDED
#define WAVEFORM_PART_EDITOR_H_INCLUDED

#include "grid_container.h"
#include "selector.h"
#include "text_entry.h"

namespace fmpire
{
class WaveformPart;

class WaveformPartEditor :
	public GridContainer,
	public Selector::Callback,
	public TextEntry::Callback
{
public:
	WaveformPartEditor(Widget* parent);
	virtual ~WaveformPartEditor() noexcept;

	virtual void on_selected(Selector* const selector,
							 const int index,
							 const std::string& option) override;

	virtual void on_value_changed(TextEntry* const text_entry,
								  const std::wstring& value) override;

	void set_part(WaveformPart* const p);

	struct Callback
	{
		virtual void on_part_edited(WaveformPartEditor* const part_editor,
									WaveformPart* const part) = 0;
	};

	void set_callback(Callback* const cb) { callback = cb; }

private:
	Ref<Selector> type_selector;
	Ref<TextEntry> function_editor;

	Ref<WaveformPart> part;

	Callback* callback;
};

} // namespace fmpire

#endif // WAVEFORM_PART_EDITOR_H_INCLUDED
