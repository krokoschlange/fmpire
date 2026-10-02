#include "piano_keyboard.h"

#include "draw_operations.h"
#include "fmpire_window.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace fmpire
{

namespace
{
constexpr int WHITE_KEYS_PER_OCTAVE = 7;
constexpr int WHITE_KEY_COUNT =
	PianoKeyboard::OCTAVE_COUNT * WHITE_KEYS_PER_OCTAVE + 1;

constexpr int WHITE_KEY_SEMITONES[WHITE_KEYS_PER_OCTAVE] =
	{0, 2, 4, 5, 7, 9, 11};

struct BlackKey
{
	// the key sits on the boundary after this white key of the octave
	int after_white_key;
	int semitone;
};

constexpr BlackKey BLACK_KEYS[] = {{0, 1}, {1, 3}, {3, 6}, {4, 8}, {5, 10}};

constexpr float BLACK_KEY_WIDTH = 0.6f;
constexpr float BLACK_KEY_HEIGHT = 0.62f;

constexpr int MIN_VELOCITY = 30;
constexpr int MAX_VELOCITY = 127;
constexpr int COMPUTER_KEY_VELOCITY = 100;

struct ComputerKey
{
	uint key;
	// semitones above the leftmost C of the keyboard
	int semitone;
};

// White keys on the home row, black keys on the row above. Keys are matched by
// the character they produce, so 'y' and 'z' both play the same note to cover
// QWERTY and QWERTZ layouts, where those two swap places.
constexpr ComputerKey COMPUTER_KEYS[] = {{'a', 0},
										 {'w', 1},
										 {'s', 2},
										 {'e', 3},
										 {'d', 4},
										 {'f', 5},
										 {'t', 6},
										 {'g', 7},
										 {'y', 8},
										 {'z', 8},
										 {'h', 9},
										 {'u', 10},
										 {'j', 11},
										 {'k', 12},
										 {'o', 13},
										 {'l', 14},
										 {'p', 15}};

// keys that belong to the host or the window system rather than the piano
constexpr uint SHORTCUT_MODIFIERS =
	kModifierControl | kModifierAlt | kModifierSuper;
} // namespace

PianoKeyboard::PianoKeyboard(Widget* parent) :
	FMpireWidget(parent),
	octave(DEFAULT_OCTAVE),
	pressed_note(-1),
	callback(nullptr)
{
}

PianoKeyboard::~PianoKeyboard() noexcept
{
}

void PianoKeyboard::set_octave(const int new_octave)
{
	octave = std::clamp(new_octave, MIN_OCTAVE, MAX_OCTAVE);
	repaint();
}

void PianoKeyboard::set_callback(Callback* const cb)
{
	callback = cb;
}

int PianoKeyboard::lowest_note() const
{
	return (octave + 1) * 12;
}

bool PianoKeyboard::is_sounding(const int note) const
{
	if (note == pressed_note)
	{
		return true;
	}
	for (const auto& [key, key_note] : held_keys)
	{
		if (key_note == note)
		{
			return true;
		}
	}
	return false;
}

float PianoKeyboard::white_key_width() const
{
	return static_cast<float>(getWidth()) / WHITE_KEY_COUNT;
}

float PianoKeyboard::black_key_height() const
{
	return getHeight() * BLACK_KEY_HEIGHT;
}

int PianoKeyboard::note_at(const float x, const float y) const
{
	const float white_width = white_key_width();

	if (y < black_key_height())
	{
		const float half_black_width = white_width * BLACK_KEY_WIDTH / 2;
		for (int oct = 0; oct < OCTAVE_COUNT; oct++)
		{
			for (const BlackKey& black : BLACK_KEYS)
			{
				const float center =
					(oct * WHITE_KEYS_PER_OCTAVE + black.after_white_key + 1)
					* white_width;
				if (std::abs(x - center) < half_black_width)
				{
					return lowest_note() + oct * 12 + black.semitone;
				}
			}
		}
	}

	const int white_idx =
		std::clamp(static_cast<int>(x / white_width), 0, WHITE_KEY_COUNT - 1);
	return lowest_note() + (white_idx / WHITE_KEYS_PER_OCTAVE) * 12
		 + WHITE_KEY_SEMITONES[white_idx % WHITE_KEYS_PER_OCTAVE];
}

int PianoKeyboard::velocity_at(const float y) const
{
	// hitting a key further down plays it harder
	const float position = std::clamp(y / getHeight(), 0.0f, 1.0f);
	return MIN_VELOCITY
		 + static_cast<int>(position * (MAX_VELOCITY - MIN_VELOCITY));
}

void PianoKeyboard::press_note(const int note, const int velocity)
{
	pressed_note = note;
	if (callback)
	{
		callback->on_key_pressed(this, note, velocity);
	}
	repaint();
}

void PianoKeyboard::release_note()
{
	if (pressed_note < 0)
	{
		return;
	}
	const int note = pressed_note;
	pressed_note = -1;
	if (callback)
	{
		callback->on_key_released(this, note);
	}
	repaint();
}

void PianoKeyboard::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	const float line_width = theme->line_very_thin;
	const float radius = theme->corner_radius;
	const float white_width = white_key_width();
	const float height = getHeight();

	for (int white_idx = 0; white_idx < WHITE_KEY_COUNT; white_idx++)
	{
		const int semitone = WHITE_KEY_SEMITONES[white_idx
												 % WHITE_KEYS_PER_OCTAVE];
		const int note =
			lowest_note() + (white_idx / WHITE_KEYS_PER_OCTAVE) * 12 + semitone;
		const float left = white_idx * white_width;

		if (is_sounding(note))
		{
			theme->highlight.setFor(context);
		}
		else
		{
			Color(230, 230, 230).setFor(context);
		}
		fill_rounded_box(context,
						 left,
						 0,
						 white_width,
						 height,
						 radius,
						 line_width,
						 Corner::BOTTOM);

		theme->foreground.setFor(context);
		draw_rounded_box(context,
						 left,
						 0,
						 white_width,
						 height,
						 radius,
						 line_width,
						 Corner::BOTTOM);

		if (semitone == 0)
		{
			Color(60, 60, 60).setFor(context);
			draw_text_clipped(context,
							  ("C"
							   + std::to_string(octave
												+ white_idx
													  / WHITE_KEYS_PER_OCTAVE))
								  .c_str(),
							  theme->font.c_str(),
							  scaled_font_size(font_role::LABEL,
											   window->get_ui_scale()),
							  Anchor::BOTTOM_CENTER,
							  left + white_width / 2,
							  height - line_width - 3,
							  left,
							  0,
							  white_width,
							  height);
		}
	}

	const float black_width = white_width * BLACK_KEY_WIDTH;
	const float black_height = black_key_height();
	for (int oct = 0; oct < OCTAVE_COUNT; oct++)
	{
		for (const BlackKey& black : BLACK_KEYS)
		{
			const int note = lowest_note() + oct * 12 + black.semitone;
			const float center =
				(oct * WHITE_KEYS_PER_OCTAVE + black.after_white_key + 1)
				* white_width;

			if (is_sounding(note))
			{
				theme->highlight.setFor(context);
			}
			else
			{
				Color(25, 25, 25).setFor(context);
			}
			fill_rounded_box(context,
							 center - black_width / 2,
							 0,
							 black_width,
							 black_height,
							 radius,
							 line_width,
							 Corner::BOTTOM);

			theme->foreground.setFor(context);
			draw_rounded_box(context,
							 center - black_width / 2,
							 0,
							 black_width,
							 black_height,
							 radius,
							 line_width,
							 Corner::BOTTOM);
		}
	}
}

