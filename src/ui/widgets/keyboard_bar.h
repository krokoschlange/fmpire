#ifndef FMPIRE_KEYBOARD_BAR_H_INCLUDED
#define FMPIRE_KEYBOARD_BAR_H_INCLUDED

#include "int_editor.h"
#include "piano_keyboard.h"
#include "relative_container.h"

namespace fmpire
{

// The strip at the bottom of the window: octave control and the keyboard.
class KeyboardBar : public RelativeContainer, public IntEditor::Callback
{
public:
	explicit KeyboardBar(Widget* parent);
	virtual ~KeyboardBar() noexcept;

	void set_callback(PianoKeyboard::Callback* const cb);

	void idle();

	void on_value_changed(IntEditor* const editor, const int value) override;

private:
	Ref<IntEditor> octave_editor;
	Ref<PianoKeyboard> keyboard;
};

} // namespace fmpire

#endif // FMPIRE_KEYBOARD_BAR_H_INCLUDED
