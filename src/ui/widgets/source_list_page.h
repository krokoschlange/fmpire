#ifndef SOURCE_LIST_PAGE_H_INCLUDED
#define SOURCE_LIST_PAGE_H_INCLUDED

#include "border.h"
#include "grid_container.h"
#include "label.h"
#include "mod_source_list.h"
#include "modulation_model.h"
#include "scroll_container.h"

#include <string>

namespace fmpire
{
class StateManager;

class SourceListPage : public GridContainer, public ModulationModel::Listener
{
public:
	SourceListPage(Widget* parent,
				   StateManager& state_mgr,
				   const std::string& placeholder_text);
	virtual ~SourceListPage() noexcept;

	virtual void on_modulation_changed() override;

private:
	ModulationModel& model;

	Ref<ScrollContainer> list_scroll;
	Ref<ModSourceList> source_list;
	Ref<Border> body_border;
	Ref<Label> body_label;
};

} // namespace fmpire

#endif // SOURCE_LIST_PAGE_H_INCLUDED
