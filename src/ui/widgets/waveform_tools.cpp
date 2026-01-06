#include "waveform_tools.h"
#include "utils.h"
#include "waveform.h"
#include "waveform_part.h"
#include <cmath>
#include <string>

namespace fmpire
{

WaveformTool::WaveformTool(Waveform* wf) :
	waveform(wf)
{
}

WaveformTool::~WaveformTool()
{
}

FreeDrawTool::FreeDrawTool(Waveform* wf, SamplesWaveformPart* existing_part) :
	WaveformTool(wf),
	part(existing_part)
{
}

FreeDrawTool::~FreeDrawTool()
{
}

void FreeDrawTool::start(const WaveformToolStepInfo& info)
{
	if (part == nullptr)
	{
		part = new SamplesWaveformPart();

		part->get_samples() = {info.snapped_y * 2 - 1};
		part->set_start(info.snapped_x * waveform->get_width());
		part->set_end(part->get_start() + 1);

		waveform->insert_part(part);
	}

	last_info = info;
}

void FreeDrawTool::step(const WaveformToolStepInfo& info)
{
	uint32_t draw_start = last_info.snapped_x * waveform->get_width();
	uint32_t draw_end = info.snapped_x * waveform->get_width();

	float y_start = last_info.snapped_y * 2 - 1;
	float y_end = info.snapped_y * 2 - 1;

	if (draw_start > draw_end)
	{
		uint32_t tmp = draw_start;
		draw_start = draw_end;
		draw_end = tmp;

		float tmpf = y_start;
		y_start = y_end;
		y_end = tmpf;
	}

	uint32_t start = std::min(draw_start, part->get_start());
	uint32_t end = std::max(draw_end, part->get_end());

	std::vector<float> new_samples(end - start);
	std::copy(part->get_samples().begin(),
			  part->get_samples().end(),
			  new_samples.begin() + (part->get_start() - start));

	for (size_t i = 0; i < draw_end - draw_start; i++)
	{
		new_samples[i + draw_start - start] =
			lerp(y_start, y_end, (float) i / (draw_end - draw_start));
	}

	part->get_samples() = new_samples;
	part->set_start(start);
	part->set_end(end);

	last_info = info;
}

void FreeDrawTool::end()
{
}

FunctionDrawTool::FunctionDrawTool(Waveform* wf) :
	WaveformTool(wf),
	part(nullptr)
{
}

FunctionDrawTool::~FunctionDrawTool()
{
}

void FunctionDrawTool::start(const WaveformToolStepInfo& info)
{
	start_x = info.snapped_x;
	start_y = info.snapped_y * 2 - 1;

	part = new FunctionWaveformPart();
	part->set_start(start_x * waveform->get_width());
	part->set_end(part->get_start() + 1);
	waveform->insert_part(part);
}

void FunctionDrawTool::step(const WaveformToolStepInfo& info)
{
	part->set_function(create_function(info.snapped_x, info.snapped_y * 2 - 1));

	part->set_start(std::min(info.snapped_x, start_x) * waveform->get_width());
	part->set_end(std::max(info.snapped_x, start_x) * waveform->get_width());
}

void FunctionDrawTool::end()
{
}

std::string LineDrawTool::create_function(float end_x, float end_y)
{
	float x1 = start_x < end_x ? start_x : end_x;
	float x2 = start_x < end_x ? end_x : start_x;
	float y1 = start_x < end_x ? start_y : end_y;
	float y2 = start_x < end_x ? end_y : start_y;

	float m = (y2 - y1) / (x2 - x1);
	float c = y1 - x1 * m;

	return std::to_string(m) + " * x + " + std::to_string(c);
}

std::string HalfSineTool::create_function(float end_x, float end_y)
{
	float amp = end_y - start_y;
	float freq = M_PI / (end_x - start_x);
	return std::to_string(amp) + "sin((x-" + std::to_string(start_x) + ")*"
		 + std::to_string(freq) + ")+" + std::to_string(start_y);
}

std::string SineSlopeTool::create_function(float end_x, float end_y)
{
	float amp = (end_y - start_y) * 0.5f;
	float freq = M_PI / (end_x - start_x);
	return std::to_string(amp) + "sin((x-" + std::to_string(start_x) + ")*"
		 + std::to_string(freq) + "-0.5*pi)+" + std::to_string(start_y) + "+"
		 + std::to_string((end_y - start_y) * 0.5f);
}

} // namespace fmpire
