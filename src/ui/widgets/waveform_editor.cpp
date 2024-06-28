#include "waveform_editor.h"

#include "draw_operations.h"
#include "Geometry.hpp"
#include "waveform.h"
#include "waveform_part.h"

namespace fmpire
{
WaveformEditor::WaveformEditor(Widget* parent) :
	RelativeContainer(parent),
	waveform(nullptr),
	selection(nullptr)
{
}

WaveformEditor::~WaveformEditor() noexcept
{
}

void WaveformEditor::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();

	theme->background.setFor(context);
	Line<float> line(0, getHeight() / 2, getWidth(), getHeight() / 2);
	line.draw(context, theme->line_thin);

	std::vector<float> samples;

	if (waveform)
	{
		samples = waveform->sample_all();
	}
	if (samples.size() < 2)
	{
		samples = {0, 0};
	}

	if (selection)
	{
		// float x0 = (float) selection->
		// Rectangle<float> selection_box(, 0, , getHeight());
		theme->foreground.setFor(context);
	}

	theme->highlight.setFor(context);
	for (size_t sample = 1; sample < samples.size(); sample++)
	{
		float x0 = (float) (sample - 1) / (samples.size() - 1);
		float x1 = (float) sample / (samples.size() - 1);
		x0 *= getWidth();
		x1 *= getWidth();
		float y0 = getHeight() / 2 - samples[sample - 1] * getHeight() / 2;
		float y1 = getHeight() / 2 - samples[sample] * getHeight() / 2;
		line = Line<float>(x0, y0, x1, y1);
		line.draw(context, theme->line_thin);
	}

	RelativeContainer::onDisplay();

	theme->foreground.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 theme->line_thin);
}

bool WaveformEditor::onMouse(const MouseEvent& event)
{
	return false;
}

bool WaveformEditor::onMotion(const MotionEvent& event)
{
	return false;
}

void WaveformEditor::onPositionChanged(const PositionChangedEvent& event)
{
	RelativeContainer::onPositionChanged(event);
}

void WaveformEditor::onResize(const ResizeEvent& event)
{
	RelativeContainer::onResize(event);
}

} // namespace fmpire
