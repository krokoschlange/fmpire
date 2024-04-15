#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <cmath>
#include <cstdint>
#include <string>

namespace fmpire
{

inline float lerp(const float a, const float b, const float t)
{
	return a + (b - a) * t;
}

inline double lerp(const double a, const double b, const double t)
{
	return a + (b - a) * t;
}

inline float wrap(const float max, const float value)
{
	return std::fmod(max + std::fmod(value, max), max);
}

inline float wrap(const float min, const float max, const float value)
{
	return min + wrap(value - min, max - min);
}

std::string encode_base32(const uint8_t* const data, const size_t size);
size_t decode_base32(const std::string& encoded,
					 const size_t offset,
					 uint8_t* decoded,
					 const size_t decoded_size);

} // namespace fmpire

#endif // UTILS_H_INCLUDED
