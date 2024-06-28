#ifndef ENUM_OPS_H_INCLUDED
#define ENUM_OPS_H_INCLUDED


#include <type_traits>

namespace fmpire
{

template<typename T, bool = std::is_enum<T>::value> struct is_bitflag;

template<typename T> struct is_bitflag<T, true> : std::false_type
{
};

template<typename T,
		 typename std::enable_if<is_bitflag<T>::value>::type* = nullptr>
T operator|(const T& a, const T& b)
{
	using type = typename std::underlying_type<T>::type;
	return static_cast<T>(static_cast<type>(a) | static_cast<type>(b));
}

template<typename T,
		 typename std::enable_if<is_bitflag<T>::value>::type* = nullptr>
T& operator|=(T& a, const T& b)
{
	using type = typename std::underlying_type<T>::type;
	a = static_cast<T>(static_cast<type>(a) | static_cast<type>(b));
	return a;
}

template<typename T,
		 typename std::enable_if<is_bitflag<T>::value>::type* = nullptr>
T operator&(const T& a, const T& b)
{
	using type = typename std::underlying_type<T>::type;
	return static_cast<T>(static_cast<type>(a) & static_cast<type>(b));
}

template<typename T,
		 typename std::enable_if<is_bitflag<T>::value>::type* = nullptr>
T& operator&=(T& a, const T& b)
{
	using type = typename std::underlying_type<T>::type;
	a = static_cast<T>(static_cast<type>(a) & static_cast<type>(b));
	return a;
}

template<typename T,
		 typename std::enable_if<is_bitflag<T>::value>::type* = nullptr>
bool any(const T& a)
{
	using type = typename std::underlying_type<T>::type;
	return static_cast<type>(a);
}


} // namespace fmpire

#endif // ENUM_OPS_H_INCLUDED
