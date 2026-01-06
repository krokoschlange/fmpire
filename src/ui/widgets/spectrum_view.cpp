#include "spectrum_view.h"
#include "draw_operations.h"
#include "fmpire_widget.h"

#include "waveform.h"

#include "FftRealPair.hpp"
#include <cmath>

namespace fmpire
{

SpectrumView::SpectrumView(Widget* parent) :
	FMpireWidget(parent)
{
}

SpectrumView::~SpectrumView() noexcept
{
}

void SpectrumView::set_waveform(Waveform* wf)
{
	waveform = wf;

	std::vector<float> samples = waveform->sample_all();

	std::vector<double> real(samples.begin(), samples.end());
	std::vector<double> imag(waveform->get_width(), 0.0);
	Fft::transform(real, imag);

	amplitude.resize(real.size());
	phase.resize(imag.size());
	float max_amp = 0.0f;

	for (uint32_t i = 0; i < real.size(); i++)
	{
		float amp =
			sqrt(real[i] * real[i] + imag[i] * imag[i]) / real.size() * 2;
		max_amp = std::max(amp, max_amp);

		amplitude[i] = amp;
		phase[i] = (atan2(-real[i], -imag[i]) / M_PI) * 0.5 + 0.5;
	}

	for (uint32_t i = 0; i < amplitude.size(); i++)
	{
		if (max_amp > 0.0)
		{
			amplitude[i] /= max_amp;
		}
		if (amplitude[i] < 0.001)
		{
			phase[i] = 0;
		}
	}
}

void SpectrumView::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	theme->background.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 theme->line_very_thin);

	clip_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 0);

	Line<float> line(0, getHeight() * 0.5, getWidth(), getHeight() * 0.5);
	line.draw(context, theme->line_very_thin);

	uint32_t box_count = std::min<uint32_t>(amplitude.size(), 128);
	float box_width = (float) getWidth() / box_count;

	for (uint32_t i = 0; i < box_count; i++)
	{
		theme->highlight.setFor(context);
		Rectangle<float> rect(i * box_width,
							  getHeight() * 0.5 * (1 - amplitude[i]),
							  box_width,
							  getHeight() * 0.5 * amplitude[i]);

		if (amplitude[i] > 0.0f)
		{
			rect.draw(context);
		}

		if (phase[i] > 0.0f)
		{
			rect = Rectangle<float>(i * box_width,
									getHeight() * (1 - 0.5 * phase[i]),
									box_width,
									getHeight() * 0.5 * phase[i]);
			rect.draw(context);
		}
	}
}

} // namespace fmpire
