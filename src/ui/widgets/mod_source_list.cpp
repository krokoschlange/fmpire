#include "mod_source_list.h"

#include "Color.hpp"
#include "draw_operations.h"

#include <algorithm>
#include <set>

namespace fmpire
{

namespace
{
constexpr size_t grid_columns = 4;
constexpr float tile_margin = 2.0f;
constexpr float macro_tile_height = 52.0f;
constexpr float macro_caption_height = 14.0f;
constexpr float midi_tile_height = 20.0f;

constexpr float toggle_width = 26.0f;
constexpr float toggle_margin = 6.0f;

constexpr size_t macro_grid_rows =
	(FMPIRE_MACRO_COUNT + grid_columns - 1) / grid_columns;

struct FixedSource
{
	SourceType type;
	uint16_t index;
	const char* label;
	const char* tooltip;
};

const FixedSource fixed_sources[] = {
	{SourceType::VELOCITY, 0, "VEL", "Velocity"},
	{SourceType::KEY, 0, "KEY", "Key"},
	{SourceType::PITCH_BEND, 0, "PB", "Pitch bend"},
	{SourceType::CHANNEL_PRESSURE, 0, "AT", "Channel aftertouch"},
	{SourceType::POLY_PRESSURE, 0, "PAT", "Polyphonic aftertouch"},
	{SourceType::MIDI_CC, 1, "MW", "Mod wheel (CC 1)"},
	{SourceType::MIDI_CC, 2, "BR", "Breath (CC 2)"},
	{SourceType::MIDI_CC, 11, "EXPR", "Expression (CC 11)"},
};

constexpr size_t fixed_source_count =
	sizeof(fixed_sources) / sizeof(fixed_sources[0]);
constexpr size_t midi_grid_rows =
	(fixed_source_count + grid_columns - 1) / grid_columns;

bool is_fixed_cc(const uint16_t controller)
{
	for (const FixedSource& fixed : fixed_sources)
	{
		if (fixed.type == SourceType::MIDI_CC && fixed.index == controller)
		{
			return true;
		}
	}
	return false;
}

void set_armed_color(const GraphicsContext& context)
{
	Color(255, 150, 40).setFor(context);
}
} // namespace

ModSourceList::ModSourceList(Widget* parent, StateManager& state_mgr) :
	FMpireWidget(parent),
	state_manager(state_mgr),
	model(state_mgr.get_modulation())
{
	for (size_t macro = 0; macro < FMPIRE_MACRO_COUNT; macro++)
	{
		tiles.push_back({{SourceType::MACRO, static_cast<uint16_t>(macro)},
						 "M" + std::to_string(macro + 1),
						 "Macro " + std::to_string(macro + 1)});

		Ref<Knob> knob = new Knob(this);
		knob->set_callback(this);
		knob->set_default_value(0);
		knob->set_value(state_manager.get_macro(macro));
		knob->set_tooltip("Macro " + std::to_string(macro + 1));
		macro_knobs.push_back(knob);
	}

	for (const FixedSource& fixed : fixed_sources)
	{
		tiles.push_back({{fixed.type, fixed.index}, fixed.label, fixed.tooltip});
	}

	model.add_listener(this);
	state_manager.add_macro_listener(this);
	rebuild_rows();
}

ModSourceList::~ModSourceList() noexcept
{
	state_manager.remove_macro_listener(this);
	model.remove_listener(this);
}

float ModSourceList::get_content_height() const
{
	// rows are rebuilt on every model change, which happens before layout
	return grid_height() + rows.size() * row_height;
}

void ModSourceList::on_modulation_changed()
{
	rebuild_rows();
	repaint();
}

void ModSourceList::on_macro_changed(const size_t index, const float value)
{
	if (index < macro_knobs.size())
	{
		macro_knobs[index]->set_value(value);
	}
}

int ModSourceList::macro_of(Knob* const knob) const
{
	for (size_t macro = 0; macro < macro_knobs.size(); macro++)
	{
		if (static_cast<Knob*>(macro_knobs[macro]) == knob)
		{
			return macro;
		}
	}
	return -1;
}

void ModSourceList::drag_started(Knob* const knob)
{
	const int macro = macro_of(knob);
	if (macro >= 0)
	{
		state_manager.begin_macro_edit(macro);
	}
}

void ModSourceList::drag_ended(Knob* const knob)
{
	const int macro = macro_of(knob);
	if (macro >= 0)
	{
		state_manager.end_macro_edit(macro);
	}
}

void ModSourceList::value_changed(Knob* const knob, const float value)
{
	const int macro = macro_of(knob);
	if (macro >= 0)
	{
		state_manager.set_macro(macro, value);
	}
}

void ModSourceList::rebuild_rows()
{
	rows.clear();

	for (size_t id = 0; id < model.modulator_id_count(); id++)
	{
		if (!model.has_modulator(id))
		{
			continue;
		}

		const bool is_lfo =
			model.get_modulator(id).settings.type == ModulatorType::LFO;
		rows.push_back({{SourceType::MODULATOR, static_cast<uint16_t>(id)},
						std::string(is_lfo ? "LFO " : "ENV ")
							+ std::to_string(id + 1)});
	}

	// controllers that are routed but don't have a fixed entry
	std::set<uint16_t> extra_controllers;
	for (size_t slot = 0; slot < model.route_slot_count(); slot++)
	{
		const RouteSettings* route = model.get_route(slot);
		if (route && route->source.type == SourceType::MIDI_CC
			&& !is_fixed_cc(route->source.index))
		{
			extra_controllers.insert(route->source.index);
		}
	}
	for (const uint16_t controller : extra_controllers)
	{
		rows.push_back({{SourceType::MIDI_CC, controller},
						"CC " + std::to_string(controller)});
	}
}

float ModSourceList::tile_width() const
{
	return getWidth() / static_cast<float>(grid_columns);
}

float ModSourceList::grid_height() const
{
	return macro_grid_rows * macro_tile_height
		 + midi_grid_rows * midi_tile_height;
}

Rectangle<double> ModSourceList::tile_rect(const size_t tile) const
{
	const float width = tile_width();
	if (tile < FMPIRE_MACRO_COUNT)
	{
		return Rectangle<double>((tile % grid_columns) * width,
								 (tile / grid_columns) * macro_tile_height,
								 width,
								 macro_tile_height);
	}

	const size_t index = tile - FMPIRE_MACRO_COUNT;
	return Rectangle<double>(
		(index % grid_columns) * width,
		macro_grid_rows * macro_tile_height
			+ (index / grid_columns) * midi_tile_height,
		width,
		midi_tile_height);
}

Rectangle<double> ModSourceList::tile_toggle_rect(const size_t tile) const
{
	const Rectangle<double> rect = tile_rect(tile);
	const float height =
		tile < FMPIRE_MACRO_COUNT ? macro_caption_height : rect.getHeight();
	return Rectangle<double>(rect.getX() + tile_margin,
							 rect.getY() + tile_margin,
							 rect.getWidth() - 2 * tile_margin,
							 height - tile_margin);
}

Rectangle<double> ModSourceList::tile_knob_rect(const size_t tile) const
{
	const Rectangle<double> rect = tile_rect(tile);
	return Rectangle<double>(rect.getX() + tile_margin,
							 rect.getY() + macro_caption_height,
							 rect.getWidth() - 2 * tile_margin,
							 rect.getHeight() - macro_caption_height
								 - tile_margin);
}

float ModSourceList::toggle_left() const
{
	return getWidth() - toggle_width - toggle_margin;
}

int ModSourceList::tile_at(const Point<double>& pos) const
{
	for (size_t tile = 0; tile < tiles.size(); tile++)
	{
		if (tile_rect(tile).contains(pos))
		{
			return tile;
		}
	}
	return -1;
}

int ModSourceList::row_at(const Point<double>& pos) const
{
	const double y = pos.getY() - grid_height();
	if (y < 0)
	{
		return -1;
	}

	const size_t index = static_cast<size_t>(y / row_height);
	return index < rows.size() ? static_cast<int>(index) : -1;
}

void ModSourceList::layout_knobs()
{
	for (size_t macro = 0; macro < macro_knobs.size(); macro++)
	{
		const Rectangle<double> rect = tile_knob_rect(macro);
		macro_knobs[macro]->setAbsolutePos(getAbsoluteX()
											   + static_cast<int>(rect.getX()),
										   getAbsoluteY()
											   + static_cast<int>(rect.getY()));
		macro_knobs[macro]->setSize(static_cast<uint>(rect.getWidth()),
									static_cast<uint>(rect.getHeight()));
	}
}

void ModSourceList::onPositionChanged(const PositionChangedEvent& event)
{
	layout_knobs();
}

void ModSourceList::onResize(const ResizeEvent& event)
{
	layout_knobs();
}

void ModSourceList::draw_tile(const GraphicsContext& context,
							  const size_t tile,
							  const SourceId armed)
{
	const bool is_armed =
		armed.type != SourceType::NONE && armed == tiles[tile].source;
	const bool is_macro = tile < FMPIRE_MACRO_COUNT;

	const Rectangle<double> box = tile_rect(tile);
	theme->background.setFor(context);
	draw_rounded_box(context,
					 box.getX() + tile_margin,
					 box.getY() + tile_margin,
					 box.getWidth() - 2 * tile_margin,
					 box.getHeight() - 2 * tile_margin,
					 theme->corner_radius,
					 1);

	// macros arm through their caption, the knob below is the value
	const Rectangle<double> toggle = tile_toggle_rect(tile);
	if (is_armed)
	{
		set_armed_color(context);
		fill_rounded_box(context,
						 toggle.getX(),
						 toggle.getY(),
						 toggle.getWidth(),
						 toggle.getHeight(),
						 theme->corner_radius,
						 1);
	}

	Color(255, 255, 255).setFor(context);
	draw_text(context,
			  tiles[tile].label.c_str(),
			  theme->font.c_str(),
			  (is_macro ? macro_caption_height : midi_tile_height) * 0.6f,
			  Anchor::CENTER,
			  toggle.getX() + toggle.getWidth() * 0.5f,
			  toggle.getY() + toggle.getHeight() * 0.5f);
}

void ModSourceList::draw_row(const GraphicsContext& context,
							 const size_t index,
							 const size_t selected,
							 const SourceId armed)
{
	const Row& row = rows[index];
	const float width = getWidth();
	const float top = grid_height() + index * row_height;
	const bool is_modulator = row.source.type == SourceType::MODULATOR;
	const bool is_selected = is_modulator && row.source.index == selected;
	const bool is_armed = armed.type != SourceType::NONE && armed == row.source;

	Corner corners = index == 0
					   ? Corner::TOP
					   : (index == rows.size() - 1 ? Corner::BOTTOM
												   : Corner::NONE);
	if (rows.size() == 1)
	{
		corners = Corner::ALL;
	}

	if (is_selected)
	{
		theme->foreground.setFor(context);
		fill_rounded_box(context,
						 0,
						 top,
						 width,
						 row_height,
						 theme->corner_radius,
						 theme->line_thin,
						 corners);
		theme->highlight.setFor(context);
	}
	else
	{
		theme->background.setFor(context);
	}
	draw_rounded_box(context,
					 0,
					 top,
					 width,
					 row_height,
					 theme->corner_radius,
					 is_selected ? theme->line_thin : 1,
					 corners);

	if (is_modulator)
	{
		const float thumbnail_left = width * 0.4f;
		const float thumbnail_width =
			std::max(toggle_left() - 8.0f, thumbnail_left + 2.0f)
			- thumbnail_left;
		const Curve& curve = model.get_modulator(row.source.index).curve;
		const size_t sample_count =
			std::max<size_t>((size_t) thumbnail_width, 2);
		std::vector<Point<float>> line(sample_count);
		for (size_t i = 0; i < sample_count; i++)
		{
			const float pos = (float) i / (sample_count - 1);
			line[i].setX(thumbnail_left + pos * thumbnail_width);
			line[i].setY(top + row_height * (0.85f - 0.7f * curve.sample(pos)));
		}
		theme->highlight.setFor(context);
		draw_line_string(context, line, 1);
	}

	Color(255, 255, 255).setFor(context);
	draw_text(context,
			  row.label.c_str(),
			  theme->font.c_str(),
			  row_height * 0.36f,
			  Anchor::LEFT_CENTER,
			  6,
			  top + row_height * 0.5f);

	// program toggle
	const float toggle_top = top + row_height * 0.2f;
	const float toggle_height = row_height * 0.6f;
	if (is_armed)
	{
		set_armed_color(context);
		fill_rounded_box(context,
						 toggle_left(),
						 toggle_top,
						 toggle_width,
						 toggle_height,
						 theme->corner_radius,
						 1);
	}
	else
	{
		theme->background.setFor(context);
		draw_rounded_box(context,
						 toggle_left(),
						 toggle_top,
						 toggle_width,
						 toggle_height,
						 theme->corner_radius,
						 1);
	}
	Color(255, 255, 255).setFor(context);
	draw_text(context,
			  "M",
			  theme->font.c_str(),
			  toggle_height * 0.6f,
			  Anchor::CENTER,
			  toggle_left() + toggle_width * 0.5f,
			  toggle_top + toggle_height * 0.5f);
}

void ModSourceList::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();
	const size_t selected = model.get_selected_modulator();
	const SourceId armed = model.get_armed_source();

