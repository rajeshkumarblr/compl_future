#include <deque>
#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <vector>

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
  const size_t total = files.size();
  size_t processed = 0;

  for (const auto &file : files) {
    // If we reach the limit of futures in flight, wait for the oldest one
    if (futures.size() >= static_cast<size_t>(maxFuturesInFlight)) {
      results.push_back(futures.front().get());
      futures.pop_front();
      processed++;

      // Progress reporting
      if (processed % 100 == 0 || processed == total) {
        std::cout << "\rProcessing: " << processed << "/" << total << " ("
                  << (processed * 100 / total) << "%) " << std::flush;
      }
    }
    // Launch a new future via the mapper
    futures.push_back(mapper(file));
  }

  // Wait for all remaining futures to complete
  while (!futures.empty()) {
    results.push_back(futures.front().get());
    futures.pop_front();
    processed++;
    std::cout << "\rProcessing: " << processed << "/" << total << " ("
              << (processed * 100 / total) << "%) " << std::flush;
  }
  std::cout << "\nDone!" << std::endl;

  return results;
}

int main(int argc, char *argv[]) {
  std::string root_path = "/home/rajesh";
  int maxInFlight = 10;

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

  // 3. Define the mapper: gets the file size asynchronously
  auto mapper = [](const File &f) -> std::future<MetaData> {
    return std::async(std::launch::async, [f]() {
      try {
        return MetaData{f.name, fs::file_size(f.name)};
      } catch (...) {
        return MetaData{f.name, 0};
      }
    });
  };

  // 4. Process the files with the specified concurrency limit
  auto results = processBatch(files_to_process, mapper, maxInFlight);

  // 5. Print truncated results
  std::cout << "\nSample Results (Top 10):" << std::endl;
  for (size_t i = 0; i < results.size() && i < 10; ++i) {
    std::cout << "[" << i + 1 << "] " << results[i].filename << ": "
              << results[i].size << " bytes" << std::endl;
  }

  return 0;
}