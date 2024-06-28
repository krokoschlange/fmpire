#include "theme.h"

namespace fmpire
{

Theme ThemeCollection::themes[5] = {
	Theme(Color(99, 106, 119),
		  Color(35, 87, 136),
		  Color(0, 128, 255),
		  3,
		  6,
		  5,
		  "Noto Sans Black"),
	Theme(Color(99, 106, 119),
		  Color(133, 68, 34),
		  Color(255, 93, 8),
		  3,
		  6,
		  5,
		  "Noto Sans Heavy"),
	Theme(Color(99, 106, 119),
		  Color(35, 135, 60),
		  Color(2, 255, 65),
		  3,
		  6,
		  5,
		  "Noto Sans Heavy"),
	Theme(Color(99, 106, 119),
		  Color(134, 35, 67),
		  Color(255, 0, 83),
		  3,
		  6,
		  5,
		  "Noto Sans Heavy"),
	Theme(Color(20, 20, 20),
		  Color(109, 109, 109),
		  Color(229, 229, 229),
		  3,
		  6,
		  5,
		  "Noto Sans Heavy"),
};

}
