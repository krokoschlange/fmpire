#include "source_list_page.h"

#include "state_manager.h"

namespace fmpire
{

SourceListPage::SourceListPage(Widget* parent,
							   StateManager& state_mgr,
							   const std::string& placeholder_text) :
	GridContainer(parent),
	model(state_mgr.get_modulation())
{
	add_row(1, 0, 0, 100, 0);
	add_column(1, 0, 0, 170, 0);
	add_column(5, 0, 0, 200, 0);

	list_scroll = new ScrollContainer(this);
	list_scroll->set_scroll_mode(ScrollContainer::VERTICAL);
	put(list_scroll, 0, 0);

	source_list = new ModSourceList(list_scroll, state_mgr);

	body_border = new Border(this);
	body_label = new Label(body_border);
	body_label->set_text(placeholder_text);
	put(body_border, 0, 1);

	model.add_listener(this);
	on_modulation_changed();
}

SourceListPage::~SourceListPage() noexcept
{
	model.remove_listener(this);
}

void SourceListPage::on_modulation_changed()
{
	list_scroll->set_scroll_area(0, source_list->get_content_height());
	repaint();
}

} // namespace fmpire
