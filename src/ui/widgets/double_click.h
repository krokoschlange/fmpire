#ifndef DOUBLE_CLICK_H_INCLUDED
#define DOUBLE_CLICK_H_INCLUDED

#include <src/DistrhoDefines.h>

namespace fmpire
{

class DoubleClick
{
public:
	static void set_threshold(uint time);

	static bool is_double_click(uint button, uint time);

private:
	static uint current_button;
	static uint current_time;
	static uint last_button;
	static uint last_time;
	static uint threshold;
	static uint difference_threshold;
};

} // namespace fmpire

#endif // DOUBLE_CLICK_H_INCLUDED
