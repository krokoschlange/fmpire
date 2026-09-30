#include "keyboard_bar.h"

namespace fmpire
{

KeyboardBar::KeyboardBar(Widget* parent) :
	RelativeContainer(parent)
{
	octave_editor = new IntEditor(this);
	octave_editor->set_limits(PianoKeyboard::MIN_OCTAVE,
							  PianoKeyboard::MAX_OCTAVE);
	octave_editor->set_default_value(PianoKeyboard::DEFAULT_OCTAVE);
	octave_editor->set_value(PianoKeyboard::DEFAULT_OCTAVE);
	octave_editor->set_label("Octave");
	octave_editor->set_tooltip("Octave of the leftmost key");
	octave_editor->set_callback(this);
	put(octave_editor, 0.005, 0.15, 0.09, 0.7);

	keyboard = new PianoKeyboard(this);
	keyboard->set_octave(PianoKeyboard::DEFAULT_OCTAVE);
	put(keyboard, 0.1, 0, 0.9, 1);
}

KeyboardBar::~KeyboardBar() noexcept
{
}

void KeyboardBar::set_callback(PianoKeyboard::Callback* const cb)
{
	keyboard->set_callback(cb);
}

void KeyboardBar::idle()
{
	keyboard->idle();
}

void KeyboardBar::on_value_changed(IntEditor* const editor, const int value)
{
	keyboard->set_octave(value);
}

} // namespace fmpire
