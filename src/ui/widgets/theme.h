#ifndef FMPIRE_THEME_H_INCLUDED
#define FMPIRE_THEME_H_INCLUDED

#include "Color.hpp"

#include <string>

namespace fmpire
{


class Theme
{
public:
	Theme(Color bg,
		  Color fg,
		  Color hl,
		  Color sec,
		  float very_thin,
		  float thin,
		  float thick,
		  float radius,
		  std::string font_face) :
		background(bg),
		foreground(fg),
		highlight(hl),
		secondary(sec),
		line_very_thin(very_thin),
		line_thin(thin),
		line_thick(thick),
		corner_radius(radius),
		font(font_face)
	{
	}

	Color background;
	Color foreground;
	Color highlight;
	// Accent used for modulation/automation indicators (knob mod ring, armed
	// source highlight, LFO/envelope playhead), distinct from the selection
	// highlight above.
	Color secondary;
	float line_very_thin;
	float line_thin;
	float line_thick;
	float corner_radius;
	std::string font;
};

class ThemeCollection
{
public:
	static Theme themes[5];
};

// Named font sizes (in px, at a 1.0 UI scale / 1024x768 design reference)
// shared by every widget, so that text of the same visual weight (titles,
// control captions, tooltips, ...) ends up the same size everywhere instead
// of each widget picking its own fraction of its own local size.
namespace font_role
{
constexpr float NAV = 40.0f;
constexpr float TITLE = 24.0f;
constexpr float HEADING = 18.0f;
constexpr float BODY = 14.0f;
constexpr float LABEL = 12.0f;
constexpr float CAPTION = 10.0f;
} // namespace font_role

} // namespace fmpire

#endif // FMPIRE_THEME_H_INCLUDED
