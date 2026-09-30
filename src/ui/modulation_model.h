#ifndef MODULATION_MODEL_H_INCLUDED
#define MODULATION_MODEL_H_INCLUDED

#include "curve.h"
#include "mod_types.h"

#include "defines.h"

#include <array>
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

		// The live modulation reported for a route slot changed (this is
		// called often, unlike on_modulation_changed).
		virtual void on_route_meter_changed(const size_t slot) {}

		// The playhead reported for a modulator changed (also often).
		virtual void on_playhead_changed(const size_t id) {}
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

	// Live modulation: how far the DSP currently moves the target of the
	// route in `slot` away from its knob value (-1..1, in knob units; the
	// same for all routes of one target). Only the first FMPIRE_METER_COUNT
	// slots are reported; the others read as 0. UI-only, sends nothing.
	float get_route_meter(const size_t slot) const;
	void set_route_meter(const size_t slot, const float value);

	// The live offset of a target (0 if no route modulates it).
	float get_target_meter(const TargetType target,
						   const size_t target_object) const;

	// Where modulator `id` currently is on its curve (0..1), or -1 if it
	// isn't playing (or is beyond the first FMPIRE_PLAYHEAD_COUNT ids).
	// UI-only, sends nothing.
	float get_playhead(const size_t id) const;
	void set_playhead(const size_t id, const float value);

	// Cross modulation depths (0..1): how much oscillator `modulator` modulates
	// oscillator `carrier`, for type 0..3 = AM/FM/PM/RM. Setting one sends it
	// to the DSP; it does not notify the listeners (the matrix view that edits
	// it already shows the value).
	float get_matrix_depth(const size_t type,
						   const size_t modulator,
						   const size_t carrier) const;
	void set_matrix_depth(const size_t type,
						  const size_t modulator,
						  const size_t carrier,
						  const float value);

	// The sections that follow the oscillators in the full state; replaces the
	// current contents and does not send anything.
	void parse_state(std::string_view& state);

private:
	StateManager& state_manager;
	std::vector<ModulatorEntry> modulators;
	std::vector<std::optional<RouteSettings>> routes;
	std::array<float, matrix_type_count * FMPIRE_OSC_COUNT * FMPIRE_OSC_COUNT>
		matrix_depths;
	std::array<float, FMPIRE_METER_COUNT> route_meters;
	std::array<float, FMPIRE_PLAYHEAD_COUNT> playheads;
	std::vector<Listener*> listeners;
	size_t selected_modulator;
	SourceId armed_source;

	static size_t matrix_index(const size_t type,
							   const size_t modulator,
							   const size_t carrier);

	void notify();
	void fix_selection();
	void send_route(const RouteSettings& route);
	void remove_routes_of_modulator(const size_t id);
	bool default_bipolar(const SourceId source) const;
};

} // namespace fmpire

#endif // MODULATION_MODEL_H_INCLUDED
