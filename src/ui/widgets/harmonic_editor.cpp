#include "harmonic_editor.h"
#include "Base.hpp"
#include "draw_operations.h"
#include "waveform_part.h"

namespace fmpire
{

HarmonicEditor::HarmonicEditor(Widget* parent) :
	FMpireWidget(parent)
{
}

HarmonicEditor::~HarmonicEditor() noexcept
{
}

void HarmonicEditor::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	theme->background.setFor(context);
	Rectangle<float> rect(0,
						  getHeight() * 0.5 - box_width * 0.25,
						  getWidth(),
						  box_width * 0.5);
	rect.draw(context);

	uint32_t box_count = part != nullptr ? part->get_harmonics().size() : 128;

	for (uint32_t i = 0; i < box_count; i++)
	{
		draw_text(context,
				  std::to_string(i).c_str(),
				  theme->font.c_str(),
				  box_width * 0.5,
				  Anchor::CENTER,
				  box_width * (i + 0.5),
				  getHeight() * 0.5);
	}

	if (part == nullptr)
	{
		return;
	}
}

bool HarmonicEditor::onMouse(const MouseEvent& event)
{
	return false;
}

bool HarmonicEditor::onMotion(const MotionEvent& event)
{
	return false;
}

} // namespace fmpire
