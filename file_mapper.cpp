#include "BoundedQueue.h"
#include "print_utils.h"
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

vector<string> getFiles(const string &path) {
  vector<string> filePaths;
  try {
    if (fs::exists(path) && fs::is_directory(path)) {
      for (const auto &entry : fs::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
          filePaths.push_back(entry.path().string());
        }
      }
    }
  } catch (const fs::filesystem_error &e) {
    cerr << "Error during traversal: " << e.what() << endl;
  }
  return filePaths;
}

std::vector<std::string> fileMetadataMapper(const std::string &filepath) {
  std::vector<std::string> metadata;

  try {
    // 1. Get File Size natively via filesystem
    auto size = fs::file_size(filepath);
    metadata.push_back("Size: " + std::to_string(size) + " bytes");

    // 2. Get the Extension (e.g., ".csv", ".json")
    metadata.push_back("Ext: " + fs::path(filepath).extension().string());

  } catch (const std::exception &e) {
    // If we hit a permissions error, safely log it in our metadata
    metadata.push_back("Error: " + std::string(e.what()));
  }

  return metadata;
}

using MapperFunc = std::function<std::vector<std::string>(const std::string &)>;

std::map<std::string, std::vector<std::string>>
processFiles(const std::vector<std::string> &files, MapperFunc mapper,
             int maxFuturesInFlight,
             int numWorkers) // You need to specify how many threads to spin up!
{
  BoundedQueue<std::string> bq(maxFuturesInFlight);
  std::map<std::string, std::vector<std::string>> fileMap;
  std::mutex map_mutex; // A lock specifically to protect our results map

  // 1. Define the Worker logic
  auto worker = [&]() {
    while (auto file = bq.pop()) {
      // Step A: Do the heavy lifting OUTSIDE the lock.
      // Multiple threads can run the mapper concurrently.
      auto mapdata = mapper(*file);

      // Step B: The Critical Section. Lock the map just long enough to insert.
      {
        std::lock_guard<std::mutex> lock(map_mutex);
        fileMap[*file] = std::move(mapdata);
      }
    }
  };

  // 2. Start the Consumer Threads FIRST
  std::vector<std::thread> threads;
  for (int i = 0; i < numWorkers; ++i) {
    threads.emplace_back(worker);
  }

  // 3. The Main Thread becomes the Producer
  for (const auto &file : files) {
    // If the queue fills up, the main thread gracefully sleeps here
    // until the workers process some files. This is perfect backpressure!
    bq.push(file);
  }

  // 4. Shutdown gracefully
  bq.stop(); // Tell the queue no more files are coming

  for (auto &t : threads) {
    t.join(); // Wait for all workers to finish their current files and exit
  }

  return fileMap;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <directory_path>" << endl;
    return 1;
  }

  string rootPath = argv[1];
  cout << "Scanning directory: " << rootPath << "..." << endl;

  vector<string> filePaths = getFiles(rootPath);
  auto resultMap = processFiles(filePaths, fileMetadataMapper, 10, 4);
  // Print the final map
  for (const auto &[file, meta] : resultMap) {
    std::cout << "\nFile: " << file << "\n";
    for (const auto &m : meta) {
      std::cout << "  -> " << m << "\n";
    }
  }
  return 0;
}