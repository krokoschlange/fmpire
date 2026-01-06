#include "waveform_part_editor.h"

#include "ref_counted.h"
#include "selector.h"
#include "text_entry.h"
#include "waveform_part.h"

namespace fmpire
{

WaveformPartEditor::WaveformPartEditor(Widget* parent) :
	GridContainer(parent)
{
	add_row(1, 0, 0, 0, 20);
	add_row(1, 0, 0, 0, 20);
	add_row(1, 0, 0, 0, 0);

	add_column(1, 0, 0, 0, 0);

	type_selector = new Selector(this);
	type_selector->add_option("SMPL");
	type_selector->add_option("FUNC");
	type_selector->add_option("HARM");
	type_selector->set_callback(this);
	put(type_selector, 0, 0);

	function_editor = new TextEntry(this);
	function_editor->set_callback(this);
	put(function_editor, 1, 0);
}

WaveformPartEditor::~WaveformPartEditor() noexcept
{
}

void WaveformPartEditor::on_selected(Selector* const selector,
									 const int index,
									 const std::string& option)
{
}

void WaveformPartEditor::on_value_changed(TextEntry* const text_entry,
										  const std::wstring& value)
{
	if (text_entry == function_editor)
	{
		if (part && part->get_type() == WaveformPart::Type::FUNCTION)
		{
			Ref<FunctionWaveformPart> function =
				static_ref_cast<FunctionWaveformPart>(part);

			function->set_function(text_entry->get_text_utf8());

			if (callback)
			{
				callback->on_part_edited(this, part);
			}
		}
	}
}

void WaveformPartEditor::set_part(WaveformPart* const p)
{
	part = p;

	if (part)
	{
		type_selector->select((int) part->get_type());
	}

	if (part && part->get_type() == WaveformPart::Type::FUNCTION)
	{
		Ref<FunctionWaveformPart> function =
			static_ref_cast<FunctionWaveformPart>(part);

		function_editor->set_text(function->get_function());
	}
}

} // namespace fmpire
