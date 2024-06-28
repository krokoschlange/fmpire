#ifndef SELECTOR_H_INCLUDED
#define SELECTOR_H_INCLUDED

#include "fmpire_widget.h"

#include <string>
#include <vector>

namespace fmpire
{

class Selector : public FMpireWidget
{
public:
	Selector(Widget* parent);
	virtual ~Selector() noexcept;

	void add_option(std::string option);
	void remove_option(std::string option);
	void clear_options();

	void select(const int index, const bool emit_callback = false);
	
	void set_text_y_scale(const float scale);

	struct Callback
	{
		virtual void on_selected(Selector* const selector,
								 const int index,
								 const std::string& option) = 0;
	};

	void set_callback(Callback* cb);

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;

private:
	std::vector<std::string> options;
	int selected;
	int hover;

	float text_y_scale;

	Callback* callback;
};


} // namespace fmpire

#endif // SELECTOR_H_INCLUDED
