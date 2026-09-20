#include "curve.h"

#include "utils.h"

#include <algorithm>
#include <cmath>

namespace fmpire
{

namespace
{

constexpr float min_gap = 1e-4f;

float clamp_range(const float value, const float min, const float max)
{
	return std::min(std::max(value, min), max);
}

// Power easing: 0 -> 0, 1 -> 1, linear for bend 0. A positive bend always
// bulges the curve upwards, whichever direction the segment runs: it rises
// quickly and then flattens out, with an exponent growing exponentially with
// the bend, so a bend of 1 is an (almost) sharp corner. A negative bend is the
// mirror image: it stays low and then shoots up.
float segment_value(const float y0, const float y1, const float bend, const float t)
{
	const float signed_bend = y1 >= y0 ? bend : -bend;
	const float exponent = std::pow(Curve::bend_exponent, std::fabs(signed_bend));
	const float shaped = signed_bend >= 0.0f ? 1.0f - std::pow(1.0f - t, exponent)
											 : std::pow(t, exponent);
	return y0 + (y1 - y0) * shaped;
}

// Where on the anti-diagonal of a segment's box (from (0, 1) to (1, 0), in
// segment coordinates) the bulging-up power curve of the given exponent
// crosses it, as the x position in (0, 0.5]: the root of (1 - a)^p = a.
float diagonal_crossing(const float exponent)
{
	float low = 0.0f;
	float high = 0.5f;
	for (int i = 0; i < 40; i++)
	{
		const float mid = (low + high) * 0.5f;
		if (std::pow(1.0f - mid, exponent) > mid)
		{
			low = mid;
		}
		else
		{
			high = mid;
		}
	}
	return (low + high) * 0.5f;
}

// how much further than the cursor the handle travels from the centre of the
// diagonal, so the strongest bend is reached before the cursor is at the
// corner
constexpr float handle_reach = 1.1f;

} // namespace

Curve::Curve() :
	sustain_index(2)
{
	points = {
		{0.0f, 0.0f, 0.11f},
		{0.08f, 1.0f, -0.11f},
		{0.35f, 0.6f, 0.0f},
		{1.0f, 0.0f, -0.11f},
	};
}

Curve Curve::make_envelope()
{
	return Curve();
}

Curve Curve::make_lfo()
{
	// Approximates a sine with one bend per quarter wave.
	Curve curve;
	curve.points = {
		{0.0f, 0.5f, 0.085f},
		{0.25f, 1.0f, 0.085f},
		{0.5f, 0.5f, -0.085f},
		{0.75f, 0.0f, -0.085f},
		{1.0f, 0.5f, 0.0f},
	};
	curve.sustain_index = 0;
	return curve;
}

void Curve::set_sustain_index(const size_t index)
{
	sustain_index = std::min(index, points.size() - 1);
}

size_t Curve::add_point(const float x, const float y)
{
	if (points.size() >= max_points || !std::isfinite(x) || !std::isfinite(y))
	{
		return npos;
	}

	const float new_x = clamp_range(x, 0.0f, 1.0f);
	size_t index = 1;
	while (index < points.size() - 1 && points[index].x <= new_x)
	{
		index++;
	}

	const float lower = points[index - 1].x;
	const float upper = points[index].x;
	if (upper - lower < 2.0f * min_gap)
	{
		return npos;
	}

	const CurvePoint new_point{
		clamp_range(new_x, lower + min_gap, upper - min_gap),
		clamp_range(y, 0.0f, 1.0f),
		0.0f};
	points.insert(points.begin() + index, new_point);

	if (index <= sustain_index)
	{
		sustain_index++;
	}
	return index;
}

void Curve::remove_point(const size_t index)
{
	if (index == 0 || index + 1 >= points.size() || points.size() <= 2)
	{
		return;
	}

	points.erase(points.begin() + index);
	if (sustain_index > index)
	{
		sustain_index--;
	}
	sustain_index = std::min(sustain_index, points.size() - 1);
}

void Curve::move_point(const size_t index, const float x, const float y)
{
	if (index >= points.size() || !std::isfinite(x) || !std::isfinite(y))
	{
		return;
	}

	points[index].y = clamp_range(y, 0.0f, 1.0f);

	if (index == 0 || index + 1 == points.size())
	{
		return;
	}

	const float lower = points[index - 1].x + min_gap;
	const float upper = points[index + 1].x - min_gap;
	if (lower <= upper)
	{
		points[index].x = clamp_range(x, lower, upper);
	}
}

void Curve::set_bend(const size_t index, const float bend)
{
	if (index + 1 >= points.size() || !std::isfinite(bend))
	{
		return;
	}
	points[index].bend = clamp_range(bend, -1.0f, 1.0f);
}

void Curve::segment_handle(const size_t index, float& x, float& y) const
{
	x = 0.0f;
	y = 0.0f;
	if (index + 1 >= points.size())
	{
		return;
	}

	const CurvePoint& a = points[index];
	const CurvePoint& b = points[index + 1];
	const float signed_bend = b.y >= a.y ? a.bend : -a.bend;
	const float exponent = std::pow(bend_exponent, std::fabs(signed_bend));

	// (position, 1 - position) in segment coordinates, where the value runs
	// from a.y (0) to b.y (1)
	float position = diagonal_crossing(exponent);
	if (signed_bend < 0.0f)
	{
		position = 1.0f - position;
	}
	x = a.x + (b.x - a.x) * position;
	y = a.y + (b.y - a.y) * (1.0f - position);
}

void Curve::set_segment_handle(const size_t index, const float x, const float y)
{
	if (index + 1 >= points.size() || !std::isfinite(x) || !std::isfinite(y))
	{
		return;
	}

	const CurvePoint& a = points[index];
	const CurvePoint& b = points[index + 1];
	const float width = b.x - a.x;
	const float span = b.y - a.y;
	if (width < min_gap || std::fabs(span) < 1e-4f)
	{
		return;
	}

	// project (x, y), in segment coordinates, onto the anti-diagonal
	const float segment_x = (x - a.x) / width;
	const float segment_y = (y - a.y) / span;
	float position = (segment_x + 1.0f - segment_y) * 0.5f;
	position = clamp_range(0.5f + (position - 0.5f) * handle_reach, 0.0f, 1.0f);

	// the curve through (m, 1 - m) has exponent log(m) / log(1 - m)
	const float nearest_end = std::max(std::min(position, 1.0f - position), 1e-6f);
	const float exponent = clamp_range(std::log(nearest_end)
										   / std::log(1.0f - nearest_end),
									   1.0f,
									   bend_exponent);
	const float strength = std::log(exponent) / std::log(bend_exponent);
	const float signed_bend = position < 0.5f ? strength : -strength;

	points[index].bend = clamp_range(span >= 0.0f ? signed_bend : -signed_bend,
									 -1.0f,
									 1.0f);
}

float Curve::sample(const float pos) const
{
	const float position = clamp_range(pos, 0.0f, 1.0f);

	size_t index = 0;
	while (index + 2 < points.size() && points[index + 1].x <= position)
	{
		index++;
	}

	const CurvePoint& a = points[index];
	const CurvePoint& b = points[index + 1];
	const float span = b.x - a.x;
	const float t =
		span > 1e-6f ? clamp_range((position - a.x) / span, 0.0f, 1.0f) : 1.0f;

	return segment_value(a.y, b.y, a.bend, t);
}

void Curve::bake(float* table) const
{
	for (size_t entry = 0; entry <= table_size; entry++)
	{
		table[entry] = sample(static_cast<float>(entry) / table_size);
	}
}

std::string Curve::encode() const
{
	const uint32_t count = points.size();
	const uint32_t sustain = sustain_index;

	std::string str;
	str += encode_base64(reinterpret_cast<const uint8_t*>(&count), sizeof(count));
	str += encode_base64(reinterpret_cast<const uint8_t*>(&sustain),
						 sizeof(sustain));
	str += encode_base64(reinterpret_cast<const uint8_t*>(points.data()),
						 points.size() * sizeof(CurvePoint));
	return str;
}

void Curve::decode(std::string_view& data)
{
	uint32_t count = 0;
	uint32_t sustain = 0;
	decode_base64(data, reinterpret_cast<uint8_t*>(&count), sizeof(count));
	decode_base64(data, reinterpret_cast<uint8_t*>(&sustain), sizeof(sustain));

	if (count < 2 || count > max_points)
	{
		*this = Curve();
		return;
	}

	points.resize(count);
	decode_base64(data,
				  reinterpret_cast<uint8_t*>(points.data()),
				  count * sizeof(CurvePoint));
	sustain_index = sustain;
	sanitize();
}

void Curve::sanitize()
{
	for (CurvePoint& p : points)
	{
		p.x = std::isfinite(p.x) ? p.x : 0.0f;
		p.y = std::isfinite(p.y) ? clamp_range(p.y, 0.0f, 1.0f) : 0.0f;
		p.bend = std::isfinite(p.bend) ? clamp_range(p.bend, -1.0f, 1.0f) : 0.0f;
	}

	points.front().x = 0.0f;
	points.back().x = 1.0f;
	for (size_t index = 1; index + 1 < points.size(); index++)
	{
		points[index].x = clamp_range(points[index].x, points[index - 1].x, 1.0f);
	}

	sustain_index = std::min(sustain_index, points.size() - 1);
}

} // namespace fmpire
