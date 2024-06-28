#ifndef TOOLTIP_H_INCLUDED
#define TOOLTIP_H_INCLUDED

#include "fmpire_widget.h"

namespace fmpire
{
class FMpireUI;

class Tooltip : public FMpireWidget
{
public:
	Tooltip(Widget* parent);
	virtual ~Tooltip() noexcept;

	void request(const std::string& txt,
				 const float x,
				 const float y,
				 const bool pin = false);
	void set_text(const std::string& txt);
	void set_text_size(const float size);

	void unpin();

	void idle();

	bool handle_motion(const MotionEvent& event);

protected:
	void onDisplay() override;

private:
	std::string text;
	float text_size;

	bool requested;
	bool pinned;
	Point<double> last_mouse_pos;
	uint64_t last_time;
	uint64_t delay;
};


} // namespace fmpire

#endif // TOOLTIP_H_INCLUDED
