#ifndef WAVETABLE_HISTORY_H_INCLUDED
#define WAVETABLE_HISTORY_H_INCLUDED

#include <string>
#include <vector>

namespace fmpire
{

class WavetableHistory
{
public:
	WavetableHistory();

	void reset(const std::string& baseline);
	void push(const std::string& snapshot);

	bool is_empty() const;
	bool undo_possible() const;
	bool redo_possible() const;

	const std::string& undo();
	const std::string& redo();

private:
	static constexpr size_t max_entries = 32;

	std::vector<std::string> entries;
	size_t index;
};

} // namespace fmpire

#endif // WAVETABLE_HISTORY_H_INCLUDED
