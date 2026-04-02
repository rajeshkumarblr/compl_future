#include <chrono>
#include <deque>
#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

// 1. Define your input and output types
struct File {
  std::string name;
};

struct MetaData {
  std::string filename;
  uintmax_t size;
};

// 2. Define the generic function signature using std::function
// Return std::future because it's the C++ equivalent of CompletableFuture
using MapperFunc = std::function<std::future<MetaData>(const File &)>;

// 3. Process files in multiple batches with a sliding window of active futures
std::vector<MetaData> processBatch(const std::vector<File> &files,
                                   MapperFunc mapper, int maxFuturesInFlight) {
  std::vector<MetaData> results;
  std::deque<std::future<MetaData>> futures;

  for (const auto &file : files) {
    // If we reach the limit of futures in flight, wait for the oldest one
    if (futures.size() >= static_cast<size_t>(maxFuturesInFlight)) {
      results.push_back(futures.front().get());
      futures.pop_front();
    }
    // Launch a new future via the mapper
    futures.push_back(mapper(file));
  }

  // Wait for all remaining futures to complete
  while (!futures.empty()) {
    results.push_back(futures.front().get());
    futures.pop_front();
  }
  std::cout << "\nDone!" << std::endl;

  return results;
}

int main(int argc, char *argv[]) {
  std::string root_path = "/home/rajesh/proj/dlake";
  int maxInFlight = 10;
  // 1. Define a truly generic print function using std::function
  // This can be reassigned to ANY function/lambda with the same signature.
  std::function<void(const std::string &)> print =
      [](const std::string &message) {
        std::cout << "[INFO] " << message << std::endl;
      };

  print("Starting the future process...");

  // Example: Reassigning to a different "custom" function
  print = [](const std::string &message) {
    std::cout << ">>> CUSTOM LOG: " << message << std::endl;
  };

  print("Print behavior has been customized!");
  // 1. Handle command line arguments
  if (argc > 1) {
    root_path = argv[1];
  }
  if (argc > 2) {
    try {
      maxInFlight = std::stoi(argv[2]);
      if (maxInFlight <= 0) {
        throw std::invalid_argument("Concurrency must be positive");
      }
    } catch (...) {
      std::cerr << "Warning: Invalid concurrency limit '" << argv[2]
                << "'. Using default: 10" << std::endl;
      maxInFlight = 10;
    }
  }

  std::vector<File> files_to_process;

  std::cout << "Searching for files in: " << root_path << "..." << std::endl;
  std::cout << "Concurrency limit (maxInFlight): " << maxInFlight << std::endl;

  // 2. Traverse the directory recursively
  try {
    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
      std::cerr << "Error: " << root_path << " is not a valid directory."
                << std::endl;
      return 1;
    }

    for (const auto &entry : fs::recursive_directory_iterator(
             root_path, fs::directory_options::skip_permission_denied)) {
      if (entry.is_regular_file()) {
        files_to_process.push_back({entry.path().string()});
      }
    }
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error during directory traversal: " << e.what() << std::endl;
  }

  if (files_to_process.empty()) {
    std::cout << "No files found to process." << std::endl;
    return 0;
  }

  std::cout << "Found " << files_to_process.size() << " files." << std::endl;

  std::vector<MetaData> myResults;
  std::mutex resultsMutex;

  // 3. Define the mapper: gets the file size asynchronously
  auto mapper = [&myResults,
                 &resultsMutex](const File &f) -> std::future<MetaData> {
    return std::async(std::launch::async, [&myResults, &resultsMutex, f]() {
      try {
        MetaData m = MetaData{f.name, fs::file_size(f.name)};
        {
          std::lock_guard<std::mutex> lock(resultsMutex);
          myResults.push_back(m);
        }
        return m;
      } catch (...) {
        return MetaData{f.name, 0};
      }
    });
  };

  // 4. Process the files with the specified concurrency limit
  auto results = processBatch(files_to_process, mapper, maxInFlight);

  // 5. Print truncated results
  std::cout << "\nResults:" << std::endl;
  for (size_t i = 0; i < myResults.size(); ++i) {
    std::cout << "[" << i + 1 << "] " << results[i].filename << ": "
              << results[i].size << " bytes" << std::endl;
  }

  return 0;
}