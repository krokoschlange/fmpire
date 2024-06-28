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
		  float thin,
		  float thick,
		  float radius,
		  std::string font_face) :
		background(bg),
		foreground(fg),
		highlight(hl),
		line_thin(thin),
		line_thick(thick),
		corner_radius(radius),
		font(font_face)
	{
	}

	Color background;
	Color foreground;
	Color highlight;
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

} // namespace fmpire

#endif // FMPIRE_THEME_H_INCLUDED
