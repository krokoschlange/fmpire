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

	harmonic_type_selector = new Selector(this);
	harmonic_type_selector->add_option("Sin");
	harmonic_type_selector->add_option("Tri");
	harmonic_type_selector->add_option("Saw");
	harmonic_type_selector->add_option("Sqr");
	harmonic_type_selector->set_callback(this);
	put(harmonic_type_selector, 1, 0);
}

WaveformPartEditor::~WaveformPartEditor() noexcept
{
}

void WaveformPartEditor::on_selected(Selector* const selector,
									 const int index,
									 const std::string& option)
{
	if (selector == type_selector)
	{
		if (part && (int) part->get_type() != index)
		{
			Ref<WaveformPart> new_part =
				WaveformPart::create((WaveformPart::Type) index);
			new_part->set_start(part->get_start());
			new_part->set_end(part->get_end());

			if (callback)
			{
				callback->on_part_edited(this, new_part);
			}
		}
	}
	else if (selector == harmonic_type_selector)
	{
		if (part && part->get_type() == WaveformPart::Type::HARMONIC)
		{
			Ref<HarmonicsWaveformPart> harmonic =
				static_ref_cast<HarmonicsWaveformPart>(part);

			harmonic->set_harmonic_type(
				(HarmonicsWaveformPart::HarmonicType) index);

			if (callback)
			{
				callback->on_part_edited(this, part);
			}
		}
	}
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

	bool is_function = part && part->get_type() == WaveformPart::Type::FUNCTION;
	function_editor->setVisible(is_function);

	if (is_function)
	{
		Ref<FunctionWaveformPart> function =
			static_ref_cast<FunctionWaveformPart>(part);

		function_editor->set_text(function->get_function());
	}

	bool is_harmonic = part && part->get_type() == WaveformPart::Type::HARMONIC;
	harmonic_type_selector->setVisible(is_harmonic);

	if (is_harmonic)
	{
		Ref<HarmonicsWaveformPart> harmonic =
			static_ref_cast<HarmonicsWaveformPart>(part);

		harmonic_type_selector->select((int) harmonic->get_harmonic_type());
	}
}

} // namespace fmpire
