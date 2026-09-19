#ifndef TEXT_ENTRY_H_INCLUDED
#define TEXT_ENTRY_H_INCLUDED

#include "fmpire_widget.h"
#include <string>

namespace fmpire
{

class TextEntry : public FMpireWidget
{
public:
	TextEntry(Widget* widget);
	virtual ~TextEntry() noexcept;

	struct Callback
	{
		virtual void on_value_changed(TextEntry* const text_entry,
									  const std::wstring& value) = 0;
	};

	void set_callback(Callback* const cb) { callback = cb; }

	void set_text(const std::wstring& text) { value = text; }

	const std::wstring& get_text() const { return value; }

	void set_text(const std::string& text);
	std::string get_text_utf8() const;

protected:
	virtual void onDisplay() override;

	virtual bool onMouse(const MouseEvent& event) override;
	virtual bool onCharacterInput(const CharacterInputEvent& event) override;
	virtual bool onKeyboard(const KeyboardEvent& event) override;

private:
	float get_text_advance(const std::wstring& text) const;
	void update_scroll();

	std::wstring value;
	int cursor_position;
	float scroll_offset;

	Callback* callback;
};

} // namespace fmpire

#endif // TEXT_ENTRY_H_INCLUDED
