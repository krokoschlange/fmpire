#ifndef MOD_SOURCE_H_INCLUDED
#define MOD_SOURCE_H_INCLUDED

#include "defines.h"
#include "mod_types.h"

#include <array>
#include <atomic>
#include <cstdint>

namespace fmpire
{

struct GlobalSources
{
	GlobalSources() :
		pitch_bend(0.5f),
		channel_pressure(0.0f)
	{
		for (std::atomic<float>& macro : macros)
		{
			macro.store(0.0f, std::memory_order_relaxed);
		}
		cc.fill(0.0f);
	}

	std::array<std::atomic<float>, FMPIRE_MACRO_COUNT> macros;

	std::array<float, 128> cc;
	float pitch_bend;
	float channel_pressure;
};

struct VoiceSources
{
	float velocity = 0.0f;
	float key = 0.0f; // note / 127
	float poly_pressure = 0.0f;
};

} // namespace fmpire

#endif // MOD_SOURCE_H_INCLUDED
