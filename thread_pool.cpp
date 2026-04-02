#include <chrono>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

// 1. ThreadPool implementation
class ThreadPool {
public:
  ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; ++i) {
      workers.emplace_back([this] {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(
                lock, [this] { return this->stop || !this->tasks.empty(); });
            if (this->stop && this->tasks.empty())
              return;
            task = std::move(this->tasks.front());
            this->tasks.pop();
          }
          task();
        }
      });
    }
  }

  template <class F, class... Args>
  auto enqueue(F &&f, Args &&...args)
      -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);

      // don't allow enqueueing after stopping the pool
      if (stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");

      tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
      worker.join();
  }

private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};

// 2. Data structures
struct File {
  std::string name;
};

struct MetaData {
  std::string filename;
  uintmax_t size;
};

int main(int argc, char *argv[]) {
  std::string root_path = "/home/rajesh/proj/dlake";
  int numThreads = 10;

  // Generic print function
  std::function<void(const std::string &)> print =
      [](const std::string &message) {
        std::cout << "[THREAD_POOL] " << message << std::endl;
      };

  if (argc > 1) {
    root_path = argv[1];
  }
  if (argc > 2) {
    try {
      numThreads = std::stoi(argv[2]);
      if (numThreads <= 0)
        numThreads = 10;
    } catch (...) {
      numThreads = 10;
    }
  }

  print("Starting the Thread Pool process with " + std::to_string(numThreads) + " threads...");

  std::vector<File> files_to_process;
  try {
    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
      std::cerr << "Error: " << root_path << " is not a valid directory." << std::endl;
      return 1;
    }
    for (const auto &entry : fs::recursive_directory_iterator(root_path)) {
      if (entry.is_regular_file()) {
        files_to_process.push_back({entry.path().string()});
      }
    }
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error during traversal: " << e.what() << std::endl;
  }

  if (files_to_process.empty()) {
    print("No files found.");
    return 0;
  }

  print("Found " + std::to_string(files_to_process.size()) + " files.");

  ThreadPool pool(numThreads);
  std::vector<std::future<MetaData>> futures;
  std::vector<MetaData> results;
  std::mutex resultsMutex;

  for (const auto &file : files_to_process) {
    futures.push_back(pool.enqueue([&results, &resultsMutex, file]() {
      try {
        MetaData m = MetaData{file.name, fs::file_size(file.name)};
        {
          std::lock_guard<std::mutex> lock(resultsMutex);
          results.push_back(m);
        }
        return m;
      } catch (...) {
        return MetaData{file.name, 0};
      }
    }));
  }

  // Wait for all results
  for (auto &f : futures) {
    f.get();
  }

  print("Processed all files.");
  std::cout << "\nResults (First 5):" << std::endl;
  for (size_t i = 0; i < std::min(results.size(), (size_t)5); ++i) {
    std::cout << "[" << i + 1 << "] " << results[i].filename << ": "
              << results[i].size << " bytes" << std::endl;
  }

  return 0;
}
