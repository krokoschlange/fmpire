#ifndef CURVE_H_INCLUDED
#define CURVE_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace fmpire
{

struct CurvePoint
{
	float x; // normalized time, 0..1
	float y; // normalized value, 0..1
	// shape of the segment leaving this point, -1..1: 0 is linear, 1 a sharp
	// corner bulging up, -1 a sharp corner bulging down
	float bend;
};

// A user-drawn curve of breakpoints with a bend per segment. Used for both
// envelopes (played once, holding at the sustain point) and LFOs (looped).
// Shared by the UI editor and the DSP.
class Curve
{
public:
	static constexpr size_t table_size = 1024;
	static constexpr size_t max_points = 256;
	static constexpr size_t npos = static_cast<size_t>(-1);

	// Exponent of the power curve of a segment with the strongest bend. The
	// corner gets as sharp as 1/bend_exponent of the segment (the baked table
	// can't resolve much more than that anyway).
	static constexpr float bend_exponent = 1000.0f;

	Curve();

	static Curve make_envelope();
	static Curve make_lfo();

	size_t size() const { return points.size(); }

	const CurvePoint& point(const size_t index) const { return points[index]; }

	size_t get_sustain_index() const { return sustain_index; }

	void set_sustain_index(const size_t index);

	// Inserts a point ordered by x and returns its index, or npos when full.
	size_t add_point(const float x, const float y);

	// The first and last point cannot be removed.
	void remove_point(const size_t index);

	// The first and last point keep their x; other points stay between their
	// neighbours so indices never change while dragging.
	void move_point(const size_t index, const float x, const float y);

	void set_bend(const size_t index, const float bend);

	// Position of the bend handle of segment `index`: the point where the
	// segment crosses the diagonal of its bounding box that runs against the
	// segment's direction. The handle stays on the curve and moves along that
	// diagonal as the bend changes: from the centre (no bend) to the corner
	// where a sharp bend has its corner.
	void segment_handle(const size_t index, float& x, float& y) const;

	// Sets the bend of segment `index` so that its handle is as close as
	// possible to (x, y): the position is projected onto the diagonal.
	void set_segment_handle(const size_t index, const float x, const float y);

	// pos is clamped to 0..1.
	float sample(const float pos) const;

	// Fills table_size + 1 entries (the last one is the value at pos = 1).
	void bake(float* table) const;

	// Linearly interpolated lookup into a baked table, pos clamped to 0..1.
	static inline float lookup(const float* table, const float pos)
	{
		const float p = pos <= 0.0f ? 0.0f : (pos >= 1.0f ? 1.0f : pos);
		const float scaled = p * static_cast<float>(table_size);
		size_t index = static_cast<size_t>(scaled);
		if (index >= table_size)
		{
			return table[table_size];
		}
		const float fraction = scaled - static_cast<float>(index);
		return table[index] + (table[index + 1] - table[index]) * fraction;
	}

	std::string encode() const;
	void decode(std::string_view& data);

private:
	std::vector<CurvePoint> points;
	size_t sustain_index;

	void sanitize();
};

} // namespace fmpire

#endif // CURVE_H_INCLUDED
