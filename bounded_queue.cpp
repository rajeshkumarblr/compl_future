#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono_literals;


template <typename T> class BoundedQueue {
private:
  long unsigned capacity;
  bool shouldStop;
  queue<T> q;
  mutex m;
  condition_variable queue_full, queue_empty;

public:
  BoundedQueue(int cap) : capacity(cap), shouldStop(false) {}

  void push(T &val) {
    unique_lock<mutex> lock(m);
    queue_full.wait(lock, [&]() { return q.size() < capacity || shouldStop; });
    if (shouldStop)
      return; // Exit cleanly if we were woken up to stop
    q.push(val);
    queue_empty.notify_one();
  }

  std::optional<T> pop() {
    unique_lock<mutex> lock(m);
    queue_empty.wait(lock, [&]() { return !q.empty() || shouldStop; });
    if (shouldStop && q.empty())
      return std::nullopt;
    T val = q.front();
    q.pop();
    queue_full.notify_one();
    return val;
  }

  void stop() {
    shouldStop = true;
    queue_full.notify_all();
  }
};

// Do NOT do this in production!
template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
  os << "[";
  if (!v.empty()) {
    os << v[0];
    for (size_t i = 1; i < v.size(); ++i) {
      os << ", " << v[i];
    }
  }
  return os << "]";
}

int main() {

  bool shouldStop = false;

  BoundedQueue<int> bq(100);
  auto prod = [&]() {
    std::mt19937 prodGen(std::random_device{}());
    std::uniform_int_distribution<> disProd(50, 200);
    std::uniform_int_distribution<> disProdVal(1, 100);
    while (!shouldStop) {
      int val = disProdVal(prodGen);
      bq.push(val);
      std::this_thread::sleep_for(std::chrono::milliseconds(disProd(prodGen)));
    }
  };

  vector<int> myVec;
  auto cons = [&]() {
    std::mt19937 consGen(std::random_device{}());
    std::uniform_int_distribution<> disCons(100, 300);
    while (!shouldStop) {
      auto val = bq.pop();
      if (val) {
        myVec.push_back(*val);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(disCons(consGen)));
    }
  };

  auto producer = thread(prod);
  auto consumer = thread(cons);

  // sleep for 30sec
  std::this_thread::sleep_for(5s);
  shouldStop = true;
  bq.stop();
  producer.join();
  consumer.join();

  // print the vector.
  cout << myVec;
  return 0;
}