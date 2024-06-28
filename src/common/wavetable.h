#ifndef WAVETABLE_H_INCLUDED
#define WAVETABLE_H_INCLUDED

#include "extra/ScopedPointer.hpp"

#include <memory>
#include <vector>

namespace fmpire
{

class Wavetable
{
public:
	Wavetable();
	virtual ~Wavetable() noexcept;

	float sample(const float position,
				 const float phase,
				 const bool interp_waves = true,
				 const bool interp_phase = false) const;

	void get_size(size_t& w, size_t& h) const;

private:
	ScopedPointer<std::vector<float>> data;
	size_t width;
	size_t height;

	void get_sample_position(const float position,
							 const float phase,
							 size_t& pos_smpl,
							 size_t& phase_smpl) const;
	float get_position_interpolation_factor(const float position) const;
	float get_phase_interpolation_factor(const float phase) const;
	float read(const size_t position, const size_t phase) const;
};


} // namespace fmpire

#endif // WAVETABLE_H_INCLUDED
