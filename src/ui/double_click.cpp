#include "double_click.h"

#include "DistrhoUtils.hpp"

namespace fmpire
{
void DoubleClick::set_threshold(uint time)
{
	threshold = time;
}

bool DoubleClick::is_double_click(uint button, uint time)
{
	if (button != current_button
		|| (time - current_time) > difference_threshold)
	{
		last_button = current_button;
		last_time = current_time;
		current_button = button;
		current_time = time;
	}

	return (button == last_button && (time - last_time) < threshold);
}

uint DoubleClick::current_button = 0;
uint DoubleClick::current_time = 0;
uint DoubleClick::last_button = 0;
uint DoubleClick::last_time = 0;
uint DoubleClick::threshold = 300;
uint DoubleClick::difference_threshold = 10;

} // namespace fmpire
