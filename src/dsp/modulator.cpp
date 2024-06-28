#include "modulator.h"

#include "voice.h"

namespace fmpire
{

Modulator::Modulator()
{
}

Modulator::~Modulator() noexcept
{
}

float Modulator::sample()
{
	return 1;
}

float Modulator::get_release()
{
	return release;
}

ModulatorTarget::ModulatorTarget()
{
}

ModulatorTarget::~ModulatorTarget() noexcept
{
}

float* ModulatorTarget::resolve(const ModulatorTargetObjects& objects) const
{
	switch (target_type)
	{
	case TargetType::OSC_VOLUME:
		return &objects.osc_voices[target_object_id].volume;
	case TargetType::OSC_WT_POS:
		return &objects.osc_voices[target_object_id].wavetable_position;
	case TargetType::OSC_DETUNE:
		return &objects.osc_voices[target_object_id].detune;
	case TargetType::OSC_PAN:
		return &objects.osc_voices[target_object_id].pan;
	case TargetType::OSC_UNISON_DETUNE:
		return &objects.osc_voices[target_object_id].unison_detune;
	case TargetType::OSC_UNISON_SPREAD:
		return &objects.osc_voices[target_object_id].unison_spread;
	case TargetType::MOD_AMOUNT:
		return &objects.mod_voices[target_object_id].amount;
	case TargetType::MOD_FREQ:
		return &objects.mod_voices[target_object_id].frequency;
	default:
		return nullptr;
	}
}


} // namespace fmpire
