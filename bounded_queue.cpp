#include "BoundedQueue.h"
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono_literals;

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

  BoundedQueue<int> bq(100);
  auto prod = [&]() {
    std::mt19937 prodGen(std::random_device{}());
    std::uniform_int_distribution<> disProd(50, 200);
    std::uniform_int_distribution<> disProdVal(1, 100);
    while (true) {
      if (!bq.push(disProdVal(prodGen))) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(disProd(prodGen)));
    }
  };

  vector<int> myVec;
  auto cons = [&]() {
    std::mt19937 consGen(std::random_device{}());
    std::uniform_int_distribution<> disCons(100, 300);
    while (auto val = bq.pop()) {
      myVec.push_back(*val);
      std::this_thread::sleep_for(std::chrono::milliseconds(disCons(consGen)));
    }
    return myVec;
  };

  auto producer = thread(prod);
  auto consumer = thread(cons);

  // sleep for 30sec
  std::this_thread::sleep_for(5s);
  bq.stop();
  producer.join();
  consumer.join();

  // print the vector.
  cout << myVec << endl;
  return 0;
}