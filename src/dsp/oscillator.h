#ifndef OSCILLATOR_H_INCLUDED
#define OSCILLATOR_H_INCLUDED

#include "wavetable.h"

namespace fmpire
{

class Oscillator
{
public:
	Oscillator();
	virtual ~Oscillator() noexcept;

	void set_wavetable(Wavetable* wavetable);

private:
	Wavetable* wavetable;
};


} // namespace fmpire

#endif // OSCILLATOR_H_INCLUDED
