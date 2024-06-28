#ifndef LABEL_H_INCLUDED
#define LABEL_H_INCLUDED

#include "fmpire_widget.h"

namespace fmpire
{

class Label : public FMpireWidget
{
public:
	Label(Widget* parent);
	virtual ~Label() noexcept;

	void set_text(std::string txt);
	void set_text_y_scale(const float scale);

protected:
	void onDisplay() override;

private:
	std::string text;
	float text_y_scale;
};

} // namespace fmpire

#endif // LABEL_H_INCLUDED
