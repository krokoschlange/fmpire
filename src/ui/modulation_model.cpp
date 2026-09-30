#include "modulation_model.h"

#include "defines.h"
#include "state_manager.h"
#include "utils.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace fmpire
{

namespace
{
constexpr size_t max_modulators = FMPIRE_ID_SPACE;
constexpr size_t max_routes = FMPIRE_ID_SPACE;

std::string mod_key(const size_t id, const char* const sub_key)
{
	return std::string(KEY_MOD_PREFIX) + std::to_string(id) + "/" + sub_key;
}

std::string route_key(const size_t slot, const char* const sub_key)
{
	return std::string(KEY_ROUTE_PREFIX) + std::to_string(slot) + "/" + sub_key;
}
} // namespace

ModulationModel::ModulationModel(StateManager& state_mgr) :
	state_manager(state_mgr),
	selected_modulator(npos)
{
	matrix_depths.fill(0.0f);
	route_meters.fill(0.0f);
	playheads.fill(-1.0f);
}

size_t ModulationModel::matrix_index(const size_t type,
									 const size_t modulator,
									 const size_t carrier)
{
	return (type * FMPIRE_OSC_COUNT + carrier) * FMPIRE_OSC_COUNT + modulator;
}

float ModulationModel::get_matrix_depth(const size_t type,
										const size_t modulator,
										const size_t carrier) const
{
	if (type >= matrix_type_count || modulator >= FMPIRE_OSC_COUNT
		|| carrier >= FMPIRE_OSC_COUNT)
	{
		return 0.0f;
	}
	return matrix_depths[matrix_index(type, modulator, carrier)];
}

void ModulationModel::set_matrix_depth(const size_t type,
									   const size_t modulator,
									   const size_t carrier,
									   const float value)
{
	if (type >= matrix_type_count || modulator >= FMPIRE_OSC_COUNT
		|| carrier >= FMPIRE_OSC_COUNT)
	{
		return;
	}

	const float depth = std::clamp(value, 0.0f, 1.0f);
	matrix_depths[matrix_index(type, modulator, carrier)] = depth;

	state_manager.set_state(std::string(KEY_OSC_PREFIX) + std::to_string(carrier)
								+ "/" + KEY_OSC_MOD_DEPTH + std::to_string(type)
								+ "/" + std::to_string(modulator),
							encode_base64(reinterpret_cast<const uint8_t*>(&depth),
										  sizeof(depth)));
}

void ModulationModel::set_armed_source(const SourceId source)
{
	const SourceId new_source = armed_source == source ? SourceId() : source;
	if (!(new_source == armed_source))
	{
		armed_source = new_source;
		notify();
	}
}

void ModulationModel::select_modulator(const size_t id)
{
	const size_t new_selection = has_modulator(id) ? id : npos;
	if (new_selection != selected_modulator)
	{
		selected_modulator = new_selection;
		notify();
	}
}

// Keeps the selection on an existing modulator (or the first one).
void ModulationModel::fix_selection()
{
	if (has_modulator(selected_modulator))
	{
		return;
	}

	selected_modulator = npos;
	for (size_t id = 0; id < modulators.size(); id++)
	{
		if (modulators[id].exists)
		{
			selected_modulator = id;
			return;
		}
	}
}

void ModulationModel::add_listener(Listener* const listener)
{
	listeners.push_back(listener);
}

void ModulationModel::remove_listener(Listener* const listener)
{
	listeners.erase(std::remove(listeners.begin(), listeners.end(), listener),
					listeners.end());
}

float ModulationModel::get_route_meter(const size_t slot) const
{
	return slot < route_meters.size() ? route_meters[slot] : 0.0f;
}

void ModulationModel::set_route_meter(const size_t slot, const float value)
{
	if (slot >= route_meters.size() || route_meters[slot] == value)
	{
		return;
	}
	route_meters[slot] = value;

	const std::vector<Listener*> current = listeners;
	for (Listener* const listener : current)
	{
		listener->on_route_meter_changed(slot);
	}
}

float ModulationModel::get_target_meter(const TargetType target,
										const size_t target_object) const
{
	// every route of a target reports the same offset
	const size_t count = std::min(routes.size(), route_meters.size());
	for (size_t index = 0; index < count; index++)
	{
		if (routes[index] && routes[index]->target == target
			&& routes[index]->target_object == target_object)
		{
			return route_meters[index];
		}
	}
	return 0.0f;
}

float ModulationModel::get_playhead(const size_t id) const
{
	return id < playheads.size() ? playheads[id] : -1.0f;
}

void ModulationModel::set_playhead(const size_t id, const float value)
{
	if (id >= playheads.size() || playheads[id] == value)
	{
		return;
	}
	playheads[id] = value;

	const std::vector<Listener*> current = listeners;
	for (Listener* const listener : current)
	{
		listener->on_playhead_changed(id);
	}
}

void ModulationModel::notify()
{
	const std::vector<Listener*> current = listeners;
	for (Listener* const listener : current)
	{
		listener->on_modulation_changed();
	}
}

bool ModulationModel::has_modulator(const size_t id) const
{
	return id < modulators.size() && modulators[id].exists;
}

const ModulationModel::ModulatorEntry&
	ModulationModel::get_modulator(const size_t id) const
{
	static const ModulatorEntry empty;
	return id < modulators.size() ? modulators[id] : empty;
}

size_t ModulationModel::add_modulator(const ModulatorType type)
{
	size_t id = 0;
	while (id < modulators.size() && modulators[id].exists)
	{
		id++;
	}
	if (id >= max_modulators)
	{
		return npos;
	}
	if (id >= modulators.size())
	{
		modulators.resize(id + 1);
	}

	ModulatorEntry& entry = modulators[id];
	entry = ModulatorEntry();
	entry.exists = true;
	entry.settings.type = type;
	if (type == ModulatorType::LFO)
	{
		entry.curve = Curve::make_lfo();
		entry.settings.length_seconds = 2.0f;
	}
	else
	{
		entry.curve = Curve::make_envelope();
	}

	state_manager.set_state(mod_key(id, KEY_MOD_CREATE),
							entry.settings.encode() + entry.curve.encode());
	selected_modulator = id;
	notify();
	return id;
}

void ModulationModel::remove_modulator(const size_t id)
{
	if (!has_modulator(id))
	{
		return;
	}

	modulators[id] = ModulatorEntry();
	while (!modulators.empty() && !modulators.back().exists)
	{
		modulators.pop_back();
	}

	// the DSP drops the same routes when it removes the modulator
	remove_routes_of_modulator(id);

	state_manager.set_state(mod_key(id, KEY_MOD_REMOVE), "");
	if (armed_source.type == SourceType::MODULATOR && armed_source.index == id)
	{
		armed_source = SourceId();
	}
	fix_selection();
	notify();
}

void ModulationModel::set_modulator_settings(const size_t id,
											 const ModulatorSettings& settings)
{
	if (!has_modulator(id))
	{
		return;
	}
	modulators[id].settings = settings;
	state_manager.set_state(mod_key(id, KEY_MOD_SETTINGS), settings.encode());
	notify();
}

void ModulationModel::set_modulator_curve(const size_t id, const Curve& curve)
{
	if (!has_modulator(id))
	{
		return;
	}
	modulators[id].curve = curve;
	state_manager.set_state(mod_key(id, KEY_MOD_CURVE), curve.encode());
	notify();
}

const RouteSettings* ModulationModel::get_route(const size_t slot) const
{
	if (slot < routes.size() && routes[slot].has_value())
	{
		return &*routes[slot];
	}
	return nullptr;
}

size_t ModulationModel::find_route(const SourceId source,
								   const TargetType target,
								   const size_t target_object) const
{
	for (size_t slot = 0; slot < routes.size(); slot++)
	{
		const std::optional<RouteSettings>& route = routes[slot];
		if (route && route->source == source && route->target == target
			&& route->target_object == target_object)
		{
			return slot;
		}
	}
	return npos;
}

bool ModulationModel::default_bipolar(const SourceId source) const
{
	switch (source.type)
	{
	case SourceType::PITCH_BEND:
		return true;
	case SourceType::MODULATOR:
		return has_modulator(source.index)
			&& modulators[source.index].settings.type == ModulatorType::LFO;
	default:
		return false;
	}
}

size_t ModulationModel::set_route_amount(const SourceId source,
										 const TargetType target,
										 const size_t target_object,
										 const float amount)
{
	const float clamped = std::clamp(amount, -1.0f, 1.0f);
	const bool remove = std::fabs(clamped) < 0.001f;
	const size_t existing = find_route(source, target, target_object);

	if (existing != npos)
	{
		if (remove)
		{
			remove_route(existing);
			return npos;
		}

		routes[existing]->amount = clamped;
		state_manager.set_state(route_key(existing, KEY_ROUTE_AMOUNT),
								encode_base64(reinterpret_cast<const uint8_t*>(&clamped),
											  sizeof(clamped)));
		notify();
		return existing;
	}

	if (remove)
	{
		return npos;
	}

	size_t slot = 0;
	while (slot < routes.size() && routes[slot].has_value())
	{
		slot++;
	}
	if (slot >= max_routes)
	{
		return npos;
	}
	if (slot >= routes.size())
	{
		routes.resize(slot + 1);
	}

	RouteSettings route;
	route.slot = slot;
	route.source = source;
	route.target = target;
	route.target_object = target_object;
	route.amount = clamped;
	route.bipolar = default_bipolar(source);
	routes[slot] = route;

	send_route(route);
	notify();
	return slot;
}

void ModulationModel::set_route_bipolar(const size_t slot, const bool bipolar)
{
	if (!get_route(slot))
	{
		return;
	}
	routes[slot]->bipolar = bipolar;
	send_route(*routes[slot]);
	notify();
}

void ModulationModel::remove_route(const size_t slot)
{
	if (!get_route(slot))
	{
		return;
	}
	routes[slot].reset();
	while (!routes.empty() && !routes.back().has_value())
	{
		routes.pop_back();
	}
	state_manager.set_state(route_key(slot, KEY_ROUTE_REMOVE), "");
	notify();
}

void ModulationModel::send_route(const RouteSettings& route)
{
	state_manager.set_state(route_key(route.slot, KEY_ROUTE_SET),
							route.encode());
}

void ModulationModel::remove_routes_of_modulator(const size_t id)
{
	for (std::optional<RouteSettings>& route : routes)
	{
		if (!route)
		{
			continue;
		}

		const bool uses_as_source =
			route->source.type == SourceType::MODULATOR && route->source.index == id;
		const bool uses_as_target =
			is_modulator_target(route->target)
			&& route->target_object == id;
		if (uses_as_source || uses_as_target)
		{
			route.reset();
		}
	}
	while (!routes.empty() && !routes.back().has_value())
	{
		routes.pop_back();
	}
}

void ModulationModel::parse_state(std::string_view& state)
{
	modulators.clear();
	routes.clear();
	matrix_depths.fill(0.0f);

	while (!state.empty())
	{
		if (state.starts_with(MATRIX_DATA_STRING))
		{
			// per carrier: [type][modulator]
			state.remove_prefix(MATRIX_DATA_STRING.size());
			for (size_t carrier = 0; carrier < FMPIRE_OSC_COUNT; carrier++)
			{
				for (size_t type = 0; type < matrix_type_count; type++)
				{
					for (size_t modulator = 0; modulator < FMPIRE_OSC_COUNT;
						 modulator++)
					{
						float value = 0.0f;
						decode_base64(state,
									  reinterpret_cast<uint8_t*>(&value),
									  sizeof(value));
						matrix_depths[matrix_index(type, modulator, carrier)] =
							std::isfinite(value) ? std::clamp(value, 0.0f, 1.0f)
												 : 0.0f;
					}
				}
			}
		}
		else if (state.starts_with(MODULATOR_DATA_STRING))
		{
			state.remove_prefix(MODULATOR_DATA_STRING.size());
			uint32_t id = 0;
			decode_base64(state, reinterpret_cast<uint8_t*>(&id), sizeof(id));
			if (id >= max_modulators)
			{
				break;
			}

			ModulatorEntry entry;
			entry.exists = true;
			entry.settings.decode(state);
			entry.curve.decode(state);

			if (id >= modulators.size())
			{
				modulators.resize(id + 1);
			}
			modulators[id] = entry;
		}
		else if (state.starts_with(ROUTE_DATA_STRING))
		{
			state.remove_prefix(ROUTE_DATA_STRING.size());
			RouteSettings route;
			route.decode(state);
			if (route.slot >= max_routes)
			{
				break;
			}

			if (route.slot >= routes.size())
			{
				routes.resize(route.slot + 1);
			}
			routes[route.slot] = route;
		}
		else
		{
			break;
		}
	}

	if (armed_source.type == SourceType::MODULATOR
		&& !has_modulator(armed_source.index))
	{
		armed_source = SourceId();
	}
	fix_selection();
	notify();
}

} // namespace fmpire
