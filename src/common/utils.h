#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "arc.h"

namespace fmpire
{

float lerp(float a, float b, float t);

double lerp(double a, double b, double t);

void draw_rounded_box(const GraphicsContext& context, float top, float left, float width, float height, float radius, float line_width = 1);


}

#endif // UTILS_H_INCLUDED
