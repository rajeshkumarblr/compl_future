#ifndef BOUNDED_QUEUE_H
#define BOUNDED_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <typename T> class BoundedQueue {
private:
  long unsigned capacity;
  bool shouldStop;
  std::queue<T> q;
  std::mutex m;
  std::condition_variable queue_full, queue_empty;

public:
  BoundedQueue(int cap) : capacity(cap), shouldStop(false) {}

  bool push(const T &val) {
    std::unique_lock<std::mutex> lock(m);
    queue_full.wait(lock, [this]() { return q.size() < capacity || shouldStop; });
    if (shouldStop)
      return false;
    q.push(val);
    queue_empty.notify_one();
    return true;
  }

  std::optional<T> pop() {
    std::unique_lock<std::mutex> lock(m);
    queue_empty.wait(lock, [this]() { return !q.empty() || shouldStop; });
    if (shouldStop && q.empty())
      return std::nullopt;
    T val = std::move(q.front());
    q.pop();
    queue_full.notify_one();
    return val;
  }

  void stop() {
    {
      std::lock_guard<std::mutex> lock(m);
      shouldStop = true;
    }
    queue_full.notify_all();
    queue_empty.notify_all();
  }
};

#endif // BOUNDED_QUEUE_H
