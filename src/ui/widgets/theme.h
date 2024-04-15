#ifndef FMPIRE_THEME_H_INCLUDED
#define FMPIRE_THEME_H_INCLUDED

#include "Color.hpp"

namespace fmpire
{


class Theme
{
public:
	Theme(Color bg, Color fg, Color hl, float thin, float thick) :
		background(bg),
		foreground(fg),
		highlight(hl),
		line_thin(thin),
		line_thick(thick)
	{
	}

	Color background;
	Color foreground;
	Color highlight;
	float line_thin;
	float line_thick;
};

class ThemeCollection
{
public:
	static Theme themes[5];
};

} // namespace fmpire

#endif // FMPIRE_THEME_H_INCLUDED
