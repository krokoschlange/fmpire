#ifndef MODULATION_MODEL_H_INCLUDED
#define MODULATION_MODEL_H_INCLUDED

#include "curve.h"
#include "mod_types.h"

#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

namespace fmpire
{
class StateManager;

// UI-side mirror of the DSP's modulators and routes. Edits update the mirror,
// notify listeners and send the matching state keys to the DSP; ids and route
// slots are chosen here and honoured by the DSP, so both sides agree.
class ModulationModel
{
public:
	static constexpr size_t npos = static_cast<size_t>(-1);

	struct ModulatorEntry
	{
		bool exists = false;
		ModulatorSettings settings;
		Curve curve;
	};

	struct Listener
	{
		virtual void on_modulation_changed() = 0;
	};

	explicit ModulationModel(StateManager& state_mgr);

	void add_listener(Listener* const listener);
	void remove_listener(Listener* const listener);

	// Modulators. Ids are stable; removed ids leave a gap that add_modulator
	// reuses.
	size_t modulator_id_count() const { return modulators.size(); }

	bool has_modulator(const size_t id) const;
	const ModulatorEntry& get_modulator(const size_t id) const;

	// Programming mode (UI-only): while a source is armed, modulatable knobs
	// edit that source's routes instead of their own value. Setting the armed
	// source again (or SourceType::NONE) disarms it.
	bool has_armed_source() const { return armed_source.type != SourceType::NONE; }

	SourceId get_armed_source() const { return armed_source; }

	void set_armed_source(const SourceId source);

	// The modulator shown in the editor (UI-only state), or npos.
	size_t get_selected_modulator() const { return selected_modulator; }

	void select_modulator(const size_t id);

	size_t add_modulator(const ModulatorType type);
	void remove_modulator(const size_t id);
	void set_modulator_settings(const size_t id, const ModulatorSettings& settings);
	void set_modulator_curve(const size_t id, const Curve& curve);

	// Routes.
	size_t route_slot_count() const { return routes.size(); }

	// nullptr when the slot is unused.
	const RouteSettings* get_route(const size_t slot) const;

	// Slot of the route for this exact source/target pair, or npos.
	size_t find_route(const SourceId source,
					  const TargetType target,
					  const size_t target_object) const;

	// Creates the route, or updates its amount if it exists; an amount of 0
	// removes it. Returns the slot, or npos when there is no route afterwards.
	size_t set_route_amount(const SourceId source,
							const TargetType target,
							const size_t target_object,
							const float amount);

	void set_route_bipolar(const size_t slot, const bool bipolar);
	void remove_route(const size_t slot);

	// The sections that follow the oscillators in the full state; replaces the
	// current contents and does not send anything.
	void parse_state(std::string_view& state);

private:
	StateManager& state_manager;
	std::vector<ModulatorEntry> modulators;
	std::vector<std::optional<RouteSettings>> routes;
	std::vector<Listener*> listeners;
	size_t selected_modulator;
	SourceId armed_source;

	void notify();
	void fix_selection();
	void send_route(const RouteSettings& route);
	void remove_routes_of_modulator(const size_t id);
	bool default_bipolar(const SourceId source) const;
};

} // namespace fmpire

#endif // MODULATION_MODEL_H_INCLUDED
