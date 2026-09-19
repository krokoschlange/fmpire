#include "waveform_bulk_editor.h"

#include "label.h"
#include "state_manager.h"

#include <limits>

namespace fmpire
{

namespace
{
constexpr float LABEL_PROPORTION = 0.3f;
} // namespace

WaveformBulkEditor::WaveformBulkEditor(Widget* parent, StateManager& state_mgr) :
	GridContainer(parent),
	state_manager(state_mgr),
	callback(nullptr),
	selected_op(0),
	wav_amount_is_count(false),
	selected_morph_type(0)
{
	add_row(1, 0, 0, 26, 34);
	add_row(4, 0, 0, 100, 0);
	add_row(1, 0, 0, 30, 38);
	add_column(1, 0, 0, 0, 0);

	op_selector = new Selector(this);
	op_selector->add_option("Math");
	op_selector->add_option("WAV");
	op_selector->add_option("Morph");
	op_selector->set_callback(this);
	put(op_selector, 0, 0);

	// Math group
	math_group = new GridContainer(this);
	math_group->add_row(1, 0, 0, 26, 34);
	math_group->add_row(1, 0, 0, 26, 34);
	math_group->add_row(1, 0, 0, 26, 34);
	math_group->add_column(1, 0, 0, 0, 0);
	math_group->add_column(2, 0, 0, 0, 0);
	put(math_group, 1, 0);

	math_function_label = new Label(math_group);
	math_function_label->set_text("f(x,y)=");
	math_group->put(math_function_label, 0, 0);

	math_function_editor = new TextEntry(math_group);
	math_group->put(math_function_editor, 0, 1);

	math_start_editor = new IntEditor(math_group);
	math_start_editor->set_label_position(IntEditor::LabelPosition::LEFT,
										  LABEL_PROPORTION);
	math_start_editor->set_limits(0, std::numeric_limits<int>::max());
	math_start_editor->set_label("Start");
	math_group->put(math_start_editor, 1, 0, 1, 2);

	math_amount_editor = new IntEditor(math_group);
	math_amount_editor->set_label_position(IntEditor::LabelPosition::LEFT,
										   LABEL_PROPORTION);
	math_amount_editor->set_limits(1, std::numeric_limits<int>::max());
	math_amount_editor->set_default_value(1);
	math_amount_editor->set_label("Amount");
	math_group->put(math_amount_editor, 2, 0, 1, 2);

	// WAV group
	wav_group = new GridContainer(this);
	wav_group->add_row(1, 0, 0, 26, 34);
	wav_group->add_row(1, 0, 0, 26, 34);
	wav_group->add_row(1, 0, 0, 26, 34);
	wav_group->add_row(1, 0, 0, 26, 34);
	wav_group->add_column(3, 0, 0, 0, 0);
	wav_group->add_column(1, 0, 0, 0, 0);
	put(wav_group, 1, 0);

	wav_file_editor = new TextEntry(wav_group);
	wav_group->put(wav_file_editor, 0, 0);

	wav_browse_button = new Button(wav_group);
	wav_browse_button->set_text("...");
	wav_browse_button->set_callback(this);
	wav_group->put(wav_browse_button, 0, 1);

	wav_start_editor = new IntEditor(wav_group);
	wav_start_editor->set_label_position(IntEditor::LabelPosition::LEFT,
										 LABEL_PROPORTION);
	wav_start_editor->set_limits(0, std::numeric_limits<int>::max());
	wav_start_editor->set_label("Start");
	wav_group->put(wav_start_editor, 1, 0, 1, 2);

	wav_amount_mode_selector = new Selector(wav_group);
	wav_amount_mode_selector->add_option("Count");
	wav_amount_mode_selector->add_option("Width");
	wav_amount_mode_selector->set_callback(this);
	wav_group->put(wav_amount_mode_selector, 2, 0, 1, 2);

	wav_amount_editor = new IntEditor(wav_group);
	wav_amount_editor->set_label_position(IntEditor::LabelPosition::LEFT,
										  LABEL_PROPORTION);
	wav_amount_editor->set_limits(1, std::numeric_limits<int>::max());
	wav_amount_editor->set_default_value(2048);
	wav_group->put(wav_amount_editor, 3, 0, 1, 2);

	wav_amount_mode_selector->select(1, false); // default: Width, matches cr42ynth
	update_wav_amount_label();

	// Morph group
	morph_group = new GridContainer(this);
	morph_group->add_row(1, 0, 0, 26, 34);
	morph_group->add_row(1, 0, 0, 26, 34);
	morph_group->add_row(1, 0, 0, 26, 34);
	morph_group->add_column(1, 0, 0, 0, 0);
	put(morph_group, 1, 0);

	morph_start_editor = new IntEditor(morph_group);
	morph_start_editor->set_label_position(IntEditor::LabelPosition::LEFT,
										   LABEL_PROPORTION);
	morph_start_editor->set_limits(0, std::numeric_limits<int>::max());
	morph_start_editor->set_label("Start");
	morph_group->put(morph_start_editor, 0, 0);

	morph_amount_editor = new IntEditor(morph_group);
	morph_amount_editor->set_label_position(IntEditor::LabelPosition::LEFT,
											LABEL_PROPORTION);
	morph_amount_editor->set_limits(1, std::numeric_limits<int>::max());
	morph_amount_editor->set_default_value(1);
	morph_group->put(morph_amount_editor, 1, 0);
	morph_amount_editor->set_label("Amount");

	morph_type_selector = new Selector(morph_group);
	morph_type_selector->add_option("Crossfade");
	morph_type_selector->add_option("Spectral");
	morph_type_selector->add_option("Spectral (0 fund.)");
	morph_type_selector->add_option("Spectral (0 all)");
	morph_type_selector->set_callback(this);
	morph_group->put(morph_type_selector, 2, 0);
	morph_type_selector->select(0, false);

	add_button = new Button(this);
	add_button->set_text("Add");
	add_button->set_callback(this);
	put(add_button, 2, 0);

	op_selector->select(0, false);
	update_group_visibility();
}

WaveformBulkEditor::~WaveformBulkEditor() noexcept
{
}

void WaveformBulkEditor::update_group_visibility()
{
	math_group->setVisible(selected_op == 0);
	wav_group->setVisible(selected_op == 1);
	morph_group->setVisible(selected_op == 2);
}

void WaveformBulkEditor::update_wav_amount_label()
{
	wav_amount_editor->set_label(wav_amount_is_count ? "Amount" : "Width");
}

void WaveformBulkEditor::on_selected(Selector* const selector,
									 const int index,
									 const std::string& option)
{
	if (selector == op_selector)
	{
		selected_op = index;
		update_group_visibility();
	}
	else if (selector == wav_amount_mode_selector)
	{
		wav_amount_is_count = index == 0;
		update_wav_amount_label();
	}
	else if (selector == morph_type_selector)
	{
		selected_morph_type = index;
	}
}

void WaveformBulkEditor::on_press(Button* const button)
{
	if (button == wav_browse_button)
	{
		state_manager.open_file_browser(
			[this](const char* filename)
			{
				if (filename)
				{
					wav_file_editor->set_text(std::string(filename));
				}
			});
	}
	else if (button == add_button)
	{
		if (!callback)
		{
			return;
		}

		switch (selected_op)
		{
		case 0:
			callback->on_bulk_math(math_start_editor->get_value(),
								   math_amount_editor->get_value(),
								   math_function_editor->get_text_utf8());
			break;
		case 1:
		{
			int amount = wav_amount_is_count ? wav_amount_editor->get_value() : -1;
			int width = wav_amount_is_count ? -1 : wav_amount_editor->get_value();
			callback->on_bulk_wav(wav_start_editor->get_value(),
								  amount,
								  width,
								  wav_file_editor->get_text_utf8());
			break;
		}
		case 2:
			if (selected_morph_type == 0)
			{
				callback->on_bulk_crossfade(morph_start_editor->get_value(),
											morph_amount_editor->get_value());
			}
			else
			{
				callback->on_bulk_spectral(morph_start_editor->get_value(),
										   morph_amount_editor->get_value(),
										   selected_morph_type == 3,
										   selected_morph_type >= 2);
			}
			break;
		default:
			break;
		}
	}
}

} // namespace fmpire
