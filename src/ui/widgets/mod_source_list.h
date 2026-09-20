#ifndef MOD_SOURCE_LIST_H_INCLUDED
#define MOD_SOURCE_LIST_H_INCLUDED

#include "fmpire_widget.h"
#include "knob.h"
#include "modulation_model.h"
#include "state_manager.h"

#include <string>
#include <vector>

namespace fmpire
{

// List of every modulation source. At the top is a compact grid of the built-in
// sources: the host macros (a small knob each) and the MIDI/note sources. Below
// it come the user's LFOs/envelopes, one row each. Every source can be armed:
// while armed, modulatable knobs edit that source's amount.
//
// Several lists can exist at once (OSC/MOD/FX tabs); they share the model, so
// arming a source in one shows in all. Meant to sit in a vertical
// ScrollContainer, which sizes it to get_content_height().
class ModSourceList :
	public FMpireWidget,
	public ModulationModel::Listener,
	public StateManager::MacroListener,
	public Knob::Callback
{
public:
	static constexpr float row_height = 34.0f;

	ModSourceList(Widget* parent, StateManager& state_mgr);
	virtual ~ModSourceList() noexcept;

	float get_content_height() const;

	virtual void on_modulation_changed() override;

	virtual void on_macro_changed(const size_t index, const float value) override;

	virtual void drag_started(Knob* const knob) override;
	virtual void drag_ended(Knob* const knob) override;
	virtual void value_changed(Knob* const knob, const float value) override;

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;
	void onPositionChanged(const PositionChangedEvent& event) override;
	void onResize(const ResizeEvent& event) override;

private:
	// a built-in source in the grid at the top
	struct Tile
	{
		SourceId source;
		std::string label;
		std::string tooltip;
	};

	// a source in the list below the grid
	struct Row
	{
		SourceId source;
		std::string label;
	};

	StateManager& state_manager;
	ModulationModel& model;
	std::vector<Tile> tiles;
	std::vector<Ref<Knob>> macro_knobs;
	std::vector<Row> rows;

	void rebuild_rows();
	void layout_knobs();

	// geometry, in widget coordinates
	float tile_width() const;
	float grid_height() const;
	Rectangle<double> tile_rect(const size_t tile) const;
	// clickable part of a tile (the caption above the knob for macros)
	Rectangle<double> tile_toggle_rect(const size_t tile) const;
	Rectangle<double> tile_knob_rect(const size_t tile) const;
	float toggle_left() const;

	int tile_at(const Point<double>& pos) const;
	int row_at(const Point<double>& pos) const;
	int macro_of(Knob* const knob) const;

	void draw_tile(const GraphicsContext& context,
				   const size_t tile,
				   const SourceId armed);
	void draw_row(const GraphicsContext& context,
				  const size_t index,
				  const size_t selected,
				  const SourceId armed);
};

} // namespace fmpire

#endif // MOD_SOURCE_LIST_H_INCLUDED
