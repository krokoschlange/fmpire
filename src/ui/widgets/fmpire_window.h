#ifndef FMPIRE_WINDOW_H_INCLUDED
#define FMPIRE_WINDOW_H_INCLUDED

#include "tooltip.h"
#include "TopLevelWidget.hpp"

namespace fmpire
{

class FMpireWindow
{
public:
	FMpireWindow(TopLevelWidget* tlw);
	virtual ~FMpireWindow() noexcept;

	Tooltip& get_tooltip();

	void set_focus(FMpireWidget* widget);

	FMpireWidget* get_focus() const { return focus; }

	void set_ui_scale(float scale) { ui_scale = scale; }

	float get_ui_scale() const { return ui_scale; }

protected:
	void reinit_tooltip();

private:
	Tooltip* tooltip;
	FMpireWidget* focus;

	TopLevelWidget* top_level_widget;

	float ui_scale = 1.0f;
};

} // namespace fmpire

#endif // FMPIRE_WINDOW_H_INCLUDED
