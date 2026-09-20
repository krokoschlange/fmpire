#include "wavetable_history.h"

namespace fmpire
{

WavetableHistory::WavetableHistory() : index(0)
{
}

void WavetableHistory::reset(const std::string& baseline)
{
	entries.clear();
	entries.push_back(baseline);
	index = 0;
}

void WavetableHistory::push(const std::string& snapshot)
{
	entries.erase(entries.begin(), entries.begin() + index);
	index = 0;

	if (entries.size() >= max_entries)
	{
		entries.pop_back();
	}

	entries.insert(entries.begin(), snapshot);
}

bool WavetableHistory::is_empty() const
{
	return entries.empty();
}

bool WavetableHistory::undo_possible() const
{
	return entries.size() > 0 && index < entries.size() - 1;
}

bool WavetableHistory::redo_possible() const
{
	return index > 0;
}

const std::string& WavetableHistory::undo()
{
	index += 1;
	return entries[index];
}

const std::string& WavetableHistory::redo()
{
	index -= 1;
	return entries[index];
}

} // namespace fmpire
