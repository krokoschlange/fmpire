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

inline bool starts_with(const std::string& string, const std::string& prefix)
{
	return (string.compare(0, prefix.size(), prefix) == 0);
}

std::string encode_base32(const uint8_t* const data, const size_t size);
size_t decode_base32(const std::string& encoded,
					 size_t offset,
					 uint8_t* decoded,
					 const size_t decoded_size);
void decode_base32(std::string_view& encoded,
				   uint8_t* decoded,
				   const size_t decoded_size);

template<typename T, size_t N, size_t i = N, typename T2, typename... Ts>
constexpr std::array<T, N> init_array(T2 t, Ts... ts)
{
	if constexpr (i <= 1)
	{
		return std::array<T, N>{t, ts...};
	}
	else
	{
		return init_array<T, N, i - 1>(t, t, ts...);
	}
}

template<typename T, size_t N, size_t i = N, typename T2, typename... Ts>
constexpr std::array<T, N> init_array_from_args_array(std::array<T2, N>& args,
													  Ts... ts)
{
	if constexpr (i <= 1)
	{
		return std::array<T, N>{T(args[0]), ts...};
	}
	else
	{
		return init_array_from_args_array<T, N, i - 1>(args,
													   T(args[i - 1]),
													   ts...);
	}
}

} // namespace fmpire

#endif // UTILS_H_INCLUDED
