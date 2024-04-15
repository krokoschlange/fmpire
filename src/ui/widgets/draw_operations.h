#ifndef DRAW_OPERATIONS_H_INCLUDED
#define DRAW_OPERATIONS_H_INCLUDED

#include "DistrhoUI.hpp"

namespace fmpire
{

void draw_rounded_box(const GraphicsContext& context,
					  float top,
					  float left,
					  float width,
					  float height,
					  float radius,
					  float line_width = 1);


}

#endif // DRAW_OPERATIONS_H_INCLUDED
