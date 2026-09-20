#ifndef PATCH_H_INCLUDED
#define PATCH_H_INCLUDED

#include "defines.h"
#include "modulator.h"
#include "oscillator.h"

#include <array>
#include <vector>

namespace fmpire
{

// Everything the audio thread reads to make sound, apart from the note and
// modulation playback state that lives in the voices: the oscillators, the
// modulators and the routes. It is built completely on a state thread and
// handed to the audio thread through a TripleBuffer, which only ever reads it.
struct Patch
{
	std::array<Oscillator, FMPIRE_OSC_COUNT> oscillators;

	// indexed by modulator id / route slot; unused ids and slots are disabled
	// modulators / inactive routes
	std::vector<Modulator> modulators;
	std::vector<ModRoute> routes;
};

} // namespace fmpire

#endif // PATCH_H_INCLUDED