bool PianoKeyboard::onMouse(const MouseEvent& event)
{
	if (event.button != 1)
	{
		return false;
	}

	if (!event.press)
	{
		if (pressed_note < 0)
		{
			return false;
		}
		release_note();
		return true;
	}

	if (!contains_clipped(event.pos))
	{
		return false;
	}

	press_note(note_at(event.pos.getX(), event.pos.getY()),
			   velocity_at(event.pos.getY()));
	return true;
}

bool PianoKeyboard::onMotion(const MotionEvent& event)
{
	if (pressed_note < 0)
	{
		return false;
	}

	// While the mouse button is held this widget owns the mouse, like a knob
	// being dragged: sliding over the keys plays them one after the other, but
	// leaving the keyboard keeps the last note sounding until the release.
	if (contains_clipped(event.pos))
	{
		const int note = note_at(event.pos.getX(), event.pos.getY());
		if (note != pressed_note)
		{
			release_note();
			press_note(note, velocity_at(event.pos.getY()));
		}
	}
	return true;
}

bool PianoKeyboard::onKeyboard(const KeyboardEvent& event)
{
	// a release must always end its note, even if a field got focus meanwhile
	if (!event.press)
	{
		return release_key(event.key, event.time);
	}

	// text and number fields need the keys for typing
	if (window->get_focus() != nullptr || (event.mod & SHORTCUT_MODIFIERS))
	{
		return false;
	}

	return press_key(event.key, event.time);
}

bool PianoKeyboard::press_key(const uint key, const uint time)
{
	for (const ComputerKey& computer_key : COMPUTER_KEYS)
	{
		if (computer_key.key != key)
		{
			continue;
		}

		const auto pending = pending_releases.find(key);
		if (pending != pending_releases.end())
		{
			const bool is_repeat = pending->second == time;
			pending_releases.erase(pending);
			if (is_repeat)
			{
				// key repeat, the note just keeps sounding
				return true;
			}
			finish_key(key);
		}

		// some systems repeat the press without a release in between
		if (!held_keys.contains(key))
		{
			const int note = lowest_note() + computer_key.semitone;
			held_keys[key] = note;
			if (callback)
			{
				callback->on_key_pressed(this, note, COMPUTER_KEY_VELOCITY);
			}
			repaint();
		}
		return true;
	}
	return false;
}

bool PianoKeyboard::release_key(const uint key, const uint time)
{
	if (!held_keys.contains(key))
	{
		return false;
	}

	pending_releases[key] = time;
	return true;
}

void PianoKeyboard::finish_key(const uint key)
{
	const auto held = held_keys.find(key);
	if (held == held_keys.end())
	{
		return;
	}

	const int note = held->second;
	held_keys.erase(held);
	if (callback)
	{
		callback->on_key_released(this, note);
	}
	repaint();
}

void PianoKeyboard::idle()
{
	for (const auto& [key, time] : pending_releases)
	{
		finish_key(key);
	}
	pending_releases.clear();
}

} // namespace fmpire
