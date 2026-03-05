#ifndef INCLUDED_RING_
#define INCLUDED_RING_

#include <mutex>
#include <condition_variable>
#include <array>
#include <optional>

// Thread-safe circular buffer for SCSP:
// - If the buffer is full, the producer sleeps (BSP in this case)
// - If the buffer is empty, the consumer gets NIL (MonitorWindow in this case)
template <typename T, size_t N>
class Ring
{
  std::array<T, N> d_data;
  size_t d_head = 0;
  size_t d_tail = 0;
  size_t d_count = 0;

  std::mutex d_mutex;
  std::condition_variable d_not_full;

public:
  Ring() = default;
  ~Ring() = default;

  void push(T &&value);
  std::optional<T> try_pop();
};

template <typename T, size_t N>
void Ring<T, N>::push(T &&value)
{
  std::unique_lock lock{d_mutex};
  d_not_full.wait(lock, [this](){ return d_count < N - 1; });

  d_data[d_head] = std::move(value);
  d_head = (d_head + 1) % N;
  d_count++;
}

template <typename T, size_t N>
std::optional<T> Ring<T, N>::try_pop()
{
  std::lock_guard _{d_mutex};

  if (d_count == 0) return std::nullopt;

  T result = std::move(d_data[d_tail]);
  d_tail = (d_tail + 1) % N;
  d_count--;

  d_not_full.notify_one();

  return result;
}

#endif // INCLUDED_RING_
