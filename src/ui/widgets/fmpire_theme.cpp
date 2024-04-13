#include "fmpire_theme.h"

namespace fmpire
{

FMpireTheme FMpireThemeCollection::themes[5] = {
	FMpireTheme(Color(99, 106, 119), Color(35, 87, 136), Color(0, 128, 255), 1, 3),
	FMpireTheme(Color(99, 106, 119), Color(133, 68, 34), Color(255, 93, 8), 1, 3),
	FMpireTheme(Color(99, 106, 119), Color(35, 135, 60), Color(2, 255, 65), 1, 3),
	FMpireTheme(Color(99, 106, 119), Color(134, 35, 67), Color(255, 0, 83), 1, 3),
	FMpireTheme(Color(20, 20, 20), Color(109, 109, 109), Color(229, 229, 229), 1, 3),
};

}
