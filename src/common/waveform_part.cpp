#include "waveform_part.h"

#include "utils.h"

#include "exprtk.hpp"
#include "FftRealPair.hpp"

#define SAMPLES_STRING  std::string("SAMPLES")
#define FUNCTION_STRING std::string("FUNCTION")
#define HARMONIC_STRING std::string("HARMONIC")

namespace fmpire
{

std::shared_ptr<WaveformPart> WaveformPart::create(Type type)
{
	switch (type)
	{
	case Type::SAMPLES:
		return std::make_shared<SamplesWaveformPart>();
	case Type::FUNCTION:
		return std::make_shared<FunctionWaveformPart>();
	case Type::HARMONIC:
		return std::make_shared<HarmonicsWaveformPart>();
	}
	return std::shared_ptr<WaveformPart>();
}

std::shared_ptr<WaveformPart> WaveformPart::create(std::string_view& data)
{
	std::shared_ptr<WaveformPart> part;
	if (data.starts_with(SAMPLES_STRING))
	{
		part = create(Type::SAMPLES);
	}
	else if (data.starts_with(FUNCTION_STRING))
	{
		part = create(Type::FUNCTION);
	}
	else if (data.starts_with(HARMONIC_STRING))
	{
		part = create(Type::HARMONIC);
	}
	else
	{
		return part;
	}
	part->decode(data);
	return part;
}

WaveformPart::WaveformPart(const Type part_type) :
	type(part_type),
	waveform_size(0),
	start(0),
	end(1),
	waveform_index(0)
{
}

WaveformPart::~WaveformPart() noexcept
{
}

WaveformPart::Type WaveformPart::get_type() const
{
	return type;
}

std::string WaveformPart::encode_generic_part_parameters() const
{
	std::string str =
		encode_base32(reinterpret_cast<const uint8_t*>(&waveform_size),
					  sizeof(waveform_size));
	str +=
		encode_base32(reinterpret_cast<const uint8_t*>(&start), sizeof(start));
	str += encode_base32(reinterpret_cast<const uint8_t*>(&end), sizeof(end));
	str += encode_base32(reinterpret_cast<const uint8_t*>(&waveform_index),
						 sizeof(waveform_index));
	return str;
}

void WaveformPart::decode_generic_part_parameters(std::string_view& data)
{
	decode_base32(data,
				  reinterpret_cast<uint8_t*>(&waveform_size),
				  sizeof(waveform_size));
	decode_base32(data, reinterpret_cast<uint8_t*>(&start), sizeof(start));
	decode_base32(data, reinterpret_cast<uint8_t*>(&end), sizeof(end));
	decode_base32(data,
				  reinterpret_cast<uint8_t*>(&waveform_index),
				  sizeof(waveform_index));
}

SamplesWaveformPart::SamplesWaveformPart() :
	WaveformPart(Type::SAMPLES)
{
}

SamplesWaveformPart::~SamplesWaveformPart() noexcept
{
}

float SamplesWaveformPart::sample(size_t position) const
{
	if (position < start || position >= end)
	{
		return 0;
	}
	size_t idx = std::min(position - start, samples.size());
	return samples[idx];
}

std::string SamplesWaveformPart::encode() const
{
	std::string str = SAMPLES_STRING;
	str += encode_generic_part_parameters();
	uint32_t sample_count = samples.size();
	str += encode_base32(reinterpret_cast<const uint8_t*>(&sample_count),
						 sizeof(sample_count));
	str += encode_base32(reinterpret_cast<const uint8_t*>(samples.data()),
						 samples.size() * sizeof(float));
	return str;
}

void SamplesWaveformPart::decode(std::string_view& data)
{
	if (!data.starts_with(SAMPLES_STRING))
	{
		return;
	}
	data.remove_prefix(SAMPLES_STRING.size());
	decode_generic_part_parameters(data);
	uint32_t sample_count;
	decode_base32(data,
				  reinterpret_cast<uint8_t*>(&sample_count),
				  sizeof(sample_count));
	samples.resize(sample_count);
	decode_base32(data,
				  reinterpret_cast<uint8_t*>(samples.data()),
				  sample_count * sizeof(float));
}

FunctionWaveformPart::FunctionWaveformPart() :
	WaveformPart(Type::FUNCTION),
	symbol_table(new exprtk::symbol_table<float>()),
	expression(new exprtk::expression<float>()),
	parser(new exprtk::parser<float>())
{
	symbol_table->add_constants();
	symbol_table->add_variable("x", variable_x);
	symbol_table->add_variable("y", variable_y);

	expression->register_symbol_table(*symbol_table);
}

FunctionWaveformPart::~FunctionWaveformPart() noexcept
{
	delete symbol_table;
	delete expression;
	delete parser;
}

float FunctionWaveformPart::sample(size_t position) const
{
	if (position < start || position >= end)
	{
		return 0;
	}
	variable_x = position / (waveform_size - 1);
	variable_y = waveform_index;
	return expression->value();
}

std::string FunctionWaveformPart::encode() const
{
	std::string str = FUNCTION_STRING;
	str += encode_generic_part_parameters();
	uint32_t str_len = function.size();
	str += encode_base32(reinterpret_cast<const uint8_t*>(&str_len),
						 sizeof(str_len));
	str += function;
	return str;
}

void FunctionWaveformPart::decode(std::string_view& data)
{
	if (!data.starts_with(FUNCTION_STRING))
	{
		return;
	}
	data.remove_prefix(FUNCTION_STRING.size());
	decode_generic_part_parameters(data);
	uint32_t str_len = 0;
	decode_base32(data, reinterpret_cast<uint8_t*>(&str_len), sizeof(str_len));
	function = data.substr(0, str_len);

	update();
}

void FunctionWaveformPart::update()
{
	bool ok = parser->compile(function, *expression);
	if (!ok)
	{
		parser->compile("0", *expression);
	}
}

HarmonicsWaveformPart::HarmonicsWaveformPart() :
	WaveformPart(Type::HARMONIC)
{
}

HarmonicsWaveformPart::~HarmonicsWaveformPart() noexcept
{
}

float HarmonicsWaveformPart::sample(size_t position) const
{
	if (position < start || position >= end)
	{
		return 0;
	}
	size_t idx = std::min(position, samples.size());
	return samples[idx];
}

std::string HarmonicsWaveformPart::encode() const
{
	std::string str = HARMONIC_STRING;
	str += encode_generic_part_parameters();
	str += encode_base32(reinterpret_cast<const uint8_t*>(&type), sizeof(type));
	uint32_t harmonic_count = harmonics.size();
	str += encode_base32(reinterpret_cast<const uint8_t*>(&harmonic_count),
						 sizeof(harmonic_count));
	str += encode_base32(reinterpret_cast<const uint8_t*>(&harmonics),
						 harmonics.size() * sizeof(Harmonic));
	return str;
}

void HarmonicsWaveformPart::decode(std::string_view& data)
{
	if (!data.starts_with(HARMONIC_STRING))
	{
		return;
	}
	data.remove_prefix(HARMONIC_STRING.size());
	decode_generic_part_parameters(data);
	decode_base32(data, reinterpret_cast<uint8_t*>(type), sizeof(type));
	uint32_t harmonic_count = 0;
	decode_base32(data,
				  reinterpret_cast<uint8_t*>(harmonic_count),
				  sizeof(harmonic_count));
	harmonics.resize(harmonic_count);
	decode_base32(data,
				  reinterpret_cast<uint8_t*>(harmonics.data()),
				  harmonic_count * sizeof(Harmonic));

	update();
}

void create_tri_wave(float data[], size_t size)
{
	for (size_t smpl = 0; smpl < size; smpl++)
	{
		float phase = (float) smpl / size;
		if (phase < 0.25)
		{
			data[smpl] = 4 * phase;
		}
		else if (phase < 0.75)
		{
			data[smpl] = -4 * phase + 2;
		}
		else
		{
			data[smpl] = 4 * phase - 4;
		}
	}
}

void create_saw_wave(float data[], size_t size)
{
	for (size_t smpl = 0; smpl < size; smpl++)
	{
		float phase = (float) smpl / (size - 1);
		data[smpl] = 2 * phase - 1;
	}
}

void create_sqr_wave(float data[], size_t size)
{
	for (size_t smpl = 0; smpl < size; smpl++)
	{
		float phase = (float) smpl / size;
		if (phase < 0.5)
		{
			data[smpl] = -1;
		}
		else
		{
			data[smpl] = 1;
		}
	}
}

void HarmonicsWaveformPart::update()
{
	samples.resize(waveform_size, 0);
	if (type == HarmonicType::SIN)
	{
		std::vector<double> real(waveform_size, 0);
		std::vector<double> imag(waveform_size, 0);

		for (size_t harm = 0; harm < harmonics.size(); harm++)
		{
			real[harm] = -sin(harmonics[harm].phase * 2 * M_PI)
					   * harmonics[harm].amplitude;
			imag[harm] = -cos(harmonics[harm].phase * 2 * M_PI)
					   * harmonics[harm].amplitude;
		}

		Fft::inverseTransform(real, imag);

		samples.assign(real.begin(), real.end());
	}
	else
	{
		float base_wave[waveform_size];
		switch (type)
		{
		case HarmonicType::TRI:
			create_tri_wave(base_wave, waveform_size);
			break;
		case HarmonicType::SAW:
			create_saw_wave(base_wave, waveform_size);
			break;
		case HarmonicType::SQR:
			create_sqr_wave(base_wave, waveform_size);
			break;
		default:
			break;
		}
		for (size_t smpl = 0; smpl < waveform_size; smpl++)
		{
			float phase = (float) smpl / waveform_size;
			float value = 0;
			for (size_t harm = 0; harm < harmonics.size(); harm++)
			{
				float harm_phase = wrap(1, phase + harmonics[harm].phase);
				size_t wave_pos =
					(size_t) (harm_phase * waveform_size) % waveform_size;
				value += base_wave[wave_pos] * harmonics[harm].amplitude;
			}
			samples[smpl] = value;
		}
	}
}


} // namespace fmpire
