#ifndef WAVEFORM_PART_H_INCLUDED
#define WAVEFORM_PART_H_INCLUDED

#include "exprtk.hpp"

#include <string>
#include <vector>

namespace fmpire
{

class WaveformPart
{
public:
	WaveformPart();
	virtual ~WaveformPart() noexcept;

	inline bool contains(size_t position) const;
	virtual float sample(size_t position) const = 0;

	virtual std::string encode() const = 0;
	virtual void decode(const std::string& data) = 0;

protected:
	uint32_t waveform_size;
	uint32_t start;
	uint32_t end;
	uint32_t waveform_index;

	std::string encode_generic_part_parameters() const;
	size_t decode_generic_part_parameters(const std::string& data,
										  const size_t byte_offset);
};

class SamplesWaveformPart : public WaveformPart
{
public:
	SamplesWaveformPart();
	virtual ~SamplesWaveformPart() noexcept;

	virtual float sample(size_t position) const override;

	virtual std::string encode() const override;
	virtual void decode(const std::string& data) override;

protected:
	std::vector<float> samples;
};

class FunctionWaveformPart : public WaveformPart
{
public:
	FunctionWaveformPart();
	virtual ~FunctionWaveformPart() noexcept;

	virtual float sample(size_t position) const override;

	virtual std::string encode() const override;
	virtual void decode(const std::string& data) override;

protected:
	std::string function;

	exprtk::symbol_table<float> symbol_table;
	exprtk::expression<float> expression;
	exprtk::parser<float> parser;
	mutable float variable_x;
	mutable float variable_y;

	void update();
};

class HarmonicsWaveformPart : public WaveformPart
{
public:
	HarmonicsWaveformPart();
	virtual ~HarmonicsWaveformPart() noexcept;

	virtual float sample(size_t position) const override;

	virtual std::string encode() const override;
	virtual void decode(const std::string& data) override;

	enum Type
	{
		SIN = 0,
		TRI = 1,
		SAW = 2,
		SQR = 3,
	};

protected:
	struct Harmonic
	{
		float phase;
		float amplitude;
	};

	std::vector<Harmonic> harmonics;
	Type type;

	std::vector<float> samples;

	void update();
};

} // namespace fmpire

#endif // WAVEFORM_PART_H_INCLUDED
