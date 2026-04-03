#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono_literals;

std::queue<int> q;
mutex m;
condition_variable queue_full, queue_empty;
using prodFunc = std::function<int()>;
using consumerFunc = std::function<void(int val)>;

class ProducerThread {
private:
  long unsigned capacity;
  bool shouldStop;
  prodFunc produce;

public:
  ProducerThread(int cap, ::prodFunc pf)
      : capacity(cap), shouldStop(false), produce(pf) {}

  void run() {
    while (true) {
      int val = produce();
      {
        unique_lock<mutex> lock(m);
        queue_full.wait(lock,
                        [&]() { return q.size() < capacity || shouldStop; });
        if (shouldStop)
          break; // Exit cleanly if we were woken up to stop
        q.push(val);
        queue_empty.notify_one();
      }
    }
  }

  void stop() {
    shouldStop = true;
    queue_full.notify_all();
  }
};

class ConsumerThread {
private:
  bool shouldStop;
  consumerFunc cFunc;

public:
  ConsumerThread(consumerFunc cf) : shouldStop(false), cFunc(cf) {}

  void run() {
    int val;
    while (true) {
      {
        unique_lock<mutex> lock(m);
        queue_empty.wait(lock, [&]() { return !q.empty() || shouldStop; });
        if (shouldStop && q.empty())
          break; // Exit cleanly if we were woken up to stop
        val = q.front();
        q.pop();
        queue_full.notify_one();
      }
      cFunc(val);
    }
  }
  void stop() {
    {
      std::lock_guard<std::mutex> lock(m);
      shouldStop = true;
    }
    queue_empty.notify_all();
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
  std::mt19937 prodGen(std::random_device{}());
  std::mt19937 consGen(std::random_device{}());
  std::uniform_int_distribution<> disProd(50, 200);
  std::uniform_int_distribution<> disProdVal(1, 100);
  std::uniform_int_distribution<> disCons(100, 300);

  vector<int> myVec;
  prodFunc pf = [&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(disProd(prodGen)));
    return disProdVal(prodGen);
  };

  consumerFunc cf = [&](int val) {
    std::this_thread::sleep_for(std::chrono::milliseconds(disCons(consGen)));
    myVec.push_back(val);
  };

  ProducerThread producerThread(10, pf);
  ConsumerThread consThread(cf);

  auto producer = thread(&ProducerThread::run, &producerThread);
  auto consumer = thread(&ConsumerThread::run, &consThread);

  // sleep for 30sec
  std::this_thread::sleep_for(5s);
  // send a signal to stop both threads.
  producerThread.stop();
  consThread.stop();
  producer.join();
  consumer.join();

  // print the vector.
  cout << myVec;
  return 0;
}