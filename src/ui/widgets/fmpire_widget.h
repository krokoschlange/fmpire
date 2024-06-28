#ifndef FMPIRE_WIDGET_H_INCLUDED
#define FMPIRE_WIDGET_H_INCLUDED

#include "SubWidget.hpp"

#include "theme.h"

namespace fmpire
{
class FMpireWindow;

class FMpireWidget : public SubWidget
{
public:
	FMpireWidget(Widget* parent);
	virtual ~FMpireWidget() noexcept;

protected:
	Theme* theme;

	void show_tooltip(const std::string& text,
					  float x,
					  float y,
					  const bool pin = false);
	void update_tooltip(const std::string& text);
	void unpin_tooltip();

	FMpireWindow* window;
};

} // namespace fmpire

#endif // FMPIRE_WIDGET_H_INCLUDED
