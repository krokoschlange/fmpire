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

protected:
	void reinit_tooltip();

private:
	Tooltip* tooltip;
	FMpireWidget* focus;

	TopLevelWidget* top_level_widget;
};

} // namespace fmpire

#endif // FMPIRE_WINDOW_H_INCLUDED
