#ifndef TRIPLE_BUFFER_H_INCLUDED
#define TRIPLE_BUFFER_H_INCLUDED

#include <array>
#include <atomic>
#include <cstddef>

namespace fmpire
{

// Hands the latest complete value of T from one writer thread to one reader
// thread without locks or waiting on either side.
//
// The writer fills write_buffer() completely (its old contents are
// meaningless) and calls publish(). The reader calls read() whenever it wants
// the newest published value; it can keep using the returned buffer until its
// next read(). Publishing several times before the reader looks simply
// replaces the unread value, so nothing piles up while the reader is idle.
//
// The three buffers are owned by the writer, the reader and the exchange slot
// in between; no buffer is ever touched by both threads.
template<typename T> class TripleBuffer
{
public:
	TripleBuffer() :
		back(0),
		front(2),
		middle(1)
	{
	}

	TripleBuffer(const TripleBuffer&) = delete;
	TripleBuffer& operator=(const TripleBuffer&) = delete;

	// Only before the buffer is shared between threads: to give all three
	// buffers their initial value.
	T& initial_buffer(const size_t index) { return buffers[index]; }

	// Writer side.
	T& write_buffer() { return buffers[back]; }

	void publish()
	{
		const int previous =
			middle.exchange(back | fresh, std::memory_order_acq_rel);
		back = previous & index_mask;
	}

	// Reader side.
	const T& read()
	{
		if (middle.load(std::memory_order_relaxed) & fresh)
		{
			const int previous =
				middle.exchange(front, std::memory_order_acq_rel);
			front = previous & index_mask;
		}
		return buffers[front];
	}

private:
	static constexpr int index_mask = 3;
	static constexpr int fresh = 4;

	std::array<T, 3> buffers;

	// writer's buffer / reader's buffer (each only used by its own thread)
	int back;
	int front;

	// the buffer in between (index) and whether it holds something unread
	std::atomic<int> middle;
};

} // namespace fmpire

#endif // TRIPLE_BUFFER_H_INCLUDED
