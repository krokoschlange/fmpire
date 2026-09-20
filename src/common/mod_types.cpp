#include "mod_types.h"

#include "utils.h"

#include <algorithm>
#include <cmath>

namespace fmpire
{

namespace
{

template<typename T> void append(std::string& out, const T value)
{
	out += encode_base64(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
}

template<typename T> T read(std::string_view& in)
{
	T value{};
	decode_base64(in, reinterpret_cast<uint8_t*>(&value), sizeof(T));
	return value;
}

float finite_clamped(const float value,
					 const float min,
					 const float max,
					 const float fallback)
{
	return std::isfinite(value) ? std::clamp(value, min, max) : fallback;
}

} // namespace

std::string ModulatorSettings::encode() const
{
	std::string out;
	append<uint8_t>(out, static_cast<uint8_t>(type));
	append<uint8_t>(out, use_beats ? 1 : 0);
	append<float>(out, length_seconds);
	append<float>(out, length_beats);
	append<float>(out, amount);
	append<float>(out, phase_offset);
	return out;
}

void ModulatorSettings::decode(std::string_view& data)
{
	const uint8_t raw_type = read<uint8_t>(data);
	type = raw_type == static_cast<uint8_t>(ModulatorType::LFO)
			 ? ModulatorType::LFO
			 : ModulatorType::ENVELOPE;
	use_beats = read<uint8_t>(data) != 0;
	length_seconds = finite_clamped(read<float>(data), 0.001f, 3600.0f, 1.0f);
	length_beats = finite_clamped(read<float>(data), 0.001f, 4096.0f, 1.0f);
	amount = finite_clamped(read<float>(data), 0.0f, 1.0f, 1.0f);
	phase_offset = finite_clamped(read<float>(data), 0.0f, 1.0f, 0.0f);
}

std::string RouteSettings::encode() const
{
	std::string out;
	append<uint32_t>(out, slot);
	append<uint32_t>(out, source.pack());
	append<uint8_t>(out, static_cast<uint8_t>(target));
	append<uint16_t>(out, target_object);
	append<float>(out, amount);
	append<uint8_t>(out, bipolar ? 1 : 0);
	return out;
}

void RouteSettings::decode(std::string_view& data)
{
	slot = read<uint32_t>(data);
	source = SourceId::unpack(read<uint32_t>(data));
	const uint8_t raw_target = read<uint8_t>(data);
	target = raw_target < static_cast<uint8_t>(TargetType::COUNT)
			   ? static_cast<TargetType>(raw_target)
			   : TargetType::OSC_VOLUME;
	target_object = read<uint16_t>(data);
	amount = finite_clamped(read<float>(data), -1.0f, 1.0f, 0.0f);
	bipolar = read<uint8_t>(data) != 0;
}

} // namespace fmpire
