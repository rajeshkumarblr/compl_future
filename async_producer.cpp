#include <future>
#include <iostream>
#include <queue>

using namespace std;

class ProducerThread {
public:
  queue<int> run() {
    queue<int> q;
    for (int i = 0; i < 10; i++) {
      q.push(i);
    }
    return q;
  }
};

int main() {
  ProducerThread p;

  // FIX: Add 'return' inside the lambda
  auto fut = async(launch::async, [&p] { return p.run(); });

  // Wait for completion AND safely extract the data in one step
  queue<int> q = fut.get();

  while (!q.empty()) {
    cout << q.front() << ",";
    q.pop();
  }
  cout << "\n";
  return 0;
}