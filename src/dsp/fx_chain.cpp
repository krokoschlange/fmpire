#include "fx_chain.h"

namespace fmpire
{

FXChain::FXChain()
{
}

FXChain::~FXChain() noexcept
{
}

void FXChain::run(float** inout, size_t size)
{
	for (size_t effect_idx = 0; effect_idx < effects.size(); effect_idx++)
	{
	}
}


} // namespace fmpire
