#ifndef FX_CHAIN_H_INCLUDED
#define FX_CHAIN_H_INCLUDED

#include <vector>

namespace fmpire
{

class Effect;

class FXChain
{
public:
	FXChain();
	virtual ~FXChain() noexcept;

	void run(float** inout, size_t size);

private:
	std::vector<Effect*> effects;
};

} // namespace fmpire

#endif // FX_CHAIN_H_INCLUDED
