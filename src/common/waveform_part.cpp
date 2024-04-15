#include "waveform_part.h"

#include "utils.h"

#include "FftRealPair.hpp"

#define SAMPLES_STRING  std::string("SAMPLES")
#define FUNCTION_STRING std::string("FUNCTION")
#define HARMONIC_STRING std::string("HARMONIC")

namespace fmpire
{

WaveformPart::WaveformPart() :
	waveform_size(0),
	start(0),
	end(1),
	waveform_index(0)
{
}

WaveformPart::~WaveformPart() noexcept
{
}

bool WaveformPart::contains(size_t position) const
{
	return (position >= start && position < end);
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

size_t WaveformPart::decode_generic_part_parameters(const std::string& data,
													const size_t byte_offset)
{
	size_t offset = byte_offset;
	offset += decode_base32(data,
							offset,
							reinterpret_cast<uint8_t*>(&waveform_size),
							sizeof(waveform_size));
	offset += decode_base32(data,
							offset,
							reinterpret_cast<uint8_t*>(&start),
							sizeof(start));
	offset += decode_base32(data,
							offset,
							reinterpret_cast<uint8_t*>(&end),
							sizeof(end));
	offset += decode_base32(data,
							offset,
							reinterpret_cast<uint8_t*>(&waveform_index),
							sizeof(waveform_index));
	return offset;
}

SamplesWaveformPart::SamplesWaveformPart() :
	WaveformPart()
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

void SamplesWaveformPart::decode(const std::string& data)
{
	size_t offset = SAMPLES_STRING.size();
	if (data.substr(0, offset) != SAMPLES_STRING)
	{
		return;
	}
	offset = decode_generic_part_parameters(data, offset);
	uint32_t sample_count;
	offset += decode_base32(data,
							offset,
							reinterpret_cast<uint8_t*>(&sample_count),
							sizeof(sample_count));
	samples.resize(sample_count);
	offset += decode_base32(data,
							offset,
							reinterpret_cast<uint8_t*>(samples.data()),
							sample_count * sizeof(float));
}

FunctionWaveformPart::FunctionWaveformPart() :
	WaveformPart()
{
	symbol_table.add_constants();
	symbol_table.add_variable("x", variable_x);
	symbol_table.add_variable("y", variable_y);

	expression.register_symbol_table(symbol_table);
}

FunctionWaveformPart::~FunctionWaveformPart() noexcept
{
}

float FunctionWaveformPart::sample(size_t position) const
{
	if (position < start || position >= end)
	{
		return 0;
	}
	variable_x = position / (waveform_size - 1);
	variable_y = waveform_index;
	return expression.value();
}

std::string FunctionWaveformPart::encode() const
{
	std::string str = FUNCTION_STRING;
	str += encode_generic_part_parameters();
	str += function;
	return str;
}

void FunctionWaveformPart::decode(const std::string& data)
{
	size_t offset = FUNCTION_STRING.size();
	if (data.substr(0, offset) != FUNCTION_STRING)
	{
		return;
	}
	offset = decode_generic_part_parameters(data, offset);
	function = data.substr(offset);

	update();
}

void FunctionWaveformPart::update()
{
	bool ok = parser.compile(function, expression);
	if (!ok)
	{
		parser.compile("0", expression);
	}
}

HarmonicsWaveformPart::HarmonicsWaveformPart() :
	WaveformPart()
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
	size_t harmonic_count = harmonics.size();
	str += encode_base32(reinterpret_cast<const uint8_t*>(&harmonic_count),
						 sizeof(harmonic_count));
	str += encode_base32(reinterpret_cast<const uint8_t*>(&harmonics),
						 harmonics.size() * sizeof(Harmonic));
	return str;
}

void HarmonicsWaveformPart::decode(const std::string& data)
{
	size_t offset = HARMONIC_STRING.size();
	if (data.substr(0, offset) != HARMONIC_STRING)
	{
		return;
	}
	offset = decode_generic_part_parameters(data, offset);
	offset += decode_base32(data,
							offset,
							reinterpret_cast<uint8_t*>(type),
							sizeof(type));
	size_t harmonic_count = 0;
	offset += decode_base32(data,
							offset,
							reinterpret_cast<uint8_t*>(harmonic_count),
							sizeof(harmonic_count));
	harmonics.resize(harmonic_count);
	offset += decode_base32(data,
							offset,
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
	if (type == SIN)
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
		case TRI:
			create_tri_wave(base_wave, waveform_size);
			break;
		case SAW:
			create_saw_wave(base_wave, waveform_size);
			break;
		case SQR:
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
