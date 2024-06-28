#ifndef WAVEFORM_PART_H_INCLUDED
#define WAVEFORM_PART_H_INCLUDED

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace fmpire
{

class WaveformPart
{
public:
	enum class Type
	{
		SAMPLES,
		FUNCTION,
		HARMONIC,
	};

	static std::shared_ptr<WaveformPart> create(const Type type);
	static std::shared_ptr<WaveformPart> create(std::string_view& data);
	virtual ~WaveformPart() noexcept;

	inline bool contains(size_t position) const
	{
		return (position >= start && position < end);
	}

	Type get_type() const;

	virtual float sample(size_t position) const = 0;

	virtual std::string encode() const = 0;
	virtual void decode(std::string_view& data) = 0;

protected:
	WaveformPart(const Type part_type);

	const Type type;
	uint32_t waveform_size;
	uint32_t start;
	uint32_t end;
	uint32_t waveform_index;

	std::string encode_generic_part_parameters() const;
	void decode_generic_part_parameters(std::string_view& data);
};

class SamplesWaveformPart : public WaveformPart
{
public:
	SamplesWaveformPart();
	virtual ~SamplesWaveformPart() noexcept;

	virtual float sample(size_t position) const override;

	virtual std::string encode() const override;
	virtual void decode(std::string_view& data) override;

	inline std::vector<float>& get_samples() { return samples; }

protected:
	std::vector<float> samples;
};

} // namespace fmpire

namespace exprtk
{
template<typename T> class symbol_table;
template<typename T> class expression;
template<typename T> class parser;
} // namespace exprtk

namespace fmpire
{

class FunctionWaveformPart : public WaveformPart
{
public:
	FunctionWaveformPart();
	virtual ~FunctionWaveformPart() noexcept;

	virtual float sample(size_t position) const override;

	virtual std::string encode() const override;
	virtual void decode(std::string_view& data) override;

	inline void set_function(std::string func)
	{
		function = func;
		update();
	}

	inline std::string get_function() const { return function; }

protected:
	std::string function;

	exprtk::symbol_table<float>* symbol_table;
	exprtk::expression<float>* expression;
	exprtk::parser<float>* parser;
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
	virtual void decode(std::string_view& data) override;

	enum class HarmonicType
	{
		SIN = 0,
		TRI = 1,
		SAW = 2,
		SQR = 3,
	};

	struct Harmonic
	{
		float phase;
		float amplitude;
	};

	inline std::vector<Harmonic>& get_harmonics() { return harmonics; }

	void update();

protected:
	std::vector<Harmonic> harmonics;
	HarmonicType type;

	std::vector<float> samples;
};

} // namespace fmpire

#endif // WAVEFORM_PART_H_INCLUDED
