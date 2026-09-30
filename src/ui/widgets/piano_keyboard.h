#ifndef FMPIRE_PIANO_KEYBOARD_H_INCLUDED
#define FMPIRE_PIANO_KEYBOARD_H_INCLUDED

#include "fmpire_widget.h"

#include <map>

namespace fmpire
{

// On-screen keyboard that can be played with the mouse or, while no text or
// number field is being edited, with the computer keyboard (home row and the
// row above it, like the piano layout of most DAWs). It only reports notes
// through its callback, sending them to the DSP is up to the owner.
class PianoKeyboard : public FMpireWidget
{
public:
	static constexpr int OCTAVE_COUNT = 4;
	static constexpr int DEFAULT_OCTAVE = 2;
	static constexpr int MIN_OCTAVE = -1;
	// the topmost key is the C above the last octave and must stay a valid
	// MIDI note
	static constexpr int MAX_OCTAVE = 127 / 12 - 1 - OCTAVE_COUNT;

	PianoKeyboard(Widget* parent);
	virtual ~PianoKeyboard() noexcept;

	// octave of the leftmost C, following the convention that MIDI note 60 is
	// C4
	void set_octave(const int new_octave);
	int get_octave() const { return octave; }

	struct Callback
	{
		virtual void on_key_pressed(PianoKeyboard* const keyboard,
									const int note,
									const int velocity) = 0;
		virtual void on_key_released(PianoKeyboard* const keyboard,
									 const int note) = 0;
	};

	void set_callback(Callback* const cb);

	// has to be called regularly, it ends the notes of released computer keys
	void idle();

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;
	bool onKeyboard(const KeyboardEvent& event) override;

private:
	int octave;
	// notes currently held, as actual MIDI notes so they can still be
	// released after the octave changed. pressed_note is the one held with the
	// mouse (-1 if none), held_keys maps the computer keys to their notes.
	int pressed_note;
	std::map<uint, int> held_keys;
	// Computer keys that were released but whose note still sounds, with the
	// time of the release event. The key repeat of the window system arrives
	// as a release followed by a press with the same time, so a release only
	// takes effect on the next idle, when it is clear no such press follows.
	std::map<uint, uint> pending_releases;
	Callback* callback;

	int lowest_note() const;
	bool is_sounding(const int note) const;
	float white_key_width() const;
	float black_key_height() const;
	int note_at(const float x, const float y) const;
	int velocity_at(const float y) const;

	void press_note(const int note, const int velocity);
	void release_note();

	bool press_key(const uint key, const uint time);
	bool release_key(const uint key, const uint time);
	void finish_key(const uint key);
};

} // namespace fmpire

#endif // FMPIRE_PIANO_KEYBOARD_H_INCLUDED
