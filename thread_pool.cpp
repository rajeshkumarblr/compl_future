#include <atomic>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

// 1. Specific Data structures
struct File {
  std::string name;
};

struct MetaData {
  std::string filename;
  uintmax_t size;
};

// 2. Simplified Thread Pool specific for File processing
class FileProcessorPool {
public:
  // Constructor takes the number of worker threads and the specific worker function
  FileProcessorPool(size_t threads, std::function<void(const File &)> worker)
      : workerFunc(worker), activeTasks(0), stop(false) {
    for (size_t i = 0; i < threads; ++i) {
      workers.emplace_back([this] {
        for (;;) {
          File file;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(
                lock, [this] { return this->stop || !this->files.empty(); });
            if (this->stop && this->files.empty())
              return;
            file = std::move(this->files.front());
            this->files.pop();
          }
          // Execute the specific worker function
          workerFunc(file);
          activeTasks--;
          // Notify any thread waiting for all tasks to complete
          completion_cv.notify_all();
        }
      });
    }
  }

  // Simple method to push a file into the processing queue
  void addFile(const File &f) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      files.push(f);
      activeTasks++;
    }
    condition.notify_one();
  }

  // Wait until all queued files have been processed
  void waitForCompletion() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    completion_cv.wait(lock, [this] { return files.empty() && activeTasks == 0; });
  }

  // Destructor stops all threads
  ~FileProcessorPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
      if (worker.joinable())
        worker.join();
    }
  }

private:
  std::function<void(const File &)> workerFunc;
  std::vector<std::thread> workers;
  std::queue<File> files;
  std::atomic<int> activeTasks;

  std::mutex queue_mutex;
  std::condition_variable condition;
  std::condition_variable completion_cv;
  bool stop;
};

int main(int argc, char *argv[]) {
  std::string root_path = "/home/rajesh/proj/dlake";
  int numThreads = 10;

  // Generic print function
  auto print = [](const std::string &message) {
    std::cout << "[POOL_V2] " << message << std::endl;
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

  print("Starting the Simplified Thread Pool process with " +
        std::to_string(numThreads) + " threads...");

  std::vector<File> files_to_scan;
  try {
    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
      std::cerr << "Error: " << root_path << " is not a valid directory."
                << std::endl;
      return 1;
    }
    for (const auto &entry : fs::recursive_directory_iterator(root_path)) {
      if (entry.is_regular_file()) {
        files_to_scan.push_back({entry.path().string()});
      }
    }
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error during traversal: " << e.what() << std::endl;
  }

  if (files_to_scan.empty()) {
    print("No files found.");
    return 0;
  }

  print("Found " + std::to_string(files_to_scan.size()) + " files.");

  std::vector<MetaData> results;
  std::mutex resultsMutex;

  // 3. Define the specific worker function logic
  auto workerLogic = [&](const File &file) {
    try {
      MetaData m = MetaData{file.name, fs::file_size(file.name)};
      std::lock_guard<std::mutex> lock(resultsMutex);
      results.push_back(m);
    } catch (...) {
      // Ignore errors
    }
  };

  // 4. Initialize the pool with the specific worker
  FileProcessorPool pool(numThreads, workerLogic);

  // 5. Add all files to the pool
  for (const auto &file : files_to_scan) {
    pool.addFile(file);
  }

  // 6. Wait for all files to be processed
  print("Processing files...");
  pool.waitForCompletion();

  print("Processed all files.");
  std::cout << "\nResults (First 5):" << std::endl;
  for (size_t i = 0; i < std::min(results.size(), (size_t)5); ++i) {
    std::cout << "[" << i + 1 << "] " << results[i].filename << ": "
              << results[i].size << " bytes" << std::endl;
  }

  return 0;
}