	for (size_t tile = 0; tile < tiles.size(); tile++)
	{
		draw_tile(context, tile, armed);
	}

	for (size_t index = 0; index < rows.size(); index++)
	{
		draw_row(context, index, selected, armed);
	}
}

bool ModSourceList::onMouse(const MouseEvent& event)
{
	// the macro knobs
	const bool is_handled = FMpireWidget::onMouse(event);

	if (is_handled || !event.press || event.button != 1
		|| !contains_clipped(event.pos))
	{
		return is_handled;
	}

	const int tile = tile_at(event.pos);
	if (tile >= 0)
	{
		if (tile_toggle_rect(tile).contains(event.pos))
		{
			model.set_armed_source(tiles[tile].source);
		}
		return true;
	}

	const int index = row_at(event.pos);
	if (index < 0)
	{
		return false;
	}

	const Row& row = rows[index];
	if (event.pos.getX() >= toggle_left())
	{
		model.set_armed_source(row.source);
	}
	else if (row.source.type == SourceType::MODULATOR)
	{
		model.select_modulator(row.source.index);
	}
	return true;
}

bool ModSourceList::onMotion(const MotionEvent& event)
{
	if (FMpireWidget::onMotion(event))
	{
		return true;
	}

	if (contains_clipped(event.pos))
	{
		const int tile = tile_at(event.pos);
		if (tile >= 0 && tile_toggle_rect(tile).contains(event.pos))
		{
			show_tooltip(tiles[tile].tooltip,
						 event.absolutePos.getX(),
						 event.absolutePos.getY());
		}
	}
	return false;
}

} // namespace fmpire
